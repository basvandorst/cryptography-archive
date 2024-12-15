/*	mdfile.h - C headers for PGP MD5 Message digest routines.
	PGP 1.0 used MD4, now we use MD5.
*/

#include "md5.h"	/* for MD5 message digest stuff */

/* Prototypes for MD5 routines */

/* Computes and returns the message digest from file position,longcount bytes */
int MDfile0_len(MD5_CTX *mdContext, FILE *f, word32 longcount);

/* Computes the message digest for a specified file */
int MDfile(MD5_CTX *mdContext,char *filename);

/* Used to finish the message digest, including extra material. */
void MD_addbuffer (MD5_CTX *mdContext, byte *buf, int buflen, byte digest[16]);

