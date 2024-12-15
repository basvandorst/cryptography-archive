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
 * @(#) pwdchk.c: Check the passwd file		(Rel. 1.0)
 *
 * Usage	: 	pwdchk [-u uid] [-m(update)] [-p poll_time]
 *
 * Alghoritms	:
 *	Test if all uid < UID entry in passwd are legal
 *
 * Customizations :
 * 	prv_uid		Minimal UID to test (check <)
 *
 * By Marco & Franco	Milano	4/1988
 *
 * @(#) pwdchk.c: Check the passwd file		(Rel. 2.0)
 * By Marco 		Milano	11/1991
 *	Support of shared passwrd and audit id.
 *
 */
/*
 * $Author: marco $
 * $Id: pwdchk.c,v 2.5 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 2.5 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: pwdchk.c,v 2.5 1991/11/26 12:23:29 marco Exp marco $";
#endif	lint

#include <stdio.h>
#include <errno.h>
#include <sgtty.h>
#include <signal.h>
#include <pwd.h>
#include "config.h"

#ifdef	FAST_PWENT
#include "fastpwent.h"

#define	setpwent()	FastSetPwEnt(GET_FROM_BOTH | PANIC_ON_BAD_ORDER)
#define	getpwent()	FastGetPwEnt()
#define	endpwent()	FastEndPwEnt()
#endif	FAST_PWENT

extern struct passwd	*getpwent();

			/*
			 * Mail At System Manager
			 */
#define	MAILLOG		"SuLog-Mail"

int	poll		= PWDCHK_POLL;

extern	FILE	*fopen();

extern int	optind;
extern int	opterr;
extern char	*optarg;

int	prv_uid	= MINIMAL_UID;

#ifdef	DEBUG
#define	a_msg	A_msg
#define	a_send	A_send
#endif

main(argc, argv)
int	argc;
char	**argv;
{
	int	c;

	opterr = 0;
	while ((c = getopt(argc, argv, "u:mp:")) != EOF)
		switch (c) {
			case 'u' :
				prv_uid	= atoi(optarg);
				break;

			case 'm' :
				update(0, UPDF);
				exit(0);

			case 'p' :
				if ((poll = atoi(optarg)) <= 0)
					Usage(argv[0], "Illegal Poll Time");
				break;

			default	 :
				Usage(argv[0], "Illegal Option");
		}


#if	defined(NOTERM) && !defined(DEBUG)
	noterm();
#endif

	for (;;) {
#ifdef	DEBUG
		printf("Start\n");
#endif
		tryWith(UPDF);
#ifdef	DEBUG
		printf("Sleep %d\n", poll);
#endif

		sleep(poll);
	}

	/*NOTREACHED*/
	return	0;
}

tryWith(ufile)
char	*ufile;
{
	FILE	*u;

	if ((u = fopen(ufile, "r")) == NULL) {
		if (errno != ENOENT)
			serror(ufile);
		update(1, ufile);
		if ((u = fopen(ufile, "r")) == NULL)
			serror(ufile);
	}

	pwdcheck(u, ufile);

	fclose(u);
}

serror(file)
char	*file;
{
	a_msg("Pwdchk I/O error on file %s, errno %d\n", file, errno);
	a_send(MAILLOG, "Pwdchk error");
	exit(1);
}

static int	pos	= -1;
#ifdef	DEBUG
static int	nuser, 
		nopen;
#endif

pwdcheck(u, ufile)
FILE	*u;
char	*ufile;
{
	struct passwd	*pwd;
	int		bad	= 0;

	pos	= -1;
#ifdef	DEBUG
	nuser	= 0;
	nopen	= 0;
#endif
	(void) setpwent();
	while (pwd = getpwent()) {
#ifdef	DEBUG
		if ((++nuser % 100) == 0) {
			printf("%d\r", nuser);
			fflush(stdout);
		}
#endif	DEBUG
		if (pwd->pw_uid < prv_uid)
			bad	+= e_check(u, pwd->pw_name, pwd->pw_passwd, 
					      pwd->pw_uid, pwd->pw_gid);
	}
	(void) endpwent();
#ifdef	DEBUG
	printf("User %d; Open %d\n", nuser, nopen);
#endif

	if (bad) {
		a_msg("Update Db\n");
		a_send(MAILLOG, "Suspect-Password-Entry");
		update(1, ufile);
	}
}

int	e_check(u, user, pass, uid, gid)
FILE	*u;
char	*user, *pass;
int	uid, gid;
{
	char		line[256], U[80], P[80];
	int		uId, gId;
	register int	i;
	void		get_item();

	i	= pos;
	do {
		i++;
		if (fgets(line, sizeof(line), u) == NULL) {
			rewind(u);
			i	= 0;
#ifdef	DEBUG
			nopen++;
#endif
			if (fgets(line, sizeof(line), u) == NULL)
				serror("passwd");
		}

		get_item(line, U, P, &uId, &gId);

		if (strcmp(user, U) != 0)
			continue;

		pos	= i;
		if (strcmp(pass, P)) {
			a_msg("Password of %s has changed\n", user);
			return 1;
		}

		if (uid != uId) {
			a_msg("Uid(%d) of %s has changed to %d\n", 
							uid, user, uId);
			return 1;
		}

		if (gid != gId) {
			a_msg("Gid(%d) of %s has changed to %d\n",
							gid, user, gId);
			return 1;
		}

		return 0;
	} while (i != pos);

	a_msg("New Passwd Entry: User = %s, Uid = %d\n", user, uid);

	pos	= i;

	return 1;
}

update(remote, ufile)
int	remote;
char	*ufile;
{
	FILE		*u;
	struct passwd	*pwd;

#ifdef	DEBUG
	printf("Update %s\n", ufile);
#endif
	if ((u = fopen(ufile, "w")) == NULL) {
		if (remote)
			serror(ufile);

		perror(ufile);
		exit(1);
	}
	(void) setpwent();
	while (pwd = getpwent()) {
		if (pwd->pw_uid > prv_uid)
			continue;

		fprintf(u, "%s:%s:%d:%d\n", pwd->pw_name, pwd->pw_passwd, 
						pwd->pw_uid, pwd->pw_gid);
	}
	(void) endpwent();

	fclose(u);
#ifdef	DEBUG
	printf("End Update %s\n", ufile);
#endif
}

Usage(tool, err)
char	*tool;
char	*err;
{
	fprintf(stderr, "%s	: %s\n", tool, err);
	fprintf(stderr, "Usage	: %s [-u uid] [-m] [-p poll_time]\n", tool);
	exit(1);
}

#ifdef	DEBUG
A_msg(a, b, c, d, e, f, g, h)
{
	printf(a, b, c, d, e, f, g, h);
}

A_send()
{
}
#endif

void	get_item(line, user, pass, uid, gid)
char	*line, *user, *pass;
int	*uid, *gid;
{
	register char	*str, *cp;
	char		buffer[256];
	extern char	*strcpy();

	/*
	 * Works on a local buffer to avoid side effects.
	 */
	str = strcpy(buffer, line);

	while (*str != ':')
		*user++ = *str++;
	*user = '\0';

	str++;
	while (*str != ':')
		*pass++ = *str++;
	*pass = '\0';

	cp = ++str;
	while (*cp != ':')
		cp++;
	*cp = '\0';
	*uid = atoi(str);

	str = ++cp;
	while (*cp != ':' && *cp != '\n')
		cp++;
	*cp = '\0';
	*gid = atoi(str);
}
