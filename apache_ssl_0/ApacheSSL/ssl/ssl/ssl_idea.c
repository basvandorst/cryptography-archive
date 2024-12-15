/* ssl/ssl_idea.c */
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

/* Andreas Bogk <bogk@inf.fu-berlin.de> send patches for IDEA 
 * just before I released this 'update'.  His code was amazingly
 * similar to mine.  So a many thank for the effort but unfortunatly since my
 * code was in the 'distribution' first, I'll ship my version.
 * Sorry Andreas :-).
 */

#ifndef NOIDEA
#include <stdio.h>
#include "idea.h"
#include "md5.h"
#include "ssl_locl.h"
#include "ssl_idea.h"

int ssl_enc_idea_cbc_init(s, client)
SSL *s;
int client;
	{
	MD5_CTX md5s;
	unsigned char *md5d1,*md5d2;
	IDEA_CBC_STATE *is;
	IDEA_KEY_SCHEDULE id;
	CONN *c;

	if (s->crypt_state == NULL)
		s->crypt_state=(char *)malloc(sizeof(IDEA_CBC_STATE));
	if (s->crypt_state == NULL) goto err;
	is=(IDEA_CBC_STATE *)s->crypt_state;

	c=s->conn;
	if (c->key_material != NULL) free(c->key_material);
	md5d1=c->key_material=(unsigned char *)malloc(MD5_DIGEST_LENGTH*2);
	if (c->key_material == NULL) goto err;
	c->key_material_length=MD5_DIGEST_LENGTH*2;
	md5d2=&(c->key_material[MD5_DIGEST_LENGTH]);
	
	MD5_Init(&md5s);
	MD5_Update(&md5s,s->conn->master_key,s->conn->master_key_length);
	MD5_Update(&md5s,(unsigned char *)"0",1);
	MD5_Update(&md5s,s->challenge,s->challenge_length);
	MD5_Update(&md5s,s->conn_id,s->conn_id_length);
	MD5_Final(md5d1,&md5s);

	MD5_Init(&md5s);
	MD5_Update(&md5s,s->conn->master_key,s->conn->master_key_length);
	MD5_Update(&md5s,(unsigned char *)"1",1);
	MD5_Update(&md5s,s->challenge,s->challenge_length);
	MD5_Update(&md5s,s->conn_id,s->conn_id_length);
	MD5_Final(md5d2,&md5s);

	if (client)
		{
		idea_set_encrypt_key(md5d1,&id);
		idea_set_decrypt_key(&id,&(is->idea_read_ks));
		idea_set_encrypt_key(md5d2,&(is->idea_write_ks));
		s->read_key=md5d1;
		s->write_key=md5d2;
		}
	else
		{
		idea_set_encrypt_key(md5d2,&id);
		idea_set_decrypt_key(&id,&(is->idea_read_ks));
		idea_set_encrypt_key(md5d1,&(is->idea_write_ks));
		s->read_key=md5d2;
		s->write_key=md5d1;
		}

	memset((char *)&(id.data[0][0]),0,sizeof(id));
	memcpy(is->read_iv,s->conn->key_arg,IDEA_BLOCK);
	memcpy(is->write_iv,s->conn->key_arg,IDEA_BLOCK);
	return(1);
err:
	SSLerr(SSL_F_SSL_ENC_IDEA_CBC_INIT,ERR_R_MALLOC_FAILURE);
	return(0);
	}

/* read/writes from s->mac_data using length for encrypt and decrypt.
 * it sets the s->padding, s->length and s->pad_data ptr if we
 * are encrypting */
void ssl_enc_idea_cbc(s)
SSL *s;
	{
	IDEA_CBC_STATE *is;
	unsigned long l;

	is=(IDEA_CBC_STATE *)s->crypt_state;
	l=s->length;
	l=(l+(IDEA_BLOCK-1))/IDEA_BLOCK*IDEA_BLOCK;

#ifdef IDEA_DEBUG
	SSL_TRACE(SSL_ERR,"IDEA CBC length=%d\n",s->length);
	ssl_print_bytes(SSL_ERR,l,s->mac_data);
	SSL_TRACE(SSL_ERR,"\n");
#endif

	if (s->send)
		idea_cbc_encrypt(s->mac_data,s->mac_data,(unsigned long)l,
			&(is->idea_write_ks),is->write_iv,IDEA_ENCRYPT);
	else
		idea_cbc_encrypt(s->mac_data,s->mac_data,(unsigned long)l,
			&(is->idea_read_ks),is->read_iv,IDEA_DECRYPT);

#ifdef IDEA_DEBUG
	ssl_print_bytes(SSL_ERR,l,s->mac_data);
	SSL_TRACE(SSL_ERR,"\n");
#endif

	}

#endif
