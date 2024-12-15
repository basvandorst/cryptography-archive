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
 * @(#) suchk.c	: Check the su log file		(Rel. 1.1)
 *
 * Usage	: suchk [-u <uid>] [-p <second>] [-l <logfile>]
 *
 * Usage Example : (in /etc/rc)
 *		suchk -l /usr/adm/sulog -p 60
 *
 * Alghoritms	:
 *	Test if any Illegal SU operations have been made
 *
 * By Marco	Milano 4/1988 - 12/1990
 */
/*
 * $Author: marco $
 * $Id: suchk.c,v 2.5 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 2.5 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: suchk.c,v 2.5 1991/11/26 12:23:29 marco Exp marco $";
#endif	lint

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <sgtty.h>
#include "config.h"

			/*
			 * Format of /usr/adm/sulog
			 */
#define	SUFMT	"%*s %[0-9/] %[0-9:] %c %[a-z0-9] %[a-zA-Z0-9]-%[a-zA-Z0-9]"

			/*
			 * Mail At System Manager
			 */
#define	MAILLOG		"SuLog-Mail"

int	uid			= MINIMAL_UID;
int	poll			= SUCHK_POLL;
char	sulog[MAXPATHLEN + 1]	= SULOG;

extern	FILE	*fopen();

extern int	optind;
extern int	opterr;
extern char	*optarg;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE	*fsu;
	int	c;

	while ((c = getopt(argc, argv, "p:u:l:")) != EOF)
		switch (c) {
			case 'p' :
				if ((poll = atoi(optarg)) <= 0)
					Usage(argv[0], "Illegal Poll Time");
				break;

			case 'u' :
				if ((uid = atoi(optarg)) <= 0)
					Usage(argv[0], "Illegal UID");
				break;

			case 'l' :
				strcpy(sulog, optarg);
				break;

			default	 :
				Usage(argv[0], "Illegal Option");
		}

	if ((fsu = fopen(sulog, "r")) == NULL) {
		perror(sulog);
		exit(1);
	}

#if	defined(NOTERM) && !defined(DEBUG)
	noterm();
#endif

	(void) setpwent();

	(void) fseek(fsu, 0L, 2);	/* End Of File	*/
	for (;;) {
		sucheck(fsu);
		sleep(poll);
	}
}

sucheck(fsu)
FILE	*fsu;
{
	char	res,
		tty[100],	from[100],
		to[100],	day[100],
		hour[100],	buf[BUFSIZ];
	struct	passwd	src, dst, *p,
			*getpwnam();
	int	nfault	= 0;
	struct stat	st;
	long		lseek();

	if (fstat(fileno(fsu), &st) == -1) {
		a_msg("error : %d\n", errno);
		a_send(MAILLOG, "suchk error");

		return;
	}

	if (st.st_size < lseek(fileno(fsu), 0L, 1))
		fseek(fsu, 0L, 0);

	while (fgets(buf, BUFSIZ, fsu) != NULL) {
		sscanf(buf, SUFMT, day, hour, &res, tty, from, to);
		if (res == '-')
			continue;
		/*
		 * SU Has been made
		 */
		if ((p = getpwnam(from)) == NULL)
			continue;
		src	= *p;

		if ((p = getpwnam(to)) == NULL)
			continue;
		dst	= *p;

		if (src.pw_uid > dst.pw_uid && dst.pw_uid < uid &&
						    src.pw_uid >= uid) {
			/*
			 * Illegal Su Operation
			 */
			nfault++;
			a_msg("%s %s, at %8s : From %14s su to %14s\n",
				day, hour, tty, from, to);
		}
	}

	if (nfault)
		a_send(MAILLOG, "Illegal-SU");
}

Usage(tool, err)
char	*tool;
char	*err;
{
	fprintf(stderr, "%s	: %s\n", tool, err);
	fprintf(stderr, "Usage	: %s [-u <uid>] [-p <second>] [-l <logfile>]\n",
							tool);
	exit(1);
}
