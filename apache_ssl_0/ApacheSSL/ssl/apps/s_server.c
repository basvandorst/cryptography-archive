/* apps/s_server.c */
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "s_net.h"
#include "x509.h"
#include "ssl.h"
#include "apps.h"
#include "s_apps.h"
#include "err.h"
#include "pem.h"

#ifndef NOPROTO
static int body(char *hostname, int s);
static void close_accept_socket(void );
static void usage(void);
static int init_ssl_connection(SSL *s);
#else
static int body();
static void close_accept_socket();
static void usage();
static int init_ssl_connection();
#endif

#define BUFSIZZ	10*1024
static int accept_socket=-1;

#define TEST_CERT	"server.pem"
#define PROG		s_server_main

extern int verify_depth;
extern int verify_error;

int verify=SSL_VERIFY_NONE;
char *cert_file=TEST_CERT,*key_file=NULL;
static int nbio=0;
static SSL_CTX *ctx=NULL;


static void usage()
	{
	fprintf(stderr,"usage: server args\n");
	fprintf(stderr,"\n");
	fprintf(stderr," -port arg     - port to connect to (default is %d\n",PORT);
	fprintf(stderr," -verify arg   - turn on peer certificate verification\n");
	fprintf(stderr," -Verify arg   - turn on peer certificate verification, must have a cert.\n");
	fprintf(stderr," -cert arg     - certificate file to use, PEM format assumed\n");
	fprintf(stderr,"                 (default is %s)\n",TEST_CERT);
	fprintf(stderr," -key arg      - RSA file to use, PEM format assumed, in cert file if\n");
	fprintf(stderr,"                 not specified (default is %s)\n",TEST_CERT);
	fprintf(stderr," -nbio         - Run with non-blocking IO\n");
	fprintf(stderr," -CApath arg   - PEM format directory of CA's\n");
	fprintf(stderr," -CAfile arg   - PEM format file of CA's\n");
	}

int MAIN(argc, argv)
int argc;
char *argv[];
	{
	int port=PORT;
	char *CApath=NULL,*CAfile=NULL;
	int badop=0;
	int ret=1;

	ctx=SSL_CTX_new();
	if (ctx == NULL)
		goto end;

	verify_depth=0;
	verify_error=VERIFY_OK;
	nbio=0;

	argc--;
	argv++;

	while (argc >= 1)
		{
		if	(strcmp(*argv,"-port") == 0)
			{
			if (--argc < 1) goto bad;
			port=atoi(*(++argv));
			if (port == 0) goto bad;
			}
		else if	(strcmp(*argv,"-verify") == 0)
			{
			verify=SSL_VERIFY_PEER;
			if (--argc < 1) goto bad;
			verify_depth=atoi(*(++argv));
			fprintf(stderr,"verify depth is %d\n",verify_depth);
			}
		else if	(strcmp(*argv,"-Verify") == 0)
			{
			verify=SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
			if (--argc < 1) goto bad;
			verify_depth=atoi(*(++argv));
			fprintf(stderr,"verify depth is %d, must return a certificate\n",verify_depth);
			}
		else if	(strcmp(*argv,"-cert") == 0)
			{
			if (--argc < 1) goto bad;
			cert_file= *(++argv);
			}
		else if	(strcmp(*argv,"-key") == 0)
			{
			if (--argc < 1) goto bad;
			key_file= *(++argv);
			}
		else if	(strcmp(*argv,"-CApath") == 0)
			{
			if (--argc < 1) goto bad;
			CApath= *(++argv);
			}
		else if	(strcmp(*argv,"-CAfile") == 0)
			{
			if (--argc < 1) goto bad;
			CAfile= *(++argv);
			}
#ifdef FIONBIO	
		else if	(strcmp(*argv,"-nbio") == 0)
			{ nbio=1; }
#endif
		else
			{
			fprintf(stderr,"unknown option %s\n",*argv);
			badop=1;
			break;
			}
		argc--;
		argv++;
		}
	if (badop)
		{
bad:
		usage();
		goto end;
		}

	if (cert_file == NULL)
		{
		fprintf(stderr,"You must specify a certificate file for the server to use\n");
		goto end;
		}
	SSL_load_error_strings();

	SSL_debug("server.log");
	if ((!SSL_load_verify_locations(ctx,CAfile,CApath)) ||
		(!SSL_set_default_verify_paths(ctx)))
		{
		fprintf(stderr,"X509_load_verify_locations\n");
		ERR_print_errors(stderr);
		goto end;
		}

	fprintf(stderr,"ACCEPT\n");
	do_server(port,NULL,body);
	ret=0;
end:
	if (ctx != NULL) SSL_CTX_free(ctx);
	EXIT(ret);
	}

static int body(hostname, s)
char *hostname;
int s;
	{
	char buf[BUFSIZZ];
	fd_set readfds;
	int ret=1,width;
	int l,k,i=1;
	static SSL *con=NULL;

#ifdef FIONBIO	
	if (nbio)
		{
		fprintf(stderr,"turning on non blocking io\n");
		ioctl(s,FIONBIO,&i);
		}
#endif

	fprintf(stderr,"CONNECTED\n");
	if (con == NULL)
		{
		con=(SSL *)SSL_new(ctx);
		SSL_set_verify(con,verify,verify_callback);
		if (!set_cert_stuff(con,cert_file,key_file))
			{
			ret=0;
			goto err;
			}
		}
	SSL_clear(con);
	SSL_set_fd(con,s);
	verify_error=VERIFY_OK;

	width=s+1;
	for (;;)
		{
		FD_ZERO(&readfds);
		FD_SET(fileno(stdin),&readfds);
		FD_SET(s,&readfds);
		select(width,&readfds,NULL,NULL,NULL);
		if (FD_ISSET(fileno(stdin),&readfds))
			{
			i=read(fileno(stdin),buf,BUFSIZZ);
			if ((i <= 0) || (buf[0] == 'Q'))
				{
				fprintf(stderr,"DONE\n");
				close_accept_socket();
				ret=1;
				exit(0);
				goto err;
				}
			l=k=0;
			for (;;)
				{
				/* should do a select for the write */
				k=SSL_write(con,&(buf[l]),(unsigned int)i);
				if (nbio && should_retry(k))
					{
					fprintf(stderr,"Write BLOCK\n");
					continue;
					}
				if (k <= 0)
					{
					ERR_print_errors(stderr);
					fprintf(stderr,"DONE\n");
					ret=1;
					goto err;
					}
				l+=k;
				i-=k;
				if (i <= 0) break;
				}
			}
		if (FD_ISSET(s,&readfds))
			{
			if (!SSL_is_init_finished(con))
				{
				i=init_ssl_connection(con);
				
				if (i < 0)
					{
					ret=0;
					goto err;
					}
				else if (i == 0)
					{
					ret=1;
					goto err;
					}
				}
			else
				{
				i=SSL_read(con,(char *)buf,BUFSIZZ);
				if (nbio && should_retry(i))
					{
					fprintf(stderr,"Read BLOCK\n");
					}
				else if (i <= 0)
					{
					ERR_print_errors(stderr);
					fprintf(stderr,"DONE\n");
					ret=1;
					goto err;
					}
				else
					write(fileno(stdout),buf,
						(unsigned int)i);
				}
			}
		}
err:
/*	if (con != NULL) SSL_free(con); */
	fprintf(stderr,"CONNECTION CLOSED\nACCEPT\n");
	return(ret);
	}

static void close_accept_socket()
	{
	fprintf(stderr,"shutdown\n");
	if (accept_socket >= 0)
		shutdown(accept_socket,2);
	}

static int init_ssl_connection(con)
SSL *con;
	{
	int i;
	char *str;
	X509 *peer;

	if ((i=SSL_accept(con)) <= 0)
		{
		if (should_retry(i))
			{
			fprintf(stderr,"DELAY\n");
			return(1);
			}

		fprintf(stderr,"ERROR\n");
		if (verify_error != VERIFY_OK)
			{
			fprintf(stderr,"verify error:%s\n",
				X509_cert_verify_error_string(verify_error));
			}
		else
			ERR_print_errors(stderr);
		return(0);
		}
	peer=SSL_get_peer_certificate(con);
	if (peer != NULL)
		{
		printf("Client certificate\n");
		PEM_write_X509(stdout,peer);
		str=X509_NAME_oneline(X509_get_subject_name(peer));
		printf("subject=%s\n",str);
		free(str);
		str=X509_NAME_oneline(X509_get_issuer_name(peer));
		printf("issuer=%s\n",str);
		free(str);
		X509_free(peer);
		}
	printf("CIPHER is %s\n",SSL_get_cipher(con));
	return(1);
	}
