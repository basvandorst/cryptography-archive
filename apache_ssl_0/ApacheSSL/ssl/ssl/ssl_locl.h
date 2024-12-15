/* ssl/ssl_locl.h */
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

#ifndef HEADER_SSL_LOCL_H
#define HEADER_SSL_LOCL_H
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "rsa.h"
#include "x509.h"

#if !defined(_LIBC) || defined(NOCONST)
#define const
#endif

#define c2l(c,l)	(l = ((unsigned long)(*((c)++)))     , \
			 l|=(((unsigned long)(*((c)++)))<< 8), \
			 l|=(((unsigned long)(*((c)++)))<<16), \
			 l|=(((unsigned long)(*((c)++)))<<24))

/* NOTE - c is not incremented as per c2l */
#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) { \
			case 8: l2 =((unsigned long)(*(--(c))))<<24; \
			case 7: l2|=((unsigned long)(*(--(c))))<<16; \
			case 6: l2|=((unsigned long)(*(--(c))))<< 8; \
			case 5: l2|=((unsigned long)(*(--(c))));     \
			case 4: l1 =((unsigned long)(*(--(c))))<<24; \
			case 3: l1|=((unsigned long)(*(--(c))))<<16; \
			case 2: l1|=((unsigned long)(*(--(c))))<< 8; \
			case 1: l1|=((unsigned long)(*(--(c))));     \
				} \
			}

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24)&0xff))

#define n2l(c,l)	(l =((unsigned long)(*((c)++)))<<24, \
			 l|=((unsigned long)(*((c)++)))<<16, \
			 l|=((unsigned long)(*((c)++)))<< 8, \
			 l|=((unsigned long)(*((c)++))))

#define l2n(l,c)	(*((c)++)=(unsigned char)(((l)>>24)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)    )&0xff))

/* NOTE - c is not incremented as per l2c */
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 8: *(--(c))=(unsigned char)(((l2)>>24)&0xff); \
			case 7: *(--(c))=(unsigned char)(((l2)>>16)&0xff); \
			case 6: *(--(c))=(unsigned char)(((l2)>> 8)&0xff); \
			case 5: *(--(c))=(unsigned char)(((l2)    )&0xff); \
			case 4: *(--(c))=(unsigned char)(((l1)>>24)&0xff); \
			case 3: *(--(c))=(unsigned char)(((l1)>>16)&0xff); \
			case 2: *(--(c))=(unsigned char)(((l1)>> 8)&0xff); \
			case 1: *(--(c))=(unsigned char)(((l1)    )&0xff); \
				} \
			}

#define n2s(c,s)	(s =((unsigned int)(*((c)++)))<< 8, \
			 s|=((unsigned int)(*((c)++))))
#define s2n(s,c)	(*((c)++)=(unsigned char)(((s)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((s)    )&0xff))

/* LOCAL STUFF */

#ifdef _Windows
#include <winsock.h>
#define Write(a,b,c)	send((a),(b),(c),0)
#define Read(a,b,c)	recv((a),(b),(c),0)
#define Fputc(a,b)	fputc((a),(b))
#else
#define Read(a,b,c)	read((a),(b),(c))
#define Write(a,b,c)	write((a),(b),(c))
#define Fputc(a,b)	fputc((a),(b))
#endif

#define SSL_DECRYPT	0
#define SSL_ENCRYPT	1

#define TWO_BYTE_BIT	0x80
#define SEC_ESC_BIT	0x40
#define TWO_BYTE_MASK	0x7fff
#define THREE_BYTE_MASK	0x3fff

#define INC32(a)	((a)=((a)+1)&0xffffffffL)
#define DEC32(a)	((a)=((a)-1)&0xffffffffL)
#define MAX_MAC_SIZE	20 /* up from 16 for SSLv3 */

#define CHALLENGE_LENGTH	16
/*#define CHALLENGE_LENGTH	32 */
#define MIN_CHALLENGE_LENGTH	16
#define MAX_CHALLENGE_LENGTH	32
#define CONNECTION_ID_LENGTH	16
#define SESSION_ID_LENGTH	16
#define MAX_CERT_CHALLENGE_LENGTH	32
#define MIN_CERT_CHALLENGE_LENGTH	16

/* used in SSL->peer_status */
#define SSL_PEER_NOT_SET	0
#define SSL_PEER_IN_SSL		1
#define SSL_PEER_IN_CERT	2

#ifdef NOIDEA
#define MISSING_IDEA	-l
#else
#define MISSING_IDEA
#endif
#ifdef NORC4 /* 2 less ciphers */
#define MISSING_RC4	-2
#else
#define MISSING_RC4
#endif

#define NUM_CIPHERS	(12 MISSING_IDEA MISSING_RC4)

typedef struct cipher_st
	{
	int valid;
	char *name;
	int (*crypt_init)();
	void (*crypt)();
	void (*hash)();
	unsigned int key_size;
	unsigned int mac_size;
	unsigned int block_size;
	unsigned int key_arg_size;

	unsigned char c1;
	unsigned char c2;
	unsigned char c3;
	unsigned char enc_bits;
	} CIPHER;

typedef struct ssl_conn_st
	{
	int num_ciphers;
	CIPHER *ciphers[NUM_CIPHERS+1];
	CIPHER *cipher;
	unsigned int key_arg_length;
	unsigned char *key_arg;
	unsigned int master_key_length;
	unsigned char *master_key;
	unsigned int key_material_length;
	unsigned char *key_material;
	/* session_id - valid? */
	unsigned int session_id_length;
	unsigned char *session_id;

	struct cert_st *cert;

	unsigned int references;
	unsigned long timeout;
	unsigned long time;
	} CONN;

/*
#define CERT_INVALID		0
#define CERT_PUBLIC_KEY		1
#define CERT_PRIVATE_KEY	2
*/
typedef struct cert_st
	{
	int cert_type;
	int (*public_encrypt)();
	int (*private_decrypt)();
	X509 *x509;
	RSA *publickey; /* when extracted */
	RSA *privatekey;
	int references;
	} CERT;

extern CIPHER ssl_ciphers[];

/*#define MAC_DEBUG	*/

/*#define ERR_DEBUG	*/
/*#define ABORT_DEBUG	*/
/*#define PKT_DEBUG 1   */
/*#define DES_DEBUG	*/
/*#define DES_OFB_DEBUG	*/
/*#define SSL_DEBUG	*/
/*#define RSA_DEBUG	*/ 
/*#define IDEA_DEBUG	*/ 

#define ST_INIT				0x0100
#define SSL_ST_INIT(a)			((a)&ST_INIT)

#define ST_BEFORE			0x01
#define ST_START_ENCRYPTION		(0x02|ST_INIT)
#define ST_OK				0x03

/* client */
#define ST_SEND_CLIENT_HELLO_A		(0x10|ST_INIT)
#define ST_SEND_CLIENT_HELLO_B		(0x11|ST_INIT)
#define ST_GET_SERVER_HELLO_A		(0x12|ST_INIT)
#define ST_GET_SERVER_HELLO_B		(0x13|ST_INIT)
#define ST_SEND_CLIENT_MASTER_KEY_A	(0x14|ST_INIT)
#define ST_SEND_CLIENT_MASTER_KEY_B	(0x15|ST_INIT)
#define ST_SEND_CLIENT_FINISHED_A	(0x18|ST_INIT)
#define ST_SEND_CLIENT_FINISHED_B	(0x19|ST_INIT)
#define ST_SEND_CLIENT_CERTIFICATE_A	(0x1A|ST_INIT)
#define ST_SEND_CLIENT_CERTIFICATE_B	(0x1B|ST_INIT)
#define ST_SEND_CLIENT_CERTIFICATE_C	(0x1C|ST_INIT)
#define ST_SEND_CLIENT_CERTIFICATE_D	(0x1D|ST_INIT)
#define ST_GET_SERVER_VERIFY_A		(0x1E|ST_INIT)
#define ST_GET_SERVER_VERIFY_B		(0x1F|ST_INIT)
#define ST_GET_SERVER_FINISHED_A	(0x20|ST_INIT)
#define ST_GET_SERVER_FINISHED_B	(0x21|ST_INIT)
/* server */
#define ST_GET_CLIENT_HELLO_A		(0x40|ST_INIT)
#define ST_GET_CLIENT_HELLO_B		(0x41|ST_INIT)
#define ST_SEND_SERVER_HELLO_A		(0x42|ST_INIT)
#define ST_SEND_SERVER_HELLO_B		(0x43|ST_INIT)
#define ST_GET_CLIENT_MASTER_KEY_A	(0x44|ST_INIT)
#define ST_GET_CLIENT_MASTER_KEY_B	(0x45|ST_INIT)
#define ST_SEND_SERVER_VERIFY_A		(0x46|ST_INIT)
#define ST_SEND_SERVER_VERIFY_B		(0x47|ST_INIT)
#define ST_GET_CLIENT_FINISHED_A	(0x48|ST_INIT)
#define ST_GET_CLIENT_FINISHED_B	(0x49|ST_INIT)
#define ST_SEND_SERVER_FINISHED_A	(0x4A|ST_INIT)
#define ST_SEND_SERVER_FINISHED_B	(0x4B|ST_INIT)
#define ST_SEND_REQUEST_CERTIFICATE_A	(0x4C|ST_INIT)
#define ST_SEND_REQUEST_CERTIFICATE_B	(0x4D|ST_INIT)
#define ST_SEND_REQUEST_CERTIFICATE_C	(0x4E|ST_INIT)
#define ST_SEND_REQUEST_CERTIFICATE_D	(0x4F|ST_INIT)

#define ST_READ_HEADER			0x80
#define ST_READ_BODY			0x81

#include "ssl.h"
#include "err.h"

/* this is for debug and tracing stuff */
#include "ssl_trc.h"

/* these are now externally visible ... in ssl.h 
extern FILE *SSL_LOG;
extern FILE *SSL_ERR;
*/

#ifdef PKT_DEBUG
void ssl_debug();
#endif

#ifndef NOPROTO
CERT *ssl_cert_new(void);
void ssl_conn_free(SSL *,CONN *c);
void ssl_cert_free(CERT *c);
void ssl_return_error(SSL *s);
RSA *ssl_rsa_extract_public_key(X509 *x);
int ssl_set_certificate(SSL *s, int type, int len, unsigned char *data);
int ssl_set_cert_type(CERT *c, int type);
int ssl_new_conn(SSL *s, int session);
int ssl_get_prev_conn(SSL *s, int len, unsigned char *session);
void ssl_add_hash_conn(SSL *s, CONN *c);
int ssl_rsa_public_decrypt(CERT *c, int len, unsigned char *from, unsigned char *to);
void ssl_print_bytes(FILE *f, int n, char *b);
int ssl_cipher_cmp(CIPHER *a,CIPHER *b);
int ssl_cipher_ptr_cmp(CIPHER **ap,CIPHER **bp);
int ssl_part_read(SSL *s,unsigned long f, int i);
int ssl_do_write(SSL *s);
#else
CERT *ssl_cert_new();
void ssl_conn_free();
void ssl_cert_free();
RSA *ssl_rsa_extract_public_key();
int ssl_set_certificate();
int ssl_set_cert_type();
int ssl_new_conn();
int ssl_get_prev_conn();
void ssl_add_hash_conn();
void ssl_return_error();
int ssl_rsa_public_decrypt();
void ssl_print_bytes();
int ssl_cipher_cmp();
int ssl_cipher_ptr_cmp();
int ssl_part_read();
int ssl_do_write();
#endif

#endif

extern int errno;
