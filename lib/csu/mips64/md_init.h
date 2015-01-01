/* $OpenBSD: md_init.h,v 1.9 2015/01/01 19:43:10 miod Exp $ */

/*-
 * Copyright (c) 2001 Ross Harvey
 * Copyright (c) 2001 Simon Burge
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the NetBSD
 *      Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Allocate 32 bytes for the stack frame.  Store GP at SP+16 (since
 * this is where code generated by the compiler for fallthru processing
 * expects it to be), and the RA at SP+24.
 */

/* this is gross... */

#ifdef __ABICALLS__
#define MD_FUNCTION_PROLOGUE(entry_pt)			\
	"	.frame $sp,32,$31		\n"	\
	"	.set	reorder			\n"	\
	"	dsubu    $sp,$sp,32		\n"	\
	"	.cpsetup $25, 16, "#entry_pt"	\n"	\
	"	sd      $ra ,24($sp)		\n"
#else
#define MD_FUNCTION_PROLOGUE(entry_pt)			\
	"       dsubu	$sp,$sp,32		\n"	\
	"       sd	$ra ,24($sp)		\n"
#endif


#define MD_SECTION_PROLOGUE(sect, entry_pt)		\
	__asm (						\
	".section "#sect",\"ax\",@progbits	\n"	\
	".align 2				\n"	\
	".globl "#entry_pt"			\n"	\
	".type "#entry_pt"@function		\n"	\
	".ent "#entry_pt"			\n"	\
	#entry_pt":				\n"	\
	MD_FUNCTION_PROLOGUE(entry_pt)			\
	"       /* fall thru */			\n"	\
	".previous")

#define MD_SECTION_EPILOGUE(sect)                       \
	__asm (						\
	".section "#sect",\"ax\",@progbits      \n"	\
	"	ld	$ra ,24($sp)		\n"	\
	" # next should have been a .cpreturn 	\n"	\
	"	ld	$gp,16($sp)		\n"	\
	"	.set	noreorder		\n"	\
	"	j	$ra			\n"	\
	"	daddu	$sp,$sp,32		\n"	\
	"	.set	reorder			\n"	\
	".previous")

#define MD_SECT_CALL_FUNC(section, func)		\
	__asm (".section "#section", \"ax\"	\n"	\
	"	jal	" #func "		\n"	\
	".previous")

#define MD_CRT0_START				\
	__asm(					\
	".text					\n" \
	"	.align	3			\n" \
	"	.globl	__start			\n" \
	"	.ent	__start			\n" \
	"	.type	__start, @function	\n" \
	"__start:				\n" \
	"	lui	$gp, %hi(%neg(%gp_rel(__start)))	\n" \
	"	addiu	$gp, $gp, %lo(%neg(%gp_rel(__start)))	\n" \
	"	daddu	$gp, $gp, $t9		\n" \
	"	move	$a0, $sp		\n" \
	"	dsrl	$a1, $sp, 4		\n" /* align stack on a */ \
	"	dsll	$sp, $a1, 4		\n" /* 16 byte boundary */ \
	"	move	$a1, $v0		\n" \
	"	dla	$t9, ___start		\n" \
	"	jr	$t9			\n" \
	"	.end	__start			\n" \
	"	.previous")

#define MD_RCRT0_START				\
	__asm(					\
	".text					\n" \
	"	.align	3			\n" \
	"	.globl	__start			\n" \
	"	.ent	__start			\n" \
	"	.type	__start, @function	\n" \
	"__start:				\n" \
	"	dsubu	$sp, $sp, 160		\n" \
	"	lui	$gp, %hi(%neg(%gp_rel(__start)))	\n" \
	"	addiu	$gp, $gp, %lo(%neg(%gp_rel(__start)))	\n" \
	"	daddu	$gp, $gp, $t9		\n" \
	"	dla	$s1, 1f			\n" \
	"	bgezal	$zero, 1f		\n" \
	"1:					\n" \
	"	dsubu	$s0, $ra, $s1		\n" \
	"	daddu	$a0, $sp, 160		\n" \
	"	daddu	$a1, $sp, 0		\n" \
	"	dla	$t9, _dl_boot_bind	\n" \
	"	daddu	$t9, $s0		\n" \
	"	jalr	$t9			\n" \
	"	daddu	$sp, $sp, 160		\n" \
	"	move	$a0, $sp		\n" \
	"	dsrl	$a1, $sp, 4		\n" /* align stack on a */ \
	"	dsll	$sp, $a1, 4		\n" /* 16 byte boundary */ \
	"	move	$a1, $zero		\n" \
	"	dla	$t9, ___start		\n" \
	"	jr	$t9			\n" \
	"	.end	__start			\n" \
	"	.globl	_dl_exit		\n" \
	"	.ent	_dl_exit		\n" \
	"	.type	_dl_exit, @function	\n" \
	"_dl_exit:				\n" \
	"	li	$v0, 1			\n" \
	"	syscall				\n" \
	"	j	$ra			\n" \
	"	.end	_dl_exit		\n" \
	"	.globl	_dl_printf		\n" \
	"	.ent	_dl_printf		\n" \
	"	.type	_dl_printf, @function	\n" \
	"_dl_printf:				\n" \
	"	j	$ra			\n" \
	"	.end	_dl_printf		\n" \
	"	.previous")

struct kframe {
	long	kargc;
	char	*kargv[1];	/* size depends on kargc */
	char	kargstr[1];	/* size varies */
	char	kenvstr[1];	/* size varies */
};

#define	MD_START_ARGS		struct kframe *kfp, void (*cleanup)(void)
#define	MD_START_SETUP				\
	char	**argv, **envp;			\
	int	argc;				\
						\
	argc = kfp->kargc;			\
	argv = &kfp->kargv[0];			\
	environ = envp = argv + argc + 1;
