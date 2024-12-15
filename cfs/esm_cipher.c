/*
 * ESM crypto interface
 * v0.6
 */

/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include "global.h"
#include "rsaref.h"
#include "esm.h"

unsigned char check[8];

unsigned char skey1[8];
unsigned char skey2[8];
unsigned char skey3[8];
unsigned char mkey1[8];
unsigned char mkey2[8];
unsigned char mkey3[8];
unsigned char ivin[8];
unsigned char ivout[8];

R_RANDOM_STRUCT rs;

R_DH_PARAMS dhparams[3];

unsigned char otherpub[MAXPUBKEY+2];
unsigned char ourpub[MAXPUBKEY+2];
unsigned char ourpriv[MAXPUBKEY];
int ourprivlen;
int pklen;

keystr inks, outks;

cipherinit(master)
     int master;
{
	if (master) {
		des_key_setup(skey1,inks.ek1);
		des_key_setup(skey2,inks.ek2);
		des_key_setup(skey3,inks.ek3);
		des_key_setup(mkey1,outks.ek1);
		des_key_setup(mkey2,outks.ek2);
		des_key_setup(mkey3,outks.ek3);
	} else {
		des_key_setup(skey1,outks.ek1);
		des_key_setup(skey2,outks.ek2);
		des_key_setup(skey3,outks.ek3);
		des_key_setup(mkey1,inks.ek1);
		des_key_setup(mkey2,inks.ek2);
		des_key_setup(mkey3,inks.ek3);
	}
}

/* 8 bit cfb encrypt */
unsigned int cfb8_encrypt(c)
     unsigned int c;
     
{
	int i;
	char blk[8];

	for (i=0; i<8; i++)
		blk[i]=ivout[i];
	des_block_cipher(outks.ek1,blk,0);
	des_block_cipher(outks.ek2,blk,1);
	des_block_cipher(outks.ek3,blk,0);
	for (i=0; i<7; i++)
		ivout[i]=ivout[i+1];
	ivout[7] = blk[0] ^ c;
	return (ivout[7]);
}

/*
 * 8 bit cfb decrypt
 */
unsigned int cfb8_decrypt(c)
     unsigned int c;
{
	int i;
	unsigned char blk[8];

	for (i=0; i<8; i++)
		blk[i]=ivin[i];
	des_block_cipher(inks.ek1,blk,0);
	des_block_cipher(inks.ek2,blk,1);
	des_block_cipher(inks.ek3,blk,0);
	for (i=0; i<7; i++)
		ivin[i]=ivin[i+1];
	ivin[7] = c;
	return (blk[0] ^ c);
}

/*
 * alpha -> hex; used for encoding
 *  (ascii only - bfd)
 */
int atoh(ch)
     int ch;
{
	if (isdigit(ch))
		return ch-'0';
	if (islower(ch))
		return 10+ch-'a';
	if (isupper(ch))
		return 10+ch-'A';
	else
		return -1; /*???*/
}

     
int count=0;
int secs;
unsigned int bits;
void procbit()
{
	secs--;
	bits |= ((count ^ (count>>4)) & 0xf)<<(secs*4);
	if (secs) {
		alarm(1);
		signal(SIGALRM,procbit);
		getuid();	/* do a syscall to slow things a bit */
	}
}

int verbose=1;
/*
 * init the random number generator - seed with
 * truerandbits.
 * Use a combination of OS load and processor clock skew to get
 * enough entropy to generate the secret parameter (we only need a
 * total of 128 or so bits, so we just get more than we need to
 * compensate for any non-randomness).
 * WARNING: use oldrand() (below) on new platforms
 * if you aren't sure about truerand().
 */
randinit()
{
	unsigned int n;
	int i;
	struct timeval tv;
	unsigned long truerand();
	unsigned char b[20];
	
	R_RandomInit(&rs);
	if (verbose) {
		fprintf(stderr,"randomizing...");
		fflush(stderr);
	}
	/* we just grab 160 truerand bits and keep feeding them
	   over and over to R_RandomUpdate until it's happy */
	for (i=0; i<20; i++) {
		b[i]=randbyte();
		fprintf(stderr,".");
	}
	while (R_GetRandomBytesNeeded(&n,&rs), (n>0)) {
		R_RandomUpdate(&rs,b,sizeof(b));
	}
	if (verbose) {
		fprintf(stderr,"done\n");
		fflush(stderr);
	}
}

/*
 * OLD CODE HERE.  NOT CALLED.
 * generate 8 random clock skew bits
 * we don't use this anymore.. instead we generate more
 * bits using the mitchell code.
 * modify the code to use this instead if you don't trust
 * the higher-bandwidth mitchell numbers.
 */
int rnd8()
{
	secs=2;
	bits=0;
	signal(SIGALRM,procbit);
	alarm(1);
	getpid();
	while (secs)
		count++;
	return bits;
	
}

/*
 * OLD CODE HERE.  NOT CALLED.
 * this is the old, slow but simple truerand routine
 * you should change the code use this on new platforms instead of 
 * the "randinit" above.
 */
oldrandinit()
{
	unsigned int n;
	unsigned char b[18];
	struct timeval tv;
	
	R_RandomInit(&rs);
	R_GetRandomBytesNeeded(&n,&rs); /* but we ignore */
	/* RSAREF wants 256 bytes, which is an awful lot at 4bps.  we
	   really only need enough to do justice to the entropy of
	   the block cipher (3des), so we just generate 18. */
	   
	if (verbose)
		fprintf(stderr,"Randomizing (takes about 45 secs)...");
	fflush(stderr);
	for (n=0; n<18; n++) {
		b[n]=rnd8();
		if (verbose && (n%4==3)) {
			fprintf(stderr,".");
			fflush(stderr);
		}
	}
	while (R_GetRandomBytesNeeded(&n,&rs), (n>0)) {
		R_RandomUpdate(&rs,b,18);
		if (verbose)
			fprintf(stderr,".");
	}
	/* Just for good measure, we throw in a couple other things */
	n=getpid();
	R_RandomUpdate(&rs,(unsigned char*)&n,sizeof(n));
	gettimeofday(&tv,NULL);
	R_RandomUpdate(&rs,(unsigned char*)&tv,sizeof(tv));
	if (verbose)
		fprintf(stderr,"done\n");
}

#define BLANKLINE 0
#define TERM 1
#define NONE 2
#define SHORTKEY 3
#define MEDKEY 4
#define LONGKEY 5
#define HEX 6
#define FILENAME 7

int size[3];

int keysize=0;
	
getpubkey()
{
	fprintf(stderr,"Calculator not implemented, sorry\n");
	return;
}

createdh(param)
     int param;
{
	int e;
	
	if ((param<0) || (param>2))
		return -1;
	ourprivlen=(dhparams[param].generatorLen)/2; /* from gen'd params */
	if ((e=R_SetupDHAgreement(ourpub, ourpriv, ourprivlen,
				  &dhparams[param], &rs))!=0) {
		return -1;
	}
	return 0;
}

dhagree(param,master)
     int param;
     int master;
{
	unsigned char buf[MAXPUBKEY];
	int i;
	
	if ((param<0) || (param>2))
		return -1;
	if (R_ComputeDHAgreedKey(buf,otherpub,ourpriv,ourprivlen,
				 &dhparams[param]) != 0)
		return -1;
	for (i=0; i<8; i++) {	/* always have enough agreed bits */
		mkey1[i] = buf[i];
		mkey2[i] = buf[i+8];
		mkey3[i] = buf[i+16];
		skey1[i] = buf[24+i];
		skey2[i] = buf[24+i+8];
		skey3[i] = buf[24+i+16];
		check[i] = buf[32+i];
	}
	cipherinit(master);
	return 0;
}
	    
     
delkey()
{
}

