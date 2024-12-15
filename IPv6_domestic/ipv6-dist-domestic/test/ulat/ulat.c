/*
 * ulat.c  --  Dinky UDP latency tester.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void usage(void)
{
  printf("Bad args.\n");
  exit(1);
}

/*
 * tvsub --
 *	Subtract 2 timeval structs:  out = out - in.  Out is assumed to
 * be >= in.
 */
void tvsub(out, in)
	register struct timeval *out, *in;
{
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

void main(int argc,char *argv[])

{
  int s;
  struct timeval timeout;
  char ch;
  int port = 2112;
  int receive = 1;
  struct sockaddr_in6 sin6;
  int count = 0;
  double ms,tmp;

  while ((ch = getopt(argc, argv, "srp:c:")) != EOF)
    switch(ch) {
    case 'c':
      count = atoi(optarg);
      break;
    case 's':
      receive = 0;
      break;
    case 'r':
      receive = 1;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    default:
      usage();
    }

  s = socket(AF_INET6,SOCK_DGRAM,0);
  if (s < 0)
    {
      perror("socket");
      exit(1);
    }

  bzero(&sin6,sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_len = sizeof(sin6);
  sin6.sin6_port = htons(port);
  if (receive)
    {
      if (bind(s,(struct sockaddr *)&sin6, sizeof(sin6)) <0)
	{
	  perror("Receiver bind()");
	  exit(1);
	}
      for(;;)
	{
	  char rbuf[80];
	  int flen = sizeof(sin6), rlen;

	  if ((rlen =recvfrom(s,rbuf,80,0,(struct sockaddr*)&sin6,&flen))<=0)
	    {
	      perror("receiver recvfrom()");
	      exit(1);
	    }
	  if (sendto(s,rbuf,rlen,0,(struct sockaddr *)&sin6,flen) <0)
	    {
	      perror("receiver send");
	      exit(1);
	    }
	}
    }
  else
    {
      if (optind < argc)
	{
	  struct hostent *hp;

	  if (hp = (struct hostent *)hostname2addr(argv[optind], AF_INET6))
	    bcopy(hp->h_addr_list[0],(void *)&(sin6.sin6_addr),hp->h_length);
	  else
	    if (ascii2addr(AF_INET6, argv[optind],(void *)&(sin6.sin6_addr))
		< 0) {
	      fprintf(stderr, "udpsend: %s: bad value\n", argv[optind]);
	      exit(1);
	    }
	}
      else
	{
	  IN6_ADDR_ASSIGN(sin6.sin6_addr,0,0,0,htonl(1));
	}
      if (connect(s,(struct sockaddr *)&sin6,sizeof(sin6)) <0)
	{
	  perror("connect");
	  exit(1);
	}
      if (count != 0)
	count ++;
      for (;(count == 0)?1:(--count != 1);)
	{
	  struct timeval newtime;
	  int rlen;

	  if (gettimeofday(&timeout,NULL) <0)
	    {
	      perror("gettimeofday");
	      exit(1);
	    }
	  if (send(s,(void *)&timeout,sizeof(timeout),0) < 0)
	    {
	      perror("send");
	      exit(1);
	    }
	  if ((rlen = recv(s,(void *)&timeout,sizeof(timeout),0))
	      < 0)
	    {
	      perror("recv in error");
	      exit(1);
	    }
	  if (rlen < sizeof(timeout))
	    {
	      printf("recv got only %d.\n",rlen);
	      exit(1);
	    }
	  if (gettimeofday(&newtime,NULL) <0)
	    {
	      perror("gettimeofday(2)");
	      exit(1);
	    }
	  tvsub(&newtime,&timeout);
	  ms = ((double)newtime.tv_sec) * 1000.0 +
	    ((double)newtime.tv_usec) / 1000.0;
	  printf("%f msec.\n",ms);
	  
	}
    }
}
     
