/*
 * socktest.c  --  Socket testing program.  Useful for first-cut tests on
 *                 AF_{INET,INET6}.
 *
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#       @(#)COPYRIGHT   1.1a (NRL) 17 August 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

This software and documentation were developed at NRL by various
people.  Those developers have each copyrighted the portions that they
developed at NRL and have assigned All Rights for those portions to
NRL.  Outside the USA, NRL also has copyright on the software
developed at NRL. The affected files all contain specific copyright
notices and those notices must be retained in any derived work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

        This product includes software developed at the Information
        Technology Division, US Naval Research Laboratory.

4. Neither the name of the NRL nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THE SOFTWARE PROVIDED BY NRL IS PROVIDED BY NRL AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL NRL OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressed or implied, of the US Naval
Research Laboratory (NRL).

----------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet6/in6.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

extern char *optarg;
extern int errno, optind;

int port = 7777, af = AF_INET6, server = 0, type = SOCK_DGRAM;
int s = 0, s2 = 0, datasize = 0;
struct sockaddr_in6 laddr, faddr;
char datablock[65536];
char mynamestr[256];

say(message, p1, p2)
char *message;
char *p1;
char *p2;
{
  char a2abuf[64];

  fputs(mynamestr, stdout);
  printf(message, p1, p2);
  putchar('\n');
}

main(argc, argv)
     int argc;
     char *argv[];
{
  int i;
  char *p;
  struct hostent *hp;
  char ch;
  int ahlev = 0, esplev = 0, esptlev = 0;
 
  while ((ch = getopt(argc, argv, "A:E:T:a:t:s")) != EOF)
    switch(ch) {
    case 'a':
      if (strcmp(optarg, "inet") == 0) {
	af = AF_INET;
      } else if (strcmp(optarg, "inet6") == 0) {
	af = AF_INET6;
      } else {
	fprintf(stderr,"socktest: invalid address family: %s\n", optarg);
	fprintf(stderr,"socktest: use either 'inet' or 'inet6'.\n");
	exit(1);
      }
      break;
    case 'A':
      ahlev = atoi(optarg);
      break;
    case 'E':
      esplev = atoi(optarg);
      break;
    case 'T':
      esptlev = atoi(optarg);
      break;
    case 's':
      server = 1;
      break;
    case 't':
      if (strcmp(optarg, "stream") == 0) {
	type = SOCK_STREAM;
      } else if (strcmp(optarg, "dgram") == 0) {
	type = SOCK_DGRAM;
      } else {
	fprintf(stderr,"socktest: invalid socket type: %s\n", optarg);
	fprintf(stderr,"socktest: use either 'stream' or 'dgram'.\n");
	exit(1);
      }
      break;
    default:
      usage();
    }
/*  argc -= optind;
  argv += optind;*/

  if (optind < argc)
    if ((port = atoi(argv[optind++])) <= 0) {
      fprintf(stderr, "socktest: invalid port\n");
      exit(1);
    }

  bzero(&laddr, sizeof(laddr));

  laddr.sin6_family = af;
  laddr.sin6_len = (af == AF_INET6) ? 
    sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);

  switch (af) {
  case AF_INET6:
    laddr.sin6_port = htons(port);
    break;
  case AF_INET:
    ((struct sockaddr_in *)&laddr)->sin_port = htons(port);
    break;
  }

  if (optind < argc) {
    if (hp = (struct hostent *)hostname2addr(argv[optind], af))
      bcopy(hp->h_addr_list[0], (af == AF_INET6) ?
	    (void *)&(laddr.sin6_addr) :
	    (void *)&(((struct sockaddr_in *)&laddr)->sin_addr),
	    hp->h_length);
    else
      if (ascii2addr(af, argv[optind], (af == AF_INET6) ?
	    (void *)&(laddr.sin6_addr) :
       	    (void *)&(((struct sockaddr_in *)&laddr)->sin_addr)) < 0) {
	fprintf(stderr, "socktest: %s: bad value\n", argv[optind]);
	exit(1);
    }
  }

  if ((s = socket(af, type, 0)) < 0) {
    perror("socket");
    exit(1);
  }
  
  if (type == SOCK_STREAM) {
    int val = 1;
    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) < 0) {
      perror("setsockopt (TCP_NODELAY)");
      exit(1);
    }
  }

#ifndef NO_SECURITY
  {
    int val, len;
    if (setsockopt(s, SOL_SOCKET, SO_SECURITY_AUTHENTICATION, &ahlev,
		 len = sizeof(int)) < 0) {
      perror("setsockopt (auth)");
      exit(1);
    }
    if (getsockopt(s, SOL_SOCKET, SO_SECURITY_AUTHENTICATION, &val,
		 &len) < 0) {
      perror("setsockopt (auth)");
      exit(1);
    }
    if (val != ahlev) {
      fprintf(stderr, "Requested auth level %d and got %d!\n", ahlev, val);
    }
    if (setsockopt(s, SOL_SOCKET, SO_SECURITY_ENCRYPTION_TRANSPORT, &esplev,
		 len = sizeof(int)) < 0) {
      perror("setsockopt (auth)");
      exit(1);
    }
    if (getsockopt(s, SOL_SOCKET, SO_SECURITY_ENCRYPTION_TRANSPORT, &val,
		 &len) < 0) {
      perror("setsockopt (auth)");
      exit(1);
    }
    if (val != esplev) {
      fprintf(stderr, "Requested esp-transport level %d and got %d!\n", esplev, val);
    }
    if (setsockopt(s, SOL_SOCKET, SO_SECURITY_ENCRYPTION_NETWORK, &esptlev,
		 len = sizeof(int)) < 0) {
      perror("setsockopt (auth)");
      exit(1);
    }
    if (getsockopt(s, SOL_SOCKET, SO_SECURITY_ENCRYPTION_NETWORK, &val,
		 &len) < 0) {
      perror("setsockopt (auth)");
      exit(1);
    }
    if (val != esptlev) {
      fprintf(stderr, "Requested esp-tunnel level %d and got %d!\n", esptlev, val);
    }
  }
#endif /* NO_SECURITY */

  if (server)
    goto servercode;

/* You can't connect() to an unspec address and send packets, so change
   that to loopback */
  if (af == AF_INET6) { 
    if (IS_IN6_UNSPEC(laddr.sin6_addr))
      IN6_ADDR_ASSIGN(laddr.sin6_addr, 0, 0, 0, htonl(1));
  } else {
    if (((struct sockaddr_in *)&laddr)->sin_addr.s_addr == INADDR_ANY)
      ((struct sockaddr_in *)&laddr)->sin_addr.s_addr = htonl(0x7f000001);
  }

  if (connect(s, (struct sockaddr *)&laddr, laddr.sin6_len) < 0) {
       perror("connect");
       exit(1);
  }

  sprintf(mynamestr, "%s [%s.%d]:\n\t", server ? "Server" : "Client",
	 addr2ascii((long)af, (af == AF_INET6) ?
		      (void *)&(laddr.sin6_addr) : 
		      (void *)&(((struct sockaddr_in *)&laddr)->sin_addr),
                    (af == AF_INET6) ? 
                      sizeof(struct in_addr6) : sizeof(struct in_addr),
                    NULL), port);
  say("Ready", NULL);

  while(!feof(stdin)) {
    printf("send: ");
    fgets(datablock, sizeof(datablock), stdin);
    if (p = rindex(datablock, '\n'))
	*p = 0;
    datasize=strlen(datablock)+1;
    if ((write(s, datablock, datasize)) < 0) { 
      perror("write");
      exit(1);
    };
    say("Sent '%s'", datablock);
    fflush(stdout);
    datasize=sizeof(datablock);
    if ((read(s, datablock, datasize)) < 0) {
      perror("read");
      exit(1);
    };
    say("Got  '%s'", datablock);
    fflush(stdout);
  }

servercode:
  if (bind(s, (struct sockaddr *)&laddr, laddr.sin6_len) < 0) {
    perror("bind");
    exit(1);
  }

  if (type == SOCK_STREAM) {
    if (listen(s, 1) < 0) {
      perror("listen");
      exit(1);
    }
  }

  s2 = s;

  sprintf(mynamestr, "%s [%s.%d]:\n\t", server ? "Server" : "Client",
	 addr2ascii((long)af, (af == AF_INET6) ?
		      (void *)&(laddr.sin6_addr) : 
		      (void *)&(((struct sockaddr_in *)&laddr)->sin_addr),
                    (af == AF_INET6) ? 
                      sizeof(struct in_addr6) : sizeof(struct in_addr),
                    NULL), port);
  say("Ready", NULL);

  while(1) {
    i = laddr.sin6_len;
    if (type == SOCK_STREAM) {
      if (!datasize) {
	if ((s = accept(s2, (struct sockaddr *)&faddr, &i)) < 0) {
	  perror("accept");
          exit(1);
	}
	say("Accepted connection from %s",
	    addr2ascii(faddr.sin6_family, (faddr.sin6_family == AF_INET6) ?
		       (void *)&(faddr.sin6_addr) :
		       (void *)&(((struct sockaddr_in *)&faddr)->sin_addr),
		       (faddr.sin6_family == AF_INET6) ?
		       sizeof(struct in_addr6) : sizeof(struct in_addr), 
		       NULL));
	i = laddr.sin6_len;
      }
    }
    if ((datasize = recvfrom(s, datablock, sizeof(datablock) - 1, 0,
			     (struct sockaddr *)&faddr, &i)) < 0) {
      perror("recvfrom");
      exit(1);
    }
    if (datasize) {
      datablock[datasize] = 0;
      say("Got  '%s' from %s", datablock, 
	  addr2ascii(faddr.sin6_family, (faddr.sin6_family == AF_INET6) ?
		     (void *)&(faddr.sin6_addr) :
		     (void *)&(((struct sockaddr_in *)&faddr)->sin_addr),
		     (faddr.sin6_family == AF_INET6) ?
		     sizeof(struct in_addr6) : sizeof(struct in_addr), 
		     NULL));
      fflush(stdout);
      if (sendto(s, datablock, datasize, 0, (struct sockaddr *)&faddr,
		 faddr.sin6_len) < 0) {
	perror("sendto"); 
	exit(1);
      }
    }
    else if (type == SOCK_STREAM)
      {
	fprintf(stderr,"Got EOF, closing connection.\n");
	close(s);
      }
  }
}

usage()
{
  fprintf(stderr,"usage: socktest [-t socket_type] [-a address_fam]");
  fprintf(stderr," [-A level] [-E level]\n");
  fprintf(stderr,"\t [-T level] [-s] [port] [address]\n");
  exit(1);
}
