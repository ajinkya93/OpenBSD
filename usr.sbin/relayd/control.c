/*	$OpenBSD: control.c,v 1.38 2011/05/09 12:08:47 reyk Exp $	*/

/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
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

#include <sys/queue.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <net/if.h>

#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <openssl/ssl.h>

#include "relayd.h"

#define	CONTROL_BACKLOG	5

struct ctl_connlist ctl_conns;

void		 control_accept(int, short, void *);
void		 control_close(int);

int
control_init(struct privsep *ps, struct control_sock *cs)
{
	struct relayd		*env = ps->ps_env;
	struct sockaddr_un	 sun;
	int			 fd;
	mode_t			 old_umask, mode;

	if (cs->cs_name == NULL)
		return (0);

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		log_warn("%s: socket", __func__);
		return (-1);
	}

	sun.sun_family = AF_UNIX;
	if (strlcpy(sun.sun_path, cs->cs_name,
	    sizeof(sun.sun_path)) >= sizeof(sun.sun_path)) {
		log_warn("%s: %s name too long", __func__, cs->cs_name);
		close(fd);
		return (-1);
	}

	if (unlink(cs->cs_name) == -1)
		if (errno != ENOENT) {
			log_warn("%s: unlink %s", __func__, cs->cs_name);
			close(fd);
			return (-1);
		}

	if (cs->cs_restricted) {
		old_umask = umask(S_IXUSR|S_IXGRP|S_IXOTH);
		mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	} else {
		old_umask = umask(S_IXUSR|S_IXGRP|S_IWOTH|S_IROTH|S_IXOTH);
		mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;
	}

	if (bind(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
		log_warn("%s: bind: %s", __func__, cs->cs_name);
		close(fd);
		(void)umask(old_umask);
		return (-1);
	}
	(void)umask(old_umask);

	if (chmod(cs->cs_name, mode) == -1) {
		log_warn("%s: chmod", __func__);
		close(fd);
		(void)unlink(cs->cs_name);
		return (-1);
	}

	socket_set_blockmode(fd, BM_NONBLOCK);
	cs->cs_fd = fd;
	cs->cs_env = env;

	return (0);
}

int
control_listen(struct control_sock *cs)
{
	if (cs->cs_name == NULL)
		return (0);

	if (listen(cs->cs_fd, CONTROL_BACKLOG) == -1) {
		log_warn("%s: listen", __func__);
		return (-1);
	}

	event_set(&cs->cs_ev, cs->cs_fd, EV_READ | EV_PERSIST,
	    control_accept, cs->cs_env);
	event_add(&cs->cs_ev, NULL);

	return (0);
}

void
control_cleanup(struct control_sock *cs)
{
	if (cs->cs_name == NULL)
		return;
	(void)unlink(cs->cs_name);
}

/* ARGSUSED */
void
control_accept(int listenfd, short event, void *arg)
{
	int			 connfd;
	socklen_t		 len;
	struct sockaddr_un	 sun;
	struct ctl_conn		*c;
	struct relayd		*env = arg;

	len = sizeof(sun);
	if ((connfd = accept(listenfd,
	    (struct sockaddr *)&sun, &len)) == -1) {
		if (errno != EWOULDBLOCK && errno != EINTR)
			log_warn("%s: accept", __func__);
		return;
	}

	socket_set_blockmode(connfd, BM_NONBLOCK);

	if ((c = calloc(1, sizeof(struct ctl_conn))) == NULL) {
		close(connfd);
		log_warn("%s: calloc", __func__);
		return;
	}

	imsg_init(&c->iev.ibuf, connfd);
	c->iev.handler = control_dispatch_imsg;
	c->iev.events = EV_READ;
	event_set(&c->iev.ev, c->iev.ibuf.fd, c->iev.events,
	    c->iev.handler, env);
	event_add(&c->iev.ev, NULL);

	TAILQ_INSERT_TAIL(&ctl_conns, c, entry);
}

struct ctl_conn *
control_connbyfd(int fd)
{
	struct ctl_conn	*c;

	for (c = TAILQ_FIRST(&ctl_conns); c != NULL && c->iev.ibuf.fd != fd;
	    c = TAILQ_NEXT(c, entry))
		;	/* nothing */

	return (c);
}

void
control_close(int fd)
{
	struct ctl_conn	*c;

	if ((c = control_connbyfd(fd)) == NULL) {
		log_warn("%s: fd %d not found", __func__, fd);
		return;
	}

	msgbuf_clear(&c->iev.ibuf.w);
	TAILQ_REMOVE(&ctl_conns, c, entry);

	event_del(&c->iev.ev);
	close(c->iev.ibuf.fd);
	free(c);
}

/* ARGSUSED */
void
control_dispatch_imsg(int fd, short event, void *arg)
{
	struct ctl_conn		*c;
	struct imsg		 imsg;
	struct ctl_id		 id;
	int			 n;
	int			 verbose;
	struct relayd		*env = arg;

	if ((c = control_connbyfd(fd)) == NULL) {
		log_warn("%s: fd %d not found", __func__, fd);
		return;
	}

	if (event & EV_READ) {
		if ((n = imsg_read(&c->iev.ibuf)) == -1 || n == 0) {
			control_close(fd);
			return;
		}
	}

	if (event & EV_WRITE) {
		if (msgbuf_write(&c->iev.ibuf.w) < 0) {
			control_close(fd);
			return;
		}
	}

	for (;;) {
		if ((n = imsg_get(&c->iev.ibuf, &imsg)) == -1) {
			control_close(fd);
			return;
		}

		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_CTL_SHOW_SUM:
			show(c);
			break;
		case IMSG_CTL_SESSION:
			show_sessions(c);
			break;
		case IMSG_CTL_RDR_DISABLE:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(id))
				fatalx("invalid imsg header len");
			memcpy(&id, imsg.data, sizeof(id));
			if (disable_rdr(c, &id))
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
			else {
				memcpy(imsg.data, &id, sizeof(id));
				control_imsg_forward(&imsg);
				imsg_compose_event(&c->iev, IMSG_CTL_OK,
				    0, 0, -1, NULL, 0);
			}
			break;
		case IMSG_CTL_RDR_ENABLE:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(id))
				fatalx("invalid imsg header len");
			memcpy(&id, imsg.data, sizeof(id));
			if (enable_rdr(c, &id))
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
			else {
				memcpy(imsg.data, &id, sizeof(id));
				control_imsg_forward(&imsg);
				imsg_compose_event(&c->iev, IMSG_CTL_OK,
				    0, 0, -1, NULL, 0);
			}
			break;
		case IMSG_CTL_TABLE_DISABLE:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(id))
				fatalx("invalid imsg header len");
			memcpy(&id, imsg.data, sizeof(id));
			if (disable_table(c, &id))
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
			else {
				memcpy(imsg.data, &id, sizeof(id));
				control_imsg_forward(&imsg);
				imsg_compose_event(&c->iev, IMSG_CTL_OK,
				    0, 0, -1, NULL, 0);
			}
			break;
		case IMSG_CTL_TABLE_ENABLE:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(id))
				fatalx("invalid imsg header len");
			memcpy(&id, imsg.data, sizeof(id));
			if (enable_table(c, &id))
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
			else {
				memcpy(imsg.data, &id, sizeof(id));
				control_imsg_forward(&imsg);
				imsg_compose_event(&c->iev, IMSG_CTL_OK,
				    0, 0, -1, NULL, 0);
			}
			break;
		case IMSG_CTL_HOST_DISABLE:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(id))
				fatalx("invalid imsg header len");
			memcpy(&id, imsg.data, sizeof(id));
			if (disable_host(c, &id, NULL))
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
			else {
				memcpy(imsg.data, &id, sizeof(id));
				control_imsg_forward(&imsg);
				imsg_compose_event(&c->iev, IMSG_CTL_OK,
				    0, 0, -1, NULL, 0);
			}
			break;
		case IMSG_CTL_HOST_ENABLE:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(id))
				fatalx("invalid imsg header len");
			memcpy(&id, imsg.data, sizeof(id));
			if (enable_host(c, &id, NULL))
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
			else {
				memcpy(imsg.data, &id, sizeof(id));
				control_imsg_forward(&imsg);
				imsg_compose_event(&c->iev, IMSG_CTL_OK,
				    0, 0, -1, NULL, 0);
			}
			break;
		case IMSG_CTL_SHUTDOWN:
			imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
			    0, 0, -1, NULL, 0);
			break;
		case IMSG_CTL_POLL:
			proc_compose_imsg(env->sc_ps, PROC_HCE, -1,
			    IMSG_CTL_POLL, -1, NULL, 0);
			imsg_compose_event(&c->iev, IMSG_CTL_OK,
			    0, 0, -1, NULL, 0);
			break;
		case IMSG_CTL_RELOAD:
			if (env->sc_prefork_relay > 0) {
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
				break;
			}
			proc_compose_imsg(env->sc_ps, PROC_PARENT, -1, IMSG_CTL_RELOAD,
			    -1, NULL, 0);
			/*
			 * we unconditionnaly return a CTL_OK imsg because
			 * we have no choice.
			 *
			 * so in this case, the reply relayctl gets means
			 * that the reload command has been set,
			 * it doesn't say whether the command succeeded or not.
			 */
			imsg_compose_event(&c->iev, IMSG_CTL_OK,
			    0, 0, -1, NULL, 0);
			break;
		case IMSG_CTL_NOTIFY:
			if (c->flags & CTL_CONN_NOTIFY) {
				log_debug("%s: "
				    "client requested notify more than once",
				    __func__);
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
				break;
			}
			c->flags |= CTL_CONN_NOTIFY;
			break;
		case IMSG_CTL_VERBOSE:
			IMSG_SIZE_CHECK(&imsg, &verbose);

			memcpy(&verbose, imsg.data, sizeof(verbose));

			proc_forward_imsg(env->sc_ps, &imsg, PROC_PARENT, 0);
			proc_forward_imsg(env->sc_ps, &imsg, PROC_HCE, 0);
			proc_forward_imsg(env->sc_ps, &imsg, PROC_RELAY, -1);

			memcpy(imsg.data, &verbose, sizeof(verbose));
			control_imsg_forward(&imsg);
			log_verbose(verbose);
			break;
		default:
			log_debug("%s: error handling imsg %d",
			    __func__, imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}

	imsg_event_add(&c->iev);
}

void
control_imsg_forward(struct imsg *imsg)
{
	struct ctl_conn *c;

	TAILQ_FOREACH(c, &ctl_conns, entry)
		if (c->flags & CTL_CONN_NOTIFY)
			imsg_compose_event(&c->iev, imsg->hdr.type,
			    0, imsg->hdr.pid, -1, imsg->data,
			    imsg->hdr.len - IMSG_HEADER_SIZE);
}

void
socket_set_blockmode(int fd, enum blockmodes bm)
{
	int	flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		fatal("fcntl F_GETFL");

	if (bm == BM_NONBLOCK)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if ((flags = fcntl(fd, F_SETFL, flags)) == -1)
		fatal("fcntl F_SETFL");
}
