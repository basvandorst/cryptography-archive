/*===== Module Header ============================================
** Project:
**	LOGTALK
**
** Synopsis:
**	logtalk [-s] [-w width] [-i iprompt] [-u uprompt] [-b bar]
**		[-o output] [input]...
**
** Description:
**	filter log file from talk session into readable form.
**	if no input file is specified, logtalk reads from stdin.
**	if no output file is specified with '-o' it writes to stdout.
**	The input file can be produced by "talk <user> | tee <file>".
**
** Options:
**	-s	output dialog side by side (implies "-w 80")
**	-w wid	word wrap lines at or before column 'wid'
**	-i ip	prepend talker's lines with 'ip' ("I prompt")
**	-u up	prepend responder's lines with 'up' ("You prompt")
**		(for -s: title for talker's and responder's columns)
**	-b bar	for -s only: use 'bar' to separate columns
**	-o out	outputfile
**
** See Also:
** 	tee(1), ltalk(1)
**
** Compile Command:
**	cc -o logtalk logtalk.c
**
** System Environment:
**	SunOS 4.1.1, for ANSI compatible terminals (e.g. xterm).
**	This will ONLY work for log files from ANSI compatible terminals.
**----------------------------------------------------------------
** $Log:	logtalk.c,v $
**	Revision 1.3  92/05/27  13:00:42  ruprecht
**	bugfix: distinguish talkers correctly if no "ESC..." before "---"
**	
**	Revision 1.2  92/04/23  15:27:47  ruprecht
**	logtalk now works even for non-trivial logs.
**	supports side-by-side output, various options, multiple input files.
**	
**	Revision 1.1  92/02/21  17:55:55  ruprecht
**	fixed a few obvious bugs. cleaned up the thing.
**	
**	Revision 1.0  92/02/20  15:08:51  ruprecht
**	Initial revision
**	
**==============================================================*/

/*----- Includes -----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>

/*----- Defines ------------------------------------------------*/
#define CNULL	((char *)0)
#define MAXGETS 8096		/* fgets buffer size */
#define MAXLINE 256		/* maximum terminal line length */
#define DEF_BAR_ROW 13		/* hope this is a useful fallback */
#define DEF_WIDTH 80		/* default line width for SIDEBYSIDE */

#define pexit(s) ( perror(s), exit(1) )

/*----- Global Declarations ------------------------------------*/
static char rcsid[] = "$Id: logtalk.c,v 1.3 92/05/27 13:00:42 ruprecht Exp $";

/* these strings can be modified to taste */
static char Sprompt[2][3] = { "> ", "< " }; /* default prompt for my lines */
static char Senvtalk[2][6] = { "ITALK", "UTALK" }; /* env vars for prompts */
static char StheEnd[] = "[end]\n\n"; /* dialog terminator */
static char DefSidebar[] = " | "; /* def. separator for side-by-side dialog */

char Usage[] = "Usage: %s [-options] user[@hostname][#tty] [user...]\n\
options:\n\
	-o file	   log file (default: no logging)\n\
	-s	   log talk side by side\n\
	-w width   fold logged talk lines at width\n\
	-b bar	   column separator for option -s (default: \" | \")\n\
	-i prompt  prompt for logged talkers lines\n\
	-u prompt  prompt for logged responders lines\n";

/* string constants */
static char Sbar[] = "-----";
static char Sescbar[] = ";1H-----";
static char Sring[] = "[Ring";
static char Sconnect[] = "[Connection established";
static char Sconnect1[] = "[Connected";
static char Sclosing[] = "[Connection closing. Exiting]";
static char Ssing[] = "closing. Exiting]";
static char Sclosed[] = "[Connection closed. Exiting]";
static char Ssed[] = "closed. Exiting]";

/* global variables. not modular but convenient for such a simple program */
static enum State { Talk=0, Response=1, New, Bar, Header } state = New;
static struct {			/* data structure for dialog lines */
    char *prompt;		/* prompt prefix for output */
    char str[MAXLINE];		/* buffer */
    char *ptr;			/* pointer to next position in buffer */
    char *eol;			/* pointer to current end of line */
    int row;			/* screen row. flush buffer on change */
    int realrow;
    int realcol;
} talk[2];
static int bar;
static FILE *outfile = NULL;	/* output file handle */
static int sidebyside = 1;
static char *sidebar = DefSidebar;
static char myname[64];
int width[2];

/*----- Local Functions ----------------------------------------*/
static void ProcessFile(), ProcessLine();
static void FlushTalk(), OutputSideBySide();
void InitTalk();
static char *ProcessEscape(), *EndTalk(), *FoldStr();

char* ctime();

/*===== Function Header ========================================*/
int InitLog(argc, argv)
    int argc;
    char **argv;
/*----------------------------------------------------------------
** Description:
**	state machine waiting for valid input, outputs some info.
**	talk dialog gets processed by ProcessLine()
**
** Return:
**	0 if ok, 1 on file open error.
**----------------------------------------------------------------
** History:
**	19-FEB-1992/Ru: new
**	21-FEB-1992/Ru: use static variables. Initialize talk[i].row
**	15-APR-1992/Ru: use global row. don't support script.
**	16-APR-1992/Ru: join Bar and Header states.
**==============================================================*/
{
    FILE *infile;		/* input file */
    int c, err=0;
    int wid=0;
    extern char *optarg;
    extern int optind;
    struct passwd *pw;
    
    if((pw = (struct passwd *) getpwuid(getuid())) == 0)
    {
	panic("Who are you?");
	yytalkabort(1);
    }
    
    strncpy(myname, pw->pw_name, 63);
    myname[62] = 0;
    myname[strlen(myname)+2] = 0;
    myname[strlen(myname)+1] = ' ';
    talk[Talk].prompt = myname;
    talk[Response].prompt = "she";
    
    /* process options */
    while ( ( c = getopt(argc, argv, "dsw:b:i:u:o:") ) != -1 )
    {
	switch ( c )
	{
	case 's':
	    sidebyside = 0;
	    break;
	case 'w':
	    if ( ( wid = atoi(optarg) ) < 0 )
		wid = 0;
	    break;
	case 'b':
	    sidebar = optarg;
	    break;
	case 'i':
	    talk[Talk].prompt = optarg;
	    break;
	case 'u':
	    talk[Response].prompt = optarg;
	    break;
	case 'o':
	    if ( NULL == ( outfile = fopen(optarg, "a") ) )
		pexit("talk");
	    break;
	case 'd':
/*	    _res.options |= RES_DEBUG;*/
	    break;
	default:
	    fprintf(stderr, Usage, "ytalk");
	    exit(2);
	}
    }
    if ( sidebyside )
    {
	if ( wid == 0 )
		wid = DEF_WIDTH;
	wid = ( wid - strlen(sidebar) ) / 2;
    }

    /* make sure prompts and width are set */
    for ( c=0; c<2; c++ )
    {
	if ( ( talk[c].prompt == NULL )
	    && ( ( talk[c].prompt = getenv(Senvtalk[c]) ) == NULL ) )
	{
	    talk[c].prompt = ( sidebyside ? "" : Sprompt[c] );
	}
	if ( sidebyside )
	    width[c] = wid;
	else if ( wid
		 && ( width[c] = wid - strlen(talk[1].prompt) ) < 0 )
	    width[c] = 0;
    }
}

/*===== Function Header ========================================*/
void ProcessChar(which, ch)
    char ch;
    int which;
/*----------------------------------------------------------------
** Description:
**	process str char by char. pass ESC to ProcessEscape.
**	insert chars in string, flush buffer if new row.
**
** Return:
**	void
**----------------------------------------------------------------
** History:
**	19-FEB-1992/Ru: new
**	21-FEB-1992/Ru: use static variables. advance row on '\n'.
**	15-APR-1992/Ru: return to beg of line on CR. Flush on char.
**==============================================================*/
{
    if (outfile == NULL)
	    return;
    
    /* determine state */
    state = ( which == 0 ) ? Talk : Response;
	    
    switch ( ch )
    {
    case 8:		/* Backspace */
    case 127:		/* Delete */
    case -2:		/* RUB - translated delete */
	if (talk[state].realcol > 1)
	{
	    talk[state].realcol--;
	    talk[state].eol = talk[state].str + talk[state].realcol - 1;
	}
	break;
    case 21:
	talk[state].realcol = 1;
	talk[state].eol = talk[state].ptr = talk[state].str;
	break;
    case 23:
	while(talk[state].realcol > 1)
	{
	    talk[state].realcol--;
	    talk[state].eol = talk[state].str + talk[state].realcol - 1;
	    if (*talk[state].str == ' ') break;
	}
	break;
    case '\t':		/* tab -> advance column */
	talk[state].realcol = ( talk[state].realcol / 8 ) * 8 + 9;
	break;
    case '\n':		/* cr -> return to beginning of line */
	talk[state].realcol = 1;
	talk[state].realrow++;
	break;
    default:		/* insert printable chars in buffer */
	if ( isprint(ch) )
	{
	    /* new row -> flush buffer */
	    if ( talk[state].row != talk[state].realrow )
	    {
		FlushTalk(state);
		talk[state].row = talk[state].realrow;
	    }
	    
	    /* adjust ptr */
	    talk[state].ptr = talk[state].str + talk[state].realcol - 1;
	    
	    /* fill with spaces if cursor jumped ahead */
	    while ( talk[state].eol < talk[state].ptr )
		    *(talk[state].eol)++ = ' ';
	    
	    *(talk[state].ptr)++ = ch;
	    if ( talk[state].eol < talk[state].ptr )
		    talk[state].eol = talk[state].ptr;
	    talk[state].realcol++;
	}
	break;
    }
}

/*===== Function Header ========================================*/
void InitTalk(you)
/*----------------------------------------------------------------
** Description:
**	Initialize values of struct talk.
**
** Return:
**	void
**----------------------------------------------------------------
** History:
**	21-FEB-1992/Ru: new
**	24-FEB-1992/Ru: support ITALK, UTALK
**	15-APR-1992/Ru: init to DEF_BAR_ROW, talk[i].eol
**==============================================================*/
char* you;
{
    char *str;
    int now;

    if (outfile == NULL) return;
    
    if (you != NULL && *you != 0)
    {
	talk[Response].prompt = you;
    }
    
    now = time(0);
    fprintf(outfile, "From %s %s", myname, ctime(&now));
    if (talk[Response].prompt != NULL && *talk[Response].prompt)
	    fprintf(outfile, "To: %s\n\n", talk[Response].prompt);
    
    if (sidebyside && (*talk[Talk].prompt || *talk[Response].prompt))
	    fprintf(outfile, "%-*.*s%s%.*s\n\n",
		    width[Talk], width[Talk], talk[Talk].prompt,
		    sidebar,
		    width[Response], talk[Response].prompt);
    
    talk[Talk].eol = talk[Talk].ptr = talk[Talk].str;
    talk[Talk].row = 1;
    talk[Talk].realrow = 1;
    talk[Talk].realcol = 1;
    talk[Response].eol = talk[Response].ptr = talk[Response].str;
    talk[Response].row = 1;
    talk[Response].realrow = 1;
    talk[Response].realcol = 1;
    state = New;
}

/*===== Function Header ========================================*/
static void FlushTalk(s)
    enum State s;
/*----------------------------------------------------------------
** Description:
**	output str and reset ptr.
**
** Return:
**	void
**----------------------------------------------------------------
** History:
**	19-FEB-1992/Ru: new
**	21-FEB-1992/Ru: use static var's.
**	15-APR-1992/Ru: support eol
**	23-APR-1992/Ru: fix bug if width[s] == 0
**==============================================================*/
{
    if (state == New) return;
    
    *talk[s].eol = '\0';

    if ( sidebyside )
    {
	static char buf[2][MAXLINE];
	
	if ( *buf[s] )
	{
	    if ( state == Talk )
		OutputSideBySide(buf[s], "");
	    else
		OutputSideBySide("", buf[s]);
	}
	
	strcpy(buf[s], talk[s].str);
	
	if ( *buf[(s==Response)?Talk:Response] )
	{
	    OutputSideBySide(buf[Talk], buf[Response]);
	    *buf[Talk] = *buf[Response] = '\0';
	}
    }
    else			/* not SIDEBYSIDE */
    {
	if ( talk[s].eol != talk[s].str ) /* output only if not empty */
	{
	    if ( width[s] )
	    {
		char *ptr = talk[s].str, *pnext;

		/* skip leading whitespace */
		while ( isspace(*ptr) )
		    ptr++;
	    
		/* output string in chunks fitting in width */
		while ( *ptr )
		{
		    pnext = FoldStr(ptr, width[s]);
		    fprintf(outfile, "%s%.*s\n", talk[s].prompt,
			    width[s], ptr);
		    ptr = pnext;
		    while ( isspace(*ptr) )
			ptr++;
		}
	    }
	    else		/* width == 0 -> just print */
		fprintf(outfile, "%s%s\n", talk[s].prompt, talk[s].str);
	}
    }
    talk[s].eol = talk[s].ptr = talk[s].str;
}

/*===== Function Header ========================================*/
static void OutputSideBySide(left, right)
    char *left, *right;
/*----------------------------------------------------------------
** Description:
**	output buffers side by side
**
** Return:
**	void
**----------------------------------------------------------------
** History:
**	16-APR-1992/Ru: new
**==============================================================*/
{
    char *lnext, *rnext;

    for(;;)
    {
	while ( isspace(*left) )
	    left++;
	while ( isspace(*right) )
	    right++;
	lnext = ( *left ? FoldStr(left, width[Talk]) : left );
	rnext = ( *right ? FoldStr(right, width[Response]) : right );
	if ( ( *left == 0 ) && ( *right == 0 ) )
	    return;
	
	fprintf(outfile, "%-*.*s%s%.*s\n", width[Talk], width[Talk], left,
		sidebar, width[Response], right);
	left = lnext;
	right = rnext;
    }
}

/*===== Function Header ========================================*/
static char *FoldStr(str, wid)
    char *str;
    int wid;
/*----------------------------------------------------------------
** Description:
**	limit str to wid characters
**
** Return:
**	pointer to rest of string
**----------------------------------------------------------------
** History:
**	16-APR-1992/Ru: new
**==============================================================*/
{
    char *end;
    int len = strlen(str);

    if ( wid && ( len > wid ) )	/* does str fit in wid? */
    {
	end = str + wid;	/* set max wid */
	/* search backwards for word break, fold there if found. */
	while ( ( isspace(*end) == 0 ) && ( end > str ) )
	    end--;
	if ( end > str )
	    *end++ = '\0';
	else
	    end = str+wid;
    }
    else
	end = str + len;	/* string fits, return end of string */

    return end;
}

/*===== Function Header ========================================*/
char *EndLog()
/*----------------------------------------------------------------
** Description:
**	flush all buffers and mark end of dialog
**
** Return:
**	(char *)NULL for convenience of ProcessEscape;
**----------------------------------------------------------------
** History:
*	20-FEB-1992/Ru: new
**	21-FEB-1992/Ru: use static var's.
**==============================================================*/
{
    if (outfile == NULL) return CNULL;
    if (state == New)
    {
	fclose(outfile);
	return CNULL;
    }
    
    FlushTalk(!state);
    FlushTalk(state);		/* output current state last */
    FlushTalk(!state);		/* reflush to force everything out */
    fprintf(outfile, StheEnd);
    fclose(outfile);
    state = New;
    return CNULL;
}
