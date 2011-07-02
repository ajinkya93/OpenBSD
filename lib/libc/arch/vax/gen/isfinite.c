/*	$OpenBSD: isfinite.c,v 1.4 2011/07/02 19:27:34 martynas Exp $	*/
/*
 * Copyright (c) Martynas Venckus <martynas@openbsd.org>
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

/* LINTLIBRARY */

#include <sys/cdefs.h>
#include <math.h>

/* ARGSUSED */
int
__isfinite(double d)
{
	return(1);
}

/* ARGSUSED */
int
__isfinitef(float f)
{
	return(1);
}

#ifdef	lint
/* PROTOLIB1 */
int __isfinitel(long double);
#else	/* lint */
__weak_alias(__isfinitel, __isfinite);
#endif	/* lint */

/*
 * 3BSD compatibility aliases.
 */
__weak_alias(finite, __isfinite);
__weak_alias(finitef, __isfinitef);
