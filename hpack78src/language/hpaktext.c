/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						  		  HPACK Messages 							*
*							HPAKTEXT.C  Updated 29/05/92					*
*																			*
*   This program is protected by copyright and as such if you use or copy	*
*   this code for your own purposes directly or indirectly your soul will	*
*   become the property of the author with the right to dispose of it as	*
* 									he wishes.								*
*																			*
*		Copyright 1991 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

/* Generated:		04 Dec 1992.
   From file:		HPAKTEXT.DEF
   Language:		English.
   Character set:	7-bit ASCII */

/* The following are all the text messages (except for the hidden ones)
   contained in HPACK.  The error messages are stored in a special struct
   which also contains the error exit level.  Note that the messages are
   all stored as seperate strings instead of in one unified array to make
   automatic generation by a (currently experimental) preprocessing tool
   easier */

#include <stdlib.h>
#include "defs.h"
#include "error.h"
#include "errorlvl.h"
#include "hpacklib.h"

/****************************************************************************
*																			*
*									Errors									*
*																			*
****************************************************************************/

/* When changing the following remember to change the defines in ERROR.H
   as well */

#define ERR			ERROR_INFO

			/* Severe errors */
#if defined( __ARC__ )
ERR err00 = { EXIT_INT_ERR,		"Abort on data transfer" };
#else
ERR err00 = { EXIT_INT_ERR,		"Internal error" };
#endif /* __ARC__ */
ERR err01 = { EXIT_NO_MEM,		"Out of memory" };
ERR err02 = { EXIT_NO_DISK,		"Out of disk space" };
ERR err03 = { EXIT_NO_ARCH,		"Cannot open archive file %s" };
ERR err04 = { EXIT_NO_TEMP,		"Cannot open temp file" };
ERR err05 = { EXIT_NO_PATH,		"Path %s not found" };
ERR err06 = { EXIT_NO_BASE,		"Cannot access base directory" };
ERR err07 = { EXIT_NO_MKDIR,	"Cannot create directory %s" };
ERR err08 = { EXIT_BREAK,		"Stopped at user request" };
#if defined( __MSDOS__ )
ERR err09 = { EXIT_FILE_ERR,	"File error type %02d" };
#else
ERR err09 = { EXIT_FILE_ERR,	"File error" };
#endif /* __MSDOS__ */
ERR err10 = { EXIT_DIR_CORRUPT,	"Archive directory corrupted" };

			/* Standard errors */
ERR err11 = { EXIT_LONG_PATH,	"Path %s too long" };
ERR err12 = { EXIT_LONG_PATH,	"Path %s... too long" };
ERR err13 = { EXIT_LONG_PATH,	"Path %s%s too long" };
ERR err14 = { EXIT_LONG_PATH,	"Path %s%s... too long" };
ERR err15 = { EXIT_LONG_PATH,	"Path %s/%s too long" };
ERR err16 = { EXIT_NO_OVERRIDE,	"Cannot override base path" };
ERR err17 = { EXIT_NEST,		"Too many levels of directory nesting" };
ERR err18 = { EXIT_SCRIPT_ERR,	"%s%d errors detected in script file" };
ERR err19 = { EXIT_NOT_ARCH,	"Not an HPACK archive" };
ERR err20 = { EXIT_NOTHING,		"No matching files on disk" };
ERR err21 = { EXIT_NOTHING,		"No matching files in archive" };
ERR err22 = { EXIT_NOTHING,		"No matching archives found" };
ERR err23 = { EXIT_BAD_KEYFILE,	"Bad keyfile" };
ERR err24 = { EXIT_COMMAND,		"Unknown command \'%s\'" };
ERR err25 = { EXIT_OPTION,		"Unknown directory option \'%c\'" };
ERR err26 = { EXIT_OPTION,		"Unknown overwrite option \'%c\'" };
ERR err27 = { EXIT_OPTION,		"Unknown view option \'%c\'" };
ERR err28 = { EXIT_OPTION,		"Unknown option \'%c\'" };

			/* Minor errors */
ERR err29 = { EXIT_PASS,		"Missing userID for encryption/authentication" };
ERR err30 = { EXIT_PASS,		"Cannot find secret key for userID \'%s\'" };
ERR err31 = { EXIT_PASS,		"Cannot find secret key - cannot decrypt data" };
ERR err32 = { EXIT_PASS,		"Cannot find public key for userID \'%s\'" };
ERR err33 = { EXIT_PASS,		"Cannot read random seed file" };
ERR err34 = { EXIT_PASS,		"Passwords not the same" };
ERR err35 = { EXIT_CHANGE,		"Cannot change deleted archive" };
ERR err36 = { EXIT_CHANGE,		"Cannot change multipart archive" };
ERR err37 = { EXIT_CHANGE,		"Cannot change encrypted archive" };
ERR err38 = { EXIT_CHANGE,		"Cannot change unencrypted archive" };
ERR err39 = { EXIT_CHANGE,		"Cannot change unit-compressed archive" };
ERR err40 = { EXIT_NOLONG,		"Long argument format not supported in this version" };
ERR err41 = { EXIT_BADWILD,		"Bad wildcard format in %s" };
ERR err42 = { EXIT_BADWILD,		"Wildcard expression too complex" };
ERR err43 = { EXIT_BADWILD,		"Cannot use wildcards in external pathname %s" };
ERR err44 = { EXIT_SECURITY,	"Cannot use both conventional and public-key encryption" };
ERR err45 = { EXIT_NOCRYPT,		"Cannot process encrypted archive" };

/****************************************************************************
*																			*
*									Warnings								*
*																			*
****************************************************************************/

char warn00[] =	"Warning: Truncated %u bytes of EOF padding\n";
char warn01[] =	"Warning: %s. Continue [y/n] ";
char warn02[] =	"\nWarning: File may be corrupt";
char warn03[] =	"Warning: Cannot find public key - cannot perform authentication check\n";
char warn04[] =	"Warning: Authentication data corrupted - cannot perform authentication check\n";
char warn05[] =	"\nWarning: Archive section too short, skipping...";
char warn06[] =	"Warning: Unknown script command \'%s\'\n";
char warn07[] =	"Warning: Cannot open data file %s - skipping\n";
char warn08[] =	"Warning: Cannot open script file %s - skipping\n";
char warn09[] =	"Warning: Directory %s conflicts with\n\t existing file - skipping\n";
char warn10[] =	"\nWarning: One file was corrupt";
char warn11[] =	"\nWarning: %2d files were corrupt\n";

/****************************************************************************
*																			*
*								General Messages							*
*																			*
****************************************************************************/

char mesg00[] =	"Verify multipart archive authenticity";
char mesg01[] =	"Archive fails authenticity check";
char mesg02[] =	"Verifying archive authenticity - please wait\n";
char mesg03[] =	"Authentication information will be destroyed";
char mesg04[] =	"Archive directory corrupted";
char mesg05[] =	"Archive directory corrupted, probably due to\n\t incorrect password";
char mesg06[] =	"Processing archive directory...";
char mesg07[] =	"%s [y/n] ";
char mesg08[] =	"%s %s [y/n/a] ";
char mesg09[] =	"Skipping %s\n";
char mesg10[] =	"Data is encrypted";
char mesg11[] =	"Cannot process encryption information";
char mesg12[] =	"Cannot open data file";
char mesg13[] =	"Extracting";
char mesg14[] =	" %s as";
char mesg15[] =	" as %s";
char mesg16[] =	"Unknown archiving method";
char mesg17[] =	" - skipping %s";
char mesg18[] =	"Won't overwrite existing file %s\n";
char mesg19[] =	"%s already exists: Enter new filename ";
char mesg20[] =	"Path %s... too long\n";
char mesg21[] =	"already exists - overwrite";
char mesg22[] =	"Extract";
char mesg23[] =	"Display";
char mesg24[] =	"Test";
char mesg25[] =	"\n\tFile tested OK";
char mesg26[] =	"\nHit a key...";
char mesg27[] =	"Adding";
char mesg28[] =	"\nError: ";
char mesg29[] =	"Panic: Error during error recovery";
char mesg30[] =	"Creating directory %s";
char mesg31[] =	"\nProcessing script file %s\n";
char mesg32[] =	"Path %s... too long in line %d\n";
char mesg33[] =	"^ Bad character in filename in line %d\n";
char mesg34[] =	"Maximum level of ";
char mesg35[] =	"Adding directory %s\n";
char mesg36[] =	"Checking directory %s\n";
char mesg37[] =	"Leaving directory %s\n";
char mesg38[] =	"File %s already in archive - skipping\n";
char mesg39[] =	"Add";
char mesg40[] =	"Delete";
char mesg41[] =	"Deleting %s from archive\n";
char mesg42[] =	"Freshen";
char mesg43[] =	"Replace";
char mesg44[] =	"\nArchive is \'%s\'\n\n";
char mesg45[] =	"Archive is uptodate";
char mesg46[] =	"\nDone";
char mesg47[] =	"\nDirectory: ";
char mesg48[] =	"\n\tCreated %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg49[] =	"file";
char mesg50[] =	"files";
char mesg51[] =	"\rPassword must be between 8 and 80 characters long";
char mesg52[] =	"Please enter password (8..80 characters): ";
char mesg53[] =	"Please enter secondary password (8..80 characters): ";
char mesg54[] =	"Reenter password to confirm: ";
char mesg55[] =	"Please enter password for secret key (8..80 characters): ";
char mesg56[] =	"Password is incorrect.";
char mesg57[] =	"Warning: Data fails authenticity check!\nBad";
char mesg58[] =	"Good";
char mesg59[] =	" signature from: %s.\n\tSignature made on %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg60[] =	" - One moment...";
char mesg61[] =	"This is part %u of a multipart archive\n";
char mesg62[] =	"Please insert the ";
char mesg63[] =	"next disk";
char mesg64[] =	"previous disk";
char mesg65[] =	"disk containing ";
char mesg66[] =	"part %u";
char mesg67[] =	"the last part";
char mesg68[] =	" of this archive";
char mesg69[] =	" and press a key";
char mesg70[] =	"Continuing... ";

/****************************************************************************
*																			*
*					Special Text Strings used in Viewfile.C					*
*																			*
****************************************************************************/

/* The following messages *must* have the various columns line up since they
   are used to display archive directory listings:

   System  Length   Size  Ratio   Date     Time    Name
   ------  ------  ------ -----   ----     ----    ----
    Subdirectory                dd/mm/yy hh:mm:ss  sssssssssss...
	sssss ddddddd ddddddd  dd%  dd/mm/yy hh:mm:ss #sssssssssss...
		  ------- ------- -----					   ----
		  ddddddd ddddddd  dd%				       Total: dddd files

   Grand Total:
		  ddddddd ddddddd			         dd archives, dddd files */

char vmsg00[] =	"System  Length   Size  Ratio   Date     Time    Name\n" \
				"------  ------  ------ -----   ----     ----    ----";
char vmsg01[] =	" Subdirectory                %02d/%02d/%02d %02d:%02d:%02d  %s\n";
char vmsg02[] =	" %s %7lu %7lu  %2d%%  %02d/%02d/%02d %02d:%02d:%02d %c%s\n";
char vmsg03[] =	"       ------- ------- -----                    ----\n" \
				"       %7lu %7lu  %2d%%                     Total: %u %s\n";
char vmsg04[] =	"\nGrand Total:\n" \
				"       %7lu %7lu                    %2d archives, %u %s\n";

/****************************************************************************
*																			*
*								OS-Specific Messages						*
*																			*
****************************************************************************/

#if defined( __MSDOS__ )
char osmsg00[] = "File sharing violation";
char osmsg01[] = "File locking violation";
char osmsg02[] = "File corresponds to device driver";
#endif /* __MSDOS__ */

/****************************************************************************
*																			*
*						Display Various (Long) Messages						*
*																			*
****************************************************************************/

#ifndef __MSDOS__

void showTitle( void )
	{
	hputs( "HPACK - The multi-system archiver Version 0.78a0.  Release date: 1 Sept 1992" );
	hputs( "For Amiga, Archimedes, Atari, Macintosh, MSDOS, OS/2, and UNIX" );
#if defined( __AMIGA__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Amiga port by N.Little & P.Gutmann." );
#elif defined( __ARC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Arc port by Jason, Edouard, & Peter." );
#elif defined( __ATARI__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Atari port by M.Moffatt and P.Gutmann." );
#elif defined( __MAC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Macintosh port by Peter Gutmann." );
#elif defined( __OS2__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  OS/2 port by John Burnell." );
#elif defined( __UNIX__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Unix port by Stuart Woolford." );
#else
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  xxxx port by yyyy zzzz." );
#endif /* Various OS-dependant credits */
	}

void showHelp( void )
	{
	hputs( "\nUsage: HPACK command [options] archive [data files...][@scriptfiles... ]" );
	hputs( "\nValid HPACK commands are:" );
	hputs( " A - Add files          X - Extract files       V - Directory of files" );
	hputs( " P - View files         D - Delete files        F - Freshen files in archive" );
	hputs( " R - Replace files      U - Update files        T - Test archive integrity" );
	hputs( "\nValid HPACK options are:" );
	hputs( " -K - Overwrite existing archive       -T - Touch files on extraction" );
	hputs( " -R - Recurse through subdirectories   -D - Store directories recursed into" );
	hputs( " -S - Stealth mode                     -A - Store/restore file attributes" );
	hputs( " -I - Interactive mode                 -L - Add authentication information" );
	hputs( " -W - Add archive comment              -E - Add error recovery information" );
	hputs( " -U - Unit compression mode            -C<*> - Encryption opts: see HPACK.DOC" );
	hputs( " -B<path>  - Specify base path         -D<*> - Directory options: see HPACK.DOC" );
	hputs( " -V<F,D,S> - View opt(Files,Dirs,Sort) -Z<*> - Sys-specific opts: see HPACK.DOC" );
	hputs( " -O<N,A,S,P>       - Overwrite options (None, All, Smart, Prompt)" );
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	hputs( " -X<R,Xxx,L,E,P,A> - Xlate opts (smart, CR, Hex lit, LF, EBCDIC, Prime, ASCII)" );
#elif defined( __MAC__ )
	hputs( " -X<C,Xxx,L,E,P,A> - Xlate opts (smart, CRLF, Hex lit, LF,EBCDIC,Prime,ASCII)" );
#elif defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __UNIX__ )
	hputs( " -X<R,Xxx,C,E,P,A> - Xlate opts (smart, CR, Hex lit, CRLF, EBCDIC,Prime,ASCII)" );
#endif /* Various OS-specific defines */
	hputs( "\n\"archive\" is the HPACK archive, with a default extension of .HPK" );
	hputs( "\"data files\" are the files to archive, defaulting to all files if none are" );
	hputs( "             given.  Wildcards *, ?, [], ^/!, \\ can be used." );
	exit( OK );
	}

#endif /* !__MSDOS__ */
