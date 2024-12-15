 /*
  * try - program to try out host access-control tables, including the
  * optional shell commands.
  * 
  * usage: try process_name host_name_or_address
  * 
  * where process_name is a daemon process name (argv[0] value). If a host name
  * is specified, both the name and address will be used to check the address
  * control tables. If a host address is specified, the program pretends that
  * host name lookup failed.
  */

#ifndef lint
static char sccsid[] = "@(#) try.c 1.2 92/06/11 22:21:32";
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <syslog.h>

#ifdef HOSTS_ACCESS

#ifndef	INADDR_NONE
#define	INADDR_NONE	(-1)		/* XXX should be 0xffffffff */
#endif

#include "log_tcp.h"

/* Try out a (daemon,client) pair */

try(daemon, name, addr)
char   *daemon;
char   *name;
char   *addr;
{
    printf("daemon %s: host name %s (address %s): ",
	   daemon, name, addr);
    printf("access %s\n",
	   hosts_ctl(daemon, name, addr, "you") ? "granted" : "denied");
}

/* function to intercept the real shell_cmd() */

void    shell_cmd(cmd, daemon, client)
char   *cmd;
char   *daemon;
struct from_host *client;
{
    char    buf[BUFSIZ];
    int     pid = getpid();

    percent_x(buf, sizeof(buf), cmd, daemon, client, pid);
    printf("shell command: %s: ", buf);
}

/* function to intercept the real process_options() */

process_options(options, daemon, client)
char   *options;
char   *daemon;
struct from_host *client;
{
    char    buf[BUFSIZ];
    int     pid = getpid();

    percent_x(buf, sizeof(buf), options, daemon, client, pid);
    printf("options: %s: ", buf);
}

main(argc, argv)
int     argc;
char  **argv;
{
    struct hostent *hp;

#ifdef LOG_MAIL
    openlog(argv[0], LOG_PID, FACILITY);
#else
    openlog(argv[0], LOG_PID);
#endif

    if (argc != 3) {
	fprintf(stderr, "usage: %s process_name host_name_or_address\n",
		argv[0]);
	return (1);
    } else {
	if (inet_addr(argv[2]) != INADDR_NONE) {/* pretend host name unknown */
	    try(argv[1], FROM_UNKNOWN, argv[2]);
	} else {
	    if ((hp = gethostbyname(argv[2])) == 0) {	/* bad host name */
		fprintf(stderr, "warning: host unknown: %s\n", argv[2]);
		try(argv[1], argv[2], "?.?.?.?");
	    } else {				/* use both name and address */
		while (hp->h_addr_list[0])
		    try(argv[1], hp->h_name,
			inet_ntoa(*(struct in_addr *) * hp->h_addr_list++));
	    }
	}
	return (0);
    }
}

#else

main()
{
    fprintf(stderr, "host access control is not enabled.\n");
    return (1);
}

#endif
