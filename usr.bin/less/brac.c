/*
 * Copyright (C) 1984-2012  Mark Nudelman
 * Modified for use with illumos by Garrett D'Amore.
 * Copyright 2014 Garrett D'Amore <garrett@damore.org>
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Less License, as specified in the README file.
 *
 * For more information, see the README file.
 */

/*
 * Routines to perform bracket matching functions.
 */

#include "less.h"
#include "position.h"

/*
 * Try to match the n-th open bracket
 *  which appears in the top displayed line (forwdir),
 * or the n-th close bracket
 *  which appears in the bottom displayed line (!forwdir).
 * The characters which serve as "open bracket" and
 * "close bracket" are given.
 */
void
match_brac(int obrac, int cbrac, int forwdir, int n)
{
	int c;
	int nest;
	off_t pos;
	int (*chget)(void);

	/*
	 * Seek to the line containing the open bracket.
	 * This is either the top or bottom line on the screen,
	 * depending on the type of bracket.
	 */
	pos = position((forwdir) ? TOP : BOTTOM);
	if (pos == -1 || ch_seek(pos)) {
		if (forwdir)
			error("Nothing in top line", NULL);
		else
			error("Nothing in bottom line", NULL);
		return;
	}

	/*
	 * Look thru the line to find the open bracket to match.
	 */
	do {
		if ((c = ch_forw_get()) == '\n' || c == EOI) {
			if (forwdir)
				error("No bracket in top line", NULL);
			else
				error("No bracket in bottom line", NULL);
			return;
		}
	} while (c != obrac || --n > 0);

	/*
	 * Position the file just "after" the open bracket
	 * (in the direction in which we will be searching).
	 * If searching forward, we are already after the bracket.
	 * If searching backward, skip back over the open bracket.
	 */
	if (!forwdir)
		(void) ch_back_get();

	/*
	 * Search the file for the matching bracket.
	 */
	chget = (forwdir) ? ch_forw_get : ch_back_get;
	nest = 0;
	while ((c = (*chget)()) != EOI) {
		if (c == obrac) {
			nest++;
		} else if (c == cbrac && --nest < 0) {
			/*
			 * Found the matching bracket.
			 * If searching backward, put it on the top line.
			 * If searching forward, put it on the bottom line.
			 */
			jump_line_loc(ch_tell(), forwdir ? -1 : 1);
			return;
		}
	}
	error("No matching bracket", NULL);
}
