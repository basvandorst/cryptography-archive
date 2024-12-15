/* ssl/ssl_conn.c */
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

#include <stdio.h>
#include "md5.h"
#include "lhash.h"
#include "rand.h"
#include "ssl_locl.h"

int ssl_new_conn(s, session)
SSL *s;
int session;
	{
	CONN *con,*r;
	int i;

	/* leave old connection hanging around */
	if (s->conn != NULL)
		{
		ssl_conn_free(s,s->conn);
		s->conn=NULL;
		}

	con=(CONN *)malloc(sizeof(CONN));
	if (con == NULL)
		{
		SSLerr(SSL_F_GET_NEW_CONN,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	con->num_ciphers=0;
	for (i=0; i<NUM_CIPHERS; i++) con->ciphers[i]=NULL;
	con->cipher=NULL;
	con->key_arg_length=0;
	con->key_arg=NULL;
	con->master_key_length=0;
	con->master_key=NULL;
	con->key_material_length=0;
	con->key_material=NULL;

	if (session)
		{
		con->session_id_length=SESSION_ID_LENGTH;
		con->session_id=(unsigned char *)malloc(SESSION_ID_LENGTH);
		if (con->session_id == NULL)
			{
			SSLerr(SSL_F_GET_NEW_CONN,ERR_R_MALLOC_FAILURE);
			return(0);
			}

		for (;;)
			{
			RAND_bytes(con->session_id,SESSION_ID_LENGTH);
			r=(CONN *)lh_retrieve(s->ctx->connections,
				(char *)con);
			if (r == NULL) break;
			/* else - woops a session_id match */
			}
		}
	else
		{
		con->session_id_length=0;
		con->session_id=NULL;
		}

	con->cert=NULL;
	con->references=1;
	con->timeout=100;
	con->time=time(NULL);
	s->conn=con;
	return(1);
	}

int ssl_get_prev_conn(s, len, session)
SSL *s;
int len;
unsigned char *session;
	{
	CONN *ret,data;

	/* conn_init();*/
	data.session_id_length=len;
	data.session_id=session;
	if ((ret=(CONN *)lh_retrieve(s->ctx->connections,(char *)&data))
		== NULL)
		return(0);
#ifdef SSL_DEBUG
	SSL_TRACE(SSL_ERR,"GOT ONE\n");
#endif
	if (ret->time+ret->timeout < time(NULL)) /* timeout */
		{
#ifdef SSL_DEBUG
		SSL_TRACE(SSL_ERR,"TIMEOUT ON SESSION\n");
#endif
		ssl_conn_free(s,ret);
		return(0);
		}

	/* ret->time=time(NULL); */ /* rezero timeout? */
	ret->references++;
	/* again, just leave the connection */
	if (s->conn != NULL) ssl_conn_free(s,s->conn);
	s->conn=ret;
	return(1);
	}

void ssl_add_hash_conn(s,c)
SSL *s;
CONN *c;
	{
	/* conn_init(); */
	lh_insert(s->ctx->connections,(char *)c);
	c->references++;
	}

void ssl_conn_free(s,c)
SSL *s;
CONN *c;
	{
	CONN *r;

	if (--c->references > 0) return;

	if ((c->session_id != NULL) && (s != NULL))
		{
		r=(CONN *)lh_delete(s->ctx->connections,(char *)c);
		if (r == NULL)
			{
#ifdef SSL_DEBUG
			SSL_TRACE(SSL_ERR,"freeing a conn that is not hashed\n");
#endif
			return;
			}
		}
	if (c->key_arg != NULL)		free(c->key_arg);
	if (c->master_key != NULL)	free(c->master_key);
	if (c->key_material != NULL)	free(c->key_material);
	if (c->session_id != NULL)	free(c->session_id);
	if (c->cert != NULL)		ssl_cert_free(c->cert);
	free(c);
	}
