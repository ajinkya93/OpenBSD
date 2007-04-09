/*	$OpenBSD: rtc.c,v 1.1 2007/04/09 19:59:06 kettenis Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1994 Gordon W. Ross
 * Copyright (c) 1993 Adam Glass
 * Copyright (c) 1996 Paul Kranenburg
 * Copyright (c) 1996
 * 	The President and Fellows of Harvard College. All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by Harvard University.
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 *	This product includes software developed by Paul Kranenburg.
 *	This product includes software developed by Harvard University.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * Driver for rtc device on Blade 1000, Fire V210, etc.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/systm.h>

#include <machine/bus.h>
#include <machine/autoconf.h>

#include <dev/clock_subr.h>
#include <dev/ic/mc146818reg.h>

#include <sparc64/dev/ebusreg.h>
#include <sparc64/dev/ebusvar.h>

extern todr_chip_handle_t todr_handle;

struct rtc_softc {
	struct device		sc_dv;
	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;
	struct intrhand		*sc_ih;
};

int	rtc_match(struct device *, void *, void *);
void	rtc_attach(struct device *, struct device *, void *);

struct cfattach rtc_ca = {
	sizeof(struct rtc_softc), rtc_match, rtc_attach
};

struct cfdriver rtc_cd = {
	NULL, "rtc", DV_DULL
};

int rtc_intr(void *arg);

u_int8_t rtc_read_reg(struct rtc_softc *, bus_size_t);
void rtc_write_reg(struct rtc_softc *sc, bus_size_t, u_int8_t);

int rtc_gettime(todr_chip_handle_t, struct timeval *);
int rtc_settime(todr_chip_handle_t, struct timeval *);
int rtc_getcal(todr_chip_handle_t, int *);
int rtc_setcal(todr_chip_handle_t, int);

int
rtc_match(struct device *parent, void *cf, void *aux)
{
	struct ebus_attach_args *ea = aux;

	if (strcmp("rtc", ea->ea_name) == 0)
		return (1);
	return (0);
}

void
rtc_attach(struct device *parent, struct device *self, void *aux)
{
	struct rtc_softc *sc = (void *)self;
	struct ebus_attach_args *ea = aux;
	todr_chip_handle_t handle;
	char *model;

	if (ebus_bus_map(ea->ea_iotag, 0,
	    EBUS_PADDR_FROM_REG(&ea->ea_regs[0]),
	    ea->ea_regs[0].size, 0, 0, &sc->sc_ioh) == 0) {
		sc->sc_iot = ea->ea_iotag;
	} else if (ebus_bus_map(ea->ea_memtag, 0,
	    EBUS_PADDR_FROM_REG(&ea->ea_regs[0]),
	    ea->ea_regs[0].size, 0, 0, &sc->sc_ioh) == 0) {
		sc->sc_iot = ea->ea_memtag;
	} else {
		printf("%s: can't map register\n", self->dv_xname);
		return;
	}

	model = getpropstring(ea->ea_node, "model");
#ifdef DIAGNOSTIC
	if (model == NULL)
		panic("rtc_attach: no model property");
#endif
	printf(": %s\n", model);

	/* Setup our todr_handle */
	handle = malloc(sizeof(struct todr_chip_handle), M_DEVBUF, M_NOWAIT);
	if (handle == NULL)
		panic("couldn't allocate todr_handle");
	handle->cookie = sc;
	handle->todr_gettime = rtc_gettime;
	handle->todr_settime = rtc_settime;
	handle->todr_getcal = rtc_getcal;
	handle->todr_setcal = rtc_setcal;

	handle->bus_cookie = NULL;
	handle->todr_setwen = NULL;
	todr_handle = handle;

	/* 
	 * Turn interrupts off, just in case. (Although they shouldn't
	 * be wired to an interrupt controller on sparcs).
	 */
	rtc_write_reg(sc->sc_iot, sc->sc_ioh,
	    MC_REGB, MC_REGB_BINARY | MC_REGB_24HR);

	/*
	 * On ds1287 models (which really are ns87317 chips), the
	 * interrupt is wired to the powerbutton.
	 */
	if(strcmp(model, "ds1287") == 0 && ea->ea_nintrs > 0) {
		sc->sc_ih = bus_intr_establish(sc->sc_iot, ea->ea_intrs[0],
		    IPL_BIO, 0, rtc_intr, sc, self->dv_xname);
		if (sc->sc_ih == NULL) {
			printf("%s: can't establush interrupt\n",
			    self->dv_xname);
		}
	}
}

int
rtc_intr(void *arg)
{
	extern int kbd_reset;

	if (kbd_reset == 1) {
		kbd_reset = 0;
		psignal(initproc, SIGUSR1);
	}
	return (1);
}

/*
 * Register access is indirect, through an address and data port.
 */

#define	RTC_ADDR	0
#define	RTC_DATA	1

u_int8_t 
rtc_read_reg(struct rtc_softc *sc, bus_size_t reg)
{
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, RTC_ADDR, reg);
	return (bus_space_read_1(sc->sc_iot, sc->sc_ioh, RTC_DATA));
}

void 
rtc_write_reg(struct rtc_softc *sc, bus_size_t reg, u_int8_t val)
{
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, RTC_ADDR, reg);
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, RTC_DATA, val);
}

/*
 * RTC todr routines.
 */

/*
 * Get time-of-day and convert to a `struct timeval'
 * Return 0 on success; an error number otherwise.
 */
int
rtc_gettime(todr_chip_handle_t handle, struct timeval *tv)
{
	struct rtc_softc *sc = handle->cookie;
	struct clock_ymdhms dt;
	int year;
	u_int8_t csr;

	/* Stop updates. */
	csr = rtc_read_reg(sc, MC_REGB);
	csr |= MC_REGB_SET;
	rtc_write_reg(sc, MC_REGB, csr);

	/* Read time */
	dt.dt_sec = rtc_read_reg(sc, MC_SEC);
	dt.dt_min = rtc_read_reg(sc, MC_MIN);
	dt.dt_hour = rtc_read_reg(sc, MC_HOUR);
	dt.dt_day = rtc_read_reg(sc, MC_DOM);
	dt.dt_wday = rtc_read_reg(sc, MC_DOW);
	dt.dt_mon = rtc_read_reg(sc, MC_MONTH);
	year = rtc_read_reg(sc, MC_YEAR);

	if ((year += 1900) < POSIX_BASE_YEAR)
		year += 100;

	dt.dt_year = year;

	/* time wears on */
	csr = rtc_read_reg(sc, MC_REGB);
	csr &= ~MC_REGB_SET;
	rtc_write_reg(sc, MC_REGB, csr);

	/* simple sanity checks */
	if (dt.dt_mon > 12 || dt.dt_day > 31 ||
	    dt.dt_hour >= 24 || dt.dt_min >= 60 || dt.dt_sec >= 60)
		return (1);

	tv->tv_sec = clock_ymdhms_to_secs(&dt);
	tv->tv_usec = 0;
	return (0);
}

/*
 * Set the time-of-day clock based on the value of the `struct timeval' arg.
 * Return 0 on success; an error number otherwise.
 */
int
rtc_settime(todr_chip_handle_t handle, struct timeval *tv)
{
	struct rtc_softc *sc = handle->cookie;
	struct clock_ymdhms dt;
	u_int8_t csr;
	int year;

	/* Note: we ignore `tv_usec' */
	clock_secs_to_ymdhms(tv->tv_sec, &dt);

	year = dt.dt_year % 100;

	/* enable write */
	csr = rtc_read_reg(sc, MC_REGB);
	csr |= MC_REGB_SET;
	rtc_write_reg(sc, MC_REGB, csr);

	rtc_write_reg(sc, MC_SEC, dt.dt_sec);
	rtc_write_reg(sc, MC_MIN, dt.dt_min);
	rtc_write_reg(sc, MC_HOUR, dt.dt_hour);
	rtc_write_reg(sc, MC_DOW, dt.dt_wday);
	rtc_write_reg(sc, MC_DOM, dt.dt_day);
	rtc_write_reg(sc, MC_MONTH, dt.dt_mon);
	rtc_write_reg(sc, MC_YEAR, year);

	/* load them up */
	csr = rtc_read_reg(sc, MC_REGB);
	csr &= ~MC_REGB_SET;
	rtc_write_reg(sc, MC_REGB, csr);
	return (0);
}

int
rtc_getcal(todr_chip_handle_t handle, int *vp)
{
	return (EOPNOTSUPP);
}

int
rtc_setcal(todr_chip_handle_t handle, int v)
{
	return (EOPNOTSUPP);
}
