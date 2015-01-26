/* $OpenBSD: sshkey.h,v 1.5 2015/01/26 02:59:11 djm Exp $ */

/*
 * Copyright (c) 2000, 2001 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SSHKEY_H
#define SSHKEY_H

#include <sys/types.h>

#ifdef WITH_OPENSSL
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/ec.h>
#else /* OPENSSL */
#define RSA		void
#define DSA		void
#define EC_KEY		void
#define EC_GROUP	void
#define EC_POINT	void
#endif /* WITH_OPENSSL */

#define SSH_RSA_MINIMUM_MODULUS_SIZE	768
#define SSH_KEY_MAX_SIGN_DATA_SIZE	(1 << 20)

struct sshbuf;

/* Key types */
enum sshkey_types {
	KEY_RSA1,
	KEY_RSA,
	KEY_DSA,
	KEY_ECDSA,
	KEY_ED25519,
	KEY_RSA_CERT,
	KEY_DSA_CERT,
	KEY_ECDSA_CERT,
	KEY_ED25519_CERT,
	KEY_RSA_CERT_V00,
	KEY_DSA_CERT_V00,
	KEY_UNSPEC
};

/* Default fingerprint hash */
#define SSH_FP_HASH_DEFAULT	SSH_DIGEST_SHA256

/* Fingerprint representation formats */
enum sshkey_fp_rep {
	SSH_FP_DEFAULT = 0,
	SSH_FP_HEX,
	SSH_FP_BASE64,
	SSH_FP_BUBBLEBABBLE,
	SSH_FP_RANDOMART
};

/* key is stored in external hardware */
#define SSHKEY_FLAG_EXT		0x0001

#define SSHKEY_CERT_MAX_PRINCIPALS	256
/* XXX opaquify? */
struct sshkey_cert {
	struct sshbuf	*certblob; /* Kept around for use on wire */
	u_int		 type; /* SSH2_CERT_TYPE_USER or SSH2_CERT_TYPE_HOST */
	u_int64_t	 serial;
	char		*key_id;
	u_int		 nprincipals;
	char		**principals;
	u_int64_t	 valid_after, valid_before;
	struct sshbuf	*critical;
	struct sshbuf	*extensions;
	struct sshkey	*signature_key;
};

/* XXX opaquify? */
struct sshkey {
	int	 type;
	int	 flags;
	RSA	*rsa;
	DSA	*dsa;
	int	 ecdsa_nid;	/* NID of curve */
	EC_KEY	*ecdsa;
	u_char	*ed25519_sk;
	u_char	*ed25519_pk;
	struct sshkey_cert *cert;
};

#define	ED25519_SK_SZ	crypto_sign_ed25519_SECRETKEYBYTES
#define	ED25519_PK_SZ	crypto_sign_ed25519_PUBLICKEYBYTES

struct sshkey	*sshkey_new(int);
int		 sshkey_add_private(struct sshkey *);
struct sshkey	*sshkey_new_private(int);
void		 sshkey_free(struct sshkey *);
int		 sshkey_demote(const struct sshkey *, struct sshkey **);
int		 sshkey_equal_public(const struct sshkey *,
    const struct sshkey *);
int		 sshkey_equal(const struct sshkey *, const struct sshkey *);
char		*sshkey_fingerprint(const struct sshkey *,
    int, enum sshkey_fp_rep);
int		 sshkey_fingerprint_raw(const struct sshkey *k,
    int, u_char **retp, size_t *lenp);
const char	*sshkey_type(const struct sshkey *);
const char	*sshkey_cert_type(const struct sshkey *);
int		 sshkey_write(const struct sshkey *, FILE *);
int		 sshkey_read(struct sshkey *, char **);
u_int		 sshkey_size(const struct sshkey *);

int		 sshkey_generate(int type, u_int bits, struct sshkey **keyp);
int		 sshkey_from_private(const struct sshkey *, struct sshkey **);
int	 sshkey_type_from_name(const char *);
int	 sshkey_is_cert(const struct sshkey *);
int	 sshkey_type_is_cert(int);
int	 sshkey_type_plain(int);
int	 sshkey_to_certified(struct sshkey *, int);
int	 sshkey_drop_cert(struct sshkey *);
int	 sshkey_certify(struct sshkey *, struct sshkey *);
int	 sshkey_cert_copy(const struct sshkey *, struct sshkey *);
int	 sshkey_cert_check_authority(const struct sshkey *, int, int,
    const char *, const char **);
int	 sshkey_cert_is_legacy(const struct sshkey *);

int		 sshkey_ecdsa_nid_from_name(const char *);
int		 sshkey_curve_name_to_nid(const char *);
const char *	 sshkey_curve_nid_to_name(int);
u_int		 sshkey_curve_nid_to_bits(int);
int		 sshkey_ecdsa_bits_to_nid(int);
int		 sshkey_ecdsa_key_to_nid(EC_KEY *);
int		 sshkey_ec_nid_to_hash_alg(int nid);
int		 sshkey_ec_validate_public(const EC_GROUP *, const EC_POINT *);
int		 sshkey_ec_validate_private(const EC_KEY *);
const char	*sshkey_ssh_name(const struct sshkey *);
const char	*sshkey_ssh_name_plain(const struct sshkey *);
int		 sshkey_names_valid2(const char *, int);
char		*key_alg_list(int, int);

int	 sshkey_from_blob(const u_char *, size_t, struct sshkey **);
int	 sshkey_fromb(struct sshbuf *, struct sshkey **);
int	 sshkey_froms(struct sshbuf *, struct sshkey **);
int	 sshkey_to_blob(const struct sshkey *, u_char **, size_t *);
int	 sshkey_putb(const struct sshkey *, struct sshbuf *);
int	 sshkey_puts(const struct sshkey *, struct sshbuf *);
int	 sshkey_plain_to_blob(const struct sshkey *, u_char **, size_t *);
int	 sshkey_putb_plain(const struct sshkey *, struct sshbuf *);

int	 sshkey_sign(const struct sshkey *, u_char **, size_t *,
    const u_char *, size_t, u_int);
int	 sshkey_verify(const struct sshkey *, const u_char *, size_t,
    const u_char *, size_t, u_int);

/* for debug */
void	sshkey_dump_ec_point(const EC_GROUP *, const EC_POINT *);
void	sshkey_dump_ec_key(const EC_KEY *);

/* private key parsing and serialisation */
int	sshkey_private_serialize(const struct sshkey *key, struct sshbuf *buf);
int	sshkey_private_deserialize(struct sshbuf *buf,  struct sshkey **keyp);

/* private key file format parsing and serialisation */
int	sshkey_private_to_fileblob(struct sshkey *key, struct sshbuf *blob,
    const char *passphrase, const char *comment,
    int force_new_format, const char *new_format_cipher, int new_format_rounds);
int	sshkey_parse_public_rsa1_fileblob(struct sshbuf *blob,
    struct sshkey **keyp, char **commentp);
int	sshkey_parse_private_fileblob(struct sshbuf *buffer,
    const char *passphrase, const char *filename, struct sshkey **keyp,
    char **commentp);
int	sshkey_parse_private_fileblob_type(struct sshbuf *blob, int type,
    const char *passphrase, struct sshkey **keyp, char **commentp);

#ifdef SSHKEY_INTERNAL
int ssh_rsa_sign(const struct sshkey *key, u_char **sigp, size_t *lenp,
    const u_char *data, size_t datalen, u_int compat);
int ssh_rsa_verify(const struct sshkey *key,
    const u_char *signature, size_t signaturelen,
    const u_char *data, size_t datalen, u_int compat);
int ssh_dss_sign(const struct sshkey *key, u_char **sigp, size_t *lenp,
    const u_char *data, size_t datalen, u_int compat);
int ssh_dss_verify(const struct sshkey *key,
    const u_char *signature, size_t signaturelen,
    const u_char *data, size_t datalen, u_int compat);
int ssh_ecdsa_sign(const struct sshkey *key, u_char **sigp, size_t *lenp,
    const u_char *data, size_t datalen, u_int compat);
int ssh_ecdsa_verify(const struct sshkey *key,
    const u_char *signature, size_t signaturelen,
    const u_char *data, size_t datalen, u_int compat);
int ssh_ed25519_sign(const struct sshkey *key, u_char **sigp, size_t *lenp,
    const u_char *data, size_t datalen, u_int compat);
int ssh_ed25519_verify(const struct sshkey *key,
    const u_char *signature, size_t signaturelen,
    const u_char *data, size_t datalen, u_int compat);
#endif

#ifndef WITH_OPENSSL
#undef RSA
#undef DSA
#undef EC_KEY
#undef EC_GROUP
#undef EC_POINT
#endif /* WITH_OPENSSL */

#endif /* SSHKEY_H */
