#	$OpenBSD: Makefile,v 1.21 2016/04/20 01:11:45 schwarze Exp $
#	$NetBSD: Makefile,v 1.41 2010/02/03 15:34:43 roy Exp $
#	@(#)Makefile	8.1 (Berkeley) 6/4/93

LIB=	edit

OSRCS=	chared.c common.c el.c eln.c emacs.c fcns.c filecomplete.c help.c \
	hist.c keymacro.c map.c chartype.c \
	parse.c prompt.c read.c refresh.c search.c sig.c terminal.c tty.c vi.c

MAN=	editline.3 editrc.5 editline.7

# For speed and debugging
#SRCS=   ${OSRCS} tokenizer.c history.c readline.c
# For protection
SRCS=	editline.c tokenizer.c history.c readline.c

SRCS+= tokenizern.c historyn.c
CLEANFILES+=tokenizern.c.tmp tokenizern.c historyn.c.tmp historyn.c

LIBEDITDIR?=${.CURDIR}

INCS= histedit.h
INCSDIR=/usr/include

CLEANFILES+=editline.c
CLEANFILES+=common.h.tmp editline.c.tmp emacs.h.tmp fcns.c.tmp fcns.h.tmp
CLEANFILES+=help.c.tmp help.h.tmp vi.h.tmp tc1.o tc1
CFLAGS+=-Wall
CPPFLAGS+=-I. -I${LIBEDITDIR}
CPPFLAGS+=-I. -I${.CURDIR}
#CPPFLAGS+=-DDEBUG_TTY -DDEBUG_KEY -DDEBUG_READ -DDEBUG -DDEBUG_REFRESH
#CPPFLAGS+=-DDEBUG_PASTE -DDEBUG_EDIT

AHDR=vi.h emacs.h common.h
ASRC=${LIBEDITDIR}/vi.c ${LIBEDITDIR}/emacs.c ${LIBEDITDIR}/common.c

CLEANFILES+=	${AHDR} fcns.h help.h fcns.c help.c

#SUBDIR+=	readline
HOST_SH=	sh

vi.h: vi.c makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -h ${LIBEDITDIR}/vi.c \
	    > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

emacs.h: emacs.c makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -h ${LIBEDITDIR}/emacs.c \
	    > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

common.h: common.c makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -h ${LIBEDITDIR}/common.c \
	    > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

fcns.h: ${AHDR} makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -fh ${AHDR} > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

fcns.c: ${AHDR} fcns.h help.h makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -fc ${AHDR} > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

help.c: ${ASRC} makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -bc ${ASRC} > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

help.h: ${ASRC} makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -bh ${ASRC} > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

editline.c: ${OSRCS} makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -e ${OSRCS:T} > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

tokenizern.c: makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -n tokenizer.c > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

historyn.c: makelist
	${HOST_SH} ${LIBEDITDIR}/makelist -n history.c > ${.TARGET}.tmp && \
	    mv ${.TARGET}.tmp ${.TARGET}

tc1.o:	${LIBEDITDIR}/TEST/tc1.c

tc1:	libedit.a tc1.o
	${CC} ${LDFLAGS} ${.ALLSRC} -o ${.TARGET} libedit.a ${LDADD} -lcurses

includes:
	-cd ${.CURDIR}; cmp -s histedit.h ${DESTDIR}/usr/include/histedit.h > \
		/dev/null 2>&1 || \
	${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m 444 histedit.h \
		${DESTDIR}/usr/include

.include <bsd.lib.mk>
.include <bsd.subdir.mk>
