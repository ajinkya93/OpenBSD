/*	$OpenBSD: param.h,v 1.8 1997/02/14 17:52:26 kstailey Exp $	*/
/*	$NetBSD: param.h,v 1.34 1996/03/04 05:04:40 cgd Exp $	*/

/*
 * Copyright (c) 1994, 1995 Gordon W. Ross
 * Copyright (c) 1993 Adam Glass
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1982, 1986, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	from: Utah Hdr: machparam.h 1.16 92/12/20
 *	from: @(#)param.h	8.1 (Berkeley) 6/10/93
 */

#ifndef	MACHINE

/*
 * Machine dependent constants for the Sun3 series.
 */
#define	_MACHINE	sun3
#define	MACHINE		"sun3"
#define	_MACHINE_ARCH	m68k
#define	MACHINE_ARCH	"m68k"
#define	MID_MACHINE	MID_M68K

/*
 * Round p (pointer or byte index) up to a correctly-aligned value
 * for all data types (int, long, ...).   The result is u_int and
 * must be cast to any desired pointer type.
 */
#define	ALIGNBYTES	3
#define	ALIGN(p)	(((u_int)(p) + ALIGNBYTES) &~ ALIGNBYTES)

#define	NBPG		8192		/* bytes/page */
#define	PGOFSET		(NBPG-1)	/* byte offset into page */
#define	PGSHIFT		13		/* LOG2(NBPG) */

#define NBSG		0x20000	/* bytes/segment */
#define	SEGOFSET	(NBSG-1)	/* byte offset into segment */
#define SEGSHIFT	17	        /* LOG2(NBSG) */

#define	KERNBASE	0x0E000000	/* start of kernel virtual */
#define	BTOPKERNBASE	((u_long)KERNBASE >> PGSHIFT)
#define	KERNTEXTOFF	0x0E004000	/* start of kernel text */

#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define BLKDEV_IOSIZE	2048
#define	MAXPHYS		(64 * 1024)	/* max raw I/O transfer size */

#define	CLSIZE		1
#define	CLSIZELOG2	0

/* NOTE: SSIZE, SINCR and UPAGES must be multiples of CLSIZE */
#define	SSIZE		1		/* initial stack size/NBPG */
#define	SINCR		1		/* increment of stack/NBPG */

#define	UPAGES		2		/* pages of u-area */
#define	USPACE		(UPAGES << PGSHIFT)

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define	MSIZE		128		/* size of an mbuf */
#define	MCLSHIFT	11
#define	MCLBYTES	(1 << MCLSHIFT)	/* large enough for ether MTU */
#define	MCLOFSET	(MCLBYTES - 1)
#ifndef NMBCLUSTERS
#ifdef GATEWAY
#define	NMBCLUSTERS	512		/* map size, max cluster allocation */
#else
#define	NMBCLUSTERS	256		/* map size, max cluster allocation */
#endif
#endif

/*
 * Size of kernel malloc arena in CLBYTES-sized logical pages
 */ 
#ifndef NKMEMCLUSTERS
#define	NKMEMCLUSTERS	(2048*1024/CLBYTES)
#endif

/* pages ("clicks") to disk blocks */
#define	ctod(x)		((x) << (PGSHIFT - DEV_BSHIFT))
#define	dtoc(x)		((x) >> (PGSHIFT - DEV_BSHIFT))

/* pages to bytes */
#define	ctob(x)		((x) << PGSHIFT)
#define	btoc(x)		(((x) + PGOFSET) >> PGSHIFT)

/* bytes to disk blocks */
#define	btodb(x)	((x) >> DEV_BSHIFT)
#define	dbtob(x)	((x) << DEV_BSHIFT)

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and should use the bsize
 * field from the disk label.
 * For now though just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/*
 * Mach derived conversion macros
 */
#define sun3_round_seg(x)	((((unsigned)(x)) + SEGOFSET) & ~SEGOFSET)
#define sun3_trunc_seg(x)	((unsigned)(x) & ~SEGOFSET)
#define sun3_btos(x)		((unsigned)(x) >> SEGSHIFT)
#define sun3_stob(x)		((unsigned)(x) << SEGSHIFT)

#define sun3_round_page(x)	((((unsigned)(x)) + PGOFSET) & ~PGOFSET)
#define sun3_trunc_page(x)	((unsigned)(x) & ~PGOFSET)
#define sun3_btop(x)		((unsigned)(x) >> PGSHIFT)
#define sun3_ptob(x)		((unsigned)(x) << PGSHIFT)

#include <machine/psl.h>

#if defined(_KERNEL) && !defined(_LOCORE)

#ifndef __GNUC__
/* No inline, use real function in locore.s */
extern int _spl(int new);
#else	/* GNUC */
/*
 * Define an inline function for PSL manipulation.
 * This is as close to a macro as one can get.
 * If not optimizing, the one in locore.s is used.
 * (See the GCC extensions info document.)
 */
extern __inline__ int _spl(int new)
{
	register int old;

	__asm __volatile (
		"clrl %0; movew sr,%0; movew %1,sr" :
			"&=d" (old) : "di" (new));
	return (old);
}
#endif	/* GNUC */

/*
 * The rest of this is sun3 specific, because other ports may
 * need to do special things in spl0() (i.e. simulate SIR).
 * Suns have a REAL interrupt register, so spl0() and splx(s)
 * have no need to check for any simulated interrupts, etc.
 */

#define spl0()  _spl(PSL_S|PSL_IPL0)
#define spl1()  _spl(PSL_S|PSL_IPL1)
#define spl2()  _spl(PSL_S|PSL_IPL2)
#define spl3()  _spl(PSL_S|PSL_IPL3)
#define spl4()  _spl(PSL_S|PSL_IPL4)
#define spl5()  _spl(PSL_S|PSL_IPL5)
#define spl6()  _spl(PSL_S|PSL_IPL6)
#define spl7()  _spl(PSL_S|PSL_IPL7)
#define splx(x)	_spl(x)

/* IPL used by soft interrupts: netintr(), softclock() */
#define splsoftclock()  spl1()
#define splsoftnet()    spl1()

/* Highest block device (strategy) IPL. */
#define splbio()        spl2()

/* Highest network interface IPL. */
#define splnet()        spl3()

/* Highest tty device IPL. */
#define spltty()        spl4()

/* Requirement: imp >= (highest network, tty, or disk IPL) */
#define splimp()        spl4()

/* Intersil clock hardware interrupts (hard-wired at 5) */
#define splclock()      spl5()
#define splstatclock()  splclock()

/* Zilog Serial hardware interrupts (hard-wired at 6) */
#define splzs()         spl6()

/* Block out all interrupts (except NMI of course). */
#define splhigh()       spl7()
#define splsched()      spl7()

/* Get current sr value (debug, etc.) */
extern int getsr __P((void));

extern void _delay __P((unsigned));
#define delay(us)	_delay((us)<<8)
#define	DELAY(n)	delay(n)
#endif	/* _KERNEL && !_LOCORE */

#endif	/* MACHINE */
