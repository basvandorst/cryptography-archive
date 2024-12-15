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

static const char *szServerPort;

int InitGlobalCache(const char *szPort)
    {
    szServerPort=szPort;
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
    int nServerPort=atoi(szServerPort);

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
	assert(strlen(szServerPort) < sizeof saServer.un.sun_path);
	saServer.un.sun_family=AF_UNIX;
	strcpy(saServer.un.sun_path,szServerPort);
	nSize=sizeof saServer.un;
	}

    if(connect(nSocket,(struct sockaddr *)&saServer,nSize))
       {
       perror("connect");
       assert(0);
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
