/*	$OpenBSD: md5.h,v 1.1 2015/09/11 09:18:27 guenther Exp $	*/
/*
 * Copyright (c) 2015 Philip Guenther <guenther@openbsd.org>
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

#ifndef _LIBC_MD5_H_
#define _LIBC_MD5_H_

#include_next <md5.h>
#include "namespace.h"

PROTO_NORMAL(MD5Data);
PROTO_NORMAL(MD5End);
PROTO_NORMAL(MD5File);
PROTO_NORMAL(MD5FileChunk);
PROTO_NORMAL(MD5Final);
PROTO_NORMAL(MD5Init);
PROTO_NORMAL(MD5Pad);
PROTO_NORMAL(MD5Transform);
PROTO_NORMAL(MD5Update);

#endif /* _LIBC_MD5_H_ */
