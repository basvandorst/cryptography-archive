/* apps/s_client.c */
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
#include <unistd.h>
#include <string.h>
#include "s_net.h"
#include "x509.h"
#include "ssl.h"
#include "err.h"
#include "pem.h"
#include "apps.h"
#include "s_apps.h"

#define PROG	s_client_main

/*#define SSL_HOST_NAME	"www.netscape.com" */
/*#define SSL_HOST_NAME	"193.118.187.102" */
#define SSL_HOST_NAME	"localhost"

/*#define TEST_CERT "client.pem" */ /* no default cert. */

#define BUFSIZZ 1024*10

extern int verify_depth;
extern int verify_error;

#ifdef FIONBIO
static int nbio=0;
#endif

#ifndef NOPROTO
static void usage(void);
#else
static void usage();
#endif

static void usage()
	{
	fprintf(stderr,"usage: client args\n");
	fprintf(stderr,"\n");
	fprintf(stderr," -host arg     - host or IP to connect to (default is %s)\n",SSL_HOST_NAME);
	fprintf(stderr," -port arg     - port to connect to (default is %d\n",PORT);
	fprintf(stderr," -verify arg   - turn on peer certificate verification\n");
	fprintf(stderr," -cert arg     - certificate file to use, PEM format assumed\n");
	fprintf(stderr," -key arg      - RSA file to use, PEM format assumed, in cert file if\n");
	fprintf(stderr,"                 not specified but cert fill is.\n");
	fprintf(stderr," -CApath arg   - PEM format directory of CA's\n");
	fprintf(stderr," -CAfile arg   - PEM format file of CA's\n");
	fprintf(stderr," -reconnect    - Drop and re-make the connection with the same Session-ID\n");
	fprintf(stderr," -nbio         - Run with non-blocking IO\n");
	fprintf(stderr," -cipher       - prefered cipher to use, ':' seperated list of the following\n");
	fprintf(stderr,"                 RC4-MD5 EXP-RC4-MD5 CBC-DES-MD5 CBC3-DES-MD5 CBC-IDEA-MD5\n");
	fprintf(stderr,"                 CFB-DES-NULL\n");
	}

int MAIN(argc, argv)
int argc;
char **argv;
	{
	SSL *con=NULL,*con2=NULL;
	int s,i,j,k,l,width;
	static char buf[BUFSIZZ],*p;
	fd_set readfds;
	int port=PORT;
	char *host=SSL_HOST_NAME;
	char *cert_file=NULL,*key_file=NULL;
	char *CApath=NULL,*CAfile=NULL,*cipher=NULL;
	int reconnect=0,badop=0,verify=SSL_VERIFY_NONE;
	X509 *peer;
	SSL_CTX *ctx=NULL;
	int ret=1;
	/*static struct timeval timeout={10,0};*/

	ctx=SSL_CTX_new();
	if (ctx == NULL) goto end;
	verify_depth=0;
	verify_error=VERIFY_OK;
	nbio=0;

	argc--;
	argv++;
	while (argc >= 1)
		{
		if	(strcmp(*argv,"-host") == 0)
			{
			if (--argc < 1) goto bad;
			host= *(++argv);
			}
		else if	(strcmp(*argv,"-port") == 0)
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
		else if	(strcmp(*argv,"-reconnect") == 0)
			{
			reconnect=1;
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
		else if	(strcmp(*argv,"-cipher") == 0)
			{
			if (--argc < 1) goto bad;
			cipher= *(++argv);
			}
#ifdef FIONBIO
		else if (strcmp(*argv,"-nbio") == 0)
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

	SSL_load_error_strings();

	if ((!SSL_load_verify_locations(ctx,CAfile,CApath)) ||
		(!SSL_set_default_verify_paths(ctx)))
		{
		fprintf(stderr,"error seting default verify locations\n");
		ERR_print_errors(stderr);
		goto end;
		}

	if (init_client(&s,host,port) == 0)
		{
		perror("conecting");
		SHUTDOWN(s);
		goto end;
		}
	fprintf(stderr,"CONNECTED\n");

#ifdef FIONBIO
	if (nbio)
		{
		i=1;
		fprintf(stderr,"turning on non blocking io\n");
		ioctl(s,FIONBIO,&i);
		}
#endif                                              
	con=(SSL *)SSL_new(ctx);
	SSL_set_fd(con,s);
	SSL_set_verify(con,verify,verify_callback);

	if (cipher == NULL)
		SSL_set_pref_cipher(con,getenv("SSL_CIPHER"));
	else
		SSL_set_pref_cipher(con,cipher);

	if (!set_cert_stuff(con,cert_file,key_file))
		{
		SHUTDOWN(SSL_get_fd(con));
		goto end;
		}

	/* ok, lets connect */
	for (;;)
		{
		width=SSL_get_fd(con)+1;
		for (;;)
			{
			i=SSL_connect(con);
			if (should_retry(i))
				{
				fprintf(stderr,"DELAY\n");

				FD_ZERO(&readfds);
				FD_SET(SSL_get_fd(con),&readfds);
				select(width,&readfds,NULL,NULL,NULL);
				continue;
				}
			break;
			}
		if (i <= 0)
			{
			fprintf(stderr,"ERROR\n");
			if (verify_error != VERIFY_OK)
				{
				fprintf(stderr,"verify error:%s\n",
					X509_cert_verify_error_string(verify_error));
				}
			else
				{
				fprintf(stderr,"SSL_connect was in %s\n",
					ssl_state_string(con));
				ERR_print_errors(stderr);
				}
			SHUTDOWN(SSL_get_fd(con));
			goto end;
			}
		if (reconnect)
			{
			reconnect=0;

			i=SSL_write(con,"bye\n",4);
			SHUTDOWN(SSL_get_fd(con));

			if (init_client(&s,host,port) == 0)
				{ perror("conecting"); goto end; }

#ifdef FIONBIO
			if (nbio)
				{
				i=1;
				fprintf(stderr,"turning on non blocking io\n");
				ioctl(s,FIONBIO,&i);
				}
#endif 
			fprintf(stderr,"drop the connection and reconnect with the same session id\n");
			/* test a second connection */
			/* NOTE: It is a new SSL structure so we need to set the
			 * certificate and private key for this SSL structure */
			con2=SSL_new(ctx);
			SSL_set_fd(con2,s);
			SSL_copy_session_id(con2,con);
			if (!set_cert_stuff(con2,cert_file,key_file))
				{
				SHUTDOWN(SSL_get_fd(con2));
				goto end;
				}
			SSL_free(con);
			con=con2;
			con2=NULL;
			}
		else
			break;
		}
	width=s+1;
													
	peer=SSL_get_peer_certificate(con);
	if (peer != NULL)
		{
		char *str;

		printf("Server certificate\n");
		PEM_write_X509(stdout,peer);
		str=X509_NAME_oneline(X509_get_subject_name(peer));
		printf("subject=%s\n",str);
		free(str);
		str=X509_NAME_oneline(X509_get_issuer_name(peer));
		printf("issuer=%s\n",str);
		free(str);
		X509_free(peer);
		}


	p=SSL_get_shared_ciphers(con,buf,BUFSIZ);
	if (p != NULL)
		{
		printf("---\nCiphers common between both SSL endpoints:\n");
		j=i=0;
		while (*p)
			{
			if (*p == ':')
				{
				for (; j<12; j++)
					putc(' ',stdout);
				i++;
				j=0;
				if (i%3)
					putc(' ',stdout);
				else	putc('\n',stdout);
				}
			else
				{
				putc(*p,stdout);
				j++;
				}
			p++;
			}
		putc('\n',stdout);
		}

	printf("---\nCipher is %s\n",SSL_get_cipher(con));

	for (;;)
		{
		FD_ZERO(&readfds);
		FD_SET(fileno(stdin),&readfds);
		FD_SET(SSL_get_fd(con),&readfds);

/*		printf("pending=%d\n",SSL_pending(con));*/
		i=select(width,&readfds,NULL,NULL,NULL);
		if ( i < 0)
			{
			fprintf(stderr,"bad select\n");
			SHUTDOWN(SSL_get_fd(con));
			ret=0;
			goto end;
			}
		if (FD_ISSET(fileno(stdin),&readfds))
			{
			i=read(fileno(stdin),buf,BUFSIZZ);

			if ((i <= 0) || (buf[0] == 'Q'))
				{
				fprintf(stderr,"DONE\n");
				SHUTDOWN(SSL_get_fd(con));
				ret=0;
				goto end;
				}
			
			k=l=0;
			for (;;)
				{
				k=SSL_write(con,&(buf[l]),(unsigned int)i);
				if (should_retry(k))
					{
					fprintf(stderr,"W BLOCK\n");
					continue;
					}
				if (k < 0)
					{
					ERR_print_errors(stderr);
					SHUTDOWN(SSL_get_fd(con));
					ret=0;
					goto end;
					}
				l+=k;
				i-=k;
				if (i <= 0) break;
				}
			}
		if (FD_ISSET(SSL_get_fd(con),&readfds))
			{
			i=SSL_read(con,buf,BUFSIZZ);
			if (should_retry(i))
				{
				fprintf(stderr,"R BLOCK\n");
				}
			else if (i < 0)
				{
				ERR_print_errors(stderr);
				fprintf(stderr,"ERROR\n");
				break;
				}
			else if (i == 0)
				{
				fprintf(stderr,"DONE\n");
				break;
				}
			else 
				{
				fwrite(buf,i,1,stdout);
				fflush(stdout);
				}
			}
		}
	SHUTDOWN(SSL_get_fd(con));
	ret=0;
end:
	if (con != NULL) SSL_free(con);
	if (con2 != NULL) SSL_free(con2);
	if (ctx != NULL) SSL_CTX_free(ctx);
	EXIT(ret);
	}

							     
