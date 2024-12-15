/*	passwd.c - Password reading/hashing routines
	(c) 1989 Philip Zimmermann.  All rights reserved.
	Implemented in Microsoft C.
	Routines for getting a pass phrase from the user's console.
*/

#include	<stdio.h>	/* for fprintf() */
#include	<ctype.h>	/* for isdigit(), toupper(), etc. */
#include	<string.h>	/* for strlen() */

#include	"random.h"	/* for getstring() */
#include	"md5.h"
#include	"language.h"
#include	"pgp.h"

#define MAXKEYLEN 254	/* max byte length of pass phrase */

boolean showpass = FALSE;

/*
**	hashpass - Hash pass phrase down to 128 bits (16 bytes).
**  keylen must be less than 1024.
**	Use the MD5 algorithm.
*/
void hashpass (char *keystring, int keylen, byte *hash)
{
	MD5_CTX	mdContext;
	int		i;

	/* Calculate the hash */
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *) keystring, keylen);
	MD5Final(&mdContext);
	/* Copy it to return variable */
	memcpy(hash, mdContext.digest, 16);
}	/* hashpass */


/*
**	GetHashedPassPhrase - get pass phrase from user, hashes it to an IDEA key.
	Parameters:
		returns char *keystring as the pass phrase itself
		return char *hash as the 16-byte hash of the pass phrase
				using MD5.
		byte noecho:  
			0=ask once, echo. 
			1=ask once, no echo. 
			2=ask twice, no echo.
	Return 0 if no characters are input, else return 1.
	If we return 0, the hashed key will not be useful.
*/
int GetHashedPassPhrase(char *keystring, char *hash, boolean noecho)
{	char keystr2[MAXKEYLEN+2];
	int len;

	if (showpass)
		noecho = 0;
	while (TRUE) {
		fprintf(pgpout,PSTR("\nEnter pass phrase: "));
		getstring(keystring,MAXKEYLEN-1,!noecho);
		if (noecho<2)	/* no need to ask again if user can see it */
			break;
		fprintf(pgpout,PSTR("\nEnter same pass phrase again: "));
		getstring(keystr2,MAXKEYLEN-1,!noecho);
		if (strcmp(keystring,keystr2)==0)
			break;
		fprintf(pgpout,PSTR("\n\007Error: Pass phrases were different.  Try again."));
	}
	if (noecho && filter_mode)
		putc('\n', pgpout);

	len = strlen(keystring);
	if (len == 0)
		return 0;
	/* We assume ASCII pass phrases, with no charset conversions. */
	hashpass (keystring, strlen(keystring), (byte *) hash);
	return 1;
}	/* GetHashedPassPhrase */

