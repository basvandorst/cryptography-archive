#undef TEMP_VERSION	/* if defined, temporary experimental version of PGP */
/*	pgp.c -- main module for PGP.
	PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

	Synopsis:  PGP uses public-key encryption to protect E-mail. 
	Communicate securely with people you've never met, with no secure
	channels needed for prior exchange of keys.  PGP is well featured and
	fast, with sophisticated key management, digital signatures, data
	compression, and good ergonomic design.

	The original PGP version 1.0 was written by Philip Zimmermann, of
	Phil's Pretty Good(tm) Software.  Many parts of later versions of 
	PGP were developed by an international collaborative effort, 
	involving a number of contributors, including major efforts by:
		Branko Lankester <lankeste@fwi.uva.nl> 
		Hal Finney <74076.1041@compuserve.com>
		Peter Gutmann <pgut1@cs.aukuni.ac.nz>
	Other contributors who ported or translated or otherwise helped include:
		Jean-loup Gailly in France
		Hugh Kennedy in Germany
		Lutz Frank in Germany
		Cor Bosman in The Netherlands
		Felipe Rodriquez Svensson in The Netherlands
		Armando Ramos in Spain
		Miguel Angel Gallardo Ortiz in Spain
		Harry Bush and Maris Gabalins in Latvia
		Zygimantas Cepaitis in Lithuania
		Alexander Smishlajev
		Peter Suchkow and Andrew Chernov in Russia
		David Vincenzetti in Italy
		...and others.

	Note that while many PGP source modules bear the copyright notice of
	Philip Zimmermann, some of them may have been revised or in some
	cases entirely written by other members of the PGP development team,
	who often failed to put their names in their code.

	Revisions:
		Version 1.0 - 5 Jun 91
		Version 1.4 - 19 Jan 92
		Version 1.5 - 12 Feb 92
		Version 1.6 - 24 Feb 92
		Version 1.7 - 29 Mar 92
		Version 1.8 - 23 May 92
		Version 2.0 - 2 Sep 92
		Version 2.1 - 6 Dec 92

	(c) Copyright 1990-1992 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use
	of this software, even if the damage results from defects in this
	software.  No warranty is expressed or implied.

	All the source code Philip Zimmermann wrote for PGP is available for
	free under the "Copyleft" General Public License from the Free
	Software Foundation.  A copy of that license agreement is included in
	the source release package of PGP.  Code developed by others for PGP
	is also freely available.  Other code that has been incorporated into
	PGP from other sources was either originally published in the public
	domain or was used with permission from the various authors.  See the
	PGP User's Guide for more complete information about licensing,
	patent restrictions on certain algorithms, trademarks, copyrights,
	and export controls.  

	Philip Zimmermann may be reached at:
		Boulder Software Engineering
		3021 Eleventh Street
		Boulder, Colorado 80304  USA
		(303) 541-0140  (voice or FAX)
		email:  prz@sage.cgd.ucar.edu


	PGP will run on MSDOS, Sun Unix, VAX/VMS, Ultrix, Atari ST, 
	Commodore Amiga, and OS/2.  Note:  Don't try to do anything with 
	this source code without looking at the PGP User's Guide.

	PGP combines the convenience of the Rivest-Shamir-Adleman (RSA)
	public key cryptosystem with the speed of fast conventional
	cryptographic algorithms, fast message digest algorithms, data
	compression, and sophisticated key management.  And PGP performs 
	the RSA functions faster than most other software implementations.  
	PGP is RSA public key cryptography for the masses.

	Uses RSA Data Security, Inc. MD5 Message Digest Algorithm
	as a hash for signatures.  Uses the ZIP algorithm for compression.
	Uses the ETH IPES/IDEA algorithm for conventional encryption.

	PGP generally zeroes its used stack and memory areas before exiting.
	This avoids leaving sensitive information in RAM where other users
	could find it later.  The RSA library and keygen routines also
	sanitize their own stack areas.  This stack sanitizing has not been
	checked out under all the error exit conditions, when routines exit
	abnormally.  Also, we must find a way to clear the C I/O library
	file buffers, the disk buffers, and and cache buffers.

	If you modify this code, PLEASE preserve the style of indentation
	used for {begin...end} blocks.  It drives me bats to have to deal
	with more than one style in the same program.

 	Modified: 12-Nov-92 HAJK
 	Add FDL stuff for VAX/VMS local mode. 
*/


#include <ctype.h>
#ifndef AMIGA
#include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "language.h"
#include "pgp.h"
int maintenance(char *ringfile, int options);	/* from keymaint.c */
int processConfigLine( char *option );	/* From config.c */
#ifdef  M_XENIX
char *strstr();
long time();
#endif

#ifdef MSDOS
#ifdef __ZTC__	/* Extend stack for Zortech C */
unsigned _stack_ = 12*1024;
#endif
#ifdef __TURBOC__
unsigned _stklen = 12*1024;
#endif
#endif
#define	STACK_WIPE	4096

/* Global filenames and system-wide file extensions... */
char rel_version[] = "2.1";	/* release version */
char rel_date[] = "6 Dec 92";	/* release date */
char CTX_EXTENSION[] = ".pgp";
char PGP_EXTENSION[] = ".pgp";
char ASC_EXTENSION[] = ".asc";
char SIG_EXTENSION[] = ".sig";
char BAK_EXTENSION[] = ".bak";
char HLP_EXTENSION[] = ".hlp";
char CONSOLE_FILENAME[] = "_CONSOLE";
char HELP_FILENAME[] = "pgp.hlp";

/* These files use the environmental variable PGPPATH as a default path: */
char CONFIG_FILENAME[] = "config.txt";
char PUBLIC_KEYRING_FILENAME[32] = "pubring.pgp";
char SECRET_KEYRING_FILENAME[32] = "secring.pgp";
char RANDSEED_FILENAME[32] = "randseed.bin";

/* Flags which are global across the driver code files */
boolean verbose = FALSE;	/* -l option: display maximum information */
FILE	*pgpout;			/* Place for routine messages */

/* Prototype for function in config.c */
  
int processConfigFile( char *configFileName );

static void usage();
static void key_usage();
static void arg_error();
static void initsigs();
static void do_keyopt(char);
static void do_decrypt(char *);
static void do_armorfile(char *);
void user_error();
void exitPGP(int);


#if 0 /* unused */
boolean strcontains( char *s1, char *s2 )
{	/*
	**	Searches s1 for s2, without case sensitivity.
	**	Return TRUE if found.
	**
	**	If s2 is an empty string then return TRUE.  This is because,
	**	at least in the world of mathematics, the empty set is contained
	**	in all other sets.  The Microsoft C version 6.0 strstr function
	**	behaves this way but version 5.1 does not, so we need to
	**	explicitly test for the situation. -- ALH 91/2/17
	*/

	if (s2 != NULL && s2[0] != '\0')
	{
		char buf1[256], buf2[256];	/* scratch buffers */

		if (s1 != NULL)
		{
			strncpy( buf1, s1, 256 );
			strlwr( buf1 ); /* converts to lower case */
		}
		else
			buf1[0] = '\0';
		strncpy( buf2, s2, 256 );	strlwr( buf2 ); /* converts to lower case */

		if (strstr( buf1, buf2 ) == NULL)
			return( FALSE );		/* string not found */
	}
	return(TRUE);
}	/*	strcontains */
#endif


/*	Various compression signatures: PKZIP, Zoo, GIF, Arj, and HPACK.  
	Lha(rc) is handled specially in the code; it is missing from the 
	compressSig structure intentionally.  If more formats are added, 
	put them before lharc to keep the code consistent. 
*/
char *compressSig[] =  { "PK\03\04", "ZOO ", "GIF8", "\352\140", "HPAK" };
char *compressName[] = { "PKZIP",   "Zoo",  "GIF",  "Arj",  "Hpack", "LHarc" };
char *compressExt[] =  { ".zip",  ".zoo",  ".gif",  ".arj", ".hpk",  ".lzh" };

int compressSignature(byte *header)
/* Returns file signature type from a number of popular compression formats
   or -1 if no match */
{	int i;

	for (i=0; i<sizeof(compressSig)/sizeof(*compressSig); i++)
		if (!strncmp((char *)header, compressSig[i], strlen(compressSig[i])))
			return(i);
	/* Special check for lharc files */
	if (header[2]=='-' && header[3]=='l' && (header[4]=='z'||header[4]=='h') &&
		header[6]=='-')
		return(i);
	return(-1);
}	/* compressSignature */


boolean file_compressible(char *filename)
{	/* returns TRUE iff file is likely to be compressible */
	byte header[8];
	get_header_info_from_file( filename, header, 8 );
	if (compressSignature( header ) >= 0)
		return (FALSE);	/* probably not compressible */
	return (TRUE);	/* possibly compressible */
}	/* compressible */


/* Possible error exit codes - not all of these are used.  Note that we
   don't use the ANSI EXIT_SUCCESS and EXIT_FAILURE.  To make things
   easier for compilers which don't support enum we use #defines */

#define EXIT_OK					0
#define INVALID_FILE_ERROR		1
#define FILE_NOT_FOUND_ERROR	2
#define UNKNOWN_FILE_ERROR		3
#define NO_BATCH				4
#define BAD_ARG_ERROR			5
#define INTERRUPT				6
#define OUT_OF_MEM				7

/* Keyring errors: Base value = 10 */
#define KEYGEN_ERROR			10
#define NONEXIST_KEY_ERROR		11
#define KEYRING_ADD_ERROR		12
#define KEYRING_EXTRACT_ERROR	13
#define KEYRING_EDIT_ERROR		14
#define KEYRING_VIEW_ERROR		15
#define KEYRING_REMOVE_ERROR	16
#define KEYRING_CHECK_ERROR		17
#define KEY_SIGNATURE_ERROR		18
#define KEYSIG_REMOVE_ERROR		19

/* Encode errors: Base value = 20 */
#define SIGNATURE_ERROR			20
#define RSA_ENCR_ERROR			21
#define ENCR_ERROR				22
#define COMPRESS_ERROR			23

/* Decode errors: Base value = 30 */
#define SIGNATURE_CHECK_ERROR	30
#define RSA_DECR_ERROR			31
#define DECR_ERROR				32
#define DECOMPRESS_ERROR		33


#ifdef SIGINT
void breakHandler(int sig)
/* This function is called if a BREAK signal is sent to the program.  In this
   case we zap the temporary files.
*/
{
#ifdef UNIX
	if (sig == SIGPIPE)
	{	signal(SIGPIPE, SIG_IGN);
		exitPGP(INTERRUPT);
	}
	if (sig != SIGINT)
		fprintf(stderr, "\nreceived signal %d\n", sig);
	else
#endif
		fprintf(pgpout,PSTR("\nStopped at user request\n"));
	exitPGP(INTERRUPT);
}
#endif


void clearscreen(void)
{	/* Clears screen and homes the cursor. */
	fprintf(pgpout,"\n\033[0;0H\033[J\r           \r");	/* ANSI sequence. */
	fflush(pgpout);
}

void signon_msg()
{	/*	We had to process the config file first to possibly select the 
		foreign language to translate the sign-on line that follows... */
	word32 tstamp;
		/* display message only once to allow calling multiple times */
	static boolean printed = FALSE;

	if (filter_mode || printed)
		return;
	printed = TRUE;
	fprintf(stderr,PSTR("Pretty Good Privacy %s - Public-key encryption for the masses.\n"),
		rel_version);
#ifdef TEMP_VERSION
	fprintf(stderr, "Internal development version only - not for general release.\n");
#endif
	fprintf(stderr, PSTR("(c) 1990-1992 Philip Zimmermann, Phil's Pretty Good Software. %s\n"),
		rel_date);

	get_timestamp((byte *)&tstamp);	/* timestamp points to tstamp */
	fprintf(pgpout,"Date: %s\n",ctdate(&tstamp));
}


#ifdef TEMP_VERSION	/* temporary experimental version of PGP */
#include <time.h>
#define CREATION_DATE ((unsigned long) 0x2a6f17dcL)     
	/* CREATION_DATE is Thu Jul 23 14:34:36 1992 */ 
#define LIFESPAN	((unsigned long) 30L * (unsigned long) 86400L)
	/* LIFESPAN is 30 days */
void check_expiration_date(void)
{	/* If this is an experimental version of PGP, cut its life short */
	word32 t;
	t = time(NULL);
	if (t > (CREATION_DATE + LIFESPAN))
	{	fprintf(stderr,"\n\007This experimental version of PGP has expired.\n");
		exit(-1);	/* error exit */
	}
}	/* check_expiration_date */
#else	/* no expiration date */
#define check_expiration_date() /* null statement */
#endif	/* TEMP_VERSION */



/* -f means act as a unix-style filter */
/* -i means internalize extended file attribute information, only supported
 *          between like (or very compatible) operating systems.
/* -l means show longer more descriptive diagnostic messages */
/* -m means display plaintext output on screen, like unix "more" */
/* -d means decrypt only, leaving inner signature wrapping intact */
/* -t means treat as pure text and convert to canonical text format */

/* Used by getopt function... */
#define OPTIONS "abcdefghiklmo:prstu:vwxz:ABCDEFGHIKLMO:PRSTU:VWX?"
extern int optind;
extern char *optarg;

boolean emit_radix_64 = FALSE;		/* set by config file */
boolean sign_flag = FALSE;
boolean moreflag = FALSE;
boolean filter_mode = FALSE;
boolean preserve_filename = FALSE;
boolean decrypt_only_flag = FALSE;
boolean de_armor_only = FALSE;
boolean strip_sig_flag = FALSE;
boolean clear_signatures = FALSE;
boolean c_flag = FALSE;
#ifdef VMS	/* kludge for those stupid VMS variable-length text records */
char lit_mode = MODE_TEXT;	/* MODE_TEXT or MODE_BINARY for literal packet */
#else	/* not VMS */
char lit_mode = MODE_BINARY;	/* MODE_TEXT or MODE_BINARY for literal packet */
#endif	/* not VMS */
/* my_name is substring of default userid for secret key to make signatures */
char my_name[256] = "\0"; /* null my_name means take first userid in ring */
boolean keepctx = FALSE;	/* TRUE means keep .ctx file on decrypt */
boolean compress_enabled = TRUE;	/* attempt compression before encryption */
long timeshift = 0L;	/* seconds from GMT timezone */
boolean attempt_compression; /* attempt compression before encryption */
char *outputfile = NULL;
int errorLvl = EXIT_OK;
char mcguffin[256]; /* userid search tag */
char plainfile[MAX_PATH];
#define	MAXARGC	8
int myArgc = 2;
char *myArgv[MAXARGC] = { NULL, };
char password[256] = "";


int main(int argc, char *argv[])
{
	int status, opt;
	char *inputfile = NULL;
 	char *recipient = NULL;
	char *workfile, *tempf;
	boolean nestflag = FALSE;
	boolean decrypt_mode = FALSE;
	boolean wipeflag = FALSE;
	boolean armor_flag = FALSE;		/* -a option */
	boolean separate_signature = FALSE;
	boolean keyflag = FALSE;
	boolean encrypt_flag = FALSE;
	boolean conventional_flag = FALSE;
	char *clearfile = NULL;
	char *literal_file = NULL;
	char literal_file_name[MAX_PATH];
	char cipherfile[MAX_PATH];
	char keychar = '\0';
    char *p;
	byte ctb;

	/* Initial messages to stderr */
	pgpout = stderr;

#ifdef	DEBUG1
	verbose = TRUE;
#endif

#ifndef UNIX
	if ((p = getenv("PGPPASS")) != NULL)
		strcpy(password, p);
#else
	for (opt = 1; opt < argc; ++opt)
	{	if (strcmp(argv[opt], "-z") == 0 && ++opt < argc)
		{	strcpy(password, argv[opt]);
			for (p = argv[opt]; *p; ++p)
				*p = ' ';
		}
	}
#endif

	/* Process the config file first.  Any command-line arguments will
	   override the config file settings */
	buildfilename( mcguffin, CONFIG_FILENAME );
	if ( processConfigFile( mcguffin ) < 0 )
		exit(BAD_ARG_ERROR);
	init_charset();

#ifdef MSDOS	/* only on MSDOS systems */
	if ((p = getenv("TZ")) == NULL || *p == '\0')
	{	fprintf(pgpout,PSTR("\007WARNING: Environmental variable TZ is not defined, so GMT timestamps\n\
may be wrong.  See the PGP User's Guide to properly define TZ\n\
in AUTOEXEC.BAT file.\n"));
	}
#endif	/* MSDOS */

#ifdef XVMS /* When we can trust FILEIO.C, change this to VMS, HAJK */
#define TEMP "SYS$SCRATCH"
#else
#define TEMP "TMP"
#endif /* VMS */
	if ((p = getenv(TEMP)) != NULL && *p != '\0')
		settmpdir(p);

  	/* Process all the command-line option switches: */
 	while (optind < argc)
 	{	/*
 		 * Allow random order of options and arguments (like GNU getopt)
 		 * NOTE: this does not work with GNU getopt, use getopt.c from
 		 * the PGP distribution.
 		 */
 		if ((opt = getopt(argc, argv, OPTIONS)) == EOF)
 		{	if (optind == argc)		/* -- at end */
 				break;
 			if (myArgc == MAXARGC)
 			{	signon_msg();
				fprintf(pgpout, PSTR("\007Too many arguments.\n"));
 				errorLvl = BAD_ARG_ERROR;
 				user_error();
 			}
 			myArgv[myArgc++] = argv[optind++];
 			continue;
 		}
		opt = to_lower(opt);
		if (keyflag && (keychar == '\0' || (keychar == 'v' && opt == 'v')))
		{
			if (keychar == 'v')
				keychar = 'V';
			else
				keychar = opt;
			continue;
		}
		switch (opt)
		{
			case 'a': armor_flag = TRUE; emit_radix_64 = 1; break;
			case 'b': separate_signature = strip_sig_flag = TRUE; break;
			case 'c': encrypt_flag = conventional_flag = TRUE;
					  c_flag = TRUE; break;
			case 'd': decrypt_only_flag = TRUE; break;
			case 'e': encrypt_flag = TRUE; break;
			case 'f': filter_mode = TRUE; break;
			case '?':
			case 'h': usage(); break;
#ifdef VMS
			case 'i': lit_mode = MODE_LOCAL; break;
#endif /* VMS */
			case 'k': keyflag = TRUE; break;
			case 'l': verbose = TRUE; break;
			case 'm': moreflag = TRUE; break;
			case 'p': preserve_filename = TRUE; break;
			case 'o': outputfile = optarg; break;
			case 's': sign_flag = TRUE; break;
			case 't': lit_mode = MODE_TEXT; break;
			case 'u': strncpy(my_name, optarg, sizeof(my_name)-1);
				  CONVERT_TO_CANONICAL_CHARSET(my_name);
				  break;
			case 'w': wipeflag = TRUE; break;
			case 'z': break;
			/* '+' special option: does not require - */
			case '+':
				if (processConfigLine(optarg) == 0)
					break;
				fprintf(stderr, "\n");
				/* fallthrough */
			default:
				arg_error();
		}
	}
	if (keyflag && keychar == '\0')
		key_usage();

	signon_msg();
	check_expiration_date();	/* hobble any experimental version */

#ifndef UNIX
	if (!filter_mode)
		pgpout = stdout;
#endif

#if defined(UNIX) || defined(VMS)
	umask(077); /* Make files default to private */
#endif

	initsigs();

	if (keyflag)
	{	do_keyopt(keychar);
		exitPGP(EXIT_OK);
	}

	/* -db means break off signature certificate into separate file */
	if (decrypt_only_flag && strip_sig_flag)
		decrypt_only_flag = FALSE;

	if (decrypt_only_flag && armor_flag)
		decrypt_mode = de_armor_only = TRUE;

	if (outputfile != NULL)
		preserve_filename = FALSE;

	if (!sign_flag && !encrypt_flag && !conventional_flag && !armor_flag)
	{	if (wipeflag)	/* wipe only */
		{	if (myArgc != 3)
				arg_error();	/* need one argument */
			if (wipefile(myArgv[2]) == 0 && remove(myArgv[2]) == 0)
			{	fprintf(pgpout,PSTR("\nFile %s wiped and deleted. "),myArgv[2]);
				fprintf(pgpout, "\n");
				exitPGP(EXIT_OK);
			}
			exitPGP(UNKNOWN_FILE_ERROR);
		}
		/* decrypt if none of the -s -e -c -a -w options are specified */
		decrypt_mode = TRUE;
	}

 	if (myArgc == 2)		/* no arguments */
	{
#ifdef UNIX
		if (!filter_mode && !isatty(fileno(stdin)))
		{	/* piping to pgp without arguments and no -f:
			 * switch to filter mode but don't write output to stdout
			 * if it's a tty, use the preserved filename */
			filter_mode = TRUE;
			if (isatty(fileno(stdout)) && !moreflag)
				preserve_filename = TRUE;
		}
#endif
		if (!filter_mode)
		{	fprintf(pgpout,PSTR("\nFor details on licensing and distribution, see the PGP User's Guide.\
\nFor other cryptography products and custom development services, contact:\
\nPhilip Zimmermann, 3021 11th St, Boulder CO 80304 USA, phone (303)541-0140\n"));
			if (strcmp((p = PSTR("@translator@")), "@translator@"))
				fprintf(pgpout, p);
			fprintf(pgpout,PSTR("\nFor a usage summary, type:  pgp -h\n"));
			exit(BAD_ARG_ERROR);		/* error exit */
		}
	}
	else
 	{	if (filter_mode)
 			recipient = myArgv[2];
 		else
 		{	inputfile = myArgv[2];
 			recipient = myArgv[3];
 		}
 	}


	if (filter_mode)
	{	inputfile = "stdin";
		filter_mode = TRUE;
	}
	else
	{	if (decrypt_mode && no_extension(inputfile))
		{	strcpy(cipherfile, inputfile);
			force_extension( cipherfile, ASC_EXTENSION );
			if (file_exists (cipherfile))
				inputfile = cipherfile;
			else
			{	force_extension( cipherfile, CTX_EXTENSION );
				if (file_exists (cipherfile))
					inputfile = cipherfile;
				else
				{	force_extension( cipherfile, SIG_EXTENSION );
					if (file_exists (cipherfile))
						inputfile = cipherfile;
				}
			}
		}
		if (! file_exists( inputfile ))
		{	fprintf(pgpout, PSTR("\007File [%s] does not exist.\n"), inputfile);
			errorLvl = FILE_NOT_FOUND_ERROR;
			user_error();
		}
		workfile = inputfile;
	}

	if (strlen(inputfile) >= (unsigned) MAX_PATH-4)
	{	fprintf(pgpout, PSTR("\007Invalid filename: [%s] too long\n"), inputfile );
		errorLvl = INVALID_FILE_ERROR;
		user_error();
	}
	strcpy(plainfile, inputfile);

	if (filter_mode)
		setoutdir(NULL);	/* NULL means use tmpdir */
	else
	{	if (outputfile)
			setoutdir(outputfile);
		else
			setoutdir(inputfile);
	}

	if (filter_mode)
	{	workfile = tempfile(TMP_WIPE|TMP_TMPDIR);
		readPhantomInput(workfile);
	}

	get_header_info_from_file( workfile, &ctb, 1 );
	if (decrypt_mode)
	{	if (!outputfile)
			outputfile = myArgv[3];
		if (!is_ctb(ctb) && is_armor_file(workfile, 0L))
			do_armorfile(workfile);
		else
			do_decrypt(workfile);
		exitPGP(EXIT_OK);
	}


	/*	See if plaintext input file was actually created by PGP earlier--
		If it was, maybe we should NOT encapsulate it in a literal packet.
		Otherwise, always encapsulate it.
	*/
	nestflag = FALSE;	/* First assume we will encapsulate it. */
	if (!filter_mode && legal_ctb(ctb))
	{	/*	Special case--may be a PGP-created packet, so
			do we inhibit encapsulation in literal packet? */
		fprintf(pgpout, PSTR("\n\007Input file '%s' looks like it may have been created by PGP. "),
			inputfile );
		fprintf(pgpout, PSTR("\nIs it safe to assume that it was created by PGP (y/N)? "));
		if (getyesno('n'))
			nestflag = TRUE;	/* don't re-encapsulate PGP packet */
	}	/* Possible ciphertext input file */

	if (moreflag)		/* special name to cause printout on decrypt */
	{	strcpy (literal_file_name, CONSOLE_FILENAME);
		lit_mode = MODE_TEXT;	/* will check for text file later */
	}
	else
	{	strcpy (literal_file_name, inputfile);
#ifdef MSDOS
		strlwr (literal_file_name);
#endif
	}
	literal_file = literal_file_name;

	/*	Make sure non-text files are not accidentally converted 
		to canonical text.  This precaution should only be followed 
		for US ASCII text files, since European text files may have 
		8-bit character codes and still be legitimate text files 
		suitable for conversion to canonical (CR/LF-terminated) 
		text format. */
	if (lit_mode==MODE_TEXT && !is_text_file(workfile))
	{	fprintf(pgpout, 
PSTR("\n\007Warning: '%s' is not a pure text file.\nFile will be treated as binary data.\n"), 
			workfile);
		lit_mode = MODE_BINARY; /* now expect straight binary */
	}

	if (moreflag && lit_mode==MODE_BINARY)	/* For eyes only?  Can't display binary file. */
	{	fprintf(pgpout, 
		PSTR("\n\007Error: Only text files may be sent as display-only.\n"));
		errorLvl = INVALID_FILE_ERROR;
		user_error();
	}		


	/*	See if plainfile looks like it might be incompressible, 
		by examining its contents for compression headers for 
		commonly-used compressed file formats like PKZIP, etc.
		Remember this information for later, when we are deciding
		whether to attempt compression before encryption.
	*/
	attempt_compression = compress_enabled && file_compressible(plainfile);


	if (sign_flag)
	{
		if (!filter_mode)
			fprintf(pgpout, PSTR("\nA secret key is required to make a signature. "));
		if (my_name[0] == '\0')
		{
			fprintf(pgpout, PSTR("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
		}

		if (lit_mode==MODE_TEXT)
		{		/* Text mode requires becoming canonical */
			tempf = tempfile(TMP_WIPE|TMP_TMPDIR);
			make_canonical( workfile, tempf );
			/* +clear means output file with signature in the clear,
			   only in combination with -t and -a, not with -e or -b */
			if (!encrypt_flag && !separate_signature &&
					emit_radix_64 && clear_signatures) {
				clearfile = workfile;
			} else {
				rmtemp(workfile);
			}
			workfile = tempf;
		}
		if ((emit_radix_64 || encrypt_flag) && !separate_signature)
			tempf = tempfile(TMP_WIPE|TMP_TMPDIR);
		else
			tempf = tempfile(TMP_WIPE);
		/* for clear signatures we create a separate signature */
		status = signfile(nestflag, separate_signature || (clearfile != NULL),
							my_name, workfile, tempf, lit_mode, literal_file );
		rmtemp(workfile);
		workfile = tempf;

		if (status < 0) /* signfile failed */
		{	fprintf(pgpout, PSTR("\007Signature error\n") );
			errorLvl = SIGNATURE_ERROR;
			user_error();
		}

		/*	If we just sign it without encryption, and we want radix-64
			output, we may as well compress it before converting to 
			radix-64 format. */
		if (attempt_compression && emit_radix_64 && !encrypt_flag &&
				!clearfile)
		{	tempf = tempfile(TMP_WIPE|TMP_TMPDIR);
			squish_file(workfile, tempf);
			rmtemp(workfile);
			workfile = tempf;
		}

	}	/* sign file */
	else if (!nestflag)
	{	/*	Prepend CTB_LITERAL byte to plaintext file.
			--sure wish this pass could be optimized away. */
		tempf = tempfile(TMP_WIPE);
		status = make_literal( workfile, tempf, lit_mode, literal_file );
		rmtemp(workfile);
		workfile = tempf;
	}

	if (encrypt_flag)
	{
		if (!conventional_flag)
		{
			if (!filter_mode)
				fprintf(pgpout, PSTR("\n\nRecipient's public key will be used to encrypt. "));
 			if (recipient == NULL)
			{	/* no recipient specified on command line */
				fprintf(pgpout, PSTR("\nA user ID is required to select the recipient's public key. "));
				fprintf(pgpout, PSTR("\nEnter the recipient's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			else
			{	/* recipient specified on command line */
 				strcpy( mcguffin, recipient );		/* Userid of recipient */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);
		}

		tempf = tempfile(TMP_WIPE);
		if (conventional_flag)
			status = idea_encryptfile( workfile, tempf, attempt_compression);
		else
			status = encryptfile( mcguffin, workfile, tempf, attempt_compression);

		rmtemp(workfile);
		workfile = tempf;

		if (status < 0)
		{	fprintf(pgpout, PSTR("\007Encryption error\n") );
			errorLvl = (conventional_flag ? ENCR_ERROR : RSA_ENCR_ERROR);
			user_error();
		}
	}	/* encrypt file */

	if (outputfile)		/* explicit output file overrides filter mode */
		filter_mode = (strcmp(outputfile, "-") == 0);

	if (filter_mode)
	{	if (emit_radix_64)
		{	/* NULL for outputfile means write to stdout */
			if (armor_file(workfile, NULL, inputfile, clearfile) != 0)
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
			if (clearfile)
				rmtemp(clearfile);
		}
		else
			if (writePhantomOutput(workfile) < 0)
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
		rmtemp(workfile);
	}
	else
	{	char name[MAX_PATH];
		if (outputfile)
			strcpy(name, outputfile);
		else
		{	strcpy(name, inputfile);
			drop_extension(name);
		}
		if (no_extension(name))
		{	if (emit_radix_64)
				force_extension(name, ASC_EXTENSION);
			else if (sign_flag && separate_signature)
				force_extension(name, SIG_EXTENSION);
			else
				force_extension(name, CTX_EXTENSION);
		}
		if (emit_radix_64)
		{
			if (armor_file(workfile, name, inputfile, clearfile) != 0)
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
			if (clearfile)
				rmtemp(clearfile);
		}
		else
		{	if ((outputfile = savetemp(workfile, name)) == NULL)
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
			if (!verbose && encrypt_flag)
				fprintf(pgpout, PSTR("\nCiphertext file: %s\n"), outputfile);
			else if (!verbose && sign_flag)
				fprintf(pgpout, PSTR("\nSignature file: %s\n"), outputfile);
		}
	}

	if (wipeflag)
	{	/* destroy every trace of plaintext */
		if (wipefile(inputfile) == 0)
		{	remove(inputfile);
			fprintf(pgpout,PSTR("\nFile %s wiped and deleted. "),inputfile);
			fprintf(pgpout, "\n");
		}
	}

	exitPGP(EXIT_OK);
}	/* main */

#ifdef MSDOS
#include <dos.h>
static char *dos_errlst[] = {
	"Write protect error",		/* PSTR ("Write protect error") */
	"Unknown unit",
	"Drive not ready",			/* PSTR ("Drive not ready") */
	"3", "4", "5", "6", "7", "8", "9",
	"Write error",				/* PSTR ("Write error") */
	"Read error",				/* PSTR ("Read error") */
	"General failure",
};

/* handler for msdos 'harderrors' */
#ifndef OS2
#ifdef __TURBOC__	/* Turbo C 2.0 */
static int dostrap(int errval)
#else
static void dostrap(unsigned deverr, unsigned errval)
#endif
{
	char errbuf[64];
	int i;
	sprintf(errbuf, "\r\nDOS error: %s\r\n", dos_errlst[errval]);
	i = 0;
	do
		bdos(2,(unsigned int)errbuf[i],0);
	while (errbuf[++i]);
#ifdef __TURBOC__
	return 0;	/* ignore (fopen will return NULL) */
#else
	return;
#endif
}
#endif /* MSDOS */
#endif

static void initsigs()
{
#ifdef MSDOS
#ifndef OS2
#ifdef __TURBOC__
	harderr(dostrap);
#else /* MSC */
	_harderr(dostrap);
#endif
#endif
#endif /* MSDOS */
#ifdef SIGINT
#ifdef ATARI
	signal(SIGINT,(sigfunc_t) breakHandler);
#else
	signal(SIGINT,breakHandler);
#if defined(UNIX) || defined(VMS)
	signal(SIGHUP,breakHandler);
	signal(SIGQUIT,breakHandler);
#ifdef UNIX
	signal(SIGPIPE,breakHandler);
#endif
	signal(SIGTERM,breakHandler);
#ifndef DEBUG
	signal(SIGSEGV,breakHandler);
	signal(SIGILL,breakHandler);
#ifdef SIGBUS
	signal(SIGBUS,breakHandler);
#endif
#endif /* DEBUG */
#endif /* UNIX */
#endif /* not Atari */
#endif /* SIGINT */
}	/* initsigs */


static void do_armorfile(char *armorfile)
{
	char *tempf;
	char cipherfile[MAX_PATH];
	long linepos = 0;
	int status;

	while (TRUE)
	{	/* Handle transport armor stripping */
		tempf = tempfile(0);
		status = de_armor_file(armorfile,tempf,&linepos);
		if (status)
		{	fprintf(pgpout,PSTR("\n\007Error: Transport armor stripping failed for file %s\n"),armorfile);
			errorLvl = INVALID_FILE_ERROR;
			user_error();	 /* Bad file */
		}
		if (keepctx || de_armor_only)
		{	if (outputfile)
				strcpy(cipherfile, outputfile);
			else
			{	strcpy(cipherfile, file_tail(armorfile));
				force_extension(cipherfile, CTX_EXTENSION);
			}
			if ((tempf = savetemp(tempf, cipherfile)) == NULL)
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
			fprintf(pgpout,PSTR("Stripped transport armor from '%s', producing '%s'.\n"),
				armorfile, tempf);
			if (!de_armor_only)
				do_decrypt(tempf);
		}
		else
		{	do_decrypt(tempf);
			rmtemp(tempf);
		}

		if (!is_armor_file(armorfile, linepos))
			break;

		fprintf (pgpout, PSTR("\nLooking for next packet in '%s'...\n"), armorfile);
	}
}	/* do_armorfile */


static void do_decrypt(char *cipherfile)
{
	char *outfile = NULL;
	int status, i;
	boolean nested_info = FALSE;
	char ringfile[MAX_PATH];
	byte ctb;
	byte header[8]; /* used to classify file type at the end. */
	char preserved_name[MAX_PATH];
	char *newname;

	/* will be set to the original file name after processing a literal packet */
	preserved_name[0] = '\0';

	do	/* while nested parsable info present */
	{
		if (nested_info)
		{	rmtemp(cipherfile);		/* never executed on first pass */
			cipherfile = outfile;
		}
		if (get_header_info_from_file( cipherfile, &ctb, 1) < 0)
		{	fprintf(pgpout,PSTR("\n\007Can't open ciphertext file '%s'\n"),cipherfile);
			errorLvl = FILE_NOT_FOUND_ERROR;
			user_error();
		}

		if (!is_ctb(ctb))		/* not a real CTB -- complain */
			break;

		outfile = tempfile(TMP_WIPE);

		/* PKE is Public Key Encryption */
		if (is_ctb_type( ctb, CTB_PKE_TYPE ))
		{
			if (!filter_mode)
				fprintf(pgpout,PSTR("\nFile is encrypted.  Secret key is required to read it. "));

			/* Decrypt to scratch file since we may have a LITERAL2 */
			status = decryptfile( cipherfile, outfile );

			if (status < 0) /* error return */
			{	errorLvl = RSA_DECR_ERROR;
				user_error();
			}
			nested_info = (status > 0);
		}		/* outer CTB is PKE type */

		if (is_ctb_type( ctb, CTB_SKE_TYPE ))
		{
			if (decrypt_only_flag)
			{	/* swap file names instead of just copying the file */
				rmtemp(outfile);
				outfile = cipherfile;
				cipherfile = NULL;
				fprintf(pgpout,PSTR("\nThis file has a signature, which will be left in place.\n"));
				break;	/* Do no more */
			}
			fprintf(pgpout,PSTR("\nFile has signature.  Public key is required to check signature. "));

			status = check_signaturefile( cipherfile, outfile, strip_sig_flag, preserved_name );

			if (status < 0) /* error return */
			{	errorLvl = SIGNATURE_CHECK_ERROR;
				user_error();
			}
			nested_info = (status > 0);

			if (strcmp(preserved_name, "/dev/null") == 0)
			{	rmtemp(outfile);
				fprintf(pgpout, "\n");
				return;
			}
		}		/* outer CTB is SKE type */


		if (is_ctb_type( ctb, CTB_CKE_TYPE ))
		{		/* Conventional Key Encrypted ciphertext. */
			/* Tell user it's encrypted here, and prompt for password in subroutine. */
			if (!filter_mode)
				fprintf(pgpout,PSTR("\nFile is conventionally encrypted.  "));
			/* Decrypt to scratch file since it may be a LITERAL2 */
			status = idea_decryptfile( cipherfile, outfile );
			if (status < 0) /* error return */
			{	errorLvl = DECR_ERROR;
				user_error();	/* error exit status */
			}
			nested_info = (status > 0);
		}		/* CTB is CKE type */


		if (is_ctb_type( ctb, CTB_COMPRESSED_TYPE ))
		{		/* Compressed text. */
			status = decompress_file( cipherfile, outfile );
			if (status < 0) /* error return */
			{	errorLvl = DECOMPRESS_ERROR;
				user_error();
			}
			/* Always assume nested information... */
			nested_info = TRUE;
		}		/* CTB is COMPRESSED type */


		if (is_ctb_type( ctb, CTB_LITERAL_TYPE ) || is_ctb_type( ctb, CTB_LITERAL2_TYPE))
		{		/* Raw plaintext.  Just copy it.  No more nesting. */
			/* Strip off CTB_LITERAL prefix byte from file: */
			/* strip_literal may alter plainfile; will set mode */
			status = strip_literal( cipherfile, outfile,
					preserved_name, &lit_mode);
			if (status < 0) /* error return */
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
			nested_info = FALSE;
		}		/* CTB is LITERAL type */


		if ((ctb == CTB_CERT_SECKEY) || (ctb == CTB_CERT_PUBKEY))
		{	rmtemp(outfile);
			if (decrypt_only_flag)
			{	/* swap file names instead of just copying the file */
				outfile = cipherfile;
				cipherfile = NULL;
				break;	/* no further processing */
			}
				/* Key ring.  View it. */
			fprintf(pgpout, PSTR("\nFile contains key(s).  Contents follow...") );
			if (filter_mode && !preserve_filename)
			{	/* filter mode explicit requested with -f */
				if (view_keyring( NULL, cipherfile, TRUE, FALSE ) < 0)
				{	errorLvl = KEYRING_VIEW_ERROR;
					user_error();
				}
				return; /*	No output file */
			}
			if (ctb == CTB_CERT_SECKEY)
				buildfilename(ringfile,SECRET_KEYRING_FILENAME);
			else
				buildfilename(ringfile,PUBLIC_KEYRING_FILENAME);
			/*	Ask if it should be put on key ring */
			status = addto_keyring(cipherfile,ringfile,TRUE);
			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring add error. ") );
				errorLvl = KEYRING_ADD_ERROR;
				user_error();
			}
			return; /*	No output file */
		}		/* key ring.  view it. */

	} while (nested_info);
	/* No more nested parsable information */

	if (outfile == NULL)	/* file was not encrypted */
	{	if (!filter_mode && !moreflag)
		{	fprintf(pgpout,PSTR("\007\nError: '%s' is not a ciphertext, signature, or key file.\n"),
				cipherfile);
			errorLvl = UNKNOWN_FILE_ERROR;
			user_error();
		}
		outfile = cipherfile;
	}
	else
		if (cipherfile)
			rmtemp(cipherfile);

	if (moreflag || (strcmp(preserved_name,CONSOLE_FILENAME) == 0))
	{	/* blort to screen */
		if (strcmp(preserved_name,CONSOLE_FILENAME) == 0)
		{	fprintf(pgpout, 
			PSTR("\n\nThis message is marked \"For your eyes only\".  Display now (Y/n)? "));
			if (!getyesno('y'))
			{	/* no -- abort display, and clean up */
				rmtemp(outfile);
				return;
			}
		}
		fprintf(pgpout, PSTR("\n\nPlaintext message follows...\n"));
		more_file(outfile);
		/* Disallow saving to disk if outfile is console-only: */
		if (strcmp(preserved_name,CONSOLE_FILENAME) == 0)
			clearscreen();	/* remove all evidence */
		else
		{	fprintf(pgpout, PSTR("Save this file permanently (y/N)? "));
			if (getyesno('n'))
			{	char moreFilename[256];
				fprintf(pgpout,PSTR("Enter filename to save file as: "));
				if (preserved_name[0])
					fprintf(pgpout, "[%s]: ", file_tail(preserved_name));
				getstring( moreFilename, 255, TRUE );
				if (*moreFilename == '\0')
				{	if (*preserved_name != '\0')
						savetemp (outfile, file_tail(preserved_name));
					else
						rmtemp(outfile);
				}
				else
					savetemp (outfile, moreFilename);
				return;
			}
		}
		rmtemp(outfile);
		return;
	}	/* blort to screen */

	if (outputfile)
	{	if (!strcmp(outputfile, "/dev/null"))
		{	rmtemp(outfile);
			return;
		}
		filter_mode = (strcmp(outputfile, "-") == 0);
		strcpy(plainfile, outputfile);
	}
	else
#ifdef VMS
		/* VMS null extension has to be ".", not "" */
		force_extension(plainfile, ".");
#else	/* not VMS */
		drop_extension(plainfile);
#endif	/* not VMS */

	if (!preserve_filename && filter_mode)
	{	if (writePhantomOutput(outfile) < 0)
		{	errorLvl = UNKNOWN_FILE_ERROR;
			user_error();
		}
		rmtemp(outfile);
		return;
	}

	if (preserve_filename && preserved_name[0] != '\0')
		strcpy(plainfile, file_tail(preserved_name));

	if (!verbose)	/* if other filename messages were suppressed */
		fprintf(pgpout,PSTR("\nPlaintext filename: %s"), plainfile);


	/*---------------------------------------------------------*/

	/*	One last thing-- let's attempt to classify some of the more
		frequently occurring cases of plaintext output files, as an
		aid to the user.

		For example, if output file is a public key, it should have
		the right extension on the filename.

		Also, it will likely be common to encrypt files created by
		various archivers, so they should be renamed with the archiver
		extension.
	*/
	get_header_info_from_file( outfile, header, 8 );

	newname = NULL;
	if (header[0] == CTB_CERT_PUBKEY)
	{	/* Special case--may be public key, worth renaming */
		fprintf(pgpout, PSTR("\nPlaintext file '%s' looks like it contains a public key."),
			plainfile );
		newname = maybe_force_extension( plainfile, PGP_EXTENSION );
	}	/* Possible public key output file */

	else
	if ((i = compressSignature( header )) >= 0)
	{	/*	Special case--may be an archived/compressed file, worth renaming	*/
		fprintf(pgpout, PSTR("\nPlaintext file '%s' looks like a %s file."),
			plainfile, compressName[i] );
		newname = maybe_force_extension( plainfile, compressExt[i] );
	}	/*	Possible archived/compressed output file	*/

	else
	if (is_ctb(header[0]) &&
	   (is_ctb_type (header[0], CTB_PKE_TYPE)
	 || is_ctb_type (header[0], CTB_SKE_TYPE)
	 || is_ctb_type (header[0], CTB_CKE_TYPE)))
	{	/* Special case--may be another ciphertext file, worth renaming */
		fprintf(pgpout, PSTR("\n\007Output file '%s' may contain more ciphertext or signature."),
			plainfile );
		newname = maybe_force_extension( plainfile, CTX_EXTENSION );
	}	/* Possible ciphertext output file */

	if (savetemp(outfile, (newname ? newname : plainfile)) == NULL)
	{	errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	}

	fprintf (pgpout, "\n");

} /* do_decrypt */


static void do_keyopt(char keychar)
{
	char keyfile[MAX_PATH];
	char ringfile[MAX_PATH];
	int status;

	if (filter_mode && keychar != 'r')
	{	errorLvl = NO_BATCH;
		arg_error();	 /* interactive process, no go in batch mode */
	}

	switch (keychar)
	{

		/*-------------------------------------------------------*/
		case 'g':
		{	/*	Key generation
				Arguments: bitcount, bitcount
			*/
			char	keybits[6], ebits[6];

			if (myArgc > 2)
				strncpy( keybits, myArgv[2], sizeof(keybits)-1 );
			else
				keybits[0] = '\0';

			if (myArgc > 3)
				strncpy( ebits, myArgv[3], sizeof(ebits)-1 );
			else
				ebits[0] = '\0';

			/* dokeygen writes the keys out to the key rings... */
			status = dokeygen(keybits, ebits);

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keygen error. ") );
				errorLvl = KEYGEN_ERROR;
				user_error();
			}
			return;
		}	/* Key generation */

		/*-------------------------------------------------------*/
		case 'c':
		{	/*	Key checking
				Arguments: userid, ringfile
			*/

			if (myArgc < 3)		/* Default to all user ID's */
				mcguffin[0] = '\0';
			else
			{	strcpy ( mcguffin, myArgv[2] );
				if (strcmp( mcguffin, "*" ) == 0)
					mcguffin[0] = '\0';
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			if (myArgc < 4) /* default key ring filename */
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );
			else
				strncpy( ringfile, myArgv[3], sizeof(ringfile)-1 );

			if ((myArgc < 4 && myArgc > 2)	/* Allow just key file as arg */
			&& has_extension( myArgv[2], PGP_EXTENSION ) )
			{	strcpy( ringfile, myArgv[2] );
				mcguffin[0] = '\0';
			}

			status = dokeycheck( mcguffin, ringfile, NULL );

			errorLvl = 0;
			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring check error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
			}
			if (errorLvl == 0 && mcguffin[0] != '\0')
				return;	/* just checking a single user, dont do maintenance */

			if ((status = maintenance(ringfile, 0)) < 0 && status != -7)
			{	fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
			}
			if (errorLvl)
				user_error();

			return;
		}	/* Key check */

		/*-------------------------------------------------------*/
		case 'm':
		{	/*	Maintenance pass
				Arguments: ringfile
			*/

			if (myArgc < 3) /* default key ring filename */
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );
			else
				strcpy( ringfile, myArgv[2] );

#ifdef MSDOS
			strlwr( ringfile );
#endif
			if (! file_exists( ringfile ))
				default_extension( ringfile, PGP_EXTENSION );

			if ((status = maintenance(ringfile,
					MAINT_VERBOSE|(c_flag ? MAINT_CHECK : 0))) < 0)
			{	if (status == -7)
					fprintf(pgpout, PSTR("File '%s' is not a public keyring\n"), ringfile);
				fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
				user_error();
			}
			return;
		}	/* Maintenance pass */

		/*-------------------------------------------------------*/
		case 's':
		{	/*	Key signing
				Arguments: her_id, keyfile
			*/

			if (myArgc >= 4)
				strncpy( keyfile, myArgv[3], sizeof(keyfile)-1 );
			else
				buildfilename( keyfile, PUBLIC_KEYRING_FILENAME );

			if (myArgc >= 3)
				strcpy( mcguffin, myArgv[2] );	/* Userid to sign */
			else
			{
				fprintf(pgpout, PSTR("\nA user ID is required to select the public key you want to sign. "));
				fprintf(pgpout, PSTR("\nEnter the public key's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			if (my_name[0] == '\0')
			{
				fprintf(pgpout, PSTR("\nA secret key is required to make a signature. "));
				fprintf(pgpout, PSTR("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
			}

			status = signkey ( mcguffin, my_name, keyfile );

			if (status >= 0) {
				status = maintenance(keyfile, MAINT_SILENT);
				if (status == -7)	/* ringfile is a keyfile or secret keyring */
				{	fprintf(pgpout, "Warning: '%s' is not a public keyring\n", keyfile);
					return;
				}
				if (status < 0)
					fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
			}

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Key signature error. ") );
				errorLvl = KEY_SIGNATURE_ERROR;
				user_error();
			}
			return;
		}	/* Key signing */


		/*-------------------------------------------------------*/
		case 'd':
		{	/*	Key compromise
				Arguments: userid, keyfile
			*/

			if (myArgc >= 4)
				strncpy( keyfile, myArgv[3], sizeof(keyfile)-1 );
			else
				buildfilename( keyfile, PUBLIC_KEYRING_FILENAME );

			if (myArgc >= 3)
				strcpy( mcguffin, myArgv[2] );	/* Userid to sign */
			else
			{
				fprintf(pgpout, PSTR("\nA user ID is required to select the key you want to revoke. "));
				fprintf(pgpout, PSTR("\nEnter user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			status = compromise ( mcguffin, keyfile );

			if (status >= 0) {
				status = maintenance(keyfile, MAINT_SILENT);
				if (status == -7)	/* ringfile is a keyfile or secret keyring */
				{	fprintf(pgpout, "Warning: '%s' is not a public keyring\n", keyfile);
					return;
				}
				if (status < 0)
					fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
			}

			if (status < 0)
			{
				errorLvl = KEY_SIGNATURE_ERROR;
				user_error();
			}
			return;
		}	/* Key compromise */

		/*-------------------------------------------------------*/
		case 'e':
		{	/*	Key editing
				Arguments: userid, ringfile
			*/

			if (myArgc >= 4)
				strncpy( ringfile, myArgv[3], sizeof(ringfile)-1 );
			else	/* default key ring filename */
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );

			if (myArgc >= 3)
				strcpy( mcguffin, myArgv[2] );	/* Userid to edit */
			else
			{
				fprintf(pgpout, PSTR("\nA user ID is required to select the key you want to edit. "));
				fprintf(pgpout, PSTR("\nEnter the key's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			status = dokeyedit( mcguffin, ringfile );

			if (status >= 0) {
				status = maintenance(ringfile, MAINT_SILENT);
				if (status < 0 && status != -7)
					fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
			}

			if (status < 0 && status != -7)
			{	fprintf(pgpout, PSTR("\007Keyring edit error. ") );
				errorLvl = KEYRING_EDIT_ERROR;
				user_error();
			}
			return;
		}	/* Key edit */

		/*-------------------------------------------------------*/
		case 'a':
		{	/*	Add key to key ring
				Arguments: keyfile, ringfile
			*/

			if (myArgc < 3)
				arg_error();

			strncpy( keyfile, myArgv[2], sizeof(keyfile)-1 );

#ifdef MSDOS
			strlwr( keyfile	 );
#endif
			if (! file_exists( keyfile ))
				default_extension( keyfile, PGP_EXTENSION );

			if (! file_exists( keyfile ))
			{	fprintf(pgpout, PSTR("\n\007Key file '%s' does not exist.\n"), keyfile );
				errorLvl = NONEXIST_KEY_ERROR;
				user_error();
			}

			if (myArgc < 4) /* default key ring filename */
			{	byte ctb;
				get_header_info_from_file(keyfile, &ctb, 1);
				if (ctb == CTB_CERT_SECKEY)
					buildfilename(ringfile,SECRET_KEYRING_FILENAME);
				else
					buildfilename(ringfile,PUBLIC_KEYRING_FILENAME);
			}
			else
			{	strncpy( ringfile, myArgv[3], sizeof(ringfile)-1 );
				default_extension( ringfile, PGP_EXTENSION );
			}
#ifdef MSDOS
			strlwr( ringfile );
#endif

			status = addto_keyring( keyfile, ringfile, FALSE );

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring add error. ") );
				errorLvl = KEYRING_ADD_ERROR;
				user_error();
			}
			return;
		}	/* Add key to key ring */

		/*-------------------------------------------------------*/
		case 'x':
		{	/*	Extract key from key ring
				Arguments: mcguffin, keyfile, ringfile
			*/

			if (myArgc >= 5)	/* default key ring filename */
				strncpy( ringfile, myArgv[4], sizeof(ringfile)-1 );
			else
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );

			if (myArgc >= 3)
				strcpy( mcguffin, myArgv[2] );	/* Userid to extract */
			else
			{
				fprintf(pgpout, PSTR("\nA user ID is required to select the key you want to extract. "));
				fprintf(pgpout, PSTR("\nEnter the key's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			if (myArgc >= 4)
			{	strncpy( keyfile, myArgv[3], sizeof(keyfile)-1 );
			}
			else
				keyfile[0] = '\0';

#ifdef MSDOS
			strlwr( keyfile	 );
			strlwr( ringfile );
#endif

			default_extension( ringfile, PGP_EXTENSION );

			status = extract_from_keyring( mcguffin, keyfile,
					ringfile, emit_radix_64 );

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring extract error. ") );
				errorLvl = KEYRING_EXTRACT_ERROR;
				user_error();
			}
			return;
		}	/* Extract key from key ring */

		/*-------------------------------------------------------*/
		case 'r':
		{	/*	Remove keys or selected key signatures from userid keys
				Arguments: userid, ringfile
			*/

			if (myArgc >= 4)
				strcpy( ringfile, myArgv[3] );
			else	/* default key ring filename */
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );

			if (myArgc >= 3)
				strcpy( mcguffin, myArgv[2] );	/* Userid to work on */
			else
			{	if (sign_flag)
				{
					fprintf(pgpout, PSTR("\nA user ID is required to select the public key you want to\n\
remove certifying signatures from. "));
				}
				else
				{
					fprintf(pgpout, PSTR("\nA user ID is required to select the key you want to remove. "));
				}
				fprintf(pgpout, PSTR("\nEnter the key's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

#ifdef MSDOS
			strlwr( ringfile );
#endif
			if (! file_exists( ringfile ))
				default_extension( ringfile, PGP_EXTENSION );

			if (sign_flag)		/* Remove signatures */
			{	if (remove_sigs( mcguffin, ringfile ) < 0)
				{	fprintf(pgpout, PSTR("\007Key signature remove error. ") );
					errorLvl = KEYSIG_REMOVE_ERROR;
					user_error();
				}
			}
			else		/* Remove keyring */
			{	if (remove_from_keyring( NULL, mcguffin, ringfile, (boolean) (myArgc < 4) ) < 0)
				{	fprintf(pgpout, PSTR("\007Keyring remove error. ") );
					errorLvl = KEYRING_REMOVE_ERROR;
					user_error();
				}
			}
			if ((status = maintenance(ringfile, MAINT_SILENT)) < 0 && status != -7)
			{	fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
				user_error();
			}
			return;
		}	/* remove key signatures from userid */

		/*-------------------------------------------------------*/
		case 'v':
		case 'V':		/* -kvv */
		{	/*	View or remove key ring entries, with userid match
				Arguments: userid, ringfile
			*/

			if (myArgc < 4) /* default key ring filename */
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );
			else
				strcpy( ringfile, myArgv[3] );

			if (myArgc > 2)
			{	strcpy( mcguffin, myArgv[2] );
				if (strcmp( mcguffin, "*" ) == 0)
					mcguffin[0] = '\0';
			}
			else
				*mcguffin = '\0';

			if ((myArgc == 3) && has_extension( myArgv[2], PGP_EXTENSION ))
			{	strcpy( ringfile, myArgv[2] );
				mcguffin[0] = '\0';
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

#ifdef MSDOS
			strlwr( ringfile );
#endif
			if (! file_exists( ringfile ))
				default_extension( ringfile, PGP_EXTENSION );

			/* If a second 'v' (keychar = V), show signatures too */
			if (view_keyring(mcguffin, ringfile, (boolean) (keychar == 'V'), c_flag) < 0)
			{	fprintf(pgpout, PSTR("\007Keyring view error. ") );
				errorLvl = KEYRING_VIEW_ERROR;
				user_error();
			}
			return;
		}	/* view key ring entries, with userid match */


		default:
			arg_error();
	}

} /* do_keyopt */



void user_error() /* comes here if user made a boo-boo. */
{
	fprintf(pgpout,PSTR("\nFor a usage summary, type:  pgp -h\n"));
	fprintf(pgpout,PSTR("For more detailed help, consult the PGP User's Guide.\n"));
	exitPGP(errorLvl);		/* error exit */
}

/*
 * exitPGP: wipes and removes temporary files, also tries to wipe
 * the stack.
 */
void exitPGP(int returnval)
{
	char buf[STACK_WIPE];

	if (verbose)
		fprintf(pgpout, "exitPGP: exitcode = %d\n", returnval);
	memset(password, 0, sizeof(password));
	cleanup_tmpf();
	memset(buf, 0, sizeof(buf));	/* wipe stack */
#ifdef VMS
/*
 * Fake VMS style error returns with severity in bottom 3 bits
 */
	if (returnval)
	    returnval = (returnval << 3) | 0x10000002;
	else
	    returnval = 0x10000001;
#endif /* VMS */
	exit(returnval);
}


static void arg_error()
{
	signon_msg();
	fprintf(pgpout,PSTR("\nInvalid arguments.\n"));
	errorLvl = BAD_ARG_ERROR;
	user_error();
}

static void build_helpfile(char *helpfile)
{
	if (strcmp(language, "en"))
	{	buildfilename(helpfile, language);
		force_extension(helpfile, HLP_EXTENSION);
		if (!file_exists(helpfile))
			buildfilename(helpfile, HELP_FILENAME);
	}
	else
		buildfilename(helpfile, HELP_FILENAME);
}

static void usage()
{
	char helpfile[MAX_PATH];
	char *tmphelp = helpfile;
	extern unsigned char *ext_c_ptr;

	signon_msg();
	build_helpfile(helpfile);
  
	if (ext_c_ptr)
	{	/* conversion to external format necessary */
		tmphelp = tempfile(TMP_TMPDIR);
		CONVERSION = EXT_CONV;
		if (copyfiles_by_name(helpfile, tmphelp) < 0)
			tmphelp = helpfile;
		CONVERSION = NO_CONV;
	}

	  /* built-in help if pgp.hlp is not available */
	if (more_file(tmphelp) < 0)
		fprintf(pgpout,PSTR("\nUsage summary:\
\nTo encrypt a plaintext file with recipent's public key, type:\
\n   pgp -e textfile her_userid                 (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key:\
\n   pgp -s textfile [-u your_userid]           (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key, and then encrypt it\
\n   with recipent's public key, producing a .pgp file:\
\n   pgp -es textfile her_userid [-u your_userid]\
\nTo encrypt with conventional encryption only:\
\n   pgp -c textfile\
\nTo decrypt or check a signature for a ciphertext (.pgp) file:\
\n   pgp ciphertextfile [plaintextfile]\
\nTo produce output in ASCII for email, add the -a option to other options.\
\nTo generate your own unique public/secret key pair:  pgp -kg\
\nFor help on other key management functions, type:   pgp -k\n"));
	if (ext_c_ptr)
		rmtemp(tmphelp);
	exit(BAD_ARG_ERROR);		/* error exit */
}


static void key_usage()
{
	char helpfile[MAX_PATH];

	signon_msg();
	build_helpfile(helpfile);
	if (file_exists(helpfile))
	{	fprintf(pgpout,PSTR("\nFor a usage summary, type:  pgp -h\n"));
		fprintf(pgpout,PSTR("For more detailed help, consult the PGP User's Guide.\n"));
	}
	else
		fprintf(pgpout,PSTR("\nKey management functions:\
\nTo generate your own unique public/secret key pair:\
\n   pgp -kg\
\nTo add a key file's contents to your public or secret key ring:\
\n   pgp -ka keyfile [keyring]\
\nTo remove a key or a user ID from your public or secret key ring:\
\n   pgp -kr userid [keyring]\
\nTo edit your user ID or pass phrase:\
\n   pgp -ke your_userid [keyring]\
\nTo extract (copy) a key from your public or secret key ring:\
\n   pgp -kx userid keyfile [keyring]\
\nTo view the contents of your public key ring:\
\n   pgp -kv[v] [userid] [keyring]\
\nTo check signatures on your public key ring:\
\n   pgp -kc [userid] [keyring]\
\nTo sign someone else's public key on your public key ring:\
\n   pgp -ks her_userid [-u your_userid] [keyring]\
\nTo remove selected signatures from a userid on a keyring:\
\n   pgp -krs userid [keyring]\
\n"));
	exit(BAD_ARG_ERROR);		/* error exit */
}
