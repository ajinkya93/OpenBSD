/* $OpenBSD: ressl_client.c,v 1.5 2014/10/03 14:14:40 tedu Exp $ */
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

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/x509.h>

#include <ressl.h>
#include "ressl_internal.h"

struct ressl *
ressl_client(void)
{
	struct ressl *ctx;

	if ((ctx = ressl_new()) == NULL)
		return (NULL);

	ctx->flags |= RESSL_CLIENT;

	return (ctx);
}

int
ressl_connect(struct ressl *ctx, const char *host, const char *port)
{
	struct addrinfo hints, *res, *res0;
	const char *h = NULL, *p = NULL;
	char *hs = NULL, *ps = NULL;
	int rv = -1, s = -1, ret;

	if ((ctx->flags & RESSL_CLIENT) == 0) {
		ressl_set_error(ctx, "not a client context");
		goto err;
	}

	if (host == NULL) {
		ressl_set_error(ctx, "host not specified");
		goto err;
	}

	/*
	 * If port is NULL try to extract a port from the specified host,
	 * otherwise use the default.
	 */
	if ((p = (char *)port) == NULL) {
		ret = ressl_host_port(host, &hs, &ps);
		if (ret == -1) {
			ressl_set_error(ctx, "memory allocation failure");
			goto err;
		}
		if (ret != 0)
			port = HTTPS_PORT;
	}

	h = (hs != NULL) ? hs : host;
	p = (ps != NULL) ? ps : port;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(h, p, &hints, &res0)) != 0) {
		ressl_set_error(ctx, "%s", gai_strerror(ret));
		goto err;
	}
	for (res = res0; res; res = res->ai_next) {
		s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (s == -1) {
			ressl_set_error(ctx, "socket");
			continue;
		}
		if (connect(s, res->ai_addr, res->ai_addrlen) == -1) {
			ressl_set_error(ctx, "connect");
			close(s);
			s = -1;
			continue;
		}

		break;  /* Connected. */
	}
	freeaddrinfo(res0);

	if (s == -1)
		goto err;

	if (ressl_connect_socket(ctx, s, h) != 0) {
		close(s);
		goto err;
	}

	rv = 0;

err:

	free(hs);
	free(ps);

	return (rv);
}

int
ressl_connect_socket(struct ressl *ctx, int socket, const char *hostname)
{
	union { struct in_addr ip4; struct in6_addr ip6; } addrbuf;
	X509 *cert = NULL;
	int ret;

	if ((ctx->flags & RESSL_CLIENT) == 0) {
		ressl_set_error(ctx, "not a client context");
		goto err;
	}

	ctx->socket = socket;

	if ((ctx->ssl_ctx = SSL_CTX_new(SSLv23_client_method())) == NULL) {
		ressl_set_error(ctx, "ssl context failure");
		goto err;
	}

	if (ressl_configure_ssl(ctx) != 0)
		goto err;

	if (ctx->config->verify_host) {
		if (hostname == NULL) {
			ressl_set_error(ctx, "server name not specified");
			goto err;
		}
	}

	if (ctx->config->verify_cert) {
		SSL_CTX_set_verify(ctx->ssl_ctx, SSL_VERIFY_PEER, NULL);

		if (SSL_CTX_load_verify_locations(ctx->ssl_ctx,
		    ctx->config->ca_file, ctx->config->ca_path) != 1) {
			ressl_set_error(ctx, "ssl verify setup failure");
			goto err;
		}
		if (ctx->config->verify_depth >= 0)
			SSL_CTX_set_verify_depth(ctx->ssl_ctx,
			    ctx->config->verify_depth);
	}

	if ((ctx->ssl_conn = SSL_new(ctx->ssl_ctx)) == NULL) {
		ressl_set_error(ctx, "ssl connection failure");
		goto err;
	}
	if (SSL_set_fd(ctx->ssl_conn, ctx->socket) != 1) {
		ressl_set_error(ctx, "ssl file descriptor failure");
		goto err;
	}

	/*
	 * RFC4366 (SNI): Literal IPv4 and IPv6 addresses are not
	 * permitted in "HostName".
	 */
	if (hostname != NULL &&
	    inet_pton(AF_INET, hostname, &addrbuf) != 1 &&
	    inet_pton(AF_INET6, hostname, &addrbuf) != 1) {
		if (SSL_set_tlsext_host_name(ctx->ssl_conn, hostname) == 0) {
			ressl_set_error(ctx, "SNI host name failed");
			goto err;
		}
	}

	if ((ret = SSL_connect(ctx->ssl_conn)) != 1) {
		ressl_set_error(ctx, "SSL connect failed: %i",
		    SSL_get_error(ctx->ssl_conn, ret));
		goto err;
	}

	if (ctx->config->verify_host) {
		cert = SSL_get_peer_certificate(ctx->ssl_conn);
		if (cert == NULL) {
			ressl_set_error(ctx, "no server certificate");
			goto err;
		}
		if (ressl_check_hostname(cert, hostname) != 0) {
			ressl_set_error(ctx, "host `%s' not present in"
			    " server certificate", hostname);
			goto err;
		}
	}

	return (0);

err:
	X509_free(cert);

	return (-1);
}
