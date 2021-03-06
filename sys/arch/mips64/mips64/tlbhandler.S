/*	$OpenBSD: tlbhandler.S,v 1.44 2014/08/12 19:33:59 miod Exp $ */

/*
 * Copyright (c) 1995-2004 Opsycon AB  (www.opsycon.se / www.opsycon.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 *  This code handles TLB exceptions and updates.
 */

#include <machine/param.h>
#include <machine/pte.h>
#include <machine/asm.h>
#include <machine/cpu.h>
#include <mips64/mips_cpu.h>
#include <machine/regnum.h>
#include <machine/cpustate.h>
#ifdef CPU_LOONGSON2
#include <machine/loongson2.h>
#endif

#include "assym.h"

#ifdef MIPS_PTE64
#define	PTE_LOG		3
#define	PTE_LOAD	ld
#define	PTE_OFFS	8
#else
#define	PTE_LOG		2
#define	PTE_LOAD	lw
#define	PTE_OFFS	4
#endif

	.set	mips3

	.set	noreorder	/* Default reorder mode */

/*---------------------------------------------------------------- tlb_miss
 *	Low level TLB exception handler. TLB and XTLB code are
 *	identical at the moment.
 *	This code is not copied to the exception area, instead
 *	trampolines branching to the appropriate code are built.
 */
	.set	noat

#ifdef CPU_R4000
	.globl	xtlb_miss_err_r4k
	.ent	xtlb_miss_err_r4k, 0
xtlb_miss_err_r4k:
	/*
	 * R4000 errata #27: TLB miss exception may be invoked with BadVAddr
	 * being incorrect.
	 */
	tlbp
	mfc0	k1, COP_0_TLB_INDEX
	bltz	k1, xtlb_miss			# missing!
	 nop
	ERET
	 nop
	.end	xtlb_miss_err_r4k

	.globl	xtlb_miss_err_r4000SC
	.ent	xtlb_miss_err_r4000SC, 0
xtlb_miss_err_r4000SC:
	/*
	 * R4000SC errata #12: In addition to errata #27 above, the tlbp
	 * instruction needs to be issued in uncached space, to prevent
	 * BadVAddr to be further corrupted if a VCE exception occurs
	 * immediately after the TLB miss exception has been serviced.
	 */
	tlbp
	mfc0	k1, COP_0_TLB_INDEX
	bltz	k1, 1f		# missing!
	 nop
	ERET
	 nop
1:
	LA	k1, xtlb_miss
	jr	k1
	 nop
	.end	xtlb_miss_err_r4000SC
#endif	/* CPU_R4000 */
#if defined(CPU_R5000) || defined(CPU_RM7000)
	.globl	xtlb_miss_err_r5k
	.ent	xtlb_miss_err_r5k, 0
xtlb_miss_err_r5k:
	/*
	 * R5000 errata: edge cases can trigger a TLB miss exception
	 * instead of an invalid TLB exception. Servicing the TLB miss
	 * exception would cause a duplicate TLB to be inserted, which
	 * causes the processor operation to become unpredictable (but
	 * very bad for the kernel's health, ten times out of nine).
	 *
	 * More details about the problem can be found in:
	 *  http://www.linux-mips.org/archives/linux-mips/2000-02/msg00040.html
	 *
	 * We work around the issue by checking for an existing TLB entry,
	 * and handling this as an invalid TLB exception (as it was intended
	 * to be), in this case.
	 */
	tlbp
	mfc0	k1, COP_0_TLB_INDEX
	bltz	k1, xtlb_miss		# missing!
	 nop
	b	k_tlb_inv
	 nop
	.end	xtlb_miss_err_r5k
#endif	/* CPU_R5000 || CPU_RM7000 */

	.globl	xtlb_miss
	.ent	xtlb_miss, 0
xtlb_miss:
	dmfc0	k0, COP_0_BAD_VADDR
	bltz	k0, _k_miss		# kernel address space
	PTR_SRL	k0, k0, SEGSHIFT
	sltiu	k1, k0, PMAP_SEGTABSIZE
	beqz	k1, _inv_seg		# wrong if outside pm_segtab
	nop
	GET_CPU_INFO(k1, k0)
	PTR_L	k1, CI_CURPROCPADDR(k1)
	dmfc0	k0, COP_0_BAD_VADDR
	PTR_SRL	k0, k0, SEGSHIFT
	PTR_SLL	k0, k0, LOGREGSZ
	PTR_L	k1, PCB_SEGTAB(k1)
	PTR_ADDU k1, k1, k0
	PTR_L	k1, 0(k1)		# get pointer to page table
	dmfc0	k0, COP_0_BAD_VADDR
	PTR_SRL	k0, k0, PGSHIFT - PTE_LOG
	beqz	k1, _inv_seg
	andi	k0, k0, ((NPTEPG >> 1) - 1) << (PTE_LOG + 1)
	PTR_ADDU k1, k1, k0
	PTE_LOAD	k0, 0(k1)
	PTE_LOAD	k1, PTE_OFFS(k1)
#ifdef CPU_R4000
	dsll	k0, k0, (64 - PG_FRAMEBITS - 1)	# clear bits left of PG_FRAME...
	dsll	k1, k1, (64 - PG_FRAMEBITS - 1)	# ...but PG_SP
	bltz	k0, _inv_seg			# defer to trap() if PG_SP set.
	bltz	k1, _inv_seg
	dsll	k0, k0, 1
	dsll	k1, k1, 1
#else
	dsll	k0, k0, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	dsll	k1, k1, (64 - PG_FRAMEBITS)
#endif
	dsrl	k0, k0, (64 - PG_FRAMEBITS)
	dsrl	k1, k1, (64 - PG_FRAMEBITS)
	dmtc0	k0, COP_0_TLB_LO0
	dmtc0	k1, COP_0_TLB_LO1
	TLB_HAZARD
	tlbwr				# update TLB
	TLB_HAZARD

#ifdef CPU_LOONGSON2
	li	k0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	k0, COP_0_DIAG
#endif

	ERET

_k_miss:
	dmfc0	k0, COP_0_BAD_VADDR	# must reload.
	b	k_tlb_miss		# kernel tlbmiss.
	nop

_inv_seg:				# No page table for this segment,
					# or processing is too complex to be
					# done here: invoke C code.
	mfc0	k0, COP_0_STATUS_REG
	andi	k0, SR_KSU_USER
	bne	k0, zero, go_u_general
	nop

go_k_general:
	j	k_general
	nop
go_u_general:
#ifdef CPU_LOONGSON2
	/*
	 * To work around a branch prediction issue on earlier LS2F
	 * chips, it is necessary to clear the BTB upon
	 * userland->kernel boundaries.
	 */
	li	k0, COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	k0, COP_0_DIAG
#endif
	j	u_general
	nop
	.end	xtlb_miss

	.set	at

/*---------------------------------------------------------------- k_tlb_inv
 *	Handle a TLB invalid exception from kernel mode in kernel
 *	space. This happens when we have a TLB match but an invalid
 *	entry. Try to reload.
 */
NLEAF(k_tlb_inv, 0)
	.set	noat
	LA	k1, (VM_MIN_KERNEL_ADDRESS)	# compute index
	dmfc0	k0, COP_0_BAD_VADDR		# get the fault address
	PTR_SUBU k0, k0, k1
	lw	k1, Sysmapsize			# index within range?
	PTR_SRL	k0, k0, PGSHIFT
	sltu	k1, k0, k1
	beq	k1, zero, sys_stk_chk		# No. check for valid stack
	nop

	PTR_L	k1, Sysmap
	PTR_SLL	k0, k0, PTE_LOG			# compute offset from index
	TLB_HAZARD
	tlbp					# Probe the invalid entry
	TLB_HAZARD				# necessary?
	PTR_ADDU k1, k1, k0
	and	k0, k0, PTE_OFFS		# check even/odd page
	bne	k0, zero, k_tlb_inv_odd
	nop

	mfc0	k0, COP_0_TLB_INDEX
	bltz	k0, sys_stk_chk			# probe fail
	PTE_LOAD k0, 0(k1)			# get PTE entry

	dsll	k0, k0, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	dsrl	k0, k0, (64 - PG_FRAMEBITS)
	dmtc0	k0, COP_0_TLB_LO0		# load PTE entry
	and	k0, k0, PG_V			# check for valid entry
	beq	k0, zero, go_k_general		# PTE invalid
	PTE_LOAD k0, PTE_OFFS(k1)		# get odd PTE entry
	dsll	k0, k0, (64 - PG_FRAMEBITS)
	dsrl	k0, k0, (64 - PG_FRAMEBITS)
	dmtc0	k0, COP_0_TLB_LO1		# load PTE entry
	TLB_HAZARD
	tlbwi					# write TLB
	TLB_HAZARD

#ifdef CPU_LOONGSON2
	li	k0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	k0, COP_0_DIAG
#endif

	ERET

k_tlb_inv_odd:
	mfc0	k0, COP_0_TLB_INDEX
	bltz	k0, sys_stk_chk			# probe fail
	PTE_LOAD k0, 0(k1)			# get PTE entry

	dsll	k0, k0, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	dsrl	k0, k0, (64 - PG_FRAMEBITS)
	dmtc0	k0, COP_0_TLB_LO1		# save PTE entry
	and	k0, k0, PG_V			# check for valid entry
	beq	k0, zero, go_k_general		# PTE invalid
	PTE_LOAD k0, -PTE_OFFS(k1)		# get even PTE entry
	dsll	k0, k0, (64 - PG_FRAMEBITS)
	dsrl	k0, k0, (64 - PG_FRAMEBITS)
	dmtc0	k0, COP_0_TLB_LO0		# save PTE entry
	TLB_HAZARD
	tlbwi					# update TLB
	TLB_HAZARD

#ifdef CPU_LOONGSON2
	li	k0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	k0, COP_0_DIAG
#endif

	ERET
END(k_tlb_inv)

/*---------------------------------------------------------------- k_tlb_miss
 *
 *	Handle a TLB miss exception from kernel mode in kernel space.
 *	We must check that this is coming from kernel mode. If not
 *	it's a bad address from user mode so handle properly.
 *	Load up the correct entry contents from the kernel map.
 *	k0 has bad address.
 */
NLEAF(k_tlb_miss, 0)
	.set	noat
	mfc0	k1, COP_0_STATUS_REG
	andi	k1, SR_KSU_USER
	bne	k1, zero, go_u_general
	LA	k1, (VM_MIN_KERNEL_ADDRESS)	# compute index
						# (safe if expands to > 1 insn)
	PTR_SUBU k0, k0, k1
	lw	k1, Sysmapsize			# index within range?
	PTR_SRL	k0, k0, PGSHIFT
	sltu	k1, k0, k1
	beq	k1, zero, sys_stk_chk		# No. check for valid stack
	PTR_SRL	k0, k0, 1
	PTR_L	k1, Sysmap
	PTR_SLL	k0, k0, (PTE_LOG + 1)		# compute offset from index
	PTR_ADDU k1, k1, k0
	PTE_LOAD k0, 0(k1)			# get PTE entry
	PTE_LOAD k1, PTE_OFFS(k1)		# get odd PTE entry
	dsll	k0, k0, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	dsrl	k0, k0, (64 - PG_FRAMEBITS)
	dmtc0	k0, COP_0_TLB_LO0		# load PTE entry
	dsll	k1, k1, (64 - PG_FRAMEBITS)
	dsrl	k1, k1, (64 - PG_FRAMEBITS)
	dmtc0	k1, COP_0_TLB_LO1		# load PTE entry
	TLB_HAZARD
	tlbwr					# write TLB
	TLB_HAZARD

#ifdef CPU_LOONGSON2
	li	k0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	k0, COP_0_DIAG
#endif

	ERET

sys_stk_chk:
	GET_CPU_INFO(k1, k0)
	PTR_L	k1, CI_CURPROCPADDR(k1)
	PTR_SUBU k0, sp, k1			# check to see if we have a
	sltiu	k0, 2048			#  valid kernel stack
	beqz	k0, go_k_general		# yes, handle.
	nop

	LA	a0, start-FRAMESZ(CF_SZ)-4*REGSZ # set sp to a valid place
#ifdef __mips_n64
	mfc0	a4, COP_0_STATUS_REG
	mfc0	a5, COP_0_CAUSE_REG
	move	a6, sp
#else
	mfc0	a2, COP_0_STATUS_REG
	mfc0	a3, COP_0_CAUSE_REG
	REG_S	a2, CF_ARGSZ+0*REGSZ(sp)
	REG_S	a3, CF_ARGSZ+1*REGSZ(sp)
	PTR_S	sp, CF_ARGSZ+2*REGSZ(a0)
#endif
	move	sp, a0
	dmfc0	a3, COP_0_BAD_VADDR
	move	a2, ra
	dmfc0	a1, COP_0_EXC_PC
	LA	a0, 1f
	jal	printf
	nop

	LA	sp, start-FRAMESZ(CF_SZ)	# set sp to a valid place

#ifdef DDB
	LA	a1, db_printf
	LA	a0, 2f
	jal	trapDump
	nop
#endif

	PANIC("kernel stack overflow")
	/*noreturn*/

	.data
1:
	.asciiz	"\rktlbmiss: PC %p RA %p ADR %p\nSR %p CR %p SP %p\n"
2:
	.asciiz	"stack ovf"
	.text

	.set	at
END(k_tlb_miss)

/*---------------------------------------------------------------- tlb_flush
 *	Flush the "random" entries from the TLB.
 *	Uses "wired" register to determine what register to start with.
 *	Arg "tlbsize" is the number of entries to flush.
 */
LEAF(tlb_flush, 0)
	mfc0	v1, COP_0_STATUS_REG	# Save the status register.
	ori	v0, v1, SR_INT_ENAB
	xori	v0, v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG	# Disable interrupts
	MTC0_SR_IE_HAZARD
	mfc0	ta1, COP_0_TLB_WIRED
	LA	v0, CKSEG0_BASE		# invalid address
	dmfc0	ta0, COP_0_TLB_HI	# Save the PID

#ifdef CPU_OCTEON
	PTR_SLL	ta2, ta1, PAGE_SHIFT + 1
	PTR_ADDU v0, v0, ta2
#else
	dmtc0	v0, COP_0_TLB_HI	# Mark entry high as invalid
#endif
	dmtc0	zero, COP_0_TLB_LO0	# Zero out low entry0.
	dmtc0	zero, COP_0_TLB_LO1	# Zero out low entry1.
	mtc0	zero, COP_0_TLB_PG_MASK	# Zero out mask entry.
/*
 * Align the starting value (ta1) and the upper bound (a0).
 */
1:
	mtc0	ta1, COP_0_TLB_INDEX	# Set the index register.
#ifdef CPU_OCTEON
	dmtc0	v0, COP_0_TLB_HI	# Mark entry high as invalid
	PTR_ADDU v0, v0, 2 * PAGE_SIZE
#endif
	addu	ta1, ta1, 1		# Increment index.
	TLB_HAZARD
	tlbwi				# Write the TLB entry.
	TLB_HAZARD
	bne	ta1, a0, 1b
	nop

#ifdef CPU_LOONGSON2
	li	v0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	v0, COP_0_DIAG
#endif

	dmtc0	ta0, COP_0_TLB_HI	# Restore the PID
	li	a0, TLB_PAGE_MASK
	mtc0	a0, COP_0_TLB_PG_MASK	# Restore default mask value.
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register
	MTC0_SR_IE_HAZARD
	j	ra
	nop
END(tlb_flush)

/*--------------------------------------------------------------- tlb_flush_addr
 *	Flush any TLB entries for the given address and TLB PID.
 */
LEAF(tlb_flush_addr, 0)
	mfc0	v1, COP_0_STATUS_REG	# Save the status register.
	ori	v0, v1, SR_INT_ENAB
	xori	v0, v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG	# Disable interrupts
	MTC0_SR_IE_HAZARD
	dli	v0, (PG_HVPN | PG_ASID_MASK)
	and	a0, a0, v0		# Make sure valid hi value.
	dmfc0	ta0, COP_0_TLB_HI	# Get current PID
	dmtc0	a0, COP_0_TLB_HI	# look for addr & PID
	TLB_HAZARD
	tlbp				# Probe for the entry.
	TLB_HAZARD			# necessary?
	LA	ta1, CKSEG0_BASE	# Load invalid entry.
	mfc0	v0, COP_0_TLB_INDEX	# See what we got
	bltz	v0, 1f			# index < 0 => !found
	nop
#ifdef CPU_OCTEON
	PTR_SLL	ta2, v0, PAGE_SHIFT + 1
	PTR_ADDU ta1, ta1, ta2
#endif
	dmtc0	ta1, COP_0_TLB_HI	# Mark entry high as invalid

	dmtc0	zero, COP_0_TLB_LO0	# Zero out low entry.
	dmtc0	zero, COP_0_TLB_LO1	# Zero out low entry.
	TLB_HAZARD
	tlbwi
	TLB_HAZARD

#ifdef CPU_LOONGSON2
	li	v0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	v0, COP_0_DIAG
#endif

1:
	dmtc0	ta0, COP_0_TLB_HI	# restore PID
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register
	MTC0_SR_IE_HAZARD
	j	ra
	nop
END(tlb_flush_addr)

/*---------------------------------------------------------------- tlb_update
 *	Update the TLB if highreg is found; otherwise, enter the data.
 */
LEAF(tlb_update, 0)
	mfc0	v1, COP_0_STATUS_REG	# Save the status register.
	ori	v0, v1, SR_INT_ENAB
	xori	v0, v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG	# Disable interrupts
	MTC0_SR_IE_HAZARD
	and	ta1, a0, PG_ODDPG	# ta1 = Even/Odd flag
	dli	v0, (PG_HVPN | PG_ASID_MASK)
	and	a0, a0, v0
	dmfc0	ta0, COP_0_TLB_HI	# Save current PID
	dmtc0	a0, COP_0_TLB_HI	# Init high reg
	and	a2, a1, PG_G		# Copy global bit
	li	a3, TLB_PAGE_MASK
	TLB_HAZARD
	tlbp				# Probe for the entry.
	TLB_HAZARD			# necessary?
	dsll	a1, a1, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	mfc0	v0, COP_0_TLB_INDEX	# See what we got
	bne	ta1, zero, 2f		# Decide even odd
	dsrl	a1, a1, (64 - PG_FRAMEBITS)
# EVEN
	bltz	v0, 1f			# index < 0 => !found
	nop

	TLB_HAZARD
	tlbr				# update, read entry first
	TLB_HAZARD			# necessary?
	dmtc0	a1, COP_0_TLB_LO0	# init low reg0.
	TLB_HAZARD
	tlbwi				# update slot found
	TLB_HAZARD
	b	4f
	li	v0, 1
1:
	mtc0	a3, COP_0_TLB_PG_MASK	# init mask.
	dmtc0	a0, COP_0_TLB_HI	# init high reg.
	dmtc0	a1, COP_0_TLB_LO0	# init low reg0.
	dmtc0	a2, COP_0_TLB_LO1	# init low reg1.
	TLB_HAZARD
	tlbwr				# enter into a random slot
	TLB_HAZARD
	b	4f
	li	v0, 0
# ODD
2:
	nop
	bltz	v0, 3f			# index < 0 => !found
	nop

	TLB_HAZARD
	tlbr				# read the entry first
	TLB_HAZARD			# necessary?
	dmtc0	a1, COP_0_TLB_LO1	# init low reg1.
	TLB_HAZARD
	tlbwi				# update slot found
	TLB_HAZARD
	b	4f
	li	v0, 1
3:
	mtc0	a3, COP_0_TLB_PG_MASK	# init mask.
	dmtc0	a0, COP_0_TLB_HI	# init high reg.
	dmtc0	a2, COP_0_TLB_LO0	# init low reg0.
	dmtc0	a1, COP_0_TLB_LO1	# init low reg1.
	TLB_HAZARD
	tlbwr				# enter into a random slot
	TLB_HAZARD
4:
#ifdef CPU_LOONGSON2
	li	v0, COP_0_DIAG_ITLB_CLEAR | COP_0_DIAG_BTB_CLEAR | COP_0_DIAG_RAS_DISABLE
	dmtc0	v0, COP_0_DIAG
#endif

	dmtc0	ta0, COP_0_TLB_HI	# restore PID
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register
	MTC0_SR_IE_HAZARD
	j	ra
	li	v0, 0
END(tlb_update)

/*
 * int64_t tlb_probe(vaddr_t a0);
 * Probe for a TLB entry covering the given address, and return its index
 * (< 0 if no match)
 */
LEAF(tlb_probe, 0)
	mfc0	v1, COP_0_STATUS_REG	# Save the status register.
	ori	v0, v1, SR_INT_ENAB
	xori	v0, v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG	# Disable interrupts
	MTC0_SR_IE_HAZARD
	dli	v0, (PG_HVPN | PG_ASID_MASK)
	and	a0, a0, v0
	dmfc0	ta0, COP_0_TLB_HI	# Save current PID
	dmtc0	a0, COP_0_TLB_HI	# Init high reg
	TLB_HAZARD
	tlbp				# Probe for the entry.
	TLB_HAZARD			# necessary?
	mfc0	v0, COP_0_TLB_INDEX	# Return index
	dmtc0	ta0, COP_0_TLB_HI	# restore PID
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register
	MTC0_SR_IE_HAZARD
	j	ra
	nop
END(tlb_probe)

/*
 * void tlb_update_indexed(vaddr_t a0, register_t a1, register_t a2, uint a3);
 * Update a TLB entry pair.
 */
LEAF(tlb_update_indexed, 0)
	mfc0	v1, COP_0_STATUS_REG	# Save the status register.
	ori	v0, v1, SR_INT_ENAB
	xori	v0, v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG	# Disable interrupts
	MTC0_SR_IE_HAZARD
	dmfc0	ta0, COP_0_TLB_HI	# Save current PID
	li	ta1, TLB_PAGE_MASK

	mtc0	a3, COP_0_TLB_INDEX
	mtc0	ta1, COP_0_TLB_PG_MASK	# init mask
	dmtc0	a0, COP_0_TLB_HI	# init high reg.

	dsll	a1, a1, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	dsrl	a1, a1, (64 - PG_FRAMEBITS)
	dmtc0	a1, COP_0_TLB_LO0	# init low reg0.
	dsll	a2, a2, (64 - PG_FRAMEBITS)	# clear bits left of PG_FRAME
	dsrl	a2, a2, (64 - PG_FRAMEBITS)
	dmtc0	a2, COP_0_TLB_LO1	# init low reg1.

	TLB_HAZARD
	tlbwi				# update slot
	TLB_HAZARD

	dmtc0	ta0, COP_0_TLB_HI	# restore PID
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register
	MTC0_SR_IE_HAZARD
	j	ra
	nop
END(tlb_update_indexed)

/*---------------------------------------------------------------- tlb_read
 *	Read the TLB entry.
 */
LEAF(tlb_read, 0)
	mfc0	v1, COP_0_STATUS_REG	# Save the status register.
	ori	v0, v1, SR_INT_ENAB
	xori	v0, v0, SR_INT_ENAB
	mtc0	v0, COP_0_STATUS_REG	# Disable interrupts
	MTC0_SR_IE_HAZARD
	dmfc0	v0, COP_0_TLB_HI	# Get current PID

	mtc0	a0, COP_0_TLB_INDEX	# Set the index register
	TLB_HAZARD
	tlbr				# Read from the TLB
	TLB_HAZARD			# necessary?
	mfc0	ta0, COP_0_TLB_PG_MASK	# fetch the size
	dmfc0	ta1, COP_0_TLB_HI	# fetch the hi entry
	dmfc0	ta2, COP_0_TLB_LO0	# See what we got
	dmfc0	ta3, COP_0_TLB_LO1	# See what we got
	dmtc0	v0, COP_0_TLB_HI	# restore PID
	nop
	nop
	nop				# wait for PID active
	li	a0, TLB_PAGE_MASK
	mtc0	a0, COP_0_TLB_PG_MASK	# Restore default mask value.
	mtc0	v1, COP_0_STATUS_REG	# Restore the status register
	MTC0_SR_IE_HAZARD
	sd	ta0, 0(a1)
	sd	ta1, 8(a1)
	sd	ta2, 16(a1)
	j	ra
	sd	ta3, 24(a1)
END(tlb_read)

/*---------------------------------------------------------------- tlb_get_pid
 *	Read the tlb pid value.
 */
LEAF(tlb_get_pid, 0)
	dmfc0	v0, COP_0_TLB_HI	# get PID
	li	v1, PG_ASID_MASK	# mask off PID
	j	ra
	and	v0, v0, v1		# mask off PID
END(tlb_get_pid)

/*---------------------------------------------------------------- tlb_set_pid
 *	Write the given pid into the TLB pid reg.
 */
LEAF(tlb_set_pid, 0)
	dmtc0	a0, COP_0_TLB_HI		# Write the hi reg value
	TLB_HAZARD
	j	ra
	nop
END(tlb_set_pid)

/*---------------------------------------------------------------- tlb_set_wired
 *	Write the given value into the TLB wired reg.
 */
LEAF(tlb_set_wired, 0)
	mtc0	a0, COP_0_TLB_WIRED
	j	ra
	nop
END(tlb_set_wired)

/*
 *	Initialize the TLB page mask.
 */
LEAF(tlb_set_page_mask, 0)
	mtc0	a0, COP_0_TLB_PG_MASK
	TLB_HAZARD
	j	ra
	nop
END(tlb_set_page_mask)

/*
 *	Initialize the kernel page table pointer.
 *	This is a no-op on non-R8000 processors.
 */
LEAF(tlb_set_gbase, 0)
	j	ra
	 nop
END(tlb_set_gbase)
