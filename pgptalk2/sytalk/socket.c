/* socket.c - socket functions for YTalk V2.0 */

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
#include <ctype.h>
#include <sys/time.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef NS
#include "res.h"
#endif

struct sockaddr_in bsd_sock;	/* for communicating udp with talk daemon */
struct sockaddr_in bsd42_sock;	/* for communicating udp with talk daemon */
struct sockaddr_in yt_sock;	/* for YTalk daemon */
struct in_addr myaddr;		/* caller's machine address */
char myhost[100];		/* caller's machine hostname */
short bsd_daemon = 0;		/* udp talk daemon port number */
short bsd42_daemon = 0;		/* udp talk daemon version >4.2 */
int bsd_fd, bsd42_fd, yt_fd;	/* socket descriptors */

extern char *prog;
extern int errno;
extern person p[MAXC];
extern char errstr[100];
extern int pnum;
CTL_MSG msg;
CTL_RESPONSE rc;
CTL_MSG42 msg42;
CTL_RESPONSE42 rc42;

int autofd;			/* auto invite socket fd */
struct sockaddr_in autosock;	/* auto invite socket */
int autoid = 0;			/* for the talk daemon */

/* We need to keep a list of accessed hosts and their daemon types.
 */
short find_daemon();

typedef struct {
    char *host;			/* host name */
    short dtype;		/* daemon type */
} hostlist;

#define MAXH	MAXC+2
hostlist hostl[MAXH];		/* list of accessed hosts */
int hcnt = 0;			/* count of current host list */

/* Look up a hostid by host name or inet number */

#ifdef NS
HostInfo* DoLookup(char*);
#endif

struct hostent *lookup_host(name)
char *name;
{
    register int n;
    register char *h;
    unsigned char *a;
    struct hostent *host;
    static char init = 0;
    static char addr[20];
    static char* addr_list[1];
    static struct hostent out;

#ifdef NS
    HostInfo* hi;
#endif

    if((host = (struct hostent *) gethostbyname(name)) != NULL)
	return host;
	
    out.h_name = name;
    if(!init)
    {
	out.h_aliases = NULL;
	addr_list[0] = addr;
	out.h_addr_list = (char **) addr_list;
	init = 1;
    }
    
#ifdef NS
    if ((hi = DoLookup(name)) != NULL)
    {
	bcopy(hi->addrList[0], addr, hi->addrLen);
	out.h_length = hi->addrLen;
    }
    else
#endif
    {
	out.h_length = 0;
	a = (unsigned char *) out.h_addr;
	for(h = name, n = 0; *name; name++)
	{
	    if(*name == '.' && h < name)
	    {
		if(++out.h_length >= 20 || n > 255)
			return (struct hostent *) 0;
		*(a++) = n;
		n = 0;
		h = name + 1;
		continue;
	    }
	    else if(*name >= '0' && *name <= '9')
		    n = (n * 10) + (*name - '0');
	    else
		    return (struct hostent *) 0;
	}
	if(++out.h_length >= 20 || n > 255 || h >= name)
		return (struct hostent *) 0;
	*a = n;
    }
    return &out;
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
	panic("Talk server not found");
	yytalkabort(2);
    }
    bsd_daemon = serv->s_port;

    if((serv = getservbyname("ntalk", "udp")) != 0)
	bsd42_daemon = serv->s_port;
    else
	bsd42_daemon = 518;

    gethostname(myhost, 100);
    if((host = lookup_host(myhost)) == 0)
    {
	sprintf(errstr, "Unknown host: %s", myhost);
	panic(errstr);
	yytalkabort(5);
    }
    bcopy(host->h_addr, &myaddr, host->h_length);
#ifdef CODE3
    (char) *((char *) &myaddr + 2) = CODE3;
#endif
    strcpy(myhost, host->h_name);

    bsd_fd = init_dgram(&bsd_sock);
    bsd42_fd = init_dgram(&bsd42_sock);
    yt_fd = init_dgram(&yt_sock);

    if((pw = (struct passwd *) getpwuid(getuid())) == 0)
    {
	panic("Who are you?");
	yytalkabort(1);
    }
    strncpy(msg42.l_name, pw->pw_name, NAME_SIZE);
    strncpy(msg.l_name, msg42.l_name, NAME_SIZE);

    msg42.pid = msg.pid = htonl(getpid());

    msg.ctl_addr = bsd_sock;
    msg.ctl_addr.sin_family = htons(AF_INET);
    msg42.ctl_addr = bsd42_sock;
    msg42.ctl_addr.sin_family = htons(AF_INET);
    msg42.vers = TALK_VERSION;

    (void) find_daemon(myhost);
}

/* Create and initialize the auto-invitation socket.
 */
init_autoport()
{
    int socklen;

    autosock.sin_family = AF_INET;
    autosock.sin_addr.s_addr = INADDR_ANY;
    autosock.sin_port = 0;
    if((autofd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	panic("Cannot open AUTOPORT socket");
	return -1;
    }
    if(bind(autofd,(struct sockaddr *) &autosock, sizeof(struct sockaddr_in)) == -1)
    {
	close(autofd);
	autofd = -1;
	panic("Cannot bind AUTOPORT socket");
	return -1;
    }
    socklen = sizeof(struct sockaddr_in);
    if(getsockname(autofd,(struct sockaddr *) &autosock, &socklen) == -1)
    {
	close(autofd);
	autofd = -1;
	panic("Cannot read AUTOPORT socket address");
	return -1;
    }
    autosock.sin_addr = myaddr;
    if(listen(autofd, 5) == -1)
    {
	close(autofd);
	autofd = -1;
	panic("Cannot listen on AUTOPORT socket");
	return -1;
    }
}

/* The following routines send a request across the DGRAM socket to the
 * BSD talk daemons.
 */

/* First, a quick and easy interface for the user sockets.
 */
int send_dgram(hostname, type, i)
char *hostname;
u_char type;
int i;		/* user number */
{
    int n;
    short dtype, d1, d2;
    short got_ytalk = 0;

    msg.type = type;
    if(type == ANNOUNCE)
	msg.id_num = htonl(p[i].a_id+1);
    else
	msg.id_num = htonl(p[i].id);
    msg.addr = p[i].sock;
    msg.addr.sin_family = htons(AF_INET);

    /* Find the daemon(s) their host supports.  If our two machines support
     * a daemon in common, use that one.  Else, Berkeley "talk" is already
     * screwed to the wall, but YTalk will at least work.
     */
    dtype = find_daemon(hostname);	/* the host I'm sending to */
    d1 = find_daemon(p[i].host);	/* user's host */
    d2 = hostl[0].dtype;		/* my host */
    got_ytalk = dtype & D_YTALK;

    if(dtype == 0 || d1 == 0)
	return -1;
    if((dtype & D_BSD) && (d1 & D_BSD) && (d2 & D_BSD))
	dtype = D_BSD;
    else if((dtype & D_BSD42) && (d1 & D_BSD42) && (d2 & D_BSD42))
	dtype = D_BSD42;
    else if(dtype & D_BSD42)
	dtype = D_BSD42;
    else
	dtype = D_BSD;

    dtype |= got_ytalk;
    if(sendit(hostname, dtype) != 0)
	return -1;

    if(type == ANNOUNCE)
	p[i].a_id = ntohl(rc.id_num);
    else if(type == LEAVE_INVITE || type == LOOK_UP)
	p[i].id = ntohl(rc.id_num);
    return rc.answer;
}

/* Next, an interface for the auto-invite socket.  The auto-invite socket
 * always sends to the caller's host, and always does just an invite.
 */
int send_auto(type)
u_char type;
{
    if(autofd == -1)
	return;
    msg.type = type;
    msg.id_num = htonl(autoid);
    strncpy(msg.r_name, "+AUTO", NAME_SIZE);
    msg.r_tty[0] = '\0';
    msg.addr = autosock;
    msg.addr.sin_family = htons(AF_INET);

    if(hostl[0].dtype & D_BSD42)
	if(sendit(myhost, D_BSD42) != 0)
	    return -1;
    if(hostl[0].dtype & D_BSD)
	if(sendit(myhost, D_BSD) != 0)
	    return -1;

    autoid = ntohl(rc.id_num);
    if(type == LEAVE_INVITE)
	return 0;
    return rc.answer;
}

/* sendit() sends the completed message to the talk daemon at the given
 * hostname, then reads a response packet.
 */
sendit(hostname, dtype)
char *hostname;
short dtype;	/* daemon type */
{
    int n, sel, fd;
    struct hostent *host;
    struct sockaddr_in daemon;
    struct timeval tv;
    char *packet, *rcp;
    int mlen, rlen;
    char *rtype, *mtype;
    rcpack ypack;

    if((host = lookup_host(hostname)) == 0)
    {
	sprintf(errstr, "Unknown host: %s", hostname);
	panic(errstr);
	return -1;
    }

    daemon.sin_family = AF_INET;
    daemon.sin_addr.s_addr = ((struct in_addr *) (host->h_addr))->s_addr;

    /* If this host supports the BSD4.2 daemon, set up that structure.
     */
    if(dtype & D_BSD42)
    {
	msg42.vers = TALK_VERSION;
	msg42.type = msg.type;
	msg42.addr = msg.addr;
	msg42.id_num = msg.id_num;
	strncpy(msg42.l_name, msg.l_name, NAME_SIZE);
	strncpy(msg42.r_name, msg.r_name, NAME_SIZE);
	strncpy(msg42.r_tty, msg.r_tty, TTY_SIZE);
    }

    /* Now determine which daemon to send to, and initialize the transfer
     * variables.
     */
    if(dtype & D_BSD42)
    {
	daemon.sin_port = bsd42_daemon;
	packet = (char *) &msg42;
	rcp = (char *) &rc42;
	mlen = sizeof(msg42);
	rlen = sizeof(rc42);
	mtype = &msg42.type;
	rtype = &rc42.type;
	fd = bsd42_fd;
	if(dtype & D_YTALK)
	{
	    daemon.sin_port = YTPORT;
	    ypack.type = 2;
	    ypack.length = sizeof(msg42);
	    bcopy(&msg42, ypack.buf, ypack.length);
	    packet = (char *) &ypack;
	    mlen = sizeof(ypack);
	}
    }
    else if(dtype & D_BSD)
    {
	daemon.sin_port = bsd_daemon;
	packet = (char *) &msg;
	rcp = (char *) &rc;
	mlen = sizeof(msg);
	rlen = sizeof(rc);
	mtype = &msg.type;
	rtype = &rc.type;
	fd = bsd_fd;
	if(dtype & D_YTALK)
	{
	    daemon.sin_port = YTPORT;
	    ypack.type = 1;
	    ypack.length = sizeof(msg);
	    bcopy(&msg, ypack.buf, ypack.length);
	    packet = (char *) &ypack;
	    mlen = sizeof(ypack);
	}
    }
    else
    {
	sprintf(errstr, "Unkown daemon type: %d", dtype);
	panic(errstr);
	return -1;
    }

    /* Now clear out any extraneous input still remaining on either of the
     * two sockets.  Due to dual-daemon support, this may sometimes be
     * necessary in order to prevent confusion.
     */
    for(;;)
    {
	tv.tv_sec = 0L;
	tv.tv_usec = 0L;
	sel = (1<<bsd_fd) | (1<<bsd42_fd);
	if((n = select(32, &sel, 0, 0, &tv)) <= 0)
	    break;
	if(sel & (1<<bsd_fd))
	    if(recv(bsd_fd, &rc, sizeof(rc), 0) <= 0)
	    {
		if (errno == EINTR)
		    continue;
		panic("recv failed");
		break;
	    }
	if(sel & (1<<bsd42_fd))
	    if(recv(bsd42_fd, &rc42, sizeof(rc42), 0) <= 0)
	    {
		if (errno == EINTR)
		    continue;
		panic("recv42 failed");
		break;
	    }
    }

    /* Now we need to send the actual packet.  Due to unreliability of
     * DGRAM sockets, we must resend the packet until we get a response
     * from the server.  Geez... two different daemons, both on unreliable
     * sockets, and maybe even different daemons on different machines.
     * Is *nothing* reliable anymore???
     */
    do
    {
    	do
	{
	    n = sendto(fd, packet, mlen, 0,(struct sockaddr *) &daemon, sizeof(daemon));
	    if (n != mlen)
	    {
		if (errno == EINTR)	/* interrupted by some stray signal */
		    continue;
		panic("Cannot write to talk daemon");
		return -1;
	    }

	    tv.tv_sec = 5L;
	    tv.tv_usec = 0L;
	    sel = 1<<fd;
	    if((n = select(32, &sel, 0, 0, &tv)) < 0)
	    {
		if(errno == EINTR)
		    continue;
		panic("first select failed in sendit()");
		return -1;
	    }
 	} while (n == 0);	/* ie: until we receive a reply */

    	do
	{
	    n = recv(fd, rcp, rlen, 0);
	    if (n < 0)
	    {
		if (errno == EINTR)
		    continue;
		panic("Cannot read from talk daemon");
		return -1;
	    }

	    tv.tv_sec = 0L;
	    tv.tv_usec = 0L;
	    sel = 1<<fd;
	    if((n = select(32, &sel, 0, 0, &tv)) < 0)
	    {
		if(errno == EINTR)
		    continue;
		panic("second select failed in sendit()");
		return -1;
	    }
    	} while(n > 0 && *rtype != *mtype);
    } while(*rtype != *mtype);

    /* WHEW */

    if(dtype & D_BSD42)
    {
	rc.type = rc42.type;
	rc.answer = rc42.answer;
	rc.id_num = rc42.id_num;
	rc.addr = rc42.addr;
    }

    /* Just because a person is a SYSADMIN doesn't necessarily mean he/she
     * knows everything about installing software.  In fact, many have been
     * known to install the talk daemon without setting the option required
     * to pad out the structures so that "long"s are on four-byte boundaries
     * on machines where "long"s can be on two-byte boundaries.  This "bug"
     * cost me about four hours of debugging to discover, so I'm not happy
     * right now.  Anyway, here's a quick hack to fix this problem.
     */
    if(rc.type == LOOK_UP && rc.answer == 0)
    {
	u_short t;
	bcopy(((char *)&rc.addr.sin_family)-2, (char *)&t, sizeof(t));
	if(ntohs(t) == AF_INET && ntohs(rc.addr.sin_family) != AF_INET)
	{
	    char *c;
	    c = ((char *)&rc) + sizeof(rc) - 1;
	    for(; c >= (char *)&rc.id_num; c--)
		*c = *(c-2);
	}
    }

    return 0;
}

/* find_daemon() locates the talk daemon(s) on a machine and determines
 * what version of the daemon is running.
 */
short find_daemon(hostname)
char *hostname;
{
    register int n, i;
    CTL_MSG m1;
    CTL_MSG42 m2;
    CTL_RESPONSE r1;
    CTL_RESPONSE42 r2;
    struct sockaddr_in daemon;
    struct hostent *host;
    struct timeval tv;
    int m1len, m2len;
    int sel;
    static short out;
    rcpack ypack;

    if(bsd42_daemon == 0)
	return D_BSD;

    /* If we've already used this host, look it up instead of blitting to
     * the daemons again...
     */
    for(n = 0; n < hcnt; n++)
	if(strncmp(hostl[n].host, hostname, HOSTLEN) == 0)
	    return hostl[n].dtype;
    if(hcnt >= MAXH)
    {
	errno = 0;
	panic("Too many different hostnames!");
	return D_UNKNOWN;
    }

    if((host = lookup_host(hostname)) == 0)
	return D_UNKNOWN;

    if((hostl[hcnt].host = (char *) malloc(HOSTLEN)) == NULL)
    {
	panic("Cannot allocate memory");
	return D_UNKNOWN;
    }
    strncpy(hostl[hcnt].host, hostname, HOSTLEN);

    daemon.sin_family = AF_INET;
    daemon.sin_addr.s_addr = ((struct in_addr *) (host->h_addr))->s_addr;

    m1 = msg;
    m2 = msg42;
    m1.type = m2.type = LOOK_UP;
    m1.id_num = m2.id_num = htonl(0);
    m1.r_tty[0] = m2.r_tty[0] = '\0';
    strcpy(m1.r_name, "ytalk");
    strcpy(m2.r_name, "ytalk");
    m1.ctl_addr = bsd_sock;
    m1.ctl_addr.sin_family = htons(AF_INET);
    m2.ctl_addr = bsd42_sock;
    m2.ctl_addr.sin_family = htons(AF_INET);
    m1.addr.sin_family = m2.addr.sin_family = htons(AF_INET);

    ypack.type = 0;
    ypack.length = sizeof(yt_sock);
    bcopy(&yt_sock, ypack.buf, ypack.length);

    out = 0;
    for(i = 0; i < 5; i++)
    {
	if(out & D_YTALK)
	{
	    daemon.sin_port = YTPORT;
	    ypack.type = 2;
	    ypack.length = sizeof(m2);
	    bcopy(&m2, ypack.buf, ypack.length);
	    n = sendto(bsd42_fd, &ypack, sizeof(ypack), 0,(struct sockaddr *) &daemon, sizeof(daemon));
	    if(n != sizeof(ypack) && errno != EINTR)
		perror("Warning: cannot write to YTalk daemon");

	    ypack.type = 1;
	    ypack.length = sizeof(m1);
	    bcopy(&m1, ypack.buf, ypack.length);
	    n = sendto(bsd_fd, &ypack, sizeof(ypack), 0,(struct sockaddr *) &daemon, sizeof(daemon));
	    if(n != sizeof(ypack) && errno != EINTR)
		perror("Warning: cannot write to YTalk daemon");
	}
	else
	{
	    daemon.sin_port = bsd42_daemon;
	    n = sendto(bsd42_fd, &m2, sizeof(m2), 0,(struct sockaddr *) &daemon, sizeof(daemon));
	    if(n != sizeof(m2) && errno != EINTR)
		perror("Warning: cannot write to post-4.2 talk daemon");

	    daemon.sin_port = bsd_daemon;
	    n = sendto(bsd_fd, &m1, sizeof(m1), 0,(struct sockaddr *) &daemon, sizeof(daemon));
	    if(n != sizeof(m1) && errno != EINTR)
		perror("Warning: cannot write to pre-4.2 talk daemon");

	    daemon.sin_port = YTPORT;
	    n=sendto(yt_fd, &ypack, sizeof(ypack), 0,(struct sockaddr *) &daemon, sizeof(daemon));
	    if(n != sizeof(ypack) && errno != EINTR)
		perror("Warning: cannot write to YTalk daemon");
	}

	tv.tv_sec = 4L;
	tv.tv_usec = 0L;
	sel = (1<<bsd_fd) | (1<<bsd42_fd) | (1<<yt_fd);
	if((n = select(32, &sel, 0, 0, &tv)) < 0)
	{
	    if(errno == EINTR)
		continue;
	    panic("first select failed in find_daemon()");
	    continue;
	}
	if(n == 0)
	    continue;

	do
	{
	    if(sel & (1<<bsd_fd))
	    {
		out |= D_BSD;
		if(recv(bsd_fd, &r1, sizeof(r1), 0) < 0)
		    panic("Cannot read from pre-4.2 talk daemon!");
	    }
	    if(sel & (1<<bsd42_fd))
	    {
		out |= D_BSD42;
		if(recv(bsd42_fd, &r2, sizeof(r2), 0) < 0)
		    panic("Cannot read from post-4.2 talk daemon!");
	    }
	    if(sel & (1<<yt_fd))
	    {
		out |= D_YTALK;
		if(recv(yt_fd, ypack.buf, RCSIZ, 0) < 0)
		    panic("Cannot read from YTalk daemon!");
	    }
	    tv.tv_sec = 0L;
	    tv.tv_usec = 500000L;	/* give the other daemon a chance */
	    sel = (1<<bsd_fd) | (1<<bsd42_fd);
	    if((n = select(32, &sel, 0, 0, &tv)) < 0)
	    {
		if(errno == EINTR)
		    continue;
		panic("second select failed in find_daemon()");
	    }
	} while(n > 0);
	if(out == D_YTALK)
	    continue;

	hostl[hcnt++].dtype = out;
	return out;
    }
    sprintf(errstr, "No talk daemon on %s", hostname);
    panic(errstr);
    free(hostl[hcnt].host);
    return D_UNKNOWN;
}

/* Create a datagram socket.
 */
init_dgram(sock)
struct sockaddr_in *sock;
{
    int fd, socklen;

    sock->sin_family = AF_INET;
    sock->sin_addr.s_addr = INADDR_ANY;
    sock->sin_port = 0;
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
	panic("Cannot open DGRAM socket");
	yytalkabort(2);
    }
    if(bind(fd, (struct sockaddr *) sock, sizeof(struct sockaddr_in)) != 0)
    {
	close(fd);
	panic("Cannot bind DGRAM socket");
	yytalkabort(2);
    }
    socklen = sizeof(struct sockaddr_in);
    if(getsockname(fd,(struct sockaddr *) sock, &socklen) == -1)
    {
	close(fd);
	panic("Cannot read DGRAM socket address");
	yytalkabort(2);
    }
    sock->sin_addr = myaddr;
    return fd;
}


/* Create a TCP socket for communication with other talk users.
 */
int newsock(i)
{
    int socklen;

    p[i].sock.sin_family = AF_INET;
    p[i].sock.sin_addr.s_addr = INADDR_ANY;
    p[i].sock.sin_port = 0;
    if((p[i].sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	panic("Cannot open socket");
	return -1;
    }
    if(bind(p[i].sfd, (struct sockaddr *) &p[i].sock, sizeof(struct sockaddr_in)) == -1)
    {
	close(p[i].sfd);
	panic("Cannot bind socket");
	return -1;
    }
    socklen = sizeof(struct sockaddr_in);
    if(getsockname(p[i].sfd, (struct sockaddr *) &p[i].sock, &socklen) == -1)
    {
	close(p[i].sfd);
	panic("Cannot read socket address");
	return -1;
    }
    p[i].sock.sin_addr = myaddr;
    if(listen(p[i].sfd, 5) == -1)
    {
	close(p[i].sfd);
	panic("Cannot listen on socket");
	return -1;
    }
    return 0;
}

/* Connect to another user's communication socket.
 */
int connect_to(i)
int i;	/* user number, or -1 to just return a file descriptor */
{
    int socklen, fd;
    struct sockaddr_in sock;

    sock = *(struct sockaddr_in *)&rc.addr;
    sock.sin_family = AF_INET;
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	panic("Cannot open socket");
	return -1;
    }
    if(connect(fd,(struct sockaddr *) &sock, sizeof(struct sockaddr_in)) == -1)
    {
	close(fd);
	if(errno == ECONNREFUSED)
	    return -2;
	panic("Cannot connect to host");
	return -1;
    }
    socklen = sizeof(struct sockaddr_in);
    if(getsockname(fd, (struct sockaddr *) &sock, &socklen) == -1)
    {
	close(fd);
	panic("Cannot read socket address");
	return -1;
    }
    if(i != -1)
    {
	p[i].sock = sock;
	p[i].sfd = fd;
	p[i].flags |= P_CONNECT;
    }
    return fd;
}

