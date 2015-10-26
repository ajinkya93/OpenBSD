/*	$OpenBSD: misc.c,v 1.59 2015/10/26 14:27:41 millert Exp $	*/

/* Copyright 1988,1990,1993,1994 by Paul Vixie
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "cron.h"
#include <limits.h>

#if defined(LOG_DAEMON) && !defined(LOG_CRON)
# define LOG_CRON LOG_DAEMON
#endif

#ifndef FACILITY
#define FACILITY LOG_CRON
#endif

static int LogFD = -1;

static int syslog_open = FALSE;

int
strcmp_until(const char *left, const char *right, char until)
{
	while (*left && *left != until && *left == *right) {
		left++;
		right++;
	}

	if ((*left=='\0' || *left == until) &&
	    (*right=='\0' || *right == until)) {
		return (0);
	}
	return (*left - *right);
}

void
set_cron_cwd(void)
{
	struct stat sb;
	struct group *grp = NULL;

	grp = getgrnam(CRON_GROUP);
	/* first check for CRONDIR ("/var/cron" or some such)
	 */
	if (stat(CRONDIR, &sb) < 0 && errno == ENOENT) {
		perror(CRONDIR);
		if (0 == mkdir(CRONDIR, 0710)) {
			fprintf(stderr, "%s: created\n", CRONDIR);
			stat(CRONDIR, &sb);
		} else {
			fprintf(stderr, "%s: ", CRONDIR);
			perror("mkdir");
			exit(EXIT_FAILURE);
		}
	}
	if (!S_ISDIR(sb.st_mode)) {
		fprintf(stderr, "'%s' is not a directory, bailing out.\n",
			CRONDIR);
		exit(EXIT_FAILURE);
	}
	if (chdir(CRONDIR) < 0) {
		fprintf(stderr, "cannot chdir(%s), bailing out.\n", CRONDIR);
		perror(CRONDIR);
		exit(EXIT_FAILURE);
	}

	/* CRONDIR okay (now==CWD), now look at SPOOL_DIR ("tabs" or some such)
	 */
	if (stat(SPOOL_DIR, &sb) < 0 && errno == ENOENT) {
		perror(SPOOL_DIR);
		if (0 == mkdir(SPOOL_DIR, 0700)) {
			fprintf(stderr, "%s: created\n", SPOOL_DIR);
			stat(SPOOL_DIR, &sb);
		} else {
			fprintf(stderr, "%s: ", SPOOL_DIR);
			perror("mkdir");
			exit(EXIT_FAILURE);
		}
	}
	if (!S_ISDIR(sb.st_mode)) {
		fprintf(stderr, "'%s' is not a directory, bailing out.\n",
			SPOOL_DIR);
		exit(EXIT_FAILURE);
	}
	if (grp != NULL) {
		if (sb.st_gid != grp->gr_gid)
			chown(SPOOL_DIR, -1, grp->gr_gid);
		if (sb.st_mode != 01730)
			chmod(SPOOL_DIR, 01730);
	}

	/* finally, look at AT_DIR ("atjobs" or some such)
	 */
	if (stat(AT_DIR, &sb) < 0 && errno == ENOENT) {
		perror(AT_DIR);
		if (0 == mkdir(AT_DIR, 0700)) {
			fprintf(stderr, "%s: created\n", AT_DIR);
			stat(AT_DIR, &sb);
		} else {
			fprintf(stderr, "%s: ", AT_DIR);
			perror("mkdir");
			exit(EXIT_FAILURE);
		}
	}
	if (!S_ISDIR(sb.st_mode)) {
		fprintf(stderr, "'%s' is not a directory, bailing out.\n",
			AT_DIR);
		exit(EXIT_FAILURE);
	}
	if (grp != NULL) {
		if (sb.st_gid != grp->gr_gid)
			chown(AT_DIR, -1, grp->gr_gid);
		if (sb.st_mode != 01770)
			chmod(AT_DIR, 01770);
	}
}

/* get_char(file) : like getc() but increment LineNumber on newlines
 */
int
get_char(FILE *file)
{
	int ch;

	ch = getc(file);
	if (ch == '\n')
		Set_LineNum(LineNumber + 1)
	return (ch);
}

/* unget_char(ch, file) : like ungetc but do LineNumber processing
 */
void
unget_char(int ch, FILE *file)
{
	ungetc(ch, file);
	if (ch == '\n')
		Set_LineNum(LineNumber - 1)
}

/* get_string(str, max, file, termstr) : like fgets() but
 *		(1) has terminator string which should include \n
 *		(2) will always leave room for the null
 *		(3) uses get_char() so LineNumber will be accurate
 *		(4) returns EOF or terminating character, whichever
 */
int
get_string(char *string, int size, FILE *file, char *terms)
{
	int ch;

	while (EOF != (ch = get_char(file)) && !strchr(terms, ch)) {
		if (size > 1) {
			*string++ = ch;
			size--;
		}
	}

	if (size > 0)
		*string = '\0';

	return (ch);
}

/* skip_comments(file) : read past comment (if any)
 */
void
skip_comments(FILE *file)
{
	int ch;

	while (EOF != (ch = get_char(file))) {
		/* ch is now the first character of a line.
		 */

		while (ch == ' ' || ch == '\t')
			ch = get_char(file);

		if (ch == EOF)
			break;

		/* ch is now the first non-blank character of a line.
		 */

		if (ch != '\n' && ch != '#')
			break;

		/* ch must be a newline or comment as first non-blank
		 * character on a line.
		 */

		while (ch != '\n' && ch != EOF)
			ch = get_char(file);

		/* ch is now the newline of a line which we're going to
		 * ignore.
		 */
	}
	if (ch != EOF)
		unget_char(ch, file);
}

/* int in_file(const char *string, FILE *file, int error)
 *	return TRUE if one of the lines in file matches string exactly,
 *	FALSE if no lines match, and error on error.
 */
static int
in_file(const char *string, FILE *file, int error)
{
	char line[MAX_TEMPSTR];
	char *endp;

	if (fseek(file, 0L, SEEK_SET))
		return (error);
	while (fgets(line, MAX_TEMPSTR, file)) {
		if (line[0] != '\0') {
			endp = &line[strlen(line) - 1];
			if (*endp != '\n')
				return (error);
			*endp = '\0';
			if (0 == strcmp(line, string))
				return (TRUE);
		}
	}
	if (ferror(file))
		return (error);
	return (FALSE);
}

/* int allowed(const char *username, const char *allow_file, const char *deny_file)
 *	returns TRUE if (allow_file exists and user is listed)
 *	or (deny_file exists and user is NOT listed).
 *	root is always allowed.
 */
int
allowed(const char *username, const char *allow_file, const char *deny_file)
{
	FILE	*fp;
	int	isallowed;

	if (strcmp(username, "root") == 0)
		return (TRUE);
	isallowed = FALSE;
	if ((fp = fopen(allow_file, "r")) != NULL) {
		isallowed = in_file(username, fp, FALSE);
		fclose(fp);
	} else if ((fp = fopen(deny_file, "r")) != NULL) {
		isallowed = !in_file(username, fp, FALSE);
		fclose(fp);
	}
	return (isallowed);
}

void
log_it(const char *username, pid_t xpid, const char *event, const char *detail)
{
	char **info, *info_events[] = { "CMD", "ATJOB", "BEGIN EDIT", "DELETE",
	    "END EDIT", "LIST", "MAIL", "RELOAD", "REPLACE", "STARTUP", NULL };

	if (!syslog_open) {
		openlog(ProgramName, LOG_PID, FACILITY);
		syslog_open = TRUE;		/* assume openlog success */
	}

	for (info = info_events; *info; info++)
		if (!strcmp(event, *info))
			break;
	syslog(*info ? LOG_INFO : LOG_WARNING, "(%s) %s (%s)", username, event,
	    detail);


}

void
log_close(void)
{
	if (LogFD != -1) {
		close(LogFD);
		LogFD = -1;
	}
	closelog();
	syslog_open = FALSE;
}

/* char *first_word(char *s, char *t)
 *	return pointer to first word
 * parameters:
 *	s - string we want the first word of
 *	t - terminators, implicitly including \0
 * warnings:
 *	(1) this routine is fairly slow
 *	(2) it returns a pointer to static storage
 */
char *
first_word(char *s, char *t)
{
	static char retbuf[2][MAX_TEMPSTR + 1];	/* sure wish C had GC */
	static int retsel = 0;
	char *rb, *rp;

	/* select a return buffer */
	retsel = 1-retsel;
	rb = &retbuf[retsel][0];
	rp = rb;

	/* skip any leading terminators */
	while (*s && (NULL != strchr(t, *s))) {
		s++;
	}

	/* copy until next terminator or full buffer */
	while (*s && (NULL == strchr(t, *s)) && (rp < &rb[MAX_TEMPSTR])) {
		*rp++ = *s++;
	}

	/* finish the return-string and return it */
	*rp = '\0';
	return (rb);
}

/* warning:
 *	heavily ascii-dependent.
 */
void
mkprint(dst, src, len)
	char *dst;
	unsigned char *src;
	int len;
{
	/*
	 * XXX
	 * We know this routine can't overflow the dst buffer because mkprints()
	 * allocated enough space for the worst case.
	 */
	while (len-- > 0)
	{
		unsigned char ch = *src++;

		if (ch < ' ') {			/* control character */
			*dst++ = '^';
			*dst++ = ch + '@';
		} else if (ch < 0177) {		/* printable */
			*dst++ = ch;
		} else if (ch == 0177) {	/* delete/rubout */
			*dst++ = '^';
			*dst++ = '?';
		} else {			/* parity character */
			snprintf(dst, 5, "\\%03o", ch);
			dst += strlen(dst);
		}
	}
	*dst = '\0';
}

/* warning:
 *	returns a pointer to malloc'd storage, you must call free yourself.
 */
char *
mkprints(src, len)
	unsigned char *src;
	unsigned int len;
{
	char *dst = malloc(len*4 + 1);

	if (dst)
		mkprint(dst, src, len);

	return (dst);
}


static gid_t save_egid;
int swap_gids() { save_egid = getegid(); return (setegid(getgid())); }
int swap_gids_back() { return (setegid(save_egid)); }

/* Return the offset from GMT in seconds (algorithm taken from sendmail).
 *
 * warning:
 *	clobbers the static storage space used by localtime() and gmtime().
 *	If the local pointer is non-NULL it *must* point to a local copy.
 */

/* int open_socket(void)
 *	opens a UNIX domain socket that crontab uses to poke cron.
 *	If the socket is already in use, return an error.
 */
int
open_socket(void)
{
	int		   sock;
	mode_t		   omask;
	struct sockaddr_un s_un;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1) {
		fprintf(stderr, "%s: can't create socket: %s\n",
		    ProgramName, strerror(errno));
		log_it("CRON", getpid(), "DEATH", "can't create socket");
		exit(EXIT_FAILURE);
	}
	if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) {
		fprintf(stderr, "%s: can't make socket close on exec: %s\n",
		    ProgramName, strerror(errno));
		log_it("CRON", getpid(), "DEATH",
		    "can't make socket close on exec");
		exit(EXIT_FAILURE);
	}
	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
		fprintf(stderr, "%s: can't make socket non-blocking: %s\n",
		    ProgramName, strerror(errno));
		log_it("CRON", getpid(), "DEATH",
		    "can't make socket non-blocking");
		exit(EXIT_FAILURE);
	}
	bzero(&s_un, sizeof(s_un));
	if (snprintf(s_un.sun_path, sizeof s_un.sun_path, "%s/%s",
	      SPOOL_DIR, CRONSOCK) >= sizeof(s_un.sun_path)) {
		fprintf(stderr, "%s/%s: path too long\n", SPOOL_DIR, CRONSOCK);
		log_it("CRON", getpid(), "DEATH", "path too long");
		exit(EXIT_FAILURE);
	}
	s_un.sun_family = AF_UNIX;
	s_un.sun_len = SUN_LEN(&s_un);

	if (connect(sock, (struct sockaddr *)&s_un, sizeof(s_un)) == 0) {
		fprintf(stderr, "%s: already running\n", ProgramName);
		log_it("CRON", getpid(), "DEATH", "already running");
		exit(EXIT_FAILURE);
	}
	if (errno != ENOENT)
		unlink(s_un.sun_path);

	omask = umask(007);
	if (bind(sock, (struct sockaddr *)&s_un, sizeof(s_un))) {
		fprintf(stderr, "%s: can't bind socket: %s\n",
		    ProgramName, strerror(errno));
		log_it("CRON", getpid(), "DEATH", "can't bind socket");
		umask(omask);
		exit(EXIT_FAILURE);
	}
	umask(omask);
	if (listen(sock, SOMAXCONN)) {
		fprintf(stderr, "%s: can't listen on socket: %s\n",
		    ProgramName, strerror(errno));
		log_it("CRON", getpid(), "DEATH", "can't listen on socket");
		exit(EXIT_FAILURE);
	}
	chmod(s_un.sun_path, 0660);

	return(sock);
}

void
poke_daemon(const char *spool_dir, unsigned char cookie)
{
	int sock = -1;
	struct sockaddr_un s_un;

	(void) utime(spool_dir, NULL);		/* old poke method */

	bzero(&s_un, sizeof(s_un));
	if (snprintf(s_un.sun_path, sizeof s_un.sun_path, "%s/%s",
	      SPOOL_DIR, CRONSOCK) >= sizeof(s_un.sun_path)) {
		fprintf(stderr, "%s: %s/%s: path too long\n",
		    ProgramName, SPOOL_DIR, CRONSOCK);
		return;
	}
	s_un.sun_family = AF_UNIX;
	s_un.sun_len = SUN_LEN(&s_un);
	(void) signal(SIGPIPE, SIG_IGN);
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0 &&
	    connect(sock, (struct sockaddr *)&s_un, sizeof(s_un)) == 0)
		write(sock, &cookie, 1);
	else
		fprintf(stderr, "%s: warning, cron does not appear to be "
		    "running.\n", ProgramName);
	if (sock >= 0)
		close(sock);
	(void) signal(SIGPIPE, SIG_DFL);
}

int
strtot(const char *nptr, char **endptr, time_t *tp)
{
	long long ll;

	ll = strtoll(nptr, endptr, 10);
	if (ll < 0 || (time_t)ll != ll)
		return (-1);
	*tp = (time_t)ll;
	return (0);
}
