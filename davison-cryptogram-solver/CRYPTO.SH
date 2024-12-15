Path: uunet!tektronix!tekgen!tekred!games
From: games@tekred.TEK.COM
Newsgroups: comp.sources.games
Subject: v05i066:  crypto - the cryptogram puzzler's friend
Message-ID: <3061@tekred.TEK.COM>
Date: 21 Sep 88 18:49:16 GMT
Sender: billr@tekred.TEK.COM
Reply-To: davison@drivax.UUCP (Wayne Davison)
Lines: 1835
Approved: billr@saab.CNA.TEK.COM

Submitted by: davison@drivax.UUCP (Wayne Davison)
Comp.sources.games: Volume 5, Issue 66
Archive-name: crypto

	[This is a neat program! I compiled and ran it on a Sun with
	 no problems. Looks like it will work on SysV with a minor edit
	 to the Makefile.	-br]

#! /bin/sh
# This is a shell archive.  Remove anything before this line, then unpack
# it by saving it into a file and typing "sh file".  To overwrite existing
# files, type "sh file -c".  You can also feed this as standard input via
# unshar, or by typing "sh <file", e.g..  If this archive is complete, you
# will see the following message at the end:
#		"End of shell archive."
# Contents:  README Makefile crypto.c crypto.man puzzles
# Wrapped by billr@saab on Wed Sep 21 11:49:01 1988
PATH=/bin:/usr/bin:/usr/ucb ; export PATH
if test -f 'README' -a "${1}" != "-c" ; then 
  echo shar: Will not clobber existing file \"'README'\"
else
echo shar: Extracting \"'README'\" \(1528 characters\)
sed "s/^X//" >'README' <<'END_OF_FILE'
XCrypto.c is my contribution to the world of cryptogram puzzle solvers.  I wrote
Xit a few months ago to encode and solve fortunes, and to decode the cryptograms
Xfound in _Games_ magazine.  Included is an example multi-puzzle file which will
Xlet you try out the program.  After compilation, type "crypto puzzles" and type
Xa "?" for a brief summary of commands.  Consult the manpage for more info.
X
XThe program is written using curses, and as such is (hopefully) quite portable.
XEdit the Makefile if the curses library link is not "-lcurses -ltermcap", or if
Xyour system uses strrchr() rather than rindex().
X
XCrypto helps those who enjoy solving cryptograms by:
X    o	Encoding regular text
X    o	Using pre-encoded files which may or may not include the solution
X    o	Interactivley placing letters into the puzzle
X    o	Non-interactivley encoding/decoding puzzle text
X    o	Searching /usr/dict/words for possible matches of a particular word
X    o	Creating/accessing multi-puzzle files
X
XFor example, execute the command:
X
X	fortune | crypto -e >>puzz
X
Xten times, then the command:
X
X	crypto puzz
X
Xto solve all ten puzzles at your leisure.  You can switch from puzzle to
Xpuzzle, and save the current state of any one or all ten puzzles.  You can
Xeven cheat by having the program correct one (or more) letters in the puzzle.
X--
X Wayne Davison					      ...amdahl!drivax!davison
X=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
X   bcd'f bf g hif ij jkd fi cihlm fnmcm fompmdqikchr aidqmojkh sortfiuogpc?
X
END_OF_FILE
if test 1528 -ne `wc -c <'README'`; then
    echo shar: \"'README'\" unpacked with wrong size!
fi
# end of 'README'
fi
if test -f 'Makefile' -a "${1}" != "-c" ; then 
  echo shar: Will not clobber existing file \"'Makefile'\"
else
echo shar: Extracting \"'Makefile'\" \(201 characters\)
sed "s/^X//" >'Makefile' <<'END_OF_FILE'
X##
X##	Simple Makefile:
X##
X
X##  Edit appropriately.
XLIBS	= -lcurses -ltermcap
X##  Choose one:
XCFLAGS	= -O
X## or
X#CFLAGS	= -O -Drindex=strrchr
X
Xcrypto:		crypto.c
X	cc $(CFLAGS) crypto.c -o crypto $(LIBS)
END_OF_FILE
if test 201 -ne `wc -c <'Makefile'`; then
    echo shar: \"'Makefile'\" unpacked with wrong size!
fi
# end of 'Makefile'
fi
if test -f 'crypto.c' -a "${1}" != "-c" ; then 
  echo shar: Will not clobber existing file \"'crypto.c'\"
else
echo shar: Extracting \"'crypto.c'\" \(36106 characters\)
sed "s/^X//" >'crypto.c' <<'END_OF_FILE'
X#include <stdio.h>
X#include <curses.h>
X#include <signal.h>
X
X#define	COLS	80
X#define	ROWS	(24-2)
X
X#define INIT_GUESS	1
X#define INIT_CYPHER	2
X
X#define DRAW_CRYPT_MSGS	0
X#define DRAW_CRYPT	1
X#define DRAW_MSGS	2
X#define DRAW_HINT_ON	3
X#define DRAW_HINT_OFF	4
X
X#define BUFF_SIZE	8176
X#define PUZZLE_MAX	(ROWS-2)
X
Xchar *buffer, *buff_cp, *buff_titles;		/* main buffer pointers */
Xchar *cypher, *guess, *used;			/* puzzle-specific pointers */
Xchar *cl_cypher, *cl_guess;			/* pointers into command-line */
Xchar string[90], string2[40];			/* general purpose strings */
X
Xstruct {					/* the multi-puzzle structure */
X    char *title, *ptr;
X    int  length;
X    char solved, changed;
X    int  voffset, hoffset;
X    int  maxrow, maxcol;
X    char *hint_cp, *hint_end;
X} puzzle[PUZZLE_MAX], *puzz;
X
Xint  puz_count = -1, puz_number, spacing;
X
Xchar *status_msg = NULL;
Xchar *myname, *mytty, *ttyname();
X
X
Xint  cypher_flag = 0, draw_mode = DRAW_CRYPT_MSGS;
Xchar upper_flag  = 0, use_stdin = 1, braces = 1, encrypt_flag = 0;
Xchar toggle_flag = 0, easy_flag = 0, init_flag = 0;
X
X
X#define ISPUNCT(ch)	((ch) < 'A' || (ch) > 'z' || ((ch) > 'Z' && (ch) < 'a'))
X#define ISUPPER(ch)	((ch) <= 'Z' && (ch) >= 'A')
X#define ISLOWER(ch)	((ch) >= 'a' && (ch) <= 'z')
X#define TOUPPER(ch)	(ISLOWER(ch)? (ch)-('a'-'A') : (ch))
X#define TOLOWER(ch)	(ISUPPER(ch)? (ch)+('a'-'A') : (ch))
X
Xlong random();
Xvoid wrap_it_up();
Xchar *malloc(), *rindex();
X
Xvoid main( argc, argv )
Xint  argc;
Xchar *argv[];
X{
X    int  i, j;
X    int  ch, ch2;
X
X    /*
X    || Remember who we are.
X    */
X    if( (myname = rindex( *argv, '/' )) == NULL ) {
X	myname = *argv;
X    } else {
X	++myname;
X    }
X    /*
X    || Check arg count.  If no args, tell them about us.
X    */
X    if( argc == 1 ) {
X	fprintf( stderr, "usage: %s [-cegnru] [file(s)]\n", myname );
X	fprintf( stderr, "help:  %s -h,  %s -i\n", myname, myname );
X	exit( 1 );
X    }
X
X    /*
X    || Try to grab enough memory to hold a few good cryptograms.
X    */
X    if( (buffer = malloc( BUFF_SIZE )) == NULL ) {
X	fprintf( stderr, "Sorry -- not enough memory.\n" );
X	exit( 2 );
X    }
X    buff_cp     = buffer;
X    buff_titles = buffer + BUFF_SIZE;
X
X    srandom( time(0) );			/* bump the randomizer off-track */
X
X    /*
X    || Parse the entire command tail.  Options must appear first.
X    */
X    while( --argc ) {
X	if( **++argv == '-' ) {
X	    while( *++*argv ) {
X		switch( TOLOWER(**argv) ) {
X		  case 'r':			/* rot-based encoding */
X		    if( (easy_flag = atoi( *argv+1 )) < 1 || easy_flag > 26 ) {
X			easy_flag = -1;		/* enable random rot */
X		    }
X		    while( *++*argv <= '9' && **argv >= '0' ) {
X			;
X		    }
X		    *--*argv;
X		  case 'e':
X		    encrypt_flag = 1;		/* enable encryption */
X		    break;
X		  case 'u':
X		    upper_flag = 1;		/* set upper-case flag */
X		    break;
X		  case 'c':			/* grab user-specified code */
X		    cl_cypher = (*argv)+1;
X		    ch	      = '?';
X		    i	      = INIT_CYPHER;
X		    goto c_and_g;
X		  case 'g':			/* grab initial guess */
X		    cl_guess = (*argv)+1;
X		    ch	     = '_';
X		    i	     = INIT_GUESS;
X		  c_and_g:			/* cypher & guess init */
X		    if( init_flag & i ) {
X			fprintf( stderr, "You specified too many -%c options.\n", **argv );
X			exit( 1 );
X		    }
X		    init_flag |= i;
X		    if( strlen( *argv+1 ) != 26 ) {
X			fprintf( stderr, "-%c option must be followed by exactly 26 characters.\n", **argv );
X			exit( 1 );
X		    }
X		    for( i = 0; i < 26; ++i ) {
X			++*argv;
X			if( ISPUNCT( **argv ) ) {
X			    **argv = ch;
X			} else {
X			    **argv = TOLOWER( **argv );
X			}
X		    }
X		    break;
X		  case 'n':
X		    braces = FALSE;
X		    break;
X		  case 'h':
X		    help1();		/* tell them about the command options */
X		    exit( 0 );
X		  case 'i':		/* tell them about our interactive mode */
X		    printf( "\nInteractive commands:\n" );
X		    help2();
X		    printf( " ?           Display this help message.\n" );
X		    exit( 0 );
X		  default:
X		    fprintf( stderr, "Uknown option:  \"%c\".\n", **argv );
X		    exit( 1 );
X		}/* switch */
X	    }/* while */
X	} else {			/* we found a non-option */
X	    use_stdin = FALSE;
X	    do {			/* process as many files as we find */
X		if( freopen( *argv, "r", stdin ) ) {
X		    process_file();
X		} else {
X		    fprintf( stderr, "Unable to open \"%s\" for input.\n", *argv );
X		}
X		++argv;
X	    } while( --argc );
X	    break;
X	}/* if */
X    }/* while */
X
X    if( use_stdin ) {			/* if no files were specified, */
X	process_file();			/* process stdin */
X    }
X    if( puz_count >= 0 ) {		/* finish last puzzle */
X	puzzle_eof();		/* (might decrement puz_count if empty) */
X    }
X    if( puz_count < 0 ) {
X	fprintf( stderr, "No puzzle text processed.\n" );
X	exit( 2 );
X    }
X    if( buff_cp+2 >= buff_titles ) {
X	fprintf( stderr, "Not enough room for entire puzzle file.\n" );
X    }
X
X    /*
X    || If we are going to operate interactivly, we need to get our ttyname
X    || to re-instate the stdin to our tty.  At the same time, we check our
X    || stdout, and set pipe mode if it is not a tty.
X    */
X    if( (mytty = ttyname( 1 )) == NULL ) {
X	select_puzzle( 0 );
X	if( !braces ) {				/* if no braces are output, */
X	    for( i = 0; i < 26; ++i ) {		/*output solution even if only */
X		if( guess[i] != '_' ) {		/*a partial guess is present. */
X		    puzz->solved = TRUE;
X		    break;
X		}
X	    }
X	}
X	save_cryptogram( stdout );
X	exit( 0 );
X    }
X
X    if( freopen( mytty, "r", stdin ) == NULL ) { /* get real tty input */
X        fprintf( stderr, "Unable to open \"%s\".\n", mytty );
X        exit( 2 );
X    }
X
X    initscr();					 /* setup screen handling */
X    savetty();
X    noecho();
X    raw();
X
X    signal( SIGINT, wrap_it_up );
X    if( signal( SIGQUIT, SIG_IGN ) != SIG_IGN ) {
X	signal( SIGQUIT, wrap_it_up );
X    }
X
X    /*
X    || Select the first (or only) puzzle to solve.
X    */
X    select_puzzle( 0 );			/* choose first puzzle */
X    if( puz_count ) {			/* if multiple puzzles, prompt user */
X	if( !puzzle_prompt() ) {
X	    wrap_it_up();
X	}
X    }
X    clear();				/* clear the screen */
X
X    for( ;; ) {
X	screen_update( draw_mode );	/* Update the screen */
X	draw_mode = DRAW_MSGS;		/* assume message-update next time */
X
X	ch = getch();
X	ch = TOLOWER(ch);
X	switch( ch ) {
X	  case 'Q'-'@':				/* quit the program */
X	  case '['-'@':
X	  case EOF:
X	    if( !ok_to_exit() ) {		/* is it ok to exit? */
X		ch = char_prompt( ROWS, 34, "Exit? (Y/N)? " );
X		if( ch != 'y' && ch != EOF ) {
X		    break;
X		}
X	    }
X	    wrap_it_up();			/* cleanup & exit */
X	    break;
X	  case 'U'-'@':				/* undo a guessed letter */
X	    ch = char_prompt( ROWS, 46, "Undo: " );
X	    if( ISLOWER(ch) ) {
X		guess[ch-'a'] = '_';
X		draw_mode = ch;
X		puzz->changed = TRUE;
X	    } else {
X		status_msg = "Invalid entry";
X	    }
X	    break;
X	  case 'X'-'@':				/* undo all guessed letters */
X	    for( i = 0; i < 26; ++i ) {
X		guess[i] = '_';
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    puzz->changed = TRUE;
X	    break;
X	  case 'C'-'@':				/* they want to cheat! */
X	    if( !cypher_flag ) {
X		status_msg = "Unable to cheat -- Sorry!";
X		break;
X	    }
X	    status_msg = "That's all the cheating I know!";
X	    ch = (random() % 26);
X	    for( i = 0; i < 26; ++i ) {
X		if( used[i] ) {
X		    for( j = 0; j < 26; ++j ) {
X			if( cypher[j] == i+'a' ) {
X			    break;
X			}
X		    }
X		    if( j < 26 && guess[i] != j+'a' ) {
X			if( guess[i] == '_' ) {
X			    sprintf( status_msg = string2,
X				"Cheating -- added %c --> %c.", i+'A', j+'A' );
X			} else {
X			    sprintf( status_msg = string2,
X				"Cheating -- changed %c --> %c (was %c).",
X				 i+'A', j+'A', guess[i]-'a'+'A' );
X			}
X			guess[i] = j+'a';
X			break;
X		    }
X		} else {
X		    if( guess[i] != '_' ) {
X			sprintf( status_msg = string2,
X			    "Cheating -- removed %c (was %c).",
X			     i+'A', guess[i]-'a'+'A' );
X			guess[i] = '_';
X			break;
X		    }
X		}
X		ch = (ch + 1) % 26;
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    puzz->changed = TRUE;
X	    break;
X	  case 'H'-'@':				/* Hint (dictionary search) */
X	    hint();
X	    break;
X	  case 'D'-'@':				/* define solution */
X	    if( cypher_flag > 0 ) {
X		status_msg = "I like my solution better.";
X		break;
X	    }
X	    for( i = 0; i < 26; ++i ) {
X		cypher[i] = '?';
X	    }
X	    for( i = 0; i < 26; ++i ) {
X		if( guess[i] != '_' ) {
X		    cypher[guess[i]-'a'] = i+'a';
X		}
X	    }
X	    cypher_flag = -1;
X	    status_msg = "Defined solution.";
X	    puzz->changed = TRUE;
X	    break;
X	  case 'S'-'@':				/* save current puzzle state */
X	    if( !save_file( ROWS, 34, FALSE ) ) {
X		break;
X	    }
X	    if( puzz->solved ) {
X		status_msg = "Saved solution.";
X	    } else {
X		status_msg = "Saved puzzle.";
X	    }
X	    break;
X	  case 'T'-'@':			/* enter new title for cryptogram */
X	    move( ROWS, 0 );
X	    clrtobot();
X	    if( string_prompt( ROWS, 0, "Title: " ) ) {
X		status_msg = "Aborted.";
X		break;
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    if( !*string ) {
X		if( puzz->title ) {
X		    --puzz->maxrow;
X		    puzz->title = NULL;
X		    puzz->changed = TRUE;
X		}
X		break;
X	    }
X	    if( (i = strlen( string )) > COLS-4 ) {
X		i = COLS-4;
X		string[i] = '\0';
X	    }
X	    if( (buff_titles -= i+1) < buff_cp ) {
X		status_msg = "Not enough room.";
X		buff_titles += i+1;
X		break;
X	    }
X	    if( !puzz->title ) {
X		++puzz->maxrow;
X	    }
X	    strcpy( buff_titles, string );
X	    puzz->title = buff_titles;
X	    puzz->changed = TRUE;
X	    break;
X	  case 'A'-'@':			/* toggle alphabet display mode */
X	    toggle_flag = !toggle_flag;
X	    break;
X	  case 'N'-'@':			/* next puzzle (if multi-puzzled) */
X	    if( ++puz_number > puz_count ) {
X		puz_number = 0;
X	    }
X	    select_puzzle( puz_number );
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case 'P'-'@':			/* previous puzzle (if multi-puzzled) */
X	    if( --puz_number < 0 ) {
X		puz_number = puz_count;
X	    }
X	    select_puzzle( puz_number );
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case 'M'-'@':				/* display Menu of puzzles */
X	    if( !puzzle_prompt() ) {	/* check if they hit Esc */
X		wrap_it_up();			/* cleanup & exit */
X	    }
X	    clear();
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case '?':			/* output command summary */
X	    clear();
X	    mvprintw( 0, 13, "%s -- a cryptogram solver,  By:  Wayne Davison\n",
X			     myname );
X	    refresh();
X	    help2();
X	    mvaddstr( ROWS+1, 32, "[Press any key]" );
X	    refresh();
X	    getch();				/* (fall through) */
X	  case 'L'-'@':
X	  case 'R'-'@':
X	    clear();
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case '7':				/* goto left edge */
X	    puzz->hoffset = 0;
X	  case '4':				/* go left */
X	    if( (puzz->hoffset -= 4) <= 0 ) {
X		puzz->hoffset = 0;
X		status_msg = "[At left edge]";
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case '1':				/* goto right edge */
X	    puzz->hoffset = 32760;
X	  case '6':				/* go right */
X	    if( (puzz->hoffset += 4) > puzz->maxcol-COLS ) {
X		puzz->hoffset = puzz->maxcol-COLS;
X		status_msg = "[At right edge]";
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case '9':				/* goto top */
X	    puzz->voffset = 0;
X	  case '8':				/* go up */
X	    if( (puzz->voffset -= 2) <= 0 ) {
X		puzz->voffset = 0;
X		status_msg = "[At top]";
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  case '3':				/* goto bottom */
X	    puzz->voffset = 32760;
X	  case '2':				/* go down */
X	    if( (puzz->voffset += 2) >= puzz->maxrow-(ROWS/spacing) ) {
X		puzz->voffset = puzz->maxrow-(ROWS/spacing);
X		status_msg = "[At bottom]";
X	    }
X	    draw_mode = DRAW_CRYPT_MSGS;
X	    break;
X	  default:
X	    if( ISLOWER(ch) ) {		/* check for <let><let> substitution */
X		addch( ch );
X		refresh();
X		ch2 = getch();
X		ch2 = TOLOWER(ch2);
X		if( ISLOWER(ch2) ) {
X		    guess[ch-'a'] = ch2;
X		    puzz->changed = TRUE;
X		} else if( ch2 != '\010' && ch2 != '\177' ) {
X		    guess[ch-'a'] = '_';
X		    puzz->changed = TRUE;
X		}
X		draw_mode = ch;
X	    } else {
X		status_msg = "Type '?' for help.";
X	    }
X	}
X    }/* forever */
X}
X
Xint get_cypher( num )
Xint num;
X{
X    int  i, j, k, n;
X
X    for( i = 0; i < 26; ++i ) {		/* create rotated cypher */
X	cypher[i] = ( (i+num) % 26 )+'a';
X    }
X    if( num && !easy_flag ) {		/* if !easy, scramble it up */
X	for( i = 0; i < 512; ++i ) {
X	    j = random() % 26;
X	    k = random() % 26;
X	    n = cypher[j];
X	    cypher[j] = cypher[k];
X	    cypher[k] = n;
X	}
X    }
X}
X
Xprocess_file()
X{
X    char *cp, *line_start;
X    int  i, new_puzzle;
X    int  ch;
X
X    /*
X    || Try to read in the file.  Encode (if applicable) as we read.
X    */
X    new_puzzle = TRUE;
X
X    while( buff_cp+2 < buff_titles && (ch = getchar()) != EOF ) {
X	if( new_puzzle ) {
X	    new_puzzle = FALSE;
X	    if( buff_titles-buff_cp < 26*3+4 ) {
X		fprintf( stderr, "Not enough room for all the puzzles.\n" );
X		break;
X	    }
X	    if( puz_count == PUZZLE_MAX-1 ) {
X		fprintf( stderr, "%d-puzzle maximum exceeded.\n", PUZZLE_MAX );
X		break;
X	    }
X	    if( ++puz_count ) {		/* if previous puzzle, finish it up */
X		puzzle_eof();
X	    }
X
X	    puzz	  = &puzzle[puz_count];
X	    puzz->ptr	  = (buff_cp += 26*3);
X	    puzz->title   = NULL;
X	    puzz->maxrow  = puzz->maxcol  = 0;
X	    puzz->voffset = puzz->hoffset = 0;
X
X	    line_start	= buff_cp;
X	    cypher	= buff_cp - 26;
X	    guess	= cypher  - 26;
X	    used	= guess   - 26;
X
X	    /*
X	    || Setup the cypher's substitution array.  It is either:
X	    */
X	    if( (init_flag & INIT_CYPHER) ) {	/* a user-supplied cypher, */
X		strcpy( cypher, cl_cypher );
X		cypher_flag = -1;
X		puzz->changed = TRUE;
X	    } else {
X		if( encrypt_flag ) {		/* a randomly chosen cypher, */
X		    if( easy_flag <= 0 ) {
X			get_cypher( (random() % 25)+1 );
X		    } else {
X			get_cypher( easy_flag );
X		    }
X		    cypher_flag = 1;
X		    puzz->changed = TRUE;
X		} else {
X		    get_cypher( 0 );		/* or none (1-to-1 mapping). */
X		    cypher_flag = 0;
X		    puzz->changed = use_stdin;
X		}
X	    }
X	    /*
X	    || Initialize the guess array.  It is either:
X	    */
X	    for( i = 0; i < 26; ++i ) {
X		if( init_flag & INIT_GUESS ) {
X		    guess[i] = cl_guess[i];	/* user-specified with -g, */
X		} else {
X		    guess[i] = '_';		/* or initialized to all '_'s. */
X		}
X		used[i] = 0;			/* Also, set used to all 0's. */
X	    }
X	}
X	if( ch == '\n' ) {
X	    if( line_start[0] == '{' && *(buff_cp-1) == '}'
X	     && line_start[1] == '-' ) {
X		if( (ch = line_start[2]) == 'c' || ch == 'g' ) {
X		    if( buff_cp-line_start != 30 ) {
X			fprintf( stderr, "File has invalid {-%c...} entry.\n", ch );
X			goto end_of_line;
X		    }
X		}
X		cp = &line_start[3];
X
X		switch( TOLOWER(ch) ) {
X		  case 'c':		/* found code specifier */
X		    for( i = 0; i < 26; ++i ) {
X			if( !ISPUNCT(cp[i]) ) {
X			    cp[i] = cypher[TOLOWER(cp[i])-'a'];
X			}
X		    }
X		    strncpy( cypher, cp, 26 );
X		    cypher_flag = 1;
X		    break;
X		  case 'g':		/* found guess specifier */
X		    if( init_flag & INIT_GUESS ) {
X			fprintf( stderr, "Ignored file's -g entry.\n" );
X			break;
X		    }
X		    for( i = 0; i < 26; ++i ) {
X			if( ISPUNCT(cp[i]) ) {
X			    guess[i] = '_';
X			} else {
X			    guess[i] = TOLOWER(cp[i]);
X			}
X		    }
X		    break;
X		  case 'e':		/* found end of puzzle separator */
X		    new_puzzle = TRUE;
X		    break;
X		  case 't':		/* found title specifier */
X		    if( (i = strlen( cp )-1) > COLS-4 ) {
X			i = COLS-4;
X		    }
X		    cp[i] = '\0';
X		    buff_titles -= i+1;
X		    if( line_start >= buff_titles ) {
X			break;
X		    }
X		    puzz->title = buff_titles;
X		    ++puzz->maxrow;
X		    strcpy( buff_titles, cp );
X		    break;
X		  default:
X		    fprintf( stderr, "Unknown file option:  {-%c...}.\n", ch );
X		    goto end_of_line;
X		}
X		buff_cp = line_start;
X		continue;
X	    }
X	end_of_line:
X	    while( buff_cp > line_start && *(buff_cp-1) == ' ' ) {
X		--buff_cp;		/* remove trailing spaces */
X	    }
X	    if( buff_cp == line_start
X	     && (buff_cp == puzz->ptr || *(buff_cp-2) == '\0') ) {
X		continue;		/* skip leading/multiple blank lines */
X	    }
X	    ++puzz->maxrow;		/* remember maximum row/col numbers */
X	    if( buff_cp-line_start > puzz->maxcol ) {
X		puzz->maxcol = buff_cp-line_start;
X	    }
X	    while( line_start < buff_cp ) {	/* encrypt & set used flag */
X		if( !ISPUNCT(*line_start) ) {
X		    i = TOLOWER(*line_start)-'a';
X		    if( cypher[i] != '?' ) {
X			used[cypher[i]-'a'] = 1;
X		    }
X		    if( upper_flag || ISUPPER(*line_start) ) {
X			*line_start = TOUPPER(cypher[i]);
X		    } else {
X			*line_start = cypher[i];
X		    }
X		}
X		++line_start;
X	    }
X	    *buff_cp++ = '\0';
X	    line_start = buff_cp;
X	    continue;
X	} else if( ch == '\010' && buff_cp != line_start ) {
X	    --buff_cp;				/* Backspace erases last char */
X	    continue;
X	} else if( ch == '\t' ) {
X	    ch = ' ';				/* convert tabs to spaces */
X	    while( (buff_cp-line_start+1)%8 ) {
X		*buff_cp++ = ' ';
X	    }
X	}
X	if( ch >= ' ' ) {
X	    *buff_cp++ = ch;			/* insert non-ctrl chars */
X	}
X    }
X}
X
Xpuzzle_eof()				/* finish a puzzle's definition */
X{
X    int  i;
X
X    if( buff_cp == puzz->ptr ) {	/* forget puzzle if it is empty */
X	buff_cp -= 26*3;
X	--puz_count;
X	return;
X    }
X    if( *(buff_cp-1) != '\0' ) {
X	*buff_cp++ = '\0';		/* terminate last line if needed */
X    } else if( *(buff_cp-2) == '\0' ) {
X	--buff_cp;			/* remove any trailing blank line */
X	--puzz->maxrow;
X    }
X    puzz->length = buff_cp - puzz->ptr;	/* remember length of puzzle */
X    *buff_cp++ = '\177';
X
X    puzz->hint_cp = puzz->ptr;		/* setup the hint pointers */
X    find_hint_word();
X
X    puzz->solved = FALSE;		/* set initial value of solved flag */
X    if( cypher_flag ) {
X	for( i = 0; i < 26; ++i ) {
X	    if( cypher[i] != '?'
X	     && (guess[cypher[i]-'a'] != '_' || used[cypher[i]-'a'])
X	     &&  guess[cypher[i]-'a'] != i+'a' ) {
X		break;
X	    }
X	}
X	if( i == 26 ) {
X	    puzz->solved = TRUE;
X	}
X    }
X}
X
Xint puzzle_prompt()		/* prompt user for puzzle selection */
X{
X    int  i, j, num;
X
X    j = (ROWS+2-puz_count)/2;	/* center the puzzle menu */
X    clear();
X    /*
X    || Loop through all the puzzles, looking for titles to display.
X    */
X    for( i = 0; i <= puz_count; ++i ) {
X	mvprintw( i+j, 0, "%2d. %s", i+1, puzzle[i].title? puzzle[i].title
X							 : "<Untitled>" );
X    }
X    mvaddstr( i+j+1, 0, "<Press Esc to exit, ^S to save all the puzzles>" );
X    /*
X    || Ask the user which puzzle to start with.
X    */
X    for( ;; ) {
X	move( j-1, 0 );
X	clrtoeol();
X	if( status_msg ) {
X	    mvaddstr( j-1, 34, status_msg );
X	    status_msg = NULL;
X	}
X	sprintf( string, "Which puzzle? (1 - %d)? ", puz_count+1 );
X	if( (num = string_prompt( j-2, 0, string )) == 1 ) {
X	    if( !ok_to_exit() ) {
X		num = char_prompt( j-2, 0, "Exit? (Y/N)? " );
X		if( num != 'y' && num != EOF ) {
X		    continue;
X		}
X	    }
X	    return( 0 );		/* if they want to exit, return 0 */
X	}
X	if( num == 2 ) {		/* handle saving of all puzzles */
X	    if( save_file( j-2, 0, TRUE ) ) {
X		status_msg = "Saved all puzzles.";
X	    }
X	    continue;
X	}
X	if( !*string ) {	/* null string means return to last puzzle */
X	    return( 1 );
X	}
X	sscanf( string, "%d", &num );
X	if( --num >= 0 && num <= puz_count ) {
X	    select_puzzle( num );	/* select new puzzle on valid number */
X	    return( 1 );
X	}
X    }
X}
X
Xselect_puzzle( num )		/* setup the pointers for the current puzzle */
Xint  num;
X{
X    int  i;
X
X    puz_number = num;
X
X    puzz   = &puzzle[puz_number];
X    cypher = puzz->ptr - 26;
X    guess  = cypher    - 26;
X    used   = guess     - 26;
X
X    cypher_flag = 0;
X    for( i = 0; i < 26; ++i ) {
X	if( cypher[i] == '?' ) {
X	    cypher_flag = -1;
X	    break;
X	}
X	if( cypher[i] != i+'a' ) {
X	    cypher_flag = 1;
X	}
X    }
X}
X
Xint save_file( row, col, multi_flag )	/* handle prompt & file saving */
Xint row, col, multi_flag;
X{
X    FILE *fp;
X    int  i, j;
X    char ch;
X
X    /*
X    || Prompt the user for save filename.
X    */
X    if( string_prompt( row, col, "Save file: " ) || !*string ) {
X	status_msg = "Save aborted.";
X	return( 0 );
X    }
X    if( (fp = fopen( string, "r" )) != NULL ) {	/* check for existing file */
X	fclose( fp );
X	ch = char_prompt( row+1, col, "Overwrite file? (Y/N): " );
X	if( ch != 'y' ) {
X	    status_msg = "Save aborted.";
X	    return( 0 );
X	}
X    }
X    if( (fp = fopen( string, "w" )) == NULL ) {	/* open a new file */
X	status_msg = "Unable to create file.";
X	return( 0 );
X    }
X    if( multi_flag ) {			/* if multi-puzzled, save them all */
X	j = puz_number;
X	for( i = 0; i <= puz_count; ++i ) {
X	    select_puzzle( i );
X	    save_cryptogram( fp );
X	}
X	select_puzzle( j );
X    } else {
X	save_cryptogram( fp );		/* else, save the current one */
X    }
X    fclose( fp );
X
X    return( 1 );
X}
X
Xsave_cryptogram( fp )
XFILE *fp;
X{
X    char *cp, ch;
X    int  i;
X
X    if( braces && puzz->title ) {		/* output title if present */
X	fprintf( fp, "{-t%s}\n", puzz->title );
X    }
X    if( !puzz->solved ) {	/* output cryptogram IF it's not all filled-in */
X	for( cp = puzz->ptr; *cp != '\177'; ++cp ) {
X	    if( *cp ) {
X		putc( *cp,  fp );
X	    } else {
X		putc( '\n', fp );
X	    }
X	}
X    }
X    if( braces ) {
X	for( i = 0; i < 26; ++i ) {	/* check if they've guessed anything */
X	    if( guess[i] != '_' ) {
X		break;
X	    }
X	}
X	if( i < 26 ) {
X	    fprintf( fp, "{-g%26.26s}\n", guess );/* output our current guess */
X	}
X	if( cypher_flag ) {			  /* output code if known */
X	    fprintf( fp, "{-c%26.26s}\n", cypher );
X	}
X    }
X    if( puzz->solved ) {		/* output solution if all filled-in */
X	for( cp = puzz->ptr; *cp != '\177'; ++cp ) {
X	    if( *cp ) {
X		if( !ISPUNCT(*cp) ) {
X		    ch = guess[TOLOWER(*cp)-'a'];
X		    if( ISUPPER(*cp) ) {
X			ch = TOUPPER(ch);
X		    }
X		    putc( ch,  fp );
X		} else {
X		    putc( *cp, fp );
X		}
X	    } else {
X		putc( '\n', fp );
X	    }
X	}
X    }
X    if( braces ) {
X	fprintf( fp, "{-end}\n" );		/* flag end of puzzle */
X    }
X    puzz->changed = FALSE;
X}
X
Xint ok_to_exit()			/* check if any puzzles are changed */
X{
X    int  i;
X
X    for( i = 0; i <= puz_count; ++i ) {
X	if( puzzle[i].changed ) {
X	    return( FALSE );
X	}
X    }
X    return( TRUE );
X}
X
Xint char_prompt( row, col, prompt )
Xint  row, col;
Xchar *prompt;
X{
X    int  ch;
X
X    mvaddstr( row, col, prompt );
X    clrtoeol();
X    refresh();
X    ch = getch();
X
X    return( TOLOWER(ch) );
X}
X
Xint string_prompt( row, col, prompt )
Xint  row, col;
Xchar *prompt;
X{
X    int  ch;
X    char *cp;
X
X    mvaddstr( row, col, prompt );
X    clrtoeol();
X    refresh();
X    cp = string;
X    while( (ch = getch()) != '\r' ) {
X	if( ch == '['-'@' || ch == 'C'-'@' || ch == EOF ) {
X	    *string = '\0';
X	    return( 1 );
X	}
X	if( ch == 'S'-'@' && col == 0 ) {
X	    return( 2 );
X	}
X	if( (ch == 'H'-'@' || ch == '\177') && cp != string ) {
X	    --cp;
X	    ch = '\010';
X	    addch( ch );
X	    addch( ' ' );
X	} else if( ch >= ' ' && cp-string < 80 ) {
X	    *cp++ = ch;
X	} else {
X	    continue;
X	}
X	addch( ch );
X	refresh();
X    }
X    *cp = '\0';
X
X    return( 0 );
X}
X
Xscreen_update( draw_flag )
Xint draw_flag;
X{
X    char *cp, *s, guessed[26];
X    int  i, j, row, col;
X
X    if( draw_flag <= DRAW_CRYPT ) {
X	erase();
X    }
X    if( draw_flag != DRAW_MSGS ) {
X	/*
X	|| First, skip any lines that are off the top of the screen.
X	*/
X	i = (puzz->title == NULL)? 0 : 1;
X	for( cp = puzz->ptr; i < puzz->voffset && *cp != '\177'; ++i ) {
X	    while( *cp++ ) {
X		;
X	    }
X	}
X	/*
X	|| Now, loop for as many lines as will fit on a screen.
X	*/
X	spacing = 2 + (puzz->maxrow*3 <= ROWS);
X
X	for( row = 0; row < ROWS && *cp != '\177'; row += spacing )
X	{
X	    /*
X	    || Output the title if present and visible.
X	    */
X	    if( row == 0 && puzz->voffset <= 0 && puzz->title ) {
X		mvprintw( row, 0, "%d. %s", puz_number+1, puzz->title );
X		continue;
X	    }
X	    /*
X	    || Skip any text off the left side of the screen.
X	    */
X	    for( i = 0; i < puzz->hoffset && *cp; ++i, ++cp ) {
X		;
X	    }
X	    /*
X	    || Process each line as specified.
X	    */
X	    switch( draw_flag ) {
X	      case DRAW_CRYPT:
X	      case DRAW_CRYPT_MSGS:
X		move( row+1, 0 );
X		for( col = 0; *cp && col < COLS; ++cp, ++col ) {
X		    if( ISPUNCT(*cp) ) {
X			addch( *cp );
X		    } else {
X			j = guess[TOLOWER(*cp)-'a'];
X			if( ISUPPER(*cp) ) {
X			    j = TOUPPER(j);
X			}
X			addch( j );
X		    }
X		}
X		j = *cp;
X		*cp = '\0';
X		standout();
X		mvaddstr( row, 0, cp-col );
X		standend();
X		*cp = j;
X		break;
X	      case DRAW_HINT_ON:
X	      case DRAW_HINT_OFF:
X		for( col = 0; *cp && col < COLS; ++cp, ++col ) {
X		    if( cp == puzz->hint_cp ) {
X			if( draw_flag == DRAW_HINT_ON ) {
X			    standout();
X			}
X			move( row+1, col );
X			while( cp != puzz->hint_end ) {
X			    if( (j = TOLOWER(*cp)) != '\'' ) {
X				j = guess[j-'a'];
X				if( ISUPPER(*cp) ) {
X				    j = TOUPPER(j);
X				}
X			    }
X			    addch( j );
X			    ++cp;
X			}
X			standend();
X			row = ROWS;
X			break;
X		    }
X		} /* for col */
X		break;
X	      default:
X		for( col = 0; *cp && col < COLS; ++cp, ++col ) {
X		    if( TOLOWER(*cp) == draw_flag ) {
X			move( row+1, col );
X			j = guess[TOLOWER(*cp)-'a'];
X			if( ISUPPER(*cp) ) {
X			    j = TOUPPER(j);
X			}
X			addch( j );
X		    }
X		}
X	    }
X	    /*
X	    || Skip any text off the right side of the screen.
X	    */
X	    while( *cp++ ) {
X		;
X	    }
X	} /* for screenful */
X    } /* if */
X
X    switch( draw_flag ) {
X      case DRAW_HINT_ON:
X	/*
X	|| Prompt for hint-word selection.
X	*/
X	move( ROWS, 0 );
X	clrtobot();
X	if( status_msg ) {
X	    mvaddstr( ROWS+1, 0, status_msg );
X	    status_msg = NULL;
X	}
X	for( cp = puzz->hint_cp, s = string; cp < puzz->hint_end; ++cp ) {
X	    if( (j = TOLOWER(*cp)) != '\'' ) {
X		j = guess[j-'a'];
X	    }
X	    *s++ = j;
X	}
X	*s = '\0';
X	standout();
X	mvprintw( ROWS,  0, "Look up \"%s\"?", string );
X	standend();
X	addch( ' ' );
X      case DRAW_HINT_OFF:
X      case DRAW_CRYPT:
X	break;
X      case DRAW_MSGS:
X      case DRAW_CRYPT_MSGS:
X      default:
X	/*
X	|| Draw all messages, starting with the alphabet lines.
X	*/
X	cp = string;
X	s  = string+35;
X	if( toggle_flag ) {
X	    *cp++ = '/';
X	    *s++  = '\\';
X	    for( i = 0; i < 26; ++i ) {
X		if( used[i] || guess[i] != '_' ) {
X		    *cp++ = i+'A';
X		    *s++  = TOUPPER(guess[i]);
X		} else {
X		    *cp++ = ' ';
X		    *s++  = ' ';
X		}
X	    }
X	    *cp++ = '\\';
X	    *s++  = '/';
X	} else {
X	    for( i = 0; i < 26; ++i ) {
X		guessed[i] = 0;
X		for( j = 0; j < 26; ++j ) {
X		    if( guess[j] == i+'a' ) {
X			*cp++ = j+'A';
X			*s++  = i+'A';
X			++guessed[i];
X		    }
X		}
X	    }
X	    *cp++ = ' ';
X	    *s++  = ' ';
X	    *cp++ = '[';
X	    *s++  = '[';
X	    for( i = 0; i < 26; ++i ) {
X		if( guess[i] == '_' && used[i] ) {
X		    *cp++ = i+'A';
X		}
X		if( !guessed[i] ) {
X		    *s++  = i+'A';
X		}
X	    }
X	    if( *(cp-1) != '[' ) {
X		j = ']';
X	    } else {
X		j = ')';
X		*(cp-1) = '(';
X	    }
X	    if( *(s-1) != '[' ) {
X		*s++ = ']';
X	    } else {
X		s -= 2;
X	    }
X	    for( i = 0; i < 26; ++i ) {
X		if( guess[i] == '_' && !used[i] ) {
X		    *cp++ = i+'a';
X		}
X	    }
X	    if( *(cp-1) != '(' && *(cp-1) != '[' ) {
X		*cp++ = j;
X	    } else {
X		cp -= 2;
X	    }
X	}
X	*cp = '\0';
X	*s  = '\0';
X	standout();
X	mvaddstr( ROWS,   0, string+(*string==' ') );
X	standend();
X	clrtoeol();
X	mvaddstr( ROWS+1, 0, string+35+(*(string+35)==' ') );
X	clrtoeol();
X	/*
X	|| Next, draw the status message if specified.
X	*/
X	if( status_msg ) {
X	    mvaddstr( ROWS+1, 39, status_msg );
X	    status_msg = NULL;
X	}
X	/*
X	|| Then, let them know if we like their solution.
X	*/
X	standout();
X	if( cypher_flag ) {
X	    for( i = 0; i < 26; ++i ) {
X		if( cypher[i] != '?'
X		 && (guess[cypher[i]-'a'] != '_' || used[cypher[i]-'a'])
X		 &&  guess[cypher[i]-'a'] != i+'a' ) {
X		    break;
X		}
X	    }
X	    if( i == 26 ) {
X		puzz->solved = TRUE;
X		if( cypher_flag > 0 ) {
X		    mvaddstr( ROWS, COLS-15, "You solved it!" );
X		} else {
X		    mvaddstr( ROWS, COLS-15, "Looks good!" );
X		}
X	    } else {
X		puzz->solved = FALSE;
X	    }
X	}
X	/*
X	|| Finally, prompt for a command.
X	*/
X	mvaddstr( ROWS, 34, "Command:" );
X	standend();
X	addch( ' ' );
X    }
X    refresh();
X}
X
Xvoid wrap_it_up()
X{
X    clear();
X    refresh();
X    resetty();
X    endwin();
X
X    exit( 0 );
X}
X
Xhint()
X{
X    int  ch;
X
X    for( ;; ) {
X	screen_update( DRAW_HINT_ON );
X	ch = getch();
X	if( (ch = TOLOWER(ch)) == 'y' || ch == 'M'-'@' ) {
X	    addch( 'Y' );
X	    refresh();
X	    match();
X	    return;
X	}
X	screen_update( DRAW_HINT_OFF );
X	switch( ch ) {
X	  case '6':			/* goto next word */
X	  case ' ':
X	  case 'n':
X	    puzz->hint_cp = puzz->hint_end;
X	    break;
X	  case '8':			/* goto first word on previous line */
X	    while( puzz->hint_cp != puzz->ptr && *--puzz->hint_cp ) {
X		;
X	    }
X	    if( puzz->hint_cp == puzz->ptr ) {		/* wrap to eof */
X		puzz->hint_cp = puzz->ptr + puzz->length-1;
X	    }
X	    if( *(puzz->hint_cp-1) == '\0' ) {		/* skip double '\n' */
X		--puzz->hint_cp;
X	    }
X	  case '7':			/* goto first word of line */
X	    while( puzz->hint_cp != puzz->ptr && *--puzz->hint_cp ) {
X		;
X	    }
X	    break;
X	  case '3':			/* goto last word of puzzle */
X	    puzz->hint_cp = puzz->ptr + puzz->length;
X	  case '1':			/* goto last word of line */
X	  case '2':			/* goto first word of next line */
X	    while( *puzz->hint_cp != '\177' && *puzz->hint_cp++ ) {
X		;
X	    }
X	    if( ch == '2' ) {
X		break;
X	    }
X	  case '4':			/* goto previous word */
X	    do {
X		if( puzz->hint_cp == puzz->ptr ) {
X		    puzz->hint_cp += puzz->length;
X		}
X		--puzz->hint_cp;
X	    } while( ISPUNCT(*puzz->hint_cp) );
X
X	    while( puzz->hint_cp != puzz->ptr
X	     && (*puzz->hint_cp == '\'' || !ISPUNCT(*puzz->hint_cp)) )
X	    {
X		--puzz->hint_cp;
X	    }
X	    if( puzz->hint_cp != puzz->ptr ) {
X		++puzz->hint_cp;
X	    }
X	    break;
X	  case '9':			/* goto first word of puzzle */
X	    puzz->hint_cp = puzz->ptr;
X	    break;
X	  case 'q':			/* quit hint mode */
X	  case 'Q'-'@':
X	  case '['-'@':
X	    return;
X	  case '?':			/* output help */
X	    help3( 1 );
X	    break;
X	  default:			/* tell them about the help */
X	    status_msg = "Type '?' for help.";
X	    break;
X	}
X	find_hint_word();
X    } /* forever */
X}
X
Xfind_hint_word()
X{
X  find_it:
X    while( *puzz->hint_cp != '\177' && ISPUNCT(*puzz->hint_cp) ) {
X	++puzz->hint_cp;
X    }
X    if( *puzz->hint_cp == '\177' ) {		/* wrap if at eof */
X	puzz->hint_cp = puzz->ptr;
X	goto find_it;
X    }
X    puzz->hint_end = puzz->hint_cp;
X    while( *puzz->hint_end == '\'' || !ISPUNCT(*puzz->hint_end) ) {
X	++puzz->hint_end;
X    }
X}
X
Xmatch()
X{
X    FILE *fchan;
X    extern char *fgets();
X    char token[80], str[80], hguess[26];
X    register char *s, *t;
X    register int j, slen, tlen;
X
X    fchan = fopen( "/usr/dict/words", "r" );
X    if( !fchan ) {
X	status_msg = "Couldn't find dictionary!";
X	screen_update( DRAW_HINT_OFF );
X	return;
X    }
X    move( ROWS+1, 0 );
X    clrtoeol();
X    refresh();
X    slen = strlen(string);
X    while( fgets( token, (sizeof token), fchan ) ) {
X	if( *string != '_' ) {
X	    if( TOLOWER(*token) < *string ) {
X		continue;
X	    }
X	    if( TOLOWER(*token) > *string ) {
X		break;
X	    }
X	}
X	tlen = strlen(token);
X	token[--tlen] = '\0';
X	if( tlen != slen ) {
X	    continue;
X	}
X	for( t = token; *t; ++t ) {
X	    *t = TOLOWER(*t);
X	    if( *t != '\'' && !ISLOWER(*t) ) {
X		goto nomatch;
X	    }
X	}
X	strcpy( str, string );
X
X	for( j = 0; j < 26; ++j ) {
X	    hguess[j] = 0;
X	}
X	for( j = 0; j < 26; ++j ) {
X	    if( guess[j] != '_' ) {
X		hguess[guess[j]-'a'] = j + 'a';
X	    }
X	}
X
X	for( s = str, t = token; *s; ++s, ++t) {
X	    if( *s == *t ) {
X		continue;
X	    } else if( *s != '_' || *t == '\'' || hguess[*t-'a'] ) {
X		goto nomatch;
X	    } else {
X		j = s - str;
X		hguess[*t-'a'] = TOLOWER(puzz->hint_cp[j]);
X		for( ; j < slen; ++j ) {
X		    if( TOLOWER(puzz->hint_cp[j]) == hguess[*t-'a'] ) {
X			str[j] = *t;
X		    }
X		}
X	    }
X	}
Xtryagain:
X	mvprintw( ROWS+1, 8, "\"%s\"? ", token );
X	refresh();
X	j = getch();
X	j = TOLOWER(j);
X	move( ROWS+1, 0 );
X	clrtoeol();
X	refresh();
X	switch( j ) {
X	  case 'q':
X	  case 'Q'-'@':
X	  case '['-'@':
X	    screen_update( DRAW_HINT_OFF );
X	    goto noloop;
X	  case 'y':
X	  case 'M'-'@':
X	    for( s = str, t = puzz->hint_cp; *s; ++s, ++t ) {
X		if( *t != '\'' ) {
X		    guess[TOLOWER(*t)-'a'] = *s;
X		}
X	    }
X	    puzz->changed = TRUE;
X	    screen_update( DRAW_CRYPT );
X	    goto noloop;
X	  case 'n':
X	  case ' ':
X	    break;
X	  case '?':
X	    help3( 0 );
X	    screen_update( DRAW_HINT_ON );
X	    addch( 'Y' );
X	    refresh();
X	    goto tryagain;
X	  default:
X	    mvaddstr( ROWS+1, 44, "Type '?' for help." );
X	    goto tryagain;
X	}
Xnomatch: ;
X    }
X    status_msg = "No matches found.";
X    screen_update( DRAW_HINT_OFF );
X
Xnoloop:
X    fclose(fchan);
X}
X
Xhelp1()
X{
X    printf( "\nThis program allows you to interactively solve cryptogram puzzles, or to simply\n\r" );
X    printf( "filter the input to the standard output IF the output has been redirected.  The\n\r" );
X    printf( "input can be pre-encoded or I can encode it for you.  If I encode it, I will be\n\r" );
X    printf( "able to tell you when you have correctly solved the puzzle.\n\r\n" );
X    printf( "Command-line options:\n\r" );
X    printf( "\t-u\tforce all input to Upper-case.\n\r" );
X    printf( "\t-e\tEncode the input stream with a random cypher.\n\r" );
X    printf( "\t-r\tencode using a simpler, rot-based random cypher.  For a\n\r" );
X    printf( "\t\tspecific rotation, include a number from 1 to 26, e.g.:  -r13.\n\r" );
X    printf( "\t-cSTR\tencode using the Code STR as the encryption cypher.\n\r" );
X    printf( "\t-gSTR\tinitialize the Guessed letters to STR.\n\r" );
X    printf( "\t-n\tinclude No brace-commands when saving or piping.\n\r" );
X    printf( "\tFILE\tread input from one or more FILEs instead of stdin.\n\r" );
X    printf( "\t-i\toutput the Interactive commands.\n\r" );
X    printf( "\t-h\toutput this Help list.\n\r" );
X    printf( "\t-\tthe null option can be used to avoid an empty command tail.\n\r" );
X    printf( "\nExamples:\n\r" );
X    printf( "\tfortune -l | %s -r\n\r", myname );
X    printf( "\t%s -eu puzzles\n\r", myname );
X    printf( "\t%s -n -gzyxwvutsrqponmlkjihgfedcba code.file >result.file\n\r", myname );
X}
X
Xhelp2()
X{
X    printf( "\n\r<let1><let2> guess the substitution cypher a letter at a time.\n\r" );
X    printf( "<let><space> undefine <let>'s current guess.  <space> can be any non-letter.\n\r" );
X    printf( "^U<let>      the alternate way to Undefine a letter's current guess.\n\r" );
X    printf( "^X           Undefine ALL the guessed letters.\n\r" );
X    printf( "^H           Hint -- select a word to perform a dictionary search on.\n\r" );
X    printf( "^C           Cheat -- if I know the solution, I'll correct one letter.\n\r" );
X    printf( "^A           toggle the Alpha display between the two different output modes.\n\r" );
X    printf( "^T           enter a (new) Title for this puzzle.\n\r" );
X    printf( "^D           Define the current guess to be the solution. Useful before a save.\n\r" );
X    printf( "^S<file>     Save the current puzzle, guess, & cypher information in a file.\n\r" );
X    printf( "             Use the command:  %s <file>  to restore where you were.\n\r", myname );
X    printf( "^N           go to the Next puzzle when multiple puzzles are present.\n\r" );
X    printf( "^P           go to the Previous puzzle when multiple puzzles are present.\n\r" );
X    printf( "^M           choose a puzzle from a Menu when multiple puzzles are present.\n\r" );
X    printf( "^Q, <Esc>    Quit -- leave the program.\n\r" );
X    printf( "^L, ^R       Redraw the screen.\n\r" );
X    printf( " 6,  4       Move a wide cryptogram left/right by 4 chars.\n\r" );
X    printf( " 2,  8       Move a long cryptogram up/down by 2 lines.\n\r" );
X    printf( " 7,  1       Move to the far left/right of a wide puzzle.\n\r" );
X    printf( " 9,  3       Move to the top/bottom of a long puzzle.\n\r" );
X}
X
Xhelp3( flag )
Xint flag;
X{
X    clear();
X    if( flag ) {
X	mvaddstr( 1, 0, "Word selection commands:" );
X    } else {
X	mvaddstr( 1, 0, "Dictionary look-up commands:" );
X    }
X    refresh();
X    printf( "\n\r\n\rY, <Enter>   Yes:  accept the choice.\n\r" );
X    printf( "N, <Space>   No:   reject the choice.\n\r" );
X    printf( "Q, <Esc>     Quit: exit hint mode.\n\r" );
X    if( flag ) {
X	printf( "8, 2         Move up/down, line by line.\n\r" );
X	printf( "4, 6         Move left/right, word by word.\n\r" );
X	printf( "7, 1         Move to first/last word on the current line.\n\r" );
X	printf( "9, 3         Move to first/last line in cryptogram\n\r" );
X    }
X    printf( "?            Print this message.\n\r" );
X    mvaddstr( ROWS+1, 0, "[Press any key] " );
X    refresh();
X    getch();
X    clear();
X    screen_update( DRAW_CRYPT );
X}
END_OF_FILE
if test 36106 -ne `wc -c <'crypto.c'`; then
    echo shar: \"'crypto.c'\" unpacked with wrong size!
fi
# end of 'crypto.c'
fi
if test -f 'crypto.man' -a "${1}" != "-c" ; then 
  echo shar: Will not clobber existing file \"'crypto.man'\"
else
echo shar: Extracting \"'crypto.man'\" \(5709 characters\)
sed "s/^X//" >'crypto.man' <<'END_OF_FILE'
X.TH CRYPTO 1 LOCAL
X.SH NAME
Xcrypto \- a program to generate and/or solve cryptograms
X.SH SYNOPSIS
X.B crypto
X[-cegrsu] [file(s)]
X.SH DESCRIPTION
X.I Crypto
Xis a fun program for playing around with cryptograms. It can be used
Xto encode text with a random or user-picked cypher, to interactively solve
Xcryptogram puzzles, to non-interactively apply a solution to a text file,
Xand to save the results to the file of your choice.
X.PP
XIn normal usage, text is taken from the specified file (or
Xthe standard input), encoded (if needed) and displayed for 
Xyou to solve. You can save the current puzzle at any time,
Xand take up where you left off later on.  If the cypher is
Xpicked by (or supplied to)
X.I crypto\,
Xit will inform you when you have correctly solved the puzzle.  You will
Xalso be able to cheat and see the correct solution one letter at a time.
XA simple, yet entertaining example is the command:
X.sp
Xfortune -l | crypto -r
X.sp
Xwhich encrypts a long fortune with a simple roational cypher and lets you
Xsolve it.  For a more difficult time, try:
X.sp
Xfortune -s | crypto -e
X.PP
X.I Crypto
Xcan also be used in a non-interactive manner by redirecting the output to
Xa file.  In this case, the text is read in, encoded if -e, -r, or -c was
Xspecified, and the resulting
X.I puzzle
Xis output.  If, however, a guess was
Xspecified with the -g option, the guess is applied to the puzzle and the
Xresulting
X.I solution
Xis output.  When redirecting, you will probably want to use the -n option
Xto avoid outputting the brace-commands (see below).  For example:
X.sp
Xcrypto -n -gzyxwvutsrqponmlkjihgfedcba puzz >solved
X.PP
X.I Crypto
Xuses a simple, text-based output format to save the puzzle, guess, cypher,
Xand title in a file.  These take the form of "brace-commands", each on a line
Xof their own, interspersed into the puzzle text.  {-gabc...xyz} specifies
Xthe current guess, {-cabc...xyz} specifes the cypher, and {-tTitle} specifies
Xthe title.  Any or all of these may be ommitted to take on their default
Xvalues.  In addition, the command {-end} allows you to separate the file
Xinto multiple puzzles, just as if you'd specified multiple file names on
Xthe command-line.  Each puzzle has a unique guess, cypher, and title.
X.PP
XWhen multiple puzzles are loaded, you will be presented with a menu
Xof choices for your initial puzzle.  From there, the save command (^S)
Xsaves all the puzzles into a single file.  This menu can be visited at
Xany time with the menu command (^M).
X.PP
XHere is a complete list of the interactive commands available.  You can
Xget a summary of this list with the command
X.I crypto -i
Xor by typing '?' while running
X.I crypto\.
X.PP
X.TP 13
X.B <let1><let2>
Xguess the substitution cypher a letter at a time.  This is your primary
Xtool in solving the cryptogram.  Example:  xb
X.TP 13
X.B <let><space>
Xundefine <let>'s current guess.  <space> can be any non-letter.  This
Xlets you selectively remove your letters and try again.
X.TP 13
X.B ^U<let>
Xthe alternate way to Undefine a letter's current guess.
X.TP 13
X.B ^X
XUndefine ALL the guessed letters.  Good for starting afresh.
X.TP 13
X.B ^C
XCheat -- if I know the solution, I will correct one letter in the
Xpuzzle each time you decide to cheat.
X.TP 13
X.B ^S<file>
XSave the current puzzle, guess, and cypher information in a file.
XThis allows you to continue where ever you left off with
Xthe command crypto <file>
X.TP 13
X.B ^D
XDefine the current guess to be the solution.  Useful before a save
Xto define the cypher to save along with the puzzle (if not already
Xknown).
X.TP 13
X.B ^H
XHint -- select a word to perform a dictionary search on.  This is
Xuseful for solving unknown cyphers.  All the currently guessed
Xletters affect the matching of dictionary words into the cypher word.
XWhile in hint-mode, type '?' for a brief command summary.
X.TP 13
X.B ^N
Xgo to the Next puzzle in a multi-puzzle file.
X.TP 13
X.B ^P
Xgo to the Previous puzzle in a multi-puzzle file.
X.TP 13
X.B ^M
Xpresents a Menu of the current puzzles available for selection.
X.TP 13
X.B ^A
Xtoggle the Alpha display between the two different output modes.
X.TP 13
X.B ^T
Xenter a (new) Title for this puzzle.
X.TP 13
X.B ^Q, <Esc>
XQuit -- leave the program.  Confirms the exit request if we
Xbelieve you might like to save the puzzle(s) first.
X.TP 13
X.B ^L, ^R
XRedraw the screen.
X.TP 13
X.B  6,  4
XMove a wide cryptogram left/right by 4 chars.
X.TP 13
X.B  2,  8
XMove a long cryptogram up/down by 2 lines.
X.TP 13
X.B  7,  1
XMove to the far left/right of a wide puzzle.
X.TP 13
X.B  9,  3
XMove to the top/bottom of a long puzzle.
X.PP
XThe following are valid command-line options:
X.TP 8
X.B \-u
XForce all input to Upper-case.
X.TP 8
X.B \-e
XEncode the input stream with a random cypher.
X.TP 8
X.B \-r
Xencode using a simpler, rot-based random cypher.  You can specify a specific
Xrotation by including a number from 1 to 26 after the r, as in:  -r13.
X.TP 8
X.B \-cSTR
Xencode using the Cypher STR for encryption.  STR is the 26 letters of the
Xalphabet in whatever order you choose.
X.TP 8
X.B \-gSTR
Xinitialize the Guessed letters to STR.
X.TP 8
X.B \-n
Xindicates that No brace-commands are to be included in the save or pipe
Xoutput.  Brace-commands are normally output to indicate the current
Xcypher, guess, and/or title.
X.TP 8
X.B FILE
Xread input from one or more FILEs, instead of stdin.
X.TP 8
X.B \-i
Xoutput a summary of the Interactive commands.
X.TP 8
X.B \-h
Xoutput a summary of the command-line options.
X.TP 8
X.B \-
Xthe null option can be used to avoid an empty command tail, which
Xwould output a short command-line summary.
X.SH FILES
X/usr/dict/words
X.SH AUTHOR
X.I Crypto
Xwas written by Wayne Davison.  Special thanx go to Bruce Holloway, whose
X"figure" program provided inspiration into curses & the dictionary lookup.
END_OF_FILE
if test 5709 -ne `wc -c <'crypto.man'`; then
    echo shar: \"'crypto.man'\" unpacked with wrong size!
fi
# end of 'crypto.man'
fi
if test -f 'puzzles' -a "${1}" != "-c" ; then 
  echo shar: Will not clobber existing file \"'puzzles'\"
else
echo shar: Extracting \"'puzzles'\" \(2802 characters\)
sed "s/^X//" >'puzzles' <<'END_OF_FILE'
X{-tThe Ring of Truth}
X        "B xcmmis evcw srv obveh nvssvep," pcbw Oeiwi bm c dlctvebmj
Xtibxv.
X        "Mi," Pcbw Jcmwcno, "yls B xcm.  Srv nvssvep cev Vntbpr, io
Xxilepv, io cm cmxbvms kiwv, yls srv ncmjlcjv bp srcs io Kiewie, zrbxr
XB zbnn mis lssve rvev.  Srvh cev nbmvp io c tvepv nimj umizm bm
XVntvm-niev:
X
X        "Srbp Ebmj, mi isrve, bp kcwv yh srv vntvp,
X        Zri'w gczm srvbe izm kisrve si jecy bs srvkpvntvp.
X        Elnve io xevvgve, kiescn, cmw pxcnnig,
X        Srbp bp c pnvvgve srcs gcxup dlbsv c zcnnig.
X        Srv Gizve cnkbjrsh evpsp bm srbp Nimv Ebmj.
X        Srv Gizve, cnebjrsh, oie wibmj hile Izm Srbmj.
X        Bo yeiuvm ie ylpsvw, bs xcmmis yv evkcwv.
X        Bo oilmw, pvmw si Piervw (zbsr gipscjv gevgcbw)."
X{-ccyxwvojrbaunkmigdepsltzfhq}
X{-end}
X{-tSign of the Times}
X        PGMVODM (Mgb 23 - Wmx 21)
XHmu qvy pcvyrf dw lupdwypp qwf gqwwmb ly bvupbyf.  Hmu rdjj qgcdyxy bcy
Xodwwqgjy mz puggypp lygqupy mz hmuv bmbqj jqgt mz ybcdgp.  Kmpb Pgmvodm
Xoymojy qvy kuvfyvyf.
X{-cqlgfyzecditjkwmosvpbuxrnha}
X{-end}
X{-tOver Exposure}
XQgu Qgpib Ons vx Wgvqvlinwgt:
X        Px tva bpb ynrnlu qv luq nrt lvvb hgvqh, qgut spoo cu iaprub
X        sgur hvyuvru prnbkuiqurqot vwurh qgu bnijivvy bvvi nrb noo vx
X        qgu bnij ounjh vaq.
X{-cncdbuxlgpmjoyrvwfihqaksetz}
X{-end}
X{-tIlluminating Questions}
XDmq zenf deucqeua antonaaup cmap ob beha bm sdenta e votdbiyvi?
XNmna: "Qa'vv goj ob on pmgbqeua."
X
XDmq zenf pmgbqeua antonaaup cmap ob beha bm sdenta e votdbiyvi?
XNmna: "Qa'vv cmsyzanb ob on bda zenyev."
X
XDmq zenf basd quobaup cmap ob beha bm sdenta e votdbiyvi?
XNmna: "Bda ypau sen qmuh ob myb."
X{-ceiscagtdorhvznmwlupbyxqjfk}
X{-end}
X{-tSearch for Knowledge}
XUzmqm bc v uzmoqh szbxz cuvumc uzvu bl mpmq vrhorm wbcxopmqc mdvxukh
Xszvu uzm Nrbpmqcm bc loq vrw szh bu bc zmqm, bu sbkk brcuvrukh
Xwbcvaamvq vrw gm qmakvxmw gh cojmuzbrf mpmr joqm gbyvqqm vrw
Xbrmdakbxvgkm.  Uzmqm bc vrouzmq uzmoqh szbxz cuvumc uzvu uzbc zvc
Xvkqmvwh zvaamrmw.
X                -- Uzm Zbuxzzbtmq'c Fnbwm uo uzm Fvkvdh
X{-cvgxwmlfzbetkjroaiqcunpsdhy}
X{-end}
X{-tFowl Whether}
XKgfyzytl-Kfddcytl, wx ytzjr sla,
XDsl tzxd luud ca qsl Gltzqcbl Vsla.
XDsl eflda'q tzx luud ca qsl Kfdcqcbl Afv
XYljzidl dsl'd iazytl qf kfdqitzql sfv.
X                -- Hglelgcjr Vcadfg
X{-czyjelhuscprtwafkogdqibvnxm}
X{-end}
X{-tAxiom of the Week}
XTohbh rw dz trch grfh toh ybhwhdt nzb yzwtyzdrdk xoet szj zjkot tz uh
Xqzrdk.
X{-ceumqhnkorpfgcdzyabwtjlxvsi}
X{-end}
X{-tPessimistic Pragma}
XCmnfay't dmtuwnzui:
X        Fh lmw'ei hiinfay ymmo, oma'u qmeel.  Lmw'nn yiu mxie fu.
X{-czcroihygfvpnkamdsetuwxqblj}
X{-end}
X{-tDefinition}
XMxvsyshz: Rsvygozj gy yuz yxm xl xjz'v hxsqz.
X{-cgfqkzlpuscobrjxmewvythdani}
X{-end}
X{-tDefinition II}
XRbtjyjvhpu: R ejbi-ncuupuw muchlpu tcrt ibqf bg chppf rqw wjnq guwuftihrqf.
X{-crvlwueachzxpyqjgkiftbmndso}
X{-end}
END_OF_FILE
if test 2802 -ne `wc -c <'puzzles'`; then
    echo shar: \"'puzzles'\" unpacked with wrong size!
fi
# end of 'puzzles'
fi
echo shar: End of shell archive.
exit 0
