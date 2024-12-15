 /*
  * Front end to the ULTRIX miscd service. The front end logs the remote host
  * name and then invokes the real miscd daemon. Install as "/usr/etc/miscd",
  * after moving the real miscd daemon to the "/usr/etc/..." directory.
  * Connections and diagnostics are logged through syslog(3).
  * 
  * The Ultrix miscd program implements (among others) the systat service, which
  * pipes the output from who(1) to stdout. This information is potentially
  * useful to systems crackers.
  * 
  * Compile with -DHOSTS_ACCESS in order to enable access control. See the
  * hosts_access(5) manual page for details.
  * 
  * Compile with -DPARANOID if service should be refused to hosts that pretend
  * to have someone elses host name. This gives some protection against rsh
  * and rlogin attacks that involve compromised domain name servers.
  * 
  * Compile with -DDAEMON_UMASK=nnn if daemons should run with a non-default
  * umask value (the system default is 000, resulting in world-writable
  * files).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "@(#) miscd.c 1.3 92/06/11 22:21:20";
#endif

/* System libraries. */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <syslog.h>

/* Local stuff. */

#include "log_tcp.h"

/* The following specifies where the vendor-provided daemon should go. */

#define REAL_DAEMON	"/usr/lib/libexec/miscd"

main(argc, argv)
int     argc;
char  **argv;
{
    struct from_host from;
    int     from_stat;

    /* Attempt to prevent the creation of world-writable files. */

#ifdef DAEMON_UMASK
    umask(DAEMON_UMASK);
#endif

    /*
     * Open a channel to the syslog daemon. Older versions of openlog()
     * require only two arguments.
     */

#ifdef LOG_MAIL
    (void) openlog(argv[0], LOG_PID, FACILITY);
#else
    (void) openlog(argv[0], LOG_PID);
#endif

    /*
     * Find out and verify the remote host name. Sites concerned with
     * security may choose to refuse connections from hosts that pretend to
     * have someone elses host name.
     */

    from_stat = fromhost(&from);
    if (from_stat == -1)
#ifdef PARANOID
	refuse(&from);
#else
	dontrefuse(&from);
#endif

    /*
     * Check whether this host can access the service in argv[0]. The
     * access-control code invokes optional shell commands as specified in
     * the access-control tables.
     */

#ifdef HOSTS_ACCESS
    if (!hosts_access(argv[0], &from))
	refuse(&from);
#endif

    /* Report remote client and invoke the real daemon program. */

    syslog(LOG_INFO, "connect from %s", hosts_info(&from));
    (void) execv(REAL_DAEMON, argv);
    syslog(LOG_ERR, "%s: %m", REAL_DAEMON);
    clean_exit(&from);
    /* NOTREACHED */
}
