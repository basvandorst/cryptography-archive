/*
	This code is by MJR. See README.MJR.
	Small changes by vince
*/

#include <stdio.h>
#ifdef HASSTDLIB
#include <stdlib.h>
#else
#include <sys/types.h>
#endif
#include <string.h>
#ifdef	__MSDOS__
#include <dos.h>
#else	/* Assume BSD unix */
#include <fcntl.h>
#endif
#include "md4.h"
#include "des.h"
#include "skey.h"

extern	char	*getenv();

/* Crunch a key:
 * DES decrypt the user's pad file and then crank it through MD4 and
 * collapse to 64 bits. This is defined as the user's starting key.
 */
int
deskeycrunch(result,seed,passwd)
char *result;	/* 8-byte result */
char *seed;	/* Seed, any length */
char *passwd;	/* Password, any length */
{
	MD4_CTX md;
	char *padfile;
	char desobuf[BUFSIZ];
	char desibuf[BUFSIZ];
	des_key_schedule ks;
	des_cblock kk;
	char iv[8];
	FILE *desfd;
	int i;
	/**/ 
	unsigned long	buffer[4];

	if((padfile = getenv("SKEYPADFILE")) == (char *)0) {
		fprintf(stderr,"No SKEYPADFILE in environment\n");
		return(-1);
	}

	if((desfd = fopen(padfile,"r")) == (FILE *)0) {
		perror(padfile);
		return(-1);
	}

	des_string_to_key(passwd, (des_cblock *)kk);
	des_set_key((des_cblock *)kk, ks);
	bzero(passwd,strlen(passwd));
	bzero(kk,sizeof(kk));
	bzero(iv,sizeof(iv));

	/* decrypt our pad file and MD4 it */
	MD4Init(&md);
	while((i = fread(desibuf,1,64,desfd)) >= 8) {
		if(i % 8 != 0)
			i = i - (i % 8);
		des_cbc_encrypt((des_cblock *)desibuf, (des_cblock *)desobuf,
				i, ks, (des_cblock *)iv, DES_DECRYPT);
		MD4Update(&md,(unsigned char *)desobuf, i);
	}
	fclose(desfd);
	if(i < 0) {
		perror("read");
		return(-1);
	}

	MD4Final((unsigned char *)buffer,&md);

	/* Fold result from 128 to 64 bits */
	buffer[0] ^= buffer[2];
	buffer[1] ^= buffer[3];

	memcpy(result,(char *)buffer,8);
	return 0;
}
