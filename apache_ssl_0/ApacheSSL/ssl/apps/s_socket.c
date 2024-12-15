/* apps/s_socket.c */
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
#include <errno.h>
#include "apps.h"
#include "s_net.h"
#include "ssl.h"
#include "s_apps.h"

extern int errno;

#ifdef _Windows
static struct WSAData wsa_state;
static int wsa_init_done=0;
#endif /* _Windows */

int init_client(sock, server, port)
int *sock;
char *server;
int port;
	{
	struct sockaddr_in them;
	struct hostent *host;
	struct protoent *prot;
	u_long ip_them;
	int s,i;

#ifdef _Windows
	if (!wsa_init_done) {
	  static char errbuf[1024];
	  static int err;
	  
	  wsa_init_done=1;
	  if (WSAStartup(0x0101,&wsa_state)!=0) {
	    err=WSAGetLastError();

/*	    WSAsperror(err,errbuf,sizeof(errbuf)-1); */
	    sprintf(errbuf,"error code=%d",err);

	    fprintf(stderr,"unable to start WINSOCK %s\n",
		    errbuf);
	    return(0);
	  }
	}
#endif /* _Windows */
	host=gethostbyname(server);
	if (host == NULL)
		{
		int a,b,c,d;

		if (sscanf(server,"%d.%d.%d.%d",&a,&b,&c,&d) == 4)
			ip_them=((unsigned long)a<<24L)|((unsigned long)b<<16L)|
				((unsigned long)c<<8L)|(unsigned long)d;
		else
			{
			fprintf(stderr,"unable to get %s's ip address\n",
				server);
			return(0);
			}
		}
	else
		memcpy(&ip_them,host->h_addr_list[0],sizeof(ip_them));
	memset((char *)&them,0,sizeof(them));
	them.sin_family=AF_INET;
	them.sin_port=htons((unsigned int)port);
	them.sin_addr.s_addr=ip_them;

	prot=getprotobyname("tcp");
	if (prot == NULL)
		{
#ifdef _Windows
		static struct protoent myproto;

		myproto.p_proto=0;
		prot=&myproto;
#else /* !_Windows */
		fprintf(stderr,"unable to find tcp protocol number");
		return(0);
#endif /* _Windows */		
		}
	s=socket(AF_INET,SOCK_STREAM,prot->p_proto);
	if (s == -1) { perror("socket"); return(0); }

	i=0;
	i=setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(char *)&i,sizeof(i));
	if (i < 0) { perror("keepalive"); return(0); }

	if (connect(s,(struct sockaddr *)&them,sizeof(them)) == -1)
		{ close(s); perror("connect"); return(0); }
	*sock=s;
	return(1);
	}

int do_server(port, ret, cb)
int port;
int *ret;
int (*cb)();
	{
	int sock;
	char *name;
	int accept_socket;
	int i;

	if (!init_server(&accept_socket,port)) return(0);

	if (ret != NULL)
		{
		*ret=accept_socket;
		return(1);
		}
	for (;;)
		{
		if (do_accept(accept_socket,&sock,&name) == 0)
			{
			SHUTDOWN(accept_socket);
			return(0);
			}
		i=(*cb)(name,sock);
		if (name != NULL) free(name);
		close(sock);
		if (i <= 0)
			{
			SHUTDOWN(accept_socket);
			return(i+1);
			}
		}
	}

int init_server(sock, port)
int *sock;
int port;
	{
	struct sockaddr_in server;
	struct protoent *prot;
	int s,i;
/*	struct linger ling;*/

#ifdef _Windows
	if (!wsa_init_done) {
	  static char errbuf[1024];
	  static int err;
	  
	  wsa_init_done=1;
	  if (WSAStartup(0x0101,&wsa_state)!=0) {
	    err=WSAGetLastError();

/*	    WSAsperror(err,errbuf,sizeof(errbuf)-1); */
	    sprintf(errbuf,"error code=%d",err);

	    fprintf(stderr,"unable to start WINSOCK %s\n",
		    errbuf);
	    return(0);
	  }
	}
#endif /* _Windows */

	memset((char *)&server,0,sizeof(struct sockaddr_in));
	server.sin_family=AF_INET;
	server.sin_port=htons((unsigned int)port);
	server.sin_addr.s_addr=INADDR_ANY;
	prot=getprotobyname("tcp");
	if (prot == NULL)
		{
#ifdef _Windows
		static struct protoent myproto;

		myproto.p_proto=0;
		prot=&myproto;
#else /* !_Windows */
		fprintf(stderr,"unable to find tcp protocol number");
		return(0);
#endif /* _Windows */		
		}
	s=socket(AF_INET,SOCK_STREAM,prot->p_proto);

	if (s == -1) return(0);
	if (bind(s,(struct sockaddr *)&server,sizeof(server)) == -1)
		{
		perror("bind");
		return(0);
		}
	if (listen(s,5) == -1) return(0);
/*	ling.l_onoff=1;
	ling.l_linger=0;
	i=setsockopt(s,SOL_SOCKET,SO_LINGER,(char *)&ling,sizeof(ling));
	if (i < 0) { perror("linger"); return(0); }
*/
	i=0;
	*sock=s;
	return(1);
	}

int do_accept(acc_sock, sock, host)
int acc_sock;
int *sock;
char **host;
	{
	int ret,i;
	struct hostent *h1,*h2;
	static struct sockaddr_in from;
	int len;
/*	struct linger ling; */

redoit:
	memset((char *)&from,0,sizeof(struct sockaddr));
	len=sizeof(from);
	ret=accept(acc_sock,(struct sockaddr *)&from,&len);
	if (ret == -1)
		{
		if (errno == EINTR)
			{
			/*check_timeout(); */
			goto redoit;
			}
		perror("accept");
		return(0);
		}

/*
	ling.l_onoff=1;
	ling.l_linger=0;
	i=setsockopt(ret,SOL_SOCKET,SO_LINGER,(char *)&ling,sizeof(ling));
	if (i < 0) { perror("linger"); return(0); }
	i=0;
	i=setsockopt(ret,SOL_SOCKET,SO_KEEPALIVE,(char *)&i,sizeof(i));
	if (i < 0) { perror("keepalive"); return(0); }
*/
	h1=gethostbyaddr((char *)&from.sin_addr.s_addr,
		sizeof(from.sin_addr.s_addr),AF_INET);
	if (h1 == NULL)
		{
		fprintf(stderr,"bad gethostbyaddr\n");
		*host=NULL;
		/* return(0); */
		}
	else
		{
		if ((*host=(char *)malloc(strlen(h1->h_name)+1)) == NULL)
			{
			perror("malloc");
			return(0);
			}
		strcpy(*host,h1->h_name);

		h2=gethostbyname(*host);
		i=0;
		if (h2->h_addrtype != AF_INET)
			{
			fprintf(stderr,"gethostbyname addr is not AF_INET\n");
			return(0);
			}
		}
	*sock=ret;
	return(1);
	}

int should_retry(i)
int i;
	{
	if ((i == 0) || (i == -1))
		{
		if (errno == EWOULDBLOCK) return(1);
#ifdef EPROTO
		if (errno == EPROTO) return(1);
#endif
		}
	return(0);
	}

#ifndef MSDOS
int spawn(argc, argv, in, out)
int argc;
char **argv;
int *in;
int *out;
	{
#define CHILD_READ	p1[0]
#define CHILD_WRITE	p2[1]
#define PARENT_READ	p2[0]
#define PARENT_WRITE	p1[1]
	int p1[2],p2[2];

	if ((pipe(p1) < 0) || (pipe(p2) < 0)) return(-1);

	if (fork() == 0)
		{ /* child */
		if (dup2(CHILD_WRITE,fileno(stdout)) < 0)
			perror("dup2");
		if (dup2(CHILD_WRITE,fileno(stderr)) < 0)
			perror("dup2");
		if (dup2(CHILD_READ,fileno(stdin)) < 0)
			perror("dup2");
		close(CHILD_READ); 
		close(CHILD_WRITE);

		close(PARENT_READ);
		close(PARENT_WRITE);
		execvp(argv[0],argv);
		perror("child");
		return(0);
		}

	/* parent */
	*in= PARENT_READ;
	*out=PARENT_WRITE;
	close(CHILD_READ);
	close(CHILD_WRITE);
	return(1);
	}
#endif /* MSDOS */
