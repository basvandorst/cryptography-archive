/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*
 * DSA - module for DSA key generation
 * 
 * dsa_gen_key()
 */

#include "secure.h"
#include <stdio.h>
#include "arithmetic.h"
#include "dsa.h"

#define InitDSA { if(sec_verbose) {fprintf(stderr,"DSA key generation"); fflush(stderr);} }
#define PrintRabinstest { if(sec_verbose) {fprintf(stderr,"\b\b\b\b%4d", cnt++); fflush(stderr);} }
#define EndGeneration { if(sec_verbose) {fprintf(stderr,"\n"); fflush(stderr);} }
#define Sorry { if(sec_verbose) {fprintf(stderr,"\nSorry again"); fflush(stderr);} }
#define StartGeneration(s) { if(sec_verbose) {cnt=0;fprintf(stderr,"\nGenerating "); fprintf(stderr,s); fflush(stderr);} }

/*----------------------------------------------dsa_gen_key---*/


/***************************************************************
 *
 * Procedure hashln
 *
 ***************************************************************/
#ifdef __STDC__

static void hashln(
	L_NUMBER	  ln[],
	OctetString	 *hash
)

#else

static void hashln(
	ln,
	hash
)
L_NUMBER	  ln[];
OctetString	 *hash;

#endif

{
	OctetString hashint;
	char hashoctets[SEEDOCTETS];

	hashint.octets = hashoctets;

	lntoINTEGER(ln, &hashint);

	while(hashint.noctets<SEEDOCTETS) hashoctets[hashint.noctets++] = 0;
	hashint.noctets = SEEDOCTETS;

	sha_hash(&hashint, hash, SEC_END);
}
/***************************************************************
 *
 * Procedure xorhash
 *
 ***************************************************************/
#ifdef __STDC__

static void xorhash(
	OctetString	 *hash1,
	OctetString	 *hash2
)

#else

static void xorhash(
	hash1,
	hash2
)
OctetString	 *hash1;
OctetString	 *hash2;

#endif

{
	int i;

	for(i = 0; i<HASHOCTETS; i++) hash1->octets[i] = hash1->octets[i] ^ hash2->octets[i];

}
/***************************************************************
 *
 * Procedure addvaltoln
 *
 ***************************************************************/
#ifdef __STDC__

static void addvaltoln(
	L_NUMBER	  ln1[],
	unsigned int	  value,
	L_NUMBER	  ln2[]
)

#else

static void addvaltoln(
	ln1,
	value,
	ln2
)
L_NUMBER	  ln1[];
unsigned int	  value;
L_NUMBER	  ln2[];

#endif

{
	L_NUMBER addln[2];
	addln[0] = 1;
	addln[1] = value;
	add(ln1, addln, ln2);
}
/***************************************************************
 *
 * Procedure fillzero
 *
 ***************************************************************/
#ifdef __STDC__

static void fillzero(
	L_NUMBER	  ln[]
)

#else

static void fillzero(
	ln
)
L_NUMBER	  ln[];

#endif

{
	while(ln[0]<HASHWORDS) ln[++ln[0]] = 0;
}
extern int primes[];    /* Die ersten 1000 Primzahlen          */
/***************************************************************
 *
 * Procedure test35711
 *
 ***************************************************************/
#ifdef __STDC__

static int test35711(
	L_NUMBER	  p[]
)

#else

static int test35711(
	p
)
L_NUMBER	  p[];

#endif

{
	unsigned int n, m3 = 0, m5 = 0;
	L_NUMBER d[2];
	L_NUMBER r[2];
	L_NUMBER t[MAXGENL];

	if(p[0] == 1 && p[1] <= 71 && ( p[1] ==  3 || p[1] ==  5 || p[1] ==  7 || p[1] == 11 ||
					p[1] == 13 || p[1] == 17 || p[1] == 19 || p[1] == 23 ||
					p[1] == 29 || p[1] == 31 || p[1] == 37 || p[1] == 41 ||
					p[1] == 43 || p[1] == 47 || p[1] == 53 || p[1] == 59 ||
					p[1] == 61 || p[1] == 67 || p[1] == 71)) return(2);

	for(n = 1; n<=p[0]; n++) m3 += p[n] % 3;
	
	if(!(m3 % 3)) return(0);

	for(n = 1; n<=p[0]; n++) m5 += p[n] % 5;

	if(!(m5 % 5)) return(0);

	d[0] = 1;
	d[1] = 7*11*13*17*19*23*29;
	div(p,d,t,r);

	for(n=2;n<9; n++) 
		if(!(r[1] % primes[n])) return(0);
	


	d[1] = 31*37*41*43*47;
	div(p,d,t,r);

	for(n=9;n<14; n++) 
		if(!(r[1] % primes[n])) return(0);
	


	d[1] = 53*59*61*67*71;
	div(p,d,t,r);

	for(n=14;n<19; n++) 
		if(!(r[1] % primes[n])) return(0);
	


	return(1);
}
/***************************************************************
 *
 * Procedure dsa_gen_key
 *
 ***************************************************************/
#ifdef __STDC__

RC dsa_gen_key(
	int		  keysize,
	BitString	**skey,
	BitString	**pkey
)

#else

RC dsa_gen_key(
	keysize,
	skey,
	pkey
)
int		  keysize;
BitString	**skey;
BitString	**pkey;

#endif

{



	L_NUMBER        p[MAXGENL], q[MAXGENL], g[MAXGENL], x[MAXGENL], y[MAXGENL], tmp1[MAXGENL], tmp2[MAXGENL], tmp3[MAXGENL], seed[SEEDWORDS+2], w[MAXGENL];
	OctetString 	hash1, hash2;
	char 		hashoctets1[HASHOCTETS], hashoctets2[HASHOCTETS];
	int		k, counter, offset, cnt, words_of_w;
	unsigned int 	mask_of_last_word_of_w;

	if(keysize<MINMODULUSL || keysize>MAXMODULUSL || (keysize % MODULUSSTEPS)) return(-1);

	hash1.octets = hashoctets1;
	hash2.octets = hashoctets2;

	if(sec_dsa_predefined) {
		trans(dsa_public_part[keysize/64-8].p, p);
		trans(dsa_public_part[keysize/64-8].q, q);
		trans(dsa_public_part[keysize/64-8].g, g);
	}
	else {
	
		words_of_w = (keysize-1) / 32 + 1;
		mask_of_last_word_of_w = (1 << ((keysize-1) % 32));
	
		InitDSA;
	
		do {
			StartGeneration("q:     ");
			do {
	
	/* create a random seed */
	
				rndm(SEEDBITS, seed);
				seed[0] = SEEDWORDS;
			
	/* hash seed and seed+1 */
	
				hashln(seed, &hash1);
			
				addvaltoln(seed, 1, tmp1);
	
				hashln(tmp1, &hash2);
		
	/* xor both hash results */
				xorhash(&hash1, &hash2);
	
	/* set high an low bit such that it represent an odd integer between 2^159 and 2^160 */
	
				hash1.octets[0] |=  0x01;
				hash1.octets[HASHOCTETS-1] |=  0x80;
		
				INTEGERtoln(&hash1, q);
	
				PrintRabinstest;
	
	/* repeat this loop if p is not prime */
		
			} while (!test35711(q) || (rabinstest(q) < 0));
	
		StartGeneration("p:     ");
		counter = 0;
		offset = 2;
			do {
	
	/* create a random number w with keysize bits combining the hashs of offsets to seed*/
		
				for(k = (keysize-1) / (HASHBITS); k>=0; k--) {
					addvaltoln(seed, offset + k, tmp1);
					hashln(tmp1, &hash1);
	
					INTEGERtoln(&hash1, &w[k*HASHWORDS]);
					fillzero(&w[k*HASHWORDS]);
	
			
				}
				w[words_of_w] = (w[words_of_w] & (mask_of_last_word_of_w - 1)) | mask_of_last_word_of_w;
	
				w[0] = words_of_w;
	
	/* for this w calculate the highest number p<=w+1, such that 2q divides (p-1)  */
	 
				add(q, q, tmp1);		/* tmp1 = 2q */
				div(w, tmp1, tmp2, tmp2);	/* tmp2 = w mod tmp1 = w mod (2q) */
			
				sub(w, tmp2, tmp1);		/* tmp1 = w - tmp2 = w - (w mod (2q))  --- w rounded to be dividable by 2q  */
				add(tmp1, lz_eins, p);		/* p = tmp1 + 1 = w + 1 - (w mod (2q)) --- 2q divides (p-1) */
			
				PrintRabinstest;
	
	/* if p fits the keysize then test if p is prime */
		
				if(p[0]>words_of_w || (p[0] == words_of_w && p[words_of_w] >= mask_of_last_word_of_w)) 
					if(test35711(p) && rabinstest(p)>0) goto found;
			
				counter++;
				offset += (keysize-1) / (HASHBITS) + 1;
		
			} while(counter < 4096);
		Sorry;
		} while(TRUE);
	
	
	found:
		StartGeneration("g");
	
	/* create random g with g^q = 1 mod p */
	
		sub(p, lz_eins, tmp1);		/* tmp1 = p-1 */
		div(tmp1, q, tmp2, tmp1);	/* tmp2 = tmp1 DIV q = (p-1) DIV q */
		rndm(p[0]*32, tmp3);		
		mexp(tmp3, tmp2, g, p);		/* g = rnd ^ ((p-1) DIV q) MOD p */
	}

	
	StartGeneration("x");

/* create random x  */

	rndm(SEEDBITS, x);
	x[0] = SEEDWORDS;

	hashln(x, &hash1);
	INTEGERtoln(&hash1, x);

	bzero(hash1.octets, SEEDOCTETS);

	StartGeneration("y");

/* create y = g^x MOD p  */

	mexp(g, x, y, p);

	EndGeneration;

	{       /* return generated key pair */
	KeyBits         Kbits;
	char    k1[MAXGENL*WBYTES], k2[MAXGENL*WBYTES], k3[MAXGENL*WBYTES], k4[MAXGENL*WBYTES];

	Kbits.part1.octets = k1;
	Kbits.part2.octets = k2;
	Kbits.part3.octets = k3;
	Kbits.part4.octets = k4;

	lntoINTEGER(x,&Kbits.part1);

	if(sec_dsa_predefined) {
		Kbits.part2.noctets = 0;
		Kbits.part3.noctets = 0;
		Kbits.part4.noctets = 0;
	}
	else {	
		lntoINTEGER(p,&Kbits.part2);
		lntoINTEGER(q,&Kbits.part3);
		lntoINTEGER(g,&Kbits.part4);
	}
	*skey = e_KeyBits(&Kbits);

	/* ERASE SECRET KEY */
	bzero(x,MAXGENL*WBYTES);
	bzero(k1,MAXGENL*WBYTES);


	lntoINTEGER(y,&Kbits.part1);

	*pkey = e_KeyBits(&Kbits);
	}





	return 0;
}
