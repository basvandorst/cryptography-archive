/*	passwd.h

	Added to PGP build by Steve Rencontre to prevent 'no prototype' warnings
*/

#ifndef _PASSWD_H_		/* guard against multiple includes */
#define _PASSWD_H_

#define MAXKEYLEN 254	/* max byte length of pass phrase */

extern boolean showpass;

void hashpass (char *keystring, int keylen, byte *hash);

int GetHashedPassPhrase(char *hash, boolean noecho);

#endif
