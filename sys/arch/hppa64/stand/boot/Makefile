#	$OpenBSD: Makefile,v 1.6 2016/03/30 06:38:45 jmc Exp $

MAN=	boot.8
MANSUBDIR=hppa64
S	=${.CURDIR}/../../../..
CLEANFILES+=	boot.gdb boot.map boot.lif

.if ${MACHINE} == "hppa64"
PROG=	boot
SRCS=	srt0.S exec.c conf.c

.PATH:	${S}/stand/boot
SRCS+=	bootarg.c boot.c vars.c cmd.c

.PATH:	${S}/lib/libkern/arch/hppa ${S}/lib/libkern
SRCS+=	milli.S ashrdi3.c memcmp.c memcpy.c memset.c muldi3.c \
	strcmp.c strlcpy.c strlen.c strncmp.c strncpy.c

LD?=	ld
LDFLAGS+=-Bstatic -nostartfiles -nostdlib -N -Ttext $(LINKADDR)
LDFLAGS+=-T ${.CURDIR}/ld.script -Map boot.map
LDFLAGS+=--warn-constructors --warn-common
SIZE?=	size
STRIP?=	strip
STANDIR=${.CURDIR}/..
CRTBEGIN=
CRTEND=
LINKS=	${BINDIR}/boot.lif ${BINDIR}/sdboot \
	${BINDIR}/boot.lif ${BINDIR}/stboot \
	${BINDIR}/boot.lif ${BINDIR}/xxboot

LDADD=	${LIBSA} ${LIBZ}
DPADD=	${LIBSA} ${LIBZ}

all:	boot.lif

realinstall:
	${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    boot ${DESTDIR}${BINDIR}
	${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    boot.lif ${DESTDIR}${BINDIR}

.if exists(${.CURDIR}/../../compile/GENERIC/bsd)
bsd: ${.CURDIR}/../../compile/GENERIC/bsd
	gzip -9 -c ${.CURDIR}/../../compile/GENERIC/bsd > bsd
ADDBOOT+=bsd
.endif

# probably we should check for 256k limit for ISL
boot.lif:	${PROG} ${ADDBOOT}
	-@cp ${PROG} ${PROG}.gdb
	${STRIP} ${PROG}
	${MKBOOT} -v ${PROG} ${ADDBOOT} boot.lif

${PROG}: $(OBJS) $(DPADD)
	$(LD) $(LDFLAGS) -o $(PROG) $(OBJS) $(LDADD)
	@${SIZE} $(PROG)
.else
NOPROG=
.endif

.include <bsd.prog.mk>

CPPFLAGS+=${DEBUGFLAGS} -DRELOC=${LOADADDR} -DHEAP_LIMIT=${HEAP_LIMIT}
CFLAGS+=$(SACFLAGS)
CFLAGS+=${NOPIE_FLAGS}
AFLAGS+=${NOPIE_FLAGS}
