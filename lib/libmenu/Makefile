# $OpenBSD: Makefile,v 1.16 2016/03/30 06:38:42 jmc Exp $

LIB=	menu
SRCS=	m_attribs.c m_cursor.c m_driver.c m_format.c \
	m_global.c m_hook.c m_item_cur.c m_item_nam.c m_item_new.c \
	m_item_opt.c m_item_top.c m_item_use.c m_item_val.c m_item_vis.c \
	m_items.c m_new.c m_opts.c m_pad.c m_pattern.c m_post.c \
	m_req_name.c m_scale.c m_spacing.c m_sub.c m_trace.c m_userptr.c m_win.c
HDRS=	menu.h eti.h
CFLAGS+=-I${.CURDIR} -I${.CURDIR}/../libcurses -DHAVE_CONFIG_H
MAN=	menu.3 menu_attributes.3 menu_cursor.3 menu_driver.3 menu_format.3 \
	menu_hook.3 menu_items.3 menu_mark.3 menu_new.3 menu_opts.3 \
	menu_pattern.3 menu_post.3 menu_requestname.3 menu_spacing.3 \
	menu_userptr.3 menu_win.3 mitem_current.3 mitem_name.3 mitem_new.3 \
	mitem_opts.3 mitem_userptr.3 mitem_value.3 mitem_visible.3
CLEANFILES+= menu.3

includes:
	@cd ${.CURDIR}; for i in ${HDRS}; do \
	  cmp -s $$i ${DESTDIR}/usr/include/$$i || \
	  ${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i \
	  ${DESTDIR}/usr/include; done

menu.3: menu.3tbl
	cp ${.ALLSRC} ${.TARGET}

.include <bsd.own.mk>

afterinstall:
	-cd ${DESTDIR}${LIBDIR}; \
	for i in ${_LIBS}; do \
	    ln -f $$i `echo $$i | sed 's/menu/menuw/'`; \
	done
.if (${DEBUGLIBS:L} == "yes")
	-cd ${DESTDIR}${LIBDIR}/debug; \
	    ln -f lib${LIB}.a lib`echo ${LIB} | sed 's/menu/menuw/'`.a
.endif

.include <bsd.lib.mk>
