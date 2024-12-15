/*
 * $Author: vince $
 * $Header: /users/vince/src/skey/RCS/skeyd.c,v 1.30 1996/04/26 11:43:45 vince Exp $
 * $Date: 1996/04/26 11:43:45 $
 * $Revision: 1.30 $
 * $Locker:  $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: skeyd.c,v 1.30 1996/04/26 11:43:45 vince Exp $";
#endif	lint

#ifndef SKEYD
main(argc, argv)
int	argc;
char	*argv[];
{
	printf("No-no! %s is a null program when compiler without -DSKEYD\n",
								argv[0]);
}
#else /* SKEYD */

#include <stdio.h>
#if defined(__ultrix) || defined(AIX)
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#ifdef FLOCK
#include <sys/file.h>
#endif
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "skey.h"
#include "des.h"
#include "md4.h"

static char	*skeyserver;
static int	skeyport;

#define LOCKFILE	"/etc/skeyd.lock"
#define NUMTRY		20

static char	*hname;

static int		fdlock = -1;

main(argc, argv)
int	argc;
char	*argv[];
{
	struct skeymessage	talk;
	char			*userline;
	int			i, fromlen;
	static void		timeout();

	char			*getskeyfield(), *peername();

#ifdef __ultrix
	openlog("skeyd", LOG_WARNING);
#else
	openlog("skeyd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
#endif

	if ((hname = peername(0)) == NULL) {
		perror("getpeername()");
		exit(1);
	}

	if (readskeydconf()) {
		syslog(LOG_INFO, "readskeydconf(): %m");
		exit(1);
	}

	(void)signal(SIGALRM, timeout);
	(void)alarm(SKEYDTIMEOUT);

	if (skey_read_data(0, &talk, sizeof(talk)) != sizeof(talk)) {
		syslog(LOG_INFO, "reading data from %s: %m", hname);
		exit(1);
	}
	if (talkdecode(&talk)) {
		syslog(LOG_INFO, "received corrupted data from %s", hname);
		bzero(&talk, sizeof(talk));
		talk.flag = SKEYDFAILED;
		talkencode(&talk);
		goto sendpoint;
	}
	alarm(0);

#ifdef DEBUG
	syslog(LOG_INFO, "received: `%s' from %s", talk.buf, hname);
#endif

	/* fix: decrypt and check checksum */

	switch (talk.flag) {
		case	GIVEMEINFO:
#ifdef DEBUG
			syslog(LOG_INFO, "GIVEMEINFO");
#endif
			/* get entry in keyfile */
			userline = getskeyfield(talk.buf);

			bzero(&talk, sizeof(talk));
			if (userline) {
				strncpy(talk.buf, userline, sizeof(talk.buf));
				syslog(LOG_INFO, "information request succeeded");
				talk.flag = SKEYDOK;
			}
			else {
				syslog(LOG_INFO, "information request failed");
				/* this query failed anyway, but we provide
				a sutably encrypted numll pointer as a
				response to the client */
				talk.flag = SKEYDOK;
			}
			break;

		case	UPDATEKEYS:
#ifdef SKEYINITINSECURE
		case	FORCEKEYUPDATE:
#endif
#ifdef DEBUG
			syslog(LOG_INFO, "UPDATEKEYS");
#endif
			if (lock(LOCKFILE))
				exit(1);
			/* update record */
			i = updatekeys(talk.buf, talk.flag);
			if (unlock())
				exit(1);
			bzero(&talk, sizeof(talk));
			talk.flag =  i ? SKEYDFAILED : SKEYDOK;
			syslog(LOG_INFO, "update %s", 
				i == SKEYDFAILED ? "rejected" : "succeeded");
			break;

		default:
			syslog(LOG_INFO, "panic (flag = %d), from %s",
							talk.flag, hname);
			exit(1);
	}

#ifdef DEBUG
	syslog(LOG_INFO, "sending: `%s'", talk.buf);
#endif

	talkencode(&talk);

sendpoint:
	(void)alarm(SKEYDTIMEOUT);
	if (skey_write_data(1, &talk, sizeof(talk)) != sizeof(talk)) {
		syslog(LOG_INFO, "writing data to %s: %m", hname);
		exit(1);
	}
	alarm(0);
	return(0);
}

char *
getskeyfield(username)
char	*username;
{
	static char	line[256];
	char		line2[256], *p;
	FILE		*fp;
	int		found = 0;

	syslog(LOG_INFO, "GIVEMEINFO requested for user %s from %s",
						username, hname);

	if ((fp = fopen(KEYFILE, "r")) == NULL) {
		syslog(LOG_INFO, "fopen(%s): %m", KEYFILE);
		return(NULL);
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		rip(line);
		if (line[0] == '#' || !strlen(line))
			continue;
		strncpy(line2, line, sizeof(line2));
		p = strtok(line2, SEPARATOR);
		if (strcmp(p, username) || strlen(username) != strlen(p))
			continue;
		found = 1;
		break;
	}

	(void)fclose(fp);

	return (found ? line : NULL);
}

updatekeys(newline, flag)
char	*newline;
int	flag;
{
	char		line[256], buf[256], *name, *p;
	extern int	errno;
	FILE		*in, *out;
	int		found = 0;
	long		omask;

	rip(newline);
	strncpy(buf, newline, sizeof(buf));
	name = strtok(buf, SEPARATOR);
	if (*name == '\0')
		return(1);
	
	syslog(LOG_INFO, "UPDATE requested for %s from %s",
						name, hname);
	
	/* check that /etc/skeykeys exists */
	if ((in = fopen(KEYFILE, "r")) == NULL) {
#ifdef SKEYINITINSECURE
		/* if is does not exist, so there, create it */
		if (errno != ENOENT) {
			syslog(LOG_INFO, "fopen(%s): %m", KEYFILE);
			return(1);
		}
		syslog(LOG_INFO, "%s: %m -- creating it", KEYFILE);
		if ((out = fopen(KEYFILE, "w")) == NULL) {
			syslog(LOG_INFO, "fopen(%s): %m", KEYFILE);
			(void)fclose(in);
			return(1);
		}
		if (fprintf(out, "%s\n", newline) < 0) {
			syslog(LOG_INFO, "fprintf(): %m");
			(void)fclose(out);
			return(1);
		}
		if (fclose(out) == -1) {
			syslog(LOG_INFO, "fclose(): %m");
			return(1);
		}
		return(0);
#else
		/* it does not exist, and we do not trust remote clients to
		perform a network update */
		return(1);
#endif
	}

	/* temporary file */
	(void)unlink(PTMPKEYFILE);
	omask = umask(077);
	if ((out = fopen(PTMPKEYFILE, "w")) == NULL) {
		syslog(LOG_INFO, "fopen(%s): %m", PTMPKEYFILE);
		(void)fclose(in);
		return(1);
	}
	(void)umask(omask);

	/* scan all /etc/skeykeys keys and check matching */
	errno = 0;
	while (fgets(line, sizeof(line), in) != NULL) {
		rip(line);
		p = line;
		/* a match occours */
		if (!strncmp(name, line, strlen(name)) &&
						strlen(line) >= strlen(name)) {
			if (found) {
				/* two usernames in the same password file! */
				syslog(LOG_INFO,
					"DUPLICATED SKEY RECORD for %s\n", name);
				(void)fclose(in);
				(void)fclose(out);
				return(1);
			}

			/* Perform some important sanity checks; in particular
			   check that the new key is really fff(old key).
			   The client is supposed to have done the check
			   already but we want to play it safe, just in case
			   of biased client performing replay attacks */
			if (
#ifdef SKEYINITINSECURE
			/* skip sanity check, so inits are viable. we
			   trust clients and the security of clients.
			 */
			    flag != FORCEKEYUPDATE &&
#endif
			    /* important sanity anti-spoofing check */
			    sanitychk(newline, line)) {
				(void)fclose(in);
				(void)fclose(out);
				return(1);
			}
			found = 1;
			p = newline;
		}
		if (fprintf(out, "%s\n", p) < 0) {
			syslog(LOG_INFO, "fprintf(): %m");
			(void)fclose(in);
			(void)fclose(out);
			return(1);
		}
	}
	
	if (errno) {
		syslog(LOG_INFO, "fgets(): %m");
		return(1);
	}

	/* this is a new user; we add it at the end of file */
	if (!found) {
		if (fprintf(out, "%s\n", newline) < 0) {
			syslog(LOG_INFO, "fprintf(): %m");
			(void)fclose(in);
			(void)fclose(out);
			return(1);
		}
	}

	if (fclose(in) || fclose(out)) {
		syslog(LOG_INFO, "fclose(): %m");
		return(1);
	}
	
	if (rename(PTMPKEYFILE, KEYFILE)) {
		syslog(LOG_INFO, "rename(): %m");
		return(1);
	}
	return(0);
}

lock(lockfile)
char	*lockfile;
{
	extern int	errno;
	int		count = 0;

	if (fdlock != -1) {
		syslog(LOG_INFO, "lock already set");
		return(1);
	}
	if ((fdlock = open(lockfile, O_CREAT | O_WRONLY, 0600)) == -1) {
		syslog(LOG_INFO, "%s: %m", lockfile);
		return(1);
	}

	for (count = 0; count < NUMTRY; count++) {
#ifdef FLOCK
		if (flock(fdlock, LOCK_EX) == -1)
#else
		if (lockf(fdlock, F_TLOCK, 0) == -1)
#endif
							{
			if (errno == EACCES)
				sleep(1);
			else {
#ifdef FLOCK
				perror("lockf(LOCK_EX)");
#else
				perror("lockf(F_TLOCK)");
#endif
				return(1);
			}
		}
		else
			break;
	}

	if (count == NUMTRY) {
		return(1);
	}
	return(0);
}

unlock()
{
	if (fdlock == -1) {
		syslog(LOG_INFO, "no lock is set");
		return(1);
	}
#ifdef FLOCK
	if (flock(fdlock, LOCK_UN) == -1) {
		perror("lock(LOCK_UN)");
		return(1);
	}
#else
	if (lockf(fdlock, F_ULOCK, 0) == -1) {
		perror("lock(F_ULOCK)");
		return(1);
	}
#endif
	(void)close(fdlock);
	fdlock = -1;
	return(0);
}

static void
timeout(s)
{
	(void)signal(s, timeout);
	syslog(LOG_INFO, "time out (%d seconds)", SKEYDTIMEOUT);
	exit(1);
}

sanitychk(l1, l2)
char	*l1, *l2;
{
	int	n1, n2;
	char	key1[8], key2[8], *p;
	char	*get4thtoken();


	/* gtet 2nd field (sequence number) */
	if ((n1 = get2ndtoken(l1)) == -2 ||
	    (n2 = get2ndtoken(l2)) == -2)
		return (1);
	
	if (n1 != n2 - 1) {
		syslog(LOG_INFO,
"REPLAY ATTACK (sequence mismatch) from %s (new: %d, old: %d)", hname, n1, n2);
		return(1);
	}

	/* get 4th field (key field) */
	/* first string */
	if ((p = get4thtoken(l1)) == NULL)
		return (1);
	if (atob8(key1, p) == -1) {
		syslog(LOG_INFO, "atob8(%s)", p);
		return (1);
	}
	/* second string */
	if ((p = get4thtoken(l2)) == NULL)
		return (1);
	if (atob8(key2, p) == -1) {
		syslog(LOG_INFO, "atob8(%s)", p);
		return (1);
	}

	fff(key1);

	if (memcmp(key1, key2, 8)) {
		syslog(LOG_INFO,
"REPLAY ATTACK (password mismatch) from %s (new: %s, old: %s)", hname, l1, l2);
		return(1);
	}
	return(0);
}

char *
get4thtoken(str)
char	*str;
{
	static char	*p, x[BUFLEN];

	strncpy(x, str, sizeof(x));

	if (strtok(x, SEPARATOR) == NULL ||
	    strtok(NULL, SEPARATOR) == NULL ||
	    strtok(NULL, SEPARATOR) == NULL ||
	    (p = strtok(NULL, SEPARATOR)) == NULL) {
		syslog(LOG_INFO, "received invalid data from %s", hname);
		return (NULL);
	}

	return(p);
}

get2ndtoken(str)
char	*str;
{
	char	x[BUFLEN], *p;

	strncpy(x, str, sizeof(x));

	if (strtok(x, SEPARATOR) == NULL ||
	    (p = strtok(NULL, SEPARATOR)) == NULL )  {
		syslog(LOG_INFO, "received invalid data from %s", hname);
		return (-2);
	}

	return(atoi(p));
}


char *
peername(fd)
int	fd;
{
	struct sockaddr_in	addr;
	struct hostent		*he;
	static char		name[128];
	int			addrlen = sizeof(addr);
	int			on = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
					(char *)&on, sizeof (on)) == -1) {
		syslog(LOG_INFO, "setsockopt (SO_KEEPALIVE): %m");
		return(NULL);
	}

	if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) == -1) {
		perror("getpeername()"); /* so frequent */
		syslog(LOG_INFO, "getpeername() failed");
		return(NULL);
	}
	if ((he = gethostbyaddr((char *)&addr.sin_addr,
				sizeof(struct in_addr), AF_INET)) == NULL)
		(void)strcpy(name, (char *) inet_ntoa(addr.sin_addr));
	else
		(void)strncpy(name, he->h_name, sizeof(name));
	return(name);
}

#endif /* SKEYD */
