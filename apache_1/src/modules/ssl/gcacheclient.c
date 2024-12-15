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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "gcache.h"

static const char *s_szServerPort;

void InitGlobalCache(const char *szPort)
    {
    s_szServerPort=szPort;
    }

typedef union
    {
    struct sockaddr_in in;
    struct sockaddr_un un;
    } SockAddr;

static int OpenServer()
    {
    int nSize;
    int nSocket;
    SockAddr saServer;
    int nServerPort=atoi(s_szServerPort);

    if(nServerPort)
	nSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    else
	nSocket=socket(AF_UNIX,SOCK_STREAM,0);
    if(nSocket < 0)
	{
	perror("socket");
	exit(2);
	}

    memset(&saServer,0,sizeof saServer);
    if(nServerPort)
	{
	saServer.in.sin_family=AF_INET;
	saServer.in.sin_addr.s_addr=htonl(0x7f000001);
	saServer.in.sin_port=htons(nServerPort);
	nSize=sizeof saServer.in;
	}
    else
	{
	assert(strlen(s_szServerPort) < sizeof saServer.un.sun_path);
	saServer.un.sun_family=AF_UNIX;
	strcpy(saServer.un.sun_path,s_szServerPort);
	nSize=sizeof saServer.un;
	}

    if(connect(nSocket,(struct sockaddr *)&saServer,nSize))
       {
       fprintf(stderr,"Failed to connect to socket: %s\n",s_szServerPort);
       perror("connect");
       assert(!"couldn't connect to socket");
       }
    return nSocket;
    }

static void WriteCommand(int nFD,enum Command eCommand)
    {
    static char c;

    c=eCommand;
    write(nFD,&c,1);
    }

void GlobalCacheAdd(uchar *aucKey,int nKey,uchar *aucData,
		int nData,time_t tExpiresAt)
    {
    int nFD=OpenServer();

    WriteCommand(nFD,ADD_TO_CACHE);
    WriteThing(nFD,aucKey,nKey);
    WriteThing(nFD,aucData,nData);
    write(nFD,&tExpiresAt,sizeof tExpiresAt);
    close(nFD);
    }

uchar *GlobalCacheGet(uchar *aucKey,int nKey,int *pnData,time_t *ptExpiresAt)
    {
    int nFD=OpenServer();
    uchar *ret;

    WriteCommand(nFD,GET_FROM_CACHE);
    WriteThing(nFD,aucKey,nKey);
    ret=ReadThing(nFD,pnData);
    if(ret)
	read(nFD,ptExpiresAt,sizeof *ptExpiresAt);
    close(nFD);
    return ret;
    }
