 /*
  * hosts_ctl() combines the most common applications of the host access
  * control library. routine. It bundles its arguments into a from_host
  * structure, then calls the hosts_access() access control checker. The host
  * name and user name arguments should be empty strings, "unknown" or real
  * data. if a match is found, the optional shell command is executed.
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "@(#) hosts_ctl.c 1.1 92/06/11 22:21:48";
#endif

#include <stdio.h>

#include "log_tcp.h"

/* hosts_ctl - general interface for the hosts_access() routine */

int     hosts_ctl(daemon, name, addr, user)
char   *daemon;
char   *name;
char   *addr;
char   *user;
{
    struct from_host client;
    static struct from_host zeros;

    client = zeros;
    client.name = name;
    client.addr = addr;
    client.user = user;

    return (hosts_access(daemon, &client));
}
