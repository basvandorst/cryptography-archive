/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)su.c	5.5 (Berkeley) 1/18/87";
#endif not lint

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "skey.h"

#ifndef SYSV
# include <sgtty.h>
# define TTYSTRUCT sgttyb
# define stty(fd,buf) ioctl((fd),TIOCSETN,(buf))
# define gtty(fd,buf) ioctl((fd),TIOCGETP,(buf))
#else
#ifdef BSDI
# include <termios.h>
# define termio termios
#define TCSETA TIOCSETA
#define TCGETA TIOCGETA
#else
# include <termio.h>
#endif
# define TTYSTRUCT termio
# define stty(fd,buf) ioctl((fd),TCSETA,(buf))
# define gtty(fd,buf) ioctl((fd),TCGETA,(buf))
#endif

char	userbuf[16]	= "USER=";
char	homebuf[128]	= "HOME=";
char	shellbuf[128]	= "SHELL=";
char	pathbuf[128]	= "PATH=:/usr/ucb:/bin:/usr/bin";
char	*cleanenv[] = { userbuf, homebuf, shellbuf, pathbuf, 0, 0 };
char	*user = "root";
char	*shell = "/bin/sh";
int	fulllogin;
int	fastlogin;

extern char	**environ;
struct	passwd *pwd;
char	*crypt();
char	*getpass();
char	*getenv();
char	*getlogin();

main(argc,argv)
	int argc;
	char *argv[];
{
	char *password;
	char buf[1000];
	FILE *fp;
	register char *p;
	struct skey skey;
	int i;
	char *pp;
	void rip();
	char pbuf[256];
	char skeyprompt[80];
	struct TTYSTRUCT ttyf,ttysave;
	int fflags,lword,lwordsav;

#ifdef __ultrix
	openlog("su", LOG_WARNING
#else
	openlog("su", LOG_ODELAY, LOG_AUTH);
#endif

again:
	if (argc > 1 && strcmp(argv[1], "-f") == 0) {
		fastlogin++;
		argc--, argv++;
		goto again;
	}
	if (argc > 1 && strcmp(argv[1], "-") == 0) {
		fulllogin++;
		argc--, argv++;
		goto again;
	}
	if (argc > 1 && argv[1][0] != '-') {
		user = argv[1];
		argc--, argv++;
	}
	if ((pwd = getpwuid(getuid())) == NULL) {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}
	strcpy(buf, pwd->pw_name);
	if ((pwd = getpwnam(user)) == NULL) {
		fprintf(stderr, "Unknown login: %s\n", user);
		exit(1);
	}

#if 0
	/*
	 * Only allow those in group zero to su to root.
	 */
	if (pwd->pw_uid == 0) {
		struct	group *gr;
		int i;

		if ((gr = getgrgid(0)) != NULL) {
			for (i = 0; gr->gr_mem[i] != NULL; i++)
				if (strcmp(buf, gr->gr_mem[i]) == 0)
					goto userok;
			fprintf(stderr, "You do not have permission to su %s\n",
				user);
			exit(1);
		}
	}
#endif
	userok:
#ifndef SOLARIS24
		setpriority(PRIO_PROCESS, 0, -2);
#endif

#define Getlogin()  (((p = getlogin()) && *p) ? p : buf)
	/*
	if (pwd->pw_passwd[0] == '\0' || getuid() == 0)
		goto ok;
	*/
#ifdef	OLD
	password = getpass("Password:");
	if (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0) {
		fprintf(stderr, "Sorry\n");
		if (pwd->pw_uid == 0) {
			syslog(LOG_CRIT, "BAD SU %s on %s",
					Getlogin(), ttyname(2));
		}
		exit(2);
	}
#endif
	/* Attempt a S/Key challenge */
	i = skeychallenge(&skey,user,skeyprompt);
	if (i<0) {
		fprintf(stderr, "Permission denied.\n");
		syslog(LOG_CRIT,
			"ATTEMPTED SU to %s on %s. (%s is not in SKEY database)",
			user, ttyname(2));
		exit(2);
	}

	printf("%s\n",skeyprompt);
	printf("Password:");
	fflush(stdout);

#ifndef SYSV
	/* Set normal line editing */
	fflags = fcntl(fileno(stdin),F_GETFL,0);
	fcntl(fileno(stdin),F_SETFL,fflags & ~FNDELAY);
	ioctl(fileno(stdin),TIOCLGET,&lword);
	ioctl(fileno(stdin),TIOCLGET,&lwordsav);
	lword |= LCRTERA|LCRTKIL;
	ioctl(fileno(stdin),TIOCLSET,&lword);
#endif

	/* Turn off echoing */
	gtty(fileno(stdin), &ttyf);
	gtty(fileno(stdin), &ttysave);
#ifdef SYSV
	ttyf.c_lflag &= ~(ICANON | ECHO | ECHONL);
#else
	ttyf.sg_flags &= ~(ECHO|RAW|CBREAK);
	ttyf.sg_flags |= CRMOD;
#endif			
	stty(fileno(stdin),&ttyf);

	/* Read password */
	fgets(pbuf,sizeof(pbuf),stdin);
	rip(pbuf);
	if(strlen(pbuf) == 0){
		/* Null line entered; turn echoing back on
		 * and read again
		 */
		printf(" (echo on)\nPassword:");
		fflush(stdout);
#ifdef SYSV
		ttyf.c_lflag |= (ICANON | ECHO | ECHONL);
#else
		ttyf.sg_flags |= (ECHO|CRTBS);
#endif
		stty(fileno(stdin),&ttyf);
		fgets(pbuf,sizeof(pbuf),stdin);
		rip(pbuf);
	} else {
		printf("\n");
	}
#ifdef SYSV
	stty(fileno(stdin),&ttysave);
#else
	/* Restore previous tty modes */
	fcntl(fileno(stdin),F_SETFL,fflags);
	stty(fileno(stdin),&ttysave);
	ioctl(fileno(stdin),TIOCLSET,&lwordsav);
#endif

	if(i == 0 && skeyverify(&skey,pbuf) == 0){
		/* S/Key authentication succeeded */
		if(skey.n < 5)
			printf("Warning! Change password soon\n");
		goto ok;
	} else {
		/* Try regular password check */
		pp = crypt(pbuf,pwd->pw_passwd);
		if(strcmp(pp,pwd->pw_passwd) == 0)
			goto ok;	/* Success */
	}
	fprintf(stderr, "Sorry\n");
	if (pwd->pw_uid == 0) {
		syslog(LOG_CRIT, "BAD SU %s on %s",
				Getlogin(), ttyname(2));
	}
	exit(2);

ok:
	endpwent();
	if (pwd->pw_uid == 0) {
		syslog(LOG_NOTICE, "%s on %s", Getlogin(), ttyname(2));
		closelog();
	}
	if (setgid(pwd->pw_gid) < 0) {
		perror("su: setgid");
		exit(3);
	}
	if (initgroups(user, pwd->pw_gid)) {
		fprintf(stderr, "su: initgroups failed\n");
		exit(4);
	}
	if (setuid(pwd->pw_uid) < 0) {
		perror("su: setuid");
		exit(5);
	}
	if (pwd->pw_shell && *pwd->pw_shell)
		shell = pwd->pw_shell;
	if (fulllogin) {
		cleanenv[4] = getenv("TERM");
		environ = cleanenv;
	}
	if (fulllogin || strcmp(user, "root") != 0)
		setenv("USER", pwd->pw_name, userbuf);
	setenv("SHELL", shell, shellbuf);
	setenv("HOME", pwd->pw_dir, homebuf);
#ifndef SOLARIS24
	setpriority(PRIO_PROCESS, 0, 0);
#endif
	if (fastlogin) {
		*argv-- = "-f";
		*argv = "su";
	} else if (fulllogin) {
		if (chdir(pwd->pw_dir) < 0) {
			fprintf(stderr, "No directory\n");
			exit(6);
		}
		*argv = "-su";
	} else
		*argv = "su";
	execv(shell, argv);
	fprintf(stderr, "No shell\n");
	exit(7);
}

setenv(ename, eval, buf)
	char *ename, *eval, *buf;
{
	register char *cp, *dp;
	register char **ep = environ;

	/*
	 * this assumes an environment variable "ename" already exists
	 */
	while (dp = *ep++) {
		for (cp = ename; *cp == *dp && *cp; cp++, dp++)
			continue;
		if (*cp == 0 && (*dp == '=' || *dp == 0)) {
			strcat(buf, eval);
			*--ep = buf;
			return;
		}
	}
}

char *
getenv(ename)
	char *ename;
{
	register char *cp, *dp;
	register char **ep = environ;

	while (dp = *ep++) {
		for (cp = ename; *cp == *dp && *cp; cp++, dp++)
			continue;
		if (*cp == 0 && (*dp == '=' || *dp == 0))
			return (*--ep);
	}
	return ((char *)0);
}
