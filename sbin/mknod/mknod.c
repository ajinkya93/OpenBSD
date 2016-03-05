/*	$OpenBSD: mknod.c,v 1.22 2016/03/05 16:32:54 millert Exp $	*/
/*	$NetBSD: mknod.c,v 1.8 1995/08/11 00:08:18 jtc Exp $	*/

/*
 * Copyright (c) 1997-2016 Theo de Raadt <deraadt@openbsd.org>,
 *	Marc Espie <espie@openbsd.org>,	Todd Miller <millert@openbsd.org>,
 *	Martin Natano <natano@openbsd.org>
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
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <err.h>

extern char *__progname;


struct node {
	const char *name;
	mode_t mode;
	dev_t dev;
	char type;
	char mflag;
};

static int domakenodes(struct node *, int);
static dev_t compute_device(int, char **);
__dead static void usage(int);

int
main(int argc, char *argv[])
{
	struct node *node;
	int ismkfifo;
	int n = 0;
	int mode = DEFFILEMODE;
	int mflag = 0;
	void *set;
	int ch;

	if (pledge("stdio rpath wpath cpath dpath fattr", NULL) == -1)
		err(1, "pledge");

	setlocale(LC_ALL, "");

	node = reallocarray(NULL, sizeof(struct node), argc);
	if (!node)
		err(1, NULL);


	ismkfifo = strcmp(__progname, "mkfifo") == 0;
	
	/* we parse all arguments upfront */
	while (argc > 1) {
		while ((ch = getopt(argc, argv, "m:")) != -1) {
			switch (ch) {
			case 'm':
				if (!(set = setmode(optarg))) {
					errx(1, "invalid file mode.");
					/* NOTREACHED */
				}

				/*
				 * In symbolic mode strings, the + and -
				 * operators are interpreted relative to
				 * an assumed initial mode of a=rw.
				 */
				mode = getmode(set, DEFFILEMODE);
				if ((mode & ACCESSPERMS) != mode)
					errx(1, "forbidden mode: %o", mode);
				mflag = 1;
				free(set);
				break;
			default:
				usage(ismkfifo);
			}
		}
		argc -= optind;
		argv += optind;

		if (ismkfifo) {
			while (*argv) {
				node[n].mode = mode | S_IFIFO;
				node[n].mflag = mflag;
				node[n].type = 'p';
				node[n].name = *argv;
				node[n].dev = 0;
				n++;
				argv++;
			}
			/* XXX no multiple getopt */
			break;
		} else {
			if (argc < 2)
				usage(ismkfifo);
			node[n].mode = mode;
			node[n].mflag = mflag;
			node[n].name = argv[0];
			if (strlen(argv[1]) != 1)
				errx(1, "node must be type 'b|c|p' %s",
				    argv[1]);
			node[n].type = argv[1][0];

			/* XXX computation offset by one for next getopt */
			switch(node[n].type) {
			case 'p':
				node[n].mode |= S_IFIFO;
				node[n].dev = 0;
				argv++;
				argc--;
				break;
			case 'b':
				node[n].mode |= S_IFBLK;
				goto common;
			case 'c':
				node[n].mode |= S_IFCHR;
common:
				node[n].dev = compute_device(argc, argv);
				argv+=3;
				argc-=3;
				break;
			default:
				errx(1, "node must be type 'b|c|p' %c",
				    node[n].type);
			}
			n++;
		}
		optind = 1;
		optreset = 1;
	}

	if (n == 0)
		usage(ismkfifo);

	return (domakenodes(node, n));
}

static dev_t
compute_device(int argc, char **argv)
{
	dev_t dev;
	char *endp;
	u_int major, minor;

	if (argc < 4)
		usage(0);
	major = (long)strtoul(argv[2], &endp, 0);
	if (endp == argv[2] || *endp != '\0')
		errx(1, "non-numeric major number.");
	minor = (long)strtoul(argv[3], &endp, 0);
	if (endp == argv[3] || *endp != '\0')
		errx(1, "non-numeric minor number.");
	dev = makedev(major, minor);
	if (major(dev) != major || minor(dev) != minor)
		errx(1, "major or minor number too large");
	return dev;
}

static int
domakenodes(struct node *node, int n)
{
	int done_umask = 0;
	int rv = 0;
	int i;

#if !defined(CHECK_PARSING_ONLY)
	for (i = 0; i != n; i++) {
		int r;
		/*
		 * If the user specified a mode via `-m', don't allow the umask
		 * to modify it.  If no `-m' flag was specified, the default
		 * mode is the value of the bitwise inclusive or of S_IRUSR,
		 * S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, and S_IWOTH as
		 * modified by the umask.
		 */
		if (node[i].mflag && !done_umask) {
			(void)umask(0);
			done_umask = 1;
		}

		r = mknod(node[i].name, node[i].mode, node[i].dev);
		if (r < 0) {
			warn("%s", node[i].name);
			rv = 1;
		}
	}
#else
	for (i = 0; i != n; i++)
		printf("%s %c (mode %o) dev=%d\n", node[i].name,
		    node[i].type, node[i].mode, node[i].dev);
		
#endif
	free(node);
	return rv;
}

__dead static void
usage(int ismkfifo)
{

	if (ismkfifo == 1)
		(void)fprintf(stderr, "usage: %s [-m mode] fifo_name ...\n",
		    __progname);
	else {
		(void)fprintf(stderr,
		    "usage: %s [-m mode] name b|c major minor\n",
		    __progname);
		(void)fprintf(stderr, "       %s [-m mode] name p\n",
		    __progname);
	}
	exit(1);
}
