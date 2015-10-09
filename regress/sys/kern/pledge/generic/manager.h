/*	$OpenBSD: manager.h,v 1.1 2015/10/09 06:44:13 semarie Exp $ */
/*
 * Copyright (c) 2015 Sebastien Marie <semarie@openbsd.org>
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
#ifndef _MANAGER_H_
#define _MANAGER_H_

void _start_test(int *ret, const char *test_name, const char *request,
    const char *paths[], void (*test_func)(void));

#define start_test(ret,req,paths,func) \
    _start_test(ret,#func,req,paths,func)

#define start_test1(ret,req,path,func) \
    do { \
	    const char *_paths[] = {path, NULL}; \
	    start_test(ret,req,_paths,func); \
    } while (0)

#endif /* _MANAGER_H_ */
