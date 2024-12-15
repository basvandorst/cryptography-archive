/*	idea.h - header file for idea.c
*/

#include "usuals.h"  /* typedefs for byte, word16, boolean, etc. */

#define IDEAKEYSIZE 16
#define IDEABLOCKSIZE 8


#define ROUNDS	8		/* Don't change this value, should be 8 */
#define KEYLEN	(6*ROUNDS+4)	/* length of key schedule */

typedef word16 IDEAkey[KEYLEN];
void initcfb_idea(IDEAkey Z, word16 iv0[4], byte key[16], boolean decryp);
void ideacfb(byteptr buf, int count, IDEAkey Z, word16 *iv, boolean decryp);
void close_idea(IDEAkey Z);

void init_idearand(byte key[16], byte seed[8], word32 tstamp);
byte idearand(void);
void close_idearand(void);


/* prototypes for passwd.c */

/* GetHashedPassPhrase - get pass phrase from user, hashes it to an IDEA key. */
int GetHashedPassPhrase(char *keystring, char *hash, boolean necho);

/* hashpass - Hash pass phrase down to 128 bits (16 bytes). */
void hashpass (char *keystring, int keylen, byte *hash);
