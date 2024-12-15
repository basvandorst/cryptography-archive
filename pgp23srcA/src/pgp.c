/* #define TEMP_VERSION	/ * if defined, temporary experimental version of PGP */
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
		Version 2.2 - 6 Mar 93
		Version 2.3 - 13 Jun 93
		Version 2.3a - 1 Jul 93

	(c) Copyright 1990-1993 by Philip Zimmermann.  All rights reserved.
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
#include "system.h"
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
#include "charset.h"
#include "getopt.h"
#include "config.h"
#include "keymaint.h"
#include "keyadd.h"
#include "rsaglue.h"
#ifdef  M_XENIX
char *strstr();
long time();
#endif

#ifdef MSDOS
#ifdef __ZTC__	/* Extend stack for Zortech C */
unsigned _stack_ = 24*1024;
#endif
#ifdef __TURBOC__
unsigned _stklen = 24*1024;
#endif
#endif
#define	STACK_WIPE	4096

/* Global filenames and system-wide file extensions... */
char rel_version[] = "2.3a";	/* release version */
#ifdef RSAREF
#define RSASTRING " (with RSAREF)"
#else
#define RSASTRING ""
#endif
static char rel_date[] = "1 Jul 93";	/* release date */
char PGP_EXTENSION[] = ".pgp";
char ASC_EXTENSION[] = ".asc";
char SIG_EXTENSION[] = ".sig";
char BAK_EXTENSION[] = ".bak";
static char HLP_EXTENSION[] = ".hlp";
char CONSOLE_FILENAME[] = "_CONSOLE";
static char HELP_FILENAME[] = "pgp.hlp";

/* These files use the environmental variable PGPPATH as a default path: */
static char CONFIG_FILENAME[] = "config.txt";
char PUBLIC_KEYRING_FILENAME[32] = "pubring.pgp";
char SECRET_KEYRING_FILENAME[32] = "secring.pgp";
char RANDSEED_FILENAME[32] = "randseed.bin";

/* Flags which are global across the driver code files */
boolean verbose = FALSE;	/* -l option: display maximum information */
FILE	*pgpout;			/* Place for routine messages */

static void usage(void);
static void key_usage(void);
static void arg_error(void);
static void initsigs(void);
static int do_keyopt(char);
static int do_decrypt(char *);
static void do_armorfile(char *);


/*	Various compression signatures: PKZIP, Zoo, GIF, Arj, and HPACK.  
	Lha(rc) is handled specially in the code; it is missing from the 
	compressSig structure intentionally.  If more formats are added, 
	put them before lharc to keep the code consistent. 
*/
static char *compressSig[] =  { "PK\03\04", "ZOO ", "GIF8", "\352\140",
	"HPAK", "\037\213", "\037\235", "\032\013", "\032HP%"
	/* lharc is special, must be last */ };
static char *compressName[] = { "PKZIP",   "Zoo",  "GIF",  "Arj",
	"Hpack", "gzip", "compressed", "PAK", "Hyper",
	"LHarc" };
static char *compressExt[] =  { ".zip",  ".zoo",  ".gif",  ".arj",
	".hpk", ".z", ".Z", ".pak", ".hyp",
	".lzh" };

/* "\032\0??", "ARC", ".arc" */

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


static boolean file_compressible(char *filename)
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


static void clearscreen(void)
{	/* Clears screen and homes the cursor. */
	fprintf(pgpout,"\n\033[0;0H\033[J\r           \r");	/* ANSI sequence. */
	fflush(pgpout);
}

static void signon_msg(void)
{	/*	We had to process the config file first to possibly select the 
		foreign language to translate the sign-on line that follows... */
	word32 tstamp;
		/* display message only once to allow calling multiple times */
	static boolean printed = FALSE;

	if (quietmode || printed)
		return;
	printed = TRUE;
	fprintf(stderr,PSTR("Pretty Good Privacy %s%s - Public-key encryption for the masses.\n"),
		rel_version, RSASTRING);
#ifdef TEMP_VERSION
	fprintf(stderr, "Internal development version only - not for general release.\n");
#endif
	fprintf(stderr, PSTR("(c) 1990-1993 Philip Zimmermann, Phil's Pretty Good Software. %s\n"),
		rel_date);

	get_timestamp((byte *)&tstamp);	/* timestamp points to tstamp */
	fprintf(pgpout,"Date: %s\n",ctdate(&tstamp));
}


#ifdef TEMP_VERSION	/* temporary experimental version of PGP */
#include <time.h>
#define CREATION_DATE ((unsigned long) 0x2C18C6BCL)
  /* CREATION_DATE is Fri Jun 11 17:54:04 1993 UTC */
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
 *          between like (or very compatible) operating systems. */
/* -l means show longer more descriptive diagnostic messages */
/* -m means display plaintext output on screen, like unix "more" */
/* -d means decrypt only, leaving inner signature wrapping intact */
/* -t means treat as pure text and convert to canonical text format */

/* Used by getopt function... */
#define OPTIONS "abcdefghiklmo:prstu:vwxz:ABCDEFGHIKLMO:PRSTU:VWX?"
extern int optind;
extern char *optarg;

boolean emit_radix_64 = FALSE;		/* set by config file */
static boolean sign_flag = FALSE;
boolean moreflag = FALSE;
boolean filter_mode = FALSE;
static boolean preserve_filename = FALSE;
static boolean decrypt_only_flag = FALSE;
static boolean de_armor_only = FALSE;
static boolean strip_sig_flag = FALSE;
boolean clear_signatures = FALSE;
boolean strip_spaces;
static boolean c_flag = FALSE;
boolean encrypt_to_self = FALSE; /* should I encrypt messages to myself? */
boolean batchmode = FALSE;	/* if TRUE: don't ask questions */
boolean quietmode = FALSE;
boolean force_flag = FALSE;	/* overwrite existing file without asking */
boolean pkcs_compat = 1;
#ifdef VMS	/* kludge for those stupid VMS variable-length text records */
char literal_mode = MODE_TEXT;	/* MODE_TEXT or MODE_BINARY for literal packet */
#else	/* not VMS */
char literal_mode = MODE_BINARY;	/* MODE_TEXT or MODE_BINARY for literal packet */
#endif	/* not VMS */
/* my_name is substring of default userid for secret key to make signatures */
char my_name[256] = "\0"; /* null my_name means take first userid in ring */
boolean keepctx = FALSE;	/* TRUE means keep .ctx file on decrypt */
/* Ask for each key separately if it should be added to the keyring */
boolean interactive_add = FALSE;
boolean compress_enabled = TRUE;	/* attempt compression before encryption */
long timeshift = 0L;	/* seconds from GMT timezone */
static boolean attempt_compression; /* attempt compression before encryption */
static char *outputfile = NULL;
static int errorLvl = EXIT_OK;
static char mcguffin[256]; /* userid search tag */
boolean signature_checked = FALSE;
char plainfile[MAX_PATH];
int myArgc = 2;
char **myArgv;
struct hashedpw *passwds = 0, *keypasswds = 0;
static struct hashedpw **passwdstail = &passwds;

int main(int argc, char *argv[])
{
	int status, opt;
	char *inputfile = NULL;
 	char **recipient = NULL;
	char **mcguffins;
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
	struct hashedpw *hpw;

	/* Initial messages to stderr */
	pgpout = stderr;

#ifdef	DEBUG1
	verbose = TRUE;
#endif
	/* The various places one can get passwords from.
	 * We accumulate them all into two lists.  One is
	 * to try on keys only, and is stored in no particular
	 * order, while the other is of unknown purpose so
	 * far (they may be used for conventional encryption
	 * or decryption as well), and are kept in a specific
	 * order.  If any password in the general list is found
	 * to decode a key, it is moved to the key list.
	 * The general list is not grown after initialization,
	 * so the tail pointer is not used after this.
	 */

	if ((p = getenv("PGPPASS")) != NULL)
	{	hpw = xmalloc(sizeof(struct hashedpw));
		hashpass(p, strlen(p), hpw->hash);
		/* Add to linked list of key passwords */
		hpw->next = keypasswds;
		keypasswds = hpw;
	}

	/* The -z "password" option should be used instead of PGPPASS if
	 * the environment can be displayed with the ps command (eg. BSD).
	 * If the system does not allow overwriting of the command line
	 * argument list but if it has a "hidden" environment, PGPPASS
	 * should be used.
	 */
	for (opt = 1; opt < argc; ++opt)
	{	p = argv[opt];
		if (p[0] != '-' || p[1] != 'z')
			continue;
		/* Accept either "-zpassword" or "-z password" */
		p += 2;
		if (!*p)
			p = argv[++opt];
		/* p now points to password */
		hpw = xmalloc(sizeof(struct hashedpw));
		hashpass(p, strlen(p), hpw->hash);
		/* Wipe password */
		while (*p)
			*p++ = ' ';
		/* Add to tail of linked list of passwords */
		hpw->next = 0;
		*passwdstail = hpw;
		passwdstail = &hpw->next;
	}
	/*
	 * If PGPPASSFD is set in the environment try to read the password
	 * from this file descriptor.  If you set PGPPASSFD to 0 pgp will
	 * use the first line read from stdin as password.
	 */
	if ((p = getenv("PGPPASSFD")) != NULL)
	{
		int passfd;
		if (*p && (passfd = atoi(p)) >= 0)
		{
			char pwbuf[256];
			p = pwbuf;
			while (read(passfd, p, 1) == 1 && *p != '\n')
				++p;
			hpw = xmalloc(sizeof(struct hashedpw));
			hashpass(pwbuf, p-pwbuf, hpw->hash);
			memset(pwbuf, 0, p-pwbuf);
			/* Add to tail of linked list of passwords */
			hpw->next = 0;
			*passwdstail = hpw;
			passwdstail = &hpw->next;
		}
	}

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

#ifdef VMS
#define TEMP "SYS$SCRATCH"
#else
#define TEMP "TMP"
#endif /* VMS */
	if ((p = getenv(TEMP)) != NULL && *p != '\0')
		settmpdir(p);

	if ((myArgv = (char **) malloc((argc + 2) * sizeof(char **))) == NULL) {
		fprintf(stderr, PSTR("\n\007Out of memory.\n"));
		exitPGP(7);
	}
	myArgv[0] = NULL;
	myArgv[1] = NULL;

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
			case 'i': literal_mode = MODE_LOCAL; break;
#endif /* VMS */
			case 'k': keyflag = TRUE; break;
			case 'l': verbose = TRUE; break;
			case 'm': moreflag = TRUE; break;
			case 'p': preserve_filename = TRUE; break;
			case 'o': outputfile = optarg; break;
			case 's': sign_flag = TRUE; break;
			case 't': literal_mode = MODE_TEXT; break;
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
	myArgv[myArgc] = NULL;	/* Just to make it NULL terminated */

	if (keyflag && keychar == '\0')
		key_usage();

	signon_msg();
	check_expiration_date();	/* hobble any experimental version */

	if (!filter_mode && (outputfile == NULL || strcmp(outputfile, "-")))
		pgpout = stdout;

#if defined(UNIX) || defined(VMS)
	umask(077); /* Make files default to private */
#endif

	initsigs();

	if (keyflag)
	{	status = do_keyopt(keychar);
		if (status < 0)
			user_error();
		exitPGP(status);
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
			if (!moreflag)
				pgpout = stderr;
			filter_mode = TRUE;
			if (isatty(fileno(stdout)) && !moreflag)
				preserve_filename = TRUE;
		}
#endif
		if (!filter_mode)
		{
			if (quietmode)
			{
				quietmode = FALSE;
				signon_msg();
			}
			fprintf(pgpout,PSTR("\nFor details on licensing and distribution, see the PGP User's Guide.\
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
 			recipient = &myArgv[2];
 		else
 		{	inputfile = myArgv[2];
 			recipient = &myArgv[3];
 		}
 	}


	if (filter_mode)
		inputfile = "stdin";
	else
	{	if (decrypt_mode && no_extension(inputfile))
		{	strcpy(cipherfile, inputfile);
			force_extension( cipherfile, ASC_EXTENSION );
			if (file_exists (cipherfile))
				inputfile = cipherfile;
			else
			{	force_extension( cipherfile, PGP_EXTENSION );
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
	else
		workfile = inputfile;

	get_header_info_from_file( workfile, &ctb, 1 );
	if (decrypt_mode)
	{	if (!outputfile && myArgc > 3)
			outputfile = myArgv[3];
		strip_spaces = FALSE;
		if (!is_ctb(ctb) && is_armor_file(workfile, 0L))
			do_armorfile(workfile);
		else
			if (do_decrypt(workfile) < 0)
				user_error();
		if (batchmode && !signature_checked)
			exitPGP(1);	/* alternate success, file did not have sig. */
		else
			exitPGP(EXIT_OK);
	}


	/*	See if plaintext input file was actually created by PGP earlier--
		If it was, maybe we should NOT encapsulate it in a literal packet.
		Otherwise, always encapsulate it.
	*/
	if (force_flag)	/* for use with batchmode, force nesting */
		nestflag = legal_ctb(ctb);
	else
		nestflag = FALSE;	/* First assume we will encapsulate it. */
	if (!batchmode && !filter_mode && legal_ctb(ctb))
	{	/*	Special case--may be a PGP-created packet, so
			do we inhibit encapsulation in literal packet? */
		fprintf(pgpout, PSTR("\n\007Input file '%s' looks like it may have been created by PGP. "),
			inputfile );
		fprintf(pgpout, PSTR("\nIs it safe to assume that it was created by PGP (y/N)? "));
		nestflag = getyesno('n');
	}	/* Possible ciphertext input file */

	if (moreflag)		/* special name to cause printout on decrypt */
	{	strcpy (literal_file_name, CONSOLE_FILENAME);
		literal_mode = MODE_TEXT;	/* will check for text file later */
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
	if (literal_mode==MODE_TEXT && !is_text_file(workfile))
	{	fprintf(pgpout, 
PSTR("\n\007Warning: '%s' is not a pure text file.\nFile will be treated as binary data.\n"), 
			workfile);
		literal_mode = MODE_BINARY; /* now expect straight binary */
	}

	if (moreflag && literal_mode==MODE_BINARY)	/* For eyes only?  Can't display binary file. */
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
		if (!filter_mode && !quietmode)
			fprintf(pgpout, PSTR("\nA secret key is required to make a signature. "));
		if (!quietmode && my_name[0] == '\0')
		{
			fprintf(pgpout, PSTR("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
		}

		strip_spaces = FALSE;
		clearfile = NULL;
		if (literal_mode==MODE_TEXT)
		{		/* Text mode requires becoming canonical */
			tempf = tempfile(TMP_WIPE|TMP_TMPDIR);
			/* +clear means output file with signature in the clear,
			   only in combination with -t and -a, not with -e or -b */
			if (!encrypt_flag && !separate_signature &&
					emit_radix_64 && clear_signatures) {
				clearfile = workfile;
				strip_spaces = TRUE;
			}
			make_canonical( workfile, tempf );
			if (!clearfile)
				rmtemp(workfile);
			workfile = tempf;
		}
		if ((emit_radix_64 || encrypt_flag) && !separate_signature)
			tempf = tempfile(TMP_WIPE|TMP_TMPDIR);
		else
			tempf = tempfile(TMP_WIPE);
		/* for clear signatures we create a separate signature */
		status = signfile(nestflag, separate_signature || (clearfile != NULL),
							my_name, workfile, tempf, literal_mode, literal_file );
		rmtemp(workfile);
		workfile = tempf;

		if (status < 0) /* signfile failed */
		{	fprintf(pgpout, PSTR("\007Signature error\n") );
			errorLvl = SIGNATURE_ERROR;
			user_error();
		}

		/*  We used to compress signed files only if they were also armored.
			Now that we have clear signatures it makes more sense to always
			compress signature files. */
		if (attempt_compression && !separate_signature && !encrypt_flag &&
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
		status = make_literal( workfile, tempf, literal_mode, literal_file );
		rmtemp(workfile);
		workfile = tempf;
	}

	if (encrypt_flag)
	{
		tempf = tempfile(TMP_WIPE);
		if (!conventional_flag)
		{
			if (!filter_mode && !quietmode)
				fprintf(pgpout, PSTR("\n\nRecipients' public key(s) will be used to encrypt. "));
 			if (recipient == NULL || *recipient == NULL || **recipient == '\0')
			{	/* no recipient specified on command line */
				fprintf(pgpout, PSTR("\nA user ID is required to select the recipient's public key. "));
				fprintf(pgpout, PSTR("\nEnter the recipient's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
				if ((mcguffins = (char **) malloc (2 * sizeof(char *))) == NULL) {
					fprintf(stderr, PSTR("\n\007Out of memory.\n"));
					exitPGP(7);
				}
				mcguffins[0] = mcguffin;
				mcguffins[1] = "";
			}
			else
			{	/* recipient specified on command line */
				mcguffins = recipient;
			}
			for (recipient = mcguffins; *recipient != NULL && 
			     **recipient != '\0'; recipient++) {
				CONVERT_TO_CANONICAL_CHARSET(*recipient);
			}
			status = encryptfile( mcguffins, workfile, tempf, attempt_compression);
		}
		else
			status = idea_encryptfile( workfile, tempf, attempt_compression);

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
				force_extension(name, PGP_EXTENSION);
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
			if (!quietmode)
			{
				if (encrypt_flag)
					fprintf(pgpout, PSTR("\nCiphertext file: %s\n"), outputfile);
				else if (sign_flag)
					fprintf(pgpout, PSTR("\nSignature file: %s\n"), outputfile);
			}
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
	return(0);	/* to shut up lint and some compilers */
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
#ifndef __GNUC__ /* DJGPP's not MSC */
	_harderr(dostrap);
#endif
#endif
#endif
#endif /* MSDOS */
#ifdef SIGINT
#ifdef ATARI
	signal(SIGINT,(sigfunc_t) breakHandler);
#else
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT,breakHandler);
#if defined(UNIX) || defined(VMS)
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP,breakHandler);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT,breakHandler);
#ifdef UNIX
	signal(SIGPIPE,breakHandler);
#endif
	signal(SIGTERM,breakHandler);
#ifndef DEBUG
	signal(SIGTRAP,breakHandler);
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
	int success = 0;

	while (TRUE)
	{	/* Handle transport armor stripping */
		tempf = tempfile(0);
		strip_spaces = FALSE;	/* de_armor_file() sets this for clear signature files */
		status = de_armor_file(armorfile,tempf,&linepos);
		if (status)
		{	fprintf(pgpout,PSTR("\n\007Error: Transport armor stripping failed for file %s\n"),armorfile);
			errorLvl = INVALID_FILE_ERROR;
			user_error();	 /* Bad file */
		}
		if (keepctx || de_armor_only)
		{	if (outputfile && de_armor_only)
			{	if (strcmp(outputfile, "-") == 0)
				{	writePhantomOutput(tempf);
					rmtemp(tempf);
					return;
				}
				strcpy(cipherfile, outputfile);
			}
			else
			{	strcpy(cipherfile, file_tail(armorfile));
				force_extension(cipherfile, PGP_EXTENSION);
			}
			if ((tempf = savetemp(tempf, cipherfile)) == NULL)
			{	errorLvl = UNKNOWN_FILE_ERROR;
				user_error();
			}
			if (!quietmode)
				fprintf(pgpout,PSTR("Stripped transport armor from '%s', producing '%s'.\n"),
					armorfile, tempf);
			/* -da flag: don't decrypt */
			if (de_armor_only || do_decrypt(tempf) >= 0)
				++success;
		}
		else
		{	if (do_decrypt(tempf) >= 0)
				++success;
			rmtemp(tempf);
		}

		if (!is_armor_file(armorfile, linepos))
		{
			if (!success)	/* print error msg if we didn't decrypt anything */
				user_error();
			return;
		}

		fprintf (pgpout, PSTR("\nLooking for next packet in '%s'...\n"), armorfile);
	}
}	/* do_armorfile */


static int do_decrypt(char *cipherfile)
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
			return -1;
		}

		if (!is_ctb(ctb))		/* not a real CTB -- complain */
			break;

		outfile = tempfile(TMP_WIPE);

		/* PKE is Public Key Encryption */
		if (is_ctb_type( ctb, CTB_PKE_TYPE ))
		{
			if (!quietmode)
				fprintf(pgpout,PSTR("\nFile is encrypted.  Secret key is required to read it. "));

			/* Decrypt to scratch file since we may have a LITERAL2 */
			status = decryptfile( cipherfile, outfile );

			if (status < 0) /* error return */
			{	errorLvl = RSA_DECR_ERROR;
				return -1;
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
				if (!quietmode)
					fprintf(pgpout,PSTR("\nThis file has a signature, which will be left in place.\n"));
				break;	/* Do no more */
			}
			if (!quietmode)
				fprintf(pgpout,PSTR("\nFile has signature.  Public key is required to check signature. "));

			status = check_signaturefile( cipherfile, outfile, strip_sig_flag, preserved_name );

			if (status < 0) /* error return */
			{	errorLvl = SIGNATURE_CHECK_ERROR;
				return -1;
			}
			nested_info = (status > 0);

			if (strcmp(preserved_name, "/dev/null") == 0)
			{	rmtemp(outfile);
				fprintf(pgpout, "\n");
				return 0;
			}
		}		/* outer CTB is SKE type */


		if (is_ctb_type( ctb, CTB_CKE_TYPE ))
		{		/* Conventional Key Encrypted ciphertext. */
			/* Tell user it's encrypted here, and prompt for password in subroutine. */
			if (!quietmode)
				fprintf(pgpout,PSTR("\nFile is conventionally encrypted.  "));
			/* Decrypt to scratch file since it may be a LITERAL2 */
			status = idea_decryptfile( cipherfile, outfile );
			if (status < 0) /* error return */
			{	errorLvl = DECR_ERROR;
				return -1;	/* error exit status */
			}
			nested_info = (status > 0);
		}		/* CTB is CKE type */


		if (is_ctb_type( ctb, CTB_COMPRESSED_TYPE ))
		{		/* Compressed text. */
			status = decompress_file( cipherfile, outfile );
			if (status < 0) /* error return */
			{	errorLvl = DECOMPRESS_ERROR;
				return -1;
			}
			/* Always assume nested information... */
			nested_info = TRUE;
		}		/* CTB is COMPRESSED type */


		if (is_ctb_type( ctb, CTB_LITERAL_TYPE ) || is_ctb_type( ctb, CTB_LITERAL2_TYPE))
		{		/* Raw plaintext.  Just copy it.  No more nesting. */
			/* Strip off CTB_LITERAL prefix byte from file: */
			/* strip_literal may alter plainfile; will set mode */
			status = strip_literal( cipherfile, outfile,
					preserved_name, &literal_mode);
			if (status < 0) /* error return */
			{	errorLvl = UNKNOWN_FILE_ERROR;
				return -1;
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
			if (view_keyring( NULL, cipherfile, TRUE, FALSE ) < 0)
			{	errorLvl = KEYRING_VIEW_ERROR;
				return -1;
			}
			/* filter mode explicit requested with -f */
			if (filter_mode && !preserve_filename)
				return 0; /*	No output file */
			if (batchmode)
				return 0;
			if (ctb == CTB_CERT_SECKEY)
				buildfilename(ringfile,SECRET_KEYRING_FILENAME);
			else
				buildfilename(ringfile,PUBLIC_KEYRING_FILENAME);
			/*	Ask if it should be put on key ring */
			fprintf(pgpout, PSTR("\nDo you want to add this keyfile to keyring '%s' (y/N)? "), ringfile);
			if (!getyesno('n'))
				return 0;
			status = addto_keyring(cipherfile,ringfile);
			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring add error. ") );
				errorLvl = KEYRING_ADD_ERROR;
				return -1;
			}
			return 0; /*	No output file */
		}		/* key ring.  view it. */

	} while (nested_info);
	/* No more nested parsable information */

	if (outfile == NULL)	/* file was not encrypted */
	{	if (!filter_mode && !moreflag)
		{	fprintf(pgpout,PSTR("\007\nError: '%s' is not a ciphertext, signature, or key file.\n"),
				cipherfile);
			errorLvl = UNKNOWN_FILE_ERROR;
			return -1;
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
			if (batchmode || !getyesno('y'))
			{	/* no -- abort display, and clean up */
				rmtemp(outfile);
				return 0;
			}
		}
		if (!quietmode)
			fprintf(pgpout, PSTR("\n\nPlaintext message follows...\n"));
		else
			putc('\n', pgpout);
		fprintf(pgpout, "------------------------------\n");
		more_file(outfile);
		/* Disallow saving to disk if outfile is console-only: */
		if (strcmp(preserved_name,CONSOLE_FILENAME) == 0)
			clearscreen();	/* remove all evidence */
		else if (!quietmode && !batchmode)
		{	
			fprintf(pgpout, PSTR("Save this file permanently (y/N)? "));
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
				return 0;
			}
		}
		rmtemp(outfile);
		return 0;
	}	/* blort to screen */

	if (outputfile)
	{	if (!strcmp(outputfile, "/dev/null"))
		{	rmtemp(outfile);
			return 0;
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
			return -1;
		}
		rmtemp(outfile);
		return 0;
	}

	if (preserve_filename && preserved_name[0] != '\0')
		strcpy(plainfile, file_tail(preserved_name));

	if (quietmode)
	{	if (savetemp(outfile, plainfile) == NULL)
		{	errorLvl = UNKNOWN_FILE_ERROR;
			return -1;
		}
		return 0;
	}
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
		newname = maybe_force_extension( plainfile, PGP_EXTENSION );
	}	/* Possible ciphertext output file */

	if (savetemp(outfile, (newname ? newname : plainfile)) == NULL)
	{	errorLvl = UNKNOWN_FILE_ERROR;
		return -1;
	}

	fprintf (pgpout, "\n");
	return 0;
} /* do_decrypt */


static int do_keyopt(char keychar)
{
	char keyfile[MAX_PATH];
	char ringfile[MAX_PATH];
	char *workfile;
	int status;

	if ((filter_mode || batchmode)
		&& (keychar == 'g' || keychar == 'e' || keychar == 'd'
			|| (keychar == 'r' && sign_flag)))
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
			}
			return status;
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

			status = dokeycheck( mcguffin, ringfile, CHECK_ALL );

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring check error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
			}
			if (status >= 0 && mcguffin[0] != '\0')
				return status;	/* just checking a single user, dont do maintenance */

			if ((status = maint_check(ringfile, 0)) < 0 && status != -7)
			{	fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
			}

			return (status == -7 ? 0 : status);
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

			if ((status = maint_check(ringfile,
					MAINT_VERBOSE|(c_flag ? MAINT_CHECK : 0))) < 0)
			{	if (status == -7)
					fprintf(pgpout, PSTR("File '%s' is not a public keyring\n"), ringfile);
				fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
				errorLvl = KEYRING_CHECK_ERROR;
			}
			return status;
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
				if (batchmode)	/* not interactive, userid must be on command line */
					return -1;
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
				status = maint_update(keyfile);
				if (status == -7)	/* ringfile is a keyfile or secret keyring */
				{	fprintf(pgpout, "Warning: '%s' is not a public keyring\n", keyfile);
					return 0;
				}
				if (status < 0)
					fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
			}

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Key signature error. ") );
				errorLvl = KEY_SIGNATURE_ERROR;
			}
			return status;
		}	/* Key signing */


		/*-------------------------------------------------------*/
		case 'd':
		{	/*	disable/revoke key
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
				fprintf(pgpout, PSTR("\nA user ID is required to select the key you want to revoke or disable. "));
				fprintf(pgpout, PSTR("\nEnter user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			status = disable_key ( mcguffin, keyfile );

			if (status >= 0) {
				status = maint_update(keyfile);
				if (status == -7)	/* ringfile is a keyfile or secret keyring */
				{	fprintf(pgpout, "Warning: '%s' is not a public keyring\n", keyfile);
					return 0;
				}
				if (status < 0)
					fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
			}

			if (status < 0)
				errorLvl = KEY_SIGNATURE_ERROR;
			return status;
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
				status = maint_update(ringfile);
				if (status == -7)
					status = 0;	/* ignore "not a public keyring" error */
				if (status < 0)
					fprintf(pgpout, PSTR("\007Maintenance pass error. ") );
			}

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring edit error. ") );
				errorLvl = KEYRING_EDIT_ERROR;
			}
			return status;
		}	/* Key edit */

		/*-------------------------------------------------------*/
		case 'a':
		{	/*	Add key to key ring
				Arguments: keyfile, ringfile
			*/

			if (myArgc < 3 && !filter_mode)
				arg_error();

			if (!filter_mode) {	/* Get the keyfile from args */
				strncpy( keyfile, myArgv[2], sizeof(keyfile)-1 );
				
#ifdef MSDOS
				strlwr( keyfile	 );
#endif
				if (! file_exists( keyfile ))
					default_extension( keyfile, PGP_EXTENSION );

				if (! file_exists( keyfile ))
				{	fprintf(pgpout, PSTR("\n\007Key file '%s' does not exist.\n"), keyfile );
					errorLvl = NONEXIST_KEY_ERROR;
					return -1;
				}

				workfile = keyfile;

			} else {
				workfile = tempfile(TMP_WIPE|TMP_TMPDIR);
				readPhantomInput(workfile);
			}

			if (myArgc < (filter_mode ? 3 : 4)) /* default key ring filename */
			{	byte ctb;
				get_header_info_from_file(workfile, &ctb, 1);
				if (ctb == CTB_CERT_SECKEY)
					buildfilename(ringfile,SECRET_KEYRING_FILENAME);
				else
					buildfilename(ringfile,PUBLIC_KEYRING_FILENAME);
			}
			else
			{	strncpy( ringfile, myArgv[(filter_mode ? 2 : 3)], sizeof(ringfile)-1 );
				default_extension( ringfile, PGP_EXTENSION );
			}
#ifdef MSDOS
			strlwr( ringfile );
#endif

			status = addto_keyring( workfile, ringfile);

			if (filter_mode)
				rmtemp(workfile);

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring add error. ") );
				errorLvl = KEYRING_ADD_ERROR;
			}
			return status;
		}	/* Add key to key ring */

		/*-------------------------------------------------------*/
		case 'x':
		{	/*	Extract key from key ring
				Arguments: mcguffin, keyfile, ringfile
			*/

			if (myArgc >= (filter_mode ? 4 : 5))	/* default key ring filename */
				strncpy( ringfile, myArgv[(filter_mode ? 3 : 4)], sizeof(ringfile)-1 );
			else
				buildfilename( ringfile, PUBLIC_KEYRING_FILENAME );

			if (myArgc >= (filter_mode ? 2 : 3))
			{	if (myArgv[2])
					/* Userid to extract */
					strcpy( mcguffin, myArgv[2] );	
				else
					strcpy( mcguffin, "" );
			}
			else
			{
				fprintf(pgpout, PSTR("\nA user ID is required to select the key you want to extract. "));
				if (batchmode)	/* not interactive, userid must be on command line */
					return -1;
				fprintf(pgpout, PSTR("\nEnter the key's user ID: "));
				getstring( mcguffin, 255, TRUE );	/* echo keyboard */
			}
			CONVERT_TO_CANONICAL_CHARSET(mcguffin);

			if (!filter_mode) {
				if (myArgc >= 4)
				{	strncpy( keyfile, myArgv[3], sizeof(keyfile)-1 );
				}
				else
					keyfile[0] = '\0';

				workfile = keyfile;
			} else {
				workfile = tempfile(TMP_WIPE|TMP_TMPDIR);
			}

#ifdef MSDOS
			strlwr( workfile );
			strlwr( ringfile );
#endif

			default_extension( ringfile, PGP_EXTENSION );

			status = extract_from_keyring( mcguffin, workfile,
					ringfile, (filter_mode ? FALSE :
						   emit_radix_64) );

			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring extract error. ") );
				errorLvl = KEYRING_EXTRACT_ERROR;
				if (filter_mode)
					rmtemp(workfile);
				return status;
			}


			if (filter_mode && !status) {
				if (emit_radix_64)
				{	/* NULL for outputfile means write to stdout */
					if (armor_file(workfile, NULL, NULL, NULL) != 0)
					{	errorLvl = UNKNOWN_FILE_ERROR;
						return -1;
					}
				}
				else
					if (writePhantomOutput(workfile) < 0)
					{	errorLvl = UNKNOWN_FILE_ERROR;
						return -1;
					}
				rmtemp(workfile);
			}

			return 0;
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
				if (batchmode)	/* not interactive, userid must be on command line */
					return -1;
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
					return -1;
				}
			}
			else		/* Remove keyring */
			{	if (remove_from_keyring( NULL, mcguffin, ringfile, (boolean) (myArgc < 4) ) < 0)
				{	fprintf(pgpout, PSTR("\007Keyring remove error. ") );
					errorLvl = KEYRING_REMOVE_ERROR;
					return -1;
				}
			}
			return 0;
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
			status = view_keyring(mcguffin, ringfile, (boolean) (keychar == 'V'), c_flag);
			if (status < 0)
			{	fprintf(pgpout, PSTR("\007Keyring view error. ") );
				errorLvl = KEYRING_VIEW_ERROR;
			}
			return status;
		}	/* view key ring entries, with userid match */

		default:
			arg_error();
	}
	return 0;
} /* do_keyopt */



void user_error() /* comes here if user made a boo-boo. */
{
	fprintf(pgpout,PSTR("\nFor a usage summary, type:  pgp -h\n"));
	fprintf(pgpout,PSTR("For more detailed help, consult the PGP User's Guide.\n"));
	exitPGP(errorLvl ? errorLvl : 127);		/* error exit */
}

#if defined(DEBUG) && defined(linux)
#include <malloc.h>
#endif
/*
 * exitPGP: wipes and removes temporary files, also tries to wipe
 * the stack.
 */
void exitPGP(int returnval)
{
	char buf[STACK_WIPE];
	struct hashedpw *hpw;

	if (verbose)
		fprintf(pgpout, "exitPGP: exitcode = %d\n", returnval);
	for (hpw = passwds; hpw; hpw = hpw->next)
		memset(hpw->hash, 0, sizeof(hpw->hash));
	for (hpw = keypasswds; hpw; hpw = hpw->next)
		memset(hpw->hash, 0, sizeof(hpw->hash));
	cleanup_tmpf();
#if defined(DEBUG) && defined(linux)
	if (verbose)
	{	struct mstats mstat;
		mstat = mstats();
		printf("%d chunks used (%d bytes)  %d bytes total\n",
			mstat.chunks_used, mstat.bytes_used, mstat.bytes_total);
	}
#endif
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
		{	rmtemp(tmphelp);
			tmphelp = helpfile;
		}
		CONVERSION = NO_CONV;
	}

	  /* built-in help if pgp.hlp is not available */
	if (more_file(tmphelp) < 0)
		fprintf(pgpout,PSTR("\nUsage summary:\
\nTo encrypt a plaintext file with recipent's public key, type:\
\n   pgp -e textfile her_userid [other userids] (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key:\
\n   pgp -s textfile [-u your_userid]           (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key, and then encrypt it\
\n   with recipent's public key, producing a .pgp file:\
\n   pgp -es textfile her_userid [other userids] [-u your_userid]\
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
	else	/* only use built-in help if there is no helpfile */
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
