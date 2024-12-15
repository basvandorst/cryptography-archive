 /*
  * refuse() reports a refused connection, and takes the consequences: in
  * case of a datagram-oriented service, the unread datagram is taken from
  * the input queue (or inetd would see the same datagram again and again);
  * the program is terminated.
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "@(#) refuse.c 1.2 92/06/11 22:21:34";
#endif

/* System libraries. */

#include <syslog.h>

/* Local stuff. */

#include "log_tcp.h"

/* refuse - refuse request from bad host */

void    refuse(client)
struct from_host *client;
{
    syslog(LOG_WARNING, "refused connect from %s", hosts_info(client));
    clean_exit(client);
    /* NOTREACHED */
}

/* dontrefuse: accept call but log strangeness */

void    dontrefuse(client)
struct from_host *client;
{
    syslog(LOG_WARNING, "suspicious connection from %s", hosts_info(client));
}
