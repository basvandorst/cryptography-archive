 /*
  * rfc931_user() consults the RFC 931 daemon on the client host to look up
  * the remote user name.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  * 
  * Inspired by the authutil package (comp.sources.unix volume 22) by Dan
  * Bernstein (brnstnd@kramden.acf.nyu.edu).
  */

#ifndef lint
static char sccsid[] = "@(#) rfc931.c 1.2 92/06/22 16:32:14";
#endif

#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>

#include "log_tcp.h"

#define	RFC931_PORT	113		/* Semi-well-known port */
#define	TIMEOUT		10		/* wait for at most 10 seconds */

extern char *strchr();

static jmp_buf timebuf;

/* timeout - handle timeouts */

static void timeout(sig)
int     sig;
{
    longjmp(timebuf, sig);
}

/* rfc931_name - return remote user name */

char   *rfc931_name(there)
struct sockaddr_in *there;		/* remote link information */
{
    struct sockaddr_in here;		/* local link information */
    struct sockaddr_in sin;		/* for talking to RFC931 daemon */
    int     length;
    int     s;
    unsigned remote;
    unsigned local;
    static char user[256];		/* XXX */
    FILE   *fp;
    char   *cp;
    char   *result = FROM_UNKNOWN;

    /* Find out local port number of our stdin. */

    length = sizeof(here);
    if (getsockname(0, (struct sockaddr *) & here, &length) == -1) {
	syslog(LOG_ERR, "getsockname: %m");
	return (result);
    }
    /* Set up timer so we won't get stuck. */

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	return (result);
    signal(SIGALRM, timeout);
    if (setjmp(timebuf)) {
	close(s);				/* not: fclose(fp) */
	return (result);
    }
    alarm(TIMEOUT);

    /* Connect to the RFC931 daemon. */

    sin = *there;
    sin.sin_port = htons(RFC931_PORT);
    if (connect(s, (struct sockaddr *) & sin, sizeof(sin)) == -1
	|| (fp = fdopen(s, "w+")) == 0) {
	close(s);
	alarm(0);
	return (result);
    }
    /* Query the RFC 931 server. Would 13-byte writes ever be broken up? */

    fprintf(fp, "%u,%u\r\n", ntohs(there->sin_port), ntohs(here.sin_port));
    fflush(fp);

    /* Read response. Kill stdio buffer or we may read back our own query. */

    setbuf(fp, (char *) 0);
    if (fscanf(fp, "%u , %u : USERID :%*[^:]:%255s", &remote, &local, user) == 3
	&& ferror(fp) == 0 && feof(fp) == 0
	&& ntohs(there->sin_port) == remote
	&& ntohs(here.sin_port) == local) {
	/* Strip trailing carriage return. */

	if (cp = strchr(user, '\r'))
	    *cp = 0;
	result = user;
    }
    alarm(0);
    fclose(fp);
    return (result);
}
