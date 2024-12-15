/* @(#) log_tcp.h 1.2 92/06/11 22:21:30 */

/* Location of the access control files. */

#define HOSTS_ALLOW	"/usr/lib/libexec/hosts.allow"
#define HOSTS_DENY	"/usr/lib/libexec/hosts.deny"

 /*
  * Structure filled in by the fromhost() routine. Prerequisites:
  * <sys/types.h> and <sys/param.h>.
  */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	1024		/* string with host name */
#endif

#define FROM_ADDRLEN	(4*3+3+1)	/* string with IP address */

struct from_host {
    int     sock_type;			/* socket type, see below */
    char   *name;			/* host name */
    char   *addr;			/* host address */
    char   *user;			/* user name */
};

#define FROM_UNKNOWN	"UNKNOWN"	/* name or address lookup failed */
#define FROM_HOST(f) \
    (((f)->name[0] && strcmp((f)->name, FROM_UNKNOWN)) ? (f)->name : (f)->addr)

/* Socket types: 0 means unknown. */

#define FROM_CONNECTED		1	/* connection-oriented */
#define FROM_UNCONNECTED	2	/* non connection-oriented */

/* Global functions. */

extern int fromhost();			/* get/validate remote host info */
extern int hosts_access();		/* access control */
extern void refuse();			/* refuse request */
extern void dontrefuse();		/* refuse request */
extern void shell_cmd();		/* execute shell command */
extern void percent_x();		/* do %<char> expansion */
extern char *rfc931_name();		/* remote name from RFC 931 daemon */
extern char *hosts_info();		/* show origin of connection */
extern void clean_exit();		/* clean up and exit */
