/*	idea.h - header file for idea.c
*/

#include "usuals.h"  /* typedefs for byte, word16, boolean, etc. */
#define DISKBUFSIZE 4096
#define IDEAKEYSIZE 16
#define IDEABLOCKSIZE 8


void initcfb_xx(word16 iv0[4], byte key[16], boolean decryp, byte key2[16]);
void xxcfb(byteptr buf, int count);
void close_xx(void);

#if 0
void init_xxrand(byte key[16], byte seed[8], word32 tstamp);
byte xxrand(void);
void close_xxrand(void);
#endif


/* prototypes for passwd.c */

/* getxxkey - get pass phrase from user, hashes it to an IDEA key. */
int getxxkey(char *keystring, char *hash, boolean noecho);

/* hashpass - Hash pass phrase down to 128 bits (16 bytes). */
void hashpass (char *keystring, int keylen, byte *hash);
