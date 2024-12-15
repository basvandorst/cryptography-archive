/*	rsagen.c - C source code for RSA public-key key generation routines.
	First version 17 Mar 87

	(c) Copyright 1987 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use 
	of this software, even if the damage results from defects in this 
	software.  No warranty is expressed or implied.  

	RSA-specific routines follow.  These are the only functions that 
	are specific to the RSA public key cryptosystem.  The other
	multiprecision integer math functions may be used for non-RSA
	applications.  Without these functions that follow, the rest of 
	the software cannot perform the RSA public key algorithm.  

	The RSA public key cryptosystem is patented by the Massachusetts
	Institute of Technology (U.S. patent #4,405,829).  This patent does
	not apply outside the USA.  Public Key Partners (PKP) holds the
	exclusive commercial license to sell and sub-license the RSA public
	key cryptosystem.  The author of this software implementation of the
	RSA algorithm is providing this software for educational use only. 
	Licensing this algorithm from PKP is the responsibility of you, the
	user, not Philip Zimmermann, the author of this software.  The author
	assumes no liability for any breach of patent law resulting from the
	unlicensed use of this software by the user.
*/

#include "mpilib.h"
#include "genprime.h"
#include "rsagen.h"
/* Define symbol PSEUDORANDOM in random.h to disable truly random numbers. */
#include "random.h"

static void derive_rsakeys(unitptr n,unitptr e,unitptr d,
	unitptr p,unitptr q,unitptr u,short ebits);
	/* Given primes p and q, derive RSA key components n, e, d, and u. */

/* The following #ifdefs determine constraints on key sizes... */

#ifdef WHOLEWORD_KEY /* some modmult algorithms are faster this way */
#ifdef MERRITT_KEY
#undef MERRITT_KEY	/* ensures MERRITT_KEY is undefined */
#endif
#endif	/* WHOLEWORD_KEY */

#ifdef MERRITT_KEY	/* if using Merritt's modmult algorithm */
#ifdef WHOLEWORD_KEY
#undef WHOLEWORD_KEY	/* ensures WHOLEWORD_KEY is undefined */
#endif
#endif	/* MERRITT_KEY */


/* Define some error status returns for RSA keygen... */
#define KEYFAILED -15		/* key failed final test */


#define swap(p,q)  { unitptr t; t = p;  p = q;  q = t; }


static void derive_rsakeys(unitptr n, unitptr e, unitptr d,
	unitptr p, unitptr q, unitptr u, short ebits)
/*	Given primes p and q, derive RSA key components n, e, d, and u. 
	The global_precision must have already been set large enough for n.
	Note that p must be < q.
	Primes p and q must have been previously generated elsewhere.
	The bit precision of e will be >= ebits.  The search for a usable
	exponent e will begin with an ebits-sized number.  The recommended 
	value for ebits is 5, for efficiency's sake.  This could yield 
	an e as small as 17.
*/
{	unit F[MAX_UNIT_PRECISION];
	unitptr ptemp, qtemp, phi, G; 	/* scratchpads */

	/*	For strong prime generation only, latitude is the amount 
		which the modulus may differ from the desired bit precision.  
		It must be big enough to allow primes to be generated by 
		goodprime reasonably fast. 
	*/
#define latitude(bits) (max(min(bits/16,12),4))	/* between 4 and 12 bits */
	
	ptemp = d;	/* use d for temporary scratchpad array */
	qtemp = u;	/* use u for temporary scratchpad array */
	phi = n;	/* use n for temporary scratchpad array */
	G = F;		/* use F for both G and F */
	
	if (mp_compare(p,q) >= 0)	/* ensure that p<q for computing u */
		swap(p,q);		/* swap the pointers p and q */

	/*	phi(n) is the Euler totient function of n, or the number of
		positive integers less than n that are relatively prime to n.
		G is the number of "spare key sets" for a given modulus n. 
		The smaller G is, the better.  The smallest G can get is 2. 
	*/
	mp_move(ptemp,p); mp_move(qtemp,q);
	mp_dec(ptemp); mp_dec(qtemp);
	mp_mult(phi,ptemp,qtemp);	/*  phi(n) = (p-1)*(q-1)  */
	mp_gcd(G,ptemp,qtemp);		/*  G(n) = gcd(p-1,q-1)  */
#ifdef DEBUG
	if (countbits(G) > 12)		/* G shouldn't get really big. */
		mp_display("\007G = ",G);	/* Worry the user. */
#endif /* DEBUG */
	mp_udiv(ptemp,qtemp,phi,G);	/* F(n) = phi(n)/G(n)  */
	mp_move(F,qtemp);

	/*	We now have phi and F.  Next, compute e...
		Strictly speaking, we might get slightly faster results by
		testing all small prime e's greater than 2 until we hit a 
		good e.  But we can do just about as well by testing all 
		odd e's greater than 2.
		We could begin searching for a candidate e anywhere, perhaps
		using a random 16-bit starting point value for e, or even
		larger values.  But the most efficient value for e would be 3, 
		if it satisfied the gcd test with phi.
		Parameter ebits specifies the number of significant bits e
		should have to begin search for a workable e.
		Make e at least 2 bits long, and no longer than one bit 
		shorter than the length of phi.
	*/
	ebits = min(ebits,countbits(phi)-1);
	if (ebits==0) ebits=5;	/* default is 5 bits long */
	ebits = max(ebits,2);
	mp_init(e,0);
	mp_setbit(e,ebits-1);
	lsunit(e) |= 1;		/* set e candidate's lsb - make it odd */
	mp_dec(e);  mp_dec(e); /* precompensate for preincrements of e */
	do
	{	mp_inc(e); mp_inc(e);	/* try odd e's until we get it. */
		mp_gcd(ptemp,e,phi); /* look for e such that gcd(e,phi(n)) = 1 */
	} while (testne(ptemp,1));

	/*	Now we have e.  Next, compute d, then u, then n.
		d is the multiplicative inverse of e, mod F(n).
		u is the multiplicative inverse of p, mod q, if p<q.
		n is the public modulus p*q.
	*/
	mp_inv(d,e,F);		/* compute d such that (e*d) mod F(n) = 1 */
	mp_inv(u,p,q);			/* (p*u) mod q = 1, assuming p<q */
	mp_mult(n,p,q);	/*  n = p*q  */
	mp_burn(F);		/* burn the evidence on the stack */
}	/* derive_rsakeys */


int rsa_keygen(unitptr n, unitptr e, unitptr d,
	unitptr p, unitptr q, unitptr u, short keybits, short ebits)
/*	Generate RSA key components p, q, n, e, d, and u. 
	This routine sets the global_precision appropriate for n,
	where keybits is desired precision of modulus n.
	The precision of exponent e will be >= ebits.
	It will generate a p that is < q.
	Returns 0 for succcessful keygen, negative status otherwise.
*/
{	short pbits,qbits,separation;
	boolean too_close_together; /* TRUE iff p and q are too close */
	int status;
	int slop;

	/*	Don't let keybits get any smaller than 2 units, because	
		some parts of the math package require at least 2 units 
		for global_precision.
		Nor any smaller than the 32 bits of preblocking overhead.
		Nor any bigger than MAX_BIT_PRECISION - SLOP_BITS.
		Also, if generating "strong" primes, don't let keybits get
		any smaller than 64 bits, because of the search latitude.
	*/
	slop = max(SLOP_BITS,1); /* allow at least 1 slop bit for sign bit */
	keybits = min(keybits,(MAX_BIT_PRECISION-slop));
	keybits = max(keybits,UNITSIZE*2);
	keybits = max(keybits,32); /* minimum preblocking overhead */
#ifdef STRONGPRIMES
	keybits = max(keybits,64); /* for strong prime search latitude */
#endif	/* STRONGPRIMES */
#ifdef WHOLEWORD_KEY	/* some modmults run faster this way */
	/*	Some modmult algorithms run faster if both primes and 
		the modulus are an exact multiple of UNITSIZE bits long,
		in other words, they completely fill the most significant 
		unit.  So we will "round up" keybits to the next multiple 
		of UNITSIZE*2.
	*/
#define roundup(x,m) (((x)+(m)-1)/(m))*(m)
	keybits = roundup(keybits,UNITSIZE*2);
	if (keybits==MAX_BIT_PRECISION)	/* allow head room for sign bit */
		keybits -= UNITSIZE*2;
#endif	/* WHOLEWORD_KEY */

	set_precision(bits2units(keybits + slop));

	/*	We will need a series of truly random bits to generate the 
		primes.  We need enough random bits for keybits, plus two 
		random units for combined discarded bit losses in randombits. 
		Since we now know how many random bits we will need,
		this is the place to prefill the pool of random bits. 
	*/
	randflush();	/* ensure recycled random pool is empty */
	randaccum(keybits+2*UNITSIZE); /* get this many raw random bits ready */

	/*	separation is the minimum number bits of difference in the 
		sizes of p and q. 
	*/
#ifdef MERRITT_KEY	/* using Merritt's modmult algorithm */
	separation = 2;
#else	/* not MERRITT_KEY */
	separation = 0;
#endif	/* not MERRITT_KEY */
	pbits = (keybits-separation)/2;
	qbits = keybits - pbits;

#ifdef MERRITT_KEY
	/*	During decrypt, the primes p and q's bit length should 
		not be an exact multiple of UNITSIZE, because Merritt's 
		modmult algorithm performs slowest in that case, wasting 
		an extra unit of precision for overflow.
		Other modmult algorithms perform differently.
		Some other modmults actually performs fastest when the 
		modulus and primes p and q exactly fill the MS unit.
	*/
	{	short qtrim;
		qtrim = (qbits % UNITSIZE)+1; /* how many bits to trim from q */
		if (qtrim <= (separation/2))
			pbits += qtrim; /* allows qbits to be a bit shorter */
	}
	if ((pbits % UNITSIZE)==0)	/* inefficient to exactly fill a word */ 
		pbits -= 1;	/* one bit shorter speeds up modmult a lot. */
#endif	/* MERRITT_KEY */

	randload(pbits); /* get fresh load of raw random bits for p */
#ifdef STRONGPRIMES	/* make a good strong prime for the key */
	status = goodprime(p,pbits,pbits-latitude(pbits));
	if (status < 0) 
		return(status);	/* failed to find a suitable prime */
#else	/* just any random prime will suffice for the key */
	status = randomprime(p,pbits);
	if (status < 0) 
		return(status);	/* failed to find a random prime */
#endif	/* else not STRONGPRIMES */

	/* We now have prime p.  Now generate q such that q>p... */

	qbits = keybits - countbits(p);

#ifdef MERRITT_KEY
	if ((qbits % UNITSIZE)==0)	/* inefficient to exactly fill a word */ 
		qbits -= 1;	/* one bit shorter speeds up modmult a lot. */
#endif	/* MERRITT_KEY */

	randload(qbits); /* get fresh load of raw random bits for q */
	/*	This load of random bits will be stirred and recycled until 
		a good q is generated. */

	do	/* Generate a q until we get one that isn't too close to p. */
	{	
#ifdef STRONGPRIMES	/* make a good strong prime for the key */
		status = goodprime(q,qbits,qbits-latitude(qbits));
		if (status < 0) 
			return(status);	/* failed to find a suitable prime */
#else	/* just any random prime will suffice for the key */
		status = randomprime(q,qbits);
		if (status < 0) 
			return(status);	/* failed to find a random prime */
#endif	/* else not STRONGPRIMES */

		/* Note that at this point we can't be sure that q>p. */
		/*	See if p and q are far enough apart.  Is q-p big enough? */
		mp_move(u,q);	/* use u as scratchpad */
		mp_sub(u,p);	/* compute q-p */
		if (mp_tstminus(u))	/* p is bigger */
		{	mp_neg(u);
			too_close_together = (countbits(u) < (countbits(p)-7));
		}
		else		/* q is bigger */
			too_close_together = (countbits(u) < (countbits(q)-7));

		/* Keep trying q's until we get one far enough from p... */
	} while (too_close_together);

	/* In case sizes went awry in making p and q... */
	if (mp_compare(p,q) >= 0)	/* ensure that p<q for computing u */
	{	mp_move(u,p);
		mp_move(p,q);
		mp_move(q,u);
	}

	derive_rsakeys(n,e,d,p,q,u,ebits);
	randflush();		/* ensure recycled random pool is destroyed */

	/* Now test key just to make sure --this had better work! */
	{	unit M[MAX_UNIT_PRECISION];
		unit C[MAX_UNIT_PRECISION];
		mp_init(M,0x1234);		/* material to be signed */
		mp_init(C,0);
		status = rsa_decrypt(C,M,d,p,q,u);	/* create signature C first */
		if (status < 0)	/* modexp error? */
			return(status);	/* return error status */
		mp_init(M,0);		/* ensure test pattern M is destroyed */
		status = mp_modexp(M,C,e,n);	/* check signature C */
		if (status < 0)	/* modexp error? */
			return(status);	/* return error status */
		if (testne(M,0x1234))	/* test pattern M recovered? */
			return(KEYFAILED);	/* error return, bad key or bad math library */
	}
	return(0);	/* normal return */
}	/* rsa_keygen */

/****************** End of RSA-specific routines  *******************/

