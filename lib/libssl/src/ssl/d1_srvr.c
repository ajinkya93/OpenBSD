/* $OpenBSD: d1_srvr.c,v 1.57 2015/09/11 16:28:37 jsing Exp $ */
/*
 * DTLS implementation written by Nagendra Modadugu
 * (nagendra@cs.stanford.edu) for the OpenSSL project 2005.
 */
/* ====================================================================
 * Copyright (c) 1999-2007 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>

#include "ssl_locl.h"

#include <openssl/bn.h>
#include <openssl/buffer.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

static const SSL_METHOD *dtls1_get_server_method(int ver);
static int dtls1_send_hello_verify_request(SSL *s);

const SSL_METHOD DTLSv1_server_method_data = {
	.version = DTLS1_VERSION,
	.ssl_new = dtls1_new,
	.ssl_clear = dtls1_clear,
	.ssl_free = dtls1_free,
	.ssl_accept = dtls1_accept,
	.ssl_connect = ssl_undefined_function,
	.ssl_read = ssl3_read,
	.ssl_peek = ssl3_peek,
	.ssl_write = ssl3_write,
	.ssl_shutdown = dtls1_shutdown,
	.ssl_renegotiate = ssl3_renegotiate,
	.ssl_renegotiate_check = ssl3_renegotiate_check,
	.ssl_get_message = dtls1_get_message,
	.ssl_read_bytes = dtls1_read_bytes,
	.ssl_write_bytes = dtls1_write_app_data_bytes,
	.ssl_dispatch_alert = dtls1_dispatch_alert,
	.ssl_ctrl = dtls1_ctrl,
	.ssl_ctx_ctrl = ssl3_ctx_ctrl,
	.get_cipher_by_char = ssl3_get_cipher_by_char,
	.put_cipher_by_char = ssl3_put_cipher_by_char,
	.ssl_pending = ssl3_pending,
	.num_ciphers = ssl3_num_ciphers,
	.get_cipher = dtls1_get_cipher,
	.get_ssl_method = dtls1_get_server_method,
	.get_timeout = dtls1_default_timeout,
	.ssl3_enc = &DTLSv1_enc_data,
	.ssl_version = ssl_undefined_void_function,
	.ssl_callback_ctrl = ssl3_callback_ctrl,
	.ssl_ctx_callback_ctrl = ssl3_ctx_callback_ctrl,
};

const SSL_METHOD *
DTLSv1_server_method(void)
{
	return &DTLSv1_server_method_data;
}

static const SSL_METHOD *
dtls1_get_server_method(int ver)
{
	if (ver == DTLS1_VERSION)
		return (DTLSv1_server_method());
	return (NULL);
}

int
dtls1_accept(SSL *s)
{
	void (*cb)(const SSL *ssl, int type, int val) = NULL;
	unsigned long alg_k;
	int ret = -1;
	int new_state, state, skip = 0;
	int listen;

	ERR_clear_error();
	errno = 0;

	if (s->info_callback != NULL)
		cb = s->info_callback;
	else if (s->ctx->info_callback != NULL)
		cb = s->ctx->info_callback;

	listen = s->d1->listen;

	/* init things to blank */
	s->in_handshake++;
	if (!SSL_in_init(s) || SSL_in_before(s))
		SSL_clear(s);

	s->d1->listen = listen;

	if (s->cert == NULL) {
		SSLerr(SSL_F_DTLS1_ACCEPT, SSL_R_NO_CERTIFICATE_SET);
		return (-1);
	}

	for (;;) {
		state = s->state;

		switch (s->state) {
		case SSL_ST_RENEGOTIATE:
			s->renegotiate = 1;
			/* s->state=SSL_ST_ACCEPT; */

		case SSL_ST_BEFORE:
		case SSL_ST_ACCEPT:
		case SSL_ST_BEFORE|SSL_ST_ACCEPT:
		case SSL_ST_OK|SSL_ST_ACCEPT:

			s->server = 1;
			if (cb != NULL)
				cb(s, SSL_CB_HANDSHAKE_START, 1);

			if ((s->version & 0xff00) != (DTLS1_VERSION & 0xff00)) {
				SSLerr(SSL_F_DTLS1_ACCEPT, ERR_R_INTERNAL_ERROR);
				return -1;
			}
			s->type = SSL_ST_ACCEPT;

			if (!ssl3_setup_init_buffer(s)) {
				ret = -1;
				goto end;
			}
			if (!ssl3_setup_buffers(s)) {
				ret = -1;
				goto end;
			}

			s->init_num = 0;

			if (s->state != SSL_ST_RENEGOTIATE) {
				/* Ok, we now need to push on a buffering BIO so that
				 * the output is sent in a way that TCP likes :-)
				 * ...but not with SCTP :-)
				 */
				if (!ssl_init_wbio_buffer(s, 1)) {
					ret = -1;
					goto end;
				}

				if (!ssl3_init_finished_mac(s)) {
					ret = -1;
					goto end;
				}

				s->state = SSL3_ST_SR_CLNT_HELLO_A;
				s->ctx->stats.sess_accept++;
			} else {
				/* s->state == SSL_ST_RENEGOTIATE,
				 * we will just send a HelloRequest */
				s->ctx->stats.sess_accept_renegotiate++;
				s->state = SSL3_ST_SW_HELLO_REQ_A;
			}

			break;

		case SSL3_ST_SW_HELLO_REQ_A:
		case SSL3_ST_SW_HELLO_REQ_B:

			s->shutdown = 0;
			dtls1_clear_record_buffer(s);
			dtls1_start_timer(s);
			ret = dtls1_send_hello_request(s);
			if (ret <= 0)
				goto end;
			s->s3->tmp.next_state = SSL3_ST_SR_CLNT_HELLO_A;
			s->state = SSL3_ST_SW_FLUSH;
			s->init_num = 0;

			if (!ssl3_init_finished_mac(s)) {
				ret = -1;
				goto end;
			}
			break;

		case SSL3_ST_SW_HELLO_REQ_C:
			s->state = SSL_ST_OK;
			break;

		case SSL3_ST_SR_CLNT_HELLO_A:
		case SSL3_ST_SR_CLNT_HELLO_B:
		case SSL3_ST_SR_CLNT_HELLO_C:

			s->shutdown = 0;
			ret = ssl3_get_client_hello(s);
			if (ret <= 0)
				goto end;
			dtls1_stop_timer(s);

			if (ret == 1 && (SSL_get_options(s) & SSL_OP_COOKIE_EXCHANGE))
				s->state = DTLS1_ST_SW_HELLO_VERIFY_REQUEST_A;
			else
				s->state = SSL3_ST_SW_SRVR_HELLO_A;

			s->init_num = 0;

			/* Reflect ClientHello sequence to remain stateless while listening */
			if (listen) {
				memcpy(s->s3->write_sequence, s->s3->read_sequence, sizeof(s->s3->write_sequence));
			}

			/* If we're just listening, stop here */
			if (listen && s->state == SSL3_ST_SW_SRVR_HELLO_A) {
				ret = 2;
				s->d1->listen = 0;
				/* Set expected sequence numbers
				 * to continue the handshake.
				 */
				s->d1->handshake_read_seq = 2;
				s->d1->handshake_write_seq = 1;
				s->d1->next_handshake_write_seq = 1;
				goto end;
			}

			break;

		case DTLS1_ST_SW_HELLO_VERIFY_REQUEST_A:
		case DTLS1_ST_SW_HELLO_VERIFY_REQUEST_B:

			ret = dtls1_send_hello_verify_request(s);
			if (ret <= 0)
				goto end;
			s->state = SSL3_ST_SW_FLUSH;
			s->s3->tmp.next_state = SSL3_ST_SR_CLNT_HELLO_A;

			/* HelloVerifyRequest resets Finished MAC */
			if (!ssl3_init_finished_mac(s)) {
				ret = -1;
				goto end;
			}
			break;


		case SSL3_ST_SW_SRVR_HELLO_A:
		case SSL3_ST_SW_SRVR_HELLO_B:
			s->renegotiate = 2;
			dtls1_start_timer(s);
			ret = dtls1_send_server_hello(s);
			if (ret <= 0)
				goto end;

			if (s->hit) {
				if (s->tlsext_ticket_expected)
					s->state = SSL3_ST_SW_SESSION_TICKET_A;
				else
					s->state = SSL3_ST_SW_CHANGE_A;
			} else
				s->state = SSL3_ST_SW_CERT_A;
			s->init_num = 0;
			break;

		case SSL3_ST_SW_CERT_A:
		case SSL3_ST_SW_CERT_B:
			/* Check if it is anon DH. */
			if (!(s->s3->tmp.new_cipher->algorithm_auth &
			    SSL_aNULL)) {
				dtls1_start_timer(s);
				ret = dtls1_send_server_certificate(s);
				if (ret <= 0)
					goto end;
				if (s->tlsext_status_expected)
					s->state = SSL3_ST_SW_CERT_STATUS_A;
				else
					s->state = SSL3_ST_SW_KEY_EXCH_A;
			} else {
				skip = 1;
				s->state = SSL3_ST_SW_KEY_EXCH_A;
			}
			s->init_num = 0;
			break;

		case SSL3_ST_SW_KEY_EXCH_A:
		case SSL3_ST_SW_KEY_EXCH_B:
			alg_k = s->s3->tmp.new_cipher->algorithm_mkey;

			/* Only send if using a DH key exchange. */
			if (alg_k & (SSL_kDHE|SSL_kECDHE)) {
				dtls1_start_timer(s);
				ret = dtls1_send_server_key_exchange(s);
				if (ret <= 0)
					goto end;
			} else
				skip = 1;

			s->state = SSL3_ST_SW_CERT_REQ_A;
			s->init_num = 0;
			break;

		case SSL3_ST_SW_CERT_REQ_A:
		case SSL3_ST_SW_CERT_REQ_B:
			/*
			 * Determine whether or not we need to request a
			 * certificate.
			 *
			 * Do not request a certificate if:
			 *
			 * - We did not ask for it (SSL_VERIFY_PEER is unset).
			 *
			 * - SSL_VERIFY_CLIENT_ONCE is set and we are
			 *   renegotiating.
			 *
			 * - We are using an anonymous ciphersuites
			 *   (see section "Certificate request" in SSL 3 drafts
			 *   and in RFC 2246) ... except when the application
			 *   insists on verification (against the specs, but
			 *   s3_clnt.c accepts this for SSL 3).
			 */
			if (!(s->verify_mode & SSL_VERIFY_PEER) ||
			    ((s->session->peer != NULL) &&
			     (s->verify_mode & SSL_VERIFY_CLIENT_ONCE)) ||
			    ((s->s3->tmp.new_cipher->algorithm_auth &
			     SSL_aNULL) && !(s->verify_mode &
			     SSL_VERIFY_FAIL_IF_NO_PEER_CERT))) {
				/* no cert request */
				skip = 1;
				s->s3->tmp.cert_request = 0;
				s->state = SSL3_ST_SW_SRVR_DONE_A;
			} else {
				s->s3->tmp.cert_request = 1;
				dtls1_start_timer(s);
				ret = dtls1_send_certificate_request(s);
				if (ret <= 0)
					goto end;
				s->state = SSL3_ST_SW_SRVR_DONE_A;
				s->init_num = 0;
			}
			break;

		case SSL3_ST_SW_SRVR_DONE_A:
		case SSL3_ST_SW_SRVR_DONE_B:
			dtls1_start_timer(s);
			ret = dtls1_send_server_done(s);
			if (ret <= 0)
				goto end;
			s->s3->tmp.next_state = SSL3_ST_SR_CERT_A;
			s->state = SSL3_ST_SW_FLUSH;
			s->init_num = 0;
			break;

		case SSL3_ST_SW_FLUSH:
			s->rwstate = SSL_WRITING;
			if (BIO_flush(s->wbio) <= 0) {
				/* If the write error was fatal, stop trying */
				if (!BIO_should_retry(s->wbio)) {
					s->rwstate = SSL_NOTHING;
					s->state = s->s3->tmp.next_state;
				}

				ret = -1;
				goto end;
			}
			s->rwstate = SSL_NOTHING;
			s->state = s->s3->tmp.next_state;
			break;

		case SSL3_ST_SR_CERT_A:
		case SSL3_ST_SR_CERT_B:
			if (s->s3->tmp.cert_request) {
				ret = ssl3_get_client_certificate(s);
				if (ret <= 0)
					goto end;
			}
			s->init_num = 0;
			s->state = SSL3_ST_SR_KEY_EXCH_A;
			break;

		case SSL3_ST_SR_KEY_EXCH_A:
		case SSL3_ST_SR_KEY_EXCH_B:
			ret = ssl3_get_client_key_exchange(s);
			if (ret <= 0)
				goto end;

			s->state = SSL3_ST_SR_CERT_VRFY_A;
			s->init_num = 0;

			if (ret == 2) {
				/* For the ECDH ciphersuites when
				 * the client sends its ECDH pub key in
				 * a certificate, the CertificateVerify
				 * message is not sent.
				 */
				s->state = SSL3_ST_SR_FINISHED_A;
				s->init_num = 0;
			} else {
				s->state = SSL3_ST_SR_CERT_VRFY_A;
				s->init_num = 0;

				/* We need to get hashes here so if there is
				 * a client cert, it can be verified */
				s->method->ssl3_enc->cert_verify_mac(s,
				    NID_md5, &(s->s3->tmp.cert_verify_md[0]));
				s->method->ssl3_enc->cert_verify_mac(s,
				    NID_sha1,
				    &(s->s3->tmp.cert_verify_md[MD5_DIGEST_LENGTH]));
			}
			break;

		case SSL3_ST_SR_CERT_VRFY_A:
		case SSL3_ST_SR_CERT_VRFY_B:

			s->d1->change_cipher_spec_ok = 1;
			/* we should decide if we expected this one */
			ret = ssl3_get_cert_verify(s);
			if (ret <= 0)
				goto end;
			s->state = SSL3_ST_SR_FINISHED_A;
			s->init_num = 0;
			break;

		case SSL3_ST_SR_FINISHED_A:
		case SSL3_ST_SR_FINISHED_B:
			s->d1->change_cipher_spec_ok = 1;
			ret = ssl3_get_finished(s, SSL3_ST_SR_FINISHED_A,
			SSL3_ST_SR_FINISHED_B);
			if (ret <= 0)
				goto end;
			dtls1_stop_timer(s);
			if (s->hit)
				s->state = SSL_ST_OK;
			else if (s->tlsext_ticket_expected)
				s->state = SSL3_ST_SW_SESSION_TICKET_A;
			else
				s->state = SSL3_ST_SW_CHANGE_A;
			s->init_num = 0;
			break;

		case SSL3_ST_SW_SESSION_TICKET_A:
		case SSL3_ST_SW_SESSION_TICKET_B:
			ret = dtls1_send_newsession_ticket(s);
			if (ret <= 0)
				goto end;
			s->state = SSL3_ST_SW_CHANGE_A;
			s->init_num = 0;
			break;

		case SSL3_ST_SW_CERT_STATUS_A:
		case SSL3_ST_SW_CERT_STATUS_B:
			ret = ssl3_send_cert_status(s);
			if (ret <= 0)
				goto end;
			s->state = SSL3_ST_SW_KEY_EXCH_A;
			s->init_num = 0;
			break;


		case SSL3_ST_SW_CHANGE_A:
		case SSL3_ST_SW_CHANGE_B:

			s->session->cipher = s->s3->tmp.new_cipher;
			if (!s->method->ssl3_enc->setup_key_block(s)) {
				ret = -1;
				goto end;
			}

			ret = dtls1_send_change_cipher_spec(s,
			SSL3_ST_SW_CHANGE_A, SSL3_ST_SW_CHANGE_B);

			if (ret <= 0)
				goto end;


			s->state = SSL3_ST_SW_FINISHED_A;
			s->init_num = 0;

			if (!s->method->ssl3_enc->change_cipher_state(s,
				SSL3_CHANGE_CIPHER_SERVER_WRITE)) {
				ret = -1;
				goto end;
			}

			dtls1_reset_seq_numbers(s, SSL3_CC_WRITE);
			break;

		case SSL3_ST_SW_FINISHED_A:
		case SSL3_ST_SW_FINISHED_B:
			ret = ssl3_send_finished(s,
			    SSL3_ST_SW_FINISHED_A, SSL3_ST_SW_FINISHED_B,
			    s->method->ssl3_enc->server_finished_label,
			    s->method->ssl3_enc->server_finished_label_len);
			if (ret <= 0)
				goto end;
			s->state = SSL3_ST_SW_FLUSH;
			if (s->hit) {
				s->s3->tmp.next_state = SSL3_ST_SR_FINISHED_A;

			} else {
				s->s3->tmp.next_state = SSL_ST_OK;
			}
			s->init_num = 0;
			break;

		case SSL_ST_OK:
			/* clean a few things up */
			ssl3_cleanup_key_block(s);

			/* remove buffering on output */
			ssl_free_wbio_buffer(s);

			s->init_num = 0;

			if (s->renegotiate == 2) /* skipped if we just sent a HelloRequest */
			{
				s->renegotiate = 0;
				s->new_session = 0;

				ssl_update_cache(s, SSL_SESS_CACHE_SERVER);

				s->ctx->stats.sess_accept_good++;
				/* s->server=1; */
				s->handshake_func = dtls1_accept;

				if (cb != NULL)
					cb(s, SSL_CB_HANDSHAKE_DONE, 1);
			}

			ret = 1;

			/* done handshaking, next message is client hello */
			s->d1->handshake_read_seq = 0;
			/* next message is server hello */
			s->d1->handshake_write_seq = 0;
			s->d1->next_handshake_write_seq = 0;
			goto end;
			/* break; */

		default:
			SSLerr(SSL_F_DTLS1_ACCEPT, SSL_R_UNKNOWN_STATE);
			ret = -1;
			goto end;
			/* break; */
		}

		if (!s->s3->tmp.reuse_message && !skip) {
			if (s->debug) {
				if ((ret = BIO_flush(s->wbio)) <= 0)
					goto end;
			}

			if ((cb != NULL) && (s->state != state)) {
				new_state = s->state;
				s->state = state;
				cb(s, SSL_CB_ACCEPT_LOOP, 1);
				s->state = new_state;
			}
		}
		skip = 0;
	}
end:
	/* BIO_flush(s->wbio); */

	s->in_handshake--;

	if (cb != NULL)
		cb(s, SSL_CB_ACCEPT_EXIT, ret);
	return (ret);
}

int
dtls1_send_hello_request(SSL *s)
{
	if (s->state == SSL3_ST_SW_HELLO_REQ_A) {
		ssl3_handshake_msg_start(s, SSL3_MT_HELLO_REQUEST);
		ssl3_handshake_msg_finish(s, 0);

		s->state = SSL3_ST_SW_HELLO_REQ_B;
	}

	/* SSL3_ST_SW_HELLO_REQ_B */
	return (ssl3_handshake_write(s));
}

int
dtls1_send_hello_verify_request(SSL *s)
{
	unsigned char *d, *p;

	if (s->state == DTLS1_ST_SW_HELLO_VERIFY_REQUEST_A) {
		d = p = ssl3_handshake_msg_start(s,
		    DTLS1_MT_HELLO_VERIFY_REQUEST);

		*(p++) = s->version >> 8;
		*(p++) = s->version & 0xFF;

		if (s->ctx->app_gen_cookie_cb == NULL ||
		    s->ctx->app_gen_cookie_cb(s, s->d1->cookie,
			&(s->d1->cookie_len)) == 0) {
			SSLerr(SSL_F_DTLS1_SEND_HELLO_VERIFY_REQUEST,
			    ERR_R_INTERNAL_ERROR);
			return 0;
		}

		*(p++) = (unsigned char) s->d1->cookie_len;
		memcpy(p, s->d1->cookie, s->d1->cookie_len);
		p += s->d1->cookie_len;

		ssl3_handshake_msg_finish(s, p - d);

		s->state = DTLS1_ST_SW_HELLO_VERIFY_REQUEST_B;
	}

	/* s->state = DTLS1_ST_SW_HELLO_VERIFY_REQUEST_B */
	return (ssl3_handshake_write(s));
}

int
dtls1_send_server_hello(SSL *s)
{
	unsigned char *bufend;
	unsigned char *p, *d;
	unsigned int sl;

	if (s->state == SSL3_ST_SW_SRVR_HELLO_A) {
		d = p = ssl3_handshake_msg_start(s, SSL3_MT_SERVER_HELLO);

		*(p++) = s->version >> 8;
		*(p++) = s->version & 0xff;

		/* Random stuff */
		arc4random_buf(s->s3->server_random, SSL3_RANDOM_SIZE);
		memcpy(p, s->s3->server_random, SSL3_RANDOM_SIZE);
		p += SSL3_RANDOM_SIZE;

		/* now in theory we have 3 options to sending back the
		 * session id.  If it is a re-use, we send back the
		 * old session-id, if it is a new session, we send
		 * back the new session-id or we send back a 0 length
		 * session-id if we want it to be single use.
		 * Currently I will not implement the '0' length session-id
		 * 12-Jan-98 - I'll now support the '0' length stuff.
		 */
		if (!(s->ctx->session_cache_mode & SSL_SESS_CACHE_SERVER))
			s->session->session_id_length = 0;

		sl = s->session->session_id_length;
		if (sl > sizeof s->session->session_id) {
			SSLerr(SSL_F_DTLS1_SEND_SERVER_HELLO,
			    ERR_R_INTERNAL_ERROR);
			return -1;
		}
		*(p++) = sl;
		memcpy(p, s->session->session_id, sl);
		p += sl;

		/* put the cipher */
		if (s->s3->tmp.new_cipher == NULL)
			return -1;
		s2n(ssl3_cipher_get_value(s->s3->tmp.new_cipher), p);

		/* put the compression method */
		*(p++) = 0;

		bufend = (unsigned char *)s->init_buf->data +
		    SSL3_RT_MAX_PLAIN_LENGTH;
		if ((p = ssl_add_serverhello_tlsext(s, p, bufend)) == NULL) {
			SSLerr(SSL_F_DTLS1_SEND_SERVER_HELLO,
			    ERR_R_INTERNAL_ERROR);
			return -1;
		}

		ssl3_handshake_msg_finish(s, p - d);

		s->state = SSL3_ST_SW_SRVR_HELLO_B;
	}

	/* SSL3_ST_SW_SRVR_HELLO_B */
	return (ssl3_handshake_write(s));
}

int
dtls1_send_server_done(SSL *s)
{
	if (s->state == SSL3_ST_SW_SRVR_DONE_A) {
		ssl3_handshake_msg_start(s, SSL3_MT_SERVER_DONE);
		ssl3_handshake_msg_finish(s, 0);

		s->state = SSL3_ST_SW_SRVR_DONE_B;
	}

	/* SSL3_ST_SW_SRVR_DONE_B */
	return (ssl3_handshake_write(s));
}

int
dtls1_send_server_key_exchange(SSL *s)
{
	unsigned char *q;
	int j, num;
	unsigned char md_buf[MD5_DIGEST_LENGTH + SHA_DIGEST_LENGTH];
	unsigned int u;
	DH *dh = NULL, *dhp;
	EC_KEY *ecdh = NULL, *ecdhp;
	unsigned char *encodedPoint = NULL;
	int encodedlen = 0;
	int curve_id = 0;
	BN_CTX *bn_ctx = NULL;

	EVP_PKEY *pkey;
	unsigned char *p, *d;
	int al, i;
	unsigned long type;
	int n;
	CERT *cert;
	BIGNUM *r[4];
	int nr[4], kn;
	BUF_MEM *buf;
	EVP_MD_CTX md_ctx;

	EVP_MD_CTX_init(&md_ctx);
	if (s->state == SSL3_ST_SW_KEY_EXCH_A) {
		type = s->s3->tmp.new_cipher->algorithm_mkey;
		cert = s->cert;

		buf = s->init_buf;

		r[0] = r[1] = r[2] = r[3] = NULL;
		n = 0;

		if (type & SSL_kDHE) {
			dhp = cert->dh_tmp;
			if ((dhp == NULL) && (s->cert->dh_tmp_cb != NULL))
				dhp = s->cert->dh_tmp_cb(s, 0,
				    SSL_C_PKEYLENGTH(s->s3->tmp.new_cipher));
			if (dhp == NULL) {
				al = SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, SSL_R_MISSING_TMP_DH_KEY);
				goto f_err;
			}

			if (s->s3->tmp.dh != NULL) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_INTERNAL_ERROR);
				goto err;
			}

			if ((dh = DHparams_dup(dhp)) == NULL) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_DH_LIB);
				goto err;
			}

			s->s3->tmp.dh = dh;
			if ((dhp->pub_key == NULL || dhp->priv_key == NULL ||
			    (s->options & SSL_OP_SINGLE_DH_USE))) {
				if (!DH_generate_key(dh)) {
					SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE,
					ERR_R_DH_LIB);
					goto err;
				}
			} else {
				dh->pub_key = BN_dup(dhp->pub_key);
				dh->priv_key = BN_dup(dhp->priv_key);
				if ((dh->pub_key == NULL) ||
				    (dh->priv_key == NULL)) {
					SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_DH_LIB);
					goto err;
				}
			}
			r[0] = dh->p;
			r[1] = dh->g;
			r[2] = dh->pub_key;
		} else if (type & SSL_kECDHE) {
			const EC_GROUP *group;

			ecdhp = cert->ecdh_tmp;
			if (ecdhp == NULL && s->cert->ecdh_tmp_cb != NULL)
				ecdhp = s->cert->ecdh_tmp_cb(s, 0,
				    SSL_C_PKEYLENGTH(s->s3->tmp.new_cipher));
			if (ecdhp == NULL) {
				al = SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, SSL_R_MISSING_TMP_ECDH_KEY);
				goto f_err;
			}

			if (s->s3->tmp.ecdh != NULL) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_INTERNAL_ERROR);
				goto err;
			}

			/* Duplicate the ECDH structure. */
			if ((ecdh = EC_KEY_dup(ecdhp)) == NULL) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_ECDH_LIB);
				goto err;
			}
			s->s3->tmp.ecdh = ecdh;

			if ((EC_KEY_get0_public_key(ecdh) == NULL) ||
			    (EC_KEY_get0_private_key(ecdh) == NULL) ||
			    (s->options & SSL_OP_SINGLE_ECDH_USE)) {
				if (!EC_KEY_generate_key(ecdh)) {
					SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_ECDH_LIB);
					goto err;
				}
			}

			if (((group = EC_KEY_get0_group(ecdh)) == NULL) ||
			    (EC_KEY_get0_public_key(ecdh)  == NULL) ||
			    (EC_KEY_get0_private_key(ecdh) == NULL)) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_ECDH_LIB);
				goto err;
			}

			/* XXX: For now, we only support ephemeral ECDH
			 * keys over named (not generic) curves. For
			 * supported named curves, curve_id is non-zero.
			 */
			if ((curve_id = tls1_ec_nid2curve_id(
			    EC_GROUP_get_curve_name(group))) == 0) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, SSL_R_UNSUPPORTED_ELLIPTIC_CURVE);
				goto err;
			}

			/* Encode the public key.
			 * First check the size of encoding and
			 * allocate memory accordingly.
			 */
			encodedlen = EC_POINT_point2oct(group,
			EC_KEY_get0_public_key(ecdh),
			POINT_CONVERSION_UNCOMPRESSED,
			NULL, 0, NULL);

			encodedPoint = malloc(encodedlen);

			bn_ctx = BN_CTX_new();
			if ((encodedPoint == NULL) || (bn_ctx == NULL)) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_MALLOC_FAILURE);
				goto err;
			}


			encodedlen = EC_POINT_point2oct(group,
			EC_KEY_get0_public_key(ecdh),
			POINT_CONVERSION_UNCOMPRESSED,
			encodedPoint, encodedlen, bn_ctx);

			if (encodedlen == 0) {
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_R_ECDH_LIB);
				goto err;
			}

			BN_CTX_free(bn_ctx);
			bn_ctx = NULL;

			/* XXX: For now, we only support named (not
			 * generic) curves in ECDH ephemeral key exchanges.
			 * In this situation, we need four additional bytes
			 * to encode the entire ServerECDHParams
			 * structure.
			 */
			n = 4 + encodedlen;

			/* We'll generate the serverKeyExchange message
			 * explicitly so we can set these to NULLs
			 */
			r[0] = NULL;
			r[1] = NULL;
			r[2] = NULL;
			r[3] = NULL;
		} else {
			al = SSL_AD_HANDSHAKE_FAILURE;
			SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE,
			    SSL_R_UNKNOWN_KEY_EXCHANGE_TYPE);
			goto f_err;
		}
		for (i = 0; r[i] != NULL; i++) {
			nr[i] = BN_num_bytes(r[i]);
			n += 2 + nr[i];
		}

		if (!(s->s3->tmp.new_cipher->algorithm_auth & SSL_aNULL)) {
			if ((pkey = ssl_get_sign_pkey(s,
			    s->s3->tmp.new_cipher, NULL)) == NULL) {
				al = SSL_AD_DECODE_ERROR;
				goto f_err;
			}
			kn = EVP_PKEY_size(pkey);
		} else {
			pkey = NULL;
			kn = 0;
		}

		if (!BUF_MEM_grow_clean(buf, n + DTLS1_HM_HEADER_LENGTH + kn)) {
			SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_LIB_BUF);
			goto err;
		}
		d = (unsigned char *)s->init_buf->data;
		p = &(d[DTLS1_HM_HEADER_LENGTH]);

		for (i = 0; r[i] != NULL; i++) {
			s2n(nr[i], p);
			BN_bn2bin(r[i], p);
			p += nr[i];
		}

		if (type & SSL_kECDHE) {
			/* XXX: For now, we only support named (not generic) curves.
			 * In this situation, the serverKeyExchange message has:
			 * [1 byte CurveType], [2 byte CurveName]
			 * [1 byte length of encoded point], followed by
			 * the actual encoded point itself
			 */
			*p = NAMED_CURVE_TYPE;
			p += 1;
			*p = 0;
			p += 1;
			*p = curve_id;
			p += 1;
			*p = encodedlen;
			p += 1;
			memcpy((unsigned char*)p,
			    (unsigned char *)encodedPoint, encodedlen);
			free(encodedPoint);
			encodedPoint = NULL;
			p += encodedlen;
		}


		/* not anonymous */
		if (pkey != NULL) {
			/* n is the length of the params, they start at
			 * &(d[DTLS1_HM_HEADER_LENGTH]) and p points to the space
			 * at the end. */
			if (pkey->type == EVP_PKEY_RSA) {
				q = md_buf;
				j = 0;
				for (num = 2; num > 0; num--) {
					if (!EVP_DigestInit_ex(&md_ctx, (num == 2)
					    ? s->ctx->md5 : s->ctx->sha1, NULL))
						goto err;
					EVP_DigestUpdate(&md_ctx,
					    &(s->s3->client_random[0]),
					    SSL3_RANDOM_SIZE);
					EVP_DigestUpdate(&md_ctx,
					    &(s->s3->server_random[0]),
					    SSL3_RANDOM_SIZE);
					EVP_DigestUpdate(&md_ctx,
					    &(d[DTLS1_HM_HEADER_LENGTH]), n);
					EVP_DigestFinal_ex(&md_ctx, q,
					    (unsigned int *)&i);
					q += i;
					j += i;
				}
				if (RSA_sign(NID_md5_sha1, md_buf, j, &(p[2]),
				    &u, pkey->pkey.rsa) <= 0) {
					SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_LIB_RSA);
					goto err;
				}
				s2n(u, p);
				n += u + 2;
			} else
			if (pkey->type == EVP_PKEY_DSA) {
				/* lets do DSS */
				EVP_SignInit_ex(&md_ctx, EVP_dss1(), NULL);
				EVP_SignUpdate(&md_ctx, &(s->s3->client_random[0]), SSL3_RANDOM_SIZE);
				EVP_SignUpdate(&md_ctx, &(s->s3->server_random[0]), SSL3_RANDOM_SIZE);
				EVP_SignUpdate(&md_ctx, &(d[DTLS1_HM_HEADER_LENGTH]), n);
				if (!EVP_SignFinal(&md_ctx, &(p[2]),
				    (unsigned int *)&i, pkey)) {
					SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_LIB_DSA);
					goto err;
				}
				s2n(i, p);
				n += i + 2;
			} else
			if (pkey->type == EVP_PKEY_EC) {
				/* let's do ECDSA */
				EVP_SignInit_ex(&md_ctx, EVP_ecdsa(), NULL);
				EVP_SignUpdate(&md_ctx, &(s->s3->client_random[0]), SSL3_RANDOM_SIZE);
				EVP_SignUpdate(&md_ctx, &(s->s3->server_random[0]), SSL3_RANDOM_SIZE);
				EVP_SignUpdate(&md_ctx, &(d[DTLS1_HM_HEADER_LENGTH]), n);
				if (!EVP_SignFinal(&md_ctx, &(p[2]),
				    (unsigned int *)&i, pkey)) {
					SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, ERR_LIB_ECDSA);
					goto err;
				}
				s2n(i, p);
				n += i + 2;
			} else
			{
				/* Is this error check actually needed? */
				al = SSL_AD_HANDSHAKE_FAILURE;
				SSLerr(SSL_F_DTLS1_SEND_SERVER_KEY_EXCHANGE, SSL_R_UNKNOWN_PKEY_TYPE);
				goto f_err;
			}
		}

		d = dtls1_set_message_header(s, d,
		SSL3_MT_SERVER_KEY_EXCHANGE, n, 0, n);

		/* we should now have things packed up, so lets send
		 * it off */
		s->init_num = n + DTLS1_HM_HEADER_LENGTH;
		s->init_off = 0;

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);
	}

	s->state = SSL3_ST_SW_KEY_EXCH_B;
	EVP_MD_CTX_cleanup(&md_ctx);
	return (dtls1_do_write(s, SSL3_RT_HANDSHAKE));
f_err:
	ssl3_send_alert(s, SSL3_AL_FATAL, al);
err:
	free(encodedPoint);
	BN_CTX_free(bn_ctx);
	EVP_MD_CTX_cleanup(&md_ctx);
	return (-1);
}

int
dtls1_send_certificate_request(SSL *s)
{
	unsigned char *p, *d;
	int i, j, nl, off, n;
	STACK_OF(X509_NAME) *sk = NULL;
	X509_NAME *name;
	BUF_MEM *buf;
	unsigned int msg_len;

	if (s->state == SSL3_ST_SW_CERT_REQ_A) {
		buf = s->init_buf;

		d = p=(unsigned char *)&(buf->data[DTLS1_HM_HEADER_LENGTH]);

		/* get the list of acceptable cert types */
		p++;
		n = ssl3_get_req_cert_type(s, p);
		d[0] = n;
		p += n;
		n++;

		off = n;
		p += 2;
		n += 2;

		sk = SSL_get_client_CA_list(s);
		nl = 0;
		if (sk != NULL) {
			for (i = 0; i < sk_X509_NAME_num(sk); i++) {
				name = sk_X509_NAME_value(sk, i);
				j = i2d_X509_NAME(name, NULL);
				if (!BUF_MEM_grow_clean(buf, DTLS1_HM_HEADER_LENGTH + n + j + 2)) {
					SSLerr(SSL_F_DTLS1_SEND_CERTIFICATE_REQUEST, ERR_R_BUF_LIB);
					goto err;
				}
				p = (unsigned char *)&(buf->data[DTLS1_HM_HEADER_LENGTH + n]);
				s2n(j, p);
				i2d_X509_NAME(name, &p);
				n += 2 + j;
				nl += 2 + j;
			}
		}
		/* else no CA names */
		p = (unsigned char *)&(buf->data[DTLS1_HM_HEADER_LENGTH + off]);
		s2n(nl, p);

		d = (unsigned char *)buf->data;
		*(d++) = SSL3_MT_CERTIFICATE_REQUEST;
		l2n3(n, d);
		s2n(s->d1->handshake_write_seq, d);
		s->d1->handshake_write_seq++;

		/* we should now have things packed up, so lets send
		 * it off */

		s->init_num = n + DTLS1_HM_HEADER_LENGTH;
		s->init_off = 0;

		/* XDTLS:  set message header ? */
		msg_len = s->init_num - DTLS1_HM_HEADER_LENGTH;
		dtls1_set_message_header(s, (void *)s->init_buf->data,
		    SSL3_MT_CERTIFICATE_REQUEST, msg_len, 0, msg_len);

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);

		s->state = SSL3_ST_SW_CERT_REQ_B;
	}

	/* SSL3_ST_SW_CERT_REQ_B */
	return (dtls1_do_write(s, SSL3_RT_HANDSHAKE));
err:
	return (-1);
}

int
dtls1_send_server_certificate(SSL *s)
{
	unsigned long l;
	X509 *x;

	if (s->state == SSL3_ST_SW_CERT_A) {
		x = ssl_get_server_send_cert(s);
		if (x == NULL) {
			SSLerr(SSL_F_DTLS1_SEND_SERVER_CERTIFICATE,
			    ERR_R_INTERNAL_ERROR);
			return (0);
		}

		l = dtls1_output_cert_chain(s, x);
		s->state = SSL3_ST_SW_CERT_B;
		s->init_num = (int)l;
		s->init_off = 0;

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);
	}

	/* SSL3_ST_SW_CERT_B */
	return (dtls1_do_write(s, SSL3_RT_HANDSHAKE));
}

int
dtls1_send_newsession_ticket(SSL *s)
{
	if (s->state == SSL3_ST_SW_SESSION_TICKET_A) {
		unsigned char *p, *senc, *macstart;
		int len, slen;
		unsigned int hlen, msg_len;
		EVP_CIPHER_CTX ctx;
		HMAC_CTX hctx;
		SSL_CTX *tctx = s->initial_ctx;
		unsigned char iv[EVP_MAX_IV_LENGTH];
		unsigned char key_name[16];

		/* get session encoding length */
		slen = i2d_SSL_SESSION(s->session, NULL);
		/* Some length values are 16 bits, so forget it if session is
 		 * too long
 		 */
		if (slen > 0xFF00)
			return -1;
		/* Grow buffer if need be: the length calculation is as
 		 * follows 12 (DTLS handshake message header) +
 		 * 4 (ticket lifetime hint) + 2 (ticket length) +
 		 * 16 (key name) + max_iv_len (iv length) +
 		 * session_length + max_enc_block_size (max encrypted session
 		 * length) + max_md_size (HMAC).
 		 */
		if (!BUF_MEM_grow(s->init_buf,
		    DTLS1_HM_HEADER_LENGTH + 22 + EVP_MAX_IV_LENGTH +
		    EVP_MAX_BLOCK_LENGTH + EVP_MAX_MD_SIZE + slen))
			return -1;
		senc = malloc(slen);
		if (!senc)
			return -1;
		p = senc;
		i2d_SSL_SESSION(s->session, &p);

		p = (unsigned char *)&(s->init_buf->data[DTLS1_HM_HEADER_LENGTH]);
		EVP_CIPHER_CTX_init(&ctx);
		HMAC_CTX_init(&hctx);
		/* Initialize HMAC and cipher contexts. If callback present
		 * it does all the work otherwise use generated values
		 * from parent ctx.
		 */
		if (tctx->tlsext_ticket_key_cb) {
			if (tctx->tlsext_ticket_key_cb(s, key_name, iv, &ctx,
			    &hctx, 1) < 0) {
				free(senc);
				EVP_CIPHER_CTX_cleanup(&ctx);
				return -1;
			}
		} else {
			arc4random_buf(iv, 16);
			EVP_EncryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL,
			    tctx->tlsext_tick_aes_key, iv);
			HMAC_Init_ex(&hctx, tctx->tlsext_tick_hmac_key, 16,
			    tlsext_tick_md(), NULL);
			memcpy(key_name, tctx->tlsext_tick_key_name, 16);
		}
		l2n(s->session->tlsext_tick_lifetime_hint, p);
		/* Skip ticket length for now */
		p += 2;
		/* Output key name */
		macstart = p;
		memcpy(p, key_name, 16);
		p += 16;
		/* output IV */
		memcpy(p, iv, EVP_CIPHER_CTX_iv_length(&ctx));
		p += EVP_CIPHER_CTX_iv_length(&ctx);
		/* Encrypt session data */
		EVP_EncryptUpdate(&ctx, p, &len, senc, slen);
		p += len;
		EVP_EncryptFinal(&ctx, p, &len);
		p += len;
		EVP_CIPHER_CTX_cleanup(&ctx);

		HMAC_Update(&hctx, macstart, p - macstart);
		HMAC_Final(&hctx, p, &hlen);
		HMAC_CTX_cleanup(&hctx);

		p += hlen;
		/* Now write out lengths: p points to end of data written */
		/* Total length */
		len = p - (unsigned char *)(s->init_buf->data);
		/* Ticket length */
		p = (unsigned char *)&(s->init_buf->data[DTLS1_HM_HEADER_LENGTH]) + 4;
		s2n(len - DTLS1_HM_HEADER_LENGTH - 6, p);

		/* number of bytes to write */
		s->init_num = len;
		s->state = SSL3_ST_SW_SESSION_TICKET_B;
		s->init_off = 0;
		free(senc);

		/* XDTLS:  set message header ? */
		msg_len = s->init_num - DTLS1_HM_HEADER_LENGTH;
		dtls1_set_message_header(s, (void *)s->init_buf->data,
		SSL3_MT_NEWSESSION_TICKET, msg_len, 0, msg_len);

		/* buffer the message to handle re-xmits */
		dtls1_buffer_message(s, 0);
	}

	/* SSL3_ST_SW_SESSION_TICKET_B */
	return (dtls1_do_write(s, SSL3_RT_HANDSHAKE));
}
