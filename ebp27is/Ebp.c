/* #define TEMP_VERSION /* if defined, temporary experimental
   		            version of EBP/PGP */
/* ebp.c -- main module for EBP/PGP.
  
   EBP(c) - Even Better Privacy - is the new generation of public key
   cryptography. EBP is based on the code to PGP 2.6.3(i) and was developed
   by Niklas Jarl (LTH-LU, ISS-NUS), Jonas Loefgren (LTH-LU, ISS-NUS) and 
   Dr. Yongfei Han (ISS-NUS). The main purpose of this release is to let
   PGP users and other people that may need strong cryptography try the new
   algorithms and give them the option to choose the algorithms they trust
   the most. 
   
   LTH - Lunds Tekniska Hoegskola - Lund Institute of Technology, LU, Sweden.
   LU  - Lund University, Sweden.
   ISS - Institute of Systems Science, NUS, Singapore.
   NUS - National University of Singapore.

   WARNING! This program is for non-commercial use only! It may not be used
   commercially in any way. To do so you need a written permission not only
   from the authors of EBP, but also the authors and patentees of the various 
   algorithms. However, look out for the coming program called 
   "PC Security and Privacy(c)", which will be a fully commercial 
   cryptographic program with even more choice of algorithms. 
   
   So what's the difference between EBP and PGP? Well, for starters we have
   the new algorithms: Rabin's public key scheme (which has been proved to
   be as hard to crack as to factorize the public key), the SAFER blockcipher,
   and HAVAL hashing-function with all its possibilities to choose length of 
   the message digest and the number of passes used when hashing. Of course
   all the old algorithms have been left intact, which gives the user more
   options.

   We have also made a few improvements of the key generation and the
   mp_modexp function. The latter has been changed to count consecutive
   ones and thus be faster or at least as fast as the old mp_modexp.

   The most important improvement of the key generation is that some of
   the Fermat-tests have been replaced by Rabin-Miller strong probable 
   primality (SPRP) tests. Since Rabin-Miller is slower than Fermat when 
   the suspected prime isn't a prime, we only use the Rabin-Miller test 
   when a suspect has passed one Fermat test.

  Key Generation ++:
   By using the command ebp -kp you can choose 
   1) To have less input for your random key generation (faster and easier,
      but not advisable).
   2) To use 8 Rabin-Miller tests instead of 3 after the suspect has passed one
      Fermat test.
   3) To use the Goodprime function found in PGP.

  New commands:
  ebp -j : Algorithm help-menu.
  ebp -ja: Use Rabin's scheme for encryption of the session key.
  ebp -jb: Use RSA for encryption of the session key.
  ebp -jc: Use Rabin's scheme for signing.
  ebp -jd: Use RSA for signing.
  ebp -je: Use SAFER for conventional encryption.
  ebp -jf: Use IDEA for conventional encryption.
  ebp -jg: Use HAVAL for hashing.
  ebp -jh: Use MD5 for hashing.
  ebp -ji: Choose length of the HAVAL message digest.
  ebp -jj: Choose number of passes for HAVAL.
  ebp -jk: Set the default PGP algorithms.
  ebp -jl: Set the default EBP algorithms.

  ebp -ki: Import old PGP key to you EBP key ring.
  ebp -kp: Key Generation ++.
  ebp -kxx: Extract a public EBP key so that PGP users can add it to their
			public key rings.

  If you want to get in contact with us, please send an email or a letter to:

		Niklas Jarl				Jonas Lofgren
		Boklins v. 22				Nils Kaggsg. 1C
		S-291 77 Gards Kopinge			S-254 54 Helsingborg
		Sweden					Sweden

  Now to the old PGP information:

   PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

   Synopsis:  PGP uses public-key encryption to protect E-mail. 
   Communicate securely with people you've never met, with no secure
   channels needed for prior exchange of keys. PGP is well featured and
   fast, with sophisticated key management, digital signatures, data
   compression, and good ergonomic design.

   The original PGP version 1.0 was written by Philip Zimmermann, of
   Phil's Pretty Good(tm) Software.  Many parts of later versions of 
   PGP were developed by an international collaborative effort, 
   involving a number of contributors, including major efforts by:
   Branko Lankester <branko@hacktic.nl>
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


   (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
   The author assumes no liability for damages resulting from the use
   of this software, even if the damage results from defects in this
   software.  No warranty is expressed or implied.

   Note that while most PGP source modules bear Philip Zimmermann's
   copyright notice, many of them have been revised or entirely written
   by contributors who frequently failed to put their names in their
   code.  Code that has been incorporated into PGP from other authors
   was either originally published in the public domain or is used with
   permission from the various authors.

   PGP is available for free to the public under certain restrictions.
   See the PGP User's Guide (included in the release package) for
   important information about licensing, patent restrictions on
   certain algorithms, trademarks, copyrights, and export controls.


   Philip Zimmermann may be reached at:
   Boulder Software Engineering
   3021 Eleventh Street
   Boulder, Colorado 80304  USA
   (303) 541-0140  (voice or FAX)
   email:  prz@acm.org


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
   Uses the ETH IDEA algorithm for conventional encryption.

   EBP/PGP generally zeroes its used stack and memory areas before exiting.
   This avoids leaving sensitive information in RAM where other users
   could find it later.  The RSA library and keygen routines also
   sanitize their own stack areas.  This stack sanitizing has not been
   checked out under all the error exit conditions, when routines exit
   abnormally.  Also, we must find a way to clear the C I/O library
   file buffers, the disk buffers, and cache buffers.

   Revisions:
   Version 1.0 -  5 Jun 91
   Version 1.4 - 19 Jan 92
   Version 1.5 - 12 Feb 92
   Version 1.6 - 24 Feb 92
   Version 1.7 - 29 Mar 92
   Version 1.8 - 23 May 92
   Version 2.0 -  2 Sep 92
   Version 2.1 -  6 Dec 92
   Version 2.2 -  6 Mar 93
   Version 2.3 - 13 Jun 93
   Version 2.3a-  1 Jul 93
   Version 2.4 -  6 Nov 93
   Version 2.5 -  5 May 94
   Version 2.6 - 22 May 94
   Version 2.6.1 - 29 Aug 94
   Version 2.6.2 - 11 Oct 94
   Version 2.6.2i - 7 May 95
   Version 2.6.3(i) - 18 Jan 96

  EBP - Even Better Privacy - was developed in 1997 by Niklas Jarl, Jonas Loefgren
  and Dr. Yongfei Han to give PGP users around the world an option of newer and
  hopefully better algorithms that aren't protected by any patents for the time
  being, at least not known to the authors. As default EBP uses SAFER instead of
  IDEA, HAVAL 256-bits and 5 rounds message digest instead of MD5 128-bits and 4 
  rounds, RABIN instead of RSA, and Rabin-Miller strong probable primality (SPRP) 
  test instead of some of the Fermattests.

  EBP is based on Phil Zimmermann's PGP (Stale Schumacher's version 2.6.3(i)),
  thus this version is denoted 2.7.

   EBP	
   Version 2.7 - Spring 1997

 */



#include <ctype.h>
#ifndef AMIGA
#include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Hack to get strncasecmp to work */
#if defined(WIN32)
#define strncasecmp _strnicmp
#elif defined(AMIGA) || defined(OS2) || defined(__PUREC__)
#define strncasecmp strnicmp
#elif defined(MSDOS)
#  if defined(__TURBOC__)
#  define strncasecmp strncmpi
#  else /* MSC */
#  define strncasecmp strnicmp
#  endif
#endif

#ifdef UNIX
#include <sys/stat.h>
#endif


#include "ebp.h"
#include "system.h"
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "language.h"
#include "haval.h"
#include "havalapp.h"
#include "exitebp.h"
#include "charset.h"
#include "getopt.h"
#include "config.h"
#include "keymaint.h"
#include "keyadd.h"
#include "rsaglue.h"
#include "noise.h"

#ifdef MACTC5
#include "Macutil.h"
#include "Macutil2.h"
#include "Macutil3.h"
#include "Macutil4.h"
#include "Macbinary.h"
#include "Binhex.h"
#include "MacPGP.h"
#include "mystr.h"
void AddOutputFiles(char *filepath);
void ReInitKeyMaint(void);
extern char appPathName[];
void 		ReInitGlobals(void);
extern int level,method;
extern Boolean explicit_plainfile;
extern long infile_line;
extern int eofonce;
extern boolean savedwashed;
extern char *special;
char *Outputfile = NULL;
void check_expiration_date(void);
#define BEST -1
#define strncasecmp strncmpi
#define exit Exit
void Exit(int x);
#endif

#ifdef  M_XENIX
char *strstr();
long time();
#endif

#ifdef MSDOS
#ifdef __ZTC__			/* Extend stack for Zortech C */
unsigned _stack_ = 24 * 1024;
#endif
#ifdef __TURBOC__
unsigned _stklen = 24 * 1024;
#endif
#endif
#define	STACK_WIPE	4096

#ifdef AMIGA
#ifdef __SASC_60
/* Let the compiler allocate us an appropriate stack. */
extern long __stack = 32768L;
#endif

 /* Add the appropriate AmigaOS version string, depending on the
  * compiler flags.
  */
#ifdef USA
static const char __DOSVer[] = "$VER: EBP 2.7 (12.04.97)"
#  ifdef _M68020
        " Amiga 68020 version by Rob Knop <rknop@mop.caltech.edu>";
#  else
        " Amiga 68000 version by Rob Knop <rknop@mop.caltech.edu>";
#  endif
#else
static const char __DOSVer[] = "$VER: EBP 2.7 (12.04.97)"
#  ifdef _M68020
        " Amiga 68020 version by Peter Simons <simons@peti.rhein.de>";
#  else
        " Amiga 68000 version by Peter Simons <simons@peti.rhein.de>";
#  endif
#endif /* USA */
#endif /* AMIGA */

/* Global filenames and system-wide file extensions... */
#ifdef USA
char rel_version[] = _LANG("2.7");	/* release version */
#else
char rel_version[] = _LANG("2.7");	/* release version */
#endif
char rel_date[] = "1997-04-12";		/* official release date */

char EBP_EXTENSION[] = ".ebp";
char PGP_EXTENSION[] = ".pgp";
char ASC_EXTENSION[] = ".asc";
char SIG_EXTENSION[] = ".sig";
char BAK_EXTENSION[] = ".bak";
static char HLP_EXTENSION[] = ".hlp";
char CONSOLE_FILENAME[] = "_CONSOLE";
#ifdef MACTC5
char HELP_FILENAME[256] = "ebp.hlp";
#else
static char HELP_FILENAME[] = "ebp.hlp";
#endif

/* These files use the environmental variable EBPPATH as a default path: */
char globalPubringName[MAX_PATH];
char globalSecringName[MAX_PATH];
char globalRandseedName[MAX_PATH];
char globalAlgorithmName[MAX_PATH];
char CountName[MAX_PATH]; /* testcount */
char globalCommentString[128];

/* Flags which are global across the driver code files */
boolean verbose = FALSE;	/* -l option: display maximum information */
FILE *ebpout;			/* Place for routine messages */

static void usage(void);
static void key_usage(void);
static void alg_usage(void);
static void arg_error(void);
static void initsigs(void);
static int do_keyopt(char);
static int do_decrypt(char *);
static void do_armorfile(char *);
char ** ParseRecipients(char **);
void hashpass (char *keystring, int keylen, byte *hash);

int jarl;
/* Various compression signatures: PKZIP, Zoo, GIF, Arj, and HPACK.
   Lha(rc) is handled specially in the code; it is missing from the
   compressSig structure intentionally.  If more formats are added,
   put them before lharc to keep the code consistent.

   27-Jun-95 simons@peti.rhein.de (Peter Simons)
   Added support for lh5 archive as generated by Lha. Unfortunately,
   Lha requires special treatment also. I inserted the check right
   _before_ lharc, because lh5/lha is a special type of an lharc
   archive.
 */
static char *compressSig[] =
{"PK\03\04", "ZOO ", "GIF8", "\352\140",
 "HPAK", "\037\213", "\037\235", "\032\013", "\032HP%"
	/* lharc is special, must be last */ };
static char *compressName[] =
{"PKZIP", "Zoo", "GIF", "Arj",
 "Hpack", "gzip", "compressed", "PAK", "Hyper",
 "Lha", "Lharc"};
static char *compressExt[] =
{".zip", ".zoo", ".gif", ".arj",
 ".hpk", ".gz", ".Z", ".pak", ".hyp",
 ".lha", ".lzh"};

/* "\032\0??", "ARC", ".arc" */

/* Returns file signature type from a number of popular compression formats
   or -1 if no match */
int compressSignature(byte * header)
{
    int i;

    for (i = 0; i < sizeof(compressSig) / sizeof(*compressSig); i++)
	if (!strncmp((char *) header, compressSig[i], strlen(compressSig[i])))
	    return i;

    /* Special check for lha files */
    if (!strncmp((char *)header+2, "-lh5-", 5))
      return i;

    /* Special check for lharc files */
    if (header[2] == '-' && header[3] == 'l' &&
	(header[4] == 'z' || header[4] == 'h') &&
	header[6] == '-')
	return i+1;
    return -1;
}				/* compressSignature */

/* returns TRUE if file is likely to be compressible */
static boolean file_compressible(char *filename)
{
    byte header[8];
    get_header_info_from_file(filename, header, 8);
    if (compressSignature(header) >= 0)
	return FALSE;		/* probably not compressible */
    return TRUE;		/* possibly compressible */
}				/* compressible */

void SetKeyAlg(struct keystruct *keyalgos)
{
	char pucklodromkamelen[5];

	printf("\n");
	printf("Do you wish to use less keystrokes? (y/N): ");
	getstring(pucklodromkamelen, 5, TRUE);
	if ((pucklodromkamelen[0] == 'Y') || (pucklodromkamelen[0] == 'y'))
		keyalgos->keystrokes = NEWALG;
	else
		keyalgos->keystrokes = OLDALG;
	
	printf("Do you wish to have extra prime tests? (y/N): ");
	getstring(pucklodromkamelen, 5, TRUE);
	if ((pucklodromkamelen[0] == 'Y') || (pucklodromkamelen[0] == 'y'))
		keyalgos->sprp = NEWALG;
	else
		keyalgos->sprp = OLDALG;
	
	printf("Do you wish to use Goodprime? (y/N): ");
	getstring(pucklodromkamelen, 5, TRUE);
	if ((pucklodromkamelen[0] == 'Y') || (pucklodromkamelen[0] == 'y'))
		keyalgos->goodprime = NEWALG;
	else
		keyalgos->goodprime = OLDALG;
	printf("\n");
}

/* To avoid adding a lot of extra bytes of information for the MD length
and the number of passes used when encrypting/signing we added the
GetHLandPnbr function to calculate a specific number for each and
every combination. The combinations are numbered 97 to 111. */

short GetHLandPnbr(struct algorithms *algostruct)
{
	short i;
	if (algostruct->Hashing == OLDALG)
		return OLDALG;
	if (algostruct->Passes == 5)
		i = 0;
	else if (algostruct->Passes == 4)
		i = 5;
	else
		i = 10;
	if (algostruct->MDlength == 256)
		return (97+i);
	else if (algostruct->MDlength == 224)
		return (98+i);
	else if (algostruct->MDlength == 192)
		return (99+i);
	else if (algostruct->MDlength == 160)
		return (100+i);
	else
		return (101+i);
}	/* GetHLandPnbr */

/* The SetLengthandPasses uses the HLandP number that has been saved in
the encrypted/signed packet and calculates the length and number of passes
that was used. */

void SetLengthandPasses(struct algorithms *algostruct, byte HLandP)
{
	short i;

	if ((96 < HLandP) && (HLandP < 102))
	{
		i = 0;
		algostruct->Passes = 5;
	}
	else if ((101 < HLandP) && (HLandP < 107))
	{
		i = 5;
		algostruct->Passes = 4;
	}
	else
	{
		i = 10;
		algostruct->Passes = 3;
	}

	if ((HLandP - i) == 97)
		algostruct->MDlength = 256;
	else if ((HLandP - i) == 98)
		algostruct->MDlength = 224;
	else if ((HLandP - i) == 99)
		algostruct->MDlength = 192;
	else if ((HLandP - i) == 100)
		algostruct->MDlength = 160;
	else
		algostruct->MDlength = 128;

}		/* SetLengthandPasses */



/* Set the default PGP algorithms
(i.e. RSA, IDEA, and MD5 with 128 bits message digest and 4 passes). */

int SetPGPalg(struct algorithms *algostruct, int notext)
{
		
		algostruct->PKencrypt = OLDALG;
		algostruct->PKsign = OLDALG;
		algostruct->Blockcipher = OLDALG;
		algostruct->Hashing = OLDALG;
		algostruct->MDlength = 128;
		algostruct->Passes = 4;

	if (notext == 0)
		printf("The default PGP algorithms have been set.\n");
	return 0;
}	/* SetPGPalg */

/* Set the default EBP algorithms
(i.e. Rabin, SAFER, and HAVAL with 256 bits message digest and 5 passes). */

int SetEBPalg(struct algorithms *algostruct, int notext)
{
		
		algostruct->PKencrypt = NEWALG;
		algostruct->PKsign = NEWALG;
		algostruct->Blockcipher = NEWALG;
		algostruct->Hashing = NEWALG;
		algostruct->MDlength = 256;
		algostruct->Passes = 5;

	if (notext == 0)
		printf("The default EBP algorithms have been set.\n");
	return 0;
}	/* SetEBPalg */

/* ChangeAlg is a function that sets the algorithms the user chooses.
You can choose public key encryption system (Rabin's scheme or RSA),
public key signing system (Rabin's scheme or RSA), blockcipher (SAFER
or IDEA), hashing function (HAVAL or MD5). When you use HAVAL you can
also choose the length of the message digest (128, 160, 192, 224, or
256 bits) and the number of passes (3, 4, or 5 passes). The new algorithms
are given the number of the year we created EBP (i.e. 97), while the old
algorithms are given the number of another year (cryptic, isn't it?). */

int ChangeAlg(FILE *algfil, char keychar, struct algorithms *algostruct)
{
	char pucklodromkamelen[5];

    if (filter_mode || batchmode)
	{
	arg_error();		/* interactive process, no go in batch mode */
    }
    /*
     * If we're not doing anything that uses stdout, produce output there,
     * in case user wants to redirect it.
     */
    if (!filter_mode)
	ebpout = stdout;

    switch (keychar) {

/*-------------------------------------------------------*/
    case 'a':
	{		/*      Rabin Public Key Encrypt		*/
		algostruct->PKencrypt = NEWALG;
		printf("Rabin's scheme chosen for public key encryption.\n");
		break;
	}

/*-------------------------------------------------------*/

    case 'b':
	{			/*     RSA Public Key Encrypt		 */
		algostruct->PKencrypt = OLDALG;
		printf("RSA chosen for public key encryption.\n");
		break;
	}
	   
/*-------------------------------------------------------*/


    case 'c':
	{			/*      Rabin signing		 */
		algostruct->PKsign = NEWALG;
		printf("Rabin's scheme chosen for public key signature.\n");
		break;
	}
	
/*-------------------------------------------------------*/
    
	case 'd':
	{			/*    RSA signing	 */
		algostruct->PKsign = OLDALG;
	    printf("RSA chosen for public key signature.\n");
		break;
	}
	   
/*-------------------------------------------------------*/
    
	case 'e':
	{			/*    SAFER Block-cipher	 */
		algostruct->Blockcipher = NEWALG;
		printf("SAFER chosen for conventional encryption.\n");
		break;
	}
	   
/*-------------------------------------------------------*/

    case 'f':
	{			/*    IDEA Block-cipher  	 */
		algostruct->Blockcipher = OLDALG;
		printf("IDEA chosen for conventional encryption.\n");
		break;
	}

/*-------------------------------------------------------*/

	case 'g':
	{			/*      HAVAL one-way hashing   	 */
		algostruct->Hashing = NEWALG;
		algostruct->MDlength = 256; 
		algostruct->Passes = 5;
		printf("HAVAL chosen for one-way hashing.\n");
    	printf("The length of the digest is %d and the number of passes is %d.\n", 
				algostruct->MDlength, algostruct->Passes);
		break;
	}

/*-------------------------------------------------------*/

	case 'h':
	{			/*    MD5 one-way hashing	 */
		algostruct->Hashing = OLDALG;
		algostruct->MDlength = 128;
		algostruct->Passes = 4;
		printf("MD5 chosen for one-way hashing.\n");
		break;
	}

/*-------------------------------------------------------*/
	
	case 'i':
	{	/*     Length of the message digest for HAVAL:
			      128, 160, 192, 224, or 256 bits.		*/
		if (algostruct->Hashing == OLDALG)
			printf("Sorry, MD5 can only use 128 bits.\n");
		else {
		printf("Please enter desired digest length,\n");
		printf("128, 160, 192, 224, or 256 bits: ");
		getstring(pucklodromkamelen, 5, TRUE);
		if (pucklodromkamelen[0] == '1')
		{
        	if (pucklodromkamelen[1] == '2')
				algostruct->MDlength = 128;
			if (pucklodromkamelen[1] == '6')
				algostruct->MDlength = 160;
			if (pucklodromkamelen[1] == '9')
				algostruct->MDlength = 192;
		}
		else
		{
		if (pucklodromkamelen[1] == '2')
			algostruct->MDlength = 224;
		else
			algostruct->MDlength = 256;
		}

#ifndef SPORE
		printf("OK! The length of the message digest has been set to %d bits.\n",
				algostruct->MDlength);
#else
		printf("OK l'ah! The length of the message digest has been set to %d bits.\n",
				algostruct->MDlength);
#endif
		}
		break;
	}
	
/*-------------------------------------------------------*/
    
	case 'j':
	{	/* Number of passes in HAVAL
				3, 4, or 5 passes.	*/
        if (algostruct->Hashing == OLDALG)
			printf("Sorry, MD5 can only use 4 passes.\n");
		else {
		printf("Please enter desired number of passes,\n");
		printf("3, 4, or 5 passes: ");
        getstring(pucklodromkamelen, 5, TRUE);
		if (pucklodromkamelen[0] == '3')
			algostruct->Passes = 3;
		else
		if (pucklodromkamelen[0] == '4')
			algostruct->Passes = 4;
		else
			algostruct->Passes = 5;
		
#ifndef SPORE
		printf("OK! The number of HAVAL passes has been set to %d.\n", 
				algostruct->Passes);
#else
		printf("OK l'ah! The number of HAVAL passes has been set to %d.\n", 
				algostruct->Passes);	
#endif
		}
		break;
	}

/*-------------------------------------------------------*/

  
	case 'k':
	{	/* Set PGP algorithms	*/
        SetPGPalg(algostruct, FALSE);
		break;
	}

/*-------------------------------------------------------*/
  
	case 'l':
	{	/* Set EBP algorithms	*/
        SetEBPalg(algostruct, FALSE);
		break;
	}

/*-------------------------------------------------------*/

    default:
	arg_error();
    }

fwrite(algostruct, sizeof(algostruct), 3, algfil);
rewind(algfil);

	printf("\n");
	printf("Your EBP is now using: \n");
	if (algostruct->PKencrypt == NEWALG)
		printf("Rabin's scheme for public key encryption.\n");
	else
		printf("RSA for public key encryption.\n");
	if (algostruct->PKsign == NEWALG)
		printf("Rabin's scheme for public key signature.\n");
	else
		printf("RSA for public key signature.\n");
	if (algostruct->Blockcipher == NEWALG)
		printf("SAFER for conventional encryption.\n");
	else
		printf("IDEA for conventional encryption.\n");
	if (algostruct->Hashing == NEWALG)
		printf("HAVAL for one-way hashing with %d bits length and %d passes.\n",
		algostruct->MDlength, algostruct->Passes);
	else
		printf("MD5 for one-way hashing.\n");
	printf("\n");
	return 0;
}	/* ChangeAlg */

/* If the algorithmfile can't be found then the default EBP algorithms
will be set by SetAlgorithms */

int SetAlgorithms(FILE *algfil, struct algorithms *algostruct)
{
		
		algostruct->PKencrypt = NEWALG;
		algostruct->PKsign = NEWALG;
		algostruct->Blockcipher = NEWALG;
		algostruct->Hashing = NEWALG;
		algostruct->MDlength = 256;
		algostruct->Passes = 5;
	
    printf("The default EBP algorithms have been set.\n");
	printf("\n");
	fwrite(algostruct, sizeof(algostruct), 3, algfil);
	rewind(algfil);
	return 0;
}	/* SetAlgorithms */

/* Due to a lack of time, we didn't save the algorithms in the keys, so
we have to try all possible alternatives when accessing a key if the
algorithms are different from when the key was generated. */

void TryAlg(int tries, struct algorithms *algostruct)
{
	if (tries == 1)
		SetEBPalg(algostruct, tries);		/* EBP default */
	else if (tries == 2)
		SetPGPalg(algostruct, tries);		/* PGP default */
	else if (tries == 3)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 86, 128, 4 */
	else if (tries == 4)
	{
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 256, 5 */
		algostruct->Hashing = NEWALG;
		algostruct->MDlength = 256;
		algostruct->Passes = 5;
	}
	else if (tries == 5)
		algostruct->Passes = 4;				/* 86, 86, 86, 97, 256, 4 */
	else if (tries == 6)
		algostruct->MDlength = 128;			/* 86, 86, 86, 97, 128, 4 */
	else if (tries == 7)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 128, 4 */
	else if (tries == 8)
		algostruct->MDlength = 256;			/* 86, 86, 97, 97, 256, 4 */
	else if (tries == 9)
		algostruct->MDlength = 224;			/* 86, 86, 97, 97, 224, 4 */
	else if (tries == 10)
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 224, 4 */
	else if (tries == 11)
		algostruct->MDlength = 192;			/* 86, 86, 86, 97, 192, 4 */
	else if (tries == 12)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 192, 4 */
	else if (tries == 13)
		algostruct->MDlength = 160;			/* 86, 86, 97, 97, 160, 4 */
	else if (tries == 14)
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 160, 4 */
	else if (tries == 15)
		algostruct->Passes = 5;				/* 86, 86, 86, 97, 160, 5 */
	else if (tries == 16)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 160, 5 */
	else if (tries == 17)
		algostruct->MDlength = 128;			/* 86, 86, 97, 97, 128, 5 */
	else if (tries == 18)
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 128, 5 */
	else if (tries == 19)
		algostruct->MDlength = 192;			/* 86, 86, 86, 97, 192, 5 */
	else if (tries == 20)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 192, 5 */
	else if (tries == 21)
		algostruct->MDlength = 224;			/* 86, 86, 97, 97, 224, 5 */
	else if (tries == 22)
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 224, 5 */
	else if (tries == 23)
		algostruct->Passes = 3;				/* 86, 86, 86, 97, 224, 3 */
	else if (tries == 24)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 224, 3 */
	else if (tries == 25)
		algostruct->MDlength = 256;			/* 86, 86, 97, 97, 256, 3 */
	else if (tries == 26)
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 256, 3 */
	else if (tries == 27)
		algostruct->MDlength = 128;			/* 86, 86, 86, 97, 128, 3 */
	else if (tries == 28)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 128, 3 */
	else if (tries == 29)
		algostruct->MDlength = 192;			/* 86, 86, 97, 97, 192, 3 */
	else if (tries == 30)
		algostruct->Blockcipher = OLDALG;	/* 86, 86, 86, 97, 192, 3 */
	else if (tries == 31)
		algostruct->MDlength = 160;			/* 86, 86, 86, 97, 160, 3 */
	else if (tries == 32)
		algostruct->Blockcipher = NEWALG;	/* 86, 86, 97, 97, 160, 3 */

}

/* Copystruct allows us to save the set algorithms for later use. */

void copystruct(struct algorithms *alg2, struct algorithms *alg1)
{
	alg2->PKencrypt = alg1->PKencrypt;
	alg2->PKsign = alg1->PKsign;
	alg2->Blockcipher = alg1->Blockcipher;
	alg2->Hashing = alg1->Hashing;
	alg2->MDlength = alg1->MDlength;
	alg2->Passes = alg1->Passes;
	
}


/* Possible error exit codes - not all of these are used.  Note that we
   don't use the ANSI EXIT_SUCCESS and EXIT_FAILURE.  To make things
   easier for compilers which don't support enum we use #defines */

#define EXIT_OK				0
#define INVALID_FILE_ERROR		1
#define FILE_NOT_FOUND_ERROR		2
#define UNKNOWN_FILE_ERROR		3
#define NO_BATCH			4
#define BAD_ARG_ERROR			5
#define INTERRUPT			6
#define OUT_OF_MEM			7

/* Keyring errors: Base value = 10 */
#define KEYGEN_ERROR			10
#define NONEXIST_KEY_ERROR		11
#define KEYRING_ADD_ERROR		12
#define KEYRING_EXTRACT_ERROR		13
#define KEYRING_EDIT_ERROR		14
#define KEYRING_VIEW_ERROR		15
#define KEYRING_REMOVE_ERROR		16
#define KEYRING_CHECK_ERROR		17
#define KEY_SIGNATURE_ERROR		18
#define KEYSIG_REMOVE_ERROR		19

/* Encode errors: Base value = 20 */
#define SIGNATURE_ERROR			20
#define RSA_ENCR_ERROR			21
#define ENCR_ERROR			22
#define COMPRESS_ERROR			23

/* Decode errors: Base value = 30 */
#define SIGNATURE_CHECK_ERROR		30
#define RSA_DECR_ERROR			31
#define DECR_ERROR			32
#define DECOMPRESS_ERROR		33


#ifdef SIGINT

/* This function is called if a BREAK signal is sent to the program.  In this
   case we zap the temporary files.
 */
void breakHandler(int sig)
{
#ifdef UNIX
    if (sig == SIGPIPE) {
	signal(SIGPIPE, SIG_IGN);
	exitEBP(INTERRUPT);
    }
    if (sig != SIGINT)
	fprintf(stderr, "\nreceived signal %d\n", sig);
    else
#endif
	fprintf(ebpout, LANG("\nStopped at user request\n"));
    exitEBP(INTERRUPT);
}
#endif

/* Clears screen and homes the cursor. */
static void clearscreen(void)
{
    fprintf(ebpout, "\n\033[0;0H\033[J\r           \r");  /* ANSI sequence. */
    fflush(ebpout);
}

/* We had to process the config file first to possibly select the 
   foreign language to translate the sign-on line that follows... */
static void signon_msg(void)
{    
    word32 tstamp;
    /* display message only once to allow calling multiple times */
    static boolean printed = FALSE;

    if (quietmode || printed)
	return;
    printed = TRUE;
	fprintf(stderr,
		LANG("Even Better Privacy %s. WARNING: For non-commercial use only!\n"),
	    rel_version);
    fprintf(stderr,
LANG("Based on Philip Zimmermann's Pretty Good Privacy(tm) - PGP.\n"),
	    stderr);
#ifdef TEMP_VERSION
    fputs(
"Internal development version only - not for general release.\n", stderr);
#endif
    fputs(LANG("EBP (c) 1997 Niklas Jarl, Jonas Loefgren & Dr. Yongfei Han.\n"),
    stderr);
/*	fputs(LANG("ISS - Institute of Systems Science, Singapore.\n"),
    stderr); */
/*  fputs(LANG("  PGP code (c) 1990-96 Philip Zimmermann, Phil's Pretty Good Software.\n"),
    stderr);
    fputs(LANG("SAFER code (c) 1995 Richard De Moliner, Swiss Federal Institute of Technology.\n"),
    stderr);
	fputs(LANG("HAVAL code (c) 1993-96 Yuliang Zheng, Monash University.\n"),
    stderr); *//* We had too much text on the screen already, but these are the 
	people who made it all possible. */
    fprintf(stderr, " %s\n",LANG(rel_date));
#ifdef USA
    fputs(LANG(signon_legalese), stderr);
#endif
    fputs(
#ifdef USA
LANG("Export of this software may be restricted by the U.S. government.\n"),
#else
LANG("International version - not for use in the USA.\n"),
#endif
	  stderr);

    get_timestamp((byte *) & tstamp);	/* timestamp points to tstamp */
    fprintf(ebpout, LANG("Current time: %s\n"), ctdate(&tstamp));
}


#ifdef TEMP_VERSION		/* temporary experimental version of PGP */
#include <time.h>
#define CREATION_DATE 0x30FE3640ul
				/* CREATION_DATE is
				   Thu Jan 18, 1996 1200 hours UTC */
#define LIFESPAN	((unsigned long) 60L * (unsigned long) 86400L)
				/* LIFESPAN is 60 days */

/* If this is an experimental version of PGP, cut its life short */
void check_expiration_date(void)
{
    if (get_timestamp(NULL) > (CREATION_DATE + LIFESPAN)) {
	fprintf(stderr,
		"\n\007This experimental version of PGP has expired.\n");
	exit(-1);		/* error exit */
    }
}				/* check_expiration_date */
#else				/* no expiration date */
#define check_expiration_date()	/* null statement */
#endif				/* TEMP_VERSION */

/* -f means act as a unix-style filter */
/* -i means internalize extended file attribute information, only supported
 *          between like (or very compatible) operating systems. */
/* -l means show longer more descriptive diagnostic messages */
/* -m means display plaintext output on screen, like unix "more" */
/* -d means decrypt only, leaving inner signature wrapping intact */
/* -t means treat as pure text and convert to canonical text format */

/* Used by getopt function... */
#define OPTIONS "abcdefghijklmo:prstu:vwxz:ABCDEFGHIJKLMO:PRSTU:VWX?"
extern int optind;
extern char *optarg;

#define INCLUDE_MARK "-@"
#define INCLUDE_MARK_LEN sizeof(INCLUDE_MARK)-1	/* skip the \0 */

boolean emit_radix_64 = FALSE;	/* set by config file */
static boolean sign_flag = FALSE;
boolean moreflag = FALSE;
boolean filter_mode = FALSE;
static boolean preserve_filename = TRUE;
static boolean decrypt_only_flag = FALSE;
static boolean de_armor_only = FALSE;
static boolean strip_sig_flag = FALSE;
boolean clear_signatures = TRUE;
boolean strip_spaces;
static boolean c_flag = FALSE;
static boolean u_flag = FALSE;		/* Did I get my_name from -u? */
boolean encrypt_to_self = FALSE; /* should I encrypt messages to myself? */
boolean sign_new_userids = TRUE;
boolean batchmode = FALSE;	/* if TRUE: don't ask questions */
boolean quietmode = FALSE;
boolean force_flag = FALSE;	/* overwrite existing file without asking */
#ifdef VMS			/* kludge for those stupid VMS variable-length
				   text records */
char literal_mode = MODE_TEXT;	/* MODE_TEXT or MODE_BINARY for literal
				   packet */
#else				/* not VMS */
char literal_mode = MODE_BINARY; /* MODE_TEXT or MODE_BINARY for literal
				    packet */
#endif				/* not VMS */
/* my_name is substring of default userid for secret key to make signatures */
char my_name[256] = "\0";	/* null my_name means take first userid
				   in ring */
boolean keepctx = FALSE;	/* TRUE means keep .ctx file on decrypt */
/* Ask for each key separately if it should be added to the keyring */
boolean interactive_add = FALSE;
boolean compress_enabled = TRUE; /* attempt compression before encryption */
long timeshift = 0L;		/* seconds from GMT timezone */
int version_byte = VERSION_BYTE_NEW;
boolean nomanual = 0;
/* If non-zero, initialize file to this many random bytes */
int makerandom = 0;


static char *outputfile = NULL;
#ifndef MACTC5
static int errorLvl = EXIT_OK;
#else
int errorLvl = EXIT_OK;
#endif
static char mcguffin[256];	/* userid search tag */
boolean signature_checked = FALSE;
int checksig_pass = 0;
boolean use_charset_header;
char charset_header[16] = "";
char plainfile[MAX_PATH];
int myArgc = 2;
char **myArgv;
struct hashedpw *passwds = 0, *keypasswds = 0;
static struct hashedpw **passwdstail = &passwds;

#ifdef MACTC5
extern unsigned long EBPStart, WNECalls;

void ReInitGlobals()
{
	int i;
    char scratch[64];
    WNECalls = 0;
    if (verbose)
    	EBPStart = TickCount();
    else
    	EBPStart = 0;
    Abort = FALSE;
	BreakCntl = 0;
	ebpout = stderr;
    optind = 1;
	errorLvl = EXIT_OK;
    myArgc = 2;
    myArgv = nil;
    emit_radix_64 = FALSE;		/* set by config file */
    sign_flag = FALSE;
    moreflag = FALSE;
    filter_mode = FALSE;
    preserve_filename = TRUE;
    decrypt_only_flag = FALSE;
    de_armor_only = FALSE;
    strip_sig_flag = FALSE;
    u_flag = FALSE;
	c_flag = FALSE;
    signature_checked = FALSE;
	literal_mode = MODE_BINARY;	/* MODE_TEXT or MODE_BINARY for literal packet */
    errorLvl = EXIT_OK;
    outputfile = Outputfile;
    method = BEST; 	/* one of BEST, DEFLATE (only), or STORE (only) */
    level = 9;		/* 0=fastest compression, 9=best compression */
    special = NULL;	/* List of special suffixes */
    infile_line = 0;
	eofonce = 0;
	savedwashed = FALSE;
	ReInitKeyMaint();
	settmpdir(nil);
	setoutdir(nil);
	makerandom = 0;
	if (xcli_opt[0]) {
    	if (argv[argc] == nil)
    		argv[argc] = malloc((size_t) 80);
		if (argv[argc] == nil) {
			BailoutAlert(LANG("Out of memory"));
			ExitToShell();
		}
		strcpy(argv[argc], xcli_opt);
		argc++;
		fprintf(ebpout, "         %s\n", xcli_opt);
	}
	for (i = 0; i <= 63; i++)
		scratch[i] = to_upper(xcli_opt[i]);
	if (strcmp(xcli_opt, "+NOMANUAL=ON")==0) nomanual = true;
	else nomanual = false;
 }

int init_ebp()
{
	int err=0;
	ebpout=stderr;
	/* Process the config file first.  Any command-line arguments will
	   override the config file settings */
	buildfilename( mcguffin, "config.txt");
	if ( processConfigFile( mcguffin ) < 0 )
		err=BAD_ARG_ERROR;
	init_charset();
	signon_msg();
	g_armor_flag=emit_radix_64;
	g_text_mode=(literal_mode == MODE_TEXT);
	g_clear_signatures=clear_signatures;
	EBPSetFinfo(globalRandseedName,'RSed','MEBP');
	set_precision(MAX_UNIT_PRECISION);
	return err;
}


void Exit(int x) {

	errorLvl = x;
	if (myArgv)
		free(myArgv);
	if (mcguffins)
		free(mcguffins);
	mac_cleanup_tmpf();
	longjmp(jmp_env,5);
}


int ebp_dispatch(int argc, char *argv[])
{
	int status, opt;
	char *inputfile = NULL;
	char **recipient = NULL;
/*	char **mcguffins;   zigf made global so we can free */
	boolean macbin_flag = FALSE;
#else

int main(int argc, char *argv[])
{
    int status, opt, countnumber;
    char *inputfile = NULL;
    char **recipient = NULL;
    char **mcguffins;
#endif /* MACTC5 */
    char *workfile, *tempf;
    boolean nestflag = FALSE;
    boolean decrypt_mode = FALSE;
    boolean wipeflag = FALSE;
    boolean armor_flag = FALSE;	/* -a option */
    boolean separate_signature = FALSE;
    boolean algorithmflag = FALSE;
	boolean keyflag = FALSE;
	boolean countflag = FALSE;
    boolean encrypt_flag = FALSE;
    boolean conventional_flag = FALSE;
    boolean attempt_compression; /* attempt compression before encryption */
    boolean output_stdout;	/* Output goes to stdout */
    char *clearfile = NULL;
    char *literal_file = NULL;
    char literal_file_name[MAX_PATH];
    char cipherfile[MAX_PATH];
    char keychar = '\0';
    char *p;
    byte ctb;
    struct hashedpw *hpw;
	FILE *algfil;
	FILE *countfil;

	/* testcount */

    /* Initial messages to stderr */
    ebpout = stderr;
	buildfilename(CountName, "count.ebp");
	if (!(countfil=fopen(CountName, "r+b")))
	{
		countfil = fopen(CountName, "w+b");
		countnumber = 1;	
	} else {
		fread(&countnumber, sizeof(countnumber), 1, countfil);
		rewind(countfil);
		countnumber++;
		if (countnumber == 1000) {
			printf("Even Better Privacy (c) - EBP \n\n");
			printf("This EBP has been used 1000 times! Since you seem to have \n");
			printf("a lot of secrets, we suggest you buy a copy of a commercial \n");
			printf("cryptographic program or at least update your freeware EBP. \n\n");
			printf("Niklas Jarl <e92nja@efd.lth.se> \n");
			printf("Jonas Loefgren <e92jl@efd.lth.se> \n\n");
			countnumber = 0;
			fwrite(&countnumber, sizeof(countnumber), 1, countfil);
			fclose(countfil);
			countflag = TRUE;
		}
	}
		
	fwrite(&countnumber, sizeof(countnumber), 1, countfil);
	fclose(countfil);

    buildfilename(globalAlgorithmName, "algoritm.ebp");
	if (!(algfil=fopen(globalAlgorithmName, "r+b")))
	{
		algfil = fopen(globalAlgorithmName, "w+b");
		SetAlgorithms(algfil, &algostruct);
	}
	else
	{
		fread(&algostruct, sizeof(algostruct), 3, algfil);
		rewind(algfil);
	}
	
	printf("Your EBP is currently using: \n");
	if (algostruct.PKencrypt == NEWALG)
		printf("Rabin's scheme for public key encryption.\n");
	else
		printf("RSA for public key encryption.\n");
	if (algostruct.PKsign == NEWALG)
		printf("Rabin's scheme for public key signature.\n");
	else
		printf("RSA for public key signature.\n");
	if (algostruct.Blockcipher == NEWALG)
		printf("SAFER for conventional encryption.\n");
	else
		printf("IDEA for conventional encryption.\n");
	if (algostruct.Hashing == NEWALG)
		printf("HAVAL for one-way hashing with %d bits length and %d passes.\n",
		algostruct.MDlength, algostruct.Passes);
	else
		printf("MD5 for one-way hashing.\n");
	printf("\n");


#ifdef MACTC5
	ReInitGlobals();
#endif
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

#ifndef MACTC5
    if ((p = getenv("EBPPASS")) != NULL) {
	hpw = xmalloc(sizeof(struct hashedpw));
if (algostruct.Hashing == NEWALG)
	haval_string(p, hpw->hash);
else
	hashpass(p, strlen(p), hpw->hash);

	/* Add to linked list of key passwords */
	hpw->next = keypasswds;
	keypasswds = hpw;
    }
    /* The -z "password" option should be used instead of EBPPASS if
     * the environment can be displayed with the ps command (eg. BSD).
     * If the system does not allow overwriting of the command line
     * argument list but if it has a "hidden" environment, EBPPASS
     * should be used.
     */
    for (opt = 1; opt < argc; ++opt) {
	p = argv[opt];
	if (p[0] != '-' || p[1] != 'z')
	    continue;
	/* Accept either "-zpassword" or "-z password" */
	p += 2;
	if (!*p)
	    p = argv[++opt];
	/* p now points to password */
	if (!p)
	    break;		/* End of arg list - ignore */
	hpw = xmalloc(sizeof(struct hashedpw));
if (algostruct.Hashing == NEWALG)
	haval_string(p, hpw->hash);
else
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
     * If EBPPASSFD is set in the environment try to read the password
     * from this file descriptor.  If you set EBPPASSFD to 0 ebp will
     * use the first line read from stdin as password.
     */
    if ((p = getenv("EBPPASSFD")) != NULL) {
	int passfd;
	if (*p && (passfd = atoi(p)) >= 0) {
	    char pwbuf[256];
	    p = pwbuf;
	    while (read(passfd, p, 1) == 1 && *p != '\n')
		++p;
	    hpw = xmalloc(sizeof(struct hashedpw));
    if (algostruct.Hashing == NEWALG)
        haval_string(pwbuf, hpw->hash);
    else
		hashpass(pwbuf, p - pwbuf, hpw->hash);
 
		memset(pwbuf, 0, p - pwbuf);
	    /* Add to tail of linked list of passwords */
	    hpw->next = 0;
	    *passwdstail = hpw;
	    passwdstail = &hpw->next;
	}
    }
    /* Process the config file.  The following override each other:
       - Hard-coded defaults
       - The system config file
       - Hard-coded defaults for security-critical things
       - The user's config file
       - Environment variables
       - Command-line options.
     */
    opt = 0;			/* Number of config files read */
#ifdef EBP_SYSTEM_DIR
    strcpy(mcguffin, EBP_SYSTEM_DIR);
    strcat(mcguffin, "config.txt");
    if (access(mcguffin, 0) == 0) {
	opt++;
	/*
	 * Note: errors here are NOT fatal, so that people
	 * can use EBP/PGP with a corrputed system file.
	 */
	processConfigFile(mcguffin);
    }
#endif

    /*
     * These must be personal; the system config file may not
     * influence them.
     */

    buildfilename(globalPubringName, "pubring.ebp");
    buildfilename(globalSecringName, "secring.ebp");
	buildfilename(globalRandseedName, "randseed.bin");
    my_name[0] = '\0';

    /* Process the config file first.  Any command-line arguments will
       override the config file settings */
#if defined(UNIX) || defined(MSDOS) || defined(OS2) || defined (WIN32)
    /* Try "ebp.ini" on MS-DOS or ".ebprc" on Unix */
#ifdef UNIX
#ifndef PGP
    buildfilename(mcguffin, ".ebprc");
#else
	buildfilename(mcguffin, ".pgprc");
#else
#endif
#else
    buildfilename(mcguffin, "ebp.ini");
#endif
    if (access(mcguffin, 0) != 0)
	buildfilename(mcguffin, "config.txt");
#else
    buildfilename(mcguffin, "config.txt");
#endif
    if (access(mcguffin, 0) == 0) {
	opt++;
	if (processConfigFile(mcguffin) < 0)
	    exit(BAD_ARG_ERROR);
    }
    if (!opt)
	fprintf(ebpout, LANG("\007No configuration file found.\n"));

    init_charset();
#endif /* MACTC5 */

#ifdef MSDOS			/* only on MSDOS systems */
    if ((p = getenv("TZ")) == NULL || *p == '\0') {
	fprintf(ebpout,LANG("\007WARNING: Environmental variable TZ is not \
defined, so GMT timestamps\n\
may be wrong.  See the EBP documentation to properly define TZ\n\
in AUTOEXEC.BAT file.\n"));
    }
#endif				/* MSDOS */

#ifdef VMS
#define TEMP "SYS$SCRATCH"
#else
#define TEMP "TMP"
#endif				/* VMS */
    if ((p = getenv(TEMP)) != NULL && *p != '\0')
	settmpdir(p);

    if ((myArgv = (char **) malloc((argc + 2) * sizeof(char **))) == NULL) {
	fprintf(stderr, LANG("\n\007Out of memory.\n"));
	exitEBP(7);
    }
    myArgv[0] = NULL;
    myArgv[1] = NULL;

    /* Process all the command-line option switches: */
    while (optind < argc) {
	/*
	 * Allow random order of options and arguments (like GNU getopt)
	 * NOTE: this does not work with GNU getopt, use getopt.c from
	 * the PGP distribution.
	 */
	if ((!strncasecmp(argv[optind], INCLUDE_MARK, INCLUDE_MARK_LEN)) ||
           ((opt = ebp_getopt(argc, argv, OPTIONS)) == EOF)) {
	    if (optind == argc)	/* -- at end */
		break;
	    myArgv[myArgc++] = argv[optind++];
	    continue;
	}
	opt = to_lower(opt);
	if ((keyflag||algorithmflag) && (keychar == '\0' || (keychar == 'v' && opt == 'v') || (keychar == 'x' && opt == 'x'))) {
	    if (keychar == 'v')
		keychar = 'V';
	    else if (keychar == 'x')
		keychar = 'X';
		else
		keychar = opt;
	    continue;
	}
	switch (opt) {
	case 'a':
	    armor_flag = TRUE;
	    emit_radix_64 = 1;
	    break;
	case 'b':
	    separate_signature = strip_sig_flag = TRUE;
	    break;
	case 'c':
	    encrypt_flag = conventional_flag = TRUE;
	    c_flag = TRUE;
	    break;
	case 'd':
	    decrypt_only_flag = TRUE;
	    break;
	case 'e':
	    encrypt_flag = TRUE;
	    break;
#ifdef MACTC5
	case 'f':
		if (macbin_flag == FALSE) filter_mode = TRUE;
		break;
#else
	case 'f':
	    filter_mode = TRUE;
	    break;
#endif
	case '?':
	case 'h':
	    usage();
	    break;
#ifdef VMS
	case 'i':
	    literal_mode = MODE_LOCAL;
	    break;
#else
#ifdef MACTC5
	case 'i':
		macbin_flag = TRUE;
		moreflag = FALSE;
		literal_mode = MODE_BINARY;
		filter_mode = FALSE;
		break;
#endif /* MACTC5 */
	case 'j':
		algorithmflag = TRUE;
		break;
#endif				/* VMS */
	case 'k':
	    keyflag = TRUE;
	    break;
	case 'l':
	    verbose = TRUE;
	    break;
#ifdef MACTC5
	case 'm':
		if( macbin_flag == FALSE )
			moreflag = TRUE;
		break;
#else
	case 'm':
	    moreflag = TRUE;
	    break;
#endif
	case 'p':
	    preserve_filename = FALSE;
	    break;
	case 'o':
	    outputfile = optarg;
	    break;
	case 's':
	    sign_flag = TRUE;
	    break;
#ifdef MACTC5
	case 't':
		if( macbin_flag == FALSE )
			literal_mode = MODE_TEXT;
		break;
#else
	case 't':
	    literal_mode = MODE_TEXT;
	    break;
#endif
	case 'u':
	    strncpy(my_name, optarg, sizeof(my_name) - 1);
	    CONVERT_TO_CANONICAL_CHARSET(my_name);
	    u_flag = TRUE;
	    break;
	case 'w':
	    wipeflag = TRUE;
	    break;
	case 'z':
	    break;
	    /* '+' special option: does not require - */
	case '+':
	    if (processConfigLine(optarg) == 0) {
                if (!strncasecmp(optarg,"charset",7))
                    init_charset();
		break;
	    }
	    fprintf(stderr, "\n");
	    /* fallthrough */
	default:
	    arg_error();
	}
    }
    myArgv[myArgc] = NULL;	/* Just to make it NULL terminated */

	if (countflag) 
		exitEBP(EXIT_OK);

	if (algorithmflag && keychar == '\0')
		alg_usage();

	if (algorithmflag) {
		status = ChangeAlg(algfil, keychar, &algostruct);
		fclose(algfil); 
		exitEBP(status);
	}

    if (keyflag && keychar == '\0')
		key_usage();

    signon_msg();
    check_expiration_date();	/* hobble any experimental version */

    /*
     * Write to stdout if explicitly asked to, or in filter mode and
     * no explicit file name was given.
     */
    output_stdout = outputfile ? strcmp(outputfile, "-")  == 0 : filter_mode;

#if 1
    /* At request of Peter Simons, use stderr always. Sounds reasonable. */
    /* JIS: Put this code back in... removing it broke too many things */
    if (!output_stdout)
	ebpout = stdout;
#endif


#if defined(UNIX) || defined(VMS)
    umask(077);			/* Make files default to private */
#endif

    initsigs();			/* Catch signals */
    noise();			/* Start random number generation */
    jarl = HASHBYTES;
    if (keyflag) {
	status = do_keyopt(keychar);
	if (status < 0)
	    user_error();
	exitEBP(status);
    }
    /* -db means break off signature certificate into separate file */
    if (decrypt_only_flag && strip_sig_flag)
	decrypt_only_flag = FALSE;

    if (decrypt_only_flag && armor_flag)
	decrypt_mode = de_armor_only = TRUE;

    if (outputfile != NULL)
	preserve_filename = FALSE;

    if (!sign_flag && !encrypt_flag && !conventional_flag && !armor_flag) {
	if (wipeflag) {		/* wipe only */
	    if (myArgc != 3)
		arg_error();	/* need one argument */
	    if (wipefile(myArgv[2]) == 0 && remove(myArgv[2]) == 0) {
		fprintf(ebpout,
			LANG("\nFile %s wiped and deleted. "), myArgv[2]);
		fprintf(ebpout, "\n");
		exitEBP(EXIT_OK);
	    } else if (file_exists(myArgv[2]))
	        fprintf(ebpout,
LANG("\n\007Error: Can't wipe out file '%s' - read only, maybe?\n"),
                        myArgv[2]);
            else {
	        fprintf(ebpout,
		        LANG("\n\007File '%s' does not exist.\n"), myArgv[2]);
	    }
	    exitEBP(UNKNOWN_FILE_ERROR);
	}
	/* decrypt if none of the -s -e -c -a -w options are specified */
	decrypt_mode = TRUE;
    }
    if (myArgc == 2) {		/* no arguments */
#ifdef UNIX
	if (!filter_mode && !isatty(fileno(stdin))) {
	    /* piping to ebp without arguments and no -f:
	     * switch to filter mode but don't write output to stdout
	     * if it's a tty, use the preserved filename */
	    if (!moreflag)
		ebpout = stderr;
	    filter_mode = TRUE;
	    if (isatty(fileno(stdout)) && !moreflag)
		preserve_filename = TRUE;
	}
#endif
	if (!filter_mode) {
	    if (quietmode) {
		quietmode = FALSE;
		signon_msg();
	    }
		fprintf(ebpout,
LANG("\nIf you have any problem with EBP 2.7 don't hesitate to contact\
\nNiklas Jarl   <e92nja@efd.lth.se> or\
\nJonas Loefgren <e92jl@efd.lth.se> or\
\nDr. Yongfei Han <yfhan@iss.nus.sg> or <yongfei@techasia.com>\
\nPlease note that this EBP version is for non-commercial use only.
\nIf you need a commercial crypto-program, please contact Dr. Han.\n"));


	   /* fprintf(ebpout,
LANG("\nFor details on licensing and distribution, see the PGP User's Guide.\
\nFor other cryptography products and custom development services, contact:\
\nPhilip Zimmermann, 3021 11th St, Boulder CO 80304 USA, \
phone +1 303 541-0140\n")); */
	    if (strcmp((p = LANG("@translator@")), "@translator@"))
		fprintf(ebpout, p);
	    fprintf(ebpout, LANG("\nFor a usage summary, type:  ebp -h\n"));
#ifdef MACTC5
		exitEBP(BAD_ARG_ERROR);
#else
	    exit(BAD_ARG_ERROR);	/* error exit */
#endif
	}
    } else {
	if (filter_mode) {
	    recipient = &myArgv[2];
	} else {
	    inputfile = myArgv[2];
	    recipient = &myArgv[3];
	}
	recipient = ParseRecipients(recipient);
    }


    if (filter_mode) {
	inputfile = "stdin";
    } else if (makerandom > 0) {	/* Create the input file */
	/*
	 * +makerandom=<bytes>: Create an input file consisting of <bytes>
	 * cryptographically strong random bytes, before applying the
	 * encryption options of PGP/EBP.  This is an advanced option, so
	 * assume the user knows what he's doing and don't bother about
	 * overwriting questions.  E.g.
	 * ebp +makerandom=24 foofile
	 *	Create "foofile" with 24 random bytes in it.
	 * ebp +makerandom=24 -ea foofile recipient
	 *	The same, but also encrypt it to "recipient", creating
	 *	foofile.asc as well.
	 * This feature was created to allow PGP to create and send keys
	 * around for other applications to use.
	 */
	status = cryptRandWriteFile(inputfile, (struct SaferCfbContext *)0,
	                       (unsigned)makerandom);
	if (status < 0) {
		fprintf(stderr,"Error writing file \"%s\"\n",inputfile);
		exitEBP(INVALID_FILE_ERROR);
	}
	fprintf(ebpout, LANG("File %s created containing %d random bytes.\n"),
		inputfile, makerandom);
	/* If we aren't encrypting, don't bother trying to decrypt this! */
	if (decrypt_mode)
		exitEBP(EXIT_OK);

	/* This is obviously NOT a text file */
	literal_mode = MODE_BINARY;
    } else {
	if (decrypt_mode && no_extension(inputfile)) {
	    strcpy(cipherfile, inputfile);
	    force_extension(cipherfile, ASC_EXTENSION);
	    if (file_exists(cipherfile))
		inputfile = cipherfile;
	    else {
		force_extension(cipherfile, EBP_EXTENSION);
		if (file_exists(cipherfile))
		    inputfile = cipherfile;
		else {
			force_extension(cipherfile, PGP_EXTENSION);
			if (file_exists(cipherfile)) 
				inputfile = cipherfile;
			else {
			force_extension(cipherfile, SIG_EXTENSION);
		    if (file_exists(cipherfile))
				inputfile = cipherfile;
			}
		}
		}
	}
	if (!file_exists(inputfile)) {
	    fprintf(ebpout,
		    LANG("\n\007File '%s' does not exist.\n"), inputfile);
	    errorLvl = FILE_NOT_FOUND_ERROR;
	    user_error();
	}
    }

    if (strlen(inputfile) >= (unsigned) MAX_PATH - 4) {
	fprintf(ebpout, 
		LANG("\007Invalid filename: '%s' too long\n"), inputfile);
	errorLvl = INVALID_FILE_ERROR;
	user_error();
    }
    strcpy(plainfile, inputfile);

    if (filter_mode) {
	setoutdir(NULL);	/* NULL means use tmpdir */
    } else {
	if (outputfile)
	    setoutdir(outputfile);
	else
	    setoutdir(inputfile);
    }

    if (filter_mode) {
	workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	readPhantomInput(workfile);
    } else {
	workfile = inputfile;
    }

    get_header_info_from_file(workfile, &ctb, 1);
    if (decrypt_mode) {
	strip_spaces = FALSE;
	if (!is_ctb(ctb) && is_armor_file(workfile, 0L))
	    do_armorfile(workfile);
	else if (do_decrypt(workfile) < 0)
	    user_error();
#ifdef MACTC5
	if (verbose) fprintf(stderr, "Final file = %s.\n", plainfile);
	/* Allow for overide of auto-unmacbin : 205b */
	if( (macbin_flag == FALSE) && is_macbin(plainfile) ) 
		bin2mac(plainfile,TRUE);
	else {
		AddOutputFiles(plainfile);
		EBPSetFinfo(plainfile,FType,FCreator);
	}
	if (use_clipboard) File2Scrap(plainfile);
#endif
	if (batchmode && !signature_checked)
	    exitEBP(1);		/* alternate success, file did not have sig. */
	else
	    exitEBP(EXIT_OK);
    }
    /*
     * See if plaintext input file was actually created by EBP earlier--
     * If it was, maybe we should NOT encapsulate it in a literal packet.
     * (nestflag = TRUE).  Otherwise, always encapsulate it (default).
     * (Why test for filter_mode???)
     */
    if (!batchmode && !filter_mode && legal_ctb(ctb)) {
	/*      Special case--may be a EBP-created packet, so
	   do we inhibit encapsulation in literal packet? */
	fprintf(ebpout,
LANG("\n\007Input file '%s' looks like it may have been created by EBP. "),
		inputfile);
	fprintf(ebpout,
LANG("\nIs it safe to assume that it was created by EBP (y/N)? "));
	nestflag = getyesno('n');
    } else if (force_flag && makerandom == 0 && legal_ctb(ctb)) {
	nestflag = TRUE;
    }

    if (moreflag && makerandom == 0) {
	/* special name to cause printout on decrypt */
	strcpy(literal_file_name, CONSOLE_FILENAME);
	literal_mode = MODE_TEXT;	/* will check for text file later */
    } else {
	strcpy(literal_file_name, file_tail(inputfile));
#ifdef MSDOS
	strlwr(literal_file_name);
#endif
    }
    literal_file = literal_file_name;

    /*      Make sure non-text files are not accidentally converted 
       to canonical text.  This precaution should only be followed 
       for US ASCII text files, since European text files may have 
       8-bit character codes and still be legitimate text files 
       suitable for conversion to canonical (CR/LF-terminated) 
       text format. */
    if (literal_mode == MODE_TEXT && !is_text_file(workfile)) {
	fprintf(ebpout,
LANG("\nNote: '%s' is not a pure text file.\n\
File will be treated as binary data.\n"),
		workfile);
	literal_mode = MODE_BINARY;	/* now expect straight binary */
    }
    if (moreflag && literal_mode == MODE_BINARY) {
	/* For eyes only?  Can't display binary file. */
	fprintf(ebpout,
LANG("\n\007Error: Only text files may be sent as display-only.\n"));
	errorLvl = INVALID_FILE_ERROR;
	user_error();
    }

    /*  
     * See if plainfile looks like it might be incompressible, 
     * by examining its contents for compression headers for 
     * commonly-used compressed file formats like PKZIP, etc.
     * Remember this information for later, when we are deciding
     * whether to attempt compression before encryption.
     *
     * Naturally, don't bother if we are making a separate signature or
     * clear-signed message.  Also, don't bother trying to compress a
     * EBP/PGP message, as it's probably already compressed.
     */
    attempt_compression = compress_enabled && !separate_signature &&
                          !nestflag && !clearfile && makerandom == 0 &&
                          file_compressible(plainfile);

#ifdef MACTC5
	if(( macbin_flag == TRUE ) && (nestflag==FALSE)) {
		char *saveworkfile;
		nestflag = false;
		saveworkfile = workfile;
		workfile = tempfile(TMP_WIPE|TMP_TMPDIR);
		if (mac2bin(saveworkfile, workfile)!=0) {
			fprintf(ebpout, LANG("\n\007Error: MacBinary failed!\n"));
			errorLvl = INVALID_FILE_ERROR;
			rmtemp(workfile);
			exitEBP(errorLvl);
		}
	}
#endif
    if (sign_flag) {
	if (!filter_mode && !quietmode)
	    fprintf(ebpout,
LANG("\nA secret key is required to make a signature. "));
	if (!quietmode && my_name[0] == '\0') {
	    fprintf(ebpout,
LANG("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
	}
	strip_spaces = FALSE;
	clearfile = NULL;
	if (literal_mode == MODE_TEXT) {
	    /* Text mode requires becoming canonical */
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	    /* +clear means output file with signature in the clear,
	       only in combination with -t and -a, not with -e or -b */
	    if (!encrypt_flag && !separate_signature &&
		emit_radix_64 && clear_signatures) {
		clearfile = workfile;
		strip_spaces = TRUE;
	    }
	    make_canonical(workfile, tempf);
	    if (!clearfile)
		rmtemp(workfile);
	    workfile = tempf;
	}
	if (attempt_compression || encrypt_flag || emit_radix_64 ||
	    output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	/* for clear signatures we create a separate signature */
	status = signfile(nestflag, separate_signature || (clearfile != NULL),
		   my_name, workfile, tempf, literal_mode, literal_file);
	rmtemp(workfile);
	workfile = tempf;

	if (status < 0) {	/* signfile failed */
	    fprintf(ebpout, LANG("\007Signature error\n"));
	    errorLvl = SIGNATURE_ERROR;
	    user_error();
	}
    } else if (!nestflag) {	/* !sign_file */
	/*      Prepend CTB_LITERAL byte to plaintext file.
	   --sure wish this pass could be optimized away. */
	if (attempt_compression || encrypt_flag || emit_radix_64 ||
	    output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	/* for clear signatures we create a separate signature */
	status = make_literal(workfile, tempf, literal_mode, literal_file);
	rmtemp(workfile);
	workfile = tempf;
    }

    if (encrypt_flag) {
        if (emit_radix_64 || output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	if (!conventional_flag) {
	    if (!filter_mode && !quietmode)
		fprintf(ebpout,
LANG("\n\nRecipients' public key(s) will be used to encrypt. "));
	    if (recipient == NULL || *recipient == NULL ||
		**recipient == '\0') {
		/* no recipient specified on command line */
		fprintf(ebpout,
LANG("\nA user ID is required to select the recipient's public key. "));
		fprintf(ebpout, LANG("\nEnter the recipient's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the recipient's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */

		if ((mcguffins = (char **) malloc(2 * sizeof(char *))) == NULL)
		{
		    fprintf(stderr, LANG("\n\007Out of memory.\n"));
		    exitEBP(7);
		}
		mcguffins[0] = mcguffin;
		mcguffins[1] = "";
	    } else {
		/* recipient specified on command line */
		mcguffins = recipient;
	    }
	    for (recipient = mcguffins; *recipient != NULL &&
		 **recipient != '\0'; recipient++) {
		CONVERT_TO_CANONICAL_CHARSET(*recipient);
	    }
	    status = encryptfile(mcguffins, workfile, tempf,
				 attempt_compression);
	} else {
	    status = safer_encryptfile(workfile, tempf, attempt_compression);
	}

	rmtemp(workfile);
	workfile = tempf;

	if (status < 0) {
	    fprintf(ebpout, LANG("\007Encryption error\n"));
	    errorLvl = (conventional_flag ? ENCR_ERROR : RSA_ENCR_ERROR);
	    user_error();
	} 
    } else if (attempt_compression && !separate_signature && !clearfile) {
	/*
	 * PGP used to be parsimonious about compression; originally, it only
	 * did it for files that were being encrypted (to reduce the
	 * redundancy in the plaintext), but it should really do it for
	 * anything where it's not a bad idea.
	 */
        if (emit_radix_64 || output_stdout)
	    tempf = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    tempf = tempfile(TMP_WIPE);
	squish_file(workfile, tempf);
	rmtemp(workfile);
	workfile = tempf;
    }

    /*
     * Write to stdout if explicitly asked to, or in filter mode and
     * no explicit file name was given.
     */
    if (output_stdout) {
	if (emit_radix_64) {
	    /* NULL for outputfile means write to stdout */
	    if (armor_file(workfile, NULL, inputfile, clearfile, FALSE) != 0) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (clearfile)
		rmtemp(clearfile);
	} else {
	    if (writePhantomOutput(workfile) < 0) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	}
	rmtemp(workfile);
    } else {
	char name[MAX_PATH];
        char *t;
	if (outputfile) {
	    strcpy(name, outputfile);
	} else {
	    strcpy(name, inputfile);
	    drop_extension(name);
	}
        do {
	    if (!outputfile && no_extension(name)) {
	        if (emit_radix_64)
				force_extension(name, ASC_EXTENSION);
	        else if (sign_flag && separate_signature)
				force_extension(name, SIG_EXTENSION);
	        else 
			if ((algostruct.PKencrypt == OLDALG) && (algostruct.PKsign == OLDALG) &&
				(algostruct.Blockcipher == OLDALG) && (algostruct.Hashing == OLDALG))
				force_extension(name, PGP_EXTENSION);
			 else 
				force_extension(name, EBP_EXTENSION);
			
#ifdef MACTC5
			if (addresfork) {
				drop_extension(name);
				force_extension(name, ".sdf");
			}
#endif
	    }
            if (!file_exists(name)) break;
            t=ck_dup_output(name, TRUE, !clearfile);
            if (t==NULL) user_error();
            if (clearfile && !strcmp(t,name)) break;
            strcpy(name,t);
        } while (TRUE);
	if (emit_radix_64) {
	    if (armor_file(workfile, name, inputfile, clearfile, FALSE) != 0) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (clearfile)
		rmtemp(clearfile);
	} else {
	    if ((outputfile = savetemp(workfile, name)) == NULL) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (!quietmode) {
		if (encrypt_flag)
		    fprintf(ebpout,
			    LANG("\nCiphertext file: %s\n"), outputfile);
		else if (sign_flag)
		    fprintf(ebpout,
			    LANG("\nSignature file: %s\n"), outputfile);
	    }
	}
#ifdef MACTC5
		AddOutputFiles(name);
		if (addresfork) {
			if(!AddResourceFork(name)) {
				short frefnum,len,i;
				char *p,*q;
				Handle h;
				c2pstr(name);
				q=file_tail(argv[2]);
				len=strlen(q);
				frefnum=OpenResFile((uchar *)name);
				h=NewHandle(len+1);
				HLock(h);
				p=*h;
				*p++=len;
				for (i=0; i<len; i++) *p++=*q++;
				AddResource(h,'STR ',500,(uchar *)"");
				ChangedResource(h);
				WriteResource(h);
				UpdateResFile(frefnum);
				CloseResFile(frefnum);
				p2cstr((uchar *)name);
			} else {
				BailoutAlert("AddResFork failed!");
				exitEBP(-1);
			}
		} 	
		if (binhex_flag) {
			if (binhex(name)) {
				BailoutAlert("BinHex failed!");
				exitEBP(-1);
			}
			remove(name);
		}
		if (use_clipboard) File2Scrap(name);
#endif /* MACTC5 */
    }

    if (wipeflag) {
	/* destroy every trace of plaintext */
	if (wipefile(inputfile) == 0) {
	    remove(inputfile);
	    fprintf(ebpout, LANG("\nFile %s wiped and deleted. "), inputfile);
	    fprintf(ebpout, "\n");
	}
    }

#ifdef MACTC5
	if(!addresfork && !use_clipboard)
		if (!emit_radix_64) EBPSetFinfo(outputfile,'Cryp','MEBP');
#endif

    exitEBP(EXIT_OK);
    return 0;			/* to shut up lint and some compilers */
#ifdef MACTC5
}				/* ebp_dispatch */
#else
}				/* main */
#endif

#ifdef MSDOS
#include <dos.h>
static char *dos_errlst[] =
{
    "Write protect error",	/* LANG ("Write protect error") */
    "Unknown unit",
    "Drive not ready",		/* LANG ("Drive not ready") */
    "3", "4", "5", "6", "7", "8", "9",
    "Write error",		/* LANG ("Write error") */
    "Read error",		/* LANG ("Read error") */
    "General failure",
};

/* handler for msdos 'harderrors' */
#ifndef OS2
#ifdef __TURBOC__		/* Turbo C 2.0 */
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
	bdos(2, (unsigned int) errbuf[i], 0);
    while (errbuf[++i]);
#ifdef __TURBOC__
    return 0;			/* ignore (fopen will return NULL) */
#else
    return;
#endif
}
#endif				/* MSDOS */
#endif

static void initsigs()
{
#ifdef MSDOS
#ifndef OS2
#ifdef __TURBOC__
    harderr(dostrap);
#else				/* MSC */
#ifndef __GNUC__		/* DJGPP's not MSC */
    _harderr(dostrap);
#endif
#endif
#endif
#endif				/* MSDOS */
#ifdef SIGINT
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, breakHandler);
#if defined(UNIX) || defined(VMS) || defined(ATARI)
#ifndef __PUREC__ /* PureC doesn't recognise all signals */
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	signal(SIGHUP, breakHandler);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, breakHandler);
#endif
#ifdef UNIX
    signal(SIGPIPE, breakHandler);
#endif
    signal(SIGTERM, breakHandler);
#ifdef MACTC5
	signal(SIGABRT,breakHandler);
	signal(SIGTERM,breakHandler);
#ifndef DEBUG
    signal(SIGTRAP, breakHandler);
    signal(SIGSEGV, breakHandler);
    signal(SIGILL, breakHandler);
#ifdef SIGBUS
    signal(SIGBUS, breakHandler);
#endif
#endif				/* DEBUG */
#endif				/* MACTC5 */
#endif				/* UNIX */
#endif				/* SIGINT */
}				/* initsigs */


static void do_armorfile(char *armorfile)
{
    char *tempf;
    char cipherfile[MAX_PATH];
    long lastpos, linepos = 0;
    int status;
    int success = 0;

    for (;;) {
	/* Handle transport armor stripping */
	tempf = tempfile(0);
	strip_spaces = FALSE;	/* de_armor_file() sets
				   this for clear signature files */
        use_charset_header = TRUE;
        lastpos = linepos;
	status = de_armor_file(armorfile, tempf, &linepos);
	if (status) {
	    fprintf(ebpout,
LANG("\n\007Error: Transport armor stripping failed for file %s\n"),
		    armorfile);
	    errorLvl = INVALID_FILE_ERROR;
	    user_error();	/* Bad file */
	}
	if (keepctx || de_armor_only) {
	    if (outputfile && de_armor_only) {
		if (strcmp(outputfile, "-") == 0) {
		    writePhantomOutput(tempf);
		    rmtemp(tempf);
		    return;
		}
		strcpy(cipherfile, outputfile);
	    } else
		if ((algostruct.PKencrypt == OLDALG) && (algostruct.PKsign == OLDALG) &&
			(algostruct.Blockcipher == OLDALG) && (algostruct.Hashing == OLDALG)) {
				strcpy(cipherfile, file_tail(armorfile));
				force_extension(cipherfile, PGP_EXTENSION);
		} else {
		strcpy(cipherfile, file_tail(armorfile));
		force_extension(cipherfile, EBP_EXTENSION);
	    }
	    if ((tempf = savetemp(tempf, cipherfile)) == NULL) {
		errorLvl = UNKNOWN_FILE_ERROR;
		user_error();
	    }
	    if (!quietmode)
		fprintf(ebpout,
LANG("Stripped transport armor from '%s', producing '%s'.\n"),
			armorfile, tempf);
	    /* -da flag: don't decrypt */
	    if (de_armor_only || do_decrypt(tempf) >= 0)
		++success;
	} else {
	    if (charset_header[0])        /* Check signature with charset from Charset: header */
		checksig_pass = 1;
	    if (do_decrypt(tempf) >= 0)
		++success;
	    rmtemp(tempf);
	    if (charset_header[0]) {
		if (checksig_pass == 2) { /* Sigcheck failed: try again with local charset */
		    tempf = tempfile(0);
		    use_charset_header = FALSE;
		    linepos = lastpos;
		    de_armor_file(armorfile, tempf, &linepos);
		    if (do_decrypt(tempf) >= 0)
		        ++success;
		    rmtemp(tempf);
		}
		checksig_pass = 0;
	    }
	}

	if (!is_armor_file(armorfile, linepos)) {
	    if (!success)	/* print error msg if we didn't
				   decrypt anything */
		user_error();
	    return;
	}
	fprintf(ebpout,
		LANG("\nLooking for next packet in '%s'...\n"), armorfile);
    }
}				/* do_armorfile */


static int do_decrypt(char *cipherfile)
{
    char *outfile = NULL;
    int status, i;
    boolean nested_info = FALSE;
    char ringfile[MAX_PATH];
    byte ctb;
    byte header[8];		/* used to classify file type at the end. */
    char preserved_name[MAX_PATH];
    char *newname;

    /* will be set to the original file name after processing a
       literal packet */
    preserved_name[0] = '\0';

    do {			/* while nested parsable info present */
	if (nested_info) {
	    rmtemp(cipherfile);	/* never executed on first pass */
	    cipherfile = outfile;
	}
	if (get_header_info_from_file(cipherfile, &ctb, 1) < 0) {
	    fprintf(ebpout,
LANG("\n\007Can't open ciphertext file '%s'\n"), cipherfile);
	    errorLvl = FILE_NOT_FOUND_ERROR;
	    return -1;
	}
	if (!is_ctb(ctb))	/* not a real CTB -- complain */
	    break;

	if (moreflag)
	    outfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	else
	    outfile = tempfile(TMP_WIPE);

	/* PKE is Public Key Encryption */
	if (is_ctb_type(ctb, CTB_PKE_TYPE)) {

	    if (!quietmode)
		fprintf(ebpout,
LANG("\nFile is encrypted.  Secret key is required to read it. "));

	    /* Decrypt to scratch file since we may have a LITERAL2 */
	    status = decryptfile(cipherfile, outfile);

	    if (status < 0) {	/* error return */
		errorLvl = RSA_DECR_ERROR;
		return -1;
	    }
	    nested_info = (status > 0);

	} else if (is_ctb_type(ctb, CTB_SKE_TYPE)) {

	    if (decrypt_only_flag) {
		/* swap file names instead of just copying the file */
		rmtemp(outfile);
		outfile = cipherfile;
		cipherfile = NULL;
		if (!quietmode)
		    fprintf(ebpout,
LANG("\nThis file has a signature, which will be left in place.\n"));
		nested_info = FALSE;
		break;		/* Do no more */
	    }
	    if (!quietmode && checksig_pass<=1)
		fprintf(ebpout,
LANG("\nFile has signature.  Public key is required to check signature.\n"));

	    status = check_signaturefile(cipherfile, outfile,
					 strip_sig_flag, preserved_name);

	    if (status < 0) {	/* error return */
		errorLvl = SIGNATURE_CHECK_ERROR;
		return -1;
	    }
	    nested_info = (status > 0);

	    if (strcmp(preserved_name, "/dev/null") == 0) {
		rmtemp(outfile);
		fprintf(ebpout, "\n");
		return 0;
	    }
	} else if (is_ctb_type(ctb, CTB_CKE_TYPE)) {

	    /* Conventional Key Encrypted ciphertext. */
	    /* Tell user it's encrypted here, and prompt for
	       password in subroutine. */
	    if (!quietmode)
		fprintf(ebpout, LANG("\nFile is conventionally encrypted.  "));
	    /* Decrypt to scratch file since it may be a LITERAL2 */
	    status = safer_decryptfile(cipherfile, outfile);
	    if (status < 0) {	/* error return */
		errorLvl = DECR_ERROR;
		return -1;	/* error exit status */
	    }
	    nested_info = (status > 0);

	} else if (is_ctb_type(ctb, CTB_COMPRESSED_TYPE)) {

	    /* Compressed text. */
	    status = decompress_file(cipherfile, outfile);
	    if (status < 0) {	/* error return */
		errorLvl = DECOMPRESS_ERROR;
		return -1;
	    }
	    /* Always assume nested information... */
	    nested_info = TRUE;

	} else if (is_ctb_type(ctb, CTB_LITERAL_TYPE) ||
		   is_ctb_type(ctb, CTB_LITERAL2_TYPE)) { /* Raw plaintext.
							     Just copy it.
							     No more nesting.
							   */

	    /* Strip off CTB_LITERAL prefix byte from file: */
	    /* strip_literal may alter plainfile; will set mode */
	    status = strip_literal(cipherfile, outfile,
				   preserved_name, &literal_mode);
	    if (status < 0) {	/* error return */
		errorLvl = UNKNOWN_FILE_ERROR;
		return -1;
	    }
	    nested_info = FALSE;
	} else if (ctb == CTB_CERT_SECKEY || ctb == CTB_CERT_PUBKEY) {

	    rmtemp(outfile);
	    if (decrypt_only_flag) {
		/* swap file names instead of just copying the file */
		outfile = cipherfile;
		cipherfile = NULL;
		nested_info = FALSE;	/* No error */
		break;		/* no further processing */
	    }
	    /* Key ring.  View it. */
	    fprintf(ebpout,
LANG("\nFile contains key(s).  Contents follow..."));
	    if (view_keyring(NULL, cipherfile, TRUE, FALSE) < 0) {
		errorLvl = KEYRING_VIEW_ERROR;
		return -1;
	    }
	    /* filter mode explicit requested with -f */
	    if (filter_mode && !preserve_filename)
		return 0;	/*    No output file */
	    if (batchmode)
		return 0;
	    if (ctb == CTB_CERT_SECKEY)
		strcpy(ringfile, globalSecringName);
	    else
		strcpy(ringfile, globalPubringName);
	    /*      Ask if it should be put on key ring */
	    fprintf(ebpout,
LANG("\nDo you want to add this keyfile to keyring '%s' (y/N)? "), ringfile);
	    if (!getyesno('n'))
		return 0;
	    status = addto_keyring(cipherfile, ringfile);
	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring add error. "));
		errorLvl = KEYRING_ADD_ERROR;
		return -1;
	    }
	    return 0;		/*    No output file */

	} else {		/* Unrecognized CTB */
	    break;
	}

    } while (nested_info);
    /* No more nested parsable information */

    /* Stopped early due to error */
    if (nested_info) {
	fprintf(ebpout,
"\7\nERROR: Nested data has unexpected format.  CTB=0x%02X\n", ctb);
	if (outfile)
	    rmtemp(outfile);
	if (cipherfile)
	    rmtemp(cipherfile);
	errorLvl = UNKNOWN_FILE_ERROR;
	return -1;
    }
    if (outfile == NULL) {	/* file was not encrypted */
	if (!filter_mode && !moreflag) {
	    fprintf(ebpout,
LANG("\007\nError: '%s' is not a ciphertext, signature, or key file.\n"),
		    cipherfile);
	    errorLvl = UNKNOWN_FILE_ERROR;
	    return -1;
	}
	outfile = cipherfile;
    } else {
	if (cipherfile)
	    rmtemp(cipherfile);
    }

    if (moreflag || (strcmp(preserved_name, CONSOLE_FILENAME) == 0)) {
	/* blort to screen */
	if (strcmp(preserved_name, CONSOLE_FILENAME) == 0) {
	    fprintf(ebpout,
LANG("\n\nThis message is marked \"For your eyes only\".  Display now \
(Y/n)? "));
	    if (batchmode
#ifdef UNIX
            || !isatty(fileno(stdout))	/* stdout is redirected! */
#endif
            || filter_mode || !getyesno('y')) {
		/* no -- abort display, and clean up */
                fprintf(ebpout, "\n");
		rmtemp(outfile);
		return 0;
	    }
	}
	if (!quietmode)
	    fprintf(ebpout, LANG("\n\nPlaintext message follows...\n"));
	else
	    putc('\n', ebpout);
	fprintf(ebpout, "------------------------------\n");
	more_file(outfile, strcmp(preserved_name, CONSOLE_FILENAME) == 0);
	/* Disallow saving to disk if outfile is console-only: */
	if (strcmp(preserved_name, CONSOLE_FILENAME) == 0) {
	    clearscreen();	/* remove all evidence */
	} else if (!quietmode && !batchmode) {
	    fprintf(ebpout, LANG("Save this file permanently (y/N)? "));
	    if (getyesno('n')) {
		char moreFilename[256];
		fprintf(ebpout, LANG("Enter filename to save file as: "));
#ifdef AMIGA
                requesterdesc=LANG("Enter filename to save file as: ");
#endif
                if (preserved_name[0]) {
		    fprintf(ebpout, "[%s]: ", file_tail(preserved_name));
#ifdef AMIGA
                    strcat(requesterdesc, "[");
                    strcat(requesterdesc, file_tail(preserved_name));
                    strcat(requesterdesc, "]:");
#endif
                }
#ifdef MACTC5
		if(!GetFilePath(LANG("Enter filename to save file as:"),moreFilename,PUTFILE))
			strcpy(moreFilename,"");
		else
			fprintf(ebpout, "%s\n",moreFilename);
#else
		getstring(moreFilename, 255, TRUE);
#endif
		if (*moreFilename == '\0') {
		    if (*preserved_name != '\0')
			savetemp(outfile, file_tail(preserved_name));
		    else
			rmtemp(outfile);
		} else
		    savetemp(outfile, moreFilename);
		return 0;
	    }
	}
	rmtemp(outfile);
	return 0;
    }				/* blort to screen */
    if (outputfile) {
	if (!strcmp(outputfile, "/dev/null")) {
	    rmtemp(outfile);
	    return 0;
	}
	filter_mode = (strcmp(outputfile, "-") == 0);
	strcpy(plainfile, outputfile);
    } else {
#ifdef VMS
	/* VMS null extension has to be ".", not "" */
	force_extension(plainfile, ".");
#else				/* not VMS */
	drop_extension(plainfile);
#endif				/* not VMS */
    }

    if (!preserve_filename && filter_mode) {
	if (writePhantomOutput(outfile) < 0) {
	    errorLvl = UNKNOWN_FILE_ERROR;
	    return -1;
	}
	rmtemp(outfile);
	return 0;
    }
    if (preserve_filename && preserved_name[0] != '\0')
	strcpy(plainfile, file_tail(preserved_name));

    if (quietmode) {
	if (savetemp(outfile, plainfile) == NULL) {
	    errorLvl = UNKNOWN_FILE_ERROR;
	    return -1;
	}
	return 0;
    }
    if (!verbose)	       /* if other filename messages were suppressed */
	fprintf(ebpout, LANG("\nPlaintext filename: %s"), plainfile);


/*---------------------------------------------------------*/

    /*      One last thing-- let's attempt to classify some of the more
       frequently occurring cases of plaintext output files, as an
       aid to the user.

       For example, if output file is a public key, it should have
       the right extension on the filename.

       Also, it will likely be common to encrypt files created by
       various archivers, so they should be renamed with the archiver
       extension.
     */
    get_header_info_from_file(outfile, header, 8);

    newname = NULL;
#ifdef MACTC5
	if (header[0] == CTB_CERT_SECKEY)
		EBPSetFinfo(plainfile,'SKey','MEBP');
#endif
    if (header[0] == CTB_CERT_PUBKEY) {
	/* Special case--may be public key, worth renaming */
#ifdef MACTC5
		EBPSetFinfo(plainfile,'PKey','MEBP');
#endif
	fprintf(ebpout,
LANG("\nPlaintext file '%s' looks like it contains a public key."),
		plainfile);
	newname = maybe_force_extension(plainfile, EBP_EXTENSION);
    }
    /* Possible public key output file */ 
    else if ((i = compressSignature(header)) >= 0) {
	/*      Special case--may be an archived/compressed file,
		worth renaming
	*/
	fprintf(ebpout, LANG("\nPlaintext file '%s' looks like a %s file."),
		plainfile, compressName[i]);
	newname = maybe_force_extension(plainfile, compressExt[i]);
    } else if (is_ctb(header[0]) &&
	       (is_ctb_type(header[0], CTB_PKE_TYPE)
		|| is_ctb_type(header[0], CTB_SKE_TYPE)
		|| is_ctb_type(header[0], CTB_CKE_TYPE))) {
	/* Special case--may be another ciphertext file, worth renaming */
	fprintf(ebpout,
LANG("\n\007Output file '%s' may contain more ciphertext or signature."),
		plainfile);
	newname = maybe_force_extension(plainfile, EBP_EXTENSION);
    }				/* Possible ciphertext output file */
#ifdef MACTC5
	if( (newname = savetemp(outfile, (newname ? newname : plainfile))) == NULL) {
#else
    if (savetemp(outfile, (newname ? newname : plainfile)) == NULL) {
#endif
	errorLvl = UNKNOWN_FILE_ERROR;
	return -1;
    }
#ifdef MACTC5
	else if( strcmp(newname, plainfile) != 0 )	/* 203a */
		strcpy(plainfile, newname);
#endif
    fprintf(ebpout, "\n");
    return 0;
}				/* do_decrypt */

static int do_keyopt(char keychar)
{
    char keyfile[MAX_PATH];
    char ringfile[MAX_PATH];
    char *workfile;
    int status;

    if ((filter_mode || batchmode)
	&& (keychar == 'g' || keychar == 'e' || keychar == 'd'
	    || (keychar == 'r' && sign_flag))) {
	errorLvl = NO_BATCH;
	arg_error();		/* interactive process, no go in batch mode */
    }
    /*
     * If we're not doing anything that uses stdout, produce output there,
     * in case user wants to redirect it.
     */
    if (!filter_mode)
	ebpout = stdout;

    switch (keychar) {

/*-------------------------------------------------------*/
    case 'g':
	{		/*      Key generation
			   Arguments: bitcount, bitcount
			 */
	    char keybits[6], ebits[6], *username = NULL;

	    /* 
	     * Why all this code?
	     * 
	     * Some people may object to PGP insisting on finding the
	     * manual somewhere in the neighborhood to generate a key.
	     * They bristle against this seemingly authoritarian
	     * attitude.  Some people have even modified PGP to defeat
	     * this feature, and redistributed their hotwired version to
	     * others.  That creates problems for me (PRZ).
	     * 
	     * Here is the problem.  Before I added this feature, there
	     * were maimed versions of the PGP distribution package
	     * floating around that lacked the manual.  One of them was
	     * uploaded to Compuserve, and was distributed to countless
	     * users who called me on the phone to ask me why such a
	     * complicated program had no manual.  It spread out to BBS
	     * systems around the country.  And a freeware distributor got
	     * hold of the package from Compuserve and enshrined it on
	     * CD-ROM, distributing thousands of copies without the
	     * manual.  What a mess.
	     * 
	     * Please don't make my life harder by modifying PGP to
	     * disable this feature so that others may redistribute PGP
	     * without the manual.  If you run PGP on a palmtop with no
	     * memory for the manual, is it too much to ask that you type
	     * one little extra word on the command line to do a key
	     * generation, a command that is seldom used by people who
	     * already know how to use PGP?  If you can't stand even this
	     * trivial inconvenience, can you suggest a better method of
	     * reducing PGP's distribution without the manual?
	     * 
	     * PLEASE DO NOT DISABLE THIS CHECK IN THE SOURCE CODE
	     * WITHOUT AT LEAST CALLING PHILIP ZIMMERMANN 
	     * (+1 303 541-0140, or prz@acm.org) TO DISCUSS IT. 
	     */
	    if (!nomanual && manuals_missing()) {
		char const *const *dir;
		fputs(LANG("\a\nError: EBP documentation not found.\n\
PGP looked for it in the following directories:\n"), ebpout);
#ifdef MACTC5
		fprintf(ebpout, "\t\"%s\"\n", appPathName);
#else
		for (dir = manual_dirs; *dir; dir++)
		    fprintf(ebpout, "\t\"%s\"\n", *dir);
#endif	/* MACTC5 */
		fputs(
LANG("and the doc subdirectory of each of the above.  Please put a copy of\n\
both volumes of the User's Guide in one of these directories.\n\
\n\
Under NO CIRCUMSTANCES should PGP ever be distributed without the PGP\n\
User's Guide, which is included in the standard distribution package.\n\
If you got a copy of PGP without the manual, please inform whomever you\n\
got it from that this is an incomplete package that should not be\n\
distributed further.\n\
\n\
PGP will not generate a key without finding the User's Guide.\n\
There is a simple way to override this restriction.  See the\n\
PGP User's Guide for details on how to do it.\n\
\n"), ebpout);
		return KEYGEN_ERROR;
	    }
	    if (myArgc > 2)
		strncpy(keybits, myArgv[2], sizeof(keybits) - 1);
	    else
		keybits[0] = '\0';

	    if (myArgc > 3)
		strncpy(ebits, myArgv[3], sizeof(ebits) - 1);
	    else
		ebits[0] = '\0';

	    /* If the -u option is given, use that username */
	    if (u_flag && my_name != NULL && *my_name != '\0')
		username = my_name;

	    /* dokeygen writes the keys out to the key rings... */
	    status = dokeygen(keybits, ebits, username);

	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keygen error. "));
		errorLvl = KEYGEN_ERROR;
	    }
#ifdef MACTC5
		else  {
			strcpy(ringfile, globalPubringName );
			EBPSetFinfo(ringfile,'PKey','MEBP');
			strcpy(ringfile, globalSecringName  );
			EBPSetFinfo(ringfile,'SKey','MEBP');                
		}
#endif              
	    return status;
	}			/* Key generation */

/*-------------------------------------------------------*/
    case 'c':
	{			/*      Key checking
				   Arguments: userid, ringfile
				 */

	    if (myArgc < 3) {	/* Default to all user ID's */
		mcguffin[0] = '\0';
	    } else {
		strcpy(mcguffin, myArgv[2]);
		if (strcmp(mcguffin, "*") == 0)
		    mcguffin[0] = '\0';
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    if (myArgc < 4)	/* default key ring filename */
		strcpy(ringfile, globalPubringName);
	    else
		strncpy(ringfile, myArgv[3], sizeof(ringfile) - 1);

	    if ((myArgc < 4 && myArgc > 2)     /* Allow just key file as arg */
		&&has_extension(myArgv[2], EBP_EXTENSION)) {
		strcpy(ringfile, myArgv[2]);
		mcguffin[0] = '\0';
	    }
	    status = dokeycheck(mcguffin, ringfile, CHECK_ALL);

	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring check error.\n"));
		errorLvl = KEYRING_CHECK_ERROR;
	    }
	    if (status >= 0 && mcguffin[0] != '\0')
		return status;	/* just checking a single user,
				   dont do maintenance */

	    if ((status = maint_check(ringfile, 0)) < 0 && status != -7) {
		fprintf(ebpout, LANG("\007Maintenance pass error. "));
		errorLvl = KEYRING_CHECK_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
		EBPSetFinfo(ringfile,'PKey','MEBP');
		}
#endif
	    return status == -7 ? 0 : status;
	}			/* Key check */

/*-------------------------------------------------------*/
    case 'm':
	{			/*      Maintenance pass
				   Arguments: ringfile
				 */

	    if (myArgc < 3)	/* default key ring filename */
		strcpy(ringfile, globalPubringName);
	    else
		strcpy(ringfile, myArgv[2]);

#ifdef MSDOS
	    strlwr(ringfile);
#endif
	    if (!file_exists(ringfile))
		default_extension(ringfile, EBP_EXTENSION);

	    if ((status = maint_check(ringfile,
		      MAINT_VERBOSE | (c_flag ? MAINT_CHECK : 0))) < 0) {
		if (status == -7)
		    fprintf(ebpout,
			    LANG("File '%s' is not a public keyring\n"),
			    ringfile);
		fprintf(ebpout, LANG("\007Maintenance pass error. "));
		errorLvl = KEYRING_CHECK_ERROR;
	    }
#ifdef MACTC5
		EBPSetFinfo(ringfile,'PKey','MEBP');
#endif
	    return status;
	}			/* Maintenance pass */

/*-------------------------------------------------------*/
    case 's':
	{			/*      Key signing
				   Arguments: her_id, keyfile
				 */

	    if (myArgc >= 4)
		strncpy(keyfile, myArgv[3], sizeof(keyfile) - 1);
	    else
		strcpy(keyfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to sign */
	    } else {
		fprintf(ebpout,
LANG("\nA user ID is required to select the public key you want to sign. "));
		if (batchmode)	/* not interactive, userid
				   must be on command line */
		    return -1;
		fprintf(ebpout, LANG("\nEnter the public key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the public key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    if (my_name[0] == '\0') {
		fprintf(ebpout,
LANG("\nA secret key is required to make a signature. "));
		fprintf(ebpout,
LANG("\nYou specified no user ID to select your secret key,\n\
so the default user ID and key will be the most recently\n\
added key on your secret keyring.\n"));
	    }
	    status = signkey(mcguffin, my_name, keyfile);

	    if (status >= 0) {
		status = maint_update(keyfile, 0);
		if (status == -7) { /* ringfile is a keyfile or
				       secret keyring */
		    fprintf(ebpout,
			    "Warning: '%s' is not a public keyring\n",
			    keyfile);
		    return 0;
		}
		if (status < 0)
		    fprintf(ebpout, LANG("\007Maintenance pass error. "));
	    }
	    if (status < 0) {
		fprintf(ebpout, LANG("\007Key signature error. "));
		errorLvl = KEY_SIGNATURE_ERROR;
	    }
#ifdef MACTC5
		EBPSetFinfo(keyfile,'PKey','MEBP');
#endif
	    return status;
	}			/* Key signing */


/*-------------------------------------------------------*/
    case 'd':
	{			/*      disable/revoke key
				   Arguments: userid, keyfile
				 */

	    if (myArgc >= 4)
		strncpy(keyfile, myArgv[3], sizeof(keyfile) - 1);
	    else
		strcpy(keyfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to sign */
	    } else {
		fprintf(ebpout,
LANG("\nA user ID is required to select the key you want to revoke or \
disable. "));
		fprintf(ebpout, LANG("\nEnter user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    status = disable_key(mcguffin, keyfile);

	    if (status >= 0) {
		status = maint_update(keyfile, 0);
		if (status == -7) { /* ringfile is a keyfile or
				       secret keyring */
		    fprintf(ebpout, "Warning: '%s' is not a public keyring\n",
			    keyfile);
		    return 0;
		}
		if (status < 0)
		    fprintf(ebpout, LANG("\007Maintenance pass error. "));
	    }
	    if (status < 0)
		errorLvl = KEY_SIGNATURE_ERROR;
#ifdef MACTC5
		EBPSetFinfo(keyfile,'PKey','MEBP');
#endif
	    return status;
	}			/* Key compromise */

/*-------------------------------------------------------*/
    case 'e':
	{			/*      Key editing
				   Arguments: userid, ringfile
				 */

	    if (myArgc >= 4)
		strncpy(ringfile, myArgv[3], sizeof(ringfile) - 1);
	    else		/* default key ring filename */
		strcpy(ringfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to edit */
	    } else {
		fprintf(ebpout,
LANG("\nA user ID is required to select the key you want to edit. "));
		fprintf(ebpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    status = dokeyedit(mcguffin, ringfile);

	    if (status >= 0) {
		status = maint_update(ringfile, 0);
		if (status == -7)
		    status = 0;	/* ignore "not a public keyring" error */
		if (status < 0)
		    fprintf(ebpout, LANG("\007Maintenance pass error. "));
	    }
	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring edit error. "));
		errorLvl = KEYRING_EDIT_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
		EBPSetFinfo(ringfile,'PKey','MEBP');
		}
#endif
	    return status;
	}			/* Key edit */

/*-------------------------------------------------------*/
    case 'a':
	{			/*      Add key to key ring
				   Arguments: keyfile, ringfile
				 */

	    if (myArgc < 3 && !filter_mode)
		arg_error();

	    if (!filter_mode) {	/* Get the keyfile from args */
		strncpy(keyfile, myArgv[2], sizeof(keyfile) - 1);

#ifdef MSDOS
		strlwr(keyfile);
#endif
		if (!file_exists(keyfile))
		    default_extension(keyfile, EBP_EXTENSION);

		if (!file_exists(keyfile)) {
		    fprintf(ebpout,
			    LANG("\n\007Key file '%s' does not exist.\n"),
			    keyfile);
		    errorLvl = NONEXIST_KEY_ERROR;
		    return -1;
		}
		workfile = keyfile;

	    } else {
		workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
		readPhantomInput(workfile);
	    }

	    if (myArgc < (filter_mode ? 3 : 4)) { /* default key ring
						     filename */
		byte ctb;
		get_header_info_from_file(workfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
		    strcpy(ringfile, globalSecringName);
		else
		    strcpy(ringfile, globalPubringName);
	    } else {
		strncpy(ringfile, myArgv[(filter_mode ? 2 : 3)],
			sizeof(ringfile) - 1);
		default_extension(ringfile, EBP_EXTENSION);
	    }
#ifdef MSDOS
	    strlwr(ringfile);
#endif

	    status = addto_keyring(workfile, ringfile);

	    if (filter_mode)
		rmtemp(workfile);

	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring add error. "));
		errorLvl = KEYRING_ADD_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
		EBPSetFinfo(ringfile,'PKey','MEBP');
		}
#endif
	    return status;
	}			/* Add key to key ring */

/*-------------------------------------------------------*/
    case 'x':
	case 'X':
	{			/*      Extract key from key ring
				   Arguments: mcguffin, keyfile, ringfile
				 */

	    if (myArgc >= (filter_mode ? 4 : 5)) /* default key ring
						    filename */
		strncpy(ringfile, myArgv[(filter_mode ? 3 : 4)],
			sizeof(ringfile) - 1);
	    else
		strcpy(ringfile, globalPubringName);

	    if (myArgc >= (filter_mode ? 2 : 3)) {
		if (myArgv[2])
		    /* Userid to extract */
		    strcpy(mcguffin, myArgv[2]);
		else
		    strcpy(mcguffin, "");
	    } else {
		fprintf(ebpout,
LANG("\nA user ID is required to select the key you want to extract. "));
		if (batchmode)	/* not interactive, userid
				   must be on command line */
		    return -1;
		fprintf(ebpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    if (!filter_mode) {
		if (myArgc >= 4)
		    strncpy(keyfile, myArgv[3], sizeof(keyfile) - 1);
		else
		    keyfile[0] = '\0';

		workfile = keyfile;
	    } else {
		workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
	    }

#ifdef MSDOS
	    strlwr(workfile);
	    strlwr(ringfile);
#endif

	    default_extension(ringfile, EBP_EXTENSION);

	    status = extract_from_keyring(mcguffin, workfile,
					  ringfile, (filter_mode ? FALSE :
						     emit_radix_64), (boolean) (keychar == 'X'));

	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring extract error. "));
		errorLvl = KEYRING_EXTRACT_ERROR;
		if (filter_mode)
		    rmtemp(workfile);
		return status;
	    }
	    if (filter_mode && !status) {
		if (emit_radix_64) {
		    /* NULL for outputfile means write to stdout */
		    if (armor_file(workfile, NULL, NULL, NULL, FALSE) != 0) {
			errorLvl = UNKNOWN_FILE_ERROR;
			return -1;
		    }
		} else {
		    if (writePhantomOutput(workfile) < 0) {
			errorLvl = UNKNOWN_FILE_ERROR;
			return -1;
		    }
		}
		rmtemp(workfile);
	    }
#ifdef MACTC5
		if (status)
			return KEYRING_EXTRACT_ERROR;
		if ((!emit_radix_64)&&(strlen(keyfile)>0)) {
		byte ctb;
		get_header_info_from_file(keyfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
			EBPSetFinfo(ringfile,'PKey','MEBP');
		}
#endif
	    return 0;
	}			/* Extract key from key ring */

/*-------------------------------------------------------*/
    case 'r':
	{	/*      Remove keys or selected key signatures from userid keys
			Arguments: userid, ringfile
		 */

	    if (myArgc >= 4)
		strcpy(ringfile, myArgv[3]);
	    else		/* default key ring filename */
		strcpy(ringfile, globalPubringName);

	    if (myArgc >= 3) {
		strcpy(mcguffin, myArgv[2]);	/* Userid to work on */
	    } else {
		if (sign_flag) {
		    fprintf(ebpout,
LANG("\nA user ID is required to select the public key you want to\n\
remove certifying signatures from. "));
		} else {
		    fprintf(ebpout,
LANG("\nA user ID is required to select the key you want to remove. "));
		}
		if (batchmode)	/* not interactive, userid must be on
				   command line */
		    return -1;
		fprintf(ebpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
                requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

#ifdef MSDOS
	    strlwr(ringfile);
#endif
	    if (!file_exists(ringfile))
		default_extension(ringfile, EBP_EXTENSION);

	    if (sign_flag) {	/* Remove signatures */
		if (remove_sigs(mcguffin, ringfile) < 0) {
		    fprintf(ebpout, LANG("\007Key signature remove error. "));
		    errorLvl = KEYSIG_REMOVE_ERROR;
		    return -1;
		}
	    } else {		/* Remove keyring */
#ifdef MACTC5
			if (remove_from_keyring( NULL, mcguffin, ringfile,
					(boolean)!strcmp(ringfile, globalPubringName))) {
#else
		if (remove_from_keyring(NULL, mcguffin, ringfile,
					(boolean) (myArgc < 4)) < 0) {
#endif
		    fprintf(ebpout, LANG("\007Keyring remove error. "));
		    errorLvl = KEYRING_REMOVE_ERROR;
		    return -1;
		}
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
		EBPSetFinfo(ringfile,'PKey','MEBP');
		EBPSetFinfo(globalPubringName,'PKey','MEBP');
		}
#endif
	    return 0;
	}			/* remove key signatures from userid */

/*-------------------------------------------------------*/
    case 'v':
    case 'V':			/* -kvv */
	{			/* View or remove key ring entries,
				   with userid match
				   Arguments: userid, ringfile
				 */

	    if (myArgc < 4)	/* default key ring filename */
		strcpy(ringfile, globalPubringName);
	    else
		strcpy(ringfile, myArgv[3]);

	    if (myArgc > 2) {
		strcpy(mcguffin, myArgv[2]);
		if (strcmp(mcguffin, "*") == 0)
		    mcguffin[0] = '\0';
	    } else {
		*mcguffin = '\0';
	    }

	    if ((myArgc == 3) && has_extension(myArgv[2], EBP_EXTENSION)) {
		strcpy(ringfile, myArgv[2]);
		mcguffin[0] = '\0';
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

#ifdef MSDOS
	    strlwr(ringfile);
#endif
	    if (!file_exists(ringfile))
		default_extension(ringfile, EBP_EXTENSION);

	    /* If a second 'v' (keychar = V), show signatures too */
	    status = view_keyring(mcguffin, ringfile,
				  (boolean) (keychar == 'V'), c_flag);
	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring view error. "));
		errorLvl = KEYRING_VIEW_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
		EBPSetFinfo(ringfile,'PKey','MEBP');
		}
#endif
	    return status;
	}			/* view key ring entries, with userid match */

/*-------------------------------------------------------*/

	case 'p':
		{		/*	Key Generation ++  */
 char keybits[6], ebits[6], *username = NULL;

	    if (!nomanual && manuals_missing()) {
		char const *const *dir;
		fputs(LANG("\a\nError: EBP documentation not found.\n\
PGP looked for it in the following directories:\n"), ebpout);
#ifdef MACTC5
		fprintf(ebpout, "\t\"%s\"\n", appPathName);
#else
		for (dir = manual_dirs; *dir; dir++)
		    fprintf(ebpout, "\t\"%s\"\n", *dir);
#endif	/* MACTC5 */
		fputs(
LANG("and the doc subdirectory of each of the above.  Please put a copy of\n\
both volumes of the EBP documentation in one of these directories.\n\
\n\
Under NO CIRCUMSTANCES should EBP ever be distributed without the EBP\n\
documentation, which is included in the standard distribution package.\n\
If you got a copy of EBP without the manual, please inform whomever you\n\
got it from that this is an incomplete package that should not be\n\
distributed further.\n\
\n\
EBP will not generate a key without finding the EBP documentation.\n\
There is a simple way to override this restriction.  See the\n\
PGP User's Guide in the EBP documentation for details on how to do it.\n\
\n"), ebpout);
		return KEYGEN_ERROR;
	    }
	    if (myArgc > 2)
		strncpy(keybits, myArgv[2], sizeof(keybits) - 1);
	    else
		keybits[0] = '\0';

	    if (myArgc > 3)
		strncpy(ebits, myArgv[3], sizeof(ebits) - 1);
	    else
		ebits[0] = '\0';

	    /* If the -u option is given, use that username */
	    if (u_flag && my_name != NULL && *my_name != '\0')
		username = my_name;

		SetKeyAlg(&keyalgos);

	    /* dokeygen writes the keys out to the key rings... */
	    status = dokeygen(keybits, ebits, username);

	    if (status < 0) {
		fprintf(ebpout, LANG("\007Keygen error. "));
		errorLvl = KEYGEN_ERROR;
	    }
#ifdef MACTC5
		else  {
			strcpy(ringfile, globalPubringName );
			EBPSetFinfo(ringfile,'PKey','MEBP');
			strcpy(ringfile, globalSecringName  );
			EBPSetFinfo(ringfile,'SKey','MEBP');                
		}
#endif              
		

		return status;
		}
/*-------------------------------------------------------*/
    case 'i':
	{			/*      Key import
				   Arguments: userid, ringfile, workfile
				 */

		if (!filter_mode) {	/* Get the keyfile from args */
	    if (myArgc < 3 && !filter_mode){
			fprintf(ebpout, LANG("\nEnter full filename for the secret key ring generated by PGP:\n "));
			getstring(keyfile, 59, TRUE);	/* echo keyboard input */

		}else 
		strncpy(keyfile, myArgv[2], sizeof(keyfile) - 1);

#ifdef MSDOS
		strlwr(keyfile);
#endif
		if (!file_exists(keyfile))
		    default_extension(keyfile, PGP_EXTENSION);

		if (!file_exists(keyfile)) {
		    fprintf(ebpout,
			    LANG("\n\007Key file '%s' does not exist.\n"),
			    keyfile);

		    errorLvl = NONEXIST_KEY_ERROR;
		    return -1;
		}
		workfile = keyfile;

	    } else {
		workfile = tempfile(TMP_WIPE | TMP_TMPDIR);
		readPhantomInput(workfile);
	    }

	    if (myArgc >= 4){
		strncpy(ringfile, myArgv[3], sizeof(ringfile) - 1);
		}else		/* default key ring filename */
		strcpy(ringfile, globalSecringName);

	    if (myArgc >= 5) {
		strcpy(mcguffin, myArgv[4]);	/* Userid to edit */
	    } else {
		fprintf(ebpout,
LANG("\nA user ID is required to select the PGP key you want to change to an EBP key. "));
		fprintf(ebpout, LANG("\nEnter the key's user ID: "));
#ifdef AMIGA
               requesterdesc=LANG("\nEnter the key's user ID: ");
#endif
		getstring(mcguffin, 255, TRUE);		/* echo keyboard */
	    }
	    CONVERT_TO_CANONICAL_CHARSET(mcguffin);

	    status = dokeyupdate(mcguffin, ringfile, workfile);

	    if (status >= 0) 
		status = maint_update(ringfile, 0);

		if (status < 0) {
		fprintf(ebpout, LANG("\007Keyring edit error. "));
		errorLvl = KEYRING_EDIT_ERROR;
	    }
#ifdef MACTC5
		{   
		byte ctb;
		get_header_info_from_file(ringfile, &ctb, 1);
		if (ctb == CTB_CERT_SECKEY)
			EBPSetFinfo(ringfile,'SKey','MEBP');
		else if (ctb == CTB_CERT_PUBKEY)
		EBPSetFinfo(ringfile,'PKey','MEBP');
		}
#endif
	    return status;
	}			/* Key import */

    default:
	arg_error();
    }
    return 0;
}				/* do_keyopt */

/* comes here if user made a boo-boo. */
void user_error()
{
    fprintf(ebpout, LANG("\nFor a usage summary, type:  ebp -h\n"));
    fprintf(ebpout,
	    LANG("For more detailed help, consult the EBP documentation.\n"));
    exitEBP(errorLvl ? errorLvl : 127);		/* error exit */
}

#if defined(DEBUG) && defined(linux)
#include <malloc.h>
#endif

/*
 * exitEBP: wipes and removes temporary files, also tries to wipe
 * the stack.
 */
void exitEBP(int returnval)
{
    char buf[STACK_WIPE];
    struct hashedpw *hpw;

    if (verbose)
	fprintf(ebpout, "exitEBP: exitcode = %d\n", returnval);
    for (hpw = passwds; hpw; hpw = hpw->next)
	memset(hpw->hash, 0, sizeof(hpw->hash));
    for (hpw = keypasswds; hpw; hpw = hpw->next)
	memset(hpw->hash, 0, sizeof(hpw->hash));
#ifdef MACTC5
	mac_cleanup_tmpf();
#else
    cleanup_tmpf();
#endif
    /* Merge any entropy we collected into the randseed.bin file */
    if (cryptRandOpen((struct SaferCfbContext *)0) >= 0)
	    cryptRandSave((struct SaferCfbContext *)0);
#if defined(DEBUG) && defined(linux)
    if (verbose) {
	struct mstats mstat;
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
#endif				/* VMS */
    exit(returnval);
}

static void arg_error()
{
    signon_msg();
    fprintf(ebpout, LANG("\nInvalid arguments.\n"));
    errorLvl = BAD_ARG_ERROR;
    user_error();
}

/*
 * Check for language specific help files in EBPPATH, then the system
 * directory.  If that fails, check for the default ebp.hlp, again
 * first a private copy, then the system-wide one.
 *
 * System-wide copies currently only exist on Unix.
 */
static void build_helpfile(char *helpfile, char const *extra)
{
    if (strcmp(language, "en")) {
	buildfilename(helpfile, language);
	strcat(helpfile, extra);
	force_extension(helpfile, HLP_EXTENSION);
	if (file_exists(helpfile))
	    return;
#ifdef EBP_SYSTEM_DIR
	strcpy(helpfile, EBP_SYSTEM_DIR);
	strcat(helpfile, language);
	strcat(helpfile, extra);
	force_extension(helpfile, HLP_EXTENSION);
	if (file_exists(helpfile))
	    return;
#endif
    }
    buildfilename(helpfile, "ebp");
    strcat(helpfile, extra);
    force_extension(helpfile, HLP_EXTENSION);
#ifdef EBP_SYSTEM_DIR
    if (file_exists(helpfile))
	return;
    strcpy(helpfile, EBP_SYSTEM_DIR);
    strcat(helpfile, "ebp");
    strcat(helpfile, extra);
    force_extension(helpfile, HLP_EXTENSION);
#endif
}

static void usage()
{
    char helpfile[MAX_PATH];
    char *tmphelp = helpfile;
    extern unsigned char *ext_c_ptr;

    signon_msg();
    build_helpfile(helpfile, "");

    if (ext_c_ptr) {
	/* conversion to external format necessary */
	tmphelp = tempfile(TMP_TMPDIR);
	CONVERSION = EXT_CONV;
	if (copyfiles_by_name(helpfile, tmphelp) < 0) {
	    rmtemp(tmphelp);
	    tmphelp = helpfile;
	}
	CONVERSION = NO_CONV;
    }
    /* built-in help if ebp.hlp is not available */
    if (more_file(tmphelp, FALSE) < 0)
	fprintf(ebpout, LANG("\nUsage summary:\
\nTo encrypt a plaintext file with recipent's public key, type:\
\n   ebp -e textfile her_userid [other userids] (produces textfile.ebp)\
\nTo sign a plaintext file with your secret key:\
\n   ebp -s textfile [-u your_userid]           (produces textfile.ebp)\
\nTo sign a plaintext file with your secret key, and then encrypt it\
\n   with recipent's public key, producing a .ebp file:\
\n   ebp -es textfile her_userid [other userids] [-u your_userid]\
\nTo encrypt with conventional encryption only:\
\n   ebp -c textfile\
\nTo decrypt or check a signature for a ciphertext (.ebp) file:\
\n   ebp ciphertextfile [-o plaintextfile]\
\nTo produce output in ASCII for email, add the -a option to other options.\
\nFor help on choosing your algorithms, type:          ebp -j\
\nTo generate your own unique public/secret key pair:  ebp -kg\
\nTo generate key pair with key generation++:          ebp -kp\
\nFor help on other key management functions, type:    ebp -k\n"));
    if (ext_c_ptr)
	rmtemp(tmphelp);
    exit(BAD_ARG_ERROR);	/* error exit */
}

static void key_usage()
{
    char helpfile[MAX_PATH];
    char *tmphelp = helpfile;
    extern unsigned char *ext_c_ptr;

    signon_msg();
    build_helpfile(helpfile, "key");

    if (ext_c_ptr) {
	/* conversion to external format necessary */
	tmphelp = tempfile(TMP_TMPDIR);
	CONVERSION = EXT_CONV;
	if (copyfiles_by_name(helpfile, tmphelp) < 0) {
	    rmtemp(tmphelp);
	    tmphelp = helpfile;
	}
	CONVERSION = NO_CONV;
    }
    /* built-in help if key.hlp is not available */
    if (more_file(tmphelp, FALSE) < 0)
	/* only use built-in help if there is no helpfile */
	fprintf(ebpout, LANG("\nKey management functions:\
\nTo generate your own unique public/secret key pair:\
\n   ebp -kg\
\nTo do Advanced Key Generation:\
\n   ebp -kp\
\nTo add a key file's contents to your public or secret key ring:\
\n   ebp -ka keyfile [keyring]\
\nTo remove a key or a user ID from your public or secret key ring:\
\n   ebp -kr userid [keyring]\
\nTo edit your user ID or pass phrase:\
\n   ebp -ke your_userid [keyring]\
\nTo extract (copy) a key from your public or secret key ring:\
\n   ebp -kx userid keyfile [keyring]\
\nTo extract a public EBP key into a public PGP key ring:\
\n   ebp -kxx userid keyfile [keyring]\
\nTo view the contents of your public key ring:\
\n   ebp -kv[v] [userid] [keyring]\
\nTo check signatures on your public key ring:\
\n   ebp -kc [userid] [keyring]\
\nTo sign someone else's public key on your public key ring:\
\n   ebp -ks her_userid [-u your_userid] [keyring]\
\nTo remove selected signatures from a userid on a keyring:\
\n   ebp -krs userid [keyring]\
\nTo import old PGP key to your EBP key rings:\
\n   ebp -ki [pgp secring] [ebp secring] [userid]\
\n"));
    if (ext_c_ptr)
	rmtemp(tmphelp);
    exit(BAD_ARG_ERROR);	/* error exit */
}

static void alg_usage()
{
    char helpfile[MAX_PATH];
    char *tmphelp = helpfile;
    extern unsigned char *ext_c_ptr;

    signon_msg();
    build_helpfile(helpfile, "alg");

    if (ext_c_ptr) {
	/* conversion to external format necessary */
	tmphelp = tempfile(TMP_TMPDIR);
	CONVERSION = EXT_CONV;
	if (copyfiles_by_name(helpfile, tmphelp) < 0) {
	    rmtemp(tmphelp);
	    tmphelp = helpfile;
	}
	CONVERSION = NO_CONV;
    }
    /* built-in help if key.hlp is not available */
    if (more_file(tmphelp, FALSE) < 0)
	/* only use built-in help if there is no helpfile */
	fprintf(ebpout, LANG("\nAlgorithm functions:\
\nTo use Rabin's public key scheme to encrypt blockcipher key:\
\n   ebp -ja\
\nTo use RSA to encrypt blockcipher key:\
\n   ebp -jb\
\nTo use Rabin's public key scheme to sign a message digest:\
\n   ebp -jc\
\nTo use RSA to sign a message digest:\
\n   ebp -jd\
\nTo use SAFER for conventional encryption:\
\n   ebp -je\
\nTo use IDEA for conventional encryption:\
\n   ebp -jf\
\nTo use HAVAL to create message digests:\
\n   ebp -jg\
\nTo use MD5 to create message digests:\
\n   ebp -jh\
\nTo set desired length of the message digest:\
\n   ebp -ji\
\nTo set the desired number of passes:\
\n   ebp -jj\
\nTo set the default PGP algorithms:\
\n   ebp -jk\
\nTo set the default EBP algorithms:\
\n   ebp -jl\
\n"));
    if (ext_c_ptr)
	rmtemp(tmphelp);
    exit(BAD_ARG_ERROR);	/* error exit */
}

char **ParseRecipients(char **recipients)
{
	/*
	 * ParseRecipients() expects an array of pointers to
	 * characters, usually the array returned by the C startup
	 * code. Then it will look for entries beginning with the
  	 * string "-@" followed by a filename, which may be appended
 	 * directly or seperated by a blank.
 	 *
 	 * If the file exists and is readable, the routine will load
 	 * the contents and insert it into the command line as if the
 	 * names had been specified there.
 	 *
 	 * Each entry in the file consists of one line. The file line
 	 * will be treated as one argument, no matter whether it
 	 * contains spaces or not. Lines beginning with "#" will be
 	 * ignored and treated as comments. Empty lines will be ignored
 	 * also. Trailing white spaces will be removed.
 	 *
 	 * Currently, ParseRecipients() uses one fixed buffer, meaning,
 	 * that one single line must not be longer than 255 characters.
 	 * The number of included lines is unlimited.
 	 *
 	 * When any kind of problem occurs, PGP will terminate and do
 	 * nothing. No need to test for an error, the result is always
 	 * correct.
 	 *
  	 *             21-Sep-95, Peter Simons <simons@peti.rhein.de>
 	 */

	char **backup = recipients, **new;
	int entrynum;
 	int MAX_RECIPIENTS = 128;   /* The name is somewhat wrong. of
 				     * course the memory handling is
 				     * dynamic.
 				     */

 	/* Check whether we need to do something or not. */
 	while(*recipients) {
 		if (!strncasecmp(*recipients, INCLUDE_MARK, INCLUDE_MARK_LEN))
 		    break;
 		recipients++;
 	}
 	if (!*recipients)
 	  return backup;	/* nothing happened */

 	recipients=backup;
 	if (!(new = malloc(MAX_RECIPIENTS * sizeof(char *))))
 	  exitEBP(OUT_OF_MEM);
 	entrynum = 0;

 	while(*recipients) {
 		if (strncasecmp(*recipients, INCLUDE_MARK, INCLUDE_MARK_LEN))
                {
 			new[entrynum++] = *recipients++;
 			if (entrynum == MAX_RECIPIENTS) {
 				/* Current buffer is too small.
 				 * Use realloc() to largen itt.
 				 */
 				MAX_RECIPIENTS += 128;
 				if (!(new = realloc(new,
                                 MAX_RECIPIENTS * sizeof(char *))))
 				  exitEBP(OUT_OF_MEM);
 			}
 		}
 		else {
 			/* We got a hit! Load the file and parse it. */
 			FILE *fh;
 			char *filename, tempbuf[256];

 			if (strlen(*recipients) == INCLUDE_MARK_LEN)
 			  filename = *++recipients;
 			else
 			  filename = *recipients+INCLUDE_MARK_LEN;
 			fprintf(ebpout, LANG("\nIncluding \"%s\"...\n"), filename);
 			if (!(fh = fopen(filename, "r"))) {
 				perror("EBP");
 				exitEBP(UNKNOWN_FILE_ERROR);
 			}
 			while(fgets(tempbuf, sizeof(tempbuf)-1, fh)) {
 				int i = strlen(tempbuf);

 				/* Test for comments or empty lines. */
 				if (!i || *tempbuf == '#')
 				  continue;

 				/* Remove trailing blanks. */
 				while (isspace(tempbuf[i-1]))
 				  i--;
 				tempbuf[i] = '\0';

 				/* Copy new entry to new */
 				if (!(new[entrynum++] = strdup(tempbuf)))
 				  exitEBP(OUT_OF_MEM);
 				if (entrynum == MAX_RECIPIENTS) {
 					/* Current buffer is too small.
 					 * Use realloc() to largen itt.
 					 */
 					MAX_RECIPIENTS += 128;
 					if (!(new = realloc(new,
                                         MAX_RECIPIENTS * sizeof(char *))))
 					  exitEBP(OUT_OF_MEM);
 				}
 			}
 			if (ferror(fh)) {
 				perror("EBP");
 				exitEBP(UNKNOWN_FILE_ERROR);
 			}
 			fclose(fh);
 			recipients++;
 		}
 	}

 	/*
 	 * We have to write one trailing NULL pointer.
 	 * Check array size first.
 	 */
 	if (entrynum == MAX_RECIPIENTS) {
 		if (!(new = realloc(new, (MAX_RECIPIENTS+1) * sizeof(char *))))
 		  exitEBP(OUT_OF_MEM);
 	}
 	new[entrynum] = NULL;
 	return new;
}
