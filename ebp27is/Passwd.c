/*	passwd.c - Password reading/hashing routines
	Implemented in Microsoft C.
	Routines for getting a pass phrase from the user's console.

   EBP(c) - Even Better Privacy - is the new generation of public key
   cryptography. EBP is based on the code to PGP 2.6.3(i) and was developed
   by Niklas Jarl (LTH, ISS-NUS), Jonas Loefgren (LTH, ISS-NUS) and 
   Dr. Yongfei Han (ISS-NUS). The main purpose of this release is to let
   PGP users and other people that may need strong cryptography try the new
   algorithms and give them the option to choose the algorithms they trust
   the most. 
   
   WARNING! This program is for non-commercial use only! It may not be used
   commercially in any way. To do so you need a written permission not only
   by the authors of EBP, but also the authors and patentees of the various 
   algorithms. However, look out for the coming program called 
   "PC Security and Privacy(c)", which will be a fully commercial 
   cryptographic program with even more choice of algorithms. 

  Now over to the old PGP information:

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
*/

#include	<stdio.h>	/* for fprintf() */
#include	<ctype.h>	/* for isdigit(), toupper(), etc. */
#include	<string.h>	/* for strlen() */

#include	"random.h"	/* for getstring() */
#include	"md5.h"
#include    "haval.h"
#include    "havalapp.h"
#include	"language.h"
#include	"ebp.h"
#include    "charset.h"

#ifdef AMIGA
 #include "system.h"
#endif

#define MAXKEYLEN 254	/* max byte length of pass phrase */

boolean showpass = FALSE;

/*
**	hashpass - Hash pass phrase down to 128 bits (16 bytes).
**  keylen must be less than 1024.
**	Use the MD5 algorithm.
*/
void hashpass (char *keystring, int keylen, byte *hash)
{
	struct MD5Context mdContext;

	/* Calculate the hash */
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *) keystring, keylen);
	MD5Final(hash, &mdContext);
} /* hashpass */


/*
**	GetHashedPassPhrase - get pass phrase from user,
		 hashes it to a 128-bit SAFER key.
	Parameters:
		returns char *keystring as the pass phrase itself
		return char *hash as the 32-byte hash of the pass phrase
				using HAVAL.
		byte noecho:  
			0=ask once, echo. 
			1=ask once, no echo. 
			2=ask twice, no echo.
	Return 0 if no characters are input, else return 1.
	If we return 0, the hashed key will not be useful.
*/
int GetHashedPassPhrase(char *hash, boolean noecho, int tries, 
						char keystr1[MAXKEYLEN+2])
{	char keystr2[MAXKEYLEN+2];
	int len;
	if ((tries == 0)||(tries == -1))
	{
	if (showpass)
		noecho = 0;
	for (;;) {
		fprintf(ebpout,LANG("\nEnter pass phrase: "));
#ifdef AMIGA
        requesterdesc=LANG("\nEnter pass phrase: ");
#endif
		getstring(keystr1,MAXKEYLEN-1,!noecho);
		if (noecho<2)	/* no need to ask again if user can see it */
			break;
		fprintf(ebpout,LANG("\nEnter same pass phrase again: "));
#ifdef AMIGA
        requesterdesc=LANG("\nEnter same pass phrase again: ");
#endif
		getstring(keystr2,MAXKEYLEN-1,!noecho);
		if (strcmp(keystr1,keystr2)==0)
			break;
		fprintf(ebpout,
LANG("\n\007Error: Pass phrases were different.  Try again."));
		memset(keystr2, 0, sizeof(keystr2));
	}
	if (noecho && (filter_mode || quietmode))
		putc('\n', ebpout);
	}
	len = strlen(keystr1);
	if (len == 0)
		return 0;
	CONVERT_TO_CANONICAL_CHARSET(keystr1);
	
if (algostruct.Hashing == NEWALG)
	haval_string(keystr1, (byte *) hash);
else
	hashpass (keystr1, strlen(keystr1), (byte *) hash);

if (tries == -1)
	memset(keystr1, 0, sizeof(keystr1));
	memset(keystr2, 0, sizeof(keystr2));
	return 1;
} /* GetHashedPassPhrase */

