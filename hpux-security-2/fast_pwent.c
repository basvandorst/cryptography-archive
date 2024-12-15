/*
 * $Author: marco $
 * $Id: fast_pwent.c,v 1.1 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 1.1 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: fast_pwent.c,v 1.1 1991/11/26 12:23:29 marco Exp marco $";
#endif	lint

#include <stdio.h>
#include <pwd.h>
#include <errno.h>
#include "fastpwent.h"

#ifdef	DEBUG
#define	PASSWD	"./passwd"
#define	SPASSWD	"./spasswd"
#else	DEBUG
#define	PASSWD	"/etc/passwd"
#define	SPASSWD	"/.secure/etc/passwd"
#endif	DEBUG

#define	EQSTR(x, y)	(strcmp(x, y) == 0)

FILE	*pfp	= NULL;
FILE	*spfp	= NULL;

static int	panicMode	= 0;
static char	*GetPassword();
static char	*Strtok();

#define	strtok	Strtok

FastSetPwEnt(mode)
int	mode;
{
	if (pfp)
		fclose(pfp);

	if (spfp)
		fclose(spfp);

	pfp	= spfp	= NULL;

	panicMode	= mode & PANIC_ON_BAD_ORDER;

	if (mode & GET_FROM_PASSWD && (pfp = fopen(PASSWD, "r")) == NULL)
		return -1;

	if (mode & GET_FROM_SHARED)
		spfp	= fopen(SPASSWD, "r");

	return 0;
}

FastEndPwEnt()
{
	if (pfp)
		fclose(pfp);

	if (spfp)
		fclose(spfp);

	pfp	= spfp	= NULL;
}

struct passwd	*FastGetPwEnt()
{
	static char		line[1024];
	static char		sline[1024];
	static struct passwd	pwd;
	char			*name;

	if (pfp) {
		if (fgets(line, BUFSIZ, pfp) == NULL)
			return NULL;

		pwd.pw_name	= strtok(line, ":");
#ifdef	SYSV
		pwd.pw_passwd	= GetPassword(strtok(NULL, ":"), &pwd.pw_age);
#else	SYSV
		pwd.pw_passwd	= strtok(NULL, ":");
#endif	SYSV
		pwd.pw_uid	= atoi(strtok(NULL, ":"));
		pwd.pw_gid	= atoi(strtok(NULL, ":"));
		pwd.pw_comment	= NULL;
		pwd.pw_gecos	= strtok(NULL, ":");
		pwd.pw_dir	= strtok(NULL, ":\n");
		pwd.pw_shell	= strtok(NULL, ":\n");

		if (! spfp)
			return &pwd;

		name = fgets(sline, BUFSIZ, spfp);
		if (!name || !EQSTR(name = strtok(sline, ":"), pwd.pw_name)) {
			errno	= ENOENT;
			if (panicMode) {
				perror(name);
				exit(1);
			}
			return NULL;
		}
#ifdef	SYSV
		pwd.pw_passwd	= GetPassword(strtok(NULL, ":"), &pwd.pw_age);
#else	SYSV
		pwd.pw_passwd	= strtok(NULL, ":");
#endif	SYSV

#ifdef	AUDIT
		pwd.pw_audid	= atoi(strtok(NULL, ":"));
		pwd.pw_audflg	= atoi(strtok(NULL, "\n"));
#endif	AUDIT

		return &pwd;
	} else if (spfp) {
		name = fgets(line, BUFSIZ, spfp);

		pwd.pw_name	= strtok(line, ":");
#ifdef	SYSV
		pwd.pw_passwd	= GetPassword(strtok(NULL, ":"), &pwd.pw_age);
#else	SYSV
		pwd.pw_passwd	= strtok(NULL, ":");
#endif	SYSV
#ifdef	AUDIT
		pwd.pw_audid	= atoi(strtok(NULL, ":"));
		pwd.pw_audflg	= atoi(strtok(NULL, "\n"));
#endif	AUDIT

		pwd.pw_uid	= -1;
		pwd.pw_gid	= -1;
		pwd.pw_comment	= NULL;
		pwd.pw_gecos	= NULL;
		pwd.pw_dir	= NULL;
		pwd.pw_shell	= NULL;

		return &pwd;

	} else
		return NULL;
}

#ifdef	SYSV
static char	*GetPassword(pwd, age)
char		*pwd;
char		**age;
{
	static char	stAge[4];

	*age	= NULL;
	if (!pwd || ! *pwd)
		return NULL;

	if (*pwd != ',')
		return pwd;

	strncpy(stAge, pwd, 3);
	*age	= stAge;

	return pwd + 3;
}
#endif	SYSV

static char	*Strtok(buf, sep)
char		*buf;
char		*sep;
{
	static char	*start;
	register char	*save;
	extern char	*strchr();

	if (buf)
		start	= buf;

	save	= start;
	while (*start && strchr(sep, *start) == NULL)
		start++;

	if (*start)
		*(start++)	= '\0';	

	return save;
}
