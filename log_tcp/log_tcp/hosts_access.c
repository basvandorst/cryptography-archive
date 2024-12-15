 /*
  * This module implements a simple access control language that is based on
  * host (or domain) names, netgroup, internet addresses (or network numbers)
  * and daemon process names. When a match is found an optional shell command
  * is executed and the search is terminated.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Compile with -DNETGROUP if your library provides support for netgroups.
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "@(#) hosts_access.c 1.8 92/06/12 15:55:52";
#endif

 /* System libraries. */

#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <syslog.h>
#include <ctype.h>

extern char *fgets();
extern char *strchr();
extern char *strtok();

#ifndef	INADDR_NONE
#define	INADDR_NONE	(-1)		/* XXX should be 0xffffffff */
#endif

/* Local stuff. */

#include "log_tcp.h"

/* Delimiters for lists of daemons or clients. */

static char sep[] = ", \t";

/* Constants to be used in assignments only, not in comparisons... */

#define	YES		1
#define	NO		0
#define	FAIL		(-1)

/* Forward declarations. */

static int table_match();
static int list_match();
static int masked_match();
static void list_split();

/* hosts_access - host access control facility */

int     hosts_access(daemon, client)
char   *daemon;
struct from_host *client;		/* host or user name may be empty */
{

    /*
     * If the (daemon, client) pair is matched by an entry in the file
     * /etc/hosts.allow, access is granted. Otherwise, if the (daemon,
     * client) pair is matched by an entry in the file /etc/hosts.deny,
     * access is denied. Otherwise, access is granted. A non-existent
     * access-control file is treated as an empty file.
     */

    if (table_match(HOSTS_ALLOW, daemon, client))
	return (YES);
    if (table_match(HOSTS_DENY, daemon, client))
	return (NO);
    return (YES);
}

/* table_match - match table entries with (daemon, client) pair */

static int table_match(table, daemon, client)
char   *table;
char   *daemon;
struct from_host *client;		/* host or user name may be empty */
{
    FILE   *fp;
    char    sv_list[BUFSIZ];		/* becomes list of daemons */
    char   *cl_list;			/* becomes list of clients */
    char   *argv[sizeof(sv_list) / 2 + 1];
    char   *sh_cmd;			/* becomes optional shell command */
    int     match;
    int     end;

    /* The following variables should always be tested together. */

    int     sv_match = NO;		/* daemon matched */
    int     cl_match = NO;		/* client matced */

    /*
     * Process the table one line at a time. Lines that begin with a '#'
     * character are ignored. Non-comment lines are broken at the ':'
     * character (we complain if there is none). The first field is matched
     * against the daemon process name (argv[0]), the second field against
     * the host name or address. A non-existing table is treated as if it
     * were an empty table. When a match is found an optional shell command
     * is executed.
     * 
     * Daemon and client lists are converted to arrays of character pointers
     * because the client list may have to be processed twice: once to match
     * the host name, and once for matching the address.
     */

    if (fp = fopen(table, "r")) {
	while (!(sv_match && cl_match) && fgets(sv_list, sizeof(sv_list), fp)) {
	    if (sv_list[end = strlen(sv_list) - 1] != '\n') {
		syslog(LOG_ERR, "%s: missing newline or line too long", table);
		continue;
	    } else {
		sv_list[end] = '\0';		/* strip trailing newline */
	    }
	    if (sv_list[0] == '#')		/* skip comments */
		continue;
	    if ((cl_list = strchr(sv_list, ':')) == 0) {
		syslog(LOG_ERR, "%s: malformed entry: \"%s\"", table, sv_list);
		continue;
	    }
	    *cl_list++ = '\0';			/* split 1st and 2nd fields */
	    if ((sh_cmd = strchr(cl_list, ':')) != 0)
		*sh_cmd++ = '\0';		/* split 2nd and 3rd fields */
	    list_split(sv_list, argv);
	    if ((sv_match = list_match(argv, daemon)) != 0) {
		list_split(cl_list, argv);
		if (client->name[0] == 0
		    || (cl_match = list_match(argv, client->name)) == 0)
		    cl_match = list_match(argv, client->addr);
	    }
	}
	(void) fclose(fp);
    }
    match = (sv_match == YES && cl_match == YES);
    if (match && sh_cmd)
	shell_cmd(sh_cmd, daemon, client);
    return (match);
}

/* list_split - destructively convert string to array of character pointers */

static void list_split(string, argv)
register char *string;
register char **argv;
{
    for (*argv = strtok(string, sep); *argv; *++argv = strtok((char *) 0, sep))
	 /* void */ ;
}

/* list_match - match a string against a list of tokens */

static int list_match(list, string)
char  **list;
char   *string;
{
    char   *tok;
    int     tok_len;
    int     str_len;
    char   *cut;

    /*
     * Process tokens one at a time. Return YES if a token has the magic
     * value "ALL". Return FAIL if the token is "FAIL". If the token starts
     * with a "." (domain name), return YES if it matches the last fields of
     * the string. If the token has the magic value "LOCAL", return YES if
     * the string does not contain a "." character. If the token ends on a
     * "." (network number), return YES if it matches the first fields of the
     * string. If the token begins with a "@" (netgroup name), return YES if
     * the string is a (host) member of the netgroup. Return YES if the token
     * fully matches the string. If the token is a netnumber/netmask pair,
     * return YES if the address is a member of the specified subnet.
     */

    for (tok = *list; tok; tok = *++list) {
	if (tok[0] == '.') {			/* domain: match last fields */
	    if ((str_len = strlen(string)) > (tok_len = strlen(tok))
		&& strcasecmp(tok, string + str_len - tok_len) == 0)
		return (YES);
#ifdef	NETGROUP
	} else if (tok[0] == '@') {		/* netgroup: look it up */
	    if (!isdigit(string[0])
		&& innetgr(tok + 1, string, (char *) 0, (char *) 0))
		return (YES);
#endif
	} else if (strcasecmp(tok, "ALL") == 0) {	/* all: match any */
	    return (YES);
	} else if (strcasecmp(tok, "FAIL") == 0) {	/* fail: match any */
	    return (FAIL);
	} else if (strcasecmp(tok, "LOCAL") == 0) {	/* local: no dots */
	    if (strchr(string, '.') == 0 && strcasecmp(string, "unknown") != 0)
		return (YES);
	} else if (!strcasecmp(tok, string)) {	/* match host name or address */
	    return (YES);
	} else if (tok[(tok_len = strlen(tok)) - 1] == '.') {	/* network */
	    if (strncmp(tok, string, tok_len) == 0)
		return (YES);
	} else if ((cut = strchr(tok, '/')) != 0) {	/* netnumber/netmask */
	    if (isdigit(string[0]) && masked_match(tok, cut, string))
		return (YES);
	}
    }
    return (NO);
}

/* masked_match - match address against netnumber/netmask */

static int masked_match(tok, slash, string)
char   *tok;
char   *slash;
char   *string;
{
    unsigned long net;
    unsigned long mask;
    unsigned long addr;

    if ((addr = inet_addr(string)) == INADDR_NONE)
	return (NO);
    *slash = 0;
    net = inet_addr(tok);
    *slash = '/';
    if (net == INADDR_NONE || (mask = inet_addr(slash + 1)) == INADDR_NONE) {
	syslog(LOG_ERR, "bad net/mask access control: %s", tok);
	return (NO);
    }
    return ((addr & mask) == net);
}
