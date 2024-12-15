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

#define DEBUG		1

#if DEBUG

void Dump(const char *szName,uchar *p,int n)
    {
    printf("%s(",szName);
    for( ; n-- > 0 ; ++p)
	printf("%02x",*p);
    printf(")\n");
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
    int n;

    aucKey=ReadThing(nFD,&nKey);
    Dump("Get",aucKey,nKey);
    p=LocalCacheFind(aucKey,nKey);
    if(!p)
	{
	WriteThing(nFD,NULL,0);
#if DEBUG
	printf("Miss!\n");
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

    read(nFD,&c,1);
    switch(c)
	{
    case ADD_TO_CACHE:
	HandleAdd(nFD);
	break;

    case GET_FROM_CACHE:
	HandleGet(nFD);
	break;
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
	    fprintf(stderr,"[%s] gache: accept failed, error=%d\n",get_time(),
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
