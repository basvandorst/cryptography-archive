/* lib/ssl/ssl.h */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
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
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
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

#ifndef HEADER_SSL_H 
#define HEADER_SSL_H 

/* Protocol Version Codes */
#define SSL_CLIENT_VERSION	0x0002
#define SSL_SERVER_VERSION	0x0002

/* Protocol Message Codes */
#define SSL_MT_ERROR			0
#define SSL_MT_CLIENT_HELLO		1
#define SSL_MT_CLIENT_MASTER_KEY	2
#define SSL_MT_CLIENT_FINISHED		3
#define SSL_MT_SERVER_HELLO		4
#define SSL_MT_SERVER_VERIFY		5
#define SSL_MT_SERVER_FINISHED		6
#define SSL_MT_REQUEST_CERTIFICATE	7
#define SSL_MT_CLIENT_CERTIFICATE	8

/* Error Message Codes */
#define SSL_PE_NO_CIPHER		0x0001
#define SSL_PE_NO_CERTIFICATE		0x0002
#define SSL_PE_BAD_CERTIFICATE		0x0004
#define SSL_PE_UNSUPPORTED_CERTIFICATE_TYPE 0x0006

/* Cipher Kind Values */
#define SSL_CK_NULL_WITH_MD5			0x00,0x00,0x00 /* v3 */
#define SSL_CK_RC4_128_WITH_MD5			0x01,0x00,0x80
#define SSL_CK_RC4_128_EXPORT40_WITH_MD5	0x02,0x00,0x80
#define SSL_CK_RC2_128_CBC_WITH_MD5		0x03,0x00,0x80
#define SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5	0x04,0x00,0x80
#define SSL_CK_IDEA_128_CBC_WITH_MD5		0x05,0x00,0x80
#define SSL_CK_DES_64_CBC_WITH_MD5		0x06,0x00,0x40
#define SSL_CK_DES_64_CBC_WITH_SHA		0x06,0x01,0x40 /* v3 */
#define SSL_CK_DES_192_EDE3_CBC_WITH_MD5	0x07,0x00,0xc0
#define SSL_CK_DES_192_EDE3_CBC_WITH_SHA	0x07,0x01,0xc0 /* v3 */

#define SSL_CK_DES_64_CFB64_WITH_MD5_1		0xff,0x08,0x00 /* SSLeay */
#define SSL_CK_NULL				0xff,0x08,0x10 /* SSLeay */

/* text strings for the ciphers */
#define SSL_TXT_NULL_WITH_MD5			"NULL-MD5"
#define SSL_TXT_RC4_128_WITH_MD5		"RC4-MD5"
#define SSL_TXT_RC4_128_EXPORT40_WITH_MD5	"EXP-RC4-MD5"
#define SSL_TXT_RC2_128_CBC_WITH_MD5		"RC2-MD5"
#define SSL_TXT_RC2_128_CBC_EXPORT40_WITH_MD5	"EXP-RC2-MD5"
#define SSL_TXT_IDEA_128_CBC_WITH_MD5		"IDEA-CBC-MD5"
#define SSL_TXT_DES_64_CBC_WITH_MD5		"DES-CBC-MD5"
#define SSL_TXT_DES_64_CBC_WITH_SHA		"DES-CBC-SHA"
#define SSL_TXT_DES_192_EDE3_CBC_WITH_MD5	"DES-CBC3-MD5"
#define SSL_TXT_DES_192_EDE3_CBC_WITH_SHA	"DES-CBC3-SHA"

#define SSL_TXT_DES_64_CFB64_WITH_MD5_1		"DES-CFB-M1"
#define SSL_TXT_NULL				"NULL"

/* Certificate Type Codes */
#define SSL_CT_X509_CERTIFICATE			0x01

/* Authentication Type Code */
#define SSL_AT_MD5_WITH_RSA_ENCRYPTION		0x01

/* Upper/Lower Bounds */
#define SSL_MAX_MASTER_KEY_LENGTH_IN_BITS	256
#define SSL_MAX_SESSION_ID_LENGTH_IN_BYTES	16
#define SSL_MIN_RSA_MODULUS_LENGTH_IN_BYTES	64
#define SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER	32767 
#define SSL_MAX_RECORD_LENGTH_3_BYTE_HEADER	16383 /**/

#ifndef HEADER_X509_H
#include "x509.h"
#endif

#ifdef HEADER_X509_H
#define SSL_FILETYPE_PEM	X509_FILETYPE_PEM
#define SSL_FILETYPE_ASN1	X509_FILETYPE_ASN1
#else
#define SSL_FILETYPE_PEM	1
#define SSL_FILETYPE_ASN1	2
#endif

typedef struct ssl_ctx_st
	{
#ifdef HEADER_X509_H
	CERTIFICATE_CTX *cert;
#else
	char *cert;
#endif
#ifdef HEADER_LHASH_H
	LHASH *connections;
#else
	char *connections;
#endif

	int references;

	/* used to do 'reverse' ssl */
	int reverse;

	/* used by client half*/
	char *cipher;
	unsigned int challenge_length;
	unsigned char *challenge;
	unsigned int master_key_length;
	unsigned char *master_key;
	unsigned int key_arg_length;
	unsigned char *key_arg;
	/* used by server */
	unsigned int conn_id_length;
	unsigned char *conn_id;
	} SSL_CTX;

typedef struct ssl_st
	{
	/* There are 2 fd's even though they are normally both the
	 * same.  This is so data can be read and written to a file,
	 * don't ask, it's a special case hack */
	int rfd; /* used by SSL_read */
	int wfd; /* used by SSL_write */
	int version;	/* procol version */

	int state;	/* where we are */
	unsigned int state_conn_id_length;	/* tmp */
	unsigned int state_cert_type;		/* tmp */
	unsigned int state_cert_length;		/* tmp */
	unsigned int state_csl; 		/* tmp */

	unsigned int state_clear; 		/* tmp */
	unsigned int state_enc; 		/* tmp */

	unsigned char *state_ccl;		/* tmp */

	unsigned int state_cipher_spec_length;
	unsigned int state_session_id_length;

	unsigned int state_clen;
	unsigned int state_rlen;

	int rstate;	/* where we are when reading */

	unsigned char *init_buf;/* buffer used during init */
	int init_num;	/* amount read/written */
	int init_off;	/* amount read/written */

	int read_ahead;
	int escape;
	int three_byte_header;
	int send;		/* direction of packet */
	int clear_text;		/* clear text */
	int hit;		/* reusing a previous session */

	/* non-blocking io info, used to make sure the same args were passwd */
	int wpend_tot;
	char *wpend_buf;

	int wpend_off;		/* offset to data to write */
	int wpend_len;  	/* number of bytes passwd to write */
	int wpend_ret;  	/* number of bytes to return to caller */

	int rpend_off;		/* offset to read position */
	int rpend_len;		/* number of bytes to read */

	/* buffer raw data */
	int rbuf_left;
	int rbuf_offs;
	unsigned char *rbuf;
	unsigned char *wbuf;

	/* used internally by ssl_read to talk to read_n */
	unsigned char *packet;
	unsigned int packet_length;

	/* set by read_n */
	unsigned int length;
	unsigned int padding;
	unsigned int ract_data_length; /* Set when things are encrypted. */
	unsigned int wact_data_length; /* Set when things are decrypted. */
	unsigned char *ract_data;
	unsigned char *wact_data;
	unsigned char *mac_data;
	unsigned char *pad_data;

	/* crypto */
	int num_pref_cipher;
	char **pref_cipher;	/* char ** is malloced and the rest is
				 * on malloced block pointed to by
				 * perf_cipher[0] */
	char *crypt_state;	/* cryptographic state */
	unsigned char *read_key;
	unsigned char *write_key;

	/* conn info */
#ifdef HEADER_SSL_LOCL_H
	CONN *conn;
	CERT *cert;
#else
	char *conn;
	char *cert;
#endif

	unsigned int challenge_length;
	unsigned char *challenge;
	unsigned int conn_id_length;
	unsigned char *conn_id;

	/* packet specs */
	/* send is true for send; false for recieve */

	unsigned char *write_ptr;	/* used to point to the start due to
					 * 2/3 byte header. */

	unsigned long read_sequence;
	unsigned long write_sequence;

	/* special stuff */
	int trust_level;	/* not used yet */
	int peer_status;	/* 0, not set, 1 in ->perr, 2 in
				 * ->conn->cert->x509 */
#ifdef HEADER_X509_H
	X509 *peer;
#else
	char *peer;		/* actually a X509 * */
#endif
	int verify_mode;	/* 0 don't care about verify failure.
				 * 1 fail if verify fails */
	int (*verify_callback)(); /* fail if callback returns 0 */

	SSL_CTX *ctx;
	} SSL;

#define SSL_VERIFY_NONE			0x00
#define SSL_VERIFY_PEER			0x01
#define SSL_VERIFY_FAIL_IF_NO_PEER_CERT	0x02

#define SSL_RWERR_BAD_WRITE_RETRY	(-2)
#define SSL_RWERR_BAD_MAC_DECODE	(-3)
#define SSL_RWERR_INTERNAL_ERROR	(-4) /* should not get this one */


#define SSL_set_default_verify_paths(ctx) \
		X509_set_default_verify_paths((ctx)->cert)
#define SSL_load_verify_locations(ctx,CAfile,CApath) \
		X509_load_verify_locations((ctx)->cert,(CAfile),(CApath))

#ifndef NOPROTO
SSL_CTX *SSL_CTX_new(void );
void	SSL_CTX_free(SSL_CTX *);
int	SSL_accept(SSL *s);
void	SSL_clear(SSL *s);
int	SSL_connect(SSL *s);
int	SSL_copy_session_id(SSL *to, SSL *from);
void	SSL_debug(char *file);
void	SSL_flush_connections(void);
void	SSL_free(SSL *s);
char *	SSL_get_cipher(SSL *s);
int	SSL_get_fd(SSL *s);
char *	SSL_get_pref_cipher(SSL *s, int n);
char *	SSL_get_shared_ciphers(SSL *s, char *buf, int len);
int	SSL_get_read_ahead(SSL * s);
long	SSL_get_time(SSL * s);
long	SSL_get_timeout(SSL * s);
SSL_CTX *SSL_get_SSL_CTX(SSL *s);
int	SSL_is_init_finished(SSL *s);
SSL *	SSL_new(SSL_CTX *);
int	SSL_pending(SSL *s);
int	SSL_read(SSL *s, char *buf, unsigned int len);
void	SSL_set_fd(SSL *s, int fd);
int	SSL_set_pref_cipher(SSL *s, char *str);
void	SSL_set_read_ahead(SSL * s, int yes);
int	SSL_set_timeout(SSL * s, int t);
void	SSL_set_verify(SSL *s, int mode, int (*callback) ());
int	SSL_use_RSAPrivateKey(SSL *ssl, RSA *rsa);
int	SSL_use_RSAPrivateKey_ASN1(SSL *ssl, unsigned char *d, long len);
int	SSL_use_RSAPrivateKey_file(SSL *ssl, char *file, int type);
int	SSL_use_certificate(SSL *ssl, X509 *x);
int	SSL_use_certificate_ASN1(SSL *ssl, int len, unsigned char *d);
int	SSL_use_certificate_file(SSL *ssl, char *file, int type);
int	SSL_write(SSL *s, char *buf, unsigned int len);
void	ERR_load_SSL_strings(void );
void	SSL_load_error_strings(void );
char * 	ssl_state_string(SSL *s);
char * 	ssl_rstate_string(SSL *s);
#ifdef HEADER_X509_H
X509 *	SSL_get_peer_certificate(SSL *s);
#else
char *	SSL_get_peer_certificate(SSL *s);
#endif

#else

SSL_CTX *SSL_CTX_new();
void	SSL_CTX_free();
int	SSL_accept();
void	SSL_clear();
int	SSL_connect();
int	SSL_copy_session_id();
void	SSL_debug();
void	SSL_flush_connections();
void	SSL_free();
char *	SSL_get_cipher();
int	SSL_get_fd();
char *	SSL_get_pref_cipher();
int	SSL_get_read_ahead();
long	SSL_get_time();
long	SSL_get_timeout();
int	SSL_is_init_finished();
SSL *	SSL_new();
int	SSL_pending();
int	SSL_read();
void	SSL_set_fd();
int	SSL_set_pref_cipher();
char *	SSL_get_shared_ciphers();
void	SSL_set_read_ahead();
int	SSL_set_timeout();
void	SSL_set_verify();
int	SSL_use_RSAPrivateKey();
int	SSL_use_RSAPrivateKey_ASN1();
int	SSL_use_RSAPrivateKey_file();
int	SSL_use_certificate();
int	SSL_use_certificate_ASN1();
int	SSL_use_certificate_file();
int	SSL_write();
void	ERR_load_SSL_strings();
void	SSL_load_error_strings();
char * 	ssl_state_string();
char * 	ssl_rstate_string();
#ifdef HEADER_X509_H
X509 *	SSL_get_peer_certificate();
#else
char *	SSL_get_peer_certificate();
#endif
#endif

/* tjh added these two dudes to enable external control
 * of debug and trace logging
 */
extern FILE *SSL_ERR;
extern FILE *SSL_LOG;

/* BEGIN ERROR CODES */
/* Error codes for the SSL functions. */

/* Function codes. */
#define SSL_F_CLIENT_CERTIFICATE			 100
#define SSL_F_CLIENT_HELLO				 101
#define SSL_F_CLIENT_MASTER_KEY				 102
#define SSL_F_GET_CLIENT_FINISHED			 103
#define SSL_F_GET_CLIENT_HELLO				 104
#define SSL_F_GET_CLIENT_MASTER_KEY			 105
#define SSL_F_GET_NEW_CONN				 106
#define SSL_F_GET_SERVER_FINISHED			 107
#define SSL_F_GET_SERVER_HELLO				 108
#define SSL_F_GET_SERVER_VERIFY				 109
#define SSL_F_REQUEST_CERTIFICATE			 110
#define SSL_F_SERVER_HELLO				 111
#define SSL_F_SSL_ACCEPT				 112
#define SSL_F_SSL_CERT_NEW				 113
#define SSL_F_SSL_CERT_RSA_PRIVATE_DECRYPT		 114
#define SSL_F_SSL_CERT_RSA_PRIVATE_ENCRYPT		 115
#define SSL_F_SSL_CERT_RSA_PUBLIC_DECRYPT		 116
#define SSL_F_SSL_CERT_RSA_PUBLIC_ENCRYPT		 117
#define SSL_F_SSL_CONNECT				 118
#define SSL_F_SSL_ENC_DES_CBC_INIT			 119
#define SSL_F_SSL_ENC_DES_EDE3_CBC_INIT			 120
#define SSL_F_SSL_ENC_IDEA_CBC_INIT			 121
#define SSL_F_SSL_ENC_NULL_INIT				 122
#define SSL_F_SSL_ENC_RC4_INIT				 123
#define SSL_F_SSL_NEW					 124
#define SSL_F_SSL_READ					 125
#define SSL_F_SSL_SET_CERTIFICATE			 126
#define SSL_F_SSL_USE_CERTIFICATE_ASN1			 127
#define SSL_F_SSL_USE_CERTIFICATE_FILE			 128
#define SSL_F_SSL_USE_RSAPRIVATEKEY			 129
#define SSL_F_SSL_USE_RSAPRIVATEKEY_ASN1		 130
#define SSL_F_SSL_USE_RSAPRIVATEKEY_FILE		 131
#define SSL_F_SSL_WRITE					 132

/* Reason codes. */
#define SSL_R_BAD_AUTHENTICATION_TYPE			 100
#define SSL_R_BAD_CHECKSUM				 101
#define SSL_R_BAD_MAC_DECODE				 102
#define SSL_R_BAD_RESPONSE_ARGUMENT			 103
#define SSL_R_BAD_SESSION_ID_LENGTH			 104
#define SSL_R_BAD_SSL_FILETYPE				 105
#define SSL_R_BAD_STATE					 106
#define SSL_R_BAD_WRITE_RETRY				 107
#define SSL_R_CHALLENGE_IS_DIFFERENT			 108
#define SSL_R_CIPHER_TABLE_SRC_ERROR			 109
#define SSL_R_CONNECTION_ID_IS_DIFFERENT		 110
#define SSL_R_INVALID_CHALLENGE_LENGTH			 111
#define SSL_R_NO_CERTIFICATE_SET			 112
#define SSL_R_NO_CERTIFICATE_SPECIFIED			 113
#define SSL_R_NO_CIPHER_LIST				 114
#define SSL_R_NO_CIPHER_MATCH				 115
#define SSL_R_NO_CIPHER_WE_TRUST			 116
#define SSL_R_NO_PRIVATEKEY				 117
#define SSL_R_NO_PUBLICKEY				 118
#define SSL_R_NULL_SSL_CTX				 119
#define SSL_R_PEER_DID_NOT_RETURN_A_CERTIFICATE		 120
#define SSL_R_PEER_ERROR				 121
#define SSL_R_PEER_ERROR_CERTIFICATE			 122
#define SSL_R_PEER_ERROR_NO_CIPHER			 123
#define SSL_R_PEER_ERROR_UNSUPPORTED_CERTIFICATE_TYPE	 124
#define SSL_R_PERR_ERROR_NO_CERTIFICATE			 125
#define SSL_R_PUBLIC_KEY_ENCRYPT_ERROR			 126
#define SSL_R_READ_WRONG_PACKET_TYPE			 127
#define SSL_R_REVERSE_CONN_ID_LENGTH_IS_WRONG		 128
#define SSL_R_REVERSE_KEY_ARG_LENGTH_IS_WRONG		 129
#define SSL_R_REVERSE_MASTER_KEY_LENGTH_IS_WRONG	 130
#define SSL_R_SESSION_ID_IS_DIFFERENT			 131
#define SSL_R_SHORT_READ				 132
#define SSL_R_UNKNOWN_REMOTE_ERROR_TYPE			 133
#define SSL_R_UNKNOWN_STATE				 134

#endif
