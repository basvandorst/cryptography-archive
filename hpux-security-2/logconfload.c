/*
 * Copyright (c) 1990 Marco Negri & siLAB Staff
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission of the authors.
 * The authors makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Please send any improvements, bug fixes, useful modifications, and comments
 * to security@ghost.unimi.it
 */
/*
 * @(#) logconfload.h: Check the Login Operation		(Rel. 2.0)
 *
 * By Marco	Milano 12/90
 */
/*
 * $Author: marco $
 * $Id: logconfload.c,v 2.3 1991/10/21 16:44:09 marco Exp marco $
 * $Date: 1991/10/21 16:44:09 $
 * $Revision: 2.3 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: logconfload.c,v 2.3 1991/10/21 16:44:09 marco Exp marco $";
#endif	lint

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include "logconf.h"
#include "config.h"

char	*getword(buf)
char	**buf;
{
	char		*s;
	register char	*p;

	for (p = *buf; *p == ' ' || *p == '\t'; p++);
	if (! *p || *p == '\n')
		return NULL;
	s	= p;
	for (; *p && *p != '\n' && *p != ' ' && *p != '\t'; p++);
	*p	= '\0';
	*buf	= p + 1;

	return	s;
}

get_uid(s, uid)
char	*s;
uid_t	uid[2];
{
	struct passwd	*pwd, *getpwnam();
#ifdef	DEBUG
	static init	= 0;

	if (init++ == 0)
		(void) setpwent();
#endif	DEBUG

	if (isdigit(*s)) {
		uid[0]	= atoi(s);
		while (isdigit(*s)) s++;
		if (*s++ == '-')
			uid[1]	= atoi(s);
		else 
			uid[1]	= uid[0];
		if (uid[0] > uid[1]) {
			fprintf(stderr, "Illegal uid range	%d-%d\n", 
							uid[0], uid[1]);
			return 0;
		}
	} else {
		if ((pwd = getpwnam(s)) == NULL) {
			fprintf(stderr, "user unknown	%s\n", s);
			return 0;
		}

		uid[0]	= uid[1]	= pwd->pw_uid;
	}

	return	1;
}

logConfLoad(file, log, entry)
char		*file;
log_entry	*log;
int		entry;
{
	FILE		*fp;
	char		buf[BUFSIZ],
			line_save[BUFSIZ];
	struct hostent	*host, *gethostbyname();
	char		*p;
	register char	*s, *line;
	u_long		host_addr;
	int		type;
	uid_t		uid[2];
	int		nentry	= 0;


	if ((fp = fopen(file, "r")) == NULL) {
		perror(file);
		exit(1);
	}

	while ((p = fgets(buf, BUFSIZ, fp)) != NULL) {
		strcpy(line_save, buf);
		if ((s = getword(&p)) == NULL || *s == '#')
			continue;

		if (*s == '@') {
			if (! *++s && (s = getword(&p)) == NULL)
				goto syntax_error;
			
			type	= HOST_LINE;
			if ((host = gethostbyname(s)) == NULL) {
				perror(s);
				continue;
			}

			host_addr = ((struct in_addr *) host->h_addr)->s_addr;
		} else {
			type	= TTY_LINE;
			line	= s;
		}
		if ((s = getword(&p)) == NULL || *s != ':')
			goto syntax_error;

		if (! *++s && (s = getword(&p)) == NULL)
			goto syntax_error;

		do {
			if (! get_uid(s, uid))
				continue;
#ifdef	DEBUG
			printf("%d-%d	: %s\n", uid[0], uid[1], 
				(type == TTY_LINE) ? line : host->h_name);
#endif	DEBUG
			log->type	= type;
			if (type == TTY_LINE)
				strncpy(log->line.tty, line, LINESZ);
			else
				log->line.host_addr	= host_addr;
			log->uid[0]	= uid[0];
			log->uid[1]	= uid[1];
			log++;
			nentry++;
			entry--;
		} while (s = getword(&p));
		continue;

syntax_error:
		fprintf(stderr, "Syntax error	: %s\n", line_save);
	}

	return nentry;
}

/*
main()
{
	log_entry	log[MAXENTRY];

	logConfLoad("logconf", &log, MAXENTRY);
}
*/
