/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 *     from: @(#)encrypt.h     8.1 (Berkeley) 6/4/93
 *     $OpenBSD: encrypt.h,v 1.2 2003/06/02 19:38:25 millert Exp $
 *     $NetBSD: encrypt.h,v 1.4 1996/02/24 01:15:20 jtk Exp $
 */

/*
 * This source code is no longer held under any constraint of USA
 * `cryptographic laws' since it was exported legally.  The cryptographic
 * functions were removed from the code and a "Bones" distribution was
 * made.  A Commodity Jurisdiction Request #012-94 was filed with the
 * USA State Department, who handed it to the Commerce department.  The
 * code was determined to fall under General License GTDA under ECCN 5D96G,
 * and hence exportable.  The cryptographic interfaces were re-added by Eric
 * Young, and then KTH proceeded to maintain the code in the free world.
 */

/*
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 *
 * Export of this software from the United States of America is assumed
 * to require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

/* $KTH: encrypt.h,v 1.4 1997/01/24 23:10:56 assar Exp $ */

#ifndef	__ENCRYPT__
#define	__ENCRYPT__

#define	DIR_DECRYPT		1
#define	DIR_ENCRYPT		2

#define	VALIDKEY(key)	( key[0] | key[1] | key[2] | key[3] | \
			  key[4] | key[5] | key[6] | key[7])

#define	SAMEKEY(k1, k2)	(!memcmp(k1, k2, sizeof(des_cblock)))

typedef	struct {
	short		type;
	int		length;
	unsigned char	*data;
} Session_Key;

typedef struct {
	char	*name;
	int	type;
	void	(*output) (unsigned char *, int);
	int	(*input) (int);
	void	(*init) (int);
	int	(*start) (int, int);
	int	(*is) (unsigned char *, int);
	int	(*reply) (unsigned char *, int);
	void	(*session) (Session_Key *, int);
	int	(*keyid) (int, unsigned char *, int *);
	void	(*printsub) (unsigned char *, int, unsigned char *, int);
} Encryptions;

#define	SK_DES		1	/* Matched Kerberos v5 KEYTYPE_DES */

#include "enc-proto.h"

extern int encrypt_debug_mode;
extern int (*decrypt_input) (int);
extern void (*encrypt_output) (unsigned char *, int);
#endif
