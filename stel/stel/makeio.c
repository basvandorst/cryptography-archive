/*
 * $Author: vince $
 * $Header: /users/vince/src/stel-dist/stel/RCS/makeio.c,v 1.114 1996/04/03 08:40:32 vince Exp $
 * $Date: 1996/04/03 08:40:32 $
 * $Revision: 1.114 $
 * $Locker:  $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: makeio.c,v 1.114 1996/04/03 08:40:32 vince Exp $";
#endif	lint

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pwd.h>
#include "regex.h"

#ifdef HAS_SYS_PARAM_H
#include <sys/param.h>
#endif

#if defined(BSD)    /* pure BSD-based */
#ifndef SIGCLD
#define SIGCLD  SIGCHLD
#endif
#endif

#include "skey.h"
#include "defs.h"

#define ESCAPE_CMD	"escape"
#define SKEY_CMD	"skey"
#define SKEY_CMD_BIS	"s/key"
#define LOG_CMD		"log"
#define GET_CMD		"get"
#define PUT_CMD		"put"
#define min(x, y)	(x < y ? x  : y)

static int		child_is_dead;
static int		idle_timeout;

/* idea stuff */
#include "idea.h"
IDEAkey			s_ideakey;	/* server */
IDEAkey			c_ideakey;	/* client */

/* des stuff */
/* #include "des.h" */
des_key_schedule	s_schedule1;	/* server */
des_key_schedule	s_schedule2;	/* server */
des_key_schedule	s_schedule3;	/* server */
des_key_schedule	c_schedule1;	/* client */
des_key_schedule	c_schedule2;	/* client */
des_key_schedule	c_schedule3;	/* client */

/* control messages, file xfer integrity */
long	crc32val;

/* md5 stuff */
#include "global.h"
#include "md5.h"

#if defined (IRIX)
#define sigmask(n)	((unsigned int)1 << (((n) - 1) & (32 - 1)))
#endif
#define CONTROL(x)	((x) & 0x1f)		/* CTRL(x) is not portable */
static int		escape = CONTROL(']');
static long		oldmask;
static long		start;
static char		skeybuf[SKEYBUFSIZE];
static unsigned long	userstrokes, traffic, xfertraffic;

/* logging stuff */
static int		logging;
int			logfh; /* visible by stel.c */
static char		logfilename[256];

/* flags stuff. used for file transfer */
#ifdef COMBOFLAG
static int		comboflag = COMBO_FLG_DATA;
#endif /* COMBOFLAG */

static char		fromfile[256];
static char		tofile[256];
static int		userauthenticated;

char			*doskeymatch();

extern int		errno;
extern char		*sys_errlist[];

/* in skeyresp() */
static struct termios saved_ttymode, noecho_ttymode;

int
makeio(in, out, channel, authfh, side, firstack, remhostname, old, new,
						timeout, username)
int		in, out, channel, authfh;
int		firstack;
char		*remhostname, *username;
struct termios	*old, *new;
int		timeout;
{
	int		ret;
	int		mask;
	char		buf[MAKEIOBUF];
	void		deadchild();
	void		idletimeout();

	struct timeval	seltimer, *tim = NULL;
	start = time(NULL);

	crcgen(); /* init crc engime */

	/* block signals */
	(void)signal(SIGCLD, deadchild);
	(void)signal(SIGALRM, idletimeout);
#ifndef SOLARIS
	oldmask = sigblock(sigmask(SIGCLD) | sigmask(SIGALRM));
#endif

	/* init regexp */
	if (!(firstack & FLG_NO_ESCAPE))
		(void)doskeymatch(NULL, SKEYPATTERN);

	for (; ; ) {
		/* unblock signals */
#ifdef SOLARIS
		(void)sigrelse(SIGCLD);
		(void)sigrelse(SIGALRM);
#else
		(void)sigsetmask(oldmask);
#endif

		/* some signal occurred. may be that some data arrived
		   when executing critical (signals inhibited) code, so we
		   flush streams, by waiting one more second for more data */
		if (child_is_dead || idle_timeout) {
			seltimer.tv_sec = 1;
			seltimer.tv_usec = 0;
			tim = &seltimer;
		}

		mask = (1 << in) | (1 << channel);

		if (!userauthenticated &&
		    authfh >= 0 &&
		    firstack & FLG_ACTIVE_ATTACKS)
			mask |= (1 << authfh);

#ifdef SOLARIS
		switch (select(32, (fd_set *)&mask, 0, 0, tim))
#else
		switch (select(32, &mask, 0, 0, tim))
#endif
							{
			case	-1:
				if (errno == EINTR && child_is_dead) {
					return(0);
				}
				if (errno == EINTR && idle_timeout) {
					/* client side, notify user */
					if (side == CLIENT_SIDE)
						cprintf("session timeout\r\n");
					return(0);
				}
				syslog(LOGPRI, "select(): %m");
				return(1);
			case	0:
				/*
				printf("select times out\n");
				*/
				return(0);
		}

#ifdef SOLARIS
		(void)sighold(SIGCLD);
		(void)sighold(SIGALRM);
#else
		/* entering critical code, block signals */
		oldmask = sigblock(sigmask(SIGCLD));
#endif

		if (mask & (1 << in)) {
			ret = doinput(in, channel, buf, side, firstack,
						remhostname, old, new);

		}

		else if (mask & (1 << channel)) {
			ret = dochannel(channel, out, buf, side, firstack,
					remhostname, username, timeout);
		}

		else if (mask & (1 << authfh)) {
			char	authbuf[100];

			bzero(authbuf, sizeof(authbuf));
			if (read(authfh, authbuf, sizeof(authbuf)) !=
							strlen(USRAUTH) ||
			    memcmp(authbuf, USRAUTH, strlen(USRAUTH))) {
				syslog(LOGPRI, "bad authentication channel! bad msg: `%s'",
					authbuf);
				ret = 1;
			}
			if (close(authfh) == -1) {
				syslog(LOGPRI, "close(authfh): %m");
				ret = 1;
			}
			userauthenticated++;
			ret = -1;
		}

		/* clean ending */
		if (ret == 0)
			break;
		/* an error occurred */
		else if (ret == 1)
			return(1);
	}
	return(0);
}

void
deadchild()
{
#if defined(hpux) || defined(SOLARIS) || defined(IRIX) || defined(AIX) || defined(BSDI)
        int             wstatus;
#else
        union wait      wstatus;
#endif

#ifdef SOLARIS24
	(void)wait(NULL);
#else
        while (wait3(&wstatus, WNOHANG, NULL) > 0)
                ;
#endif
        child_is_dead++;
        (void)signal(SIGCLD, deadchild);

        return;
}

void
idletimeout()
{
#if defined(hpux) || defined(SOLARIS) || defined(IRIX) || defined(AIX) || defined(BSDI)
        int             wstatus;
#else
        union wait      wstatus;
#endif

#ifdef SOLARIS24
	(void)wait(NULL);
#else
        while (wait3(&wstatus, WNOHANG, NULL) > 0)
                ;
#endif
        idle_timeout++;
        (void)signal(SIGALRM, idletimeout);
        return;
}

doescape(remhostname, old, new, firstack, buf, len)
char		*remhostname, *buf;
struct termios *old, *new;
int		firstack;
int		*len;
{
	char	line[80];
	int	quit = 0;
	char	*skeyprompt;
	char	*control(), *skeyresp();

	/* turn into cooked mode */
	if (tcsetattr(0, TCSANOW, old) == -1) {
		perror("tcgetattr()");
		return(1);
	}

	(void)alarm(0);

	for (; ;) {
		printf("\n");
		cprintf("> ");

		if (fgets(line, sizeof(line), stdin) == NULL) {
			quit = 1; /* user hit ^D */
			break;
		}

		rip(line);

		/* return to remote */
		if (line[0] == '\0') {
			break;
		}

		/* return to remote */
		if (!strncmp(line, "return", strlen(line))) {
			break;
		}

		/* terminate connection */
		if (!strncmp(line, "close", strlen(line)) ||
		    !strncmp(line, "quit", strlen(line))) {
			quit = 1;
			break;
		}

		/* calculate skey response. skey arguments can be
		provided at the command line */
		if (!strncmp(line, SKEY_CMD, strlen(SKEY_CMD)) ||
		    !strncmp(line, SKEY_CMD_BIS, strlen(SKEY_CMD_BIS))) {
			char	*p = skeyresp(line, 0);

			if (p)
				cprintf("skey response: %s\n", p);
			continue;
		}

		/* automatic skey prompting */
		if (!strncmp(line, "autoskey", strlen(line))) {
			char	*p;

			skeyprompt = doskeymatch(skeybuf, NULL);
			if (!skeyprompt) {
				cprintf("no skey challenge found\r\n");
				break;
			}

			p = skeyresp(skeyprompt, 1);
			if (p == NULL) /* failed */
				return(0);
			*len = strlen(p) + 1;
			strcpy(buf, p);
			buf[strlen(p)] = '\n';
			buf[strlen(p) + 1] = '\0';

			quit = -1;
			break;
		}

		/* log stdout to file */
		if (!strncmp(line, LOG_CMD, strlen(LOG_CMD))) {
			if (logging) {
				if (close(logfh))
					perror(logfilename);
				logfh = 0;
				printf("Capture is OFF (%s)\n", logfilename);
				logging = 0;
			}
			else {
				char	*p;

				p = (char *)strtok(line +
						strlen(LOG_CMD), " \t");
				strcpy(logfilename, p ? p : tmpnam(NULL));
				if ((logfh = open(logfilename,
					O_WRONLY | O_CREAT, 0600)) == -1) {
					perror(logfilename);
					break;
				}
				printf("Capture is ON (%s)\n", logfilename);
				logging = 1;
			}

			break;
		}

#ifndef NOREMEXEC
		/* start xfer connection */
		if (firstack & FLG_ACTIVE_ATTACKS &&
		    (!strncmp(line, GET_CMD, strlen(GET_CMD)) ||
		    !strncmp(line, PUT_CMD, strlen(PUT_CMD)))) {
			char	*p, *q, *linep, mode;

			mode = !strncmp(line, GET_CMD, strlen(GET_CMD)) ?
				COMBO_FLG_GET : COMBO_FLG_PUT;

			linep = line + strlen(GET_CMD);
			if ((p = (char *)strtok(linep, " \t")) == NULL ||
			    (q = (char *)strtok(NULL, " \t")) == NULL) {
				if (mode == COMBO_FLG_GET)
					printf("usage: get remotefile localfile\n");
				else
					printf("usage: put localfile remotefile\n");
				continue;
			}

			/* set sources and destination files */
			bzero(fromfile, sizeof(fromfile));
			strncpy(fromfile, p, strlen(p));

			bzero(tofile, sizeof(tofile));
			strncpy(tofile, q, strlen(q));

			/* set buffer */
			strcpy(buf, XFERTOKEN);
			*len = strlen(buf);

			/* set flags */
			comboflag |= mode;

			quit = -1;
			break;
		}
#endif /* NOREMEXEC */

		/* display some parameters and information */
		if (!strncmp(line, "status", strlen(line))) {
			int hrs, min, sec;
			long elp;
			char *mode;

			elp = time(NULL) - start;
			hrs = (int)(elp / 3600L);
			min = (int)((elp -  (long)hrs * 3600L) / 60L);
			sec = (int)(elp - (long)hrs * 3600L - (long)min * 60L);

			if (firstack & FLG_NO_ENCRYPTION)
				mode = "DISABLED: THIS SESSION IS INSECURE!";
			else if (firstack & FLG_USE_IDEA)
				mode = "ENABLED (IDEA)";
			else if (firstack & FLG_USE_SINGLE)
				mode = "ENABLED (SINGLE DES)";
			else if (firstack & FLG_USE_RC4)
				mode = "ENABLED (RC4)";
			else
				mode = "ENABLED (TRIPLE DES)";

			printf("Connected to %s.\n", remhostname);
			printf("Connection time: %s", ctime(&start));
			printf("Elapsed time: %d h, %d m, %d s\n",
								hrs, min, sec);
			printf("Inbound traffic: %ld bytes\n", traffic);
			printf("Outbound traffic: %ld bytes\n", userstrokes);
			printf("File xfer traffic: %ld bytes\n", xfertraffic);
			printf("Logging to file: %s\n", 
				logging ? logfilename : "OFF");
			printf("Escape character is '%s'\n", control(escape));
			printf("Data encryption: %s\n", mode);
			printf("Active attacks protection: %s\n",
				firstack & FLG_ACTIVE_ATTACKS ?
				"ENABLED" : "DISABLED");
			printf("\n");

			break;
		}

		/* escape to shell */
		if (line[0] == '!') {
			char	*shell;

			if (line[1] == '\0') {
				if ((shell = (char *)getenv("SHELL")) == NULL)
					shell = "/bin/sh";
			}
			else
				shell = line + 1;
			
#ifdef SOLARIS
			(void)sigrelse(SIGCLD);
			(void)sigrelse(SIGALRM);
#else
			(void)sigsetmask(oldmask);
#endif
			(void)signal(SIGCLD, SIG_DFL);
			(void)signal(SIGALRM, SIG_DFL);

			if (system(shell) == -1)
				perror("system()");
			(void)signal(SIGCLD, deadchild);
			/* (void)signal(SIGALRM, idletimeout); */
#ifdef SOLARIS
			/* (void)sighold(SIGALRM); */
			(void)sighold(SIGCLD);
#else
			oldmask = sigblock(sigmask(SIGCLD) | sigmask(SIGALRM));
#endif
			break;
		}

		if (!strcmp(line, "z")) {
			(void)kill(0, SIGTSTP);
			break;
		}

		if (!strncmp(line, ESCAPE_CMD, strlen(ESCAPE_CMD))) {
			unsigned char	x;
			char		*escbuf;
			
			/* make escbuf point to first non command character */
			escbuf = line + strlen(ESCAPE_CMD);
			while (isspace(*escbuf))
				escbuf++;

			if (*escbuf) {
				x = *escbuf;
			}
			else {
				printf("Current escape character is: '%s'\n",
							control(escape));
				printf("Enter new escape character: ");
				(void)fflush(stdout);
				x = (unsigned char)getchar();
			}

			if (isspace(x))
				printf("escape character not set\n");
			else {
				escape = x;
				printf("escape character set to: %s\n",
							control(escape));
			}
			continue;
		}

		if (!strncmp(line, "sendesc", strlen(line))) {
			quit = -1;
			/* everything is in the buffer already */
			break;
		}

		if (!strcmp(line, "?")) {
			printf("Commands may be abbreviated.  * means: arguments supported\n\n");
			
			printf("close, quit, ^D	close current connection\n");
			printf("return, ^M	return to remote system\n");
			printf("skey, s/key *	generate skey response\n");
			printf("autoskey	generate and send skey response\n");
			printf("status		display operating parameters\n");
			printf("escape      *	set escape character\n");
			printf("sendesc		send escape character\n");
			printf("log 	    *	log to file\n");
#ifndef NOREMEXEC
			if (firstack & FLG_ACTIVE_ATTACKS) {
				printf("put	    *	upload a file\n");
				printf("get         *	download a file\n");
			}
#endif
			printf("!	    *	shell escape\n");
			printf("z		suspend telnet\n");
			printf("?		print help information\n");
			continue;
		}

		printf("?Invalid command. Try '?'\n");
	}

	/* return to raw mode */
	if (tcsetattr(0, TCSANOW, new) == -1) {
		perror("tcgetattr()");
		return(1);
	}
	return(quit);
}

char *
skeyresp(line, automjr)
char	*line;
int	automjr;
{
	char		*seed, *seqn, passwd[80];
	static char	buf[256];
	int		n;
	int		desmode = 0;
	char		key[8];
	void		sk_restore_ttymode();
	void		(*oldsig) ();

	cprintf(
"Reminder: do not use this while logged in via telnet or dial-in\n");

	if (*line == '[') /* skip trailing [ */
		line++;
	if (!strncmp(line, SKEY_CMD, strlen(SKEY_CMD)))
		line += strlen(SKEY_CMD); /* skip the first word */
	else if (!strncmp(line, SKEY_CMD_BIS, strlen(SKEY_CMD_BIS)))
		line += strlen(SKEY_CMD_BIS); /* skip the first word */
	seqn = (char *)strtok(line, ", \t[]"); /* check command line args */
	if (seqn == NULL) {
		cprintf("sequence number, seed: ");
		(void)fflush(stdout);
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			cprintf("EOF\n");
			return(NULL);
		}
		rip(buf);

		seqn = (char *)strtok(buf, ", \t[]");
		if (seqn == NULL) {
			cprintf("no input?\n");
			return(NULL);
		}
	}
	n = atoi(seqn);
	if (n < 0) {
		cprintf("%d not positive\n", n);
		return(NULL);
	}
	seed = (char *)strtok(NULL, " \t[]");
	if (seed == NULL) {
		cprintf("no seed provided?\n");
		return(NULL);
	}

	/* cprintf("[s/key %d %s]\n", n, seed); */
	cprintf("s/key %d %s      Enter secret password:", n, seed);

	/* ECHO OFF */
	if (tcgetattr(0, &saved_ttymode) == -1) {
		perror("tcgetattr()");
		return(NULL);
	}
	if ((oldsig = signal(SIGINT, SIG_IGN)) != SIG_IGN)
		(void)signal(SIGINT, sk_restore_ttymode);
	if (tcgetattr(0, &noecho_ttymode) == -1) {
		perror("tcgetattr()");
		return(NULL);
	}
	noecho_ttymode.c_lflag &= ~ECHO;
	if (tcsetattr(0, TCSANOW, &noecho_ttymode) == -1) {
		perror("tcsetattr()");
		return(NULL);
	}

	if (fgets(passwd, sizeof(passwd), stdin) == NULL) {
		cprintf("no password provided\n");
		return(NULL);
	}
	rip(passwd);

	/* ECHO ON */
	if (tcsetattr(0, TCSANOW, &saved_ttymode) == -1) {
		perror("tcsetattr()");
		return(NULL);
	}
	if (oldsig != SIG_IGN)
		(void)signal(SIGINT, oldsig);
	printf("\n");

	if ((char *)getenv("SKEYPADFILE") != NULL) {
		if (automjr) {
			cprintf("(using mjr DES padding mode)\n");
			cprintf("using skeypadfile %s\n",
							getenv("SKEYPADFILE"));
			desmode = 1;
		}
		else {
			cprintf("use mjr DES padding mode ? ");
			if (fgets(buf, sizeof(buf), stdin) == NULL) {
				cprintf("EOF\n");
				return(NULL);
			}
			rip(buf);
			if (buf[0] == 'y')
				desmode = 1;
		}
	}

	/* Crunch seed and password into starting key */
	if(desmode ? deskeycrunch(key, seed, passwd) :
		keycrunch(key,seed,passwd) != 0) {
		cprintf("key crunch failed\n");
		return(NULL);
	}

	while(n-- != 0)
		fff(key);
	btoe(buf, key);
	return(buf);
}

/* restore - restore terminal modes when user aborts command */
void sk_restore_ttymode()
{
	if (tcsetattr(0, TCSANOW, &saved_ttymode) == -1)
		perror("tcsetattr()");
	exit(1);
}


storeskeybuf(buf, len)
char	*buf;
int	len;
{
	static int	skeyoffset;
	static int	skeybufsize = sizeof(skeybuf);

	if (skeyoffset == 0) /* init buffer */
		bzero(skeybuf, skeybufsize);
	
	/* buffer fits into skeybuf, the simplest case */
	if (len + skeyoffset < skeybufsize) {
		memcpy(skeybuf + skeyoffset, buf, len);
		skeyoffset += len;
	}
	/* buffer does not fit into skeybuf, and in fact it is larger
	than skeybufsize, so a full shift is performed */
	else if (len >= skeybufsize) {
		char	 *p;

		/* get last skeybufsize bytes from buf */
		p = buf + (len - skeybufsize);
		memcpy(skeybuf, p, skeybufsize);
		/* set offset at end of buffer */
		skeyoffset = skeybufsize - 1;
	}
	/* buffer does not fit into skeybuf, but it is smaller than
	skeybufsize. Two partial shifts are performed */
	else if (len < skeybufsize) {
		int	diff;

		/* how many exceeding bytes? */
		diff = len - (skeybufsize - skeyoffset) + 1;
		/* first shift */
		memcpy(skeybuf, skeybuf + diff, skeyoffset - diff);
		/* second shift */
		memcpy(skeybuf + skeyoffset - diff, buf, len);
		/* set offset at end of buffer */
		skeyoffset = skeybufsize - 1;
	}
}

char *
doskeymatch(buf, pat)
char	*buf, *pat;
{
	static regex_t		storedre;
	regex_t			re;
	regmatch_t		pm;
	static char		prompt[128];
	char			*pbuf, *rbuf, result[128];
	int			found = 0;

	if (pat) {
		/* it seems that re is somewhat corrupred by regexec().
		so we store it in a safe place and use a temporary copy */
		(void)regcomp(&storedre, pat, REG_EXTENDED);
		return(NULL);
	}

	bzero(result, sizeof(result));
	bzero(&pm, sizeof(pm));
	(void)memcpy(&re, &storedre, sizeof(re));

	/* skip all matches but last one */
	pbuf = buf;
	while (!regexec(&re, pbuf, 1, &pm, 0)) {
		if (sizeof(result) < pm.rm_eo - pm.rm_so + 2) /* overflow */
			continue;
		strncpy(result, pbuf + pm.rm_so, pm.rm_eo - pm.rm_so);
		result[pm.rm_eo - pm.rm_so] = '\0';
		found++;
		pbuf += pm.rm_eo;
	}

	if (!found)
		return(NULL);

	/* remove all \n\r */
	rbuf = result;
	pbuf = prompt;
	while (*rbuf) {
		if (*rbuf != '\n' && *rbuf != '\r')
			*pbuf++ = *rbuf;
		rbuf++;
	}
	*pbuf = '\0';

	return (prompt);
}

doinput(in, channel, buf, side, firstack, remhostname, old, new)
int		in, channel;
int		side, firstack;
char		*buf, *remhostname;
struct 		termios	*old, *new;
{
	static unsigned long	sequence;
	/*
	register		side;
	*/
	static			enc, cipher, cipherinit;
	int			len;

	/*
	side = remhostname ? CLIENT_SIDE : SERVER_SIDE;
	*/
	if (!cipherinit++) {
		if (!(firstack & FLG_NO_ENCRYPTION))
			enc = DES_ENCRYPT;
		else
			enc = -1;

		cipher = 0; /* use single des by default */
		if (firstack & FLG_USE_SINGLE)
			cipher = DES_CIPHER;
		else if (firstack & FLG_USE_IDEA)
			cipher = IDEA_CIPHER;
		else if (firstack & FLG_USE_RC4)
			cipher = RC4_CIPHER;
		else
			cipher = TRIPLEDES_CIPHER;
	}

	if ((len = read(in, buf, MAKEIOBUF)) == -1) {
/* #if defined(SUNOS4) || defined(linux) */
		if (errno == EIO) /* sun EOF */
			return(0);
			
/* #endif */
		syslog(LOGPRI, "read(): %m");
		return(1);
	}
	if (!len)
		return(0);

	if (side == CLIENT_SIDE &&
	    len == 1 && /* prevent accidental triggering */
	    (buf[0] == escape) && !(firstack & FLG_NO_ESCAPE)) {
		int	ret;

		ret = doescape(remhostname, old, new, firstack, buf, &len);
		if (ret == 1) { /* terminate connection */
			return(0);
		}

		cprintf("Returning to remote\r\n");
		if (ret == 0) { /* cont connection */
			return(-1);
		}
		/* if ret == -1 fall trought, so buf is
		passed on to the server */
	}

	userstrokes += len;

	/* check integrity and ordering */
	if (firstack & FLG_ACTIVE_ATTACKS) {
		struct dataset {
			unsigned long	seq;
#ifdef COMBOFLAG
			int		flag;
#endif /* COMBOFLAG */
			long		crc32;
			int		len;
		} set;

		crc32val = 0L;
		UpdateCRC(buf, len);
		sequence++;

		/* set it */
		set.seq = htonl(sequence);
#ifdef COMBOFLAG
		set.flag = htonl(comboflag);
#endif /* COMBOFLAG */
		set.crc32 = htonl(crc32val);
		set.len = htonl(len);

		/*
		{
			char	tmp[MAKEIOBUF * 2];

			bzero(tmp, sizeof(buf));
			memcpy(tmp, buf, len);
			syslog(LOGPRI, "SENDING data = ``%s''", buf);
		}
		*/

		/* encrypt it */
		cryptbuf(&set, sizeof(set), enc, cipher, side);
		cryptbuf(buf, len, enc, cipher, side);

		/* send it.
		eventually, send put/get commands
		*/
		if (write_data(channel, &set, sizeof(set)) != sizeof(set) ||
		    write_data(channel, buf, len) != len) {
			syslog(LOGPRI, "write_data(dataset+buf)");
			return(1);
		}

#ifndef NOREMEXEC
		/* perform GET command */
		if (firstack & FLG_ACTIVE_ATTACKS &&
		    (comboflag & COMBO_FLG_GET || comboflag & COMBO_FLG_PUT)) {
			int	ret;
			ret = doclientftp(channel, fromfile, tofile,
						enc, cipher, comboflag);
			if (ret) {
				cprintf("File xfer failed\r\n");
				if (ret == 1)
					return(1);
			}
			comboflag = COMBO_FLG_DATA;
		}
#endif /* NOREMEXEC */

	}
	/**/ 
	else {
		/* encrypt data buffer */
		cryptbuf(buf, len, enc, cipher, side);

		if (write_data(channel, buf, len) != len) {
			syslog(LOGPRI, "write_data(net): %m");
			return(1);
		}
	}

	return(-1);
}

dochannel(channel, out, buf, side, firstack, remhostname, username, timeout)
int	channel, out;
char	*buf;
int	side, firstack;
char	*remhostname, *username;
int	timeout;
{
	combopacket		**p;
	combopacket		**extract();
	static unsigned long	sequence;
	register		i, len, iside;
	static			dec, cipher, cipherinit;

	/* INVERTED, for encryption issues */
	iside = side == CLIENT_SIDE ? SERVER_SIDE : CLIENT_SIDE;

	if (!cipherinit++) {
		if (!(firstack & FLG_NO_ENCRYPTION))
			dec = DES_DECRYPT;
		else
			dec = -1;

		if (firstack & FLG_USE_SINGLE)
			cipher = DES_CIPHER;
		else if (firstack & FLG_USE_IDEA)
			cipher = IDEA_CIPHER;
		else if (firstack & FLG_USE_RC4)
			cipher = RC4_CIPHER;
		else
			cipher = TRIPLEDES_CIPHER;
	}

	/* check integrity and ordering */
	if (firstack & FLG_ACTIVE_ATTACKS) {
		if ((len = read(channel, buf, MAKEIOBUF)) == -1) {
			syslog(LOGPRI, "read(): %m");
			return(1);
		}

		if (!len) /* disconnection */
			return(0);

		traffic += len;
		if (side == CLIENT_SIDE && firstack & FLG_DEBUG) {
			cprintf("hexdump buffer %d bytes: ", len);
			hexdump(NULL, buf, len);
		}

		/* decode buffer */
		cryptbuf(buf, len, dec, cipher, iside);

		if ((p = extract(buf, len)) == NULL)
			return(1);

		for (i = 0; p[i]; i++) {
			/* increase sequence number */
			sequence++;

			/* check sequence for remote packet */
			if (sequence != p[i]->seq) {
				syslog(LOGPRI, "data out of order, possible active attack! packet #%lu expected, #%lu received",
							sequence, p[i]->seq);
				if (side == CLIENT_SIDE)
					cprintf("DATA OUT OF ORDER, possible active attack!\n\r");
				return(1);
			}

			/* check checksum for remote packet */
			crc32val = 0L;
			UpdateCRC(p[i]->data, p[i]->len);
			if (crc32val != p[i]->crc32) {
				syslog(LOGPRI, "garbled data, possible active attack! packet #%lu, len = %d, crc32 %ld expected %ld received",
					sequence,
					p[i]->len,
					crc32val,
					p[i]->crc32);
		/*
		{
			char	buf[MAKEIOBUF * 2];

			bzero(buf, sizeof(buf));
			memcpy(buf, p[i]->data, p[i]->len > 1000 ? 1000 : p[i]->len);
			syslog(LOGPRI, "garbled buffer (%d) = ``%s''", p[i]->len,  buf);
		}
		*/
				if (side == CLIENT_SIDE)
					cprintf("GARBLED DATA, possible active attack!\n\r");
				return(1);
			}

			/* log to file. only client gets logging set */
			if (logging) {
				if (write(logfh, p[i]->data, p[i]->len) == -1) {
					cprintf("%s: %s\r\n",
						logfilename,
						sys_errlist[errno]);
					(void)close(logfh);
					logfh = 0;
					logging = 0;
				}
			}

#ifndef NOREMEXEC
			if (firstack & FLG_ACTIVE_ATTACKS &&
			    /* double check */
			    (p[i]->flag & COMBO_FLG_GET ||
			    p[i]->flag & COMBO_FLG_PUT) &&
			    /* this is a triple check! */
			    !memcmp(p[i]->data, XFERTOKEN, strlen(XFERTOKEN))) {
				int	ret;

				ret = doserverftp(channel, dec, cipher,
					p[i]->flag, username, remhostname);
				if (ret == 1)
						return(1);
				continue;
			}
#endif /* NOREMEXEC */

			/* write buffer to OUT */
			if (write_data(out, p[i]->data, p[i]->len) !=
								p[i]->len) {
				syslog(LOGPRI, "write_data(out): %m");
				return(1);
			}

			/* store output into a circular buffer, for skey
			challenge detection and skey response automatic
			generation
			*/
			storeskeybuf(p[i]->data, p[i]->len);
		}
	}
	/**/
	else {
		if ((len = read(channel, buf, MAKEIOBUF)) == -1) {
			syslog(LOGPRI, "read(): %m");
			return(1);
		}

		if (!len)
			return(0);

		traffic += len;

		if (side == CLIENT_SIDE && firstack & FLG_DEBUG) {
			cprintf("hexdump buffer %d bytes: ", len);
			hexdump(NULL, buf, len);
		}

		/* decode buffer */
		cryptbuf(buf, len, dec, cipher, iside);

		/* log to file. only client gets logging set */
		if (logging) {
			if (write(logfh, buf, len) == -1) {
				cprintf("%s: %s\r\n", logfilename,
							sys_errlist[errno]);
				(void)close(logfh);
				logfh = 0;
				logging = 0;
			}
		}

		if (write_data(out, buf, len) != len) {
			syslog(LOGPRI, "write_data(out): %m");
			return(1);
		}

		/* store output into a circular buffer, for skey
		challenge detection and skey response automatic
		generation */
		storeskeybuf(buf, len);
	}

	/* server side: receiving some kind of user's input */
	if (side == SERVER_SIDE && timeout) {
		(void)alarm(timeout * 60); /* reset timer */
	}

	return(-1);
}

combopacket **
extract(buf, len)
char	*buf;
int	len;
{
	static combopacket	*p[MAXCOMBONUM];
	static combopacket	*x;
	int			objnum;
	static char		intbuf[MAKEIOBUF * 2], tmp[MAKEIOBUF * 2];
	static int		intbufoffset; /* append offset */
	int			readoffset; /* reading offset */
	int			datalen; /* size of current data buffer */
	long			dataseq; /* seq number of current data buf */
#ifdef COMBOFLAG
	int			dataflag; /* option flag */
#endif /* COMBOFLAG */
	unsigned long		datacrc32; /* checksum of current data buf */
	int			remaining; /* decreasing counter */
	static int		olddatalen;
	static unsigned long	olddataseq;
#ifdef COMBOFLAG
	static int		olddataflag;
#endif /* COMBOFLAG */
	static long		olddatacrc32;
	int			readheader = 0;
	register		i;

	/* free array elements */
	for (i = 0; p[i]; i++) {
		free(p[i]->data);
		free(p[i]);
	}

	/* overflow! should never happen */
	if (len + intbufoffset > MAKEIOBUF * 2) {
		syslog(LOGPRI, "extract: buffer overflow!\n");
		return(NULL);
	}

	/* store buf into internal buffer.  we handle packet
	fragmentation in this way */
	memcpy(intbuf + intbufoffset, buf, len);
	remaining = intbufoffset + len;

	/* build combopacket array */
	for (objnum = 0, readoffset = 0; ; objnum++) {
		/*
		syslog(LOGPRI, "extract: ANALYZING OBJECT # %d", objnum);
		*/

		/* handle residue data.
		at least the non-dynamic part of combopacket should fit
		if it does not, continue and collect more data. */
		if (!olddatalen && remaining <
				sizeof(dataseq) +
#ifdef COMBOFLAG
				sizeof(dataflag) +
#endif /* COMBOFLAG */
				sizeof(datacrc32) +
				sizeof(datalen)) {
			/*
			syslog(LOGPRI, "BREAKING at the first point: remaining = %d", remaining);
			*/
			break;
		}

		if (objnum >= MAXCOMBONUM - 1) {
			syslog(LOGPRI, "extract: packetnum overflow!\n");
			return(NULL);
		}

		if (!olddatalen) {
			/* get sequence number of this packet */
			memcpy(&dataseq,
				intbuf +
				readoffset,
					sizeof(dataseq));
			dataseq = ntohl(dataseq);
#ifdef COMBOFLAG
			memcpy(&dataflag,
				intbuf +
				readoffset +
				sizeof(dataseq),
					sizeof(dataflag));
			dataflag = ntohl(dataflag);
#endif /* COMBOFLAG */

			/* get checksum of this packet */
			memcpy(&datacrc32,
				intbuf +
				readoffset +
				sizeof(dataseq)
#ifdef COMBOFLAG
				+ sizeof(dataflag)
#endif /* COMBOFLAG */
					, sizeof(datacrc32));
			datacrc32 = ntohl(datacrc32);

			/* get size of this packet */
			memcpy(&datalen,
				intbuf +
				readoffset +
				sizeof(dataseq) +
#ifdef COMBOFLAG
				sizeof(dataflag) +
#endif /* COMBOFLAG */
				sizeof(datacrc32),
					sizeof(datalen));
			datalen = ntohl(datalen);

			remaining -= sizeof(datalen) +
				     sizeof(dataseq) +
#ifdef COMBOFLAG
				     sizeof(dataflag) + 
#endif /* COMBOFLAG */
				     sizeof(datacrc32);
			readheader++;
		}
		else {
			datalen = olddatalen;
			dataseq = olddataseq;
#ifdef COMBOFLAG
			dataflag = olddataflag;
#endif /* COMBOFLAG */
			datacrc32 = olddatacrc32;
		}

		/* this is a special case: we read datalen and we find out
		that there is not enough data in the buffer.  we have to
		keep track of datalen before returning to get more data */
		if (datalen > remaining) {
			olddatalen = datalen;
			olddataseq = dataseq;
#ifdef COMBOFLAG
			olddataflag = dataflag;
#endif /* COMBOFLAG */
			olddatacrc32 = datacrc32;
			/*
			syslog(LOGPRI, "extract: BREAKING at the second point (datalen = %d >>> remaining = %d)",
							datalen, remaining);
			*/
			break;
		}

		/*
		syslog(LOGPRI, "extract: before *x setting...");
		syslog(LOGPRI, "extract: *x dump: %lu, %ld, %d, %s\n",
			dataseq, datacrc32, datalen, intbuf + readoffset);
		*/

		/* allocate x */
		if ((x = (combopacket *)malloc(sizeof(combopacket))) == NULL) {
			syslog(LOGPRI, "extract: malloc()");
			return(NULL);
		}

		/* set x.seq */
		x->seq = dataseq;

#ifdef COMBOFLAG
		/* set x.flag */
		x->flag = dataflag;
#endif /* COMBOFLAG */

		/* set x.crc32 */
		x->crc32 = datacrc32;

		/* set x.len */
		x->len = datalen;

		/* set x.data */
		if ((x->data = (char *)malloc(datalen)) == NULL) {
			perror("extract: malloc()");
			return(NULL);
		}
		/* copy data into x->data, skipping datalen */
		if (/* !olddatalen */ readheader)
			memcpy(x->data, intbuf +
					readoffset +
					sizeof(datalen) +
				 	sizeof(dataseq) +
#ifdef COMBOFLAG
					sizeof(dataflag) +
#endif /* COMBOFLAG */
				 	sizeof(datacrc32),
								datalen);
		else {
			memcpy(x->data, intbuf + readoffset, datalen);
		}

		/*
		syslog(LOGPRI, "extract: *x dump...");
		syslog(LOGPRI, "extract: *x dump: seq = %lu, crc = %ld, len =  %d",
			dataseq, datacrc32, datalen);
		{
			char	buf[MAKEIOBUF * 2];

			bzero(buf, sizeof(buf));
			memcpy(buf, x->data, datalen);
			syslog(LOGPRI, "extract: data = ``%s''", buf);
		}
		*/
		

		/* set array element */
		p[objnum] = x;

		/* update pointers */
		if (/* !olddatalen */ readheader) {
			readoffset += datalen +
					sizeof(datalen) +
				 	sizeof(dataseq) +
#ifdef COMBOFLAG
					sizeof(dataflag) +
#endif /* COMBOFLAG */
				 	sizeof(datacrc32);
			/* ``remaining'' has already been decreased by
			header's size */
			remaining -= datalen;
		}
		else {
			readoffset += datalen;
			remaining -= datalen;
		}
		olddatalen = 0;
#ifdef COMBOFLAG
		olddataflag = 0;
#endif /* COMBOFLAG */
		olddatacrc32 = 0L;
		olddataseq = 0L;
		readheader = 0;
	}

	/* reduce intbuffer */
	memcpy(tmp, intbuf, MAKEIOBUF * 2);
	bzero(intbuf, sizeof(intbuf));
	/* if (!olddatalen || fragmentation) */
	if (!readheader) /* this nly happens when breaking out of the loop */
		memcpy(intbuf, tmp + readoffset, MAKEIOBUF * 2 - readoffset);
	else /* skip header */
		memcpy(intbuf, tmp +
				readoffset +
				sizeof(datalen) +
				sizeof(dataseq) +
#ifdef COMBOFLAG
				sizeof(dataflag) +
#endif /* COMBOFLAG */
				sizeof(datacrc32),
				MAKEIOBUF * 2 -
				readoffset -
				sizeof(datalen) -
				sizeof(dataseq) -
#ifdef COMBOFLAG
				sizeof(dataflag) -
#endif /* COMBOFLAG */
				sizeof(datacrc32));
	intbufoffset = remaining;

	/* lasty element should be zero */
	p[objnum] = NULL;

	return(p);
}


/* client read()s (blocks) first */
doclientftp(net, in, out, enc, cipher, mode)
char	*in, *out;
int	net, enc, cipher, mode;
{
	char		buf[100];
	unsigned char	digest[16];
	combocmd	cmd;
	comboanswer	answer;
	int		localfh, len;
	int		error = 0;
	MD5_CTX		mdContext;
	long		crc32tmp;

	/* read what ack from server */
	if (read_data(net, buf, strlen(WHAT_ACK)) != strlen(WHAT_ACK)) {
		cprintf("Bad WHAT_ACK from server\r\n");
		return(1);
	}

	/* decrypt what ack */
	cryptbuf(buf, strlen(WHAT_ACK), enc, cipher, CLIENT_SIDE);

	/* check WHAT_ACK */
	if (memcmp(buf, WHAT_ACK, strlen(WHAT_ACK))) {
		cprintf("Corrupted WHAT_ACK from server\r\n");
		return(1);
	}

	bzero(&cmd, sizeof(cmd));

	/* set operation */
	cmd.cmd = mode;

	/* put local ``in'' file into remote ``out'' file */
	if (mode & COMBO_FLG_PUT) {
		struct stat	sbuf;

		/* set filename */
		strncpy(cmd.filename, out, sizeof(cmd.filename));

		if (stat(in, &sbuf) == -1) {
			cprintf("PUT failed: %s: %s\r\n",
						in, sys_errlist[errno]);
			error++;
		}
		cmd.filesize = len = (unsigned long)sbuf.st_size;
		/* open file for reading */
		if ((localfh = open(in, O_RDONLY)) == -1) {
			cprintf("local error: %s: %s\r\n",
						in, sys_errlist[errno]);
			error++;
		}
	}
	/* get remote ``in'' file and put it into local ``out'' file */
	else if (mode & COMBO_FLG_GET) {
		/* set filename */
		strncpy(cmd.filename, in, sizeof(cmd.filename));

		/* set filesize */
		cmd.filesize = (unsigned long)0L;
		/* open for writing */
		if ((localfh = open(out, O_WRONLY | O_CREAT | O_TRUNC,
							0600)) == -1) {
			cprintf("local error: %s: %s\r\n",
						out, sys_errlist[errno]);
			error++;
		}
	}
	else
		error++;

	if (error)
		cmd.cmd = 0;
	
	cmd.cmd = htonl(cmd.cmd);
	cmd.filesize = htonl(cmd.filesize);

	/* calculate checksum */
	MD5Init(&mdContext);
	MD5Update(&mdContext, &cmd, sizeof(cmd) - 16);
	MD5Final(cmd.checksum, &mdContext);

	/* encrypt cmd */
	cryptbuf(&cmd, sizeof(cmd), enc, cipher, CLIENT_SIDE);

	/* write cmd to server.
	if error occurred, let server know about it 
	*/
	if (write_data(net, &cmd, sizeof(cmd)) != sizeof(cmd)) {
		cprintf("Error sending cmd to server\r\n");
		close(localfh);
		return(1);
	}

	if (error) {
		return(-1); /* error gets handled */
	}

	if (read_data(net, &answer, sizeof(answer)) != sizeof(answer)) {
		cprintf("Bad answer from server\r\n");
		close(localfh);
		return(1);
	}

	/* decrypt answer */
	cryptbuf(&answer, sizeof(answer), enc, cipher, CLIENT_SIDE);

	/* check answer integrity */
	MD5Init(&mdContext);
	MD5Update(&mdContext, &answer, sizeof(answer) - 16);
	MD5Final(digest, &mdContext);
	if (memcmp(digest, answer.checksum, 16)) {
		cprintf("Corrupted ANSWER from server\r\n");
		return(1);
	}

	answer.answer = ntohl(answer.answer);
	answer.filesize = ntohl(answer.filesize);

	if (answer.answer) {
		cprintf("Remote error: %s\r\n", answer.message);
		return(-1);
	}

	if (mode & COMBO_FLG_PUT) {
		/* copy file to network */
		if (streamcp(localfh, net, len, 1, enc, cipher, 1)) {
			cprintf("Error uploading %s: %s\r\n",
							in, sys_errlist[errno]);
			(void)close(localfh);
			return(1);
		}
		if (close(localfh) == -1) {
			cprintf("close(%s): %s\r\n", in, sys_errlist[errno]);
			return(1);
		}
		cprintf("Local file %s UPLOADED to %s\r\n", in, out);
	}
	else if (mode & COMBO_FLG_GET) {
		len = answer.filesize;
		/* copy network to file */
		if (streamcp(net, localfh, len, 1, enc, cipher, 0)) {
			cprintf("Error downloading %s: %s\r\n",
							in, sys_errlist[errno]);
			(void)close(localfh);
			return(1);
		}
		if (close(localfh) == -1) {
			cprintf("close(%s): %s\r\n", out, sys_errlist[errno]);
			return(1);
		}
		cprintf("Remote file %s DOWNLOADED to %s\r\n", in, out);
	}

	/* check the integrity of the xfered file */
	crc32tmp = htonl(crc32val);

	/* crypt crc32tmp  */
	cryptbuf(&crc32tmp, sizeof(crc32tmp), enc, cipher, CLIENT_SIDE);

	/* send encrypted checksum to server, get checksum from server */
	if (write_data(net, &crc32tmp, sizeof(crc32tmp)) != sizeof(crc32tmp) ||
	    read_data(net, &crc32tmp, sizeof(crc32tmp)) != sizeof(crc32tmp)) {
		cprintf("Cannot verify transfer integrity!\r\n");
		return(1);
	}

	/* decrypt crc32tmp */
	cryptbuf(&crc32tmp, sizeof(crc32tmp), enc, cipher, SERVER_SIDE);
	crc32tmp = ntohl(crc32tmp);
	if (crc32tmp != crc32val) {
		cprintf("File is CORRUPTED!  Do NOT use it!\r\n");
		return(1);
	}

	return(0);
}

doserverftp(net, dec, cipher, mode, username, remhostname)
char	*username, *remhostname;
int	net, dec, cipher, mode;
{
	char		buf[100];
	unsigned char	digest[16];
	combocmd	cmd;
	comboanswer	answer;
	int		localfh, len;
	int		error = 0;
	MD5_CTX		mdContext;
	long		crc32tmp;

	/* send what ack to client */
	strncpy(buf, WHAT_ACK, sizeof(buf));
	cryptbuf(buf, strlen(WHAT_ACK), dec, cipher, CLIENT_SIDE);
	if (write_data(net, buf, strlen(WHAT_ACK)) != strlen(WHAT_ACK)) {
		syslog(LOGPRI, "XFER: error sending WHAT_ACK to %s",
								remhostname);
		return(1);
	}

	/* read combocmd from client */
	if (read_data(net, &cmd, sizeof(cmd)) != sizeof(cmd)) {
		syslog(LOGPRI, "XFER: bad cmd, connection with %s broken",
							remhostname);
		return(1);
	}

	/* decrypt cmd */
	cryptbuf(&cmd, sizeof(cmd), dec, cipher, CLIENT_SIDE);

	/* check cmd integrity */
	MD5Init(&mdContext);
	MD5Update(&mdContext, &cmd, sizeof(cmd) - 16);
	MD5Final(digest, &mdContext);
	if (memcmp(digest, cmd.checksum, 16)) {
		syslog(LOGPRI, "XFER: corrupted cmd from %s", remhostname);
		return(1);
	}

	cmd.cmd = ntohl(cmd.cmd);
	cmd.filesize = ntohl(cmd.filesize);

	if (cmd.cmd == 0) { /* handled error */
		return(-1);
	}

	/* double check mode */
	if (mode != cmd.cmd) {
		syslog(LOGPRI, "XFER: bad command from %s", remhostname);
		return(1);
	}

	bzero(&answer, sizeof(answer));

	answer.answer = 0; /* successful by default */

	/* send file to client */
	if (!userauthenticated) {
		syslog(LOGPRI,
		"XFER %s ATTEMPTED to %s %s from %s, not being authenticated!",
			username,
			mode & COMBO_FLG_GET ? "GET" : "PUT",
			cmd.filename,
			remhostname);
		strncpy(answer.message, "not authenticated!",
						sizeof(answer.message));
		error++;
	}
	else if (mode & COMBO_FLG_GET) {
		struct stat	sbuf;

		/* set filesize */
		if (stat(cmd.filename, &sbuf) == -1) {
			syslog(LOGPRI, "GET failed by %s from %s: %s: %m",
					username, remhostname, cmd.filename);
			answer.answer = 1;
			strncpy(answer.message, sys_errlist[errno],
							sizeof(answer.message));
			error++;
		}
		answer.filesize = len = (unsigned long)sbuf.st_size;
		/* open file for reading */
		if ((localfh = open(cmd.filename, O_RDONLY)) == -1) {
			syslog(LOGPRI, "GET failed by %s from from %s: %s: %m",
					username, remhostname, cmd.filename);
			strncpy(answer.message, sys_errlist[errno],
							sizeof(answer.message));
			error++;
		}
	}
	/* receive file from client */
	else if (mode & COMBO_FLG_PUT) {
		len = cmd.filesize;
		answer.filesize = (unsigned long)0L;
		if ((localfh = open(cmd.filename, O_WRONLY | O_CREAT | O_TRUNC,
							0600)) == -1) {
			syslog(LOGPRI, "PUT failed by %s from %s: %s: %m",
					username, remhostname, cmd.filename);
			strncpy(answer.message, sys_errlist[errno],
							sizeof(answer.message));
			error++;
		}
	}
	else /* should never happen */
		error++;

	if (error)
		answer.answer = 1; /* fails */
	
	answer.answer = htonl(answer.answer);
	answer.filesize = htonl(answer.filesize);

	/* calculate answer checksum */
	MD5Init(&mdContext);
	MD5Update(&mdContext, &answer, sizeof(answer) - 16);
	MD5Final(answer.checksum, &mdContext);

	/* crypt answer */
	cryptbuf(&answer, sizeof(answer), dec, cipher, CLIENT_SIDE);

	if (write_data(net, &answer, sizeof(answer)) != sizeof(answer)) {
		syslog(LOGPRI, "XFER: error sending answer to %s", remhostname);
		return(1);
	}

	
	if (error)
		return(-1);
	
	/* copy file to network */
	if (mode & COMBO_FLG_PUT) {
		if (streamcp(net, localfh, len, 0, dec, cipher, 0)) {
			syslog(LOGPRI, "XFER: error storing %s from %s: %m",
						cmd.filename, remhostname);
			(void)close(localfh);
			return(1);
		}
		if (close(localfh) == -1) {
			syslog(LOGPRI, "XFER: close(%s): %m", cmd.filename);
			return(1);
		}
		syslog(LOGPRI, "%s UPLOADED %s (%d bytes) from %s",
				username, cmd.filename, len, remhostname);
	}
	/* copy network to file */
	else if (mode & COMBO_FLG_GET) {
		if (streamcp(localfh, net, len, 0, dec, cipher, 1)) {
			syslog(LOGPRI, "XFER: error sending %s to %s: %m",
						cmd.filename, remhostname);
			(void)close(localfh);
			return(1);
		}
		if (close(localfh) == -1) {
			syslog(LOGPRI, "XFER: close(%s): %m", cmd.filename);
			return(1);
		}
		syslog(LOGPRI, "%s DOWNLOADED %s (%d bytes) from %s",
				username, cmd.filename, len, remhostname);
	}

	/* check the integrity of the xfered file */
	crc32tmp = htonl(crc32val);

	/* crypt crc32tmp  */
	cryptbuf(&crc32tmp, sizeof(crc32tmp), dec, cipher, SERVER_SIDE);

	/* send encrypted checksum to client, get checksum from client */
	if (write_data(net, &crc32tmp, sizeof(crc32tmp)) != sizeof(crc32tmp) ||
	    read_data(net, &crc32tmp, sizeof(crc32tmp)) != sizeof(crc32tmp)) {
		syslog(LOGPRI, "XFER: cannot verify integrity for %s from %s",
						cmd.filename, remhostname);
		return(1);
	}

	/* decrypt crc32tmp */
	cryptbuf(&crc32tmp, sizeof(crc32tmp), dec, cipher, CLIENT_SIDE);
	crc32tmp = ntohl(crc32tmp);
	if (crc32tmp != crc32val) {
		syslog(LOGPRI, "XFER: corrupted file: %s from %s",
						cmd.filename, remhostname);
		return(1);
	}

	return(0);
}

streamcp(from, to, size, isclientside, enc, cipher, crcthencrypt)
int	from, to, size, isclientside, enc, cipher, crcthencrypt;
{
	char		buf[MAKEIOBUF];
	register	todo, toread;
	struct timeval	first, second;
	struct timezone	tz;

	if (isclientside) {
		cprintf("xfering %d bytes, using %d bytes/hash mark:  ",
						size, sizeof(buf));
	}

	/* reset crc32 */
	crc32val = 0L;

	(void)gettimeofday(&first, &tz);

	todo = size;
	while (todo > 0) {
		toread = todo < sizeof(buf) ? todo : sizeof(buf);
		if (read_data(from, buf, toread) != toread) {
			return(1);
		}

		if (crcthencrypt) { /* from fle to network */
			UpdateCRC(buf, toread);
		}

		cryptbuf(buf, toread, enc, cipher, CLIENT_SIDE);

		if (!crcthencrypt) { /* from net to file */
			UpdateCRC(buf, toread);
		}

		if (write_data(to, buf, toread) != toread) {
			return(1);
		}

		todo -= toread;

		if (isclientside) {
			printf("#");
			fflush(stdout);
		}
		xfertraffic += toread;
	}

	(void)gettimeofday(&second, &tz);

	if (isclientside) {
		float	ratio;

		if (first.tv_usec > second.tv_usec) {
			second.tv_usec += 1000000;
			second.tv_sec--;
		}

		ratio = (float)size * (float)1000000;
		ratio /= (float)(second.tv_sec - first.tv_sec) *
						(float)1000000 +
			((float)(second.tv_usec - first.tv_usec));
		ratio /= (float)1024;

		printf("\r\n");
		fflush(stdout);
		cprintf("%d bytes received in %lu.%ld seconds (%.2f Kbytes/s)\r\n",
				size, 
				second.tv_sec - first.tv_sec,
				(second.tv_usec - first.tv_usec) / 10000L,
				ratio);
	}
	return(0);
}
