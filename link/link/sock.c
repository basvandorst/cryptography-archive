
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include "bcopy.h"

#ifndef PORT
#define PORT 3012
#endif  PORT

get_connect()
{
  int s,s2,len;
  struct sockaddr_in addr,remote;

  fprintf(stderr,"listening on port %d\n",PORT);
  s=socket(AF_INET,SOCK_STREAM,0);
  if(s<0) {
    perror("socket");
    return(-1);
  }
  addr.sin_addr.s_addr   = INADDR_ANY;
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(PORT); 
  bzero(&remote,sizeof(remote));
  if(bind(s,&addr,sizeof(addr))<0) {
    perror("bind");
    return(-1);
  }
  if(listen(s,1)<0) {
    perror("listen");
    return(-1);
  }
  len = sizeof(remote);
  s2=accept(s,&remote,&len);
  close(s);
  if(s2>0) fprintf(stderr,"got it\n");
  else perror("accept");
  return(s2);
}

