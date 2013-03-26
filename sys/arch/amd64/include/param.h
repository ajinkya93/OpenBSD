/*	$OpenBSD: param.h,v 1.22 2013/03/26 05:04:10 deraadt Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 */

#ifndef	_MACHINE_PARAM_H_
#define	_MACHINE_PARAM_H_

#ifdef _KERNEL
#ifdef _LOCORE
#include <machine/psl.h>
#else
#include <machine/cpu.h>
#endif
#endif

#define	_MACHINE	amd64
#define	MACHINE		"amd64"
#define	_MACHINE_ARCH	amd64
#define	MACHINE_ARCH	"amd64"
#define	MID_MACHINE	MID_AMD64

#define	PAGE_SHIFT	12
#define	PAGE_SIZE	(1 << PAGE_SHIFT)
#define	PAGE_MASK	(PAGE_SIZE - 1)

#define	KERNBASE	0xffffffff80000000	/* start of kernel virtual space */

#ifdef _KERNEL

#define	KERNTEXTOFF	(KERNBASE+0x1000000)	/* start of kernel text */
#define	KERNTEXTOFF_HI	0xffffffff
#define	KERNTEXTOFF_LO	0x81000000

#define	KERNBASE_HI	0xffffffff
#define	KERNBASE_LO	0x80000000

#define	NBPG		PAGE_SIZE		/* bytes/page */
#define	PGSHIFT		PAGE_SHIFT		/* LOG2(PAGE_SIZE) */
#define	PGOFSET		PAGE_MASK		/* byte offset into page */

#define	UPAGES		5			/* pages of u-area */
#define	USPACE		(UPAGES * PAGE_SIZE)	/* total size of u-area */
#define	USPACE_ALIGN	0			/* u-area alignment 0-none */

#define	NMBCLUSTERS	6144			/* map size, max cluster allocation */

#ifndef	MSGBUFSIZE
#define	MSGBUFSIZE	(16 * PAGE_SIZE)	/* default message buffer size */
#endif

/*
 * Maximum size of the kernel malloc arena in PAGE_SIZE-sized
 * logical pages.
 */
#define	NKMEMPAGES_MAX_DEFAULT	((128 * 1024 * 1024) >> PAGE_SHIFT)

#endif /* _KERNEL */

#endif /* _MACHINE_PARAM_H_ */
