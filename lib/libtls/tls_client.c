/* $OpenBSD: tls_client.c,v 1.27 2015/09/11 12:56:55 beck Exp $ */
/*
 * Copyright (c) 2014 Joel Sing <jsing@openbsd.org>
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
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/x509.h>

#include <tls.h>
#include "tls_internal.h"

struct tls *
tls_client(void)
{
	struct tls *ctx;

	if ((ctx = tls_new()) == NULL)
		return (NULL);

	ctx->flags |= TLS_CLIENT;

	return (ctx);
}

static int
tls_connect_host(struct tls *ctx, const char *host, const char *port,
    int af, int flag)
{
	struct addrinfo hints, *res, *res0;
	int s = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = af;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = flag;

	if ((s = getaddrinfo(host, port, &hints, &res0)) != 0) {
		tls_set_error(ctx, "%s", gai_strerror(s));
		return (-1);
	}
	for (res = res0; res; res = res->ai_next) {
		s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (s == -1) {
			tls_set_error(ctx, "socket");
			continue;
		}
		if (connect(s, res->ai_addr, res->ai_addrlen) == -1) {
			tls_set_error(ctx, "connect");
			close(s);
			s = -1;
			continue;
		}

		break;  /* Connected. */
	}
	freeaddrinfo(res0);

	return (s);
}

int
tls_connect(struct tls *ctx, const char *host, const char *port)
{
	return tls_connect_servername(ctx, host, port, NULL);
}

int
tls_connect_servername(struct tls *ctx, const char *host, const char *port,
    const char *servername)
{
	const char *h = NULL, *p = NULL;
	char *hs = NULL, *ps = NULL;
	int rv = -1, s = -1, ret;

	if ((ctx->flags & TLS_CLIENT) == 0) {
		tls_set_errorx(ctx, "not a client context");
		goto err;
	}

	if (host == NULL) {
		tls_set_errorx(ctx, "host not specified");
		goto err;
	}

	/*
	 * If port is NULL try to extract a port from the specified host,
	 * otherwise use the default.
	 */
	if ((p = (char *)port) == NULL) {
		ret = tls_host_port(host, &hs, &ps);
		if (ret == -1) {
			tls_set_errorx(ctx, "memory allocation failure");
			goto err;
		}
		if (ret != 0) {
			tls_set_errorx(ctx, "no port provided");
			goto err;
		}
	}

	h = (hs != NULL) ? hs : host;
	p = (ps != NULL) ? ps : port;

	/*
	 * First check if the host is specified as a numeric IP address,
	 * either IPv4 or IPv6, before trying to resolve the host.
	 * The AI_ADDRCONFIG resolver option will not return IPv4 or IPv6
	 * records if it is not configured on an interface;  not considering
	 * loopback addresses.  Checking the numeric addresses first makes
	 * sure that connection attempts to numeric addresses and especially
	 * 127.0.0.1 or ::1 loopback addresses are always possible.
	 */
	if ((s = tls_connect_host(ctx, h, p, AF_INET, AI_NUMERICHOST)) == -1 &&
	    (s = tls_connect_host(ctx, h, p, AF_INET6, AI_NUMERICHOST)) == -1 &&
	    (s = tls_connect_host(ctx, h, p, AF_UNSPEC, AI_ADDRCONFIG)) == -1)
		goto err;

	if (servername == NULL)
		servername = h;

	if (tls_connect_socket(ctx, s, servername) != 0) {
		close(s);
		goto err;
	}

	ctx->socket = s;

	rv = 0;

 err:
	free(hs);
	free(ps);

	return (rv);
}

int
tls_connect_socket(struct tls *ctx, int s, const char *servername)
{
	return tls_connect_fds(ctx, s, s, servername);
}

int
tls_connect_fds(struct tls *ctx, int fd_read, int fd_write,
    const char *servername)
{
	union { struct in_addr ip4; struct in6_addr ip6; } addrbuf;
	int rv = -1;

	if ((ctx->flags & TLS_CLIENT) == 0) {
		tls_set_errorx(ctx, "not a client context");
		goto err;
	}

	if (fd_read < 0 || fd_write < 0) {
		tls_set_errorx(ctx, "invalid file descriptors");
		goto err;
	}

	if (servername != NULL) {
		if ((ctx->servername = strdup(servername)) == NULL) {
			tls_set_errorx(ctx, "out of memory");
			goto err;
		}
	}

	if ((ctx->ssl_ctx = SSL_CTX_new(SSLv23_client_method())) == NULL) {
		tls_set_errorx(ctx, "ssl context failure");
		goto err;
	}

	if (tls_configure_ssl(ctx) != 0)
		goto err;
	if (tls_configure_keypair(ctx, 0) != 0)
		goto err;

	if (ctx->config->verify_name) {
		if (servername == NULL) {
			tls_set_errorx(ctx, "server name not specified");
			goto err;
		}
	}

	if (ctx->config->verify_cert &&
	    (tls_configure_ssl_verify(ctx, SSL_VERIFY_PEER) == -1))
		goto err;

	if ((ctx->ssl_conn = SSL_new(ctx->ssl_ctx)) == NULL) {
		tls_set_errorx(ctx, "ssl connection failure");
		goto err;
	}
	if (SSL_set_app_data(ctx->ssl_conn, ctx) != 1) {
		tls_set_errorx(ctx, "ssl application data failure");
		goto err;
	}
	if (SSL_set_rfd(ctx->ssl_conn, fd_read) != 1 ||
	    SSL_set_wfd(ctx->ssl_conn, fd_write) != 1) {
		tls_set_errorx(ctx, "ssl file descriptor failure");
		goto err;
	}

	/*
	 * RFC4366 (SNI): Literal IPv4 and IPv6 addresses are not
	 * permitted in "HostName".
	 */
	if (servername != NULL &&
	    inet_pton(AF_INET, servername, &addrbuf) != 1 &&
	    inet_pton(AF_INET6, servername, &addrbuf) != 1) {
		if (SSL_set_tlsext_host_name(ctx->ssl_conn, servername) == 0) {
			tls_set_errorx(ctx, "server name indication failure");
			goto err;
		}
	}

	rv = 0;

 err:
	return (rv);
}

int
tls_handshake_client(struct tls *ctx)
{
	X509 *cert = NULL;
	int ssl_ret;
	int rv = -1;

	if ((ctx->flags & TLS_CLIENT) == 0) {
		tls_set_errorx(ctx, "not a client context");
		goto err;
	}

	if ((ssl_ret = SSL_connect(ctx->ssl_conn)) != 1) {
		rv = tls_ssl_error(ctx, ctx->ssl_conn, ssl_ret, "handshake");
		goto err;
	}

	if (ctx->config->verify_name) {
		cert = SSL_get_peer_certificate(ctx->ssl_conn);
		if (cert == NULL) {
			tls_set_errorx(ctx, "no server certificate");
			goto err;
		}
		if ((rv = tls_check_name(ctx, cert,
		    ctx->servername)) != 0) {
			if (rv != -2)
				tls_set_errorx(ctx, "name `%s' not present in"
				    " server certificate", ctx->servername);
			goto err;
		}
	}

	ctx->state |= TLS_HANDSHAKE_COMPLETE;
	rv = 0;

 err:
	X509_free(cert);

	return (rv);
}
