/*	$OpenBSD: enqueue.c,v 1.105 2015/11/03 05:06:24 mmcc Exp $	*/

/*
 * Copyright (c) 2005 Henning Brauer <henning@bulabula.org>
 * Copyright (c) 2009 Jacek Masiulaniec <jacekm@dobremiasto.net>
 * Copyright (c) 2012 Gilles Chehade <gilles@poolp.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/tree.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#include <grp.h>
#include <imsg.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "smtpd.h"

extern struct imsgbuf	*ibuf;

void usage(void);
static void build_from(char *, struct passwd *);
static int parse_message(FILE *, int, int, FILE *);
static void parse_addr(char *, size_t, int);
static void parse_addr_terminal(int);
static char *qualify_addr(char *);
static void rcpt_add(char *);
static int open_connection(void);
static int get_responses(FILE *, int);
static int send_line(FILE *, int, char *, ...);
static int enqueue_offline(int, char *[], FILE *, FILE *);
static int savedeadletter(struct passwd *, FILE *);

extern int srv_connected(void);

enum headerfields {
	HDR_NONE,
	HDR_FROM,
	HDR_TO,
	HDR_CC,
	HDR_BCC,
	HDR_SUBJECT,
	HDR_DATE,
	HDR_MSGID,
	HDR_MIME_VERSION,
	HDR_CONTENT_TYPE,
	HDR_CONTENT_DISPOSITION,
	HDR_CONTENT_TRANSFER_ENCODING,
	HDR_USER_AGENT
};

struct {
	char			*word;
	enum headerfields	 type;
} keywords[] = {
	{ "From:",			HDR_FROM },
	{ "To:",			HDR_TO },
	{ "Cc:",			HDR_CC },
	{ "Bcc:",			HDR_BCC },
	{ "Subject:",			HDR_SUBJECT },
	{ "Date:",			HDR_DATE },
	{ "Message-Id:",		HDR_MSGID },
	{ "MIME-Version:",		HDR_MIME_VERSION },
	{ "Content-Type:",		HDR_CONTENT_TYPE },
	{ "Content-Disposition:",	HDR_CONTENT_DISPOSITION },
	{ "Content-Transfer-Encoding:",	HDR_CONTENT_TRANSFER_ENCODING },
	{ "User-Agent:",		HDR_USER_AGENT },
};

#define	LINESPLIT		990
#define	SMTP_LINELEN		1000
#define	TIMEOUTMSG		"Timeout\n"

#define WSP(c)			(c == ' ' || c == '\t')

int	  verbose = 0;
char	  host[HOST_NAME_MAX+1];
char	 *user = NULL;
time_t	  timestamp;

struct {
	int	  fd;
	char	 *from;
	char	 *fromname;
	char	**rcpts;
	char	 *dsn_notify;
	char	 *dsn_ret;
	char	 *dsn_envid;
	int	  rcpt_cnt;
	int	  need_linesplit;
	int	  saw_date;
	int	  saw_msgid;
	int	  saw_from;
	int	  saw_mime_version;
	int	  saw_content_type;
	int	  saw_content_disposition;
	int	  saw_content_transfer_encoding;
	int	  saw_user_agent;
	int	  noheader;
} msg;

struct {
	uint		quote;
	uint		comment;
	uint		esc;
	uint		brackets;
	size_t		wpos;
	char		buf[SMTP_LINELEN];
} pstate;

static void
qp_encoded_write(FILE *fp, char *buf, size_t len)
{
	while (len) {
		if (*buf == '=')
			fprintf(fp, "=3D");
		else if (*buf == ' ' || *buf == '\t') {
			char *p = buf;

			while (*p != '\n') {
				if (*p != ' ' && *p != '\t')
					break;
				p++;
			}
			if (*p == '\n')
				fprintf(fp, "=%2X", *buf & 0xff);
			else
				fprintf(fp, "%c", *buf & 0xff);
		}
		else if (! isprint((unsigned char)*buf) && *buf != '\n')
			fprintf(fp, "=%2X", *buf & 0xff);
		else
			fprintf(fp, "%c", *buf);
		buf++;
		len--;
	}
}

int
enqueue(int argc, char *argv[], FILE *ofp)
{
	int			 i, ch, tflag = 0;
	char			*fake_from = NULL, *buf = NULL;
	struct passwd		*pw;
	FILE			*fp, *fout;
	size_t			 sz = 0, envid_sz = 0;
	ssize_t			 len;
	int			 fd;
	char			 sfn[] = "/tmp/smtpd.XXXXXXXXXX";
	char			*line;
	int			 dotted;
	int			 inheaders = 0;
	int			 save_argc;
	char			**save_argv;
	int			 no_getlogin = 0;

	memset(&msg, 0, sizeof(msg));
	time(&timestamp);

	save_argc = argc;
	save_argv = argv;

	while ((ch = getopt(argc, argv,
	    "A:B:b:E::e:F:f:iJ::L:mN:o:p:qR:StvV:x")) != -1) {
		switch (ch) {
		case 'f':
			fake_from = optarg;
			break;
		case 'F':
			msg.fromname = optarg;
			break;
		case 'N':
			msg.dsn_notify = optarg;
			break;
		case 'R':
			msg.dsn_ret = optarg;
			break;
		case 'S':
			no_getlogin = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			msg.dsn_envid = optarg;
			break;
		/* all remaining: ignored, sendmail compat */
		case 'A':
		case 'B':
		case 'b':
		case 'E':
		case 'e':
		case 'i':
		case 'L':
		case 'm':
		case 'o':
		case 'p':
		case 'x':
			break;
		case 'q':
			/* XXX: implement "process all now" */
			return (EX_SOFTWARE);
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (getmailname(host, sizeof(host)) == -1)
		err(EX_NOHOST, "getmailname");
	if (no_getlogin) {
		if ((pw = getpwuid(getuid())) == NULL)
			user = "anonymous";
		if (pw != NULL)
			user = xstrdup(pw->pw_name, "enqueue");
	}
	else {
		uid_t ruid = getuid();

		if ((user = getlogin()) != NULL && *user != '\0') {
			if ((pw = getpwnam(user)) == NULL ||
			    (ruid != 0 && ruid != pw->pw_uid))
				pw = getpwuid(ruid);
		} else if ((pw = getpwuid(ruid)) == NULL) {
			user = "anonymous";
		}
		user = xstrdup(pw ? pw->pw_name : user, "enqueue");
	}

	build_from(fake_from, pw);

	while (argc > 0) {
		rcpt_add(argv[0]);
		argv++;
		argc--;
	}

	if ((fd = mkstemp(sfn)) == -1 ||
	    (fp = fdopen(fd, "w+")) == NULL) {
		int saved_errno = errno;
		if (fd != -1) {
			unlink(sfn);
			close(fd);
		}
		errc(EX_UNAVAILABLE, saved_errno, "mkstemp");
	}
	unlink(sfn);
	msg.noheader = parse_message(stdin, fake_from == NULL, tflag, fp);

	if (msg.rcpt_cnt == 0)
		errx(EX_SOFTWARE, "no recipients");

	/* init session */
	rewind(fp);

	/* check if working in offline mode */
	/* If the server is not running, enqueue the message offline */

	if (!srv_connected()) {
		if (pledge("stdio", NULL) == -1)
			err(1, "pledge");

		return (enqueue_offline(save_argc, save_argv, fp, ofp));
	}

	if ((msg.fd = open_connection()) == -1)
		errx(EX_UNAVAILABLE, "server too busy");

	if (pledge("stdio wpath cpath", NULL) == -1)
		err(1, "pledge");

	fout = fdopen(msg.fd, "a+");
	if (fout == NULL)
		err(EX_UNAVAILABLE, "fdopen");

	/*
	 * We need to call get_responses after every command because we don't
	 * support PIPELINING on the server-side yet.
	 */

	/* banner */
	if (! get_responses(fout, 1))
		goto fail;

	send_line(fout, verbose, "EHLO localhost\n");
	if (! get_responses(fout, 1))
		goto fail;

	if (msg.dsn_envid != NULL)
		envid_sz = strlen(msg.dsn_envid);

	send_line(fout, verbose, "MAIL FROM:<%s> %s%s %s%s\n",
	    msg.from,
	    msg.dsn_ret ? "RET=" : "",
	    msg.dsn_ret ? msg.dsn_ret : "",
	    envid_sz ? "ENVID=" : "",
	    envid_sz ? msg.dsn_envid : "");
	if (! get_responses(fout, 1))
		goto fail;

	for (i = 0; i < msg.rcpt_cnt; i++) {
		send_line(fout, verbose, "RCPT TO:<%s> %s%s\n",
		    msg.rcpts[i],
		    msg.dsn_notify ? "NOTIFY=" : "",
		    msg.dsn_notify ? msg.dsn_notify : "");
		if (! get_responses(fout, 1))
			goto fail;
	}

	send_line(fout, verbose, "DATA\n");
	if (! get_responses(fout, 1))
		goto fail;

	/* add From */
	if (!msg.saw_from)
		send_line(fout, 0, "From: %s%s<%s>\n",
		    msg.fromname ? msg.fromname : "",
		    msg.fromname ? " " : "",
		    msg.from);

	/* add Date */
	if (!msg.saw_date)
		send_line(fout, 0, "Date: %s\n", time_to_text(timestamp));

	if (msg.need_linesplit) {
		/* we will always need to mime encode for long lines */
		if (!msg.saw_mime_version)
			send_line(fout, 0, "MIME-Version: 1.0\n");
		if (!msg.saw_content_type)
			send_line(fout, 0, "Content-Type: text/plain; "
			    "charset=unknown-8bit\n");
		if (!msg.saw_content_disposition)
			send_line(fout, 0, "Content-Disposition: inline\n");
		if (!msg.saw_content_transfer_encoding)
			send_line(fout, 0, "Content-Transfer-Encoding: "
			    "quoted-printable\n");
	}

	/* add separating newline */
	if (msg.noheader)
		send_line(fout, 0, "\n");
	else
		inheaders = 1;

	for (;;) {
		if ((len = getline(&buf, &sz, fp)) == -1) {
			if (feof(fp))
				break;
			else
				err(EX_UNAVAILABLE, "getline");
		}

		/* newlines have been normalized on first parsing */
		if (buf[len-1] != '\n')
			errx(EX_SOFTWARE, "expect EOL");

		dotted = 0;
		if (buf[0] == '.') {
			fputc('.', fout);
			dotted = 1;
		}

		line = buf;

		if (inheaders) {
			if (strncasecmp("from ", line, 5) == 0)
				continue;
			if (strncasecmp("return-path: ", line, 13) == 0)
				continue;
		}
		
		if (msg.saw_content_transfer_encoding || msg.noheader ||
		    inheaders || !msg.need_linesplit) {
			send_line(fout, 0, "%.*s", (int)len, line);
			if (inheaders && buf[0] == '\n')
				inheaders = 0;
			continue;
		}

		/* we don't have a content transfer encoding, use our default */
		do {
			if (len < LINESPLIT) {
				qp_encoded_write(fout, line, len);
				break;
			}
			else {
				qp_encoded_write(fout, line,
				    LINESPLIT - 2 - dotted);
				send_line(fout, 0, "=\n");
				line += LINESPLIT - 2 - dotted;
				len -= LINESPLIT - 2 - dotted;
			}
		} while (len);
	}
	free(buf);
	send_line(fout, verbose, ".\n");
	if (! get_responses(fout, 1))
		goto fail;

	send_line(fout, verbose, "QUIT\n");
	if (! get_responses(fout, 1))
		goto fail;

	fclose(fp);
	fclose(fout);

	exit(EX_OK);

fail:
	if (pw)
		savedeadletter(pw, fp);
	exit(EX_SOFTWARE);
}

static int
get_responses(FILE *fin, int n)
{
	char	*buf = NULL;
	size_t	 sz = 0;
	ssize_t	 len;
	int	 e, ret = 0;

	fflush(fin);
	if ((e = ferror(fin))) {
		warnx("ferror: %d", e);
		goto err;
	}

	while (n) {
		if ((len = getline(&buf, &sz, fin)) == -1) {
			if (ferror(fin)) {
				warn("getline");
				goto err;
			} else if (feof(fin))
				break;
			else
				err(EX_UNAVAILABLE, "getline");
		}

		/* account for \r\n linebreaks */
		if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n')
			buf[--len - 1] = '\n';

		if (len < 4) {
			warnx("bad response");
			goto err;
		}

		if (verbose)
			printf("<<< %.*s", (int)len, buf);

		if (buf[3] == '-')
			continue;
		if (buf[0] != '2' && buf[0] != '3') {
			warnx("command failed: %.*s", (int)len, buf);
			goto err;
		}
		n--;
	}

	ret = 1;
err:
	free(buf);
	return ret;
}

static int
send_line(FILE *fp, int v, char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf(fp, fmt, ap);
	va_end(ap);

	if (v) {
		va_start(ap, fmt);
		printf(">>> ");
		ret = vprintf(fmt, ap);
		va_end(ap);
	}

	return (ret);
}

static void
build_from(char *fake_from, struct passwd *pw)
{
	char	*p;

	if (fake_from == NULL)
		msg.from = qualify_addr(user);
	else {
		if (fake_from[0] == '<') {
			if (fake_from[strlen(fake_from) - 1] != '>')
				errx(1, "leading < but no trailing >");
			fake_from[strlen(fake_from) - 1] = 0;
			p = xstrdup(fake_from + 1, "build_from");

			msg.from = qualify_addr(p);
			free(p);
		} else
			msg.from = qualify_addr(fake_from);
	}

	if (msg.fromname == NULL && fake_from == NULL && pw != NULL) {
		int	 len, apos;

		len = strcspn(pw->pw_gecos, ",");
		if ((p = memchr(pw->pw_gecos, '&', len))) {
			apos = p - pw->pw_gecos;
			if (asprintf(&msg.fromname, "%.*s%s%.*s",
			    apos, pw->pw_gecos,
			    pw->pw_name,
			    len - apos - 1, p + 1) == -1)
				err(1, NULL);
			msg.fromname[apos] = toupper((unsigned char)msg.fromname[apos]);
		} else {
			if (asprintf(&msg.fromname, "%.*s", len,
			    pw->pw_gecos) == -1)
				err(1, NULL);
		}
	}
}

static int
parse_message(FILE *fin, int get_from, int tflag, FILE *fout)
{
	char	*buf = NULL;
	size_t	 sz = 0;
	ssize_t	 len;
	uint	 i, cur = HDR_NONE;
	uint	 header_seen = 0, header_done = 0;

	memset(&pstate, 0, sizeof(pstate));
	for (;;) {
		if ((len = getline(&buf, &sz, fin)) == -1) {
			if (feof(fin))
				break;
			else
				err(EX_UNAVAILABLE, "getline");
		}

		/* account for \r\n linebreaks */
		if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n')
			buf[--len - 1] = '\n';

		if (len == 1 && buf[0] == '\n')		/* end of header */
			header_done = 1;

		if (!WSP(buf[0])) {	/* whitespace -> continuation */
			if (cur == HDR_FROM)
				parse_addr_terminal(1);
			if (cur == HDR_TO || cur == HDR_CC || cur == HDR_BCC)
				parse_addr_terminal(0);
			cur = HDR_NONE;
		}

		/* not really exact, if we are still in headers */
		if (len + (buf[len - 1] == '\n' ? 0 : 1) >= LINESPLIT)
			msg.need_linesplit = 1;

		for (i = 0; !header_done && cur == HDR_NONE &&
		    i < nitems(keywords); i++)
			if ((size_t)len > strlen(keywords[i].word) &&
			    !strncasecmp(buf, keywords[i].word,
			    strlen(keywords[i].word)))
				cur = keywords[i].type;

		if (cur != HDR_NONE)
			header_seen = 1;

		if (cur != HDR_BCC) {
			send_line(fout, 0, "%.*s", (int)len, buf);
			if (buf[len - 1] != '\n')
				fputc('\n', fout);
			if (ferror(fout))
				err(1, "write error");
		}

		/*
		 * using From: as envelope sender is not sendmail compatible,
		 * but I really want it that way - maybe needs a knob
		 */
		if (cur == HDR_FROM) {
			msg.saw_from++;
			if (get_from)
				parse_addr(buf, len, 1);
		}

		if (tflag && (cur == HDR_TO || cur == HDR_CC || cur == HDR_BCC))
			parse_addr(buf, len, 0);

		if (cur == HDR_DATE)
			msg.saw_date++;
		if (cur == HDR_MSGID)
			msg.saw_msgid++;
		if (cur == HDR_MIME_VERSION)
			msg.saw_mime_version = 1;
		if (cur == HDR_CONTENT_TYPE)
			msg.saw_content_type = 1;
		if (cur == HDR_CONTENT_DISPOSITION)
			msg.saw_content_disposition = 1;
		if (cur == HDR_CONTENT_TRANSFER_ENCODING)
			msg.saw_content_transfer_encoding = 1;
		if (cur == HDR_USER_AGENT)
			msg.saw_user_agent = 1;
	}

	free(buf);
	return (!header_seen);
}

static void
parse_addr(char *s, size_t len, int is_from)
{
	size_t	 pos = 0;
	int	 terminal = 0;

	/* unless this is a continuation... */
	if (!WSP(s[pos]) && s[pos] != ',' && s[pos] != ';') {
		/* ... skip over everything before the ':' */
		for (; pos < len && s[pos] != ':'; pos++)
			;	/* nothing */
		/* ... and check & reset parser state */
		parse_addr_terminal(is_from);
	}

	/* skip over ':' ',' ';' and whitespace */
	for (; pos < len && !pstate.quote && (WSP(s[pos]) || s[pos] == ':' ||
	    s[pos] == ',' || s[pos] == ';'); pos++)
		;	/* nothing */

	for (; pos < len; pos++) {
		if (!pstate.esc && !pstate.quote && s[pos] == '(')
			pstate.comment++;
		if (!pstate.comment && !pstate.esc && s[pos] == '"')
			pstate.quote = !pstate.quote;

		if (!pstate.comment && !pstate.quote && !pstate.esc) {
			if (s[pos] == ':') {	/* group */
				for (pos++; pos < len && WSP(s[pos]); pos++)
					;	/* nothing */
				pstate.wpos = 0;
			}
			if (s[pos] == '\n' || s[pos] == '\r')
				break;
			if (s[pos] == ',' || s[pos] == ';') {
				terminal = 1;
				break;
			}
			if (s[pos] == '<') {
				pstate.brackets = 1;
				pstate.wpos = 0;
			}
			if (pstate.brackets && s[pos] == '>')
				terminal = 1;
		}

		if (!pstate.comment && !terminal && (!(!(pstate.quote ||
		    pstate.esc) && (s[pos] == '<' || WSP(s[pos]))))) {
			if (pstate.wpos >= sizeof(pstate.buf))
				errx(1, "address exceeds buffer size");
			pstate.buf[pstate.wpos++] = s[pos];
		}

		if (!pstate.quote && pstate.comment && s[pos] == ')')
			pstate.comment--;

		if (!pstate.esc && !pstate.comment && !pstate.quote &&
		    s[pos] == '\\')
			pstate.esc = 1;
		else
			pstate.esc = 0;
	}

	if (terminal)
		parse_addr_terminal(is_from);

	for (; pos < len && (s[pos] == '\r' || s[pos] == '\n'); pos++)
		;	/* nothing */

	if (pos < len)
		parse_addr(s + pos, len - pos, is_from);
}

static void
parse_addr_terminal(int is_from)
{
	if (pstate.comment || pstate.quote || pstate.esc)
		errx(1, "syntax error in address");
	if (pstate.wpos) {
		if (pstate.wpos >= sizeof(pstate.buf))
			errx(1, "address exceeds buffer size");
		pstate.buf[pstate.wpos] = '\0';
		if (is_from)
			msg.from = qualify_addr(pstate.buf);
		else
			rcpt_add(pstate.buf);
		pstate.wpos = 0;
	}
}

static char *
qualify_addr(char *in)
{
	char	*out;

	if (strlen(in) > 0 && strchr(in, '@') == NULL) {
		if (asprintf(&out, "%s@%s", in, host) == -1)
			err(1, "qualify asprintf");
	} else
		out = xstrdup(in, "qualify_addr");

	return (out);
}

static void
rcpt_add(char *addr)
{
	void	*nrcpts;
	char	*p;
	int	n;

	n = 1;
	p = addr;
	while ((p = strchr(p, ',')) != NULL) {
		n++;
		p++;
	}

	if ((nrcpts = reallocarray(msg.rcpts,
	    msg.rcpt_cnt + n, sizeof(char *))) == NULL)
		err(1, "rcpt_add realloc");
	msg.rcpts = nrcpts;

	while (n--) {
		if ((p = strchr(addr, ',')) != NULL)
			*p++ = '\0';
		msg.rcpts[msg.rcpt_cnt++] = qualify_addr(addr);
		if (p == NULL)
			break;
		addr = p;
	}
}

static int
open_connection(void)
{
	struct imsg	imsg;
	int		fd;
	int		n;

	imsg_compose(ibuf, IMSG_CTL_SMTP_SESSION, IMSG_VERSION, 0, -1, NULL, 0);

	while (ibuf->w.queued)
		if (msgbuf_write(&ibuf->w) <= 0 && errno != EAGAIN)
			err(1, "write error");

	while (1) {
		if ((n = imsg_read(ibuf)) == -1)
			errx(1, "imsg_read error");
		if (n == 0)
			errx(1, "pipe closed");

		if ((n = imsg_get(ibuf, &imsg)) == -1)
			errx(1, "imsg_get error");
		if (n == 0)
			continue;

		switch (imsg.hdr.type) {
		case IMSG_CTL_OK:
			break;
		case IMSG_CTL_FAIL:
			errx(1, "server disallowed submission request");
		default:
			errx(1, "unexpected imsg reply type");
		}

		fd = imsg.fd;
		imsg_free(&imsg);

		break;
	}

	return fd;
}

static int
enqueue_offline(int argc, char *argv[], FILE *ifile, FILE *ofile)
{
	int	i, ch;

	for (i = 1; i < argc; i++) {
		if (strchr(argv[i], '|') != NULL) {
			warnx("%s contains illegal character", argv[i]);
			ftruncate(fileno(ofile), 0);
			exit(EX_SOFTWARE);
		}
		if (fprintf(ofile, "%s%s", i == 1 ? "" : "|", argv[i]) < 0)
			goto write_error;
	}

	if (fputc('\n', ofile) == EOF)
		goto write_error;

	while ((ch = fgetc(ifile)) != EOF) {
		if (fputc(ch, ofile) == EOF)
			goto write_error;
	}

	if (ferror(ifile)) {
		warn("read error");
		ftruncate(fileno(ofile), 0);
		exit(EX_UNAVAILABLE);
	}

	if (fclose(ofile) == EOF)
		goto write_error;

	return (EX_TEMPFAIL);
write_error:
	warn("write error");
	ftruncate(fileno(ofile), 0);
	exit(EX_UNAVAILABLE);
}

static int
savedeadletter(struct passwd *pw, FILE *in)
{
	char	 buffer[PATH_MAX];
	FILE	*fp;
	char	*buf = NULL;
	size_t	 sz = 0;
	ssize_t	 len;

	(void)snprintf(buffer, sizeof buffer, "%s/dead.letter", pw->pw_dir);

	if (fseek(in, 0, SEEK_SET) != 0)
		return 0;
	
	if ((fp = fopen(buffer, "w")) == NULL)
		return 0;

	/* add From */
	if (!msg.saw_from)
		fprintf(fp, "From: %s%s<%s>\n",
		    msg.fromname ? msg.fromname : "",
		    msg.fromname ? " " : "",
		    msg.from);

	/* add Date */
	if (!msg.saw_date)
		fprintf(fp, "Date: %s\n", time_to_text(timestamp));

	if (msg.need_linesplit) {
		/* we will always need to mime encode for long lines */
		if (!msg.saw_mime_version)
			fprintf(fp, "MIME-Version: 1.0\n");
		if (!msg.saw_content_type)
			fprintf(fp, "Content-Type: text/plain; "
			    "charset=unknown-8bit\n");
		if (!msg.saw_content_disposition)
			fprintf(fp, "Content-Disposition: inline\n");
		if (!msg.saw_content_transfer_encoding)
			fprintf(fp, "Content-Transfer-Encoding: "
			    "quoted-printable\n");
	}

	/* add separating newline */
	if (msg.noheader)
		fprintf(fp, "\n");

	while ((len = getline(&buf, &sz, in)) != -1) {
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		fprintf(fp, "%s\n", buf);
	}

	free(buf);
	fprintf(fp, "\n");
	fclose(fp);
	return 1;
}
