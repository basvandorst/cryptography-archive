/* ytalkd.c - YTalk pass-thru daemon */

/*			   NOTICE
 *
 * Copyright (c) 1990 Britt Yenne.  All rights reserved.
 * 
 * This software is provided AS-IS.  The author gives no warranty,
 * real or assumed, and takes no responsibility whatsoever for any 
 * use or misuse of this software, or any damage created by its use
 * or misuse.
 * 
 * This software may be freely copied and distributed provided that
 * no part of this NOTICE is deleted or edited in any manner.
 * 
 */

/* Mail comments or questions to yenne@ccwf.cc.utexas.edu */

#include "ytalk.h"
#include <sys/time.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/file.h>

struct sockaddr_in bsd_sock;	/* for communicating udp with talk daemon */
struct sockaddr_in bsd42_sock;	/* for communicating udp with talk daemon */
struct in_addr myaddr;		/* caller's machine address */
char myhost[100];		/* caller's machine hostname */
short bsd_daemon = 0;		/* udp talk daemon port number */
short bsd42_daemon = 0;		/* udp talk daemon version >4.2 */
int bsd_fd, bsd42_fd;		/* socket descriptors */

char errstr[100];
extern int errno;
rcpack rc;

main(argc, argv)
char **argv;
{
    int fd;

    if((fd = open("/dev/tty", O_RDWR)) >= 0)
    {
	ioctl(fd, TIOCNOTTY);
	close(fd);
    }
    close(0);
    close(1);
    close(2);
    if(fork())
	exit(0);

    init_socks();	/* Initialize sockets */
    for(;;)
    {
	readit();
	sendit(myhost);
    }
}

/* Initialize sockets and message parameters.
 */
init_socks()
{
    struct hostent *host;
    struct servent *serv;
    struct passwd *pw;

    if((serv = getservbyname("talk", "udp")) == 0)
    {
	perror("Talk server not found");
	exit(2);
    }
    bsd_daemon = serv->s_port;

    if((serv = getservbyname("ntalk", "udp")) != 0)
	bsd42_daemon = serv->s_port;

    gethostname(myhost, 100);
    if((host = (struct hostent *) gethostbyname(myhost)) == 0)
    {
	sprintf(errstr, "Unknown host: %s", myhost);
	panic(errstr);
	yytalkabort(5);
    }
    bcopy(host->h_addr, &myaddr, host->h_length);
    strcpy(myhost, host->h_name);

    bsd_fd = init_dgram(&bsd_sock, YTPORT);
    bsd42_fd = init_dgram(&bsd42_sock, 0);
}

/* readit() reads a packet from the input datagram socket.
 */
readit()
{
    int n;

    do
    {
	if((n = recv(bsd_fd, &rc, sizeof(rc), 0)) != sizeof(rc))
	{
	    if (errno == EINTR)
		continue;
	    panic("recv failed");
	    continue;
	}
    } while(n <= 0);

    return n;
}

/* sendit() then transmits to the other daemons.
 */
sendit(hostname)
char *hostname;
{
    int n;
    struct hostent *host;
    struct sockaddr_in daemon;

    if((host = (struct hostent *) gethostbyname(hostname)) == 0)
    {
	sprintf(errstr, "Unknown host: %s", hostname);
	panic(errstr);
	return -1;
    }

    if(rc.type)
    {
	daemon.sin_addr.s_addr = ((struct in_addr *) (host->h_addr))->s_addr;

	if(rc.type == 1)
	    daemon.sin_port = bsd_daemon;
	else if(rc.type == 2)
	    daemon.sin_port = bsd42_daemon;
	else
	{
	    sprintf(errstr, "Unkown daemon type: %d", rc.type);
	    panic(errstr);
	    return -1;
	}
    }
    else
	bcopy(rc.buf, &daemon, sizeof(daemon));
    daemon.sin_family = AF_INET;

    if(daemon.sin_port == 0)
	return 0;

    do
    {
	n = sendto(bsd_fd, rc.buf, rc.length, 0, &daemon, sizeof(daemon));
	if (n != rc.length)
	{
	    if (errno == EINTR)	/* interrupted by some stray signal */
		continue;
	    panic("Cannot write to talk daemon");
	    return -1;
	}
    } while(n != rc.length);

    return 0;
}

/* Create a datagram socket.
 */
init_dgram(sock, port)
struct sockaddr_in *sock;
short port;
{
    int fd, socklen;

    sock->sin_family = AF_INET;
    sock->sin_addr.s_addr = INADDR_ANY;
    sock->sin_port = port;
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
	perror("Cannot open DGRAM socket");
	exit(2);
    }
    if(bind(fd, sock, sizeof(struct sockaddr_in), 0) != 0)
    {
	perror("Cannot bind DGRAM socket");
	exit(2);
    }
    socklen = sizeof(struct sockaddr_in);
    if(getsockname(fd, sock, &socklen) == -1)
    {
	perror("Cannot read DGRAM socket address");
	exit(2);
    }
    sock->sin_addr = myaddr;
    return fd;
}

panic(str)
char *str;
{
    fprintf(stderr, "%s\n", str);
    return;
}
