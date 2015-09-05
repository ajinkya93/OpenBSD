/*	$OpenBSD: db.h,v 1.1 2015/09/05 11:28:35 guenther Exp $	*/
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

#ifndef _LIBC_DB_H_
#define	_LIBC_DB_H_

#include_next <db.h>
#include "namespace.h"

__BEGIN_HIDDEN_DECLS
DB	*__bt_open(const char *, int, int, const BTREEINFO *, int);
DB	*__hash_open(const char *, int, int, const HASHINFO *, int);
DB	*__rec_open(const char *, int, int, const RECNOINFO *, int);
void	__dbpanic(DB *dbp);
__END_HIDDEN_DECLS

PROTO_NORMAL(dbopen);

#endif /* !_LIBC_DB_H_ */
