#!/usr/bin/env perl

$flavour = shift;

$0 =~ m/(.*[\/\\])[^\/\\]+$/; $dir=$1;
( $xlate="${dir}ppc-xlate.pl" and -f $xlate ) or
( $xlate="${dir}perlasm/ppc-xlate.pl" and -f $xlate) or
die "can't locate ppc-xlate.pl";

open STDOUT,"| $^X $xlate $flavour ".shift || die "can't call $xlate: $!";

if ($flavour=~/64/) {
    $CMPLI="cmpldi";
    $SHRLI="srdi";
    $SIGNX="extsw";
} else {
    $CMPLI="cmplwi";
    $SHRLI="srwi";
    $SIGNX="mr";
}

$code=<<___;
.machine	"any"
.text

#if 0
.globl	.OPENSSL_ppc64_probe
.align	4
.OPENSSL_ppc64_probe:
	fcfid	f1,f1
	extrdi	r0,r0,32,0
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,0,0
#endif

.globl	.OPENSSL_wipe_cpu
.align	4
.OPENSSL_wipe_cpu:
	xor	r0,r0,r0
	fmr	f0,f31
	fmr	f1,f31
	fmr	f2,f31
	mr	r3,r1
	fmr	f3,f31
	xor	r4,r4,r4
	fmr	f4,f31
	xor	r5,r5,r5
	fmr	f5,f31
	xor	r6,r6,r6
	fmr	f6,f31
	xor	r7,r7,r7
	fmr	f7,f31
	xor	r8,r8,r8
	fmr	f8,f31
	xor	r9,r9,r9
	fmr	f9,f31
	xor	r10,r10,r10
	fmr	f10,f31
	xor	r11,r11,r11
	fmr	f11,f31
	xor	r12,r12,r12
	fmr	f12,f31
	fmr	f13,f31
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,0,0

.globl	.OPENSSL_atomic_add
.align	4
.OPENSSL_atomic_add:
Ladd:	lwarx	r5,0,r3
	add	r0,r4,r5
	stwcx.	r0,0,r3
	bne-	Ladd
	$SIGNX	r3,r0
	blr
	.long	0
	.byte	0,12,0x14,0,0,0,2,0
	.long	0
___

$code =~ s/\`([^\`]*)\`/eval $1/gem;
print $code;
close STDOUT;
