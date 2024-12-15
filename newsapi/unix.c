#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>

#include "newsapi.h"
#include "nntp.h"
#include "util.h"

#define SOCKET_BUFFER_SIZE	4096
#define SERVER_LINE_SIZE	4096

/* #define bcopy(FROM, TO, SIZE)	memmove (TO, FROM, SIZE)  */

/* some of this was ripped off from 'tinyirc.c' */
/* this is *not* meant to be the final unix version */

int connect_to_server(char *hostname, news_server_type * server)
{
  struct sockaddr_in sa;
  struct hostent     *hp;
  int    s, result;
  int ioctl_blocking = O_NONBLOCK;
  nntp_impdep_type * impdep = NNTP_STUFF(server);

  char * sockbuf = (char *) news_malloc (SOCKET_BUFFER_SIZE);
  char * server_line = (char *) news_malloc (SERVER_LINE_SIZE);

  if (!sockbuf || !server_line)
    return (ERR_CANT_MALLOC);

  impdep->sockbuf = sockbuf;
  impdep->server_line = server_line;
  impdep->sockbuf_index = 0;  
  impdep->chars_in_buf = 0;

  if((hp=gethostbyname(hostname))==NULL) {
    return (ERR_CANT_RESOLVE_HOSTNAME);
  }

  memset(&sa, (char) 0, sizeof(sa)); 
  /* bzero(&sa, sizeof(sa)); */
  bcopy(hp->h_addr, (char *)&sa.sin_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short)119);

  if((s=socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
    return (ERR_CANT_GET_SOCKET);
  }

  if(connect(s,(struct sockaddr *) &sa, sizeof(sa)) < 0) {
    close(s);
    return (ERR_CANT_CONNECT);
  }

  /* fix this */
  result = fcntl(s, F_SETFL, &ioctl_blocking);
  if (result == -1) {
    perror("");
  }

  impdep->socket = s;
  return (0);
}

int
get_server_char(news_server_type * server, char * ch)
{
  int bytes_read;
  nntp_impdep_type * impdep = NNTP_STUFF(server);

  /* if our buffer's empty, grab another block from the server */
  if (impdep->sockbuf_index >= impdep->chars_in_buf) {
    fd_set set;
    int retval;
    struct timeval timeout;
     
    /* Initialize the file descriptor set.  */
    FD_ZERO (&set);
    FD_SET (impdep->socket, &set);
     
    /* Initialize the timeout data structure.  */
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    
    /* `select' returns 0 if timeout, 1 if input available, -1 if error.  */

    retval = (select (FD_SETSIZE, &set, NULL, NULL, &timeout));
    switch (retval) {
      /* timeout */
    case 0:
      fprintf (stderr, "* select timed out\n");
      return (ERR_READING_SOCKET);
      break;
      /* success */
    case 1:
      bytes_read = recv (impdep->socket,
			 impdep->sockbuf,
			 SOCKET_BUFFER_SIZE, 0);
      if (bytes_read < 1) {
	return(ERR_READING_SOCKET);
      }
      impdep->chars_in_buf = bytes_read;
      impdep->sockbuf_index = 0; /* reset the index */
      break;
    case -1:
      fprintf (stderr, "* select error\n");
      perror ("");
      return (ERR_READING_SOCKET);
      break;
    default:
      fprintf (stderr, "* select returned a TOTALLY unexpected value %d\n", retval);
      return (ERR_READING_SOCKET);
      break;
    }
  }
  
  *ch = (impdep->sockbuf)[(impdep->sockbuf_index)++];
  return (0);
}

int put_server_data(news_server_type * server,char * outbuf, size_t length)
{
  if(write(NNTP_STUFF(server)->socket, outbuf, length) < 0 ) {
    fprintf (stderr, "* write() failed in put_server_data()!\n");
    return(ERR_SENDING_LINE);
  }
  return(0);
}

int close_server (news_server_type * server)
{
  return (close(NNTP_STUFF(server)->socket));
}

