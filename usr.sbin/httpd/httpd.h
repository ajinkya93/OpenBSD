/*	$OpenBSD: httpd.h,v 1.42 2014/08/04 18:12:15 reyk Exp $	*/

/*
 * Copyright (c) 2006 - 2014 Reyk Floeter <reyk@openbsd.org>
 * Copyright (c) 2006, 2007 Pierre-Yves Ritschard <pyr@openbsd.org>
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

#ifndef _HTTPD_H
#define _HTTPD_H

#include <sys/tree.h>

#include <sys/param.h>		/* MAXHOSTNAMELEN */
#include <limits.h>
#include <imsg.h>
#include <ressl.h>

#define CONF_FILE		"/etc/httpd.conf"
#define HTTPD_SOCKET		"/var/run/httpd.sock"
#define HTTPD_USER		"www"
#define HTTPD_SERVERNAME	"OpenBSD httpd"
#define HTTPD_DOCROOT		"/htdocs"
#define HTTPD_INDEX		"index.html"
#define HTTPD_FCGI_SOCKET	"/run/slowcgi.sock"
#define HTTPD_ACCESS_LOG	"/logs/access.log"
#define HTTPD_ERROR_LOG		"/logs/error.log"
#define HTTPD_SSL_KEY		"/conf/server.key"
#define HTTPD_SSL_CERT		"/conf/server.crt"
#define FD_RESERVE		5

#define SERVER_MAX_CLIENTS	1024
#define SERVER_TIMEOUT		600
#define SERVER_CACHESIZE	-1	/* use default size */
#define SERVER_NUMPROC		3
#define SERVER_MAXPROC		32
#define SERVER_MAXHEADERLENGTH	8192
#define SERVER_BACKLOG		10
#define SERVER_OUTOF_FD_RETRIES	5

#define MEDIATYPE_NAMEMAX	128	/* file name extension */
#define MEDIATYPE_TYPEMAX	64	/* length of type/subtype */

#define CONFIG_RELOAD		0x00
#define CONFIG_MEDIA		0x01
#define CONFIG_SERVERS		0x02
#define CONFIG_ALL		0xff

enum httpchunk {
	TOREAD_UNLIMITED		= -1,
	TOREAD_HTTP_HEADER		= -2,
	TOREAD_HTTP_CHUNK_LENGTH	= -3,
	TOREAD_HTTP_CHUNK_TRAILER	= -4,
	TOREAD_HTTP_NONE		= -5
};

#if DEBUG
#define DPRINTF		log_debug
#else
#define DPRINTF(x...)	do {} while(0)
#endif

struct ctl_flags {
	u_int8_t	 cf_opts;
	u_int32_t	 cf_flags;
};

enum key_type {
	KEY_TYPE_NONE		= 0,
	KEY_TYPE_COOKIE,
	KEY_TYPE_HEADER,
	KEY_TYPE_PATH,
	KEY_TYPE_QUERY,
	KEY_TYPE_URL,
	KEY_TYPE_MAX
};

TAILQ_HEAD(kvlist, kv);
RB_HEAD(kvtree, kv);

struct kv {
	char			*kv_key;
	char			*kv_value;

	enum key_type		 kv_type;

#define KV_FLAG_INVALID		 0x01
#define KV_FLAG_GLOBBING	 0x02
	u_int8_t		 kv_flags;

	struct kvlist		 kv_children;
	struct kv		*kv_parent;
	TAILQ_ENTRY(kv)		 kv_entry;

	RB_ENTRY(kv)		 kv_node;
};

struct portrange {
	in_port_t		 val[2];
	u_int8_t		 op;
};

struct address {
	struct sockaddr_storage	 ss;
	int			 ipproto;
	int			 prefixlen;
	struct portrange	 port;
	char			 ifname[IFNAMSIZ];
	TAILQ_ENTRY(address)	 entry;
};
TAILQ_HEAD(addresslist, address);

/* initially control.h */
struct control_sock {
	const char	*cs_name;
	struct event	 cs_ev;
	struct event	 cs_evt;
	int		 cs_fd;
	int		 cs_restricted;
	void		*cs_env;

	TAILQ_ENTRY(control_sock) cs_entry;
};
TAILQ_HEAD(control_socks, control_sock);

struct {
	struct event	 ev;
	int		 fd;
} control_state;

enum blockmodes {
	BM_NORMAL,
	BM_NONBLOCK
};

struct imsgev {
	struct imsgbuf		 ibuf;
	void			(*handler)(int, short, void *);
	struct event		 ev;
	struct privsep_proc	*proc;
	void			*data;
	short			 events;
};

#define IMSG_SIZE_CHECK(imsg, p) do {				\
	if (IMSG_DATA_SIZE(imsg) < sizeof(*p))			\
		fatalx("bad length imsg received");		\
} while (0)
#define IMSG_DATA_SIZE(imsg)	((imsg)->hdr.len - IMSG_HEADER_SIZE)

struct ctl_conn {
	TAILQ_ENTRY(ctl_conn)	 entry;
	u_int8_t		 flags;
	u_int			 waiting;
#define CTL_CONN_NOTIFY		 0x01
	struct imsgev		 iev;

};
TAILQ_HEAD(ctl_connlist, ctl_conn);

enum imsg_type {
	IMSG_NONE,
	IMSG_CTL_OK,
	IMSG_CTL_FAIL,
	IMSG_CTL_VERBOSE,
	IMSG_CTL_RESET,
	IMSG_CTL_SHUTDOWN,
	IMSG_CTL_RELOAD,
	IMSG_CTL_NOTIFY,
	IMSG_CTL_END,
	IMSG_CTL_START,
	IMSG_CTL_REOPEN,
	IMSG_CFG_SERVER,
	IMSG_CFG_MEDIA,
	IMSG_CFG_DONE,
	IMSG_LOG_ACCESS,
	IMSG_LOG_ERROR
};

enum privsep_procid {
	PROC_ALL	= -1,
	PROC_PARENT	= 0,
	PROC_SERVER,
	PROC_LOGGER,
	PROC_MAX
} privsep_process;

/* Attach the control socket to the following process */
#define PROC_CONTROL	PROC_LOGGER

struct privsep_pipes {
	int				*pp_pipes[PROC_MAX];
};

struct privsep {
	struct privsep_pipes		*ps_pipes[PROC_MAX];
	struct privsep_pipes		*ps_pp;

	struct imsgev			*ps_ievs[PROC_MAX];
	const char			*ps_title[PROC_MAX];
	pid_t				 ps_pid[PROC_MAX];
	u_int8_t			 ps_what[PROC_MAX];

	u_int				 ps_instances[PROC_MAX];
	u_int				 ps_ninstances;
	u_int				 ps_instance;

	struct control_sock		 ps_csock;
	struct control_socks		 ps_rcsocks;

	/* Event and signal handlers */
	struct event			 ps_evsigint;
	struct event			 ps_evsigterm;
	struct event			 ps_evsigchld;
	struct event			 ps_evsighup;
	struct event			 ps_evsigpipe;
	struct event			 ps_evsigusr1;

	int				 ps_noaction;
	struct passwd			*ps_pw;
	struct httpd			*ps_env;
};

struct privsep_proc {
	const char		*p_title;
	enum privsep_procid	 p_id;
	int			(*p_cb)(int, struct privsep_proc *,
				    struct imsg *);
	pid_t			(*p_init)(struct privsep *,
				    struct privsep_proc *);
	void			(*p_shutdown)(void);
	u_int			 p_instance;
	const char		*p_chroot;
	struct privsep		*p_ps;
	struct httpd		*p_env;
};

enum fcgistate {
	FCGI_READ_HEADER,
	FCGI_READ_CONTENT,
	FCGI_READ_PADDING
};

struct client {
	u_int32_t		 clt_id;
	pid_t			 clt_pid;
	void			*clt_srv;
	void			*clt_srv_conf;
	u_int32_t		 clt_srv_id;
	struct sockaddr_storage	 clt_srv_ss;

	int			 clt_s;
	in_port_t		 clt_port;
	struct sockaddr_storage	 clt_ss;
	struct bufferevent	*clt_bev;
	char			*clt_buf;
	size_t			 clt_buflen;
	struct evbuffer		*clt_output;
	struct event		 clt_ev;
	void			*clt_desc;

	int			 clt_fd;
	struct ressl		*clt_ressl_ctx;
	struct bufferevent	*clt_srvbev;

	off_t			 clt_toread;
	size_t			 clt_headerlen;
	int			 clt_persist;
	int			 clt_line;
	int			 clt_done;
	int			 clt_chunk;
	int			 clt_inflight;
	enum fcgistate		 clt_fcgi_state;
	int			 clt_fcgi_toread;
	int			 clt_fcgi_padding_len;
	int			 clt_fcgi_type;
	struct evbuffer		*clt_srvevb;

	struct evbuffer		*clt_log;
	struct timeval		 clt_timeout;
	struct timeval		 clt_tv_start;
	struct timeval		 clt_tv_last;
	struct event		 clt_inflightevt;

	SPLAY_ENTRY(client)	 clt_nodes;
};
SPLAY_HEAD(client_tree, client);

#define SRVFLAG_INDEX		0x0001
#define SRVFLAG_NO_INDEX	0x0002
#define SRVFLAG_AUTO_INDEX	0x0004
#define SRVFLAG_NO_AUTO_INDEX	0x0008
#define SRVFLAG_ROOT		0x0010
#define SRVFLAG_LOCATION	0x0020
#define SRVFLAG_FCGI		0x0040
#define SRVFLAG_NO_FCGI		0x0080
#define SRVFLAG_LOG		0x0100
#define SRVFLAG_NO_LOG		0x0200
#define SRVFLAG_SOCKET		0x0400
#define SRVFLAG_SYSLOG		0x0800
#define SRVFLAG_NO_SYSLOG	0x1000
#define SRVFLAG_SSL		0x2000

#define SRVFLAG_BITS							\
	"\10\01INDEX\02NO_INDEX\03AUTO_INDEX\04NO_AUTO_INDEX"		\
	"\05ROOT\06LOCATION\07FCGI\10NO_FCGI\11LOG\12NO_LOG\13SOCKET"	\
	"\14SYSLOG\15NO_SYSLOG"

#define TCPFLAG_NODELAY		0x01
#define TCPFLAG_NNODELAY	0x02
#define TCPFLAG_SACK		0x04
#define TCPFLAG_NSACK		0x08
#define TCPFLAG_BUFSIZ		0x10
#define TCPFLAG_IPTTL		0x20
#define TCPFLAG_IPMINTTL	0x40
#define TCPFLAG_NSPLICE		0x80
#define TCPFLAG_DEFAULT		0x00

#define TCPFLAG_BITS						\
	"\10\01NODELAY\02NO_NODELAY\03SACK\04NO_SACK"		\
	"\05SOCKET_BUFFER_SIZE\06IP_TTL\07IP_MINTTL\10NO_SPLICE"

enum log_format {
	LOG_FORMAT_COMMON,
	LOG_FORMAT_COMBINED,
	LOG_FORMAT_CONNECTION
};

struct server_config {
	u_int32_t		 id;
	char			 name[MAXHOSTNAMELEN];
	char			 location[NAME_MAX];
	char			 index[NAME_MAX];
	char			 root[MAXPATHLEN];
	char			 socket[MAXPATHLEN];

	in_port_t		 port;
	struct sockaddr_storage	 ss;
	int			 prefixlen;
	struct timeval		 timeout;

	u_int16_t		 flags;
	u_int8_t		 tcpflags;
	int			 tcpbufsiz;
	int			 tcpbacklog;
	u_int8_t		 tcpipttl;
	u_int8_t		 tcpipminttl;

	enum log_format		 logformat;

	TAILQ_ENTRY(server_config) entry;
};
TAILQ_HEAD(serverhosts, server_config);

struct server {
	TAILQ_ENTRY(server)	 srv_entry;
	struct server_config	 srv_conf;
	struct serverhosts	 srv_hosts;

	int			 srv_s;
	struct event		 srv_ev;
	struct event		 srv_evt;

	struct ressl		 *srv_ressl_ctx;
	struct ressl_config	 *srv_ressl_config;

	struct client_tree	 srv_clients;
};
TAILQ_HEAD(serverlist, server);

struct media_type {
	char			 media_name[MEDIATYPE_NAMEMAX];
	char			 media_type[MEDIATYPE_TYPEMAX];
	char			 media_subtype[MEDIATYPE_TYPEMAX];
	char			*media_encoding;
	RB_ENTRY(media_type)	 media_entry;
};
RB_HEAD(mediatypes, media_type);

struct httpd {
	u_int8_t		 sc_opts;
	u_int32_t		 sc_flags;
	const char		*sc_conffile;
	struct event		 sc_ev;
	u_int16_t		 sc_prefork_server;
	u_int16_t		 sc_id;
	int			 sc_paused;

	struct serverlist	*sc_servers;
	struct mediatypes	*sc_mediatypes;

	struct privsep		*sc_ps;
	int			 sc_reload;
};

#define HTTPD_OPT_VERBOSE		0x01
#define HTTPD_OPT_NOACTION		0x04

/* control.c */
int	 control_init(struct privsep *, struct control_sock *);
int	 control_listen(struct control_sock *);
void	 control_cleanup(struct control_sock *);
void	 control_dispatch_imsg(int, short, void *);
void	 control_imsg_forward(struct imsg *);
struct ctl_conn	*
	 control_connbyfd(int);
void	 socket_set_blockmode(int, enum blockmodes);

extern  struct ctl_connlist ctl_conns;

/* parse.y */
int	 parse_config(const char *, struct httpd *);
int	 load_config(const char *, struct httpd *);
int	 cmdline_symset(char *);

/* server.c */
pid_t	 server(struct privsep *, struct privsep_proc *);
int	 server_privinit(struct server *);
void	 server_purge(struct server *);
int	 server_socket_af(struct sockaddr_storage *, in_port_t);
in_port_t
	 server_socket_getport(struct sockaddr_storage *);
int	 server_socket_connect(struct sockaddr_storage *, in_port_t,
	    struct server_config *);
void	 server_write(struct bufferevent *, void *);
void	 server_read(struct bufferevent *, void *);
void	 server_error(struct bufferevent *, short, void *);
void	 server_log(struct client *, const char *);
void	 server_log_access(const char *, ...)
	    __attribute__((__format__ (printf, 1, 2)));
void	 server_log_error(const char *, ...)
	    __attribute__((__format__ (printf, 1, 2)));
void	 server_close(struct client *, const char *);
void	 server_dump(struct client *, const void *, size_t);
int	 server_client_cmp(struct client *, struct client *);
int	 server_bufferevent_print(struct client *, const char *);
int	 server_bufferevent_write_buffer(struct client *,
	    struct evbuffer *);
int	 server_bufferevent_write_chunk(struct client *,
	    struct evbuffer *, size_t);
int	 server_bufferevent_add(struct event *, int);
int	 server_bufferevent_write(struct client *, void *, size_t);
void	 server_inflight_dec(struct client *, const char *);
struct server *
	 server_byaddr(struct sockaddr *, in_port_t);

SPLAY_PROTOTYPE(client_tree, client, clt_nodes, server_client_cmp);

/* server_http.c */
void	 server_http_init(struct server *);
void	 server_http(struct httpd *);
int	 server_httpdesc_init(struct client *);
void	 server_read_http(struct bufferevent *, void *);
void	 server_abort_http(struct client *, u_int, const char *);
u_int	 server_httpmethod_byname(const char *);
const char
	*server_httpmethod_byid(u_int);
const char
	*server_httperror_byid(u_int);
void	 server_read_httpcontent(struct bufferevent *, void *);
void	 server_read_httpchunks(struct bufferevent *, void *);
int	 server_writeheader_http(struct client *clt, struct kv *, void *);
int	 server_headers(struct client *,
	    int (*)(struct client *, struct kv *, void *), void *);
int	 server_writeresponse_http(struct client *);
int	 server_response_http(struct client *, u_int, struct media_type *,
	    size_t);
void	 server_reset_http(struct client *);
void	 server_close_http(struct client *);
int	 server_response(struct httpd *, struct client *);
const char *
	 server_http_host(struct sockaddr_storage *, char *, size_t);
void	 server_http_date(char *, size_t);
int	 server_log_http(struct client *, u_int, size_t);

/* server_file.c */
int	 server_file(struct httpd *, struct client *);
void	 server_file_error(struct bufferevent *, short, void *);

/* server_fcgi.c */
int	 server_fcgi(struct httpd *, struct client *);

/* httpd.c */
void		 event_again(struct event *, int, short,
		    void (*)(int, short, void *),
		    struct timeval *, struct timeval *, void *);
const char	*canonicalize_host(const char *, char *, size_t);
const char	*canonicalize_path(const char *, char *, size_t);
ssize_t		 path_info(char *);
void		 imsg_event_add(struct imsgev *);
int		 imsg_compose_event(struct imsgev *, u_int16_t, u_int32_t,
		    pid_t, int, void *, u_int16_t);
void		 socket_rlimit(int);
char		*get_string(u_int8_t *, size_t);
void		*get_data(u_int8_t *, size_t);
int		 sockaddr_cmp(struct sockaddr *, struct sockaddr *, int);
struct in6_addr *prefixlen2mask6(u_int8_t, u_int32_t *);
u_int32_t	 prefixlen2mask(u_int8_t);
int		 accept_reserve(int, struct sockaddr *, socklen_t *, int,
		    volatile int *);
struct kv	*kv_add(struct kvtree *, char *, char *);
int		 kv_set(struct kv *, char *, ...);
int		 kv_setkey(struct kv *, char *, ...);
void		 kv_delete(struct kvtree *, struct kv *);
struct kv	*kv_extend(struct kvtree *, struct kv *, char *);
void		 kv_purge(struct kvtree *);
void		 kv_free(struct kv *);
struct kv	*kv_inherit(struct kv *, struct kv *);
int		 kv_log(struct evbuffer *, struct kv *);
struct kv	*kv_find(struct kvtree *, struct kv *);
int		 kv_cmp(struct kv *, struct kv *);
struct media_type
		*media_add(struct mediatypes *, struct media_type *);
void		 media_delete(struct mediatypes *, struct media_type *);
void		 media_purge(struct mediatypes *);
struct media_type *
		 media_find(struct mediatypes *, char *);
int		 media_cmp(struct media_type *, struct media_type *);
RB_PROTOTYPE(kvtree, kv, kv_node, kv_cmp);
RB_PROTOTYPE(mediatypes, media_type, media_entry, media_cmp);

/* log.c */
void	log_init(int);
void	log_verbose(int);
void	log_warn(const char *, ...) __attribute__((__format__ (printf, 1, 2)));
void	log_warnx(const char *, ...) __attribute__((__format__ (printf, 1, 2)));
void	log_info(const char *, ...) __attribute__((__format__ (printf, 1, 2)));
void	log_debug(const char *, ...) __attribute__((__format__ (printf, 1, 2)));
void	vlog(int, const char *, va_list) __attribute__((__format__ (printf, 2, 0)));
__dead void fatal(const char *);
__dead void fatalx(const char *);
const char *print_host(struct sockaddr_storage *, char *, size_t);
const char *print_time(struct timeval *, struct timeval *, char *, size_t);
const char *printb_flags(const u_int32_t, const char *);
void	 getmonotime(struct timeval *);

/* proc.c */
void	 proc_init(struct privsep *, struct privsep_proc *, u_int);
void	 proc_kill(struct privsep *);
void	 proc_listen(struct privsep *, struct privsep_proc *, size_t);
void	 proc_dispatch(int, short event, void *);
pid_t	 proc_run(struct privsep *, struct privsep_proc *,
	    struct privsep_proc *, u_int,
	    void (*)(struct privsep *, struct privsep_proc *, void *), void *);
void	 proc_range(struct privsep *, enum privsep_procid, int *, int *);
int	 proc_compose_imsg(struct privsep *, enum privsep_procid, int,
	    u_int16_t, int, void *, u_int16_t);
int	 proc_composev_imsg(struct privsep *, enum privsep_procid, int,
	    u_int16_t, int, const struct iovec *, int);
int	 proc_forward_imsg(struct privsep *, struct imsg *,
	    enum privsep_procid, int);
struct imsgbuf *
	 proc_ibuf(struct privsep *, enum privsep_procid, int);
struct imsgev *
	 proc_iev(struct privsep *, enum privsep_procid, int);
void	 imsg_event_add(struct imsgev *);
int	 imsg_compose_event(struct imsgev *, u_int16_t, u_int32_t,
	    pid_t, int, void *, u_int16_t);
int	 imsg_composev_event(struct imsgev *, u_int16_t, u_int32_t,
	    pid_t, int, const struct iovec *, int);

/* config.c */
int	 config_init(struct httpd *);
void	 config_purge(struct httpd *, u_int);
int	 config_setreset(struct httpd *, u_int);
int	 config_getreset(struct httpd *, struct imsg *);
int	 config_getcfg(struct httpd *, struct imsg *);
int	 config_setserver(struct httpd *, struct server *);
int	 config_getserver(struct httpd *, struct imsg *);
int	 config_setmedia(struct httpd *, struct media_type *);
int	 config_getmedia(struct httpd *, struct imsg *);

/* logger.c */
pid_t	 logger(struct privsep *, struct privsep_proc *);

#endif /* _HTTPD_H */
