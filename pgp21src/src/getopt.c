/*
**	@(#)getopt.c	2.5 (smail) 9/15/87
*/

/*
*  This is the AT&T public domain source for getopt(3).  It is the code
*  which was given out at the 1985 UNIFORUM conference in Dallas.
*   
*  There is no manual page.  That is because the one they gave out at
*  UNIFORUM was slightly different from the current System V Release 2
*  manual page.  The difference apparently involved a note about the
*  famous rules 5 and 6, recommending using white space between an
*  option and its first argument, and not grouping options that have
*  arguments.  Getopt itself is currently lenient about both of these
*  things.  White space is allowed, but not mandatory, and the last option
*  in a group can have an argument.  That particular version of the man
*  page evidently has no official existence.  The current SVR2 man page
*  reflects the actual behavor of this getopt.
*/

#include <string.h>

/*LINTLIBRARY*/
#ifndef NULL
#define NULL	0
#endif
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	extern int write();\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}

int	opterr = 1;
int	optind = 1;
int	optopt = 0;
char	*optarg = 0;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1) {
		if(optind >= argc || (argv[optind][0] != '+' &&
		   argv[optind][0] != '-') || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(EOF);
		}
		/* '+' for config options, '+' should not be in the opts list */
		if (argv[optind][0] == '+') {
			optarg = argv[optind++] + 1;
			return '+';
		}
	}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('\0');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('\0');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

