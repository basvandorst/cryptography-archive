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
 * @(#) getfinger.c	: Check the su log file		(Rel. 1.1)
 *
 * Usage	: getfinger [-p <second>] [-l <logfile>] <Not traced process>
 *
 * Usage Example : (in /etc/rc)
 *		getfinger -l /usr/adm/inetd.log -p 60
 *
 * Alghoritms	:
 *	Send a remote finger request for every interesting connection.
 *
 * By Marco	Milano 10/1991
 */
/*
 * $Author: marco $
 * $Id: getfinger.c,v 1.2 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 1.2 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: getfinger.c,v 1.2 1991/11/26 12:23:29 marco Exp marco $";
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
#define	INETDFMT	"Connect to %[^ ] from %[^ ] (%[^ )]) %[^\n]"

			/*
			 * Mail At System Manager
			 */
int	poll			= SUCHK_POLL;
char	*inetdLog		= INETDLOG;

#define	MAXNOTTRACED	256

char	*notTraced[MAXNOTTRACED]	= {
	"fingerd",
	NULL
};

extern	FILE	*fopen();

extern int	optind;
extern int	opterr;
extern char	*optarg;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE		*infp;
	register char	**p;
	int		c;

	while ((c = getopt(argc, argv, "p:l:")) != EOF)
		switch (c) {
			case 'p' :
				if ((poll = atoi(optarg)) <= 0)
					Usage(argv[0], "Illegal Poll Time");
				break;

			case 'l' :
				inetdLog	= optarg;
				break;

			default	 :
				Usage(argv[0], "Illegal Option");
		}

	if ((argc - optind) >= MAXNOTTRACED - 1)
		Usage(argv[0], "Too many arguments");

	for (p = &notTraced[1]; optind < argc; p++, optind++)
		*p	= argv[optind];
	*p	= NULL;

	if ((infp = fopen(inetdLog, "r")) == NULL) {
		perror(inetdLog);
		exit(1);
	}

#if	defined(NOTERM) && !defined(DEBUG)
	noterm();
#endif

	(void) fseek(infp, 0L, 2);	/* End Of File	*/
	for (;;) {
		getFinger(infp);
		sleep(poll);
	}
}

getFinger(infp)
FILE	*infp;
{
	char	service[100],	host[100], address[100], 	date[100];
	char	buf[BUFSIZ];
	char	*name;
	struct stat	st;
	long		lseek();

	if (fstat(fileno(infp), &st) == -1) {
		a_msg("error : %d\n", errno);
		a_send("GetFinger", "getFinger error");

		return;
	}

	if (st.st_size < lseek(fileno(infp), 0L, 1))
		fseek(infp, 0L, 0);

	while (fgets(buf, BUFSIZ, infp) != NULL) {
		sscanf(buf, INETDFMT, service, host, address, date);
		if (monitoring(strrchr(service, '/') + 1) == 0)
			continue;
		name	= (strcmp(host, "unknown") == 0) ? address : host;
		printf("\n%20s - %20s : %s\n", date, name, service);
		fflush(stdout);
		sprintf(buf, "/usr/local/bin/finger @%s\n", host);
		system(buf);
	}
	fflush(stdout);
	fflush(stderr);
}

Usage(tool, err)
char	*tool;
char	*err;
{
	fprintf(stderr, "%s	: %s\n", tool, err);
	fprintf(stderr, "Usage	: %s [-p <second>] [-l <logfile>]\n", tool);
	exit(1);
}

monitoring(service)
char	*service;
{
	register char	**p;

	for (p = notTraced; *p && strcmp(*p, service) != 0; p++);

	return (*p == NULL);
}
