/*
* pgpRSAGlue.c - The interface between bignum math and RSA operations.
* This layer's primary reason for existence is to allow adaptation
* to other RSA math libraries for legal reasons.
*
* Written by Colin Plumb.
*
* Broken into two files by BAT. pgpRSAGlue1.c contains encryption and
* signing functions. pgpRSAGlue2.c contains decryption and verification
* functions. This was done for legal reasons.
*
* $Id: pgpRSAGlue2.c,v 1.1.2.1 1997/06/09 23:46:33 quark Exp $
*/

/*
* An alternative version of this module is used if RSAREF is needed. This
* entire source file is under the control of the following conditional:
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if (!NO_RSA && !USE_RSAREF_FULL)

#include "pgpESK.h"
#include "pgpRSAKey.h"
#include "pgpRSAGlue.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "pgpErr.h"
#include "pgpRndom.h"
#include "pgpUsuals.h"

/*
* This is handy for debugging, but VERY VERY DANGEROUS for production -
* it nicely prints every detail of your secret key!
*/
#define BNDEBUG 0

#if BNDEBUG
/* Some debugging hooks which have been left in for now. */
#include "bnprint.h"
#define bndPut(prompt, bn) (fputs(prompt, stdout), bnPrint(stdout, bn), \
				putchar('\n'))
#define bndPrintf printf
#else
#define bndPut(prompt, bn) ((void)(prompt),(void)(bn))
#if __GNUC__ > 1
/* Non-ANSI, but supresses warnings about expressions with no effect */
#define bndPrintf(arg...) (void)0
#else
/* ANSI-compliant - cast entire comma expression to void */
#define bndPrintf (void)
#endif
#endif


/*
* If you're using a legally encumbered library (ugh!) this will be
* printed in the program banner.
*/
char const banner_legalese[] = "";
/*
* This returns TRUE if the key is too big, returning the
* maximum number of bits that the library can accept. It
* is used if you want to use something icky from RSADSI, whose
* code is known to have satatic limits on key sizes. (BSAFE 2.1
* advertises 2048-bit key sizes. It lies. It's talking about
* conventional RC4 keys, whicah are useless to make anything like
* that large. RSA keys are limited to 1024 bits.
*/
int
rsaKeyTooBig(struct RSApub const *pub, struct RSAsec const *sec)
{
	(void)pub;
	(void)sec;
	return 0;	 /* Never too big! */
}


/*
* This performs a modular exponentiation using the Chinese Remainder
* Algorithm when the modulus is known to have two relatively prime
* factors n = p * q, and u = p^-1 (mod q) has been precomputed.
*
* The chinese remainder algorithm lets a computation mod n be performed
* mod p and mod q, and the results combined. Since it takes
* (considerably) more than twice as long to perform modular exponentiation
* mod n as it does to perform it mod p and mod q, time is saved.
*
* If x is the desired result, let xp and xq be the values of x mod p
* and mod q, respectively. Obviously, x = xp + p * k for some k.
* Taking this mod q, xq == xp + p*k (mod q), so p*k == xq-xp (mod q)
* and k == p^-1 * (xq-xp) (mod q), so k = u * (xq-xp mod q) mod q.
* After that, x = xp + p * k.
*
* Another savings comes from reducing the exponent d modulo phi(p)
* and phi(q). Here, we assume that p and q are prime, so phi(p) = p-1
* and phi(q) = q-1.
*/
int
bnExpModCRA(struct BigNum *x, struct BigNum const *d,
	struct BigNum const *p, struct BigNum const *q, struct BigNum const *u)
{
	struct BigNum xp, xq, k;
	int i;

bndPrintf("Performing Chinese Remainder Algorithm\n");
bndPut("x = ", x);
bndPut("p = ", p);
bndPut("q = ", q);
bndPut("d = ", d);
bndPut("u = ", u);

	bnBegin(&xp);
	bnBegin(&xq);
	bnBegin(&k);

	/* Compute xp = (x mod p) ^ (d mod p-1) mod p */
	if (bnCopy(&xp, p) < 0)		/* First, use xp to hold p-1 */
		goto fail;
	(void)bnSubQ(&xp, 1);	 	/* p > 1, so subtracting is safe. */
	if (bnMod(&k, d, &xp) < 0)	/* k = d mod (p-1) */
		goto fail;
bndPut("d mod p-1 = ", &k);
	if (bnMod(&xp, x, p) < 0)	 /* Now xp = (x mod p) */
		goto fail;
bndPut("x mod p = ", &xp);
	if (bnExpMod(&xp, &xp, &k, p) < 0)	/* xp = (x mod p)^k mod p */
		goto fail;
bndPut("xp = x^d mod p = ", &xp);

	/* Compute xq = (x mod q) ^ (d mod q-1) mod q */
	if (bnCopy(&xq, q) < 0)		/* First, use xq to hold q-1 */
		goto fail;
	(void)bnSubQ(&xq, 1);	 	/* q > 1, so subtracting is safe. */
	if (bnMod(&k, d, &xq) < 0)	/* k = d mod (q-1) */
		goto fail;
bndPut("d mod q-1 = ", &k);
	if (bnMod(&xq, x, q) < 0)	 /* Now xq = (x mod q) */
		goto fail;
bndPut("x mod q = ", &xq);
	if (bnExpMod(&xq, &xq, &k, q) < 0)	/* xq = (x mod q)^k mod q */
		goto fail;
bndPut("xq = x^d mod q = ", &xq);

	/* xp < p and PGP has p < q, so this is a no-op, but just in case */
	if (bnMod(&k, &xp, q) < 0)
		goto fail;	
bndPut("xp mod q = ", &k);
	
	i = bnSub(&xq, &k);
bndPut("xq - xp = ", &xq);
bndPrintf("With sign %d\n", i);
	if (i < 0)
		goto fail;
	if (i) {
			/*
		* Borrow out - xq-xp is negative, so bnSub returned
		* xp-xq instead, the negative of the true answer.
		* Add q back (which is subtracting from the negative)
		* so the sign flips again.
			*/
		i = bnSub(&xq, q);
		if (i < 0)
			goto fail;
bndPut("xq - xp mod q = ", &xq);
bndPrintf("With sign %d\n", i);		/* Must be 1 */
	}

	/* Compute k = xq * u mod q */
	if (bnMul(&k, u, &xq) < 0)
		goto fail;
bndPut("(xq-xp) * u = ", &k);
	if (bnMod(&k, &k, q) < 0)
		goto fail;
bndPut("k = (xq-xp)*u % q = ", &k);

	/* Now x = k * p + xp is the final answer */
	if (bnMul(x, &k, p) < 0)
		goto fail;
bndPut("k * p = ", x);
	if (bnAdd(x, &xp) < 0)
		goto fail;
bndPut("k*p + xp = ", x);

#if BNDEBUG	/* @@@ DEBUG - do it the slow way for comparison */
	if (bnMul(&xq, p, q) < 0)
		goto fail;
bndPut("n = p*q = ", &xq);
	if (bnExpMod(&xp, x, d, &xq) < 0)
		goto fail;
bndPut("x^d mod n = ", &xp);
	if (bnCmp(x, &xp) != 0) {
bndPrintf("Nasty!!!\n");
		goto fail;
	}
	bnSetQ(&k, 17);
	bnExpMod(&xp, &xp, &k, &xq);
bndPut("x^17 mod n = ", &xp);
#endif

	bnEnd(&xp);
	bnEnd(&xq);
	bnEnd(&k);
	return 0;

fail:
	bnEnd(&xp);
	bnEnd(&xq);
	bnEnd(&k);
	return PGPERR_NOMEM;
}


/*
* Decrypt a message with a public key.
* These destroy (actually, replace with a decrypted version) the
* input bignum bn.
*
* It recongizes the PGP 2.2 format, but not in all its generality;
* only the one case (framing byte = 1, length = 16) which was ever
* generated. It fakes the DER prefix in that case.
*
* Performs an RSA signature check. Returns a prefix of the unwrapped
* data in the given buf. Returns the length of the untruncated
* data, which may exceed "len". Returns <0 on error.
	*/
int
rsaPublicDecrypt(byte *buf, unsigned len, struct BigNum *bn,
	struct RSApub const *pub)
{
	unsigned bytes;

bndPrintf("RSA signature checking.\n");
	if (bnExpMod(bn, bn, &pub->e, &pub->n) < 0)
		return PGPERR_NOMEM;
bndPut("decrypted = ", bn);
	bytes = (bnBits(&pub->n)+7)/8;
	return pgpPKCSUnpack(buf, len, bn, PKCS_PAD_SIGNED, bytes);
}


/*
* Performs an RSA decryption. Returns a prefix of the unwrapped
* data in the given buf. Returns the length of the untruncated
* data, which may exceed "len". Returns <0 on error.
	*/
int
rsaPrivateDecrypt(byte *buf, unsigned len, struct BigNum *bn,
	struct RSAsec const *sec)
{
	unsigned bytes;

bndPrintf("RSA decrypting\n");
	if (bnExpModCRA(bn, &sec->d, &sec->p, &sec->q, &sec->u) < 0)
		return PGPERR_NOMEM;
bndPut("decrypted = ", bn);
	bytes = (bnBits(&sec->n)+7)/8;
	return pgpPKCSUnpack(buf, len, bn, PKCS_PAD_ENCRYPTED, bytes);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
#endif (!NO_RSA && !USE_RSAREF_FULL)
