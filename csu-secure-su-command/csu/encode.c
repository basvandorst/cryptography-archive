/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

#include <math.h>	/* abs() */

#include "csu.h"
#include "global.h"
#include "md5.h"

/* To change the number of characters that are possible, just add them to the
 * list here.  The functions will figure out how many are in the list */
#ifdef SECURE
char hex[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()+=-[]{};:<>,.?";
#else
char hex[] = "0123456789abcdef";
#endif /* SECURE */

#ifndef SECURE
/* The small version of the MD5 encode function */
void encode (char *password, char *userid) {
	MD5_CTX context;
	unsigned char digest[16];
	unsigned int len;
	char string[8];
	short int i;
	extern char hex[];

	/* figure out the length of the hex string */
	int hex_len = strlen(hex);

	/* put the userid at the end of the password and calculate the length */
	strcat(password, userid);
	len = strlen(password);

	/* MD5 the password/userid string */
	MD5Init (&context);
	MD5Update(&context, (unsigned char *)password, len);
	MD5Final (digest, &context);

	/* grab the low order of the last 8 bytes */
	for(i = 8; i < 16; ++i) {
		string[i-8] = hex[(int)digest[i]-(abs((int)digest[i]/hex_len))*hex_len];
	}

	strcpy(password, string);
}
#endif /* !SECURE */

void account_encode (char *password, char *userid) {
        MD5_CTX context;
        unsigned char digest[16];
        unsigned int len;
        char string[32];
        short int i;
        extern char hex[];

	/* figure out the length of the hex string */
	int hex_len = strlen(hex);

	/* put the userid at the end of the password and calculate the length */
        strcat(password, userid);
        len = strlen(password);

	/* MD5 the password/userid string */
        MD5Init (&context);
        MD5Update (&context, (unsigned char *)password, len);
        MD5Final (digest, &context);

	/* convert the digest into a string of bytes */
        for(i = 0; i < 16; ++i) {
                string[i*2] = hex[abs((int)digest[i]/hex_len)];
                string[i*2+1] = hex[(int)digest[i]-(abs((int)digest[i]/hex_len))*hex_len];
        }
        strcpy(password, string);
}
