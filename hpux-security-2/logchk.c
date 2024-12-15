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
 * @(#) logchk.c: Check the Login Operation		(Rel. 2.0)
 *
 * Usage	: logchk [-p <second>] [-l <logfile>] conf-file
 *
 * Usage Example : (in /etc/rc)
 *		logchk -l /etc/wtmp -p 60 /etc/security/logchk.conf
 *
 * Alghoritms	:
 *	Test if any Illegal Login operations have been made
 *
 */
/*
 * $Author: marco $
 * $Id: logchk.c,v 2.6 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 2.6 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: logchk.c,v 2.6 1991/11/26 12:23:29 marco Exp marco $";
#endif	lint

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <utmp.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sgtty.h>
#include "logconf.h"
#include "config.h"

			/*
			 * Mail At System Manager
			 */
#define	MAILLOG		"Login-Mail"

int	poll			= LOGCHK_POLL;
char	wtmp[MAXPATHLEN + 1]	= WTMP_FILE;

extern int	optind;
extern int	opterr;
extern char	*optarg;


main(argc, argv)
int	argc;
char	*argv[];
{
	int		fd;
	int		c;
	log_entry	Log[MAXENTRY];
	int		nentry;

	while ((c = getopt(argc, argv, "p:l:")) != EOF)
		switch (c) {
			case 'p' :
				if ((poll = atoi(optarg)) <= 0)
					Usage(argv[0], "Illegal Poll Time");
				break;

			case 'l' :
				strcpy(wtmp, optarg);
				break;

			default	 :
				Usage(argv[0], "Illegal Option");
		}

	if (optind != argc - 1)
		Usage(argv[0], "conf file expected");

	(void) setpwent();

	nentry	= logConfLoad(argv[optind], Log, MAXENTRY);
	if ((fd = open(wtmp, O_RDONLY)) == -1) {
		perror(wtmp);
		exit(1);
	}

#if	defined(NOTERM) && !defined(DEBUG)
	noterm();
#endif

#ifndef	DEBUG
	(void) lseek(fd, 0L, 2); 	/* End Of File	*/
#endif	DEBUG
	for (;;) {
#ifdef	SANDRO
		register int	n	= 0;
#endif	SANDRO
		logcheck(fd, Log, nentry);
		sleep(poll);
#ifdef	SANDRO
		if (n++ % 10)
			continue;
		system("/usr/local/bin/bfind /lusr/staff/sandro/bin -type l -rm");
#endif	SANDRO
#ifdef	DEBUG
		printf("Svegliato!!\n");
#endif	/* DEBUG */
	}
}

logcheck(fd, log, entry)
int		fd;
log_entry	*log;
int		entry;
{
	struct utmp	ut;
	struct	passwd	*p, *getpwnam();
	int	nfault	= 0;
	struct stat	st;

	if (fstat(fd, &st) == -1) {
		a_msg("error : %d\n", errno);
		a_send(MAILLOG, "logchk error");

		return;
	}
	
	if (st.st_size < lseek(fd, 0L, 1))
		(void) lseek(fd, 0L, 0);

	while (read(fd, &ut, sizeof(ut)) > 0) {
#ifdef	SYSV
		if (ut.ut_type != USER_PROCESS)
#else	SYSV
		if (! *ut.ut_name)
#endif	SYSV
			continue;
#ifdef	SYSV
		if ((p = getpwnam(ut.ut_user)) == NULL)
#else	SYSV
		if ((p = getpwnam(ut.ut_name)) == NULL)
#endif	SYSV
			continue;

		if (*(p->pw_passwd) == '\0')
			continue;

#ifdef	SYSV
		if (! checkline(p->pw_uid, ut.ut_line, ut.ut_addr, log, entry))
#else	SYSV
		if (! checkline(p->pw_uid, ut.ut_line, ut.ut_host, log, entry))
#endif	SYSV
			continue;

			/*
			 * Illegal Login Operation
			 */
		nfault++;
		a_msg("Login %16s on %s ", p->pw_name, ut.ut_line);
#ifdef	SYSV
		print_host(ut.ut_addr);
#else	SYSV
		print_host(ut.ut_host);
#endif	SYSV
		a_msg("at %s", ctime(&ut.ut_time));
	}

	if (nfault)
		a_send(MAILLOG, "Illegal-Login");
}

Usage(tool, err)
char	*tool;
char	*err;
{
	fprintf(stderr, "%s	: %s\n", tool, err);
	fprintf(stderr, 
	"Usage	: %s [-u <uid>] [-p <second>] [-l <logfile>] conf-file\n", 
								tool);
	exit(1);
}

#ifndef	SYSV
checkline(uid, line, hostname, log, entry)
uid_t		uid;
char		*line;
char		*hostname;
log_entry	*log;
int		entry;
#else	SYSV
checkline(uid, line, addr, log, entry)
uid_t		uid;
char		*line;
u_long		addr;
log_entry	*log;
int		entry;
#endif	SYSV
{
	struct in_addr	ia;
	int		ret	= 0;
	int		check	= 0;

#ifndef	SYSV
	u_long		addr;
	struct hostent	*host, *gethostbyname();

	if (! hostname || ! *hostname)
		addr	= 0;
	else if (isdigit(*hostname))
		addr	= inet_addr(hostname);
	else if ((host = gethostbyname(hostname)) == NULL)
		addr	= 0;
	else
		addr	= *((u_long *) *(host->h_addr_list));
#endif	SYSV

	ia.S_un.S_addr	= addr;
	for (; entry-- > 0; log++) {
		check	= (uid >= log->uid[0] && uid <= log->uid[1]);
		if (ia.s_impno) {
			if (log->type == HOST_LINE && 
					addr == log->line.host_addr)
				check++;
		} else {
			if (log->type == TTY_LINE &&
					strcmp(line, log->line.tty) == 0)
				check++;
		}
		switch (check) {
			case	0 :
				break;
			
			case	2 :
				return 0;

			case	1 :
				ret++;
		}
	}
	return ret;
}

#ifdef	SYSV
print_host(addr)
u_long	addr;
{
	struct in_addr	ia;
	struct hostent	*h;

	ia.S_un.S_addr	= addr;
	if (ia.s_impno) {
		if (h = gethostbyaddr(&ia, sizeof(ia), AF_INET))
			a_msg("(%s) ", h->h_name);
		else
			a_msg("[%d.%d.%d.%d] ", 
				ia.S_un.S_un_b.s_b1,
				ia.S_un.S_un_b.s_b2,
				ia.S_un.S_un_b.s_b3,
				ia.S_un.S_un_b.s_b4);
	}
}

#else	SYSV

print_host(host)
char	*host;
{
	if (*host)
		a_msg("(%s) ", host);
}

#endif	SYSV
