/*
 * $Author: vince $
 * $Header: /users/vince/src/stel-dist/stel/RCS/steld.c,v 1.107 1996/04/26 16:29:23 vince Exp vince $
 * $Date: 1996/04/26 16:29:23 $
 * $Revision: 1.107 $
 * $Locker: vince $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: steld.c,v 1.107 1996/04/26 16:29:23 vince Exp vince $";
#endif	lint

#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <netinet/in.h>
#ifdef SUNOS4
#include <sys/types.h>
#include <sys/label.h>
#include <sys/audit.h>
#include <pwdadj.h>
#include <utmp.h>
#endif

#ifdef HAS_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef AIX
#include <sys/ioctl.h>
#include <sys/id.h>
#include <sys/priv.h>
#endif

#ifdef SOLARIS
#include <sys/stropts.h>
#include <shadow.h>
#endif

/* md5 stuff */
#include "global.h"
#include "md5.h"

#include "skey.h"
#include "defs.h"

#include "sys_defs.h"

#define DEFAULT_PATH	"/bin:/usr/bin:/usr/ucb"

#if defined (SUNOS4) || defined (SOLARIS)
#define ERASECHAR	(0x7f)
#define KILLCHAR	('u' & 0x1f)
#else
#define ERASECHAR	('h' & 0x1f)
#define KILLCHAR	('x' & 0x1f)
#endif

/* idea stuff */
#include "idea.h"
extern IDEAkey		c_ideakey;	/* client */

/* des stuff */
/* #include "des.h" */
extern des_key_schedule	c_schedule1;	/* client */
extern des_key_schedule	c_schedule2;	/* client */
extern des_key_schedule	c_schedule3;	/* client */

/* IVs */
extern des_cblock	s_IV, c_IV;

extern char		sessionkeyhash[];

void	fireman();
void	runshell();

static username[MAXUSERNAMELEN];
static fromhost[MAXHNAMELEN];

main(argc, argv)
int	argc;
char	*argv[];
{
	int		s, fh;
	int		i, kflg, portnum;
	int		timeout;
	extern char	*optarg;
	char		*p, *peername();

#ifdef __ultrix
	(void)openlog(STELSERVER, LOG_WARNING);
#else
	(void)openlog(STELSERVER, LOG_PID | LOG_ODELAY, LOG_DAEMON);
#endif

	kflg = 0;
	portnum = 0;
	timeout = IDLETIMEOUT;
	while ((i = getopt(argc, argv, "kp:t:")) != EOF)
		switch (i) {
			/* do not kill IP-OPTIONS (killed by default) */
			case	'k':
				kflg++;
				break;
			case	'p':
				portnum = atoi(optarg);
				break;
			case	't':
				timeout = atoi(optarg);
				break;
			case	':':
			case	'?':
				syslog(LOGPRI, "bad options, exiting");
				exit(1);
				break;
		}

	if ((p = peername(0)) != NULL) { /* run by inetd */
		if (portnum) 
			syslog(LOGPRI, "run by inetd, -p option ignored");
		strncpy(fromhost, p, sizeof(fromhost));
		syslog(LOGPRI, "incoming connection from %s", fromhost);

		if (!kflg) {
#ifndef NO_IP_PROTO
			/* kill IP_PROTO options */
			if (killoptions(0, fromhost)) {
				/* can't kill IP_OPTIONS */
				(void)shutdown(0, 2);
				(void)close(0);
				exit(1);
			}
#endif
		}

		(void)serveclient(0, timeout);
		syslog(LOGPRI, "end of connection from %s", fromhost);
		(void)shutdown(0, 2);
		(void)close(0);
		exit(0);
	}

#ifndef DEBUG
	detachfrompty();
#endif /* DEBUG */

	if (portnum == 0)
		portnum = PORTNUM;
	if ((s = establish(portnum)) == -1)
		exit(1);

	(void)signal(SIGCLD, fireman);

	printf("server[%d], id = %d, starting on port %d\n",
					getpid(), VERSION, portnum);
	syslog(LOGPRI, "server[%d], id = %d, started on port %d",
					getpid(), VERSION, portnum);

	for (; ; ) {
		if ((fh = getconnection(s, fromhost)) < 0) {
			if (errno == EINTR) /* child died */
				continue;
			syslog(LOGPRI, "accept(): %m");
			exit(1);
		}

		syslog(LOGPRI, "connection from %s", fromhost);

		if (!kflg) {
#ifndef linux
		/* kill IP_PROTO options */
			if (killoptions(fh, fromhost)) {
				/* can't kill IP_OPTIONS */
				(void)shutdown(fh, 2);
				(void)close(fh);
				continue;
			}
#endif
		}


#ifdef DEBUG
		(void)serveclient(fh, timeout);
		(void)shutdown(fh, 2);
		(void)close(fh);
		syslog(LOGPRI, "end of connection from %s", fromhost);
#else
		switch(fork()) {
			case	-1:
				syslog(LOGPRI, "fork(): %m");
				continue;
			case	0:
				(void)serveclient(fh, timeout);
				(void)shutdown(fh, 2);
				(void)close(fh);
				syslog(LOGPRI, "end of connection from %s",
							fromhost);
				exit(0);
			default:
				(void)close(fh);
				break;
		}
#endif
	}

	/*NOTREACHED*/
	return(0);
}

serveclient(fh, timeout)
int	fh;
int	timeout;
{
	cpacket		pp;
	spacket		ppp;
	int		child, firstack, sreply, pty, usepty = 0;
	int		i, ret, cipher;
	int		pfdin[2], pfdout[2], authpipe[2];
	char		ttyline[16], *p, *authbuf, *authbuf2;
	void		timeexpired();
	MD5_CTX		mdContext;
	unsigned char	digest[16];

	char		*getstelsecret();

	(void)signal(SIGALRM, timeexpired);
	(void)alarm(LOGINTIMEOUT);

	(void)strcpy((char *)username, "stillunknownuser");

#ifdef ASCIIBANNER
	if (sendbanner(fh)) {
		syslog(LOGPRI, "error sending BANNER: %m");
		return(1);
	}
#endif

	if ((i = read(fh, &firstack, sizeof(firstack))) != sizeof(firstack)) {
		if (!i)
			syslog(LOGPRI, "Client has gone away.");
		else
			syslog(LOGPRI, "read(firstack): %m");
		return(1);
	}
	firstack = ntohl(firstack);

	if (!(firstack & FLG_NO_ENCRYPTION)) { /* encryption required */
		if (dhexchange(fh, SERVER_SIDE, firstack, NULL) < 0) {
			syslog(LOGPRI, "dhexchange(): %m");
			return(1);
		}
	}

	if ((i = read_data(fh, &pp, sizeof(pp))) != sizeof(pp)) {
		if (!i)
			syslog(LOGPRI, "Client has gone away.");
		else
			syslog(LOGPRI, "read(): %m");
		return(1);
	}

	if (!(firstack & FLG_NO_ENCRYPTION)) {
		if (firstack & FLG_USE_IDEA) {
			cipher = IDEA_CIPHER;
		}
		else if (firstack & FLG_USE_SINGLE) {
			cipher = DES_CIPHER;
		}
		else if (firstack & FLG_USE_RC4) {
			cipher = RC4_CIPHER;
		}
		else { /* default */
			cipher = TRIPLEDES_CIPHER;
		}

		/* set client IV */
		memcpy(c_IV, pp.random, 8);

		/* decrypt, skipping IV */
		cryptbuf((char *)&pp + 8, sizeof(pp) - 8, DES_DECRYPT, cipher,
								CLIENT_SIDE);
	}

	/* check integrity */
	MD5Init(&mdContext);
	/* skip digest itself */
	MD5Update(&mdContext, &pp, sizeof(pp) - 16);
	MD5Final(digest, &mdContext);

	sreply = MSG_OKAY_DATA;

	if (memcmp(digest, pp.digest, 16))
		sreply = MSG_CORRUPTED_DATA; /* fail */
	else if (htonl(pp.version) != VERSION) {
		sreply = MSG_WRONG_VERSION;
	}
	else {
		struct passwd	*pwd;

		/* is ~/.stelsecret is required? */
		if ((pwd = getpwnam(pp.username)) != NULL) {
			char	spath[512];
			
			(void)sprintf(spath, "%s/%s",
						pwd->pw_dir, STEL_SECRET_NAME);
			/* ~/.stelsecret authentication required */
			p = getstelsecret(spath, fromhost);
			if (p != NULL) {
				authbuf = (char *)strdup(p);
				sreply |= MSG_LOGIN_AUTH_REQUIRED;
			}
		}
			
		/* is /etc/stelsecret authentication required? */
		if ((authbuf2 = getstelsecret(ETC_STEL_SECRET_NAME, fromhost)) != NULL)
			sreply |= MSG_SYSTEM_AUTH_REQUIRED;

		/* if authentication is requred we refuse client's requests
		not to use encryption. in other words encryption is a must
		when performing mutual authentication */
		if ((firstack & FLG_NO_ENCRYPTION) &&
		    (sreply & MSG_LOGIN_AUTH_REQUIRED ||
		     sreply & MSG_SYSTEM_AUTH_REQUIRED))
			sreply = MSG_MUST_AUTH; /* fail */
	}

	i = htonl(sreply);
	ppp.sreply = i;

	if (!(firstack & FLG_NO_ENCRYPTION)) {
		memcpy(ppp.random, (char *)randomdigest(), 16);
		/* set server IV */
		memcpy(s_IV, ppp.random, 8);
	}

	MD5Init(&mdContext);
	/* skip digest itself */
	MD5Update(&mdContext, &ppp, sizeof(ppp) - 16);
	MD5Final(ppp.digest, &mdContext);

	if (!(firstack & FLG_NO_ENCRYPTION)) {
		cryptbuf((char *)&ppp + 8, sizeof(ppp) - 8, DES_ENCRYPT,
							cipher, SERVER_SIDE);
	}

	if (write(fh, &ppp, sizeof(ppp)) != sizeof(ppp)) {
		syslog(LOGPRI, "write(): %m");
		return(1);
	}

	if (sreply == MSG_WRONG_VERSION) {
		syslog(LOGPRI, "Wrong %s version: expected %d, received %d.",
					STELSERVER, VERSION, htonl(pp.version));
		return(1);
	}

	if (sreply == MSG_CORRUPTED_DATA) {
		syslog(LOGPRI, "received CORRUPTED data from client");
		return(1);
	}

	if (sreply == MSG_MUST_AUTH) {
		syslog(LOGPRI, "NO ENCRYPTION request for client rejected");
		return(1);
	}

	if (sreply & MSG_SYSTEM_AUTH_REQUIRED) {
		ret = mauth(fh, authbuf2, SERVER_SIDE, firstack);
		if (ret == -1) {
			syslog(LOGPRI, "mauth(%s): %m", ETC_STEL_SECRET_NAME);
			return(1);
		}
		else if (ret == 1) {
			syslog(LOGPRI, "%s MUTUAL AUTHENTICATION FAILED",
							ETC_STEL_SECRET_NAME);
			return(1);
		}
		/* authentication succeeded */
	}

	if (sreply & MSG_LOGIN_AUTH_REQUIRED) {
		ret = mauth(fh, authbuf, SERVER_SIDE, firstack);
		if (ret == -1) {
			syslog(LOGPRI, "mauth(%s): %m", STEL_SECRET_NAME);
			return(1);
		}
		else if (ret == 1) {
			syslog(LOGPRI, "%s MUTUAL AUTHENTICATION FAILED",
							STEL_SECRET_NAME);
			return(1);
		}
		/* authentication succeeded */
	}

	/* read modes */
	pp.mode = ntohl(pp.mode);
	pp.rows = ntohl(pp.rows);
	pp.cols = ntohl(pp.cols);
	if (!(pp.mode & FLG_DONTUSE_PTY))
		usepty = 1;
	
	(void)strncpy(username, pp.username, sizeof(username));

	if (usepty) {
		if ((pty = getpty(ttyline)) == -1) {
			syslog(LOGPRI, "no pseudo terminal available");
			return(1);
		}
		if (pipe(authpipe) == -1) {
			syslog(LOGPRI, "pipe(pipeauth): %m");
			return(1);
		}
	}
	else {
		if (pipe(pfdin) == -1 ||
		    pipe(pfdout) == -1) {
			syslog(LOGPRI, "pipe(pfd): %m");
			return(1);
		}
	}

	switch ((child = fork())) {
		case	-1:
			syslog(LOGPRI, "fork(): %m");
			if (usepty) {
				(void)close(pty);
			}
			else {
				(void)close(pfdout[0]);
				(void)close(pfdout[1]);
				(void)close(pfdin[0]);
				(void)close(pfdin[1]);
			}
			return(1);
		case	0:
			/* alarm is cancelled by fork */
			(void)alarm(LOGINTIMEOUT);

			/* prevent user interrupts */
			(void)signal(SIGINT, SIG_IGN);
			(void)signal(SIGQUIT, SIG_IGN);

			if (close(authpipe[0]) == -1) {
				syslog(LOGPRI, "close(authpipe)");
				exit(1);
			}
		
			setterminal(usepty, ttyline, pfdin, pfdout,
						pp.rows, pp.cols);

			runshell(&pp, usepty, authpipe[1]);
			/*NOTREACHED*/
	}

	(void)alarm(0); /* this is master side */

	if (usepty) {
		struct passwd	*pw;

		if (close(authpipe[1]) == -1) {
			syslog(LOGPRI, "close(authpipe)");
			return(1);
		}

		if ((pw = getpwnam((char *)username)) == NULL) {
			syslog(LOGPRI, 
			"%s LOGIN ATTEMPTED (USER NOT EXISTENT), from %s",
							username, fromhost);
			return(1);
		}

		/* temporanely set user privileges.
		this is needed for the file xfer facilities
		*/
		if (geteuid() == 0) {
#ifdef AIX
/* this code from the latest NRL OPIE FTP. thanks to Craig Metz */
			priv_t priv;

			priv.pv_priv[0] = 0;
			priv.pv_priv[1] = 0;
			setgroups(NULL, NULL);
			if (setpriv(PRIV_SET | PRIV_INHERITED |
						PRIV_EFFECTIVE |PRIV_BEQUEATH,
			   &priv, sizeof(priv_t)) < 0 ||
			   setgidx(ID_REAL|ID_EFFECTIVE, pw->pw_gid) < 0 ||
			   setuidx(ID_REAL|ID_EFFECTIVE, pw->pw_uid) < 0 ||
			   seteuid((uid_t)pw->pw_uid) < 0) {
				syslog(LOGPRI, "seteuid(%d): %m", pw->pw_uid);
				return(1);
			}
#else
			if (setegid(pw->pw_gid) == -1 ||
			    seteuid(pw->pw_uid) == -1) {
				syslog(LOGPRI, "seteuid(%d): %m", pw->pw_uid);
				return(1);
			}
#endif
		}

		if (makeio(pty, pty, fh, authpipe[0], SERVER_SIDE, firstack,
				fromhost, NULL, NULL, timeout, username))
			syslog(LOGPRI, "lost connection from %s", fromhost);

		/* restore user privileges */
		if (geteuid() == 0 &&
		    (setuid(0) == -1 || setgid(0) == -1)) {
			syslog(LOGPRI, "setuid(0): %m");
			return(1);
		}

		/* eventually kill living child; this is necessary if
		   connection with client drops somehow */
		(void)signal(SIGCLD, fireman);
		(void)kill(child, SIGKILL);

		p = ttyline + sizeof("/dev/") - 1;
#ifdef SYSV_UTMP
		UTMP_LOGOUT(p);
		(void)chown(ttyline, 0, 0);
		(void)chmod(ttyline, 0644);
#else /* SYSV_UTMP */
		if (logout(p))
			logwtmp(p, "", "");
		(void)chmod(ttyline, 0666);
		(void)chown(ttyline, 0, 0);
		*p = 'p';
		(void)chmod(ttyline, 0666);
		(void)chown(ttyline, 0, 0);
#endif /* SYSV_UTMP */
		return(0);
	}
	else {
		(void)close(pfdin[1]); (void)close(pfdout[0]);
		if (makeio(pfdin[0], pfdout[1], fh, -1, SERVER_SIDE, firstack,
				fromhost, NULL, NULL, timeout, username)) {
			syslog(LOGPRI, "lost connection from %s", fromhost);
		}
		/* eventually kill living child; this is necessary if
		   client failed */
		(void)signal(SIGCLD, fireman);
		(void)kill(child, SIGKILL);
		return(0);
	}
	/*NOTREACHED*/
}

#if defined (hpux) || defined (IRIX)
char    mailpath[] = "/usr/mail/\0\0\0\0\0\0\0\0\0\0";
#else
char    mailpath[] = "/usr/spool/mail/\0\0\0\0\0\0\0\0\0\0";
#endif
char    minusnam[16] = "-";

void
runshell(pp, usepty, authfh)
cpacket	*pp;
int	usepty, authfh;
{
	char		*namep, *line = NULL;
	struct passwd	*pwd;
	int		status;
	struct		skey	skey;
	char		skeyprompt[80], skeybuf[80], stelkeyhash[64];
	char		*ttyname(), *tty, *ttyn;
	int		cnt, nosuchuser = 0;
#ifdef SYSV_UTMP
	/* SYSV login insists on an utmp(x) entry */
	char *MAKE_UTMP_ID(), *utmp_id;
#endif

	if (usepty) {
		if ((line = (char *)ttyname(0)) == NULL) {
			(void)fprintf(stderr, "terminal not attached!?\n");
			sleepexit(1);
		}
	}

	if ((pwd = getpwnam((char *) username)) == NULL) {
		syslog(LOGPRI,
		"%s LOGIN ATTEMPTED (USER NOT EXISTENT), from %s",
							username, fromhost);
		nosuchuser = 1;
		goto unixauth; /* make the hacker believe the user exists */
	}

	if(chdir(pwd->pw_dir) < 0) {
		if ((pwd->pw_uid == 0) && (chdir("/") == 0))
			printf("warning: $home is \"/\".\n");
		else {
			printf("unable to chdir to \"%s\"\n", pwd->pw_dir);
			sleepexit(1);
		}
	}

	if (usepty) {
		if ((chmod(line, 0622) == -1 ||
		    chown(line, pwd->pw_uid, pwd->pw_gid) == -1) &&
		    !geteuid()) {
			(void)fprintf(stderr, "cannot chown and chown terminal line\n");
			sleepexit(1);
		}
	}

	/* authentication loop */
	for (cnt = 0; ; ) {
		/* slow down guessing attacks (if ever) */
		if (++cnt >= 3) {
			if (cnt >= 10) {
				syslog(LOGPRI,
					"%s REPEATED LOGIN FAILURES from %s",
							username, fromhost);
				sleepexit(1);
			}
			sleep((u_int)((cnt - 3) * 5));
		}

		if (nosuchuser)
			goto unixauth;
#ifdef SECURID
		/* sdi securID authentication */
		if (issdiregistered(username)) {
			printf("steld: SECURID authentication required for %s\n",
								username);
			if (!sdiauth(usepty, username))
				goto authenticated;
			syslog(LOGPRI, "%s SECURID LOGIN FAILURE from %s",
						username, fromhost);
			continue;

		}
#endif /* SECURID */

		/* skey authentication */
		skeyprompt[0] = '\0';
		if (skeychallenge(&skey, (char *)username, skeyprompt)) {
			/* user is not skey registered or skey daemon
			it not responding.  default to unix authentication */
			goto unixauth;
		}
		printf("steld: S/KEY authentication required for %s\n",
								username);
			
		/* printf("[%s]\n",skeyprompt); */
		printf("%s\n",skeyprompt);

		printf("Response: ");
		(void)fflush(stdout);
		if (fgets(skeybuf, sizeof(skeybuf), stdin) == NULL)
			sleepexit(1);
		rip(skeybuf);
		if (skeyverify(&skey, skeybuf))  {
			syslog(LOGPRI, "%s SKEY LOGIN FAILURE from %s",
						username, fromhost);
			puts("Invalid response");
			continue;
		}
		else { /* skey successful */
			break;
		}
unixauth:
		if (!skeyaccess(username, line, fromhost, fromhost)) {
			syslog(LOGPRI,
				"UNIX PASSWORDS NOT PERMITTED for %s from %s",
							username, fromhost);
			puts("UNIX passwords NOT permitted.");
			sleepexit(1);
		}
		/* unix password authentication */
		printf("steld: UNIX password authentication required for %s\n",
								username);
		if (mkunixauth(usepty, nosuchuser)) {
			syslog(LOGPRI, "%s LOGIN FAILURE from %s",
							username, fromhost);
			puts("Login incorrect");
			continue;
		}
		else /* unixauth successful */
			break;
	}

#ifdef SECURID
authenticated:
#endif
	if (!login_access(username, fromhost)) {
		syslog(LOGPRI, "%s LOGIN REFUSED FROM %s", username, fromhost);
		puts("Permission denied");
		sleepexit(1);
	}

	(void)alarm(0);

	/* the following are performed by superuser only */
	if (!geteuid()) {
#ifndef SYSV_UTMP
		struct utmp utmp;
#endif

		if (usepty) {
			ttyn = ttyname(0);
			if (ttyn == NULL || *ttyn == '\0')
				ttyn = "/dev/tty??";
			if ((tty = (char *) strchr(ttyn + 1, '/')))
				++tty;
			else
				tty = ttyn;
			
#ifdef SYSV_UTMP
			utmp_id = MAKE_UTMP_ID(ttyn, "tn");
			UTMP_INIT(tty, ".telnet", utmp_id);
			if (UTMP_LOGIN(tty, username, fromhost) != 0) {
				syslog(LOGPRI, "UTMP_INIT() failed");
				sleepexit(1);
			}
#else /* SYSV_UTMP */
			memset((char *)&utmp, 0, sizeof(utmp));
			(void)time(&utmp.ut_time);
			strncpy(utmp.ut_name, username, sizeof(utmp.ut_name));
#ifndef SOLARIS
			strncpy(utmp.ut_host, fromhost, sizeof(utmp.ut_host));
#endif
			strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
			login(&utmp);
#endif /* SYSV_UTMP */
		}
	}

	if (!geteuid()) {
#ifdef GENERICLOGIN
		if (initgroups(pwd->pw_name, -1) == -1) {
			syslog(LOGPRI, "initgroups(): %m");
			sleepexit(1);
		}
#ifdef hpux
		if (setgid(pwd->pw_gid) ||
				setresuid(pwd->pw_uid, pwd->pw_uid, 0))
#else
		if (setgid(pwd->pw_gid) || setuid(pwd->pw_uid))
#endif
							{
			(void)fprintf(stderr, "can not set user IDs");
			sleepexit(1);
		}
#endif /* GENERICLOGIN */
	}

	(void)strncat(mailpath, pwd->pw_name, 8);
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";

	/* provide a hashing of the session key */
	sprintf(stelkeyhash, "%02x%02x%02x%02x%02x%02x%02x%02x",
		(unsigned char)sessionkeyhash[0],
		(unsigned char)sessionkeyhash[1],
		(unsigned char)sessionkeyhash[2],
		(unsigned char)sessionkeyhash[3],
		(unsigned char)sessionkeyhash[4],
		(unsigned char)sessionkeyhash[5],
		(unsigned char)sessionkeyhash[6],
		(unsigned char)sessionkeyhash[7]);

	if (
	    putvar("HOME",      pwd->pw_dir)	||
	    putvar("LOGNAME",   pwd->pw_name)	||
	    putvar("USER",   	pwd->pw_name)	||
	    putvar("USERNAME", 	pwd->pw_name)	||
	    putvar("SHELL",     pwd->pw_shell)	||
	    putvar("MAIL",      mailpath)	||
	    putvar("TERM",	pp->TERM)	||
	    putvar("LINES",	pp->LINES)	||
	    putvar("COLUMNS",	pp->COLUMNS)	||
	    putvar("DISPLAY",	pp->DISPLAY)	||
	    putvar("WINDOWID",	pp->WINDOWID)	||
	    putvar("STELKEYHASH", stelkeyhash)	||
	    /* putvar("PATH",	pp->PATH)	|| */
	    putvar("PATH",	DEFAULT_PATH)
						) {
		(void)fprintf(stderr, "can not set environment\n");
		sleepexit(1);
	}

	if (line == NULL)
		line = "unattached";

	/* restore signals */
	(void)signal(SIGINT, SIG_DFL);
	(void)signal(SIGQUIT, SIG_DFL);

	if (usepty) {
		if (write(authfh, USRAUTH, strlen(USRAUTH)) != strlen(USRAUTH)) {
			syslog(LOGPRI, "write(authfh): %m");
			exit(1);
		}
		/* won't be used again */
		if (close(authfh) == -1) {
			syslog(LOGPRI, "close(authbuf): %m");
			exit(1);
		}
	}

	if (pp->command[0] == '\0') {
		if ((namep = (char *)strrchr(pwd->pw_shell, '/')) == NULL)
			namep = pwd->pw_shell;
		else
			namep++;
		(void)strcat(minusnam, namep);

		syslog(LOGPRI, "%s LOGIN from %s, %s, \"%s\"",
				username, fromhost, line, pwd->pw_shell);

#ifdef GENERICLOGIN
		(void)execl(pwd->pw_shell, minusnam, 0);
		syslog(LOGPRI, "execl(%s): %m", pwd->pw_shell);
		(void)fprintf(stderr, "no shell\n");
		sleepexit(1);
#else
		/* -f: skip a second authentication
		   -p: preserve environment
		*/
		(void)execl("/bin/login", "login", "-f", "-p", "-l",
							pwd->pw_name, NULL);
		syslog(LOGPRI, "execl(/bin/login): %m");
		(void)fprintf(stderr, "no /bin/login\n");
		sleepexit(1);
#endif
	}
	else {
#ifdef NOREMEXEC
		syslog(LOGPRI, "%s LOGIN FAILURE from %s, %s, rcmd not permitted, \"%s\"",
				username, fromhost, line, pp->command);
		printf("Remote command execution not permitted.\n");
		sleepexit(1);
#else
		syslog(LOGPRI, "%s LOGIN from %s, %s, \"%s\"",
					username, fromhost, line, pp->command);
		(void)signal(SIGCLD, SIG_DFL);
		if ((status = system(pp->command)) == -1) {
			syslog(LOGPRI, "system() failed: %m");
			sleepexit(1);
		}
		if (status & 0177 || status >> 8) {
			syslog(LOGPRI, "command failed \"%s\"", pp->command);
			sleepexit(1);
		}
		exit(0);
#endif
	}
}

putvar(name, value)
char	*name, *value;
{
	char		*p;
	size_t		size;
	
	if (!value || !*value)
		return(0);
	size = strlen(name) + strlen(value);
	size += 2; /* '=' + '\0' */
	if ((p = (char *) malloc(size)) == NULL) {
		syslog(LOGPRI, "malloc(): %m");
		return(1);
	}
	(void)sprintf(p, "%s=%s", name, value);
	(void)putenv(p);

	return(0);
}


mkunixauth(usepty, fail)
int	usepty, fail;
{
	char		*crypt(), *getpass();
	char		*p;
	struct passwd	*pwd;
	struct passwd	*shadow_getpwnam();

	if (!fail && (pwd = shadow_getpwnam((char *) username)) == NULL)
		/* non existent user */
		return(1);

	if (usepty) {
		p = getpass("Password: ");
		if (p == NULL || !strlen(p))
			return(1);
	}
	else {
		if ((p = (char *) malloc(MAXPWDLEN)) == NULL) {
			syslog(LOGPRI, "malloc(): %m");
			return(1);
		}
		fprintf(stderr, "Password (echo is on): ");
		(void)fflush(stderr);
		if (fgets(p, MAXPWDLEN, stdin) == NULL)
			return(1);
		/* p[strlen(p) - 2] = '\0'; */ /* remote \n\r */
	}

	if (fail)
		return(1);

	return(strcmp(pwd->pw_passwd, crypt(p, pwd->pw_passwd)));
}

#ifndef DEBUG
detachfrompty()
{
	if (fork() > 0)
		exit(0);
	else {
/*
#if defined(SUNOS4) || defined(SOLARIS)
		(void)setpgrp(getuid(), 0);
#else
		(void)setpgrp();
#endif
*/
		setsid();
	}
}
#endif

void
timeexpired(s)
int	s;
{
	(void)signal(s, timeexpired);
	syslog(LOGPRI, "time expired for %s from %s", username, fromhost);
	exit(1);
}

#ifdef AIX
#ifndef TIOCSWINSZ
#define TIOCSWINSZ	_IOW('t', 103, struct winsize)  /* set window size */
#endif
#endif
setterminal(usepty, ttyline, pfdin, pfdout, rows, cols)
char	*ttyline;
int	usepty, *pfdin, *pfdout;
int	rows, cols;
{
	int		i;
	struct winsize	os_window;

	/* pty attached */
	if (usepty) {
		int		t;
		struct termios	b;

		if ((t = open(ttyline, O_RDWR)) == -1) {
			syslog(LOGPRI, "open1(%s): %m", ttyline);
			exit(1);
		}

#ifdef SOLARIS
		if (ioctl(t, I_PUSH, "ptem") < 0 ||
		    ioctl(t, I_PUSH, "ldterm") < 0 ||
		    ioctl(t, I_PUSH, "ttcompat") < 0) {
			syslog(LOGPRI, "Cannot push streams modules onto pty");
			exit(1);
		}
#endif
		/* get terminal attributes */
		if (tcgetattr(t, &b) == -1) {
			syslog(LOGPRI, "tcgetattr(): %m");
			exit(1);
		}

#ifdef hpux
		b.c_lflag |= ECHO | ECHOK | ECHOE | ICANON | ISIG;
		b.c_iflag |= ICRNL | BRKINT | ISTRIP | IXON;
		b.c_iflag &= ~(IXANY | PARMRK);
		b.c_oflag |= ONLCR | OPOST;
#else
		b.c_lflag |= ECHO;
		b.c_iflag |= ICRNL;
		b.c_oflag |= ONLCR;
#endif
		b.c_cflag = B9600 | CS8 | CREAD | HUPCL;

		b.c_cc[VERASE] = ERASECHAR;
		b.c_cc[VKILL] = KILLCHAR;
		b.c_cc[VEOF] = 'd'&0x1f;
		b.c_cc[VINTR] = 0x3; /* ^C */
		b.c_cc[VEOL] = b.c_cc[VSUSP] = 0;
		if (tcsetattr(t, TCSANOW, &b) == -1) {
			syslog(LOGPRI, "tcsetattr(): %m");
			exit(1);
		}


		if (rows >= 0 && cols >= 0) {
			os_window.ws_row = rows;
			os_window.ws_col = cols;
			if (ioctl(t,TIOCSWINSZ,&os_window) == -1) {
				syslog(LOGPRI, "ioctl(): %m");
				exit(1);
			}
		}

		/* Acquire a controlling terminal */
		if (setsid() == -1) {
			syslog(LOGPRI, "setsid(): %m");
			exit(1);
		}
		i = t;
		if ((t = open(ttyline, O_RDWR)) < 0) {
			syslog(LOGPRI, "open2(ttyline): %m");
			exit(1);
		}
		(void)close(i);
		
		if (dup2(t, 0) == -1 ||
		    dup2(t, 1) == -1 ||
		    dup2(t, 2) == -1)  {
			syslog(LOGPRI, "dup2(): %m");
			exit(1);
		}

		return(t);

	}
	/* no pty attached */
	else {
		if (close(0) || close(1) || close(2)) {
			syslog(LOGPRI, "close(): %m");
			exit(1);
		}

#if defined(SUNOS4) || defined(SOLARIS)
		if (setpgrp(getuid(), 0) == -1)
#else
		if (setpgrp() == -1)
#endif
					{
			syslog(LOGPRI, "setpgrp(): %m");
			exit(1);
		}

		if (dup(pfdout[0]) != 0 || dup(pfdin[1]) != 1 ||
		    dup(pfdin[1]) != 2) {
			syslog(LOGPRI, "dup(): %m");
			exit(1);
		}

		if (close(pfdout[0]) == -1 || close(pfdout[1]) == -1 ||
		    close(pfdin[0]) == -1 || close(pfdin[1]) == -1) {
			syslog(LOGPRI, "close(): %m");
			exit(1);
		}
		
		return(-1); /* unused! */
	}
	/*NOTREACHED*/
}

struct passwd *
shadow_getpwnam(user)
char	*user;
{
	static struct passwd	*pwd;
#ifdef SUNOS4
	struct passwd_adjunct	*pa;
#else
#ifdef SOLARIS
	struct	spwd		*pa;
#endif
#endif
	if ((pwd = getpwnam(user)) == NULL)
		return(NULL);
	
#ifdef SUNOS4
	if (issecure()) {
		if ((pa = getpwanam(user)) == NULL)
			return(NULL);
		pwd->pw_passwd =  (char *)strdup(pa->pwa_passwd);
	}
#else
#ifdef SOLARIS
	if ((pa = getspnam(user)) == NULL)
		return(NULL);
	pwd->pw_passwd = (char *)strdup(pa->sp_pwdp);
#endif
#endif
	return(pwd);
}

#ifdef SECURID
#define SECURIDUSRLIST	"/etc/securid.conf"
issdiregistered(user)
char	*user;
{
	char	line[80];
	FILE	*fp;

	if ((fp = fopen(SECURIDUSRLIST, "r")) == NULL)
		return(0);
	while (fgets(line, sizeof(line), fp) != NULL) {
		rip(line);
		if (line[0] == '#')
			continue;
		if (!strcmp(user, line)) {
			(void)fclose(fp);
			return(1);
		}
	}
	(void)fclose(fp);
	return(0);
}
#endif

sleepexit(eval)
int eval;
{
	sleep((u_int)5);
	exit(eval);
}
