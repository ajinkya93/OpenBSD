/* $OpenBSD: poly1305.h,v 1.3 2014/03/26 04:55:35 djm Exp $ */

/* 
 * Public Domain poly1305 from Andrew Moon
 * poly1305-donna-unrolled.c from https://github.com/floodyberry/poly1305-donna
 */

#ifndef POLY1305_H
#define POLY1305_H

#include <sys/types.h>

#define POLY1305_KEYLEN		32
#define POLY1305_TAGLEN		16

void poly1305_auth(u_char out[POLY1305_TAGLEN], const u_char *m, size_t inlen,
    const u_char key[POLY1305_KEYLEN])
    __bounded((__minbytes__, 1, POLY1305_TAGLEN))
    __bounded((__buffer__, 2, 3))
    __bounded((__minbytes__, 4, POLY1305_KEYLEN));

#endif	/* POLY1305_H */
