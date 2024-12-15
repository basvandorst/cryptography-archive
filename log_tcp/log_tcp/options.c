 /*
  * General skeleton for adding options to the access control language. This
  * code can be enabled by replacing in hosts_access.c the shell_command()
  * call by a call to process_options(), with the same argument list. Shell
  * commands will still be available, be it with a slightly different syntax.
  * 
  * The code uses a slightly different format of access control rules. It
  * assumes that an access control rule looks like this:
  * 
  * daemon_list : client_list : option : option ...
  * 
  * An option is of the form "keyword" or "keyword = value". Option fields are
  * processed from left to right. Blanks around keywords, "="  and values are
  * optional.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Examples of options that are already implemented by the current skeleton:
  * 
  * user = nobody
  * 
  * Causes the process to switch its user id to that of "nobody". This normally
  * requires root privilege.
  * 
  * group = tty
  * 
  * Causes the process to change its group id to that of the "tty" group. In
  * order to switch both user and group ids you should normally switch the
  * group id before switching the user id.
  * 
  * spawn = /usr/ucb/finger -l @%h | /usr/ucb/mail root
  * 
  * Executes (in a child process) the shell command "finger -l @%h | mail root"
  * after doing the %<character> expansions described in the hosts_access(5)
  * manual page. The command is executed with stdin, stdout and stderr
  * connected to the null device. Because options are processed in order,
  * multiple spawn comands can be specified within the same access control
  * rule, though "spawn = command1; command2" would be more efficient.
  * 
  * in.ftpd : client_list : twist = /bin/echo 421 Some bogus error message
  * 
  * Sends some bogus error message to the remote client instead of running the
  * real ftp daemon. The command is subjected to %<character> expansion and
  * executed by /bin/sh. Stdin, stdout and stderr are not redirected. Some
  * inetd's only pass stdin to their servers; in that case the command should
  * write its output to stdin, as with: "command >&0". The twist'ed command
  * overlays the current process; it makes no sense to specify other options
  * on the same line after a "twist". The "twist" option was inspired by Dan
  * Bernstein's shuctl daemon wrapper control language.
  * 
  * The decision to do RFC 931 lookups can be moved to this code, too. This can
  * be accomplished by feeding the rfc931_option file to the "patch" command.
  * In that case you get a fifth option:
  * 
  * rfc931
  * 
  * Causes the daemon front ends to look up the remote user name with the RFC
  * 931 protocol.
  * 
  * Warnings:
  * 
  * This module uses the non-reentrant strtok() library routine. The options
  * argument to process_options() is destroyed.
  * 
  * There cannot be a ":" character in keywords or values. Backslash sequences
  * are not yet recognized.
  * 
  * In case of UDP connections, do not "twist" commands that use the standard
  * I/O or read(2)/write(2) routines to communicate with the client process.
  * 
  * In case of errors, use clean_exit() instead of directly calling exit(), or
  * your inetd may go into a loop.
  */

/* System libraries. */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <syslog.h>
#include <pwd.h>
#include <ctype.h>

extern char *strtok();
extern char *strchr();

/* Local stuff. */

#include "log_tcp.h"

/* List of functions that implement the options. Add yours here. */

static void user_option();		/* execute "user=name" option */
static void group_option();		/* execute "group=name" option */
static void twist_option();		/* execute "twist=command" option */
#ifdef RFC931_OPTION
static void rfc931_option();		/* execute "rfc931" option */
#endif

static char *chop_string();		/* strip leading and trailing blanks */

/* Structure of the options table. */

struct option {
    char   *name;			/* keyword name, case does not matter */
    int     need_value;			/* value required or not */
    void    (*func) ();			/* function that does the real work */
};

/* List of known keywords. Add yours here. */

static struct option option_table[] = {
    "user", 1, user_option,		/* switch user id */
    "group", 1, group_option,		/* switch group id */
    "spawn", 1, shell_cmd,		/* spawn shell command */
    "twist", 1, twist_option,		/* replace current process */
#ifdef RFC931_OPTION
    "rfc931", 0, rfc931_option,		/* do RFC 931 lookup */
#endif
    0,
};

/* process_options - process optional access control information */

process_options(options, daemon, client)
char   *options;
char   *daemon;
struct from_host *client;
{
    char   *key;
    char   *value;
    struct option *op;

    /* Light-weight parser. We're not going to duplicate PERL. */

    for (key = strtok(options, ":"); key; key = strtok((char *) 0, ":")) {
	if (value = strchr(key, '=')) {		/* keyword=value */
	    *value++ = 0;
	    value = chop_string(value);		/* strip blanks around value */
	    if (*value == 0)
		value = 0;
	}
	key = chop_string(key);			/* strip blanks around key */
	for (op = option_table; op->name; op++)	/* find keyword */
	    if (strcasecmp(op->name, key) == 0)
		break;
	if (op->name == 0) {
	    syslog(LOG_ERR, "bad option or syntax: \"%s\"", key);
	} else if (value == 0 && op->need_value) {
	    syslog(LOG_ERR, "option \"%s\" requires value", key);
	} else if (value && op->need_value == 0) {
	    syslog(LOG_ERR, "option \"%s\" requires no value", key);
	} else {
	    (*(op->func)) (value, daemon, client);
	}
    }
}

/* user_option - switch user id */

/* ARGSUSED */

static void user_option(value, daemon, client)
char   *value;
char   *daemon;
struct from_host *client;
{
    struct passwd *pwd;

    if ((pwd = getpwnam(value)) == 0) {
	syslog(LOG_ERR, "unknown user: \"%s\"", value);
	clean_exit(client);
    } else if (setuid(pwd->pw_uid)) {
	syslog(LOG_ERR, "setuid(%s): %m", value);
	clean_exit(client);
    }
}

/* group_option - switch group id */

/* ARGSUSED */

static void group_option(value, daemon, client)
char   *value;
char   *daemon;
struct from_host *client;
{
    struct passwd *pwd;

    if ((pwd = getpwnam(value)) == 0) {
	syslog(LOG_ERR, "unknown group: \"%s\"", value);
	clean_exit(client);
    } else if (setgid(pwd->pw_gid)) {
	syslog(LOG_ERR, "setgid(%s): %m", value);
	clean_exit(client);
    }
}

/* twist_option - replace process by shell command */

static void twist_option(value, daemon, client)
char   *value;
char   *daemon;
struct from_host *client;
{
    char    buf[BUFSIZ];
    int     pid = getpid();

    percent_x(buf, sizeof(buf), value, daemon, client, pid);
    (void) execl("/bin/sh", "sh", "-c", buf, (char *) 0);
    syslog(LOG_ERR, "twist \"%s\": %m", buf);
    clean_exit(client);
}

#ifdef RFC931_OPTION

/* rfc931_option - look up remote user name */

/* ARGSUSED */

static void rfc931_option(value, daemon, client)
char   *value;
char   *daemon;
struct from_host *client;
{
    if (client->sin != 0 && client->sock_type == FROM_CONNECTED)
	client->user = rfc931_name(client->sin);
}

#endif

/* chop_string - strip leading and trailing blanks from string */

static char *chop_string(start)
register char *start;
{
    register char *end;

    while (*start && isspace(*start))
	start++;

    end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
	*end-- = 0;

    return (start);
}
