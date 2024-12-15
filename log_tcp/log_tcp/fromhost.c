 /*
  * fromhost() determines the type of connection (datagram, stream), the name
  * and address of the host at the other end of standard input, and the
  * remote user name (if RFC 931 lookups are enabled). A host name of "stdin"
  * is returned if the program is run from a tty. The value "unknown" is
  * returned as a placeholder for information that could not be looked up.
  * All results are in static memory.
  * 
  * The return status is (-1) if the remote host pretends to have someone elses
  * host name, otherwise a zero status is returned.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "@(#) fromhost.c 1.5 92/06/11 22:21:24";
#endif

/* System libraries. */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>

extern char *inet_ntoa();
extern char *strncpy();
extern char *strcpy();

/* Local stuff. */

#include "log_tcp.h"

/* Forward declarations. */

static int matchname();

/* The following are to be used in assignment context, not in comparisons. */

#define	GOOD	1
#define	BAD	0

/* Initially, we know nothing about the origin of the connection. */

static struct from_host from_unknown = {
    0,					/* connected/unconnected */
    FROM_UNKNOWN,			/* remote host name */
    FROM_UNKNOWN,			/* remote host address */
    FROM_UNKNOWN,			/* remote user name */
};

 /*
  * The Apollo SR10.3 and some SYSV4 getpeername(2) versions do not return an
  * error in case of a datagram-oriented socket. Instead, they claim that all
  * UDP requests come from address 0.0.0.0. The following code works around
  * the problem. It does no harm on "normal" systems.
  */

#ifdef GETPEERNAME_BUG

static int fix_getpeername(sock, sa, len)
int     sock;
struct sockaddr *sa;
int    *len;
{
    int     ret;
    struct sockaddr_in *sin = (struct sockaddr_in *) sa;

    if ((ret = getpeername(sock, sa, len)) >= 0
	&& sa->sa_family == AF_INET
	&& sin->sin_addr.s_addr == 0) {
	errno = ENOTCONN;
	return (-1);
    } else {
	return (ret);
    }
}

#define	getpeername	fix_getpeername
#endif

/* fromhost - find out what is at the other end of standard input */

int     fromhost(f)
struct from_host *f;
{
    struct sockaddr sa;
    struct sockaddr_in *sin = (struct sockaddr_in *) (&sa);
    struct hostent *hp;
    int     length = sizeof(sa);
    char    buf[BUFSIZ];
    static char addr_buf[FROM_ADDRLEN];
    static char name_buf[MAXHOSTNAMELEN];

    /*
     * There are so many results and so many early returns that it seems
     * safest to first initialize all results to UNKNOWN.
     */

    *f = from_unknown;

    /*
     * Look up the remote host address. Hal R. Brand <BRAND@addvax.llnl.gov>
     * suggested how to get the remote host info in case of UDP connections:
     * peek at the first message without actually looking at its contents.
     */

    if (getpeername(0, &sa, &length) >= 0) {	/* assume TCP request */
	f->sock_type = FROM_CONNECTED;
    } else {
	switch (errno) {
	case ENOTSOCK:				/* stdin is not a socket */
	    if (isatty(0))
		f->name = "stdin";
	    return (0);
	case ENOTCONN:				/* assume UDP request */
	    length = sizeof(sa);
	    if (recvfrom(0, buf, sizeof(buf), MSG_PEEK, &sa, &length) < 0) {
		syslog(LOG_ERR, "recvfrom: %m");
		return (0);
	    }
#ifdef really_paranoid
	    memset(buf, 0 sizeof(buf));
#endif
	    f->sock_type = FROM_UNCONNECTED;
	    break;
	default:				/* other, punt */
	    syslog(LOG_ERR, "getpeername: %m");
	    return (0);
	}
    }
    /* At present, we can only deal with the AF_INET address family. */

    if (sa.sa_family != AF_INET) {
	syslog(LOG_ERR, "unexpected address family %ld", (long) sa.sa_family);
	return (0);
    }
    /* Save the host address. A later inet_ntoa() call may clobber it. */

    f->addr = strcpy(addr_buf, inet_ntoa(sin->sin_addr));

    /* Look up the remote user name. Does not work for UDP services. */

#ifdef RFC931
    if (f->sock_type == FROM_CONNECTED)
	f->user = rfc931_name(sin);
#endif

    /* Look up the remote host name. */

    if ((hp = gethostbyaddr((char *) &sin->sin_addr.s_addr,
			    sizeof(sin->sin_addr.s_addr),
			    AF_INET)) == 0) {
	return (0);
    }
    /* Save the host name. A later gethostbyxxx() call may clobber it. */

    f->name = strncpy(name_buf, hp->h_name, sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = 0;

    /*
     * Verify that the host name does not belong to someone else. If host
     * name verification fails, pretend that the host name lookup failed.
     */

    if (matchname(f->name, sin->sin_addr)) {
	return (0);
    } else {
	f->name = FROM_UNKNOWN;
	return (-1);				/* verification failed */
    }
}

/* matchname - determine if host name matches IP address */

static int matchname(remotehost, addr)
char   *remotehost;
struct in_addr addr;
{
    struct hostent *hp;
    int     i;

    if ((hp = gethostbyname(remotehost)) == 0) {

	/*
	 * Unable to verify that the host name matches the address. This may
	 * be a transient problem or a botched name server setup. We decide
	 * to play safe.
	 */

	syslog(LOG_ERR, "gethostbyname(%s): lookup failure", remotehost);
	return (BAD);

    } else {

	if (strcasecmp(remotehost, hp->h_name)) {
	    syslog(LOG_ERR, "host name/name mismatch: %s != %s",
		   remotehost, hp->h_name);
	    return (BAD);
	}
	/* Look up the host address in the address list we just got. */

	for (i = 0; hp->h_addr_list[i]; i++) {
	    if (memcmp(hp->h_addr_list[i], (caddr_t) & addr, sizeof(addr)) == 0)
		return (GOOD);
	}

	/*
	 * The host name does not map to the original host address. Perhaps
	 * someone has compromised a name server. More likely someone botched
	 * it, but that could be dangerous, too.
	 */

	syslog(LOG_ERR, "host name/address mismatch: %s != %s",
	       inet_ntoa(addr), hp->h_name);
	return (BAD);
    }
}

#ifdef TEST

/* Code for stand-alone testing. */

main(argc, argv)
int     argc;
char  **argv;
{
    struct from_host from;

#ifdef LOG_MAIL
    (void) openlog(argv[0], LOG_PID, FACILITY);
#else
    (void) openlog(argv[0], LOG_PID);
#endif
    (void) fromhost(&from);
    printf("%s\n", hosts_info(&from));
    return (0);
}

#endif
