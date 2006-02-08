/*	$OpenBSD: ofwi2c.c,v 1.2 2006/02/08 23:15:58 dlg Exp $	*/

/*
 * Copyright (c) 2006 Theo de Raadt
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <dev/i2c/i2cvar.h>
#include <dev/ofw/openfirm.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <sparc64/pci_machdep.h>

#include <arch/sparc64/dev/ofwi2cvar.h>

void
ofwiic_scan(struct device *self, struct i2cbus_attach_args *iba, void *aux)
{
	struct pci_attach_args *pa = aux;
	pcitag_t tag = pa->pa_tag;
	int iba_node = PCITAG_NODE(tag);
	extern int iic_print(void *, const char *);
	struct i2c_attach_args ia;
	char name[32];
	u_int32_t reg[2];
	int node;

	for (node = OF_child(iba_node); node; node = OF_peer(node)) {
		memset(name, 0, sizeof name);
		if (OF_getprop(node, "compatible", &name,
		    sizeof name) && name[0])
			ia.ia_name = name;

		if (strcmp(name, "i2c-smbus") == 0) {
			for (node = OF_child(node); node; node = OF_peer(node)) {
				memset(&ia, 0, sizeof ia);
				ia.ia_tag = iba->iba_tag;
				memset(name, 0, sizeof name);
				if (OF_getprop(node, "compatible", &name,
				    sizeof name) && name[0]) {
					ia.ia_name = name;
					if (strncmp(ia.ia_name, "i2c-",
					    strlen("i2c-")) == 0)
						ia.ia_name += strlen("i2c-");
				}
				if (OF_getprop(node, "reg", &reg,
				    sizeof reg) != sizeof reg)
					continue;
				ia.ia_addr = (reg[1] >> 1);
				if (ia.ia_name)
					config_found(self, &ia, iic_print);
			}
			return;
		}
	}
}
