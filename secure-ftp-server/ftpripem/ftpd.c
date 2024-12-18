/*
 * Copyright (c) 1985, 1988, 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985, 1988, 1990 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)ftpd.c	5.37 (Berkeley) 6/27/90";
#endif /* not lint */

/*
 * FTP server.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/dir.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#define	FTP_NAMES
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <setjmp.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#ifdef __STRICT_BSD__
#include <strings.h>
#else
#include <string.h>
#endif
#include <syslog.h>
#include <varargs.h>
#include "pathnames.h"
#include "ftpfile.h"
#include "normstrp.h"
#include "parsitpr.h"

/*
 * File containing login names
 * NOT to be used on this machine.
 * Commonly used to disallow uucp.
 */
extern	int errno;
extern	char *crypt();
extern	char version[];
extern	char *home;		/* pointer to home directory for glob */
extern	FILE *ftpd_popen(), *fopen(), *freopen();
extern	int  ftpd_pclose(), fclose();
extern	char *getline();
extern	char cbuf[];
extern	off_t restart_point;

struct	sockaddr_in ctrl_addr;
struct	sockaddr_in data_source;
struct	sockaddr_in data_dest;
struct	sockaddr_in his_addr;
struct	sockaddr_in pasv_addr;

int	data;
jmp_buf	errcatch, urgcatch;
int	logged_in;
struct	passwd *pw;
int	debug;
int	timeout = 900;    /* timeout after 15 minutes of inactivity */
int	maxtimeout = 7200;/* don't allow idle time to be set beyond 2 hours */
int	logging;
int	guest;
int   special_guest=0;  /* MRR */
#define MAX_USER_NAME 80
char  special_user[MAX_USER_NAME];
char  email_addr[MAX_USER_NAME];
int	type;
int	form;
int	stru;			/* avoid C keyword */
int	mode;
int	usedefault = 1;		/* for data transfers */
int	pdata = -1;		/* for passive mode */
int	transflag;
off_t	file_size;
off_t	byte_count;
#if !defined(CMASK) || CMASK == 0
#undef CMASK
#define CMASK 027
#endif
int	defumask = CMASK;		/* default umask value */
char	tmpline[7];
char	hostname[MAXHOSTNAMELEN];
char	remotehost[MAXHOSTNAMELEN];
char	hostbuffer[MAXHOSTNAMELEN];

/*
 * Timeout intervals for retrying connections
 * to hosts that don't accept PORT cmds.  This
 * is a kludge, but given the problems with TCP...
 */
#define	SWAITMAX	90	/* wait at most 90 seconds */
#define	SWAITINT	5	/* interval between retries */

int	swaitmax = SWAITMAX;
int	swaitint = SWAITINT;

int	checkforeign();  /* MRR */
int	lostconn();
int	myoob();
FILE	*getdatasock(), *dataconn();

#ifdef SETPROCTITLE
char	**Argv = NULL;		/* pointer to argument vector */
char	*LastArgv = NULL;	/* end of argv */
char	proctitle[BUFSIZ];	/* initial part of title */
#endif /* SETPROCTITLE */

#define LOGFILE "/usr/adm/ftplog"
#define STATFILE "/usr/adm/ftpstat"
char starttime[8],endtime[8];
int logfd = -1;
int statfd = -1;
int misclogs = 0;
long transtime,contime;
int transerr,ftpdpid;
long get_time=0;
long get_len=0;
int get_count=0;
long put_time=0;
long put_len=0;
int put_count=0;
int ab_count=0;

main(argc, argv, envp)
	int argc;
	char *argv[];
	char **envp;
{
	int addrlen, on = 1, tos;
	char *cp;

	addrlen = sizeof (his_addr);
	if (getpeername(0, (struct sockaddr *)&his_addr, &addrlen) < 0) {
		syslog(LOG_ERR, "getpeername (%s): %m",argv[0]);
		exit(1);
	}
	addrlen = sizeof (ctrl_addr);
	if (getsockname(0, (struct sockaddr *)&ctrl_addr, &addrlen) < 0) {
		syslog(LOG_ERR, "getsockname (%s): %m",argv[0]);
		exit(1);
	}
#ifdef IP_TOS
	tos = IPTOS_LOWDELAY;
	if (setsockopt(0, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
		syslog(LOG_WARNING, "setsockopt (IP_TOS): %m");
#endif
	data_source.sin_port = htons(ntohs(ctrl_addr.sin_port) - 1);
	debug = 0;
	openlog("ftpd", LOG_PID, LOG_DAEMON);
#ifdef SETPROCTITLE
	/*
	 *  Save start and extent of argv for setproctitle.
	 */
	Argv = argv;
	while (*envp)
		envp++;
	LastArgv = envp[-1] + strlen(envp[-1]);
#endif /* SETPROCTITLE */

	argc--, argv++;
	while (argc > 0 && *argv[0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'v':
			debug = 1;
			break;

		case 'd':
			debug = 1;
			break;

		case 'l':
			logging = 1;
			break;

		case 't':
			timeout = atoi(++cp);
			if (maxtimeout < timeout)
				maxtimeout = timeout;
			goto nextopt;

		case 'T':
			maxtimeout = atoi(++cp);
			if (timeout > maxtimeout)
				timeout = maxtimeout;
			goto nextopt;

		default:
			fprintf(stderr, "ftpd: Unknown flag -%c ignored.\n",
			     *cp);
			break;
		}
nextopt:
		argc--, argv++;
	}
	(void) freopen(_PATH_DEVNULL, "w", stderr);
	(void) signal(SIGPIPE, lostconn);
	(void) signal(SIGCHLD, SIG_IGN);
	if ((int)signal(SIGURG, myoob) < 0)
		syslog(LOG_ERR, "signal: %m");

	/* Try to handle urgent data inline */
#ifdef SO_OOBINLINE
	if (setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)) < 0)
		syslog(LOG_ERR, "setsockopt: %m");
#endif

#ifdef	F_SETOWN
	if (fcntl(fileno(stdin), F_SETOWN, getpid()) == -1)
		syslog(LOG_ERR, "fcntl F_SETOWN: %m");
#endif
	dolog(&his_addr);
	/*
	 * Set up default state
	 */
	data = -1;
	type = TYPE_A;
	form = FORM_N;
	stru = STRU_F;
	mode = MODE_S;
	tmpline[0] = '\0';
	(void) gethostname(hostname, sizeof (hostname));
	reply(220, "%s FTP server (%s) ready.", hostname, version);
	(void) setjmp(errcatch);
	for (;;)
		(void) yyparse();
	/* NOTREACHED */
}

lostconn()
{

	if (debug)
		syslog(LOG_DEBUG, "lost connection");
	dologout(-1);
}

static char ttyline[20];

#ifdef __GNUC__
	char *malloc();
#endif
/*
 * Helper function for sgetpwnam().
 */
char *
sgetsave(s)
	char *s;
{
#ifndef __GNUC__
	char *malloc();
#endif
	char *new = malloc((unsigned) strlen(s) + 1);

	if (new == NULL) {
		perror_reply(421, "Local resource failure: malloc");
		dologout(1);
		/* NOTREACHED */
	}
	(void) strcpy(new, s);
	return (new);
}

/*
 * Save the result of a getpwnam.  Used for USER command, since
 * the data returned must not be clobbered by any other command
 * (e.g., globbing).
 */
struct passwd *
sgetpwnam(name)
	char *name;
{
	static struct passwd save;
	register struct passwd *p;
	char *sgetsave();

	if ((p = getpwnam(name)) == NULL)
		return (p);
	if (save.pw_name) {
		free(save.pw_name);
		free(save.pw_passwd);
		free(save.pw_gecos);
		free(save.pw_dir);
		free(save.pw_shell);
	}
	save = *p;
	save.pw_name = sgetsave(p->pw_name);
	save.pw_passwd = sgetsave(p->pw_passwd);
	save.pw_gecos = sgetsave(p->pw_gecos);
	save.pw_dir = sgetsave(p->pw_dir);
	save.pw_shell = sgetsave(p->pw_shell);
	return (&save);
}

int login_attempts;		/* number of failed login attempts */
int askpasswd;			/* had user command, ask for passwd */

/*
 * USER command.
 * Sets global passwd pointer pw if named account exists and is acceptable;
 * sets askpasswd if a PASS command is expected.  If logged in previously,
 * need to reset state.  If name is "ftp" or "anonymous", the name is not in
 * _PATH_FTPUSERS, and ftp account exists, set guest and pw, then just return.
 * If account doesn't exist, ask for passwd anyway.  Otherwise, check user
 * requesting login privileges.  Disallow anyone who does not have a standard
 * shell as returned by getusershell().  Disallow anyone mentioned in the file
 * _PATH_FTPUSERS to allow people such as root and uucp to be avoided.
 */
user(name_given)
	char *name_given;
{
	register char *cp;
	char *shell;
	char *getusershell();
	char *name = name_given;
	int got_guest = 0;  

	if (logged_in) {
		if (guest) {
			reply(530, "Can't change user from guest login.");
			return;
		}
		end_login();
	}

	guest = 0;
	if(strncmp(name,"guest.",6) == 0) {
		special_guest = 1;
		strncpy(special_user,name,MAX_USER_NAME);
		name = "ftpsec";
		got_guest = 1;
	}
	if (strcmp(name, "ftp") == 0 || strcmp(name, "anonymous") == 0) {
		name = "ftp";
		got_guest = 1;
	}
	 if(got_guest) {
		if (checkuser("ftp") || checkuser("anonymous")) {
			reply(530, "User %s access denied.", name);
		} else if (0) {
/*		} else if (checkforeign(remotehost) && !special_guest) { */
			/* Modified by MRR to provide secure access */
			/* It's a foreign user--deny access. */
			char buf[512];
			long now;

			time(&now);
			reply(530, "Sorry, %s, I couldn't verify you're from USA/Canada. Access denied.",remotehost);
			if (logfd < 0) logfd = open(LOGFILE, O_WRONLY|O_APPEND);
			if (logfd >= 0) {
	  			sprintf(buf, "%s (%d) refused connection at %.20s\n", 
		  		 remotehost, getpid(), ctime(&now)+4); 
	  			write(logfd, buf, strlen(buf));
			}
			return;
		} else if ((pw = sgetpwnam(name)) != NULL) {
			guest = 1;
			askpasswd = 1;
			if(special_guest) {
				reply(331, "Welcome, secure guest user.  Enter your assigned FTP password.");
			} else {
				reply(331, "Welcome, %s.  Send email address as password.",
				remotehost);
			}
		} else
			reply(530, "User %s unknown.", name);
		return;
	}
	if (pw = sgetpwnam(name)) {
		if ((shell = pw->pw_shell) == NULL || *shell == 0)
			shell = _PATH_BSHELL;
		while ((cp = getusershell()) != NULL)
			if (strcmp(cp, shell) == 0)
				break;
		endusershell();
		if (cp == NULL || checkuser(name)) {
			reply(530, "User %s access denied.", name);
			if (logging)
				syslog(LOG_NOTICE,
				    "FTP LOGIN REFUSED FROM %s, %s",
				    remotehost, name);
			pw = (struct passwd *) NULL;
			return;
		}
	}
	reply(331, "Password required for %s.", name);
	askpasswd = 1;
	/*
	 * Delay before reading passwd after first failed
	 * attempt to slow down passwd-guessing programs.
	 */
	if (login_attempts)
		sleep((unsigned) login_attempts);
}

/*
 * Check if a user is in the file _PATH_FTPUSERS
 */
checkuser(name)
	char *name;
{
	register FILE *fd;
	register char *p;
	char line[BUFSIZ];

	if ((fd = fopen(_PATH_FTPUSERS, "r")) != NULL) {
		while (fgets(line, sizeof(line), fd) != NULL)
			if ((p = index(line, '\n')) != NULL) {
				*p = '\0';
				if (line[0] == '#')
					continue;
				if (strcmp(line, name) == 0)
					return (1);
			}
		(void) fclose(fd);
	}
	return (0);
}

/*
 * Terminate login as previous user, if any, resetting state;
 * used when USER command is given or login fails.
 */
end_login()
{

	(void) seteuid((uid_t)0);
	if (logged_in)
	  {
	    logwtmp(ttyline, "", "");
	  }
	pw = NULL;
	logged_in = 0;
	guest = 0;
}

pass(passwd)
	char *passwd;
{
	char *xpasswd, *salt;

	if (logged_in || askpasswd == 0) {
		reply(503, "Login with USER first.");
		return;
	}
	askpasswd = 0;
	if(special_guest) {
		/* guest.xxxxx user--must check pw and remotehost */
		if(!CheckSpecialGuest(special_user,passwd,remotehost)) return;
	} else if (guest) {
		/* Regular anonymous user. */
	} else {		/* Regular user--must check password. */
		if (pw == NULL)
			salt = "xx";
		else
			salt = pw->pw_passwd;
		xpasswd = crypt(passwd, salt);
		/* The strcmp does not catch null passwords! */
		if (pw == NULL || *pw->pw_passwd == '\0' ||
		    strcmp(xpasswd, pw->pw_passwd)) {
			reply(530, "Login incorrect.");
			pw = NULL;
			if (login_attempts++ >= 5) {
				syslog(LOG_NOTICE,
				    "repeated login failures from %s",
				    remotehost);
				exit(0);
			}
			return;
		}
	}
	login_attempts = 0;		/* this time successful */
	(void) setegid((gid_t)pw->pw_gid);
	(void) initgroups(pw->pw_name, pw->pw_gid);


	if (guest && logfd < 0) logfd = open(LOGFILE, O_WRONLY|O_APPEND);
	if (guest && statfd < 0) statfd = open(STATFILE, O_WRONLY|O_APPEND);
	if (guest)
	  {
	    time(&contime);
	    ftpdpid=getpid();
	  }

	/* open wtmp before chroot */
	(void)sprintf(ttyline, "ftp%d", getpid());
	logwtmp(ttyline, pw->pw_name, remotehost);
	logged_in = 1;

	if (guest) {
	  FILE *ftplock;
		ftplock = fopen("/tmp/ftp.lock", "r");
		if (ftplock != NULL) {
		  reply(550, 
		  "FTP service is currently disabled. Try later again.");
		  goto bad;
		}

		/*
		 * We MUST do a chdir() after the chroot. Otherwise
		 * the old current directory will be accessible as "."
		 * outside the new root!
		 */
		if (chroot(pw->pw_dir) < 0 || chdir("/") < 0) {
			reply(550, "Can't set guest privileges.");
			goto bad;
		}

	} else if (chdir(pw->pw_dir) < 0) {
		if (chdir("/") < 0) {
			reply(530, "User %s: can't change directory to %s.",
			    pw->pw_name, pw->pw_dir);
			goto bad;
		} else
			lreply(230, "No directory! Logging in with home=/");
	}
	if (seteuid((uid_t)pw->pw_uid) < 0) {
		reply(550, "Can't set uid.");
		goto bad;
	}
	if (guest) {
		FILE *motd;
		long now;

		lreply(230, "Guest login ok, access restrictions apply.");

		motd = fopen("/etc/motd", "r");
		if (motd != NULL) {
			char buf[BUFSIZ];

			while (fgets(buf, sizeof buf, motd) != NULL)
				printf("    %.*s\r\n", strlen(buf)-1, buf);
			fclose(motd );
		}

		time(&now);
		/* I am having a really hard time getting this to return
		 * local time on a Sun!!!  /mrr
		 */
		{
			struct tm *tmptr; 

			tmptr = localtime(&now);
			tmptr->tm_gmtoff = 4*60;
			now = timelocal(tmptr);

			reply(230, "GMT time is: %.24s", asctime(tmptr));
		}

		logident((char *)copy(passwd));

#ifdef SETPROCTITLE
		sprintf(proctitle, "%s: anonymous/%.*s", remotehost,
		    sizeof(proctitle) - sizeof(remotehost) -
		    sizeof(": anonymous/"), passwd);
		setproctitle(proctitle);
#endif /* SETPROCTITLE */
		if (logging)
			syslog(LOG_INFO, "ANONYMOUS FTP LOGIN FROM %s, %s",
			    remotehost, passwd);
	} else {
		reply(230, "User %s logged in.", pw->pw_name);
#ifdef SETPROCTITLE
		sprintf(proctitle, "%s: %s", remotehost, pw->pw_name);
		setproctitle(proctitle);
#endif /* SETPROCTITLE */
		if (logging)
			syslog(LOG_INFO, "FTP LOGIN FROM %s, %s",
			    remotehost, pw->pw_name);
	}
	home = pw->pw_dir;		/* home dir for globbing */
	(void) umask(defumask);
	return;
bad:
	/* Forget all about it... */
	end_login();
}

retrieve(cmd, name)
	char *cmd, *name;
{
	FILE *fin, *dout;
	struct stat st;
	int (*closefunc)();

	if (cmd == 0) {
		fin = fopen(name, "r"), closefunc = fclose;
		st.st_size = 0;
	} else {
		char line[BUFSIZ];

		(void) sprintf(line, cmd, name), name = line;
		fin = ftpd_popen(line, "r"), closefunc = ftpd_pclose;
		st.st_size = -1;
		st.st_blksize = BUFSIZ;
	}
	if (fin == NULL) {
		if (errno != 0)
			perror_reply(550, name);
		return;
	}
	if (cmd == 0 &&
	    (fstat(fileno(fin), &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG)) {
		reply(550, "%s: not a plain file.", name);
		goto done;
	}
	if (restart_point) {
		if (type == TYPE_A) {
			register int i, n, c;

			n = restart_point;
			i = 0;
			while (i++ < n) {
				if ((c=getc(fin)) == EOF) {
					perror_reply(550, name);
					goto done;
				}
				if (c == '\n')
					i++;
			}	
		} else if (lseek(fileno(fin), restart_point, L_SET) < 0) {
			perror_reply(550, name);
			goto done;
		}
	}
	dout = dataconn(name, st.st_size, "w");
	if (dout == NULL)
		goto done;
	time(&transtime);
	send_data(fin, dout, st.st_blksize);
	(void) fclose(dout);
	if (cmd==0) logxfer("GET",name,transtime,st.st_size);
	data = -1;
	pdata = -1;
done:
	(*closefunc)(fin);
}

store(name, mode, unique)
	char *name, *mode;
	int unique;
{
	FILE *fout, *din;
	struct stat st;
	struct stat lst;
	int (*closefunc)();
	char *gunique();

	if (unique && stat(name, &st) == 0 &&
	    (name = gunique(name)) == NULL)
		return;

	if (restart_point)
		mode = "r+w";
	fout = fopen(name, mode);
	closefunc = fclose;
	if (fout == NULL) {
		perror_reply(553, name);
		return;
	}
	if (restart_point) {
		if (type == TYPE_A) {
			register int i, n, c;

			n = restart_point;
			i = 0;
			while (i++ < n) {
				if ((c=getc(fout)) == EOF) {
					perror_reply(550, name);
					goto done;
				}
				if (c == '\n')
					i++;
			}	
			/*
			 * We must do this seek to "current" position
			 * because we are changing from reading to
			 * writing.
			 */
			if (fseek(fout, 0L, L_INCR) < 0) {
				perror_reply(550, name);
				goto done;
			}
		} else if (lseek(fileno(fout), restart_point, L_SET) < 0) {
			perror_reply(550, name);
			goto done;
		}
	}
	time(&transtime);
	din = dataconn(name, (off_t)-1, "r");
	if (din == NULL)
		goto done;
	if (receive_data(din, fout) == 0) {
		if (unique)
			reply(226, "Transfer complete (unique file name:%s).",
			    name);
		else
			reply(226, "Transfer complete.");
	}
	(void) fclose(din);
        stat(name, &lst);	
	logxfer("PUT",name,transtime,lst.st_size);
	data = -1;
	pdata = -1;
done:
	(*closefunc)(fout);
}

FILE *
getdatasock(mode)
	char *mode;
{
	int s, on = 1, tries;

	if (data >= 0)
		return (fdopen(data, mode));
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (NULL);
	(void) seteuid((uid_t)0);
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
	    (char *) &on, sizeof (on)) < 0)
		goto bad;
	/* anchor socket to avoid multi-homing problems */
	data_source.sin_family = AF_INET;
	data_source.sin_addr = ctrl_addr.sin_addr;
	for (tries = 1; ; tries++) {
		if (bind(s, (struct sockaddr *)&data_source,
		    sizeof (data_source)) >= 0)
			break;
		if (errno != EADDRINUSE || tries > 10)
			goto bad;
		sleep(tries);
	}
	(void) seteuid((uid_t)pw->pw_uid);
#ifdef IP_TOS
	on = IPTOS_THROUGHPUT;
	if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&on, sizeof(int)) < 0)
		syslog(LOG_WARNING, "setsockopt (IP_TOS): %m");
#endif
	return (fdopen(s, mode));
bad:
	(void) seteuid((uid_t)pw->pw_uid);
	(void) close(s);
	return (NULL);
}

FILE *
dataconn(name, size, mode)
	char *name;
	off_t size;
	char *mode;
{
	char sizebuf[32];
	FILE *file;
	int retry = 0, tos;

	file_size = size;
	byte_count = 0;
	if (size != (off_t) -1)
		(void) sprintf (sizebuf, " (%ld bytes)", size);
	else
		(void) strcpy(sizebuf, "");
	if (pdata >= 0) {
		struct sockaddr_in from;
		int s, fromlen = sizeof(from);

		s = accept(pdata, (struct sockaddr *)&from, &fromlen);
		if (s < 0) {
			reply(425, "Can't open data connection.");
			(void) close(pdata);
			pdata = -1;
			return(NULL);
		}
		(void) close(pdata);
		pdata = s;
#ifdef IP_TOS
		tos = IPTOS_LOWDELAY;
		(void) setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos,
		    sizeof(int));
#endif
		reply(150, "Opening %s mode data connection for %s%s.",
		     type == TYPE_A ? "ASCII" : "BINARY", name, sizebuf);
		return(fdopen(pdata, mode));
	}
	if (data >= 0) {
		reply(125, "Using existing data connection for %s%s.",
		    name, sizebuf);
		usedefault = 1;
		return (fdopen(data, mode));
	}
	if (usedefault)
		data_dest = his_addr;
	usedefault = 1;
	file = getdatasock(mode);
	if (file == NULL) {
		reply(425, "Can't create data socket (%s,%d): %s.",
		    inet_ntoa(data_source.sin_addr),
		    ntohs(data_source.sin_port), strerror(errno));
		return (NULL);
	}
	data = fileno(file);
	while (connect(data, (struct sockaddr *)&data_dest,
	    sizeof (data_dest)) < 0) {
		if (errno == EADDRINUSE && retry < swaitmax) {
			sleep((unsigned) swaitint);
			retry += swaitint;
			continue;
		}
		perror_reply(425, "Can't build data connection");
		(void) fclose(file);
		data = -1;
		return (NULL);
	}
	reply(150, "Opening %s mode data connection for %s%s.",
	     type == TYPE_A ? "ASCII" : "BINARY", name, sizebuf);
	return (file);
}

/*
 * Tranfer the contents of "instr" to
 * "outstr" peer using the appropriate
 * encapsulation of the data subject
 * to Mode, Structure, and Type.
 *
 * NB: Form isn't handled.
 */
send_data(instr, outstr, blksize)
	FILE *instr, *outstr;
	off_t blksize;
{
	register int c, cnt;
	register char *buf;
	int netfd, filefd;

	transerr=1;
	transflag++;
	if (setjmp(urgcatch)) {
	        transerr--;
		transflag = 0;
		return;
	}
	switch (type) {

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			if (c == '\n') {
				if (ferror(outstr))
					goto data_err;
				(void) putc('\r', outstr);
			}
			(void) putc(c, outstr);
		}
		fflush(outstr);
		transflag = 0;
		if (ferror(instr))
			goto file_err;
		if (ferror(outstr))
			goto data_err;
		reply(226, "Transfer complete.");
		return;

	case TYPE_I:
	case TYPE_L:
		if ((buf = malloc((u_int)blksize)) == NULL) {
			transflag = 0;
			transerr--;
			perror_reply(451, "Local resource failure: malloc");
			return;
		}
		netfd = fileno(outstr);
		filefd = fileno(instr);
		while ((cnt = read(filefd, buf, (u_int)blksize)) > 0 &&
		    write(netfd, buf, cnt) == cnt)
			byte_count += cnt;
		transflag = 0;
		(void)free(buf);
		if (cnt != 0) {
			if (cnt < 0)
				goto file_err;
			goto data_err;
		}
		reply(226, "Transfer complete.");
		return;
	default:
		transerr--;
		transflag = 0;
		reply(550, "Unimplemented TYPE %d in send_data", type);
		return;
	}

data_err:
	transflag = 0;
	transerr--;
	perror_reply(426, "Data connection");
	return;

file_err:
	transflag = 0;
	transerr--;
	perror_reply(551, "Error on input file");
}

/*
 * Transfer data from peer to
 * "outstr" using the appropriate
 * encapulation of the data subject
 * to Mode, Structure, and Type.
 *
 * N.B.: Form isn't handled.
 */
receive_data(instr, outstr)
	FILE *instr, *outstr;
{
	register int c;
	int cnt, bare_lfs = 0;
	char buf[BUFSIZ];

	transerr=1;
	transflag++;
	if (setjmp(urgcatch)) {
	        transerr--;
		transflag = 0;
		return (-1);
	}
	switch (type) {

	case TYPE_I:
	case TYPE_L:
		while ((cnt = read(fileno(instr), buf, sizeof buf)) > 0) {
			if (write(fileno(outstr), buf, cnt) != cnt)
				goto file_err;
			byte_count += cnt;
		}
		if (cnt < 0)
			goto data_err;
		transflag = 0;
		return (0);

	case TYPE_E:
		reply(553, "TYPE E not implemented.");
		transerr--;
		transflag = 0;
		return (-1);

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			if (c == '\n')
				bare_lfs++;
			while (c == '\r') {
				if (ferror(outstr))
					goto data_err;
				if ((c = getc(instr)) != '\n') {
					(void) putc ('\r', outstr);
					if (c == '\0' || c == EOF)
						goto contin2;
				}
			}
			(void) putc(c, outstr);
	contin2:	;
		}
		fflush(outstr);
		if (ferror(instr))
			goto data_err;
		if (ferror(outstr))
			goto file_err;
		transflag = 0;
		if (bare_lfs) {
			lreply(230, "WARNING! %d bare linefeeds received in ASCII mode", bare_lfs);
#ifdef NOTDEF
			printf("   File may not have transferred correctly.\r\n");
#else
			fputs("   File may not have transferred correctly.\r\n", stdout);
#endif
		}
		return (0);
	default:
		reply(550, "Unimplemented TYPE %d in receive_data", type);
		transerr--;
		transflag = 0;
		return (-1);
	}

data_err:
	transflag = 0;
	transerr--;
	perror_reply(426, "Data Connection");
	return (-1);

file_err:
	transflag = 0;
	transerr--;
	perror_reply(452, "Error writing file");
	return (-1);
}

statfilecmd(filename)
	char *filename;
{
	char line[BUFSIZ];
	FILE *fin;
	int c;

	(void) sprintf(line, "/bin/ls -lgA %s", filename);
	fin = ftpd_popen(line, "r");
	lreply(211, "status of %s:", filename);
	while ((c = getc(fin)) != EOF) {
		if (c == '\n') {
			if (ferror(stdout)){
				perror_reply(421, "control connection");
				(void) ftpd_pclose(fin);
				dologout(1);
				/* NOTREACHED */
			}
			if (ferror(fin)) {
				perror_reply(551, filename);
				(void) ftpd_pclose(fin);
				return;
			}
			(void) putc('\r', stdout);
		}
		(void) putc(c, stdout);
	}
	(void) ftpd_pclose(fin);
	reply(211, "End of Status");
}

statcmd()
{
	struct sockaddr_in *sin;
	u_char *a, *p;

	lreply(211, "%s FTP server status:", hostname, version);
	printf("     %s\r\n", version);
	printf("     Connected to %s", remotehost);
	if (!isdigit(remotehost[0]))
		printf(" (%s)", inet_ntoa(his_addr.sin_addr));
#ifdef NOTDEF
	printf("\r\n");
#else
	fputs("\r\n", stdout);
#endif
	if (logged_in) {
		if (guest)
#ifdef NOTDEF
			printf("     Logged in anonymously\r\n");
#else
			fputs("     Logged in anonymously\r\n", stdout);
#endif
		else
			printf("     Logged in as %s\r\n", pw->pw_name);
	} else if (askpasswd)
#ifdef NOTDEF
		printf("     Waiting for password\r\n");
#else
		fputs("     Waiting for password\r\n", stdout);
#endif
	else
#ifdef NOTDEF
		printf("     Waiting for user name\r\n");
#else
		fputs("     Waiting for user name\r\n", stdout);
#endif
	printf("     TYPE: %s", typenames[type]);
	if (type == TYPE_A || type == TYPE_E)
		printf(", FORM: %s", formnames[form]);
	if (type == TYPE_L)
#if NBBY == 8
		printf(" %d", NBBY);
#else
		printf(" %d", bytesize);	/* need definition! */
#endif
	printf("; STRUcture: %s; transfer MODE: %s\r\n",
	    strunames[stru], modenames[mode]);
	if (data != -1)
#ifdef NOTDEF
		printf("     Data connection open\r\n");
#else
		fputs("     Data connection open\r\n", stdout);
#endif
	else if (pdata != -1) {
#ifdef NOTDEF
		printf("     in Passive mode");
#else
		fputs("     in Passive mode", stdout);
#endif
		sin = &pasv_addr;
		goto printaddr;
	} else if (usedefault == 0) {
#ifdef NOTDEF
		printf("     PORT");
#else
		fputs("     PORT", stdout);
#endif
		sin = &data_dest;
printaddr:
		a = (u_char *) &sin->sin_addr;
		p = (u_char *) &sin->sin_port;
#define UC(b) (((int) b) & 0xff)
		printf(" (%d,%d,%d,%d,%d,%d)\r\n", UC(a[0]),
			UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
#undef UC
	} else
#ifdef NOTDEF
		printf("     No data connection\r\n");
#else
		fputs("     No data connection\r\n", stdout);
#endif
	reply(211, "End of status");
}

fatal(s)
	char *s;
{
	reply(451, "Error in server: %s\n", s);
	reply(221, "Closing connection due to server error.");
	dologout(0);
	/* NOTREACHED */
}

/* VARARGS2 */
reply(n, fmt, p0, p1, p2, p3, p4, p5)
	int n;
	char *fmt;
{
#if 1
	printf("%d ", n);
	printf(fmt, p0, p1, p2, p3, p4, p5);
#ifdef NOTDEF
	printf("\r\n");
#else
	fputs("\r\n", stdout);
#endif
#else
	char mybuf[256];

	sprintf(mybuf,"%d ",n);
	sprintf(mybuf+4,fmt,p0,p1,p2,p3,p4,p5);
	strcat(mybuf,"\r\n");
	fputs(mybuf,stdout);
#endif
	(void)fflush(stdout);
	if (debug) {
		syslog(LOG_DEBUG, "<--- %d ", n);
		syslog(LOG_DEBUG, fmt, p0, p1, p2, p3, p4, p5);
	}
}

/* VARARGS2 */
lreply(n, fmt, p0, p1, p2, p3, p4, p5)
	int n;
	char *fmt;
{
	printf("%d- ", n);
	printf(fmt, p0, p1, p2, p3, p4, p5);
#ifdef NOTDEF
	printf("\r\n");
#else
	fputs("\r\n", stdout);
#endif
	(void)fflush(stdout);
	if (debug) {
		syslog(LOG_DEBUG, "<--- %d- ", n);
		syslog(LOG_DEBUG, fmt, p0, p1, p2, p3, p4, p5);
	}
}

ack(s)
	char *s;
{
	reply(250, "%s command successful.", s);
}

nack(s)
	char *s;
{
	reply(502, "%s command not implemented.", s);
}

/* ARGSUSED */
yyerror(s)
	char *s;
{
	char *cp;

	if (cp = index(cbuf,'\n'))
		*cp = '\0';
	reply(500, "'%s': command not understood.", cbuf);
}

delete(name)
	char *name;
{
	struct stat st;

	if (stat(name, &st) < 0) {
		perror_reply(550, name);
		return;
	}
	if ((st.st_mode&S_IFMT) == S_IFDIR) {
		if (rmdir(name) < 0) {
			perror_reply(550, name);
			return;
		}
		goto done;
	}
	if (unlink(name) < 0) {
		perror_reply(550, name);
		return;
	}
done:
	misclog("DELE",name);
	ack("DELE");
}

cwd(path)
	char *path;
{
	if (chdir(path) < 0)
		perror_reply(550, path);
	else
		ack("CWD");
}

makedir(name)
	char *name;
{
	if (mkdir(name, 0777) < 0)
		perror_reply(550, name);
	else
		misclog("MKD",name);
	        reply(257, "MKD command successful.");
}

removedir(name)
	char *name;
{
	if (rmdir(name) < 0)
		perror_reply(550, name);
	else
	        misclog("RMD",name);
		ack("RMD");
}

pwd()
{
	char path[MAXPATHLEN + 1];
	extern char *getwd();

	if (getwd(path) == (char *)NULL)
		reply(550, "%s.", path);
	else
		reply(257, "\"%s\" is current directory.", path);
}

char *
renamefrom(name)
	char *name;
{
	struct stat st;

	if (stat(name, &st) < 0) {
		perror_reply(550, name);
		return ((char *)0);
	}
	reply(350, "File exists, ready for destination name");
	return (name);
}

renamecmd(from, to)
	char *from, *to;
{
        char buf[1024];
	if (rename(from, to) < 0)
		perror_reply(550, "rename");
	else
	        sprintf(buf,"%s -> %s",from,to);
	        misclog("RNTO",buf);
		ack("RNTO");
}

dolog(sin)
	struct sockaddr_in *sin;
{
	struct hostent *hp = gethostbyaddr((char *)&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);
	time_t t, time();
	extern char *ctime();

	if (hp)
		(void) strncpy(remotehost, hp->h_name, sizeof (remotehost));
	else
		(void) strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
#ifdef SETPROCTITLE
	sprintf(proctitle, "%s: connected", remotehost);
	setproctitle(proctitle);
#endif /* SETPROCTITLE */

	if (logging) {
		t = time((time_t *) 0);
		syslog(LOG_INFO, "connection from %s at %s",
		    remotehost, ctime(&t));
	}
}

/*
 * Record logout in wtmp file
 * and exit with supplied status.
 */
dologout(status)
	int status;
{
	if (logged_in) 
	  {
	    (void) seteuid((uid_t)0);
	    logwtmp(ttyline, "", "");
	    loglogof();
	  }
	/* beware of flushing buffers after a SIGPIPE */
	_exit(status);
}

myoob()
{
	char *cp;

	/* only process if transfer occurring */
	if (!transflag)
		return;
	cp = tmpline;
	if (getline(cp, 7, stdin) == NULL) {
		reply(221, "You could at least say goodbye.");
		dologout(0);
	}
	upper(cp);
	if (strcmp(cp, "ABOR\r\n") == 0) {
		tmpline[0] = '\0';
		transerr--;
		reply(426, "Transfer aborted. Data connection closed.");
		reply(226, "Abort successful");
		longjmp(urgcatch, 1);
	}
	if (strcmp(cp, "STAT\r\n") == 0) {
		if (file_size != (off_t) -1)
			reply(213, "Status: %lu of %lu bytes transferred",
			    byte_count, file_size);
		else
			reply(213, "Status: %lu bytes transferred", byte_count);
	}
}

/*
 * Note: a response of 425 is not mentioned as a possible response to
 * 	the PASV command in RFC959. However, it has been blessed as
 * 	a legitimate response by Jon Postel in a telephone conversation
 *	with Rick Adams on 25 Jan 89.
 */
passive()
{
	int len;
	register char *p, *a;

	pdata = socket(AF_INET, SOCK_STREAM, 0);
	if (pdata < 0) {
		perror_reply(425, "Can't open passive connection");
		return;
	}
	pasv_addr = ctrl_addr;
	pasv_addr.sin_port = 0;
	(void) seteuid((uid_t)0);
	if (bind(pdata, (struct sockaddr *)&pasv_addr, sizeof(pasv_addr)) < 0) {
		(void) seteuid((uid_t)pw->pw_uid);
		goto pasv_error;
	}
	(void) seteuid((uid_t)pw->pw_uid);
	len = sizeof(pasv_addr);
	if (getsockname(pdata, (struct sockaddr *) &pasv_addr, &len) < 0)
		goto pasv_error;
	if (listen(pdata, 1) < 0)
		goto pasv_error;
	a = (char *) &pasv_addr.sin_addr;
	p = (char *) &pasv_addr.sin_port;

#define UC(b) (((int) b) & 0xff)

	reply(227, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", UC(a[0]),
		UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
	return;

pasv_error:
	(void) close(pdata);
	pdata = -1;
	perror_reply(425, "Can't open passive connection");
	return;
}

/*
 * Generate unique name for file with basename "local".
 * The file named "local" is already known to exist.
 * Generates failure reply on error.
 */
char *
gunique(local)
	char *local;
{
	static char new[MAXPATHLEN];
	struct stat st;
	char *cp = rindex(local, '/');
	int count = 0;

	if (cp)
		*cp = '\0';
	if (stat(cp ? local : ".", &st) < 0) {
		perror_reply(553, cp ? local : ".");
		return((char *) 0);
	}
	if (cp)
		*cp = '/';
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	for (count = 1; count < 100; count++) {
		(void) sprintf(cp, "%d", count);
		if (stat(new, &st) < 0)
			return(new);
	}
	reply(452, "Unique file name cannot be created.");
	return((char *) 0);
}

/*
 * Format and send reply containing system error number.
 */
perror_reply(code, string)
	int code;
	char *string;
{
	reply(code, "%s: %s.", string, strerror(errno));
}

static char *onefile[] = {
	"",
	0
};

send_file_list(whichfiles)
	char *whichfiles;
{
	struct stat st;
	DIR *dirp = NULL;
	struct direct *dir;
	FILE *dout = NULL;
	register char **dirlist, *dirname;
	int simple = 0;
	char *strpbrk();

	if (strpbrk(whichfiles, "~{[*?") != NULL) {
		extern char **glob(), *globerr;

		globerr = NULL;
		dirlist = glob(whichfiles);
		if (globerr != NULL) {
			reply(550, globerr);
			return;
		} else if (dirlist == NULL) {
			errno = ENOENT;
			perror_reply(550, whichfiles);
			return;
		}
	} else {
		onefile[0] = whichfiles;
		dirlist = onefile;
		simple = 1;
	}

	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}
	while (dirname = *dirlist++) {
		if (stat(dirname, &st) < 0) {
			/*
			 * If user typed "ls -l", etc, and the client
			 * used NLST, do what the user meant.
			 */
			if (dirname[0] == '-' && *dirlist == NULL &&
			    transflag == 0) {
				retrieve("/bin/ls %s", dirname);
				return;
			}
			perror_reply(550, whichfiles);
			if (dout != NULL) {
				(void) fclose(dout);
				transflag = 0;
				data = -1;
				pdata = -1;
			}
			return;
		}

		if ((st.st_mode&S_IFMT) == S_IFREG) {
			if (dout == NULL) {
				dout = dataconn("file list", (off_t)-1, "w");
				if (dout == NULL)
					return;
				transflag++;
			}
			fprintf(dout, "%s%s\n", dirname,
				type == TYPE_A ? "\r" : "");
			byte_count += strlen(dirname) + 1;
			continue;
		} else if ((st.st_mode&S_IFMT) != S_IFDIR)
			continue;

		if ((dirp = opendir(dirname)) == NULL)
			continue;

		while ((dir = readdir(dirp)) != NULL) {
			char nbuf[MAXPATHLEN];

			if (dir->d_name[0] == '.' && dir->d_namlen == 1)
				continue;
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' &&
			    dir->d_namlen == 2)
				continue;

			sprintf(nbuf, "%s/%s", dirname, dir->d_name);

			/*
			 * We have to do a stat to insure it's
			 * not a directory or special file.
			 */
			if (simple || (stat(nbuf, &st) == 0 &&
			    (st.st_mode&S_IFMT) == S_IFREG)) {
				if (dout == NULL) {
					dout = dataconn("file list", (off_t)-1,
						"w");
					if (dout == NULL)
						return;
					transflag++;
				}
				if (nbuf[0] == '.' && nbuf[1] == '/')
					fprintf(dout, "%s%s\n", &nbuf[2],
						type == TYPE_A ? "\r" : "");
				else
					fprintf(dout, "%s%s\n", nbuf,
						type == TYPE_A ? "\r" : "");
				byte_count += strlen(nbuf) + 1;
			}
		}
		(void) closedir(dirp);
	}

	if (dout == NULL)
		reply(550, "No files found.");
	else if (ferror(dout) != 0)
		perror_reply(550, "Data connection");
	else
		reply(226, "Transfer complete.");

	transflag = 0;
	if (dout != NULL)
		(void) fclose(dout);
	data = -1;
	pdata = -1;
}

#ifdef SETPROCTITLE
/*
 * clobber argv so ps will show what we're doing.
 * (stolen from sendmail)
 * warning, since this is usually started from inetd.conf, it
 * often doesn't have much of an environment or arglist to overwrite.
 */

/*VARARGS1*/
setproctitle(fmt, a, b, c)
char *fmt;
{
	register char *p, *bp, ch;
	register int i;
	char buf[BUFSIZ];

	(void) sprintf(buf, fmt, a, b, c);

	/* make ps print our process name */
	p = Argv[0];
	*p++ = '-';

	i = strlen(buf);
	if (i > LastArgv - p - 2) {
		i = LastArgv - p - 2;
		buf[i] = '\0';
	}
	bp = buf;
	while (ch = *bp++)
		if (ch != '\n' && ch != '\r')
			*p++ = ch;
	while (p < LastArgv)
		*p++ = ' ';
}
#endif /* SETPROCTITLE */


logxfer(id,name,tim,len)
        long len,tim;
	char *name, *id;
{
	char buf[1024];
	char path[MAXPATHLEN + 1];
	long now;
	double byte_per_sec;

	if (logfd >= 0 && getwd(path) != NULL) {
		time(&now);
		if (transerr==1)
		  {
		    if ( now-tim > 2 )
		      {
				byte_per_sec = 
				  (float)len/((float)(now-tim))/1000.0;
			sprintf(buf, "%d:%s: %d sec, %d byte, %5.1lf Kb/sec, %s/%s\n",
				ftpdpid, id, 
				now-tim, len, 
				byte_per_sec,
				path, name);
		      }
		    else
		      {
			sprintf(buf, "%d:%s: %d byte, %s/%s\n",
				ftpdpid, id, 
				len, 
				path, name);
		      }
		    if (!strcmp("GET",id))
		      {
			get_count++;
			get_time=get_time+(now-tim);
			get_len=get_len+len;
		      }
		    if (!strcmp("PUT",id))
		      {
			put_count++;
			put_time=put_time+(now-tim);
			put_len=put_len+len;
		      }
		  }
		else
		  {
		    sprintf(buf, "%d:%s: TRANSFER ABORTED, %s/%s\n",
			    ftpdpid, id, 
			    path, name);
		    ab_count++;
		  }
		write(logfd, buf, strlen(buf));
	}
}

logident(ident)
        char *ident;
{
	char buf[1024],*st;
	char *myuser;
	long now;
	int i;

	time(&now);
	st=ctime(&now);
	for (i=11;i<=15;i++) starttime[i-11]=st[i];
	starttime[5]=0;

	if(special_guest) {
		myuser = special_user;
		ident = email_addr;
	} else {
		myuser = "anonymous";
	}
	if (logfd >= 0) {
	  sprintf(buf, "%d: %s %s Connected at %.20s\n%d:IDENT: '%s'\n", 
		  ftpdpid, remotehost, myuser, ctime(&now)+4, ftpdpid, ident); 
	  write(logfd, buf, strlen(buf));
	}
}

loglogof()
{
	char buf[1024],today[8],*st;
	long now,h,m,s;
	int i;

	time(&now);
	st=ctime(&now);
	for (i=4;i<=9;i++) today[i-4]=st[i];
	today[6]=0;
	for (i=11;i<=15;i++) endtime[i-11]=st[i];
	endtime[5]=0;

	if (logfd >= 0) 
	  {
	    h=(now-contime)/3600;
	    m=(now-contime-(h*3600))/60;
	    s=(now-contime-(h*3600)-(m*60));
	    sprintf(buf, "%d: connection closed at %.20s, time: %02d:%02d:%02d \n", 
		    ftpdpid, ctime(&now)+4,h,m,s);
	    write(logfd, buf, strlen(buf));
	  }

	if (statfd >=0)
	  {
	    for (i=0;i<33;i++) hostbuffer[i]=remotehost[i];
	    if (strcmp(endtime,starttime))
	      {
		sprintf(buf," %-33s %s %s-%s %2d %2d %2d ",
			hostbuffer,today,starttime,endtime,
			misclogs,ab_count,put_count);
		write(statfd, buf, strlen(buf));
	      }
	    else
	      {
		sprintf(buf," %-33s %s %s       %2d %2d %2d ",
			hostbuffer,today,starttime,
			misclogs,ab_count,put_count);
		write(statfd, buf, strlen(buf));
	      }
	    
	    if (get_count>0)
	      {
		if (get_time>2)
		  {
		    sprintf(buf, "%2d %7.1f %5.1f ",
			    get_count,((float)get_len/1000),
			    ((float)get_len/(float)get_time/1000.0));
		    write(statfd, buf, strlen(buf));
		  }
		else
		  {
		    sprintf(buf, "%2d %7.1f     - ",
			    get_count,((float)get_len/1000));
		    write(statfd, buf, strlen(buf));
		  }
	      }
	    else
	      {
		sprintf(buf, " 0       -     - ");
		write(statfd, buf, strlen(buf));
	      }

	    sprintf(buf, "\n");
	    write(statfd, buf, strlen(buf));
	  }
}

misclog(mode,name)
        char *mode,*name;
{
	char buf[1024];
	misclogs++;
	if (logfd >= 0) {
	  sprintf(buf, "%d:%s: %s\n", 
		  ftpdpid, mode, name); 
	  write(logfd, buf, strlen(buf));
	}
}

/* Additions by MRR */
/*--- function match ----------------------------------------------
 *
 *  Determine whether two character strings match.  Case insensitive.
 *
 *  Entry:  str      is a string.
 *          pattern  is a pattern to which we are comparing.
 *
 *  Exit:   Returns TRUE iff the strings match.
 */
int
match(str,pattern)
char *str;
char *pattern;
{
#define TRUE 1
#define FALSE 0

   char ch1, ch2;

   do {
      ch1 = (char) (islower(*str) ? toupper(*str) : *str);
      ch2 = (char) (islower(*pattern) ? toupper(*pattern) : *pattern);
      if(ch1 != ch2) return FALSE;
      str++; pattern++;
   } while(ch1 == ch2 && ch1 && ch2);

   if(!ch1 && !ch2) {
      return TRUE;
   } else {
      return FALSE;
   }
}

/*--- function checkforeign ---------------------------------------------
 *
 *  Check whether a given host name is foreign or domestic.
 *
 *  Entry	host	is a zero-terminated character string--a host 
 *						name.
 *
 *  Exit		Returns 1 if foreign, 0 if domestic.
 */
int
checkforeign(host)
char *host;
{
	char *domain;
	static char *domestic_doms[] =
	 {".com",".mil",".edu",".org",".ca",".gov",".us",NULL};
	static char *domestic_sites[] = {
	  "nic.sura.net", "nis.ans.net",
	  NULL};
	char **tptr = domestic_doms;
	char **siteptr = domestic_sites;

	/* Check for special cases. */
	while(*siteptr) {
		if(strcmp(host,*siteptr)==0) return 0;
		siteptr++;
	}

	/* Check for certain domains (characters after last '.') */
	if(domain = strrchr(host,'.')) {
		while(*tptr) {
			if(match(domain,*tptr)) return 0;
			tptr++;
		}
	}
	return 1;
}

/*--- function CheckSpecialGuest ------------------------------------
 *
 *  Authenticate a special secure FTP user.
 *  These are users with names of form "guest.xxxxx"; they have their
 *  own authorization file.  Each user is associated with one remote
 *  host, and they must FTP only from this host.
 *
 *  Entry:	username		Username of form guest.xxxxx
 *				passwd		password
 *				remotehost	remote host name.
 *
 *	 Exit:	Returns 1 if OK, else 0.
 *				Writes an error message if user not authorized.
 */
int
CheckSpecialGuest(username,passwd,remotehost)
char *username;
char *passwd;
char *remotehost;
{
	FILE *authstream;
	int valid = 0, j;
	char *errmsg, *cptr;
#define LINELEN 400
#define LEN_USER 8
	char line[LINELEN];
	char buf[LINELEN];
	int argc;
	char **argv=(char **) NULL;
	int reading=1;
	
	NormalizeString(username);
	NormalizeString(passwd);
	NormalizeString(remotehost);
	authstream = fopen(FTP_GUEST_FILE,"r");
	if(!authstream) {
		errmsg = "Can't access secure FTP user authorization file";
		goto endspecial;
	}
	/* Scan FTP user auth file, looking for a line that matches
    * this username.  Username was typed as "guest.xxxxxxxx";
	 * we are just looking to match the xxxxxxxx.
	 */
	while(reading && fgets(line,LINELEN,authstream)) {
		/* Put the FTP user name from this line in the file in "buf" */
		for(j=0; isalnum(line[j]); j++) buf[j] = line[j];
		buf[j] = '\0';
		if(strcmp(username+6,buf)==0) {
			for(cptr=line; *cptr; cptr++) if(*cptr=='\n') *cptr='\0';
			argc = parsit(line,&argv);
			if(strcmp(argv[1],passwd) != 0) {
				errmsg = "Incorrect password.";
				goto endspecial;
			}
			if(!HostnameMatch(remotehost,argv[2])) {
				sprintf(buf,"You must FTP from domain %s. (You are %s)",argv[2],remotehost);
				errmsg = buf;
				goto endspecial;
			}
			valid = 1;
			reading = 0;
			if(argc>3) strncpy(email_addr,argv[3],MAX_USER_NAME);
			else email_addr[0] = '\0';
			NormalizeString(email_addr);
		}
	}
	if(reading) {
		sprintf(buf,"Guest user %s unknown.",username);
		errmsg = buf;
	}
	
endspecial:;	
	if(!valid) {
		reply(530,errmsg);
	}
	if(authstream) fclose(authstream);
	
	return valid;
}

/*--- function HostnameComponents ------------------------------------
 *
 *  Calculate the number of components in the hostname portion
 *  of an Internet-style email address.
 *  E.g.,  bill@cs.bigu.edu  has 3 hostname components.
 *
 *	 Entry: user	is an email address.
 *
 *	 Exit:	 Returns number of components.
 */

int
HostnameComponents(addr)
char *addr;
{
	int ncomp = 0;
	char *cptr=addr;

	ncomp = 1;
	while(*cptr) if(*(cptr++)=='.') ncomp++;
	return ncomp;
}

/*--- function HostnameUpALevel -------------------------------------
 *
 *  Modify an Internet hostname to remove the first component.
 *  
 *  E.g., cs.bigu.edu --> bigu.edu
 *
 *	 Entry: addr	is a hostname.
 *
 *  Exit:	addr  has been modified.
 *				Returns TRUE if we were able to complete processing.
 */
int
HostnameUpALevel(addr)
char *addr;
{
	char *cptr, *targ;
	
	if(HostnameComponents(addr) < 3) return FALSE;

	targ = cptr = addr;
	for(; *cptr && *cptr!='.'; cptr++);  /* Skip past first level */
	for(cptr++; *cptr; ) *(targ++) = *(cptr++);  /* Copy rest */
	*targ = '\0';

	return TRUE;
}

/*--- function HostnameMatch --------------------------------------
 *
 *  Determine whether a given hostname matches a target hostname.
 *  of a user.  Simply requires that 
 *  the hostnames match, minus the first few
 *  components of the hostname.
 *  E.g., HostnameMatch("cs.bigu.edu","bigu.edu") returns TRUE;
 *
 *  Entry:	host			is the hostname we are testing.
 *				knownhost   is known host against which we are testing.
 *
 *	 Exit:   Returns TRUE iff the two correspond.
 */
int
HostnameMatch(user, candidate)
char *user;
char *candidate;
{
	char *user_copy;
	int trying=TRUE;
	int matchOK=FALSE;

	user_copy = malloc(strlen(user)+1);
	strcpy(user_copy,user);    /* We may modify user_copy below */
	
	/* Keep testing for a case-insensitive both otherwise exact
	 * lexicographical match while we remove successive leftmost
	 * components from the hostname of the candidate.
	 */
	do {
		if(strcasecmp(user_copy,candidate)==0) {
			trying = FALSE;
			matchOK = TRUE;
		} else {
			trying = HostnameUpALevel(user_copy);
		}
	} while(trying); 
	
	free(user_copy);
	return matchOK;
}

