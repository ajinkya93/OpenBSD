/*	$OpenBSD: intel_i2c.c,v 1.8 2016/04/16 17:39:44 kettenis Exp $	*/
/*
 * Copyright (c) 2012, 2013 Mark Kettenis <kettenis@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 2006 Dave Airlie <airlied@linux.ie>
 * Copyright © 2006-2008,2010 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Eric Anholt <eric@anholt.net>
 *	Chris Wilson <chris@chris-wilson.co.uk>
 */

#include <dev/pci/drm/drmP.h>
#include <dev/pci/drm/drm.h>
#include "intel_drv.h"
#include "i915_drv.h"
#include "i915_reg.h"

#include <dev/i2c/i2cvar.h>
#include <dev/i2c/i2c_bitbang.h>

int	intel_gmbus_acquire_bus(void *, int);
void	intel_gmbus_release_bus(void *, int);
int	intel_gmbus_exec(void *, i2c_op_t, i2c_addr_t, const void *, size_t,
	    void *buf, size_t, int);

void	intel_gpio_bb_set_bits(void *, uint32_t);
void	intel_gpio_bb_set_dir(void *, uint32_t);
uint32_t intel_gpio_bb_read_bits(void *);

int	intel_gpio_send_start(void *, int);
int	intel_gpio_send_stop(void *, int);
int	intel_gpio_initiate_xfer(void *, i2c_addr_t, int);
int	intel_gpio_read_byte(void *, u_int8_t *, int);
int	intel_gpio_write_byte(void *, u_int8_t, int);

enum disp_clk {
	CDCLK,
	CZCLK
};

int	gmbus_ports[] = { GPIOB, GPIOA, GPIOC, GPIOD, GPIOE, GPIOF };

static inline struct intel_gmbus *
to_intel_gmbus(struct i2c_controller *i2c)
{
	return container_of(i2c, struct intel_gmbus, controller);
}

static int get_disp_clk_div(struct drm_i915_private *dev_priv,
			    enum disp_clk clk)
{
	u32 reg_val;
	int clk_ratio;

	reg_val = I915_READ(CZCLK_CDCLK_FREQ_RATIO);

	if (clk == CDCLK)
		clk_ratio =
			((reg_val & CDCLK_FREQ_MASK) >> CDCLK_FREQ_SHIFT) + 1;
	else
		clk_ratio = (reg_val & CZCLK_FREQ_MASK) + 1;

	return clk_ratio;
}

static void gmbus_set_freq(struct drm_i915_private *dev_priv)
{
	int vco, gmbus_freq = 0, cdclk_div;

	BUG_ON(!IS_VALLEYVIEW(dev_priv->dev));

	vco = valleyview_get_vco(dev_priv);

	/* Get the CDCLK divide ratio */
	cdclk_div = get_disp_clk_div(dev_priv, CDCLK);

	/*
	 * Program the gmbus_freq based on the cdclk frequency.
	 * BSpec erroneously claims we should aim for 4MHz, but
	 * in fact 1MHz is the correct frequency.
	 */
	if (cdclk_div)
		gmbus_freq = (vco << 1) / cdclk_div;

	if (WARN_ON(gmbus_freq == 0))
		return;

	I915_WRITE(GMBUSFREQ_VLV, gmbus_freq);
}

void
intel_i2c_reset(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	/*
	 * In BIOS-less system, program the correct gmbus frequency
	 * before reading edid.
	 */
	if (IS_VALLEYVIEW(dev))
		gmbus_set_freq(dev_priv);

	I915_WRITE(dev_priv->gpio_mmio_base + GMBUS0, 0);
	I915_WRITE(dev_priv->gpio_mmio_base + GMBUS4, 0);
}

int
intel_gmbus_acquire_bus(void *cookie, int flags)
{
	struct intel_gmbus *bus = cookie;
	struct inteldrm_softc *dev_priv = bus->dev_priv;

	intel_aux_display_runtime_get(dev_priv);

	I915_WRITE(dev_priv->gpio_mmio_base + GMBUS0, bus->reg0);

	return (0);
}

void
intel_gmbus_release_bus(void *cookie, int flags)
{
	struct intel_gmbus *bus = cookie;
	struct inteldrm_softc *dev_priv = bus->dev_priv;

	I915_WRITE(dev_priv->gpio_mmio_base + GMBUS0, 0);

	intel_aux_display_runtime_put(dev_priv);
}

int
intel_gmbus_exec(void *cookie, i2c_op_t op, i2c_addr_t addr,
    const void *cmdbuf, size_t cmdlen, void *buf, size_t len, int flags)
{
	struct intel_gmbus *bus = cookie;
	struct inteldrm_softc *dev_priv = bus->dev_priv;
	uint32_t reg, st, val;
	int reg_offset = dev_priv->gpio_mmio_base;
	uint8_t *b;
	int i, retries;
	uint16_t rem = len;
	int bus_err = 0;

	if (cmdlen > 1)
		return (EOPNOTSUPP);

	if (I2C_OP_WRITE_P(op)) {
		val = 0;	

		b = buf;
		for (i = 0; i < 4 && rem > 0; i++, rem--) {
			val |= *b++ << (8 * i);
		}

		I915_WRITE(GMBUS3 + reg_offset, val);
	}

	reg = 0;
	if (len > 0)
		reg |= GMBUS_CYCLE_WAIT;
	if (I2C_OP_STOP_P(op))
		reg |= GMBUS_CYCLE_STOP;
	if (I2C_OP_READ_P(op))
		reg |= GMBUS_SLAVE_READ;
	else
		reg |= GMBUS_SLAVE_WRITE;
	if (cmdlen > 0) {
		reg |= GMBUS_CYCLE_INDEX;
		b = (void *)cmdbuf;
		reg |= (b[0] << GMBUS_SLAVE_INDEX_SHIFT);
	}
	reg |= (addr << GMBUS_SLAVE_ADDR_SHIFT);
	reg |= (len << GMBUS_BYTE_COUNT_SHIFT);
	I915_WRITE(GMBUS1 + reg_offset, reg | GMBUS_SW_RDY);

	if (I2C_OP_READ_P(op)) {
		b = buf;
		while (len > 0) {
			for (retries = 50; retries > 0; retries--) {
				st = I915_READ(GMBUS2 + reg_offset);
				if (st & (GMBUS_SATOER | GMBUS_HW_RDY))
					break;
				DELAY(1000);
			}
			if (st & GMBUS_SATOER) {
				bus_err = 1;
				goto out;
			}
			if ((st & GMBUS_HW_RDY) == 0)
				return (ETIMEDOUT);

			val = I915_READ(GMBUS3 + reg_offset);
			for (i = 0; i < 4 && len > 0; i++, len--) {
				*b++ = val & 0xff;
				val >>= 8;
			}
		}
	}

	if (I2C_OP_WRITE_P(op)) {
		while (rem > 0) {
			for (retries = 50; retries > 0; retries--) {
				st = I915_READ(GMBUS2 + reg_offset);
				if (st & (GMBUS_SATOER | GMBUS_HW_RDY))
					break;
				DELAY(1000);
			}
			if (st & GMBUS_SATOER) {
				bus_err = 1;
				goto out;
			}
			if ((st & GMBUS_HW_RDY) == 0)
				return (ETIMEDOUT);

			val = 0;
			for (i = 0; i < 4 && rem > 0; i++, rem--) {
				val |= *b++ << (8 * i);
			}
			I915_WRITE(GMBUS3 + reg_offset, val);
		}
	}

out:
	if (I2C_OP_STOP_P(op)) {
		for (retries = 10; retries > 0; retries--) {
			st = I915_READ(GMBUS2 + reg_offset);
			if (st & GMBUS_SATOER)
				bus_err = 1;
			if ((st & GMBUS_ACTIVE) == 0)
				break;
			DELAY(1000);
		}
		if (st & GMBUS_ACTIVE)
			return (ETIMEDOUT);
	} else {
		for (retries = 10; retries > 0; retries--) {
			st = I915_READ(GMBUS2 + reg_offset);
			if (st & GMBUS_SATOER)
				bus_err = 1;
			if (st & GMBUS_HW_WAIT_PHASE)
				break;
			DELAY(1000);
		}
		if ((st & GMBUS_HW_WAIT_PHASE) == 0)
			return (ETIMEDOUT);
	}

	/* after the bus is idle clear the bus error */
	if (bus_err) {
		I915_WRITE(GMBUS1 + reg_offset, GMBUS_SW_CLR_INT);
		I915_WRITE(GMBUS1 + reg_offset, 0);
		I915_WRITE(GMBUS0 + reg_offset, 0);
		return (ENXIO);
	}

	return (0);
}

struct i2c_bitbang_ops intel_gpio_bbops = {
	intel_gpio_bb_set_bits,
	intel_gpio_bb_set_dir,
	intel_gpio_bb_read_bits,
	{ GPIO_DATA_VAL_IN, GPIO_CLOCK_VAL_IN,
	  GPIO_DATA_DIR_OUT, GPIO_DATA_DIR_IN }
};

void
intel_gpio_bb_set_bits(void *cookie, uint32_t bits)
{
	struct intel_gmbus *bus = cookie;
	struct inteldrm_softc *dev_priv = bus->dev_priv;
	uint32_t reg;

	reg = I915_READ_NOTRACE(bus->gpio_reg);
	reg &= (GPIO_DATA_PULLUP_DISABLE | GPIO_CLOCK_PULLUP_DISABLE);
	if (bits & GPIO_DATA_VAL_IN)
		reg |= GPIO_DATA_VAL_OUT;
	if (bits & GPIO_CLOCK_VAL_IN)
		reg |= GPIO_CLOCK_VAL_OUT;
	reg |= (GPIO_DATA_VAL_MASK | GPIO_CLOCK_VAL_MASK);
	I915_WRITE_NOTRACE(bus->gpio_reg, reg);
}

void
intel_gpio_bb_set_dir(void *cookie, uint32_t bits)
{
	struct intel_gmbus *bus = cookie;
	struct inteldrm_softc *dev_priv = bus->dev_priv;
	uint32_t reg;

	reg = I915_READ_NOTRACE(bus->gpio_reg);
	reg &= (GPIO_DATA_PULLUP_DISABLE | GPIO_CLOCK_PULLUP_DISABLE);
	if (bits & GPIO_DATA_DIR_OUT)
		reg |= GPIO_DATA_DIR_OUT;
	reg |= (GPIO_DATA_DIR_MASK | GPIO_CLOCK_DIR_OUT | GPIO_CLOCK_DIR_MASK);
	I915_WRITE_NOTRACE(bus->gpio_reg, reg);
}

uint32_t
intel_gpio_bb_read_bits(void *cookie)
{
	struct intel_gmbus *bus = cookie;
	struct inteldrm_softc *dev_priv = bus->dev_priv;

	return I915_READ_NOTRACE(bus->gpio_reg);
}

int
intel_gpio_send_start(void *cookie, int flags)
{
	return (i2c_bitbang_send_start(cookie, flags, &intel_gpio_bbops));
}

int
intel_gpio_send_stop(void *cookie, int flags)
{
	return (i2c_bitbang_send_stop(cookie, flags, &intel_gpio_bbops));
}

int
intel_gpio_initiate_xfer(void *cookie, i2c_addr_t addr, int flags)
{
	return (i2c_bitbang_initiate_xfer(cookie, addr, flags, &intel_gpio_bbops));
}

int
intel_gpio_read_byte(void *cookie, u_int8_t *bytep, int flags)
{
	return (i2c_bitbang_read_byte(cookie, bytep, flags, &intel_gpio_bbops));
}

int
intel_gpio_write_byte(void *cookie, u_int8_t byte, int flags)
{
	return (i2c_bitbang_write_byte(cookie, byte, flags, &intel_gpio_bbops));
}

int
intel_setup_gmbus(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	int i;

	if (HAS_PCH_NOP(dev))
		return 0;
	else if (HAS_PCH_SPLIT(dev))
		dev_priv->gpio_mmio_base = PCH_GPIOA - GPIOA;
	else if (IS_VALLEYVIEW(dev))
		dev_priv->gpio_mmio_base = VLV_DISPLAY_BASE;
	else
		dev_priv->gpio_mmio_base = 0;

	for (i = 0; i < GMBUS_NUM_PORTS; i++) {
		struct intel_gmbus *bus = &dev_priv->gmbus[i];

		bus->dev_priv = dev_priv;

		bus->speed = GMBUS_RATE_100KHZ;
		bus->port = i + 1;
		bus->force_bit = 0;
		bus->reg0 = bus->port | bus->speed;

		bus->controller.ic_cookie = bus;
		bus->controller.ic_acquire_bus = intel_gmbus_acquire_bus;
		bus->controller.ic_release_bus = intel_gmbus_release_bus;
		bus->controller.ic_exec = intel_gmbus_exec;

		bus->gpio_reg = dev_priv->gpio_mmio_base + gmbus_ports[i];

		bus->controller.ic_send_start = intel_gpio_send_start;
		bus->controller.ic_send_stop = intel_gpio_send_stop;
		bus->controller.ic_initiate_xfer = intel_gpio_initiate_xfer;
		bus->controller.ic_read_byte = intel_gpio_read_byte;
		bus->controller.ic_write_byte = intel_gpio_write_byte;
	}

	intel_i2c_reset(dev_priv->dev);

	return (0);
}

void
intel_teardown_gmbus(struct drm_device *dev)
{
}

struct i2c_controller *
intel_gmbus_get_adapter(drm_i915_private_t *dev_priv, unsigned port)
{
	/* -1 to map pin pair to gmbus index */
	return (intel_gmbus_is_port_valid(port)) ?
	    &dev_priv->gmbus[port - 1].controller : NULL;
}

void
intel_gmbus_force_bit(struct i2c_controller *controller, bool force_bit)
{
	struct intel_gmbus *bus = to_intel_gmbus(controller);

	if (force_bit)
		bus->force_bit++;
	else
		bus->force_bit--;

	KASSERT(force_bit >= 0);

	if (bus->force_bit) {
		bus->controller.ic_exec = NULL;
		bus->reg0 = 0;
	} else {
		bus->controller.ic_exec = intel_gmbus_exec;
		bus->reg0 = bus->port | bus->speed;
	}
}
