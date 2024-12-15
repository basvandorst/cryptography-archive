/*
 * $Author: vince $
 * $Header: /users/vince/src/stel-dist/stel/RCS/stel.c,v 1.122 1996/04/26 16:29:23 vince Exp vince $
 * $Date: 1996/04/26 16:29:23 $
 * $Revision: 1.122 $
 * $Locker: vince $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: stel.c,v 1.122 1996/04/26 16:29:23 vince Exp vince $";
#endif	lint

#include <stdio.h>
#include <termios.h>
#include <signal.h>
#include <pwd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>

#ifdef HAS_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef AIX
#include <sys/ioctl.h>
#endif
#include "defs.h"

/* md5 stuff */
#include "global.h"
#include "md5.h"

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

extern int	logfh; /* log to file handler */

#define OPTIONS "l:np:tvrmMi14aDeh"

#define USAGE		\
"STEL: Secure TELnet, BETA VERSION, protocol id = %d\n\
Usage: %s [hostname] [options] [commands...]\n\
	hostname:	the system you want to connect to\n\
     	-l logname:	the username on the remote system\n\
	-p portnumber:	set port number\n\
	-i:		use IDEA cipher\n\
	-1:		use SINGLE DES cipher\n\
	-4:		use RC4 cipher\n\
	-n:		do not use data encryption\n\
	-r:		enter a random string to enhance randomness\n\
	-m:		use (small) 512 bits modulus\n\
	-M:		use (large) 2048 bits modulus\n\
	-a:		protect from active attacks\n\
	-t:		do not allocate a pseudo terminal\n\
	-e:		disable escape features\n\
	-v:		be verbose\n\
	-D:		be extra verbose\n\
	-h:		display this help screen\n\n\
Default port number is %d\n\
Default encryption algorithm is EDE TRIPLE DES\n\
Default Diffie-Hellman modulus size is 1024 bits\n\
Default escape character is '^]'\n"
#define USECRYPT	\
"This session is using %s encryption for all data transmissions.\r\n"
#define NOUSECRYPT	\
"WARNING: NOT USING DATA ENCRYPTION -- THIS SESSION IS INSECURE\r\n"
#define NOUSEPTY	\
"This session is not using pseudo terminals: passwords will be visible.\r\n"
#define BADOPTION	"option -%c is not compatible with previous options\n"
#define ACTIVEPROTECT	\
"This session is checking integrity and ordering for all data transmissions.\r\n"
struct termios		old, new;
int			ttyset;
static struct termios	saved_ttymode, noecho_ttymode; 

main(argc, argv)
int	argc;
char	*argv[];
{
	cpacket		pp;
	spacket		ppp;
	char		hostbuf[100], username[MAXUSERNAMELEN];
	char		*tohost, *env, *encmode;
	char		*rndstr, rndbuf[512], authbuf[STEL_SECRET_MAX_LEN];
	int		i, s;
	int		cipher;
	int		firstack, noptyflg, errflg, rflg;
	int		portnumber;
	extern char	*optarg;
	extern int	optind;
	struct passwd	*pwd;
	MD5_CTX		mdContext;
	unsigned char	digest[16];
	struct winsize	os_window;
	struct servent	*serv;

	void		trapper();

	char		*getstelsecret();

	/* using SYSV fashion */
	tohost = NULL;
	if (argc > 1 && argv[1][0] != '-') {
		tohost = argv[1]; /* server name */
		argv++;
		argc--;
	}

	/* parse options */
	errflg = noptyflg = rflg = 0;

	firstack = 0;
	rndstr = NULL;
	bzero(&pp, sizeof(pp));

	if ((serv = getservbyname(STELCLIENT, "tcp")) == NULL)
		portnumber = PORTNUM;
	else
		portnumber = htons(serv->s_port);
	
	while ((i = getopt(argc, argv, OPTIONS)) != -1)
		switch (i) {
			case	'l':
				strncpy(username, optarg, sizeof(username));
				strncpy(pp.username, optarg, sizeof(pp.username));
				break;

			case	't':
				noptyflg++;
				break;

			case	'n':
				if (firstack & FLG_USE_SINGLE ||
				    firstack & FLG_USE_IDEA ||
				    firstack & FLG_USE_RC4) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				firstack |= FLG_NO_ENCRYPTION;
				break;

			case	'1':
				if (firstack & FLG_NO_ENCRYPTION ||
				    firstack & FLG_USE_IDEA ||
				    firstack & FLG_USE_RC4) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				firstack |= FLG_USE_SINGLE;
				break;

			case	'i':
				if (firstack & FLG_NO_ENCRYPTION ||
				    firstack & FLG_USE_SINGLE ||
				    firstack & FLG_USE_RC4) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				firstack |= FLG_USE_IDEA;
				break;

			case	'4':
				if (firstack & FLG_NO_ENCRYPTION ||
				    firstack & FLG_USE_SINGLE ||
				    firstack & FLG_USE_IDEA) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				firstack |= FLG_USE_RC4;
				break;

			case	'm':
				if (firstack & FLG_NO_ENCRYPTION) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				firstack |= FLG_SMALL_MODULUS;
				break;

			case	'M':
				if (firstack & FLG_NO_ENCRYPTION) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				firstack |= FLG_LARGE_MODULUS;
				break;

			case	'a':
				firstack |= FLG_ACTIVE_ATTACKS;
				break;

			case	'r':
				if (firstack & FLG_NO_ENCRYPTION) {
					fprintf(stderr, BADOPTION, i);
					errflg++;
					break;
				}
				rflg++;
				break;

			case	'p':
				portnumber = atoi(optarg);
				if (portnumber < 0) {
					fprintf(stderr, "bad port number\n");
					errflg++;
				}
				break;

			case	'v':
				firstack |= FLG_BE_VERBOSE;
				break;

			case	'D':
				firstack |= FLG_BE_VERBOSE;
				firstack |= FLG_DEBUG;
				break;

			case	'e':
				firstack |= FLG_NO_ESCAPE;
				break;

			case	'h':
				(void)fprintf(stderr, USAGE,
						VERSION, STELCLIENT, PORTNUM);
				exit(0);
				break;
			
			case	'?':
			case	':':
				errflg++;
				break;
		}

	if (errflg) {
		(void)fprintf(stderr, "bad options -- %s -h for help.\n",
								STELCLIENT);
		exit(1);
	}

	if (rflg) {
		if (getstelpass("Enter a lenghty random string", rndbuf,
							sizeof(rndbuf))) {
			(void)fprintf(stderr, "string must be provided!\n");
			exit(1);
		}
		if (strlen(rndbuf) < 16) {
			(void)fprintf(stderr, "please enter a longer string!\n");
			exit(1);
		}
		rndstr = rndbuf;
	}

	/* set command */
	for (i = 0; optind < argc; optind++) {
		i += strlen(argv[optind]) + 1;
		if (i > sizeof(pp.command) - 2) {
			(void)fprintf(stderr, "command is too large.\n");
			exit(1);
		}
		(void)strcat(pp.command, argv[optind]);
		if (optind < argc -1)
			(void)strcat(pp.command, " ");
	}

	/* set username */
	if (pp.username[0] == '\0') {
		if ((pwd = getpwuid(getuid())) == NULL) {
			(void)fprintf(stderr, "Who are you?\n");
			exit(1);
		}
		strncpy(username, pwd->pw_name, sizeof(username));
		strncpy(pp.username, pwd->pw_name, sizeof(pp.username));
	}

	/* don't use terminals when deattached from a terminal */
	if (!isatty(0)) {
		noptyflg = 1;
		pp.mode |= FLG_DONTUSE_PTY;
	}
	/* command line option */
	else if (noptyflg)
		pp.mode |= FLG_DONTUSE_PTY;

	
	if (!noptyflg && isatty(0) && ioctl(0, TIOCGWINSZ, &os_window) >= 0) {
		pp.rows = os_window.ws_row;
		pp.cols = os_window.ws_col;
	}
	else {
		pp.rows = (-1);
		pp.cols = (-1);
	}

	/* set environment */
	if ((env = (char *)getenv("TERM")) != NULL)
		strncpy(pp.TERM, env, sizeof(pp.TERM));
	if ((env = (char *)getenv("DISPLAY")) != NULL)
		strncpy(pp.DISPLAY, env, sizeof(pp.DISPLAY));
	if ((env = (char *)getenv("LINES")) != NULL)
		strncpy(pp.LINES, env, sizeof(pp.LINES));
	if ((env = (char *)getenv("COLUMNS")) != NULL)
		strncpy(pp.COLUMNS, env, sizeof(pp.COLUMNS));
	if ((env = (char *)getenv("WINDOWID")) != NULL)
		strncpy(pp.WINDOWID, env, sizeof(pp.WINDOWID));
	/*
	if ((env = (char *)getenv("PATH")) != NULL)
		strncpy(pp.PATH, env, sizeof(pp.PATH));
	*/

	if (tohost == NULL) {
		printf("Hostname? ");
		if (fgets(hostbuf, sizeof(hostbuf), stdin) == NULL) {
			(void)fprintf(stderr, "no host?\n");
			exit(1);
		}
		tohost = hostbuf;
		rip(tohost);
		if (!strlen(tohost)) {
			(void)fprintf(stderr, "No host?\n");
			exit(1);
		}
	}

#ifdef __ultrix
	(void)openlog(STELCLIENT, LOG_WARNING);
#else
	(void)openlog(STELCLIENT, LOG_PID | LOG_ODELAY, LOG_DAEMON);
#endif

	if ((s = callsocket(tohost, portnumber)) == -1)
		exit(1);

	if (firstack & FLG_BE_VERBOSE)
		printf("Connected to %s on port %d.\n", tohost, portnumber);
	else
		printf("Connected to %s.\n", tohost);
	
#ifdef ASCIIBANNER
	/* silently get and ignore banner from server side */
	if (firstack & FLG_DEBUG)
		printf("\nEATING BANNER FROM SERVER\n");
	(void)eatbanner(s);
#endif
	
	/* tell master about user selected options */
	if (firstack & FLG_DEBUG)
		printf("\nSENDING FIRSTACK 0x%02X TO SERVER\n", firstack);
	i = htonl(firstack);
	if (write(s, &i, sizeof(firstack)) != sizeof(firstack)) {
		perror("write(firstack)");
		exit(1);
	}

	pp.mode = htonl(pp.mode);
	pp.rows = htonl(pp.rows);
	pp.cols = htonl(pp.cols);
	pp.version = htonl(VERSION);

	if (!(firstack & FLG_NO_ENCRYPTION)) {
		memcpy(pp.random, (char *)randomdigest(), 16);
		if (firstack & FLG_DEBUG) {
			hexdump("client stream randomizer: ", pp.random, 16);
		}
	}

	MD5Init(&mdContext);
	/* skip digest itself */
	MD5Update(&mdContext, &pp, sizeof(pp) - 16);
	MD5Final(pp.digest, &mdContext);

	if (!(firstack & FLG_NO_ENCRYPTION)) { /* encryption required */
		/* exchange keys */
		if (dhexchange(s, CLIENT_SIDE, firstack, rndstr) < 0) {
			perror("dhexchange()");
			exit(1);
		}

		/* use IDEA encryption */
		if (firstack & FLG_USE_IDEA) {
			encmode = "IDEA";
			cipher = IDEA_CIPHER;
		}

		/* use SINGLE DES encryption */
		else if (firstack & FLG_USE_SINGLE) {
			encmode = "SINGLE DES";
			cipher = DES_CIPHER;
		}

		/* use IDEA encryption */
		else if (firstack & FLG_USE_RC4) {
			encmode = "RC4";
			cipher = RC4_CIPHER;
		}

		/* use triple DES encryption by default */
		else {
			encmode = "TRIPLE DES";
			cipher = TRIPLEDES_CIPHER;
		}

		/* set client IV */
		memcpy(&c_IV, pp.random, 8);

		/* encrypt pp, skipping IV */
		cryptbuf((char *)&pp + 8, sizeof(pp) - 8, DES_ENCRYPT, cipher,
								CLIENT_SIDE);
	}

	if (firstack & FLG_DEBUG) {
		printf("\nSENDING ENVINFO TO SERVER\n");
		hexdump("first envinfo 20 bytes: ", &pp, 20);
		hexdump("next envinfo 20 random bytes : ",
							(char *)&pp + 20, 20);
		hexdump("next envinfo 20 random bytes : ",
							(char *)&pp + 40, 20);
	}

	if (write_data(s, &pp, sizeof(pp)) != sizeof(pp)) {
		perror("write()");
		exit(1);
	}

	/* we have send the packet to server, now let's listen to its reply */
	if ((i = read(s, &ppp, sizeof(ppp)) != sizeof(ppp))) {
		if (!i) {
			printf("Server has gone away.\n");
			exit(1);
		}
		perror("read()");
		exit(1);
	}

	if (!(firstack & FLG_NO_ENCRYPTION)) { /* encryption required */
		/* set server IV */
		memcpy(&s_IV, ppp.random, 8);

		/* decrypt it skipping server IV */
		cryptbuf((char *)&ppp + 8, sizeof(ppp) - 8, DES_DECRYPT,
							cipher, SERVER_SIDE);

	}

	/* check ppp integrity */
	MD5Init(&mdContext);
	/* skip digest itself */
	MD5Update(&mdContext, &ppp, sizeof(ppp) - 16);
	MD5Final(digest, &mdContext);

	if (memcmp(digest, ppp.digest, 16)) {
		printf("Received corrupted data from server!\n");
		exit(1);
	}

	ppp.sreply = ntohl(ppp.sreply);

	if (firstack & FLG_DEBUG) {
		printf("\nREAD REPLY FROM SERVER\n");
		printf("sreply flag: 0x%02X\n", ppp.sreply);

		if (!(firstack & FLG_NO_ENCRYPTION))
			hexdump("server stream randomizer: ", ppp.random, 16);
	}

	if (ppp.sreply & MSG_WRONG_VERSION) {
		printf("%s version mismatch.  Can not talk with server.\n",
								STELCLIENT);
		exit(1);
	}
					

	/* this is is weak, and it is intended as an experimental feature
	only.  the point is that /etc/stelsecret should be readable by the
	user.  this is no good since the user could make /etc/stelsecret
	public and thus completely defeat the mutual authentication method.
	actually, stel is supposed to be run as a SUID to root program in
	order to take advantage of this feature. I do not like SUIDed
	programs, though. */
	if (ppp.sreply & MSG_SYSTEM_AUTH_REQUIRED) {
		char	*systelsec;

		printf("Host mutual authentication is required by %s.\n",
								tohost);

		if ((systelsec = getstelsecret(ETC_STEL_SECRET_NAME, tohost)) == NULL) {
			printf("Cannot access %s\n", ETC_STEL_SECRET_NAME);
			if (getstelpass("Please enter it manually", authbuf,
							sizeof(authbuf))) {
				(void)fprintf(stderr, "No secret?\n");
				exit(1);
			}
			systelsec = authbuf;
		}

		i = mauth(s, systelsec, CLIENT_SIDE, firstack);
		bzero(systelsec, strlen(systelsec));
		if (i == -1) {
			perror("mauth()");
			exit(1);
		}
		else if (i == 1) {
			fprintf(stderr, "Host mutual authentication failed!\n");
			exit(1);
		}
		printf("Host mutual authentication OK\n");
	}

	/* restore ids, in case stel was a SUID program */
	if (setgid(getgid()) == -1) {
		perror("setgid()");
		exit(1);
	}
	if (setuid(getuid()) == -1) {
		perror("setuid()");
		exit(1);
	}

	if (ppp.sreply & MSG_LOGIN_AUTH_REQUIRED) {
		printf("User mutual authentication required by %s@%s.\n",
							username, tohost);

		if (getstelpass("Enter stelsecret", authbuf, sizeof(authbuf))) {
			(void)fprintf(stderr, "No secret?\n");
			exit(1);
		}

		i = mauth(s, authbuf, CLIENT_SIDE, firstack);
		if (i == -1) {
			perror("mauth()");
			exit(1);
		}
		else if (i == 1) {
			fprintf(stderr, "User mutual authentication failed!\n");
			exit(1);
		}
		printf("User mutual authentication OK\n");
	}
		
	if (ppp.sreply == MSG_MUST_AUTH) {
		printf(
			"Mutual authentication required -- NOT using encryption, aborting...\n");
		exit(1);
	}
	
	if (ppp.sreply == MSG_CORRUPTED_DATA) {
		printf("Server received corrupted data from us!\n");
		exit(1);
	}

	/* server allocates a pseudo terminal */
	if (!noptyflg) {
		(void)signal(SIGINT, trapper);
		(void)signal(SIGQUIT, trapper);
		(void)signal(SIGHUP, trapper);
		(void)signal(SIGTERM, trapper);

		/* set terminal to raw */
		if (tcgetattr(0, &old) == -1) {
			perror("tcgetattr()");
			exit(1);
		}
		(void)memcpy(&new, &old, sizeof(struct termios));
		new.c_cc[VTIME] = 0;
		new.c_cc[VMIN]  = 1;
		new.c_cc[VINTR] = 0;
		new.c_lflag &= ~(ECHO | ISIG | ICANON);
		new.c_oflag &= ~(ONLCR | OPOST);
#ifndef SOLARIS
#if defined(BSD) && (BSD >= 199306)
		new.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | 
							ISTRIP | BRKINT);
#else
		new.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IUCLC |
							ISTRIP | BRKINT);
#endif
#else /* !SOLARIS */
		new.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IUCLC |
							ISTRIP | BRKINT);
#endif /* SOLARIS */
		if (tcsetattr(0, TCSANOW, &new) == -1) {
			perror("tcgetattr()");
			exit(1);
		}
		ttyset = 1;
	}

	if (firstack & FLG_NO_ENCRYPTION)
		printf(NOUSECRYPT);
	else
		printf(USECRYPT, encmode);

	if (firstack & FLG_ACTIVE_ATTACKS)
		printf(ACTIVEPROTECT);

	/* escape status */
	if (noptyflg) {
		printf(NOUSEPTY);
		printf("Escape features disabled.\r\n");
	}
	else
		printf("Escape character is '^]'.\r\n");

	cprintf("please note the PID\r\n");
	cprintf("messages without the PID are originated by remote party\r\n");
	cprintf("----GOING REMOTE----\r\n\n");

	if (makeio(0, 1, s, -1, CLIENT_SIDE, firstack, tohost,
				&old, &new, NOTIMEOUT, username)) {
		if (logfh)
			(void)close(logfh);
		if (!noptyflg)
			(void)tcsetattr(0, TCSANOW, &old);
		cprintf("Connection with %s lost.\n", tohost);
		(void)shutdown(s, 2);
		(void)close(s);
		exit(1);
	}
	if (logfh)
		(void)close(logfh);
	(void)shutdown(s, 2);
	(void)close(s);
	if (!noptyflg)
		(void)tcsetattr(0, TCSANOW, &old);
	cprintf("Connection with %s closed.\n", tohost);
	return(0);
}

void
trapper(s)
int	s;
{
	(void)signal(s, trapper);

	(void)fprintf(stderr, "%s: trapped signal %d!\n", STELCLIENT, s);
	if (ttyset)
		(void)tcsetattr(0, TCSANOW, &old);
	exit(1);
}

getstelpass(prompt, buf, len)
char	*prompt, *buf;
int	len;
{
	void		(*oldsig) (); 
	void		gp_restore_ttymode();

	if (isatty(0)) {
		printf("%s: ", prompt);
		(void)fflush(stdout);
		/* ECHO OFF */
		tcgetattr(0, &saved_ttymode);
		if ((oldsig = signal(SIGINT, SIG_IGN)) != SIG_IGN)
			(void)signal(SIGINT, gp_restore_ttymode);
		tcgetattr(0, &noecho_ttymode);
		noecho_ttymode.c_lflag &= ~ECHO;
		tcsetattr(0, TCSANOW, &noecho_ttymode);
	}
	else {
		printf("%s (echo is on): ", prompt);
		(void)fflush(stdout);
	}

	/* get password */
	if (fgets(buf, len, stdin) == NULL)
		return(1);
	rip(buf);

	if (isatty(0)) {
		/* ECHO ON */
		tcsetattr(0, TCSANOW, &saved_ttymode);
		if (oldsig != SIG_IGN)
			(void)signal(SIGINT, oldsig);
		printf("\n");
	}
	return(0);
}

/* restore - restore terminal modes when user aborts command */
void gp_restore_ttymode()
{
	if (tcsetattr(0, TCSANOW, &saved_ttymode) == -1)
		perror("tcsetattr()");
	exit(1);
}

