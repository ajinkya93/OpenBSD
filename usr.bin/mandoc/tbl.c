/*	$OpenBSD: tbl.c,v 1.15 2015/01/27 05:20:30 schwarze Exp $ */
/*
 * Copyright (c) 2009, 2010, 2011 Kristaps Dzonsons <kristaps@bsd.lv>
 * Copyright (c) 2011, 2015 Ingo Schwarze <schwarze@openbsd.org>
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
#include <sys/types.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mandoc.h"
#include "mandoc_aux.h"
#include "libmandoc.h"
#include "libroff.h"


enum rofferr
tbl_read(struct tbl_node *tbl, int ln, const char *p, int offs)
{
	const char	*cp;
	int		 active;

	/*
	 * In the options section, proceed to the layout section
	 * after a semicolon, or right away if there is no semicolon.
	 * Ignore semicolons in arguments.
	 */

	if (tbl->part == TBL_PART_OPTS) {
		tbl->part = TBL_PART_LAYOUT;
		active = 1;
		for (cp = p; *cp != '\0'; cp++) {
			switch (*cp) {
			case '(':
				active = 0;
				continue;
			case ')':
				active = 1;
				continue;
			case ';':
				if (active)
					break;
				continue;
			default:
				continue;
			}
			break;
		}
		if (*cp == ';') {
			tbl_option(tbl, ln, p);
			if (*(p = cp + 1) == '\0')
				return(ROFF_IGN);
		}
	}

	/* Process the other section types.  */

	switch (tbl->part) {
	case TBL_PART_LAYOUT:
		tbl_layout(tbl, ln, p);
		return(ROFF_IGN);
	case TBL_PART_CDATA:
		return(tbl_cdata(tbl, ln, p) ? ROFF_TBL : ROFF_IGN);
	default:
		break;
	}

	tbl_data(tbl, ln, p);
	return(ROFF_TBL);
}

struct tbl_node *
tbl_alloc(int pos, int line, struct mparse *parse)
{
	struct tbl_node	*tbl;

	tbl = mandoc_calloc(1, sizeof(struct tbl_node));
	tbl->line = line;
	tbl->pos = pos;
	tbl->parse = parse;
	tbl->part = TBL_PART_OPTS;
	tbl->opts.tab = '\t';
	tbl->opts.decimal = '.';
	return(tbl);
}

void
tbl_free(struct tbl_node *tbl)
{
	struct tbl_row	*rp;
	struct tbl_cell	*cp;
	struct tbl_span	*sp;
	struct tbl_dat	*dp;
	struct tbl_head	*hp;

	while (NULL != (rp = tbl->first_row)) {
		tbl->first_row = rp->next;
		while (rp->first) {
			cp = rp->first;
			rp->first = cp->next;
			free(cp);
		}
		free(rp);
	}

	while (NULL != (sp = tbl->first_span)) {
		tbl->first_span = sp->next;
		while (sp->first) {
			dp = sp->first;
			sp->first = dp->next;
			if (dp->string)
				free(dp->string);
			free(dp);
		}
		free(sp);
	}

	while (NULL != (hp = tbl->first_head)) {
		tbl->first_head = hp->next;
		free(hp);
	}

	free(tbl);
}

void
tbl_restart(int line, int pos, struct tbl_node *tbl)
{
	if (TBL_PART_CDATA == tbl->part)
		mandoc_msg(MANDOCERR_TBLBLOCK, tbl->parse,
		    tbl->line, tbl->pos, NULL);

	tbl->part = TBL_PART_LAYOUT;
	tbl->line = line;
	tbl->pos = pos;

	if (NULL == tbl->first_span || NULL == tbl->first_span->first)
		mandoc_msg(MANDOCERR_TBLNODATA, tbl->parse,
		    tbl->line, tbl->pos, NULL);
}

const struct tbl_span *
tbl_span(struct tbl_node *tbl)
{
	struct tbl_span	 *span;

	assert(tbl);
	span = tbl->current_span ? tbl->current_span->next
				 : tbl->first_span;
	if (span)
		tbl->current_span = span;
	return(span);
}

void
tbl_end(struct tbl_node **tblp)
{
	struct tbl_node	*tbl;
	struct tbl_span *sp;

	tbl = *tblp;
	*tblp = NULL;

	sp = tbl->first_span;
	while (sp != NULL && sp->first == NULL)
		sp = sp->next;
	if (sp == NULL)
		mandoc_msg(MANDOCERR_TBLNODATA, tbl->parse,
		    tbl->line, tbl->pos, NULL);

	if (tbl->last_span)
		tbl->last_span->flags |= TBL_SPAN_LAST;

	if (TBL_PART_CDATA == tbl->part)
		mandoc_msg(MANDOCERR_TBLBLOCK, tbl->parse,
		    tbl->line, tbl->pos, NULL);
}
