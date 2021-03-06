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
 *
 *	$OpenBSD: SYS.h,v 1.24 2015/10/23 04:39:24 guenther Exp $
 */

#include <machine/asm.h>
#include <sys/syscall.h>


/*
 * We define a hidden alias with the prefix "_libc_" for each global symbol
 * that may be used internally.  By referencing _libc_x instead of x, other
 * parts of libc prevent overriding by the application and avoid unnecessary
 * relocations.
 */
#define _HIDDEN(x)		_libc_##x
#define _HIDDEN_ALIAS(x,y)			\
	STRONG_ALIAS(_HIDDEN(x),y);		\
	.hidden _HIDDEN(x)
#define _HIDDEN_FALIAS(x,y)			\
	_HIDDEN_ALIAS(x,y);			\
	.type _HIDDEN(x),@function

/*
 * For functions implemented in ASM that aren't syscalls.
 *   END_STRONG(x)	Like DEF_STRONG() in C; for standard/reserved C names
 *   END_WEAK(x)	Like DEF_WEAK() in C; for non-ISO C names
 */
#define	END_STRONG(x)	END(x); _HIDDEN_FALIAS(x,x); END(_HIDDEN(x))
#define	END_WEAK(x)	END_STRONG(x); .weak x


/*
 * Design note:
 *
 * System calls entry points are really named _thread_sys_{syscall},
 * and weakly aliased to the name {syscall}. This allows the thread
 * library to replace system calls at link time.
 */

/* Use both _thread_sys_{syscall} and [weak] {syscall}. */

#define	SYSENTRY(x)					\
			ENTRY(_thread_sys_ ## x);	\
			.weak _C_LABEL(x);		\
			_C_LABEL(x) = _C_LABEL(_thread_sys_ ## x)
#define	SYSENTRY_HIDDEN(x)				\
			ENTRY(_thread_sys_ ## x)
#define	__END_HIDDEN(x)	END(_thread_sys_ ## x);			\
			_HIDDEN_FALIAS(x,_thread_sys_ ## x);	\
			END(_HIDDEN(x))
#define	__END(x)	__END_HIDDEN(x); END(x)

#define	__DO_SYSCALL(x)					\
			movl $(SYS_ ## x),%eax;		\
			int $0x80

#define CERROR		_C_LABEL(__cerror)
#define _CERROR		_C_LABEL(___cerror)

/* perform a syscall */
#define	_SYSCALL_NOERROR(x,y)				\
		SYSENTRY(x);				\
			__DO_SYSCALL(y);
#define	_SYSCALL_HIDDEN_NOERROR(x,y)			\
		SYSENTRY_HIDDEN(x);			\
			__DO_SYSCALL(y);

#define	SYSCALL_NOERROR(x)				\
		_SYSCALL_NOERROR(x,x)

/* perform a syscall, set errno */
#ifdef __PIC__
#define	_SYSCALL(x,y)					\
			.text;				\
			.align 2;			\
		2:	PIC_PROLOGUE;			\
			movl PIC_GOT(CERROR), %ecx;	\
			PIC_EPILOGUE;			\
			jmp *%ecx;			\
		_SYSCALL_NOERROR(x,y)			\
			jc 2b
#define	_SYSCALL_HIDDEN(x,y)				\
			.text;				\
			.align 2;			\
		2:	PIC_PROLOGUE;			\
			movl PIC_GOT(CERROR), %ecx;	\
			PIC_EPILOGUE;			\
			jmp *%ecx;			\
		_SYSCALL_HIDDEN_NOERROR(x,y)		\
			jc 2b
#else
#define	_SYSCALL(x,y)					\
			.text;				\
			.align 2;			\
		2:					\
			jmp PIC_PLT(CERROR);		\
		_SYSCALL_NOERROR(x,y)			\
			jc 2b
#define	_SYSCALL_HIDDEN(x,y)				\
			.text;				\
			.align 2;			\
		2:					\
			jmp PIC_PLT(CERROR);		\
		_SYSCALL_HIDDEN_NOERROR(x,y)		\
			jc 2b
#endif

#define	SYSCALL(x)					\
		_SYSCALL(x,x)

/* perform a syscall, return */
#define	PSEUDO_NOERROR(x,y)				\
		_SYSCALL_NOERROR(x,y);			\
			ret;				\
		__END(x)

/* perform a syscall, set errno, return */
#define	PSEUDO(x,y)					\
		_SYSCALL(x,y);				\
			ret;				\
		__END(x)
#define	PSEUDO_HIDDEN(x,y)				\
		_SYSCALL_HIDDEN(x,y);			\
			ret;				\
		__END_HIDDEN(x)

/* perform a syscall with the same name, set errno, return */
#define	RSYSCALL(x)					\
			PSEUDO(x,x);
#define	RSYSCALL_HIDDEN(x)				\
			PSEUDO_HIDDEN(x,x)
#define	SYSCALL_END(x)	__END(x)
#define	SYSCALL_END_HIDDEN(x)				\
			__END_HIDDEN(x)

	.globl	CERROR
