/* ====================================================================
 * Copyright (c) 1995, 1996, 1997, 1998 Ben Laurie.  All rights reserved.
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
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * 4. The name "Apache-SSL Server" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * THIS SOFTWARE IS PROVIDED BY BEN LAURIE ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL BEN LAURIE OR
 * HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of patches to the Apache HTTP server interfacing it
 * to SSLeay.
 * For more information on Apache-SSL see http://www.apache-ssl.org.
 *
 * For more information on Apache see http://www.apache.org.
 *
 * For more information on SSLeay see http://www.psy.uq.oz.au/~ftp/Crypto/.
 *
 * Ben Laurie <ben@algroup.co.uk>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <memory.h>
#include <unistd.h>
#include <assert.h>
#include "gcache.h"
#include "httpd.h"

/* ap_config.h defines signal to be ap_signal... */
#undef signal

#define DEBUG		0

#if DEBUG

void Dump(const char *szName,uchar *p,int n)
    {
    fprintf(stderr,"%s(",szName);
    for( ; n-- > 0 ; ++p)
	fprintf(stderr,"%02x",*p);
    fprintf(stderr,")\n");
    }

#else

#define Dump(a,b,c)

#endif

/*
  In an ideal world, this would come from some Apache supplied library, but...
  well ... just try it :-)
*/
char *get_time() {
    time_t t;
    char *time_string;

    t=time(NULL);
    time_string = ctime(&t);
    time_string[strlen(time_string) - 1] = '\0';
    return (time_string);
}

static void HandleAdd(int nFD)
    {
    uchar *aucKey;
    int nKey;
    Cache *p;

    aucKey=ReadThing(nFD,&nKey);
    Dump("Add",aucKey,nKey);
    p=LocalCacheFind(aucKey,nKey);
    if(p)
	{
	free(p->aucData);
	free(aucKey);
	}
    else
	p=LocalCacheAdd(aucKey,nKey,0);
    p->aucData=ReadThing(nFD,&p->nData);
    read(nFD,&p->tExpiresAt,sizeof p->tExpiresAt);
    }

void HandleGet(int nFD)
    {
    uchar *aucKey;
    int nKey;
    Cache *p;

    aucKey=ReadThing(nFD,&nKey);
    Dump("Get",aucKey,nKey);
    p=LocalCacheFind(aucKey,nKey);
    if(!p)
	{
	WriteThing(nFD,NULL,0);
#if DEBUG
	fprintf(stderr,"Miss!\n");
#endif
	}
    else
	{
	WriteThing(nFD,p->aucData,p->nData);
	write(nFD,&p->tExpiresAt,sizeof p->tExpiresAt);
	}
    }
	
void HandleClient(int nFD)
    {
    static char c;
    int n;

    n=read(nFD,&c,1);
    assert(n == 1);
    switch(c)
	{
    case ADD_TO_CACHE:
	HandleAdd(nFD);
	break;

    case GET_FROM_CACHE:
	HandleGet(nFD);
	break;

    default:
	fprintf(stderr,"request was %d\n",c);
	assert(!"Unknown request");
	}
    }

typedef union
    {
    struct sockaddr_in in;
    struct sockaddr_un un;
    } SockAddr;

int main(int argc,char **argv)
    {
    const char *szPort;
    int nUserID;
    int nPort;
    int nSocket;
    SockAddr saServer;
    int one=1;
    int nSize;

    if(argc != 3)
	{
	fprintf(stderr,"%s <user id> <port>\n",argv[0]);
	exit(1);
	}

    /* Some systems unhelpfully kill us with this... */
    signal(SIGPIPE,SIG_IGN);

    fprintf(stderr,"[%s] %s started\n",get_time(),argv[0]);

    nUserID=atoi(argv[1]);
    szPort=argv[2];

    nPort=atoi(szPort);

    if(nPort)
	nSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    else
	nSocket=socket(AF_UNIX,SOCK_STREAM,0);
    if(nSocket < 0)
	{
	perror("socket");
	exit(2);
	}

    if(nPort
       && setsockopt(nSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&one,sizeof one) < 0)
	{
	perror("setsockopt");
        exit(7);
	}

    memset(&saServer,0,sizeof saServer);
    if(nPort)
	{
	saServer.in.sin_family=AF_INET;
	saServer.in.sin_port=htons(nPort);
	nSize=sizeof saServer.in;
	}
    else
	{
	if(strlen(szPort) >= sizeof saServer.un.sun_path)
	    exit(6);
	saServer.un.sun_family=AF_UNIX;
	unlink(szPort);
	strcpy(saServer.un.sun_path,szPort);
	nSize=sizeof saServer.un;
	}

    if(bind(nSocket,(struct sockaddr *)&saServer,nSize) < 0)
	{
	perror("bind");
	exit(3);
	}

    if(listen(nSocket,512) < 0)
	{
	perror("listen");
	exit(4);
	}

    if(!nPort)
	{
	chmod(szPort,0700);
	chown(szPort,nUserID,-1);
	}

    if(!geteuid() && setuid(nUserID) == -1)
	{
	fprintf(stderr,"[%s] gcache: user id=%d: ",get_time(),nUserID);
	perror("setuid");
	exit(5);
	}

    for( ; ; )
	{
	int nFD;
	int nLen;
	SockAddr saClient;

	nLen=nSize;
	nFD=accept(nSocket,(struct sockaddr *)&saClient,&nLen);
	if(nFD < 0)
	    {
	    fprintf(stderr,"[%s] gcache: accept failed, error=%d\n",get_time(),
		    errno);
	    continue;
	    }
	if(!nPort || ntohl(saClient.in.sin_addr.s_addr) == 0x7f000001)
	    HandleClient(nFD);
	else
	    fprintf(stderr,
		    "[%s] gcache: unexpected connect from %s, ignored\n",
		    get_time(),inet_ntoa(saClient.in.sin_addr));
	    
	close(nFD);
	}

    return 0;
    }
