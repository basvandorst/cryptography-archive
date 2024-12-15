 /*
  * clean_exit() cleans up and terminates the program. It should be called
  * instead of exit when for some reason the real network daemon will not or
  * cannot be run. Reason: in the case of a datagram-oriented service we must
  * discard the not-yet received data from the client. Otherwise, inetd will
  * see the same datagram again and again, and go into a loop.
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "@(#) clean_exit.c 1.1 92/06/11 22:21:52";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

extern void exit();

#include "log_tcp.h"

/* clean_exit - clean up and exit */

void    clean_exit(client)
struct from_host *client;
{
    char    buf[BUFSIZ];
    struct sockaddr sa;
    int     size = sizeof(sa);

    /*
     * Eat up the not-yet received packet. Some systems insist on a non-zero
     * source address argument in the recvfrom() call below.
     */

    if (client->sock_type == FROM_UNCONNECTED)
	(void) recvfrom(0, buf, sizeof(buf), 0, &sa, &size);

    /*
     * Be kind to the inetd. We already reported the problem via the syslogd,
     * and there is no need for additional garbage in the logfile.
     */

    exit(0);
}
