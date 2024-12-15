/* ssl/ssl_stat.c */
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
#include "ssl_locl.h"

char *ssl_state_string(s)
SSL *s;
	{
	char *str;

	switch (s->state)
		{
case ST_BEFORE: str="ST_BEFORE"; break;
case ST_START_ENCRYPTION: str="ST_START_ENCRYPTION"; break;
case ST_OK: str="ST_OK"; break;
case ST_SEND_CLIENT_HELLO_A: str="ST_SEND_CLIENT_HELLO_A"; break;
case ST_SEND_CLIENT_HELLO_B: str="ST_SEND_CLIENT_HELLO_B"; break;
case ST_GET_SERVER_HELLO_A: str="ST_GET_SERVER_HELLO_A"; break;
case ST_GET_SERVER_HELLO_B: str="ST_GET_SERVER_HELLO_B"; break;
case ST_SEND_CLIENT_MASTER_KEY_A: str="ST_SEND_CLIENT_MASTER_KEY_A"; break;
case ST_SEND_CLIENT_MASTER_KEY_B: str="ST_SEND_CLIENT_MASTER_KEY_B"; break;
case ST_SEND_CLIENT_FINISHED_A: str="ST_SEND_CLIENT_FINISHED_A"; break;
case ST_SEND_CLIENT_FINISHED_B: str="ST_SEND_CLIENT_FINISHED_B"; break;
case ST_SEND_CLIENT_CERTIFICATE_A: str="ST_SEND_CLIENT_CERTIFICATE_A"; break;
case ST_SEND_CLIENT_CERTIFICATE_B: str="ST_SEND_CLIENT_CERTIFICATE_B"; break;
case ST_SEND_CLIENT_CERTIFICATE_C: str="ST_SEND_CLIENT_CERTIFICATE_C"; break;
case ST_SEND_CLIENT_CERTIFICATE_D: str="ST_SEND_CLIENT_CERTIFICATE_D"; break;
case ST_GET_SERVER_VERIFY_A: str="ST_GET_SERVER_VERIFY_A"; break;
case ST_GET_SERVER_VERIFY_B: str="ST_GET_SERVER_VERIFY_B"; break;
case ST_GET_SERVER_FINISHED_A: str="ST_GET_SERVER_FINISHED_A"; break;
case ST_GET_SERVER_FINISHED_B: str="ST_GET_SERVER_FINISHED_B"; break;
case ST_GET_CLIENT_HELLO_A: str="ST_GET_CLIENT_HELLO_A"; break;
case ST_GET_CLIENT_HELLO_B: str="ST_GET_CLIENT_HELLO_B"; break;
case ST_SEND_SERVER_HELLO_A: str="ST_SEND_SERVER_HELLO_A"; break;
case ST_SEND_SERVER_HELLO_B: str="ST_SEND_SERVER_HELLO_B"; break;
case ST_GET_CLIENT_MASTER_KEY_A: str="ST_GET_CLIENT_MASTER_KEY_A"; break;
case ST_GET_CLIENT_MASTER_KEY_B: str="ST_GET_CLIENT_MASTER_KEY_B"; break;
case ST_SEND_SERVER_VERIFY_A: str="ST_SEND_SERVER_VERIFY_A"; break;
case ST_SEND_SERVER_VERIFY_B: str="ST_SEND_SERVER_VERIFY_B"; break;
case ST_GET_CLIENT_FINISHED_A: str="ST_GET_CLIENT_FINISHED_A"; break;
case ST_GET_CLIENT_FINISHED_B: str="ST_GET_CLIENT_FINISHED_B"; break;
case ST_SEND_SERVER_FINISHED_A: str="ST_SEND_SERVER_FINISHED_A"; break;
case ST_SEND_SERVER_FINISHED_B: str="ST_SEND_SERVER_FINISHED_B"; break;
case ST_SEND_REQUEST_CERTIFICATE_A: str="ST_SEND_REQUEST_CERTIFICATE_A"; break;
case ST_SEND_REQUEST_CERTIFICATE_B: str="ST_SEND_REQUEST_CERTIFICATE_B"; break;
case ST_SEND_REQUEST_CERTIFICATE_C: str="ST_SEND_REQUEST_CERTIFICATE_C"; break;
case ST_SEND_REQUEST_CERTIFICATE_D: str="ST_SEND_REQUEST_CERTIFICATE_D"; break;
default:	str="UNKNOWN";
		}
	return(str);
	}

char *ssl_rstate_string(s)
SSL *s;
	{
	char *str;

	switch (s->rstate)
		{
	case ST_READ_HEADER: str="ST_READ_HEADER"; break;
	case ST_READ_BODY: str="ST_READ_BODY"; break;
	default: str="unknown"; break;
		}
	return(str);
	}
