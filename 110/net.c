#include "net.h"
#include "des.h"
#include "arch.h"

#include <sys/types.h>
#ifdef WIN32
#include <winsock.h>
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif
#include <string.h>
#include <errno.h>

#include "desc.h"

/* Cleared! Will be set when we are ready :) */
static char ServerKey[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

extern char firewallhost[256];
extern char socks_username[256];
extern char socks_password[256];
extern int dopause;
extern int verboselevel;

static unsigned char ServerIP[4] = {0,0,0,0};

void Net_Init(char *server) {
  struct hostent *host = gethostbyname(server);
  if (!host) {
    fprintf(OUTPUT,"\n- net_init: Could not resolve server name!\n");
    exit(1);
  }
  memcpy(ServerIP,host->h_addr,4);
}

/* Normal client here */
unsigned int ServerConnect_normal(int ipprint) {
	
  unsigned int ServerSocket;
  struct sockaddr_in addr;
  char ch;
  int result;
#ifdef BIT64
  unsigned int ip;
#else
  unsigned long ip;
#endif
  
  if (!(ServerIP[0] | ServerIP[1] | ServerIP[2] | ServerIP[3])) {
    fprintf(OUTPUT,"\n - ServerConnect - IP not initialised!\n");
    exit(1);
  }

  if(ipprint) {
    memcpy(&ip,ServerIP,4);
    ip=ntohl(ip);
    fprintf(OUTPUT,"(%li.%li.%li.%li)...",HIBYTE(HIWORD(ip)),LOBYTE(HIWORD(ip)),HIBYTE(LOWORD(ip)),LOBYTE(LOWORD(ip)));
  }

  do { /* Failure loop */
    ServerSocket = socket(AF_INET,SOCK_STREAM,0);
    if (ServerSocket == 0xFFFFFFFF) {
      fprintf(OUTPUT,"\n- ServerConnect: Create socket failed...");
      exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    memcpy(&addr.sin_addr.s_addr,ServerIP,4);
    result = connect(ServerSocket,(struct sockaddr *)&addr,sizeof(addr));
    if (result) {
      close(ServerSocket); /* Just in case */
      fprintf(OUTPUT,"\n- ServerConnect: Could not connect to server, sleeping %i minutes...\n",NETTIMEOUTMIN);
#ifdef NO_USLEEP
      sleep(NETTIMEOUT/1000000);
#else
#ifdef WIN32
	  Sleep(NETTIMEOUT/1000);
#else
      usleep(NETTIMEOUT);
#endif
#endif
    }
  } while (result);

  if (recv(ServerSocket,&ch,1,0) != 1) {
    fprintf(OUTPUT,"\n- ServerConnect: Did not receive wakeup from server - dead daemon?\n");
    return 0xFFFFFFFF;
  }
  
  if (ch != WAKEUP_CALL) {
    fprintf(OUTPUT,"\n- ServerConnect: Received wrong wakeup number, possible change in the protocol.\n");
    fprintf(OUTPUT,"-                Check 'http://www.des.sollentuna.se' for more info.\n");
    exit(1);
  }
  
  return ServerSocket;
}
/* No, it is the SOCKS version */

unsigned int ServerConnect_socks(char *server, int ipprint) {
  struct hostent *host = gethostbyname(firewallhost);
  int thesock;
  struct sockaddr_in addr;
  /* Support only for no-authentication method */
  const unsigned char methodreport[4]={0x05,0x02,0x00,0x02}; 
  unsigned char methodid[2];
  unsigned char socksrequest[256];
  unsigned short netport = htons(SERVER_PORT);
  unsigned char reply1[4];
  unsigned char dummybuf[256];
  unsigned char length;
  unsigned char ch;

  if (!host) {
    fprintf(OUTPUT,"\n- socks_connect: Could not resolve name of SOCKS host!\n");
    exit(1);
  }

  thesock = socket(AF_INET,SOCK_STREAM,0);
  if (thesock == -1) {
    fprintf(OUTPUT,"\n- socks_connect: Failed to create socket!\n");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(1080); /* Standard SOCKS port */
  memcpy(&addr.sin_addr.s_addr,host->h_addr,host->h_length);

  if (connect(thesock,(struct sockaddr *)&addr,sizeof(addr))) {
    fprintf(OUTPUT,"\n- socks_connect: Failed to connect to SOCKS server!\n");
    exit(1);
  }

  if (send(thesock,methodreport,sizeof(methodreport),0) != sizeof(methodreport)) {
    fprintf(OUTPUT,"\n- socks_connect: Failed to send method report to SOCKS server!\n");
    close(thesock);
    exit(1);
  }

  if (recv(thesock,methodid,sizeof(methodid),0) != sizeof(methodid)) {
    fprintf(OUTPUT,"\n- socks_connect: SOCKS server did not direct authentication method!\n");
    close(thesock);
    exit(1);
  }

  if (methodid[1] == 0xFF) {
    fprintf(OUTPUT,"\n- socks_connect: SOCKS server did not accept my authentication method!\n");
    close(thesock);
    exit(1);
  }

  if (methodid[1] == 0x02) {
    char authpacket[256];
    if (!strlen(socks_username)) {
      fprintf(OUTPUT,"\n- socks_connect: Enter username: ");
      gets(socks_username);
    }
    if (!strlen(socks_password)) {
      fprintf(OUTPUT,"\n- socks_connect: Enter password: ");
#ifndef WIN32
      strcpy(socks_password,getpass(""));
#else
	  gets(socks_password);
#endif
    }
    authpacket[0] = 0x01;
    authpacket[1] = strlen(socks_username);
    strcpy(&authpacket[2],socks_username);
    authpacket[2+strlen(socks_username)] = strlen(socks_password);
    strcpy(&authpacket[3+strlen(socks_username)],socks_password);
    if (send(thesock,authpacket,3+strlen(socks_username)+strlen(socks_password),0) != 3+strlen(socks_username)+strlen(socks_password)) {
      fprintf(OUTPUT,"\n- socks_connect: Failed to authenticate with server!\n");
      close(thesock);
      return -1;
    }
    if (recv(thesock,authpacket,2,0) != 2) {
      fprintf(OUTPUT,"\n- socks_connect: Server did not respond to authentication!\n");
      close(thesock);
      return -1;
    }
    if (authpacket[1]) {
      fprintf(OUTPUT,"\n- socks_connect: Authentication with SOCKS server failed!\n");
      close(thesock);
      return -1;
    }
    if (verboselevel > 2) 
      fprintf(OUTPUT,"\n- socks_connect: Authenticated.\n");
  } /* If we have to do authentication */

  /* Ok. Either no authentication required, or we are done authenticating */

  memset(socksrequest,0,sizeof(socksrequest));
  socksrequest[0] = 0x05;                   /* Protocol version */
  socksrequest[1] = 0x01;                   /* CONNECT request */
  socksrequest[2] = 0x00;                   /* Reserved, must be 0 */
  socksrequest[3] = 0x03;                   /* We pass a FQDN */
  socksrequest[4] = strlen(server);
  strcpy(&socksrequest[5],server);
  memcpy(&socksrequest[5+strlen(server)],&netport,2);

  if (send(thesock,socksrequest,7+strlen(server),0) != 7+strlen(server)) {
    fprintf(OUTPUT,"\n- socks_connect: Failed to send connection request!\n");
    close(thesock);
    exit(1);
  }

  if (recv(thesock,reply1,4,0) != 4) {
    fprintf(OUTPUT,"\n- socks_connect: Failed to receive status from SOCKS (1)!\n");
    close(thesock);
    exit(1);
  }

  if (reply1[1]) {
    switch (reply1[1]) {
    case 0x01:fprintf(OUTPUT,"\n- socks_connect: Server - general SOCKS failure!\n");break;
    case 0x02:fprintf(OUTPUT,"\n- socks_connect: Server - connection not allowed by ruleset!\n");break;
    case 0x03:fprintf(OUTPUT,"\n- socks_connect: Server - network unreachable!\n");break;
    case 0x04:fprintf(OUTPUT,"\n- socks_connect: Server - host unreachable!\n");break;
    case 0x05:fprintf(OUTPUT,"\n- socks_connect: Server - connection refused!\n");break;
    case 0x06:fprintf(OUTPUT,"\n- socks_connect: Server - TTL expired!\n");break;
    case 0x07:fprintf(OUTPUT,"\n- socks_connect: Server - command not supported!\n");break;
    case 0x08:fprintf(OUTPUT,"\n- socks_connect: Server - address type not supported!\n");break;
    default:fprintf(OUTPUT,"\n- socks_connect: Server - unknown error!\n");break;
    }
    close(thesock);
    exit(1);
  }

  /* Connection is now established :) */
  switch (reply1[3]) { /* Handle the AType input */
  case 0x01:
    if (recv(thesock,dummybuf,4+2,0) != 4+2) {
      fprintf(OUTPUT,"\n- socks_connect - Unable to complete read of server info packet (1)!\n");
      close(thesock);
      exit(1);
    }
    break;
  case 0x03:
    if (recv(thesock,&length,1,0) != 1) {
      fprintf(OUTPUT,"\n- socks_connect - Unable to complete read of server info packet (2)!\n");
      close(thesock);
      exit(1);
    }
    if (recv(thesock,dummybuf,length+2,0) != length+2) {
      fprintf(OUTPUT,"\n- socks_connect - Unable to complete read of server info packet (3)!\n");
      close(thesock);
      exit(1);
    }
    break;
  case 0x04:
    if (recv(thesock,dummybuf,16+2,0) != 16+2) {
      fprintf(OUTPUT,"\n- socks_connect - Unable to complete read of server info packet (4)!\n");
      close(thesock);
      exit(1);
    }
    break;
  default:
    fprintf(OUTPUT,"\n- socks_connect - Server reported an unknown address type (%02X)!\n",reply1[3]);
    close(thesock);
    exit(1);
  }

  /* Yep. Connection is now open. Go for it!*/
  /* Here is something cut from the "normal" serverconnect() */

  if (recv(thesock,&ch,1,0) != 1) {
    fprintf(OUTPUT,"\n- ServerConnect: Did not receive wakeup from server - dead daemon?\n");
    return 0xFFFFFFFF;
  }
  
  if (ch != WAKEUP_CALL) {
    fprintf(OUTPUT,"\n- ServerConnect: Received wrong wakeup number, possible change in the protocol.\n");
    fprintf(OUTPUT,"-                Check 'http://www.des.sollentuna.se' for more info.\n");
    exit(1);
  }
  
  return thesock;
}
/* Normal vs Socks ServerConnect() */

unsigned int ServerConnect(char *server, int ipprint) {
  if (dopause) {
    char tmps[1024];
    fprintf(OUTPUT,"\n - About to connect to server. Press Enter to continue\n");
    gets(tmps);
  }
  if (!(ServerIP[0] | ServerIP[1] | ServerIP[2] | ServerIP[3])) 
    Net_Init(server);
  if (strlen(firewallhost))
    return ServerConnect_socks(server, ipprint);
  else 
    return ServerConnect_normal(ipprint);
}

#ifdef BIT64
unsigned int
#else
unsigned long 
#endif
RequestKeyblock(int ServerSocket, char* Rvec) {
  
  Client_Server_Request CSrequest;
  Server_Client_Request request;
  int loops = 0;
  int num;
#ifdef SWAP_BYTES
  char *s;
#endif
  memset(&CSrequest,0,sizeof(CSrequest));
  CSrequest.request = 1;
#ifdef SWAP_BYTES
  s = (char *)&CSrequest.request;
  swapl(s);
#endif

  if (send(ServerSocket,&CSrequest,sizeof(CSrequest),0) != sizeof(CSrequest)) {
    fprintf(OUTPUT,"\n- RequestKeyblock: Failed to send request to server (%i)!\n",errno);
    return 0xFFFFFFFF;
  }
  
  while ((num = recv(ServerSocket,&request,sizeof(request),MSG_PEEK)) != sizeof(request) && num && loops < 50) {
#ifdef NO_USLEEP
    sleep(1);
#else
#ifdef WIN32
	Sleep(500);
#else
	usleep(500000);
#endif
#endif
    loops++;
  }
  
  if (num != sizeof(request)) {
    fprintf(OUTPUT,"\n- RequestKeyblock: Invalid packet size received!\n");
    close(ServerSocket);
    return 0xFFFFFFFF;
  }
  
  memset(&request,0,sizeof(request));
  if (recv(ServerSocket,&request,sizeof(request),0) != sizeof(request)) {
    fprintf(OUTPUT,"\n- RequestKeyblock: Received invalid packet size!\n");
    return 0xFFFFFFFF;
  } 
  else {
#ifdef SWAP_BYTES
    s = (char *)&request.request;
    swapl(s);
    s = (char *)&request.keyspace;
    swapl(s);
#endif
    if (request.request != 128) {
      fprintf(OUTPUT,"\n- RequestKeyblock: Server has no free keyblocks (code %i)\n",request.request);
      return 0xFFFFFFFF;
    }
  }
  
  for(num=0; num<8; num++)
    Rvec[num] = request.RandomVec[num]^ServerKey[num];

  return request.keyspace;
}

#ifdef BIT64
unsigned int
#else
unsigned long 
#endif
ReportKeyblock(int ServerSocket,char *email,UINT4 keyspace,int command,char *Rvec) {
  
  Client_Server_Request CSrequest;
  Server_Client_Request request;
#ifdef SWAP_BYTES
  char *s;
#endif
  memset(&CSrequest,0,sizeof(CSrequest));
  CSrequest.request = command;
  CSrequest.keyspace = keyspace;
#ifdef TEST
  CSrequest.platform = 255;
#else
  CSrequest.platform = PLATFORM_ID | (strlen(firewallhost)?0x80:0);  /* Or with 0x80 if fw */
#endif
  strcpy(CSrequest.email,email);
  memcpy(CSrequest.RandomVec,Rvec,8);

#ifdef SWAP_BYTES
  s = (char *)&CSrequest.request;
  swapl(s);
  s = (char *)&CSrequest.keyspace;
  swapl(s);
#endif
  if (send(ServerSocket,&CSrequest,sizeof(CSrequest),0) != sizeof(CSrequest)) {
    fprintf(OUTPUT,"\n- ReportKeyblock: Failed to send report to server!\n");
    return 1;
  }
  
  memset(&request,0,sizeof(request));
  if (recv(ServerSocket,&request,sizeof(request),0) != sizeof(request)) {
    fprintf(OUTPUT,"\n- ReportKeyblock: No ACK received from server!\n");
    return 1;
  }
#ifdef SWAP_BYTES
  s = (char *)&CSrequest.request;
  swapl(s);
#endif 

  if(request.request != 0) {
    /* Server reported an error! */
    if (request.keyspace != CSrequest.keyspace) {
      fprintf(OUTPUT,"Keyblock mismatch in error report!\n");
      return 1; /* Flag error, so we do it again */
    }
    switch (request.request) {
    case 130:
      fprintf(OUTPUT,"Server rejected my report!\n");
      return 0; /* Report succcess - fools the main, so it starts with the next one */
    default:
      fprintf(OUTPUT,"Unknown error report from server!\n");
    }
    return 1; 
  }
  else
    return 0;
}
