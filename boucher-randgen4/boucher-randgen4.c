From msuinfo!uwm.edu!math.ohio-state.edu!cs.utexas.edu!not-for-mail Tue Apr  5 09:16:46 1994
Path: msuinfo!uwm.edu!math.ohio-state.edu!cs.utexas.edu!not-for-mail
From: boucher@csl.sri.com (Peter K. Boucher)
Newsgroups: sci.crypt
Subject: Modified Bosch PRNG
Date: 4 Apr 1994 18:32:14 -0500
Organization: UTexas Mail-to-News Gateway
Lines: 154
Sender: daemon@cs.utexas.edu
Message-ID: <9404042332.AA18839@redwood.csl.sri.com>
NNTP-Posting-Host: cs.utexas.edu

Did anyone describe why Bosch's PRNG is not secure yet?

The attached version is not extremely fast, but it does
very well on the statistical tests.  It was modified from
a version posted by John Taber.  Basically, the primes
are bigger.

How could one cryptanalyze it?

Peter K. Boucher
--
DISCLAIMER:  I am solely responsible for the contents of this message,
	     which should not be misconstrued as being in any way related
	     to the opinions of my employer.

============================= CLIP CLIP =================================
#define N0 467  /* These  */
#define N1 719  /* should */
#define N2 941  /* be     */
#define N3 1129 /* primes */

#define vSIZE (N0+N1+N2+N3)
#define iSIZE (1+(vSIZE/4))

#define RND_BYTE \
	(*rotor0 + *rotor1 + *rotor2 + *rotor3)

#define UPDATE_PTRS \
	if (++rotor0 >= &(prng.rotor.R0[N0])) rotor0 = prng.rotor.R0; \
	if (++rotor1 >= &(prng.rotor.R1[N1])) rotor1 = prng.rotor.R1; \
	if (++rotor2 >= &(prng.rotor.R2[N2])) rotor2 = prng.rotor.R2; \
	if (++rotor3 >= &(prng.rotor.R3[N3])) rotor3 = prng.rotor.R3; \
	if (++vector >= &(prng.v[vSIZE])) vector = prng.v;

static union
{
   unsigned char v[vSIZE];		/* vector of vSIZE random bytes	*/
   unsigned long i[iSIZE];		/* vector of iSIZE random longs */
   struct
     {
       unsigned char R0[N0],		/* ..redefined as rotors	*/
		     R1[N1],
		     R2[N2],
		     R3[N3];
     } rotor;
} prng; /* end union */

static unsigned char *rotor0, *rotor1, *rotor2, *rotor3, *vector;

unsigned long rand32()
{
#define BARREL_a ((((unsigned char)a)<<1)^(((unsigned char)a)>>7))
   register unsigned long a;

	a = RND_BYTE;
	*vector += BARREL_a;
	UPDATE_PTRS;

	a = (a<<8) + RND_BYTE;
	*vector += BARREL_a;
	UPDATE_PTRS;

	a = (a<<8) + RND_BYTE;
	*vector += BARREL_a;
	UPDATE_PTRS;

	a = (a<<8) + RND_BYTE;
	*vector += BARREL_a;
	UPDATE_PTRS;

	return( a );
}


void
srand128(seed1,seed2,seed3,seed4)
unsigned long seed1,seed2,seed3,seed4;
{
#define LC1(x) (x * 66049 + 3907864577)
#define LC2(x) (x * 69069)

    register int i,j;
    register unsigned long rdm_val=0L;
    register unsigned long x=seed1;
    register unsigned long y=seed2;

    while ((x&1) == 0) {
	x = LC1(x);
    }
    for (i=iSIZE; i--;) {
        for (j=32;j--;) {
	    if (y == 0) y = LC1(x);
	    y ^= (y>>15);
	    y ^= (y<<17);
	    rdm_val = (rdm_val>>1) | (0x80000000 & ((x=LC2(x))+y));
        }
	prng.i[i] = rdm_val;
    }

    x=seed3;
    y=seed4;
    while ((x&1) == 0) {
	x = LC1(x);
    }
    for (i=32;i--;) {
	if (y == 0) y = LC1(x);
	y ^= (y>>15);
	y ^= (y<<17);
	rdm_val = (rdm_val<<1) | (((x=LC2(x))+y)>>31);
    }
    rotor0 = &(prng.rotor.R0[rdm_val%N0]);

    for (i=32;i--;) {
	if (y == 0) y = LC1(x);
	y ^= (y>>15);
	y ^= (y<<17);
	rdm_val = (rdm_val<<1) | (((x=LC2(x))+y)>>31);
    }
    rotor1 = &(prng.rotor.R1[rdm_val%N1]);

    for (i=32;i--;) {
	if (y == 0) y = LC1(x);
	y ^= (y>>15);
	y ^= (y<<17);
	rdm_val = (rdm_val<<1) | (((x=LC2(x))+y)>>31);
    }
    rotor2 = &(prng.rotor.R2[rdm_val%N2]);

    for (i=32;i--;) {
	if (y == 0) y = LC1(x);
	y ^= (y>>15);
	y ^= (y<<17);
	rdm_val = (rdm_val<<1) | (((x=LC2(x))+y)>>31);
    }
    rotor3 = &(prng.rotor.R3[rdm_val%N3]);

    for (i=32;i--;) {
	if (y == 0) y = LC1(x);
	y ^= (y>>15);
	y ^= (y<<17);
	rdm_val = (rdm_val<<1) | (((x=LC2(x))+y)>>31);
    }
    vector = &(prng.v[rdm_val%vSIZE]);

    for (i=0; i<iSIZE; i++) {
        for (j=32;j--;) {
	    if (y == 0) y = LC1(x);
	    y ^= (y>>15);
	    y ^= (y<<17);
	    rdm_val = (rdm_val<<1) | (((x=LC2(x))+y)>>31);
        }
	prng.i[i] ^= rdm_val;
    }
}

