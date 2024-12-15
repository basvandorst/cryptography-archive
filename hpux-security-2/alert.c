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
 * @(#)alert.c	: Send "Alert Messages" at Systems Managers (Rel. 2.0)
 * @(#)alert.c	: By Marco & Carlo	Milano, 10/89, 12/90
 */
/*
 * $Author: marco $
 * $Id: alert.c,v 2.3 1991/10/21 16:44:09 marco Exp marco $
 * $Date: 1991/10/21 16:44:09 $
 * $Revision: 2.3 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: alert.c,v 2.3 1991/10/21 16:44:09 marco Exp marco $";
#endif	lint

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <utmp.h>
#include <stdio.h>
#include <fcntl.h>
#include "alertconfig.h"
#include "config.h"

#define	TMPFILE		"/tmp/alert.XXXXXX"

static char	tmpname[64];
static int	init	= 0;
static int	present	= 0;
static FILE	*tmp;
static char	*people[MAXPEOPLE];
static int	local[MAXPEOPLE];
static int	npeople = -1;
static void	(*(old[NSIG]))();

static void	a_init();
static int	send_all();
static void	load_conf();
static char	*mk_str();
static void	send_mail();
static void	send_msg();
static void	term();
static void	f_copy();

extern char	*strcpy();
extern char	*mktemp();

/*VARARGS1*/
int	a_msg(fmt, a, b, c, d, e, f, g, h)
char	*fmt;
{
	if (! init)
		a_init();

	fprintf(tmp, fmt, a, b, c, d, e, f, g, h);

	return present;
}

int	a_send(tools, subject)
char	*tools;
char	*subject;
{
	int		fd;
	char		buf[BUFSIZ];
	register int	i;
	int		e_status	= 0;

	if (init == 0)
		return	-1;

#ifdef	SYSV
	sprintf(buf, "LOGNAME=%s", tools);
	putenv(buf);
#endif	SYSV

	fclose(tmp);
	if ((fd = open(tmpname, O_RDONLY)) == -1) {
		perror(tmpname);
		term(-5);
	}

	e_status	= send_all(1, subject, fd);
	send_mail(subject, tools, people, npeople, fd);
		
	while (npeople > 0)
		free(people[--npeople]);
	close(fd);
	unlink(tmpname);
	for (i = 1; i < NSIG; i++)
		signal(i, old[i]);
	init	= 0;

	return e_status;
}

static void	a_init()
{
	register int	i;

	for (i = 1; i < NSIG; i++)
		old[i]	= signal(i, (i == SIGCLD) ? SIG_DFL : term);

	strcpy(tmpname, TMPFILE);
	if (mktemp(tmpname) == NULL) {
		perror(tmpname);
		term(-1);
	}

	if ((tmp = fopen(tmpname, "w")) == NULL) {
		perror(tmpname);
		term(-2);
	}

	load_conf();

	present	= send_all(0);
	init	= 1;
}

static void	term(sig)
int		sig;
{
	(void) unlink(tmpname);
	exit(sig + NSIG);
}

static void	load_conf()
{
	FILE		*fp;
	char		buf[BUFSIZ];
	char		*mk_str();
	register char	*p;
	register char	*s;

	if ((fp = fopen(CONFFILE, "r")) == NULL) {
		perror(CONFFILE);
		term(-3);
	}

	npeople = 0;
	while (fgets(buf, BUFSIZ, fp) != NULL) {
		for (p = buf; *p == '\n' || *p == ' ' || *p == '\t'; p++);
		if (! *p || *p == '#')
			continue;
		for (s = p++; *p != '\n' && *p != ' ' && *p != '\t'; p++);
		*p	= '\0';
		people[npeople] = mk_str(s);
		for (p = s; *p && *p != '@' && *p != '!' && *p != '%'; p++);
		local[npeople++]	= ! *p;
#ifdef	DEBUG
		printf("Load	%s\n", s);
#endif	DEBUG
	}

	fclose(fp);
}
		
static char	*mk_str(str)
char		*str;
{
	char	*p;
	char	*malloc();

	if ((p = malloc(strlen(str) + 1)) == NULL) {
		perror("malloc()");
		term(-4);
	}

	return strcpy(p, str);
}

static void	send_mail(subject, tool, nlist, count, fd)
char		*subject;
char		*tool;
char		**nlist;
int		count;
int		fd;
{
	int		pd[2];
	register int	r;
	char		buf[BUFSIZ];

	signal(SIGCLD, SIG_IGN);
	if (pipe(pd) == -1)
		return;

	switch (vfork()) {
		case	-1 :
			return;

		case	0 : {
			char		*argv[512];
			char		**s, **d;
			register int	i;
#ifdef	SENDMAIL
			char	buf[BUFSIZ];

			sprintf(buf, "-f%s", tool);
			argv[1]	= buf;
			d	= &argv[2];
#else	SENDMAIL
			d	= &argv[1];
#endif	SENDMAIL
			argv[0]	= MAIL;
			for (i = count, s = nlist; i-- > 0; *d++ = *s++);
			*d	= NULL;

			close(0);
			dup(pd[0]);
			close(pd[0]);
			close(pd[1]);
			execv(PMAIL, argv);
			return;
		}

		default	: {
			char		hdr[BUFSIZ];
			register int	i;
			register char	**p;

			(void) write(pd[1], hdr, sprintf(hdr, 
				"Subject: %s\nFrom: %s\nTo:", subject, tool));
			for (p = nlist, i = count; i-- > 0; p++)
				write(pd[1], hdr, sprintf(hdr, " %s", *p));
			(void) write(pd[1], "\n\n", 2);

			(void) lseek(fd, 0L, 0);

			while ((r = read(fd, buf, BUFSIZ)) > 0)
				write(pd[1], buf, r);
			(void) write(pd[1], "\n.\n", 3);
			(void) close(pd[0]);
			(void) close(pd[1]);
		}
	}
}

static void	send_msg(subject, tty, fd)
char		*subject;
char		*tty;
int		fd;
{
	struct stat	st;
	FILE		*fp;

	if (stat(tty, &st) == -1 || (st.st_mode & S_IFMT) != S_IFCHR)
		return;

	if ((fp = fopen(tty, "w")) == NULL)
		return;

	fprintf(fp, "\nWarning	: Alert Message\n");
	fprintf(fp, "Subject	: %s\n", subject);

	f_copy(fd, fp);
	
	fprintf(fp, "\nEND\n");
	fclose(fp);
}

static void	f_copy(fd, fp)
int		fd;
FILE		*fp;
{
	char		buf[BUFSIZ];
	register int	r;

	while ((r = read(fd, buf, BUFSIZ)) > 0)
		fwrite(buf, r, 1, fp);
}	

static int		ufd = -1;
static struct utmp	ut[128];

/*VARARGS1*/
static	send_all(mode, subject, fd)
int	mode;
char	*subject;
int	fd;
{
	register struct utmp	*s, *e;
	static char		buf[BUFSIZ];
	register char		**p;
	register int		i;
	int			send	= 0;

	if (ufd == -1 && (ufd = open(UTMP_FILE, O_RDONLY)) == -1)
		return	-1;
	else
		(void) lseek(ufd, 0L, 0);

	while ((i = read(ufd, ut, sizeof(ut))) > 0) {
		for (s = ut, e = &ut[i / sizeof(*e)]; s < e; s++) {
#ifdef	SYSV
			if (s->ut_type != USER_PROCESS)
#else	SYSV
			if (! *s->ut_name)
#endif	SYSV
				continue;

			sprintf(buf, "/dev/%s", s->ut_line);
			for (i = 0, p = people; i < npeople; i++, p++) {
				if (!local[i])
					continue;
#ifdef	DEBUG
				printf("Search	%s\n", *p);
#endif	DEBUG
#ifdef	SYSV
				if (strcmp(s->ut_user, *p))
#else	SYSV
				if (strcmp(s->ut_name, *p))
#endif	SYSV
					continue;

				send++;
				if (mode) {
					(void) lseek(fd, 0L, 0);
					send_msg(subject, buf, fd);
				}
			}
		}
	}

	return send;
}
