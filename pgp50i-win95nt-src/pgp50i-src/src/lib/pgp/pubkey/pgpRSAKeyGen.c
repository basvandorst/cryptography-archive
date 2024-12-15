/*
* pgpRSAKeyGen.c
*
* Generates an RSA Key Pair. Broken out from pgpRSAKey.c BAT 6/4/97
* The breakout is so that we can elect not to ship this in the source.
*
* $Id: pgpRSAKeyGen.c,v 1.1.2.1 1997/06/09 23:46:38 quark Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if (!NO_RSA && !NO_RSA_KEYGEN)

#include "pgpDebug.h"
#include "pgpKeyMisc.h"
#include "pgpESK.h"
#include "pgpRSAKey.h"
#include "pgpRSAGlue.h"
#include "bn.h"
#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "bnprime.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"

#define RSA_DEFAULT_EXPONENT	17

/*
* PgpRandomContext to use for primeGen callback. We really should enhance
* primeGen to pass an arg parameter along with the limit value.
*/
static struct PgpRandomContext const *staticrc;

/* Random callback for primeGen */
static unsigned randcallback(unsigned limit)
{
		return pgpRandomRange(staticrc, limit);
}

/*
* Generate an RSA secret key with modulus of the specified number of bits.
* We choose public exponent from the #define value above.
* The high two bits of each prime are always
* set to make the number more difficult to factor by forcing the
* number into the high end of the range.
* Make callbacks to progress function periodically.
* Secret key is returned in the unlocked form, with no passphrase set.
* fastgen is an unused flag which is used by the discrete log keygens to
* allow use of canned primes.
*/
struct PgpSecKey *
rsaSecGenerate(unsigned bits, Boolean fastgen,
		struct PgpRandomContext const *rc,
		int progress(void *arg, int c), void *arg, int *error)
{
		struct PgpSecKey *seckey;
		struct RSAsecPlus *sec;
		struct BigNum t;			/* temporary */
		unsigned ent;	 			/* Entropy */
		int i;
		int exp = RSA_DEFAULT_EXPONENT;

		(void) fastgen;

		*error = 0;

		/* Initialize local pointers (simplify cleanup below) */
		seckey = NULL;
		sec = NULL;
		bnBegin(&t);

		/* Allocate data structures */
		seckey = (struct PgpSecKey *)pgpMemAlloc(sizeof(*seckey));
		if (!seckey)
			goto memerror;
		sec = (struct RSAsecPlus *)pgpMemAlloc(sizeof(*sec));
		if (!sec)
			goto memerror;
	
		bnBegin(&sec->s.n);
		bnBegin(&sec->s.e);
		bnBegin(&sec->s.d);
		bnBegin(&sec->s.p);
		bnBegin(&sec->s.q);
		bnBegin(&sec->s.u);
	
		if (bnSetQ(&sec->s.e, exp))
			goto bnerror;

		/* Find p - choose a starting place */
		if (pgpBnGenRand(&sec->s.p, rc, bits/2, 0xC0, 1, bits/2-3) < 0)
			goto bnerror;

		/* And search for a prime */
		staticrc = rc;
		i = bnPrimeGen(&sec->s.p, randcallback, progress, arg, exp, 0);
		if (i < 0)
			goto bnerror;
		pgpAssert(bnModQ(&sec->s.p, exp) != 1);

		/* Make sure p and q aren't too close together */

		/* Bits of entropy needed to generate q. */
		ent = (bits+1)/2 - 3;
		/* Pick random q until we get one not too close to p */
		do {
			/* Visual separator between the two progress indicators */
			if (progress)
				progress(arg, ' ');
					if (pgpBnGenRand(&sec->s.q, rc, (bits+1)/2, 0xC0, 1, ent) < 0)
						goto bnerror;
					ent = 0;	/* No entropy charge next time around */
					if (bnCopy(&sec->s.n, &sec->s.q) < 0)
						goto bnerror;
					if (bnSub(&sec->s.n, &sec->s.p) < 0)
						goto bnerror;
					/* Note that bnSub(a,b) returns abs(a-b) */
			} while (bnBits(&sec->s.n) < bits/2-5);

			i = bnPrimeGen(&sec->s.q, randcallback, progress, arg, exp, 0);
			if (i < 0)
				 goto bnerror;
			pgpAssert(bnModQ(&sec->s.p, exp) != 1);

			/* Wash the random number pool. */
			pgpRandomStir(rc);

			/* Ensure that q is larger */
			if (bnCmp(&sec->s.p, &sec->s.q) > 0)
				 bnSwap(&sec->s.p, &sec->s.q);

			/*
			* Now we compute d,
			* the decryption exponent, from the encryption exponent.
			*/

			/* Decrement q temporarily */
			(void)bnSubQ(&sec->s.q, 1);
			/* And u = p-1, to be divided by gcd(p-1,q-1) */
			if (bnCopy(&sec->s.u, &sec->s.p) < 0)
				 goto bnerror;
			(void)bnSubQ(&sec->s.u, 1);

			/* Use t to store gcd(p-1,q-1) */
			if (bnGcd(&t, &sec->s.q, &sec->s.u) < 0) {
				 goto bnerror;
			}

			/* Let d = (p-1) / gcd(p-1,q-1) (n is scratch for the remainder) */
			i = bnDivMod(&sec->s.d, &sec->s.n, &sec->s.u, &t);
			if (i < 0)
				 goto bnerror;
			pgpAssert(bnBits(&sec->s.n) == 0);

			/* Now we have q-1 and d = (p-1) / gcd(p-1,q-1) */
			/* Find the product, n = lcm(p-1,q-1) = c * d */
			if (bnMul(&sec->s.n, &sec->s.q, &sec->s.d) < 0)
				 goto bnerror;

			/* Find the inverse of the exponent mod n */
			i = bnInv(&sec->s.d, &sec->s.e, &sec->s.n);
			if (i < 0)
				 goto bnerror;
			pgpAssert(!i);	/* We should NOT get an error here */

			/*
			* Now we have the comparatively simple task of computing
			* u = p^-1 mod q.
			*/

			/* But it *would* be nice to have q back first. */
			(void)bnAddQ(&sec->s.q, 1);

			/* Now compute u = p^-1 mod q */
			i = bnInv(&sec->s.u, &sec->s.p, &sec->s.q);
			if (i < 0)
				 goto bnerror;
			pgpAssert(!i);	/* p and q had better be relatively prime! */

			/* And finally, n = p * q */
			if (bnMul(&sec->s.n, &sec->s.p, &sec->s.q) < 0)
				 goto bnerror;

			/* And that's it... success! */

			/* Fill in structs */
			sec->cryptkey = NULL;
			sec->ckalloc = sec->cklen = 0;
			sec->locked = 0;
			rsaFillSecKey(seckey, sec);

			/* Fill in cryptkey structure, unencrypted */
			rsaChangeLock (seckey, NULL, rc, NULL, 0);

			goto done;

	bnerror:
			bnEnd(&sec->s.n);
			bnEnd(&sec->s.e);
			bnEnd(&sec->s.d);
			bnEnd(&sec->s.p);
			bnEnd(&sec->s.q);
			bnEnd(&sec->s.u);
			/* Fall through */
memerror:
			pgpMemFree(seckey);
			pgpMemFree(sec);
			seckey = NULL;
			*error = PGPERR_NOMEM;
			/* Fall through */
done:
			bnEnd(&t);
			return seckey;
	}

	#endif
