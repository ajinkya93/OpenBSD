/*                      _             _
**  _ __ ___   ___   __| |    ___ ___| |
** | '_ ` _ \ / _ \ / _` |   / __/ __| |
** | | | | | | (_) | (_| |   \__ \__ \ | mod_ssl - Apache Interface to SSLeay
** |_| |_| |_|\___/ \__,_|___|___/___/_| http://www.engelschall.com/sw/mod_ssl/
**                      |_____|
**  ssl_engine_ext.c
**  Extensions to other Apache parts
*/

/* ====================================================================
 * Copyright (c) 1998-1999 Ralf S. Engelschall. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by
 *     Ralf S. Engelschall <rse@engelschall.com> for use in the
 *     mod_ssl project (http://www.engelschall.com/sw/mod_ssl/)."
 *
 * 4. The names "mod_ssl" must not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission. For written permission, please contact
 *    rse@engelschall.com.
 *
 * 5. Products derived from this software may not be called "mod_ssl"
 *    nor may "mod_ssl" appear in their names without prior
 *    written permission of Ralf S. Engelschall.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by
 *     Ralf S. Engelschall <rse@engelschall.com> for use in the
 *     mod_ssl project (http://www.engelschall.com/sw/mod_ssl/)."
 *
 * THIS SOFTWARE IS PROVIDED BY RALF S. ENGELSCHALL ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL RALF S. ENGELSCHALL OR
 * HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */
                             /* ``Only those who attempt the absurd
                                  can achieve the impossible.''
                                           -- Unknown             */
#include "mod_ssl.h"


/*  _________________________________________________________________
**
**  SSL Extensions
**  _________________________________________________________________
*/

static void  ssl_ext_mlc_register(void);
static void  ssl_ext_mlc_unregister(void);
static void  ssl_ext_mr_register(void);
static void  ssl_ext_mr_unregister(void);
static void  ssl_ext_mp_register(void);
static void  ssl_ext_mp_unregister(void);

void ssl_ext_register(void)
{
    ssl_ext_mlc_register();
    ssl_ext_mr_register();
    ssl_ext_mp_register();
    return;
}

void ssl_ext_unregister(void)
{
    ssl_ext_mlc_unregister();
    ssl_ext_mr_unregister();
    ssl_ext_mp_unregister();
    return;
}

/*  _________________________________________________________________
**
**  SSL Extension to mod_log_config
**  _________________________________________________________________
*/

static char *ssl_ext_mlc_log_c(request_rec *r, char *a);
static char *ssl_ext_mlc_log_x(request_rec *r, char *a);

/*
 * register us for the mod_log_config function registering phase
 * to establish %{...}c and to be able to expand %{...}x variables.
 */
static void ssl_ext_mlc_register(void)
{
    ap_hook_register("ap::mod_log_config::log_c",
                     ssl_ext_mlc_log_c, AP_HOOK_NOCTX);
    ap_hook_register("ap::mod_log_config::log_x",
                     ssl_ext_mlc_log_x, AP_HOOK_NOCTX);
    return;
}

static void ssl_ext_mlc_unregister(void)
{
    ap_hook_unregister("ap::mod_log_config::log_c",
                       ssl_ext_mlc_log_c);
    ap_hook_unregister("ap::mod_log_config::log_x", 
                       ssl_ext_mlc_log_x);
    return;
}

/*
 * implement the %{..}c log function
 * (we are the only function)
 */
static char *ssl_ext_mlc_log_c(request_rec *r, char *a)
{
    char *result;

    if (ap_ctx_get(r->connection->client->ctx, "ssl") == NULL)
        return NULL;
    result = NULL;
    if (strcmp(a, "version") == 0)
        result = ssl_var_lookup(r->pool, r->server, r->connection, r, "SSL_PROTOCOL");
    else if (strcmp(a, "cipher") == 0)
        result = ssl_var_lookup(r->pool, r->server, r->connection, r, "SSL_CIPHER");
    else if (strcmp(a, "subjectdn") == 0 || strcmp(a, "clientcert") == 0)
        result = ssl_var_lookup(r->pool, r->server, r->connection, r, "SSL_CLIENT_S_DN");
    else if (strcmp(a, "issuerdn") == 0 || strcmp(a, "cacert") == 0)
        result = ssl_var_lookup(r->pool, r->server, r->connection, r, "SSL_CLIENT_I_DN");
    else if (strcmp(a, "errcode") == 0)
        result = "-";
    else if (strcmp(a, "errstr") == 0)
        result = ap_ctx_get(r->connection->client->ctx, "ssl::verify::error");
    if (result != NULL && result[0] == NUL)
        result = NULL;
    return result;
}

/*
 * extend the implementation of the %{..}x log function
 * (there can be more functions)
 */
static char *ssl_ext_mlc_log_x(request_rec *r, char *a)
{
    char *result;

    result = NULL;
    if (ap_ctx_get(r->connection->client->ctx, "ssl") != NULL)
        result = ssl_var_lookup(r->pool, r->server, r->connection, r, a);
    if (result != NULL && result[0] == NUL)
        result = NULL;
    return result;
}

/*  _________________________________________________________________
**
**  SSL Extension to mod_rewrite
**  _________________________________________________________________
*/

static char *ssl_ext_mr_lookup_variable(request_rec *r, char *var);

/*
 * register us for the mod_rewrite lookup_variable() function
 */
static void ssl_ext_mr_register(void)
{
    ap_hook_register("ap::mod_rewrite::lookup_variable",
                     ssl_ext_mr_lookup_variable, AP_HOOK_NOCTX);
    return;
}

static void ssl_ext_mr_unregister(void)
{
    ap_hook_unregister("ap::mod_rewrite::lookup_variable", 
                       ssl_ext_mr_lookup_variable);
    return;
}

static char *ssl_ext_mr_lookup_variable(request_rec *r, char *var)
{
    char *val;

    val = ssl_var_lookup(r->pool, r->server, r->connection, r, var);
    if (val[0] == NUL)
        val = NULL;
    return val;
}

/*  _________________________________________________________________
**
**  SSL Extension to mod_proxy
**  _________________________________________________________________
*/

static int   ssl_ext_mp_canon(request_rec *r, char *url);
static int   ssl_ext_mp_handler(request_rec *r, void *cr, char *url, char *proxyhost, int proxyport, char *protocol);
static int   ssl_ext_mp_set_destport(request_rec *r);
static char *ssl_ext_mp_new_connection(request_rec *r, BUFF *fb);
static void  ssl_ext_mp_close_connection(void *_fb);
static int   ssl_ext_mp_write_host_header(request_rec *r, BUFF *fb, char *host, int port, char *portstr);

/*
 * register us ...
 */
static void ssl_ext_mp_register(void)
{
    ap_hook_register("ap::mod_proxy::canon",
                     ssl_ext_mp_canon, AP_HOOK_NOCTX);
    ap_hook_register("ap::mod_proxy::handler",
                     ssl_ext_mp_handler, AP_HOOK_NOCTX);
    ap_hook_register("ap::mod_proxy::http::handler::set_destport",
                     ssl_ext_mp_set_destport, AP_HOOK_NOCTX);
    ap_hook_register("ap::mod_proxy::http::handler::new_connection",
                     ssl_ext_mp_new_connection, AP_HOOK_NOCTX);
    ap_hook_register("ap::mod_proxy::http::handler::write_host_header",
                     ssl_ext_mp_write_host_header, AP_HOOK_NOCTX);
    return;
}

static void ssl_ext_mp_unregister(void)
{
    ap_hook_unregister("ap::mod_proxy::canon", ssl_ext_mp_canon);
    ap_hook_unregister("ap::mod_proxy::handler", ssl_ext_mp_handler);
    ap_hook_unregister("ap::mod_proxy::http::handler::set_destport",
                       ssl_ext_mp_set_destport);
    ap_hook_unregister("ap::mod_proxy::http::handler::new_connection",
                       ssl_ext_mp_new_connection);
    ap_hook_unregister("ap::mod_proxy::http::handler::write_host_header",
                       ssl_ext_mp_write_host_header);
    return;
}

static int ssl_ext_mp_canon(request_rec *r, char *url)
{
    int rc;

    if (strncasecmp(url, "https:", 6) == 0) {
        rc = OK;
        ap_hook_call("ap::mod_proxy::http::canon",
                     &rc, r, url+6, "https", DEFAULT_HTTPS_PORT);
        return rc;
    }
    return DECLINED;
}

static int ssl_ext_mp_handler(
    request_rec *r, void *cr, char *url, char *proxyhost, int proxyport, char *protocol)
{
    int rc;

    if (strcasecmp(protocol, "https") == 0) {
        ap_ctx_set(r->ctx, "ssl_enabled", (void *)1);
        ap_hook_call("ap::mod_proxy::http::handler",
                     &rc, r, cr, url, proxyhost, proxyport);
        return rc;
    }
    return DECLINED;
}

static int ssl_ext_mp_set_destport(request_rec *r)
{
    if (ap_ctx_get(r->ctx, "ssl_enabled") == (void *)1)
        return DEFAULT_HTTPS_PORT;
    else
        return DEFAULT_HTTP_PORT;
}

static char *ssl_ext_mp_new_connection(request_rec *r, BUFF *fb)
{
    SSL_CTX *ssl_ctx;
    SSL *ssl;
    char *errmsg;
    int rc;

    if (ap_ctx_get(r->ctx, "ssl_enabled") != (void *)1)
        return NULL;

    /*
     * Create a SSL context and handle
     */
    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ssl_ctx);
    SSL_clear(ssl);
    SSL_set_fd(ssl, fb->fd);
    ap_ctx_set(fb->ctx, "ssl", ssl);
    ap_register_cleanup(r->pool, (void *)fb,
                        ssl_ext_mp_close_connection, ssl_ext_mp_close_connection);

    /*
     * Establish the SSL connection
     */
    if ((rc = SSL_connect(ssl)) <= 0) {
        errmsg = ap_pstrcat(r->pool, "SSL connect failed: ",
                            ERR_reason_error_string(ERR_get_error()), NULL);
        SSL_free(ssl);
        return errmsg;
    }

    return NULL;
}

static void ssl_ext_mp_close_connection(void *_fb)
{
    BUFF *fb = _fb;
    SSL *ssl;

    ssl = ap_ctx_get(fb->ctx, "ssl");
    if (ssl != NULL) {
        SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
        while (!SSL_shutdown(ssl));
        SSL_free(ssl);
        ap_ctx_set(fb->ctx, "ssl", NULL);
    }
    return;
}

static int ssl_ext_mp_write_host_header(
    request_rec *r, BUFF *fb, char *host, int port, char *portstr)
{
    if (ap_ctx_get(r->ctx, "ssl_enabled") != (void *)1)
        return DECLINED;

    if (portstr != NULL && port != DEFAULT_HTTPS_PORT) {
        ap_bvputs(fb, "Host: ", host, ":", portstr, "\r\n", NULL);
        return OK;
    }
    return DECLINED;
}

