 /*
  * This module implements a simple but effective form of login access
  * control based on login names and on host (or domain) names, internet
  * addresses (or network numbers), or on terminal line names in case of
  * non-networked logins. Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */

#ifndef lint
static char sccsid[] = "%Z% %M% %I% %E% %U%";
#endif

#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include <ctype.h>
#include <grp.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

extern struct group *getgrnam();
extern int errno;

#if !defined(MAXHOSTNAMELEN) || (MAXHOSTNAMELEN < 64)
#undef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

 /* Path name of the access control file. */

#ifndef	TABLE
#define TABLE	"/etc/login.access"
#endif

 /* Delimiters for fields and for lists of users, ttys or hosts. */

static char fs[] = ":";			/* field separator */
static char sep[] = ", \t";		/* list-element separator */

 /* Constants to be used in assignments only, not in comparisons... */

#define YES             1
#define NO              0

static int list_match();
static int user_match();
static int from_match();
static int string_match();

/* login_access - match username/group and host/tty with access control file */

login_access(user, from)
char   *user;
char   *from;
{
    FILE   *fp;
    char    line[BUFSIZ];
    char   *perm;			/* becomes permission field */
    char   *users;			/* becomes list of login names */
    char   *froms;			/* becomes list of terminals or hosts */
    int     match = NO;
    int     end;
    int     lineno = 0;			/* for diagnostics */

    /*
     * Process the table one line at a time and stop at the first match.
     * Blank lines and lines that begin with a '#' character are ignored.
     * Non-comment lines are broken at the ':' character. All fields are
     * mandatory. The first field should be a "+" or "-" character. A
     * non-existing table means no access control.
     */

    if (fp = fopen(TABLE, "r")) {
	while (!match && fgets(line, sizeof(line), fp)) {
	    lineno++;
	    if (line[end = strlen(line) - 1] != '\n') {
		syslog(LOG_ERR, "%s: line %d: missing newline or line too long",
		       TABLE, lineno);
		continue;
	    }
	    if (line[0] == '#')
		continue;			/* comment line */
	    while (end > 0 && isspace(line[end - 1]))
		end--;
	    line[end] = 0;			/* strip trailing whitespace */
	    if (line[0] == 0)			/* skip blank lines */
		continue;
	    if (!(perm = strtok(line, fs))
		|| !(users = strtok((char *) 0, fs))
		|| !(froms = strtok((char *) 0, fs))
		|| strtok((char *) 0, fs)) {
		syslog(LOG_ERR, "%s: line %d: bad field count", TABLE, lineno);
		continue;
	    }
	    if (perm[0] != '+' && perm[0] != '-') {
		syslog(LOG_ERR, "%s: line %d: bad first field", TABLE, lineno);
		continue;
	    }
	    match = (list_match(froms, from, from_match)
		     && list_match(users, user, user_match));
	}
	(void) fclose(fp);
    } else if (errno != ENOENT) {
	syslog(LOG_ERR, "cannot open %s: %m", TABLE);
    }
    return (match == 0 || (line[0] == '+'));
}

/* list_match - match an item against a list of tokens with exceptions */

static int list_match(list, item, match_fn)
char   *list;
char   *item;
int   (*match_fn) ();
{
    char   *tok;
    int     match = NO;

    /*
     * Process tokens one at a time. We have exhausted all possible matches
     * when we reach an "EXCEPT" token or the end of the list. If we do find
     * a match, look for an "EXCEPT" list and recurse to determine whether
     * the match is affected by any exceptions.
     */

    for (tok = strtok(list, sep); tok != 0; tok = strtok((char *) 0, sep)) {
	if (strcasecmp(tok, "EXCEPT") == 0)	/* EXCEPT: give up */
	    break;
	if (match = (*match_fn) (tok, item))	/* YES */
	    break;
    }
    /* Process exceptions to matches. */

    if (match != NO) {
	while ((tok = strtok((char *) 0, sep)) && strcasecmp(tok, "EXCEPT"))
	     /* VOID */ ;
	if (tok == 0 || list_match((char *) 0, item, match_fn) == NO)
	    return (match);
    }
    return (NO);
}

/* myhostname - figure out local machine name */

static char *myhostname()
{
    static char name[MAXHOSTNAMELEN + 1] = "";

    if (name[0] == 0) {
	gethostname(name, sizeof(name));
	name[MAXHOSTNAMELEN] = 0;
    }
    return (name);
}

/* netgroup_match - match group against machine or user */

static int netgroup_match(group, machine, user)
char   *group;
char   *machine;
char   *user;
{
#ifdef NIS
    static char *mydomain = 0;

    if (mydomain == 0)
	yp_get_default_domain(&mydomain);
    return (innetgr(group, machine, user, mydomain));
#else
    syslog(LOG_ERR, "NIS netgroup support not configured");
    return (NO);
#endif
}

/* user_match - match a username against one token */

static int user_match(tok, string)
char   *tok;
char   *string;
{
    struct group *group;
    int     i;
    char   *at;

    /*
     * If a token has the magic value "ALL" the match always succeeds.
     * Otherwise, return YES if the token fully matches the username, or if
     * the token is a group that contains the username.
     */

    if ((at = strchr(tok + 1, '@')) != 0) {	/* split user@host pattern */
	*at = 0;
	return (user_match(tok, string) && from_match(at + 1, myhostname()));
    } else if (tok[0] == '@') {			/* netgroup */
	return (netgroup_match(tok + 1, (char *) 0, string));
    } else if (string_match(tok, string)) {	/* ALL or exact match */
	return (YES);
    } else if (group = getgrnam(tok)) {		/* try group membership */
	for (i = 0; group->gr_mem[i]; i++)
	    if (strcasecmp(string, group->gr_mem[i]) == 0)
		return (YES);
    }
    return (NO);
}

/* from_match - match a host or tty against a list of tokens */

static int from_match(tok, string)
char   *tok;
char   *string;
{
    int     tok_len;
    int     str_len;

    /*
     * If a token has the magic value "ALL" the match always succeeds. Return
     * YES if the token fully matches the string. If the token is a domain
     * name, return YES if it matches the last fields of the string. If the
     * token has the magic value "LOCAL", return YES if the string does not
     * contain a "." character. If the token is a network number, return YES
     * if it matches the head of the string.
     */

    if (tok[0] == '@') {			/* netgroup */
	return (netgroup_match(tok + 1, string, (char *) 0));
    } else if (string_match(tok, string)) {	/* ALL or exact match */
	return (YES);
    } else if (tok[0] == '.') {			/* domain: match last fields */
	if ((str_len = strlen(string)) > (tok_len = strlen(tok))
	    && strcasecmp(tok, string + str_len - tok_len) == 0)
	    return (YES);
    } else if (strcasecmp(tok, "LOCAL") == 0) {	/* local: no dots */
	if (strchr(string, '.') == 0)
	    return (YES);
    } else if (tok[(tok_len = strlen(tok)) - 1] == '.'	/* network */
	       && strncmp(tok, string, tok_len) == 0) {
	return (YES);
    }
    return (NO);
}

/* string_match - match a string against one token */

static int string_match(tok, string)
char   *tok;
char   *string;
{

    /*
     * If the token has the magic value "ALL" the match always succeeds.
     * Otherwise, return YES if the token fully matches the string.
     */

    if (strcasecmp(tok, "ALL") == 0) {		/* all: always matches */
	return (YES);
    } else if (strcasecmp(tok, string) == 0) {	/* try exact match */
	return (YES);
    }
    return (NO);
}
