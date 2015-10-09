/*	$OpenBSD: test_tty.c,v 1.2 2015/10/09 11:42:54 semarie Exp $ */
/*
 * Copyright (c) 2015 Sebastien Marie <semarie@openbsd.org>
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

#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/ttycom.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


void
test_request_tty()
{
	int fd = STDERR_FILENO;
	struct termios ts; /* sys/termios.h */
	struct winsize ws; /* sys/ttycom.h */

	/* TODO: get a tty */

	/* tests that need tty+proc (malloc for self) */
	if (pledge("tty proc malloc", NULL) == -1)
		_exit(errno);

	/* TIOCSPGRP (tty+proc) */
	if ((tcsetpgrp(fd, 1) == -1) && (errno != ENOTTY))
		_exit(errno);
	errno = 0; /* discard error */

	/* tests that only need tty */	
	if (pledge("tty", NULL) == -1)
		_exit(errno);


	/* TIOCGETA */
	if (ioctl(fd, TIOCGETA, &ts) == -1)
		_exit(errno);

	/* TIOCGWINSZ */
	if (ioctl(fd, TIOCGWINSZ, &ws) == -1)
		_exit(errno);

	/* TIOCSBRK */
	if ((ioctl(fd, TIOCSBRK, NULL) == -1) && (errno != ENOTTY))
		_exit(errno);
	errno = 0; /* discard error */

	/* TIOCCDTR */
	if ((ioctl(fd, TIOCCDTR, NULL) == -1) && (errno != ENOTTY))
		_exit(errno);
	errno = 0; /* discard error */

	/* TIOCSETA */
	if (tcsetattr(fd, TCSANOW, &ts) == -1)
		_exit(errno);
	
	/* TIOCSETAW */
	if (tcsetattr(fd, TCSADRAIN, &ts) == -1)
		_exit(errno);
	
	/* TIOCSETAF */
	if (tcsetattr(fd, TCSAFLUSH, &ts) == -1)
		_exit(errno);
}
