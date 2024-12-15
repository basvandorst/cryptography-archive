extern char *malloc(), *realloc();

# line 28 "ftpcmd.y"

#ifndef lint
static char sccsid[] = "@(#)ftpcmd.y	5.23 (Berkeley) 6/1/90";
#endif /* not lint */

#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/ftp.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <setjmp.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>
#ifdef NeXT
#include <arpa/nameser.h>
#include <resolv.h>
#endif

extern	struct sockaddr_in data_dest;
extern	int logged_in;
extern	struct passwd *pw;
extern	int guest;
extern	int logging;
extern	int type;
extern	int form;
extern	int debug;
extern	int timeout;
extern	int maxtimeout;
extern  int pdata;
extern	char hostname[], remotehost[];
extern	char proctitle[];
extern	char *globerr;
extern	int usedefault;
extern  int transflag;
extern  char tmpline[];
char	**glob();

off_t	restart_point;

static	int cmd_type;
static	int cmd_form;
static	int cmd_bytesz;
char	cbuf[512];
char	*fromname;

char	*index();
# define A 257
# define B 258
# define C 259
# define E 260
# define F 261
# define I 262
# define L 263
# define N 264
# define P 265
# define R 266
# define S 267
# define T 268
# define SP 269
# define CRLF 270
# define COMMA 271
# define STRING 272
# define NUMBER 273
# define USER 274
# define PASS 275
# define ACCT 276
# define REIN 277
# define QUIT 278
# define PORT 279
# define PASV 280
# define TYPE 281
# define STRU 282
# define MODE 283
# define RETR 284
# define STOR 285
# define APPE 286
# define MLFL 287
# define MAIL 288
# define MSND 289
# define MSOM 290
# define MSAM 291
# define MRSQ 292
# define MRCP 293
# define ALLO 294
# define REST 295
# define RNFR 296
# define RNTO 297
# define ABOR 298
# define DELE 299
# define CWD 300
# define LIST 301
# define NLST 302
# define SITE 303
# define STAT 304
# define HELP 305
# define NOOP 306
# define MKD 307
# define RMD 308
# define PWD 309
# define CDUP 310
# define STOU 311
# define SMNT 312
# define SYST 313
# define SIZE 314
# define MDTM 315
# define UMASK 316
# define IDLE 317
# define CHMOD 318
# define LEXERR 319
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 676 "ftpcmd.y"


extern jmp_buf errcatch;

#define	CMD	0	/* beginning of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional SP then STRING */
#define	ZSTR1	5	/* SP then optional STRING */
#define	ZSTR2	6	/* optional STRING after SP */
#define	SITECMD	7	/* SITE command */
#define	NSTR	8	/* Number followed by a string */

struct tab {
	char	*name;
	short	token;
	short	state;
	short	implemented;	/* 1 if command is implemented */
	char	*help;
};

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, ZSTR1, 1,	"<sp> password" },
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "SMNT", SMNT, ARGS, 0,	"(structure mount)" },
	{ "REIN", REIN, ARGS, 0,	"(reinitialize server state)" },
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "PASV", PASV, ARGS, 1,	"(set server in passive mode)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
	{ "REST", REST, ARGS, 1,	"(restart command)" },
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
	{ "ABOR", ABOR, ARGS, 1,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name ]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, SITECMD, 1,	"site-cmd [ <sp> arguments ]" },
	{ "SYST", SYST, ARGS, 1,	"(get type of operating system)" },
	{ "STAT", STAT, OSTR, 1,	"[ <sp> path-name ]" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
	{ "MKD",  MKD,  STR1, 1,	"<sp> path-name" },
	{ "XMKD", MKD,  STR1, 1,	"<sp> path-name" },
	{ "RMD",  RMD,  STR1, 1,	"<sp> path-name" },
	{ "XRMD", RMD,  STR1, 1,	"<sp> path-name" },
	{ "PWD",  PWD,  ARGS, 1,	"(return current directory)" },
	{ "XPWD", PWD,  ARGS, 1,	"(return current directory)" },
	{ "CDUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "XCUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "STOU", STOU, STR1, 1,	"<sp> file-name" },
	{ "SIZE", SIZE, OSTR, 1,	"<sp> path-name" },
	{ "MDTM", MDTM, OSTR, 1,	"<sp> path-name" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab sitetab[] = {
	{ "UMASK", UMASK, ARGS, 1,	"[ <sp> umask ]" },
	{ "IDLE", IDLE, ARGS, 1,	"[ <sp> maximum-idle-time ]" },
	{ "CHMOD", CHMOD, NSTR, 1,	"<sp> mode <sp> file-name" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab *
lookup(p, cmd)
	register struct tab *p;
	char *cmd;
{

	for (; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include <arpa/telnet.h>

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
getline(s, n, iop)
	char *s;
	register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
/* tmpline may contain saved command from urgent mode interruption */
	for (c = 0; tmpline[c] != '\0' && --n > 0; ++c) {
		*cs++ = tmpline[c];
		if (tmpline[c] == '\n') {
			*cs++ = '\0';
			if (debug)
				syslog(LOG_DEBUG, "command: %s", s);
			tmpline[0] = '\0';
			return(s);
		}
		if (c == 0)
			tmpline[0] = '\0';
	}
	while ((c = getc(iop)) != EOF) {
		c &= 0377;
		if (c == IAC) {
		    if ((c = getc(iop)) != EOF) {
			c &= 0377;
			switch (c) {
			case WILL:
			case WONT:
				c = getc(iop);
				printf("%c%c%c", IAC, DONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case DO:
			case DONT:
				c = getc(iop);
				printf("%c%c%c", IAC, WONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case IAC:
				break;
			default:
				continue;	/* ignore command */
			}
		    }
		}
		*cs++ = c;
		if (--n <= 0 || c == '\n')
			break;
	}
	if (c == EOF && cs == s)
		return (NULL);
	*cs++ = '\0';
	if (debug)
		syslog(LOG_DEBUG, "command: %s", s);
	return (s);
}

static int
toolong()
{
	time_t now;
	extern char *ctime();
	extern time_t time();

	reply(421,
	  "Timeout (%d seconds): closing control connection.", timeout);
	(void) time(&now);
	if (logging) {
		syslog(LOG_INFO,
			"User %s timed out after %d seconds at %s",
			(pw ? pw -> pw_name : "unknown"), timeout, ctime(&now));
	}
	dologout(1);
}

yylex()
{
	static int cpos, state;
	register char *cp, *cp2;
	register struct tab *p;
	int n;
	char c, *strpbrk();
	char *copy();

	for (;;) {
		switch (state) {

		case CMD:
			(void) signal(SIGALRM, toolong);
			(void) alarm((unsigned) timeout);
			if (getline(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, "You could at least say goodbye.");
				dologout(0);
			}
			(void) alarm(0);
#ifdef SETPROCTITLE
			if (strncasecmp(cbuf, "PASS", 4) != NULL)
				setproctitle("%s: %s", proctitle, cbuf);
#endif /* SETPROCTITLE */
			if ((cp = index(cbuf, '\r'))) {
				*cp++ = '\n';
				*cp = '\0';
			}
			if ((cp = strpbrk(cbuf, " \n")))
				cpos = cp - cbuf;
			if (cpos == 0)
				cpos = 4;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cmdtab, cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				*(char **)&yylval = p->name;
				return (p->token);
			}
			break;

		case SITECMD:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			cp = &cbuf[cpos];
			if ((cp2 = strpbrk(cp, " \n")))
				cpos = cp2 - cbuf;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cp);
			p = lookup(sitetab, cp);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					state = CMD;
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				*(char **)&yylval = p->name;
				return (p->token);
			}
			state = CMD;
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR1:
		case ZSTR1:
		dostr1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = state == OSTR ? STR2 : ++state;
				return (SP);
			}
			break;

		case ZSTR2:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				*(char **)&yylval = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case NSTR:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				state = STR1;
				return (NUMBER);
			}
			state = STR1;
			goto dostr1;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'I':
			case 'i':
				return (I);

			case 'L':
			case 'l':
				return (L);

			case 'N':
			case 'n':
				return (N);

			case 'P':
			case 'p':
				return (P);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			}
			break;

		default:
			fatal("Unknown state in scanner.");
		}
		yyerror((char *) 0);
		state = CMD;
		longjmp(errcatch,0);
	}
}

upper(s)
	register char *s;
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

char *
copy(s)
	char *s;
{
	char *p;
	extern char *malloc(), *strcpy();

	p = malloc((unsigned) strlen(s) + 1);
	if (p == NULL)
		fatal("Ran out of memory.");
	(void) strcpy(p, s);
	return (p);
}

help(ctab, s)
	struct tab *ctab;
	char *s;
{
	register struct tab *c;
	register int width, NCMDS;
	char *type;

	if (ctab == sitetab)
		type = "SITE ";
	else
		type = "";
	width = 0, NCMDS = 0;
	for (c = ctab; c->name != NULL; c++) {
		int len = strlen(c->name);

		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		register int i, j, w;
		int columns, lines;

		lreply(214, "The following %scommands are recognized %s.",
		    type, "(* =>'s unimplemented)");
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
#ifdef NOTDEF
			printf("   ");
#else
			fputs("   ", stdout);
#endif
			for (j = 0; j < columns; j++) {
				c = ctab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &ctab[NCMDS])
					break;
				w = strlen(c->name) + 1;
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
#ifdef NOTDEF
			printf("\r\n");
#else
			fputs("\r\n", stdout);
#endif
		}
		(void) fflush(stdout);
#ifdef NeXT
		if (!index(hostname, '.')) {
			if (!(_res.options&RES_INIT)) res_init();
			if (_res.defdname[0]) {
				reply(214,
					"Direct comments to ftp-bugs@%s.%s.",
					hostname, _res.defdname);
				return;
			}
		}
#endif
		reply(214, "Direct comments to ftp-bugs@%s.", hostname);
		return;
	}
	upper(s);
	c = lookup(ctab, s);
	if (c == (struct tab *)0) {
		reply(502, "Unknown command %s.", s);
		return;
	}
	if (c->implemented)
		reply(214, "Syntax: %s%s %s", type, c->name, c->help);
	else
		reply(214, "%s%-*s\t%s; unimplemented.", type, width,
		    c->name, c->help);
}

sizecmd(filename)
char *filename;
{
	switch (type) {
	case TYPE_L:
	case TYPE_I: {
		struct stat stbuf;
		if (stat(filename, &stbuf) < 0 ||
		    (stbuf.st_mode&S_IFMT) != S_IFREG)
			reply(550, "%s: not a plain file.", filename);
		else
			reply(213, "%lu", stbuf.st_size);
		break;}
	case TYPE_A: {
		FILE *fin;
		register int c;
		register long count;
		struct stat stbuf;
		fin = fopen(filename, "r");
		if (fin == NULL) {
			perror_reply(550, filename);
			return;
		}
		if (fstat(fileno(fin), &stbuf) < 0 ||
		    (stbuf.st_mode&S_IFMT) != S_IFREG) {
			reply(550, "%s: not a plain file.", filename);
			(void) fclose(fin);
			return;
		}

		count = 0;
		while((c=getc(fin)) != EOF) {
			if (c == '\n')	/* will get expanded to \r\n */
				count++;
			count++;
		}
		(void) fclose(fin);

		reply(213, "%ld", count);
		break;}
	default:
		reply(504, "SIZE not implemented for Type %c.", "?AEIL"[type]);
	}
}
int yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 75
# define YYLAST 332
int yyact[]={

    34,   111,   128,   203,   201,   198,   120,   192,   191,   188,
   179,   120,   112,   114,   113,   157,    91,   103,     4,     5,
    77,   175,    33,     6,     7,     8,     9,    10,    12,    13,
    14,   137,   106,    75,    73,   202,   200,   195,    11,    36,
    35,    19,    20,    18,    21,    16,    15,    28,    17,    22,
    23,    24,    25,    26,    27,    29,   184,    30,    31,    32,
   177,   176,   151,   150,   147,   146,   133,   132,   194,   124,
   105,   104,    98,    97,    96,    95,    57,    56,   199,   196,
   193,   190,   186,   183,   182,   181,   180,   174,   173,   172,
   171,   170,   169,   168,   167,   166,   165,   161,   156,   143,
   141,   131,   159,   130,   125,   123,   160,   122,   101,   121,
   110,   109,    69,    68,    65,    58,    54,    51,    40,   185,
   178,   127,   126,   118,   117,   116,   115,   108,   107,   100,
    99,    94,    93,    92,    89,    90,    71,    63,    53,    44,
    43,    42,    41,    88,    39,    84,    38,    37,   164,    86,
    85,    79,   187,   119,    80,    45,    81,    82,   158,   102,
    87,    83,    78,    76,    74,    72,     3,     2,     1,    46,
    47,    48,    49,    50,    52,     0,     0,    55,     0,     0,
    59,    60,    61,    62,     0,    64,     0,    66,    67,     0,
     0,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   134,   135,   136,     0,     0,     0,   138,   139,   140,
     0,     0,     0,     0,   142,     0,   144,   145,     0,     0,
     0,     0,     0,     0,   152,   153,   154,   155,     0,     0,
     0,     0,     0,     0,     0,     0,   129,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   148,   149,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   163,     0,     0,     0,   162,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   197,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   189 };
int yypact[]={

 -1000,  -256, -1000, -1000,  -122,  -123,  -125,  -152,  -127,  -128,
  -129,  -130, -1000, -1000, -1000, -1000, -1000,  -153, -1000,  -131,
  -154, -1000,  -193,  -155, -1000, -1000, -1000, -1000,  -132, -1000,
  -156, -1000, -1000,  -157,  -158, -1000,  -133,  -238,  -239,  -253,
 -1000,  -106,  -116,  -124,  -257,  -136,  -137,  -138,  -195,  -197,
  -139, -1000,  -140,  -255, -1000,  -199, -1000,  -240, -1000,  -141,
  -142,  -159,  -160,  -304,  -143, -1000,  -144,  -145, -1000, -1000,
  -146,  -262,  -161, -1000,  -163, -1000,  -165,  -202,  -166,  -147,
  -148, -1000,  -267,  -167, -1000, -1000, -1000,  -169, -1000, -1000,
 -1000,  -203,  -255,  -255,  -255, -1000,  -241, -1000,  -255,  -255,
  -255,  -170, -1000, -1000, -1000,  -255,  -171,  -255,  -255, -1000,
 -1000,  -205, -1000, -1000,  -207,  -255,  -255,  -255,  -255,  -172,
 -1000, -1000, -1000, -1000,  -258, -1000,  -162,  -162,  -262, -1000,
 -1000, -1000, -1000,  -118,  -174,  -175,  -176,  -177,  -178,  -179,
  -180, -1000,  -181, -1000,  -182,  -183, -1000,  -251,  -209,  -149,
 -1000,  -263,  -184,  -185,  -186,  -187, -1000,  -215, -1000, -1000,
 -1000, -1000, -1000, -1000,  -150, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000,  -188, -1000,  -264,  -264,  -189,
 -1000, -1000, -1000, -1000,  -265,  -266, -1000,  -190, -1000,  -201,
 -1000,  -234,  -191, -1000,  -255,  -268, -1000,  -192,  -235, -1000,
  -269,  -236,  -270, -1000 };
int yypgo[]={

     0,   168,   167,   166,   165,   164,   163,   162,   161,   160,
   155,   108,   152,   153,   158,   159 };
int yyr1[]={

     0,     1,     1,     1,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     3,     3,     4,
     5,     5,    13,     6,    14,    14,    14,     7,     7,     7,
     7,     7,     7,     7,     7,     8,     8,     8,     9,     9,
     9,    11,    15,    12,    10 };
int yyr2[]={

     0,     0,     5,     4,     9,     9,     9,     5,     9,     9,
     9,     9,    17,    11,    11,    11,     7,    11,     7,    11,
    11,     5,    11,     9,     5,     7,    11,     5,     9,     5,
    11,    11,     7,     7,     9,    13,    11,    15,    19,     9,
    13,    11,     5,    11,    11,     5,     5,    11,     9,     2,
     1,     2,     2,    23,     3,     3,     3,     3,     7,     3,
     7,     3,     3,     7,     5,     3,     3,     3,     3,     3,
     3,     3,     2,     3,     1 };
int yychk[]={

 -1000,    -1,    -2,    -3,   274,   275,   279,   280,   281,   282,
   283,   294,   284,   285,   286,   302,   301,   304,   299,   297,
   298,   300,   305,   306,   307,   308,   309,   310,   303,   311,
   313,   314,   315,   278,   256,   296,   295,   269,   269,   269,
   270,   269,   269,   269,   269,   -10,   -10,   -10,   -10,   -10,
   -10,   270,   -10,   269,   270,   -10,   270,   269,   270,   -10,
   -10,   -10,   -10,   269,   -10,   270,   -10,   -10,   270,   270,
   -10,   269,    -4,   272,    -5,   272,    -6,   273,    -7,   257,
   260,   262,   263,    -8,   261,   266,   265,    -9,   267,   258,
   259,   273,   269,   269,   269,   270,   269,   270,   269,   269,
   269,   -11,   -15,   272,   270,   269,   272,   269,   269,   270,
   270,   305,   316,   318,   317,   269,   269,   269,   269,   -13,
   273,   270,   270,   270,   271,   270,   269,   269,   269,   -13,
   270,   270,   270,   269,   -11,   -11,   -11,   272,   -11,   -11,
   -11,   270,   -11,   270,   -11,   -11,   270,   269,   -10,   -10,
   270,   269,   -11,   -11,   -11,   -11,   270,   273,   -14,   264,
   268,   259,   -14,   -13,   266,   270,   270,   270,   270,   270,
   270,   270,   270,   270,   270,   272,   270,   269,   269,   273,
   270,   270,   270,   270,   271,   269,   270,   -12,   273,   -12,
   270,   273,   273,   270,   269,   271,   270,   -11,   273,   270,
   271,   273,   271,   273 };
int yydef[]={

     1,    -2,     2,     3,     0,     0,     0,     0,     0,     0,
     0,     0,    74,    74,    74,    74,    74,    74,    74,     0,
     0,    74,     0,     0,    74,    74,    74,    74,     0,    74,
     0,    74,    74,     0,     0,    74,     0,     0,    50,     0,
     7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    21,     0,     0,    24,     0,    27,     0,    29,     0,
     0,     0,     0,     0,     0,    42,     0,     0,    45,    46,
     0,     0,     0,    49,     0,    51,     0,     0,     0,    57,
    59,    61,    62,     0,    65,    66,    67,     0,    68,    69,
    70,     0,     0,     0,     0,    16,     0,    18,     0,     0,
     0,     0,    71,    72,    25,     0,     0,     0,     0,    32,
    33,     0,    74,    74,     0,     0,     0,     0,     0,     0,
    52,     4,     5,     6,     0,     8,     0,     0,     0,    64,
     9,    10,    11,     0,     0,     0,     0,     0,     0,     0,
     0,    23,     0,    28,     0,     0,    34,     0,     0,     0,
    39,     0,     0,     0,     0,     0,    48,     0,    58,    54,
    55,    56,    60,    63,     0,    13,    14,    15,    17,    19,
    20,    22,    26,    30,    31,     0,    36,     0,     0,     0,
    41,    43,    44,    47,     0,     0,    35,     0,    73,     0,
    40,     0,     0,    37,     0,     0,    12,     0,     0,    38,
     0,     0,     0,    53 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"A",	257,
	"B",	258,
	"C",	259,
	"E",	260,
	"F",	261,
	"I",	262,
	"L",	263,
	"N",	264,
	"P",	265,
	"R",	266,
	"S",	267,
	"T",	268,
	"SP",	269,
	"CRLF",	270,
	"COMMA",	271,
	"STRING",	272,
	"NUMBER",	273,
	"USER",	274,
	"PASS",	275,
	"ACCT",	276,
	"REIN",	277,
	"QUIT",	278,
	"PORT",	279,
	"PASV",	280,
	"TYPE",	281,
	"STRU",	282,
	"MODE",	283,
	"RETR",	284,
	"STOR",	285,
	"APPE",	286,
	"MLFL",	287,
	"MAIL",	288,
	"MSND",	289,
	"MSOM",	290,
	"MSAM",	291,
	"MRSQ",	292,
	"MRCP",	293,
	"ALLO",	294,
	"REST",	295,
	"RNFR",	296,
	"RNTO",	297,
	"ABOR",	298,
	"DELE",	299,
	"CWD",	300,
	"LIST",	301,
	"NLST",	302,
	"SITE",	303,
	"STAT",	304,
	"HELP",	305,
	"NOOP",	306,
	"MKD",	307,
	"RMD",	308,
	"PWD",	309,
	"CDUP",	310,
	"STOU",	311,
	"SMNT",	312,
	"SYST",	313,
	"SIZE",	314,
	"MDTM",	315,
	"UMASK",	316,
	"IDLE",	317,
	"CHMOD",	318,
	"LEXERR",	319,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"cmd_list : /* empty */",
	"cmd_list : cmd_list cmd",
	"cmd_list : cmd_list rcmd",
	"cmd : USER SP username CRLF",
	"cmd : PASS SP password CRLF",
	"cmd : PORT SP host_port CRLF",
	"cmd : PASV CRLF",
	"cmd : TYPE SP type_code CRLF",
	"cmd : STRU SP struct_code CRLF",
	"cmd : MODE SP mode_code CRLF",
	"cmd : ALLO SP NUMBER CRLF",
	"cmd : ALLO SP NUMBER SP R SP NUMBER CRLF",
	"cmd : RETR check_login SP pathname CRLF",
	"cmd : STOR check_login SP pathname CRLF",
	"cmd : APPE check_login SP pathname CRLF",
	"cmd : NLST check_login CRLF",
	"cmd : NLST check_login SP STRING CRLF",
	"cmd : LIST check_login CRLF",
	"cmd : LIST check_login SP pathname CRLF",
	"cmd : STAT check_login SP pathname CRLF",
	"cmd : STAT CRLF",
	"cmd : DELE check_login SP pathname CRLF",
	"cmd : RNTO SP pathname CRLF",
	"cmd : ABOR CRLF",
	"cmd : CWD check_login CRLF",
	"cmd : CWD check_login SP pathname CRLF",
	"cmd : HELP CRLF",
	"cmd : HELP SP STRING CRLF",
	"cmd : NOOP CRLF",
	"cmd : MKD check_login SP pathname CRLF",
	"cmd : RMD check_login SP pathname CRLF",
	"cmd : PWD check_login CRLF",
	"cmd : CDUP check_login CRLF",
	"cmd : SITE SP HELP CRLF",
	"cmd : SITE SP HELP SP STRING CRLF",
	"cmd : SITE SP UMASK check_login CRLF",
	"cmd : SITE SP UMASK check_login SP octal_number CRLF",
	"cmd : SITE SP CHMOD check_login SP octal_number SP pathname CRLF",
	"cmd : SITE SP IDLE CRLF",
	"cmd : SITE SP IDLE SP NUMBER CRLF",
	"cmd : STOU check_login SP pathname CRLF",
	"cmd : SYST CRLF",
	"cmd : SIZE check_login SP pathname CRLF",
	"cmd : MDTM check_login SP pathname CRLF",
	"cmd : QUIT CRLF",
	"cmd : error CRLF",
	"rcmd : RNFR check_login SP pathname CRLF",
	"rcmd : REST SP byte_size CRLF",
	"username : STRING",
	"password : /* empty */",
	"password : STRING",
	"byte_size : NUMBER",
	"host_port : NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER",
	"form_code : N",
	"form_code : T",
	"form_code : C",
	"type_code : A",
	"type_code : A SP form_code",
	"type_code : E",
	"type_code : E SP form_code",
	"type_code : I",
	"type_code : L",
	"type_code : L SP byte_size",
	"type_code : L byte_size",
	"struct_code : F",
	"struct_code : R",
	"struct_code : P",
	"mode_code : S",
	"mode_code : B",
	"mode_code : C",
	"pathname : pathstring",
	"pathstring : STRING",
	"octal_number : NUMBER",
	"check_login : /* empty */",
};
#endif /* YYDEBUG */
#line 1 "/usr/lib/yaccpar"
/*	@(#)yaccpar 1.10 89/04/04 SMI; from S5R3 1.10	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	{ free(yys); free(yyv); return(0); }
#define YYABORT		{ free(yys); free(yyv); return(1); }
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** static variables used by the parser
*/
static YYSTYPE *yyv;			/* value stack */
static int *yys;			/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */

int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */
	unsigned yymaxdepth = YYMAXDEPTH;

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yyv = (YYSTYPE*)malloc(yymaxdepth*sizeof(YYSTYPE));
	yys = (int*)malloc(yymaxdepth*sizeof(int));
	if (!yyv || !yys)
	{
		yyerror( "out of memory" );
		return(1);
	}
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			(void)printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			yymaxdepth += YYMAXDEPTH;
			yyv = (YYSTYPE*)realloc((char*)yyv,
				yymaxdepth * sizeof(YYSTYPE));
			yys = (int*)realloc((char*)yys,
				yymaxdepth * sizeof(int));
			if (!yyv || !yys)
			{
				yyerror( "yacc stack overflow" );
				return(1);
			}
			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			(void)printf( "Received token " );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				(void)printf( "Received token " );
				if ( yychar == 0 )
					(void)printf( "end-of-file\n" );
				else if ( yychar < 0 )
					(void)printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					(void)printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						(void)printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					(void)printf( "Error recovery discards " );
					if ( yychar == 0 )
						(void)printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						(void)printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						(void)printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			(void)printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 2:
# line 107 "ftpcmd.y"
 {
			fromname = (char *) 0;
			restart_point = (off_t) 0;
		} break;
case 4:
# line 115 "ftpcmd.y"
 {
			user((char *) yypvt[-1]);
			free((char *) yypvt[-1]);
		} break;
case 5:
# line 120 "ftpcmd.y"
 {
			pass((char *) yypvt[-1]);
			free((char *) yypvt[-1]);
		} break;
case 6:
# line 125 "ftpcmd.y"
 {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, "PORT command successful.");
		} break;
case 7:
# line 134 "ftpcmd.y"
 {
			passive();
		} break;
case 8:
# line 138 "ftpcmd.y"
 {
			switch (cmd_type) {

			case TYPE_A:
				if (cmd_form == FORM_N) {
					reply(200, "Type set to A.");
					type = cmd_type;
					form = cmd_form;
				} else
					reply(504, "Form must be N.");
				break;

			case TYPE_E:
				reply(504, "Type E not implemented.");
				break;

			case TYPE_I:
				reply(200, "Type set to I.");
				type = cmd_type;
				break;

			case TYPE_L:
#if NBBY == 8
				if (cmd_bytesz == 8) {
					reply(200,
					    "Type set to L (byte size 8).");
					type = cmd_type;
				} else
					reply(504, "Byte size must be 8.");
#else /* NBBY == 8 */
				UNIMPLEMENTED for NBBY != 8
#endif /* NBBY == 8 */
			}
		} break;
case 9:
# line 173 "ftpcmd.y"
 {
			switch (yypvt[-1]) {

			case STRU_F:
				reply(200, "STRU F ok.");
				break;

			default:
				reply(504, "Unimplemented STRU type.");
			}
		} break;
case 10:
# line 185 "ftpcmd.y"
 {
			switch (yypvt[-1]) {

			case MODE_S:
				reply(200, "MODE S ok.");
				break;

			default:
				reply(502, "Unimplemented MODE type.");
			}
		} break;
case 11:
# line 197 "ftpcmd.y"
 {
			reply(202, "ALLO command ignored.");
		} break;
case 12:
# line 201 "ftpcmd.y"
 {
			reply(202, "ALLO command ignored.");
		} break;
case 13:
# line 205 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				retrieve((char *) 0, (char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 14:
# line 212 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				store((char *) yypvt[-1], "w", 0);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 15:
# line 219 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				store((char *) yypvt[-1], "a", 0);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 16:
# line 226 "ftpcmd.y"
 {
			if (yypvt[-1])
				send_file_list(".");
		} break;
case 17:
# line 231 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL) 
				send_file_list((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 18:
# line 238 "ftpcmd.y"
 {
			if (yypvt[-1])
				retrieve("/bin/ls -lgA", "");
		} break;
case 19:
# line 243 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				retrieve("/bin/ls -lgA %s", (char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 20:
# line 250 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				statfilecmd((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 21:
# line 257 "ftpcmd.y"
 {
			statcmd();
		} break;
case 22:
# line 261 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				delete((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 23:
# line 268 "ftpcmd.y"
 {
			if (fromname) {
				renamecmd(fromname, (char *) yypvt[-1]);
				free(fromname);
				fromname = (char *) 0;
			} else {
				reply(503, "Bad sequence of commands.");
			}
			free((char *) yypvt[-1]);
		} break;
case 24:
# line 279 "ftpcmd.y"
 {
			reply(225, "ABOR command successful.");
		} break;
case 25:
# line 283 "ftpcmd.y"
 {
			if (yypvt[-1])
				cwd(pw->pw_dir);
		} break;
case 26:
# line 288 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				cwd((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 27:
# line 295 "ftpcmd.y"
 {
			help(cmdtab, (char *) 0);
		} break;
case 28:
# line 299 "ftpcmd.y"
 {
			register char *cp = (char *)yypvt[-1];

			if (strncasecmp(cp, "SITE", 4) == 0) {
				cp = (char *)yypvt[-1] + 4;
				if (*cp == ' ')
					cp++;
				if (*cp)
					help(sitetab, cp);
				else
					help(sitetab, (char *) 0);
			} else
				help(cmdtab, (char *) yypvt[-1]);
		} break;
case 29:
# line 314 "ftpcmd.y"
 {
			reply(200, "NOOP command successful.");
		} break;
case 30:
# line 318 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				makedir((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 31:
# line 325 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				removedir((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 32:
# line 332 "ftpcmd.y"
 {
			if (yypvt[-1])
				pwd();
		} break;
case 33:
# line 337 "ftpcmd.y"
 {
			if (yypvt[-1])
				cwd("..");
		} break;
case 34:
# line 342 "ftpcmd.y"
 {
			help(sitetab, (char *) 0);
		} break;
case 35:
# line 346 "ftpcmd.y"
 {
			help(sitetab, (char *) yypvt[-1]);
		} break;
case 36:
# line 350 "ftpcmd.y"
 {
			int oldmask;

			if (yypvt[-1]) {
				oldmask = umask(0);
				(void) umask(oldmask);
				reply(200, "Current UMASK is %03o", oldmask);
			}
		} break;
case 37:
# line 360 "ftpcmd.y"
 {
			int oldmask;

			if (yypvt[-3]) {
				if ((yypvt[-1] == -1) || (yypvt[-1] > 0777)) {
					reply(501, "Bad UMASK value");
				} else {
					oldmask = umask(yypvt[-1]);
					reply(200,
					    "UMASK set to %03o (was %03o)",
					    yypvt[-1], oldmask);
				}
			}
		} break;
case 38:
# line 375 "ftpcmd.y"
 {
			if (yypvt[-5] && (yypvt[-1] != NULL)) {
				if (yypvt[-3] > 0777)
					reply(501,
				"CHMOD: Mode value must be between 0 and 0777");
				else if (chmod((char *) yypvt[-1], yypvt[-3]) < 0)
					perror_reply(550, (char *) yypvt[-1]);
				else
					reply(200, "CHMOD command successful.");
			}
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 39:
# line 389 "ftpcmd.y"
 {
			reply(200,
			    "Current IDLE time limit is %d seconds; max %d",
				timeout, maxtimeout);
		} break;
case 40:
# line 395 "ftpcmd.y"
 {
			if (yypvt[-1] < 30 || yypvt[-1] > maxtimeout) {
				reply(501,
			"Maximum IDLE time must be between 30 and %d seconds",
				    maxtimeout);
			} else {
				timeout = yypvt[-1];
				(void) alarm((unsigned) timeout);
				reply(200,
				    "Maximum IDLE time set to %d seconds",
				    timeout);
			}
		} break;
case 41:
# line 409 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				store((char *) yypvt[-1], "w", 1);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 42:
# line 416 "ftpcmd.y"
 {
#ifdef unix
#ifdef BSD
			reply(215, "UNIX Type: L%d Version: BSD-%d",
				NBBY, BSD);
#else /* BSD */
			reply(215, "UNIX Type: L%d", NBBY);
#endif /* BSD */
#else /* unix */
			reply(215, "UNKNOWN Type: L%d", NBBY);
#endif /* unix */
		} break;
case 43:
# line 437 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL)
				sizecmd((char *) yypvt[-1]);
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 44:
# line 454 "ftpcmd.y"
 {
			if (yypvt[-3] && yypvt[-1] != NULL) {
				struct stat stbuf;
				if (stat((char *) yypvt[-1], &stbuf) < 0)
					perror_reply(550, "%s", (char *) yypvt[-1]);
				else if ((stbuf.st_mode&S_IFMT) != S_IFREG) {
					reply(550, "%s: not a plain file.",
						(char *) yypvt[-1]);
				} else {
					register struct tm *t;
					struct tm *gmtime();
					t = gmtime(&stbuf.st_mtime);
					reply(213,
					    "19%02d%02d%02d%02d%02d%02d",
					    t->tm_year, t->tm_mon+1, t->tm_mday,
					    t->tm_hour, t->tm_min, t->tm_sec);
				}
			}
			if (yypvt[-1] != NULL)
				free((char *) yypvt[-1]);
		} break;
case 45:
# line 476 "ftpcmd.y"
 {
			reply(221, "Goodbye.");
			dologout(0);
		} break;
case 46:
# line 481 "ftpcmd.y"
 {
			yyerrok;
		} break;
case 47:
# line 486 "ftpcmd.y"
 {
			char *renamefrom();

			restart_point = (off_t) 0;
			if (yypvt[-3] && yypvt[-1]) {
				fromname = renamefrom((char *) yypvt[-1]);
				if (fromname == (char *) 0 && yypvt[-1]) {
					free((char *) yypvt[-1]);
				}
			}
		} break;
case 48:
# line 498 "ftpcmd.y"
 {
			long atol();

			fromname = (char *) 0;
			restart_point = yypvt[-1];
			reply(350, "Restarting at %ld. %s", restart_point,
			    "Send STORE or RETRIEVE to initiate transfer.");
		} break;
case 50:
# line 512 "ftpcmd.y"
 {
			*(char **)&(yyval) = (char *)calloc(1, sizeof(char));
		} break;
case 53:
# line 523 "ftpcmd.y"
 {
			register char *a, *p;

			a = (char *)&data_dest.sin_addr;
			a[0] = yypvt[-10]; a[1] = yypvt[-8]; a[2] = yypvt[-6]; a[3] = yypvt[-4];
			p = (char *)&data_dest.sin_port;
			p[0] = yypvt[-2]; p[1] = yypvt[-0];
			data_dest.sin_family = AF_INET;
		} break;
case 54:
# line 535 "ftpcmd.y"
 {
		yyval = FORM_N;
	} break;
case 55:
# line 539 "ftpcmd.y"
 {
		yyval = FORM_T;
	} break;
case 56:
# line 543 "ftpcmd.y"
 {
		yyval = FORM_C;
	} break;
case 57:
# line 549 "ftpcmd.y"
 {
		cmd_type = TYPE_A;
		cmd_form = FORM_N;
	} break;
case 58:
# line 554 "ftpcmd.y"
 {
		cmd_type = TYPE_A;
		cmd_form = yypvt[-0];
	} break;
case 59:
# line 559 "ftpcmd.y"
 {
		cmd_type = TYPE_E;
		cmd_form = FORM_N;
	} break;
case 60:
# line 564 "ftpcmd.y"
 {
		cmd_type = TYPE_E;
		cmd_form = yypvt[-0];
	} break;
case 61:
# line 569 "ftpcmd.y"
 {
		cmd_type = TYPE_I;
	} break;
case 62:
# line 573 "ftpcmd.y"
 {
		cmd_type = TYPE_L;
		cmd_bytesz = NBBY;
	} break;
case 63:
# line 578 "ftpcmd.y"
 {
		cmd_type = TYPE_L;
		cmd_bytesz = yypvt[-0];
	} break;
case 64:
# line 584 "ftpcmd.y"
 {
		cmd_type = TYPE_L;
		cmd_bytesz = yypvt[-0];
	} break;
case 65:
# line 591 "ftpcmd.y"
 {
		yyval = STRU_F;
	} break;
case 66:
# line 595 "ftpcmd.y"
 {
		yyval = STRU_R;
	} break;
case 67:
# line 599 "ftpcmd.y"
 {
		yyval = STRU_P;
	} break;
case 68:
# line 605 "ftpcmd.y"
 {
		yyval = MODE_S;
	} break;
case 69:
# line 609 "ftpcmd.y"
 {
		yyval = MODE_B;
	} break;
case 70:
# line 613 "ftpcmd.y"
 {
		yyval = MODE_C;
	} break;
case 71:
# line 619 "ftpcmd.y"
 {
		/*
		 * Problem: this production is used for all pathname
		 * processing, but only gives a 550 error reply.
		 * This is a valid reply in some cases but not in others.
		 */
		if (logged_in && yypvt[-0] && strncmp((char *) yypvt[-0], "~", 1) == 0) {
			*(char **)&(yyval) = *glob((char *) yypvt[-0]);
			if (globerr != NULL) {
				reply(550, globerr);
				yyval = NULL;
			}
			free((char *) yypvt[-0]);
		} else
			yyval = yypvt[-0];
	} break;
case 73:
# line 641 "ftpcmd.y"
 {
		register int ret, dec, multby, digit;

		/*
		 * Convert a number that was read as decimal number
		 * to what it would be if it had been read as octal.
		 */
		dec = yypvt[-0];
		multby = 1;
		ret = 0;
		while (dec) {
			digit = dec%10;
			if (digit > 7) {
				ret = -1;
				break;
			}
			ret += digit * multby;
			multby *= 8;
			dec /= 10;
		}
		yyval = ret;
	} break;
case 74:
# line 666 "ftpcmd.y"
 {
		if (logged_in)
			yyval = 1;
		else {
			reply(530, "Please login with USER and PASS.");
			yyval = 0;
		}
	} break;
	}
	goto yystack;		/* reset registers in driver code */
}
