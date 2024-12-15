/*
 * The author of this software is John Ioannidis, ji@cs.columbia.edu.
 * Copyright (C) 1993, by John Ioannidis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */

#include "debug.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/af.h>
#include <net/netisr.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>

#include <netinet/ip_swipe.h>
#include <netinet/if_sw.h>

static int swipe_des_inited = 0;

#define DEBUGDES
#ifdef DEBUGDES
int debugdes = 0x0000;
#endif

/*
 * Stolen and adapted from Karn's (et al) DES functions.
 * JI's optimizations included.
 */

/* Sofware DES functions
 * written 12 Dec 1986 by Phil Karn, KA9Q; large sections adapted from
 * the 1977 public-domain program by Jim Gillogly
 * Modified for additional speed - 6 December 1988 Phil Karn
 * Modified for parameterized key schedules - Jan 1991 Phil Karn
 * Callers now allocate a key schedule as follows:
 *	kn = (char (*)[8])malloc(sizeof(char) * 8 * 16);
 *	or
 *	char kn[16][8];
 */
#ifndef NULL
#define	NULL	0
#endif


static void permute(),perminit(),spinit();
static long f();

/* Tables defined in the Data Encryption Standard documents */

/* initial permutation IP */
static char ip[] = {
	58, 50, 42, 34, 26, 18, 10,  2,
	60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6,
	64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1,
	59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5,
	63, 55, 47, 39, 31, 23, 15,  7
};

/* final permutation IP^-1 */
static char fp[] = {
	40,  8, 48, 16, 56, 24, 64, 32,
	39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30,
	37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28,
	35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26,
	33,  1, 41,  9, 49, 17, 57, 25
};

/* expansion operation matrix
 * This is for reference only; it is unused in the code
 * as the f() function performs it implicitly for speed
 */
#ifdef notdef
static char ei[] = {
	32,  1,  2,  3,  4,  5,
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1 
};
#endif

/* permuted choice table (key) */
static char pc1[] = {
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,

	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12,  4
};

/* number left rotations of pc1 */
static char totrot[] = {
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
};

/* permuted choice key (table) */
static char pc2[] = {
	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32
};

/* The (in)famous S-boxes */
static char si[8][64] = {
	/* S1 */
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

	/* S2 */
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

	/* S3 */
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

	/* S4 */
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

	/* S5 */
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

	/* S6 */
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

	/* S7 */
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

	/* S8 */
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

/* 32-bit permutation function P used on the output of the S-boxes */
static char p32i[] = {	
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25
};
/* End of DES-defined tables */

/* Lookup tables initialized once only at startup by desinit() */
/* (ji) Statically allocated rather than malloc'ed to make things easier */

static long sp[8][64];		/* Combined S and P boxes */

static char iperm[16][16][8];	/* Initial and final permutations */
static char fperm[16][16][8];


/* bit 0 is left-most in byte */
static int bytebit[] = {
	0200,0100,040,020,010,04,02,01
};

static int nibblebit[] = {
	 010,04,02,01
};


/* In-place encryption of 64-bit block */
int
endes(kn,block)
char (*kn)[8];		/* Key schedule */
char *block;
{
	register long left,right;
	register char *knp;
	long work[2]; 		/* Working data storage */

	if(kn == NULL || block == NULL)
		return -1;
	ipermute(block,(char *)work);	/* Initial Permutation */

	left = htonl(work[0]);
	right = htonl(work[1]);

	/* Do the 16 rounds.
	 * The rounds are numbered from 0 to 15. On even rounds
	 * the right half is fed to f() and the result exclusive-ORs
	 * the left half; on odd rounds the reverse is done.
	 */
	knp = &kn[0][0];
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);
	knp += 8;
	left ^= f(right,knp);
	knp += 8;
	right ^= f(left,knp);

	/* Left/right half swap, plus byte swap if little-endian */

	work[1] = htonl(left);
	work[0] = htonl(right);

	fpermute((char *)work,block);	/* Inverse initial permutation */
	return 0;
}
/* In-place decryption of 64-bit block. This function is the mirror
 * image of encryption; exactly the same steps are taken, but in
 * reverse order
 */
int
dedes(kn,block)
char (*kn)[8];		/* Key schedule */
char *block;
{
	register long left,right;
	register char *knp;
	long work[2];	/* Working data storage */

	if(kn == NULL || block == NULL)
		return -1;
	ipermute(block,(char *)work);	/* Initial permutation */

	/* Left/right half swap, plus byte swap if little-endian */

	right = htonl(work[0]);
	left = htonl(work[1]);

	/* Do the 16 rounds in reverse order.
	 * The rounds are numbered from 15 to 0. On even rounds
	 * the right half is fed to f() and the result exclusive-ORs
	 * the left half; on odd rounds the reverse is done.
	 */
	knp = &kn[15][0];
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);
	knp -= 8;
	right ^= f(left,knp);
	knp -= 8;
	left ^= f(right,knp);

	work[0] = htonl(left);
	work[1] = htonl(right);

	fpermute((char *)work,block);	/* Inverse initial permutation */
	return 0;
}


ipermute(inblock, outblock)
char *inblock;
long *outblock;				/* result into outblock,64 bits */
{
	register char *ib;
	register long  *ob;		/* ptr to input or output block */
	register long *p, *q;
	register unsigned char c;	/* no need to mask when shifting */

	/* Clear output block */
	outblock[0] = 0;
	outblock[1] = 0;

	ib = inblock;

	c = *ib++;
	ob = outblock;	
	p = (long *)iperm[0][(c >> 4)];
	q = (long *)iperm[1][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[2][(c >> 4)];
	q = (long *)iperm[3][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[4][(c >> 4)];
	q = (long *)iperm[5][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[6][(c >> 4)];
	q = (long *)iperm[7][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[8][(c >> 4)];
	q = (long *)iperm[9][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[10][(c >> 4)];
	q = (long *)iperm[11][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[12][(c >> 4)];
	q = (long *)iperm[13][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)iperm[14][(c >> 4)];
	q = (long *)iperm[15][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

}

fpermute(inblock, outblock)
char *inblock;
long *outblock;				/* result into outblock,64 bits */
{
	register char *ib;
	register long  *ob;		/* ptr to input or output block */
	register long *p, *q;
	register unsigned char c;	/* no need to mask when shifting */

	/* Clear output block */
	outblock[0] = 0;
	outblock[1] = 0;

	ib = inblock;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[0][(c >> 4)];
	q = (long *)fperm[1][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[2][(c >> 4)];
	q = (long *)fperm[3][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[4][(c >> 4)];
	q = (long *)fperm[5][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[6][(c >> 4)];
	q = (long *)fperm[7][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[8][(c >> 4)];
	q = (long *)fperm[9][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[10][(c >> 4)];
	q = (long *)fperm[11][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[12][(c >> 4)];
	q = (long *)fperm[13][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

	c = *ib++;
	ob = outblock;
	p = (long *)fperm[14][(c >> 4)];
	q = (long *)fperm[15][c & 0xf];
	*ob++ |= *p++ | *q++;
	*ob++ |= *p++ | *q++;

}

/* The nonlinear function f(r,k), the heart of DES */
static long
f(r,subkey)
register long r;	/* 32 bits */
register char *subkey;	/* 48-bit key for this round */
{
	register long *spp;
	register long rval,rt;
	register int er;

#ifdef	TRACE
	printf("f(%08lx, %02x %02x %02x %02x %02x %02x %02x %02x) = ",
		r,
		subkey[0], subkey[1], subkey[2],
		subkey[3], subkey[4], subkey[5],
		subkey[6], subkey[7]);
#endif
	/* Run E(R) ^ K through the combined S & P boxes.
	 * This code takes advantage of a convenient regularity in
	 * E, namely that each group of 6 bits in E(R) feeding
	 * a single S-box is a contiguous segment of R.
	 */
	subkey += 7;

	/* Compute E(R) for each block of 6 bits, and run thru boxes */
	er = ((int)r << 1) | ((r & 0x80000000) ? 1 : 0);
	spp = &sp[7][0];
	rval = spp[(er ^ *subkey--) & 0x3f];
	spp -= 64;
	rt = (unsigned long)r >> 3;
	rval |= spp[((int)rt ^ *subkey--) & 0x3f];
	spp -= 64;
	rt >>= 4;
	rval |= spp[((int)rt ^ *subkey--) & 0x3f];
	spp -= 64;
	rt >>= 4;
	rval |= spp[((int)rt ^ *subkey--) & 0x3f];
	spp -= 64;
	rt >>= 4;
	rval |= spp[((int)rt ^ *subkey--) & 0x3f];
	spp -= 64;
	rt >>= 4;
	rval |= spp[((int)rt ^ *subkey--) & 0x3f];
	spp -= 64;
	rt >>= 4;
	rval |= spp[((int)rt ^ *subkey--) & 0x3f];
	spp -= 64;
	rt >>= 4;
	rt |= (r & 1) << 5;
	rval |= spp[((int)rt ^ *subkey) & 0x3f];
#ifdef	TRACE
	printf(" %08lx\n",rval);
#endif
	return rval;
}
/* initialize a perm array */
static void
perminit(perm,p)
char perm[16][16][8];			/* 64-bit, either init or final */
char p[64];
{
	register int l, j, k;
	int i,m;

	/* Clear the permutation array */
	bzero((char *)perm,16*16*8);

	for (i=0; i<16; i++)		/* each input nibble position */
		for (j = 0; j < 16; j++)/* each possible input nibble */
		for (k = 0; k < 64; k++)/* each output bit position */
		{   l = p[k] - 1;	/* where does this bit come from*/
			if ((l >> 2) != i)  /* does it come from input posn?*/
			continue;	/* if not, bit k is 0	 */
			if (!(j & nibblebit[l & 3]))
			continue;	/* any such bit in input? */
			m = k & 07;	/* which bit is this in the byte*/
			perm[i][j][k>>3] |= bytebit[m];
		}
}

/* Initialize the lookup table for the combined S and P boxes */
static void
spinit()
{
	char pbox[32];
	int p,i,s,j,rowcol;
	long val;

	/* Compute pbox, the inverse of p32i.
	 * This is easier to work with
	 */
	for(p=0;p<32;p++){
		for(i=0;i<32;i++){
			if(p32i[i]-1 == p){
				pbox[p] = i;
				break;
			}
		}
	}
	for(s = 0; s < 8; s++){			/* For each S-box */
		for(i=0; i<64; i++){		/* For each possible input */
			val = 0;
			/* The row number is formed from the first and last
			 * bits; the column number is from the middle 4
			 */
			rowcol = (i & 32) | ((i & 1) ? 16 : 0) | ((i >> 1) & 0xf);
			for(j=0;j<4;j++){	/* For each output bit */
				if(si[s][rowcol] & (8 >> j)){
				 val |= 1L << (31 - pbox[4*s + j]);
				}
			}
			sp[s][i] = val;

#ifdef	DEBUG
			printf("sp[%d][%2d] = %08lx\n",s,i,sp[s][i]);
#endif
		}
	}
}

/*
 * Now for the real swIPe code
 */

swipe_des_init()
{
	spinit();
	perminit(iperm, ip);
	perminit(fperm, fp);
	swipe_des_inited = 1;
	return 0;
}

/*
 * swipe_des_setkey, adapted from Karn's setkey() 
 */

swipe_des_setkey(key, kn)
char *key;		/* 64 bits (will use only 56) */
char (*kn)[8];		/* Key schedule */
{
	char pc1m[56];		/* place to modify pc1 into */
	char pcr[56];		/* place to rotate pc1 into */
	register int i,j,l;
	int m;

	if (!swipe_des_inited)
	  swipe_des_init();
	
	if(kn == NULL){
		return -1;
	}

	/* Clear key schedule */
	bzero((char *)kn,16*8);

	for (j=0; j<56; j++) {		/* convert pc1 to bits of key */
		l=pc1[j]-1;		/* integer bit location	 */
		m = l & 07;		/* find bit		 */
		pc1m[j]=(key[l>>3] &	/* find which key byte l is in */
			bytebit[m])	/* and which bit of that byte */
			? 1 : 0;	/* and store 1-bit result */
	}
	for (i=0; i<16; i++) {		/* key chunk for each iteration */
		for (j=0; j<56; j++)	/* rotate pc1 the right amount */
			pcr[j] = pc1m[(l=j+totrot[i])<(j<28? 28 : 56) ? l: l-28];
			/* rotate left and right halves independently */
		for (j=0; j<48; j++){	/* select bits individually */
			/* check bit that goes to kn[j] */
			if (pcr[pc2[j]-1]){
				/* mask it in if it's there */
				l= j % 6;
				kn[i][j/6] |= bytebit[l] >> 2;
			}
		}
	}
	return 0;
}

swipe_encrypt(m0, sk, len)
struct mbuf *m0;
struct swipe_key *sk;
int len;

{
	struct mbuf *m = m0;
	struct mbuf *m1;
	register caddr_t	block;
	int ndone, ntmp, ntmp2;
	int padding = 0;
	int ncvrd = 0;
	union
	{
		u_long Ltmp[2];
		u_char Ctmp[8];
	}T;
#define tmp	T.Ctmp
#define tmpl	T.Ltmp
	
	union
	{
		u_long Liv[2];
		u_char Civ[8];
	}I;
#define iv	I.Civ
#define ivl	I.Liv	
	

	caddr_t keysched = sk->sk_keysched;
	
#ifdef DEBUGDES
	if (debugdes & 1)
	  printf("swipe_encrypt: crypting %d bytes\n", len);
#endif	
	
	/*
	 * m0 is the first mbuf in the chain, and the offset is
	 * pointint at the beginning of the swipe header. We
	 * do not include the first four octets (the header header)
	 * in the encryption, but we include everything else.
	 *
	 * The encryption mode is CBC, with a twist; if the
	 * data are not a multiple of 8, the last packet is 
	 * encrypted in sort-of CFB, by re-encrypting the penultimate
	 * block and xoring. 
	 */

	ivl[0] = ivl[1] = 0;
	
	block = mtod(m, caddr_t);
	
	if (m->m_len < 4)
	{
		printf("swipe_encrypt: header too short!!!\n");
		return -1;
	}

	block += 4;
	ndone = 4;
	ncvrd = 4;

	/*
	 * block is a pointer to the first unencrypted octet.
	 * ndone shows how many octets we've processed in this mbuf.
	 * ncvrd shows how many octets we've processed in the chain.
	 */

	while (ncvrd < len)
	{
#ifdef DEBUGDES
		if (debugdes & 8)
		  printf("swipe_encrypt: ncvrd = %d\n", ncvrd);
#endif
		if ((ntmp = m->m_len - ndone) >= 8)
		{
			/*
			 * There are at least 8 octets in this mbuf.
			 * Encrypt them in-place.
			 */
#ifdef DEBUGDES
			if (debugdes & 4)
			  printf("swipe_encrypt: block=0x%x, m->m_len = %d\n", block, m->m_len);
#endif

			if ((ntmp2 = len - ncvrd) < 8)
			{
#ifdef DEBUGDES
				if (debugdes & 0x10)
				  printf("swipe_encrypt: extra bytes in packet, (padding)\n");
#endif
				/*
				 * There are extra bytes in the mbuf
				 * that don't belong to the packet.
				 * We treat them as filler the way we
				 * would have done if we needed padding.
				 */

				padding = 8-ntmp2;
				bzero(&block[ntmp2], padding);
				block[7] = ntmp2;
			}

			if ((int)block & 3)	/* try to be smart about alignment! */
			{
				block[0] ^= iv[0]; block[1] ^= iv[1]; block[2] ^= iv[2]; block[3] ^= iv[3];
				block[4] ^= iv[4]; block[5] ^= iv[5]; block[6] ^= iv[6]; block[7] ^= iv[7];
				endes(keysched, block);
				iv[0] = *block++; iv[1] = *block++; iv[2] = *block++; iv[3] = *block++;
				iv[4] = *block++; iv[5] = *block++; iv[6] = *block++; iv[7] = *block++;
			}
			else
			{
				((u_long *)block)[0] ^= ivl[0]; ((u_long *)block)[1] ^= ivl[1];
				endes(keysched, block);
				ivl[0] = ((u_long *)block)[0];	ivl[1] = ((u_long *)block)[1];
				block+= 8;
			}
				
			ndone += 8;
			ncvrd += 8;
			
			continue;
		}
		
		/*
		 * We are here because there are fewer than 8
		 * (possibly 0) bytes in this mbuf.
		 */

#ifdef DEBUGDES
		if (debugdes & 4)
		  printf("swipe_encrypt: at boundary, ntmp = %d\n", ntmp);
#endif

		if (ntmp > 0)		/* but less than 8 */
		{
			/*
			 * We have a few octets left in this mbuf,
			 * and possibly some more in the next one
			 * (if any). We pull in up to eight octets,
			 * some (usually all) of which actually 
			 * belong to the packet. 
			 */

			bcopy(block, tmp, ntmp);
		}
		
		m1 = m;

		while ((ntmp < 8) && (m = m->m_next))
		{
			ntmp2 = min(8-ntmp, m->m_len);
			bcopy(mtod(m, caddr_t), &tmp[ntmp], ntmp2);
			ntmp += ntmp2;
#ifdef DEBUGDES
			if (debugdes & 4)
			  printf("swipe_encrypt: advancing by %d more bytes\n", ntmp2);
#endif
		}

		/*
		 * If there are extra bytes in the mbuf
		 * that don't belong to the packet,
		 * we treat them as filler the way we
		 * would have done if we needed padding.
		 */

		if ((ntmp2 = len - ncvrd) < ntmp)
		  padding = ntmp - ntmp2;
		else
		  padding = 0;

		if (ntmp < 8)
		  padding += 8-ntmp;
		
		if (padding)
		{
#ifdef DEBUGDES
			if (debugdes & 4)
			  printf("swipe_encrypt: padding by %d\n", padding);
#endif

			bzero(&tmp[8-padding], padding);
			tmp[7] = 8-padding;
		}
			
		tmpl[0] ^= ivl[0]; tmpl[1] ^= ivl[1];
		endes(keysched, tmp);
		ivl[0] = tmpl[0]; ivl[1] = tmpl[1];

		/*
		 * Let's put tmp back where it belongs
		 */

		m = m1;
		block = mtod(m, caddr_t);
		block += ndone;

		ntmp = m->m_len - ndone;
		bcopy(tmp, block, ntmp);
			
		while ((ntmp < 8) && m->m_next)
		{
			m = m->m_next;
			ntmp2 = min(8-ntmp, m->m_len);
			bcopy(&tmp[ntmp], mtod(m, caddr_t), ntmp2);
			ntmp += ntmp2;
		}
		
		ndone = ntmp2;
		block = mtod(m, caddr_t) + ndone;

		if ((ntmp2 = 8-ntmp) > 0)
		{
			if (m->m_off + m->m_len + ntmp2 >= MMAXOFF)
			{
#ifdef DEBUGDES
				if (debugdes & 4)
				  printf("swipe_encrypt: adding an mbuf\n", padding);
#endif

				MGET(m1, M_DONTWAIT, MT_DATA);
				if (!m1)
				{
					u.u_error = ENOBUFS;
					return -1;
				}
				m->m_next = m1;
				m1->m_len = ntmp2;
				m1->m_next = NULL;
				bcopy(&tmp[ntmp], mtod(m1, caddr_t), ntmp2);
			}
			else
			{
				bcopy(&tmp[ntmp], mtod(m, caddr_t)+m->m_len, ntmp2);
				m->m_len += ntmp2;
			}
		}
		
		ncvrd += 8;
	}
#ifdef DEBUGDES
	if (debugdes & 4)
	  printf("swipe_encrypt: returning %d\n", padding);
#endif

	return padding;
}

swipe_decrypt(m0, sk, len)
struct mbuf *m0;
struct swipe_key *sk;
int len;
{
	struct mbuf *m = m0;
	struct mbuf *m1;
	struct ip *ip;
	register caddr_t	block;
	int ndone, ntmp, ntmp2, ncvrd, shlen, elen;
	int padding = 0;
	
	union
	{
		u_long Ltmp[2];
		u_char Ctmp[8];
	}T;
	
	union
	{
		u_long Liv[2];
		u_char Civ[8];
	}I;

	union
	{
		u_long Livtmp[2];
		u_char Civtmp[8];
	}IT;
#define ivtmp	IT.Civtmp
#define ivtmpl	IT.Livtmp	

	caddr_t keysched = sk->sk_keysched;
	
	
	/*
	 * m0 is the first mbuf in the chain, and the offset is
	 * pointint at the beginning of the swipe header. We
	 * do not include the first four octets (the header header)
	 * in the encryption, but we include everything else
	 */

#ifdef DEBUGDES
	if (debugdes & 0x100)
	  printf("swipe_decrypt: decrypting %d bytes\n", len);
#endif	


	ivl[0] = ivl[1] = 0;
	
	block = mtod(m, caddr_t);
	
	if (m->m_len < 4)
	{
		printf("swipe_decrypt: header too short!!!\n");
		return -1;
	}

	block += 4;
	ndone = 4;
	ncvrd = 4;

	if ((len - 4) & 3)
	{
		printf("swipe_decrypt: length %d not multiple of 8 plus 4\n", len);
		return -1;
	}

	while (ncvrd < len)
	{
#ifdef DEBUGDES
		if (debugdes & 0x800)
		  printf("swipe_decrypt: ncvrd = %d\n", ncvrd);
#endif

		if ((ntmp = m->m_len - ndone) >= 8)
		{
			/*
			 * There are at least 8 octets in this mbuf.
			 * Decrypt them in-place.
			 */
#ifdef DEBUGDES
			if (debugdes & 0x400)
			  printf("swipe_decrypt: block=0x%x, m->m_len = %d\n", block, m->m_len);
#endif

			if ((ntmp2 = len - ncvrd) < 8)
			{
#ifdef DEBUGDES
				if (debugdes & 0x10)
				  printf("swipe_encrypt: extra bytes in packet, (padding)\n");
#endif
				/*
				 * There are extra bytes in the mbuf
				 * that don't belong to the packet.
				 * This shouldn't happen.
				 */
				return -1;
			}

			if ((int)block & 3)	/* try to be smart about alignment! */
			{

				ivtmp[0] = block[0]; ivtmp[1] = block[1]; 
				ivtmp[2] = block[2]; ivtmp[3] = block[3];
				ivtmp[4] = block[4]; ivtmp[5] = block[5]; 
				ivtmp[6] = block[6]; ivtmp[7] = block[7];
				dedes(keysched, block);
				*block++ ^= iv[0]; *block++ ^= iv[1];
				*block++ ^= iv[2]; *block++ ^= iv[3];
				*block++ ^= iv[4]; *block++ ^= iv[5];
				*block++ ^= iv[6]; *block++ ^= iv[7];
				ivl[0] = ivtmpl[0];
				ivl[1] = ivtmpl[1];
			}
			else
			{
				ivtmpl[0] = ((u_long *)block)[0];
				ivtmpl[1] = ((u_long *)block)[1];
				dedes(keysched, block);
				((u_long *)block)[0] ^= ivl[0];
				((u_long *)block)[1] ^= ivl[1];
				ivl[0] = ivtmpl[0];
				ivl[1] = ivtmpl[1];
				block+= 8;
			}
				
			ndone += 8;
			ncvrd += 8;
			
			continue;
		}
		
#ifdef DEBUGDES
		if (debugdes & 0x400)
		  printf("swipe_decrypt: at boundary, ntmp = %d\n", ntmp);
#endif

		if (ntmp == 0)
		  if (m = m->m_next)
		  {
			  block = mtod(m, caddr_t);
			  ndone = 0;
			  continue;
		  }
		  else
		    break;
		

		if (ntmp > 0)		/* but less than 8 */
		{
			/*
			 * We have a few octets left in this mbuf,
			 * and possibly some more in the next one
			 * (if any). We pull in up to eight octets,
			 * some (usually all) of which actually 
			 * belong to the packet. 
			 */

			bcopy(block, tmp, ntmp);
		}

		m1 = m;

		while ((ntmp < 8) && (m = m->m_next))
		{
			ntmp2 = min(8-ntmp, m->m_len);
			bcopy(mtod(m, caddr_t), &tmp[ntmp], ntmp2);
			ntmp += ntmp2;
#ifdef DEBUGDES
			if (debugdes & 0x400)
			  printf("swipe_decrypt: advancing by %d more bytes\n", ntmp2);
#endif
		}


		if (ntmp != 8)
		{
			printf("swipe_decrypt: extra bytes in packet, (ntmp=%d!=8)\n", ntmp);
			return -1;
		}
		
		ivtmpl[0] = tmpl[0]; ivtmpl[1] = tmpl[1];
		dedes(keysched, tmp);
		tmpl[0] ^= ivl[0]; tmpl[1] ^= ivl[1];
		ivl[0] = ivtmpl[0]; ivl[1] = ivtmpl[1];


		/*
		 * Let's put tmp back where it belongs
		 */

		m = m1;
		block = mtod(m, caddr_t);
		block += ndone;

		ntmp = m->m_len - ndone;
		bcopy(tmp, block, ntmp);
			
		while ((ntmp < 8) && m->m_next)
		{
			m = m->m_next;
			ntmp2 = min(8-ntmp, m->m_len);
			bcopy(&tmp[ntmp], mtod(m, caddr_t), ntmp2);
			ntmp += ntmp2;
		}

		if (ntmp != 8)
		{
			printf("swipe_decrypt: can't replace tmp in packet, (ntmp=%d!=8)\n", ntmp);
			return -1;
		}

		block = mtod(m, caddr_t);
		ndone = ntmp2;
		block += ndone;
	}
	
		
	
	/*
	 * Now for the surprise!
	 */
	
	shlen = mtod(m0, struct swipe *)->sw_hlen << 2;
	ip = (struct ip *)(mtod(m0, caddr_t) + shlen);
	elen = htons(ip->ip_len) + shlen; /* expected length */
	
#ifdef DEBUGDES
	if (debugdes & 0x2000)
	  printf("swipe_decrypt: len = %d, elen = %d\n", len, elen);
#endif

	if (elen == len)
	  return 0;
		
	if (elen > len)
	{
		printf("swipe_decrypt: received truncated packet (%d > %d)\n", elen, len);
		return -1;
	}

	/*
	 * elen is less than len because the original packet was
	 * padded. Let's make sure that it wasn't padded too much
	 */


	if ((padding = len - elen) > 7)
	{
		printf("swipe_decrypt: received packed too large\n");
		return -1;
	}
		
	for (m=m0; m; m=m->m_next)
	{
		if (elen >= m->m_len)
		{
			elen -= m->m_len;
			continue;
		}
		if ((ntmp = m->m_len - elen) < padding)
		{
			m->m_len = elen;
			m1 = m->m_next;
			if (!m1)
			{
				printf("swipe_decrypt: bad padding\n");
				return -1;
			}
			if (m1->m_len != padding - ntmp)
			{
				printf("swipe_decrypt: bad padding %d vs %d\n", m1->m_len, padding - ntmp);
				return -1;
			}
			
			if (mtod(m1, char *)[m1->m_len] != 8-padding)
			  printf("swipe_decrypt: bad padding count, will pass anyway\n");
#ifdef DEBUGDES
			if (debugdes & 0x2000)
			  printf("swipe_decrypt: padding got split; setting m->m_len = %d to %d", m->m_len, elen);
#endif

			m->m_next = NULL; /* I debugged this in my sleep (ji) */
			m_freem(m1);
			m->m_len = elen;

			return 0;
			
		}
		else
		{
			if (mtod(m, char *)[m->m_len-1] != 8-padding)
			  printf("swipe_decrypt: bad padding count 2, will pass anyway\n");
#ifdef DEBUGDES
			if (debugdes & 0x2000)
			  printf("swipe_decrypt: removing padding; setting m->m_len = %d to %d\n", m->m_len, elen);
#endif
			m->m_len = elen;
			return 0;
		}
	}
	
	return 0;

}
