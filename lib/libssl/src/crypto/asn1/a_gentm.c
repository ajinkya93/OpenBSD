/* $OpenBSD: a_gentm.c,v 1.25 2015/10/02 15:04:45 beck Exp $ */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *	 SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

/* GENERALIZEDTIME implementation, written by Steve Henson. Based on UTCTIME */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <openssl/asn1.h>
#include <openssl/err.h>

#include "o_time.h"
#include "asn1_locl.h"

int
ASN1_GENERALIZEDTIME_check(ASN1_GENERALIZEDTIME *d)
{
	if (d->type != V_ASN1_GENERALIZEDTIME)
		return (0);
	return (d->type == asn1_time_parse(d->data, d->length, NULL, d->type));
}

int
ASN1_GENERALIZEDTIME_set_string(ASN1_GENERALIZEDTIME *s, const char *str)
{
	ASN1_GENERALIZEDTIME t;

	t.type = V_ASN1_GENERALIZEDTIME;
	t.length = strlen(str);
	t.data = (unsigned char *)str;
	if (ASN1_GENERALIZEDTIME_check(&t)) {
		if (s != NULL) {
			if (!ASN1_STRING_set((ASN1_STRING *)s,
			    (unsigned char *)str, t.length))
				return 0;
			s->type = V_ASN1_GENERALIZEDTIME;
		}
		return (1);
	} else
		return (0);
}

ASN1_GENERALIZEDTIME *
ASN1_GENERALIZEDTIME_set(ASN1_GENERALIZEDTIME *s, time_t t)
{
	return ASN1_GENERALIZEDTIME_adj(s, t, 0, 0);
}

static ASN1_GENERALIZEDTIME *
ASN1_GENERALIZEDTIME_adj_internal(ASN1_GENERALIZEDTIME *s, time_t t,
    int offset_day, long offset_sec)
{
	char *p;
	struct tm *tm;
	struct tm data;

	tm = gmtime_r(&t, &data);
	if (tm == NULL)
		return (NULL);

	if (offset_day || offset_sec) {
		if (!OPENSSL_gmtime_adj(tm, offset_day, offset_sec))
			return NULL;
	}

	if ((p = gentime_string_from_tm(tm)) == NULL) {
		ASN1err(ASN1_F_ASN1_GENERALIZEDTIME_ADJ, ERR_R_MALLOC_FAILURE);
		return (NULL);
	}
	free(s->data);
	s->data = p;
	s->length = strlen(p);

	s->type = V_ASN1_GENERALIZEDTIME;
	return (s);
}

ASN1_GENERALIZEDTIME *
ASN1_GENERALIZEDTIME_adj(ASN1_GENERALIZEDTIME *s, time_t t, int offset_day,
    long offset_sec)
{
	ASN1_GENERALIZEDTIME *tmp = NULL, *ret;

	if (s == NULL) {
		tmp = ASN1_GENERALIZEDTIME_new();
		if (tmp == NULL)
			return NULL;
		s = tmp;
	}

	ret = ASN1_GENERALIZEDTIME_adj_internal(s, t, offset_day, offset_sec);
	if (ret == NULL && tmp != NULL)
		ASN1_GENERALIZEDTIME_free(tmp);

	return ret;

}
