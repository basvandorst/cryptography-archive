/*
 * pgpDSAKey.c -- Signatures using the Digital Signature Algorithm
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpDSAKey.c,v 1.10.2.4 1997/06/13 04:15:25 hal Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpDSAKey.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "pgpCipher.h"
#include "pgpCFB.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpFixedKey.h"
#include "bnprime.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"

#ifndef NULL
#define NULL 0
#endif

#define ASSERTDSA(alg) pgpAssert((ALGMASK(alg))==PGP_PKALG_DSA)

#define MAX_DSA_PRIME_BITS		1024

struct DSApub {
	struct BigNum p;		/* Public prime */
	struct BigNum q;		/* Public order of generator */
	struct BigNum g;		/* Public generator */
	struct BigNum y;		/* Public key, g**x mod p */
};

struct DSAsec {
	struct BigNum p;		/* Copy of public parameters */
	struct BigNum q;
	struct BigNum g;
	struct BigNum y;
	struct BigNum x;		/* Secret key, discrete log of y */
};


/* A PgpSecKey's priv points to this, an DSAsec plus the encrypted form... */
struct DSAsecPlus {
	struct DSAsec s;
	byte *cryptkey;
	size_t ckalloc, cklen;
	int locked;
};

/** Public key functions **/

static void
dsaPubDestroy(struct PgpPubKey *pubkey)
{
	struct DSApub *pub = (struct DSApub *)pubkey->priv;

	ASSERTDSA(pubkey->pkAlg);
	
	bnEnd(&pub->p);
	bnEnd(&pub->q);
	bnEnd(&pub->g);
	bnEnd(&pub->y);
	memset(pub, 0, sizeof(pub));
	pgpMemFree(pub);
	memset(pubkey, 0, sizeof(pubkey));
	pgpMemFree(pubkey);
}

/*
 * This returns TRUE if the key is too big, returning the
 * maximum number of bits that the library can accept.
 */
static int
dsaKeyTooBig(struct DSApub const *pub, struct DSAsec const *sec)
{
	unsigned maxsize = MAX_DSA_PRIME_BITS;
	if (pub) {
		if (bnBits(&pub->p) > maxsize)
			return maxsize;
	}
	if (sec) {
		if (bnBits(&sec->p) > maxsize)
			return maxsize;
	}
	/* Else OK */
	return 0;
}

#if 0
static void
dsaPubId8(struct PgpPubKey const *pubkey, byte *buf)
{
	pgpAssert(0);
	struct DSApub const *pub = (struct DSApub *)pubkey->priv;
	byte *keybuf;
	size_t keybuflen;
	struct PgpHash const *h;
	struct PgpHashContext *hc;

	ASSERTDSA(pubkey->pkAlg);

	/* Unfortunately we have no way of indicating failure */
	h = pgpHashByNumber (PGP_HASH_SHA);
	pgpAssert(h);
	hc = pgpHashCreate(h);
	pgpAssert(hc);

	keybuflen = dsaPubBufferLength(pubkey);
	keybuf = pgpMemAlloc(keybuflen);
	pgpAssert(keybuf);
	dsaPubToBuffer(pubkey, keybuf);

	pgpHashUpdate(hc, keybuf, keybuflen);
	memcpy (buf, pgpHashFinal(hc), 8);
	pgpHashDestroy(hc);
}
#endif

/* Return the largest possible PgpESK size for a given key */
static size_t
dsaMaxesk(struct PgpPubKey const *pubkey, PgpVersion version)
{
	(void)pubkey;
	(void)version;
	return PGPERR_PUBKEY_UNIMP;
}

/*
 * Given a buffer of at least "maxesk" bytes, make an PgpESK
 * into it and return the size of the PgpESK, or <0.
 */
static int
dsaEncrypt(struct PgpPubKey const *pubkey, byte const *key,
           size_t keylen, byte *esk, size_t *esklen,
           struct PgpRandomContext const *rc, PgpVersion version)
{
	(void)pubkey;
	(void)key;
	(void)keylen;
	(void)esk;
	(void)esklen;
	(void)rc;
	(void)version;
	return PGPERR_PUBKEY_UNIMP;
}

/*
 * Return 1 if (sig,siglen) is a valid MPI which signs
 * hash, of type h.  Verify that the type is SHA.1 and
 * the hash itself matches.
 */
static int
dsaVerify(struct PgpPubKey const *pubkey, int sigtype, byte const *sig,
	size_t siglen, struct PgpHash const *h, byte const *hash)
{
	struct DSApub const *pub = (struct DSApub *)pubkey->priv;
	struct BigNum r, s, w, u2;
	int i;
	size_t off;

	(void)sigtype;

	ASSERTDSA(pubkey->pkAlg);

	if (h->type != PGP_HASH_SHA)
		return 0;	/* No match for sure! */

	bnBegin(&r);
	bnBegin(&s);
	bnBegin(&w);
	bnBegin(&u2);

	/* sig holds two values.  Get first, r, from sig. */
	off = 0;
	i = pgpBnGetPlain(&r, sig+off, siglen-off);
	if (i <= 0)
		goto fail;
	/* Get 2nd value, s, from SIG */
	off += i;
	i = pgpBnGetPlain(&s, sig+off, siglen-off);
	if (i <= 0)
		goto fail;
	off += i;
	if (off != siglen) {
		i = PGPERR_SIG_TOOLONG;
		goto done;
	}

	/*
	 * Sanity-check r and s against the subprime q.  Both should
	 * be less than q.  If not, the signature is clearly bad.
	 */
	if (bnCmp(&r, &pub->q) >= 0 || bnCmp(&s, &pub->q) >= 0) {
		i = 0;	/* FAIL */
		goto done;
	}
	
	/* Reconstruct hash as u2 */
	/* @@@: complicated issues re packing, 160 bits is tight... */
	if (bnInsertBigBytes(&u2, hash, 0, h->hashsize) < 0)
		goto nomem;

	/*
	 * Calculate DSS check function....
	 * Given signature (r,s) and hash H (in bn), compute:
	 * w = s^-1 mod q
	 * u1 = H * w mod q
	 * u2 = r * w mod q
	 * v = g^u1 * y^u2 mod p
	 * if v == r mod q, the signature checks.
	 *
	 * To save space, we put u1 into s, H into u2, and v into w.
	 */
	if (bnInv(&w, &s, &pub->q) < 0)
		goto nomem;
	if (bnMul(&s, &u2, &w) < 0 || bnMod(&s, &s, &pub->q) < 0)
		goto nomem;
	if (bnMul(&u2, &r, &w) < 0 || bnMod(&u2, &u2, &pub->q) < 0)
		goto nomem;

        /* Now for the expensive part... */

        if (bnDoubleExpMod(&w, &pub->g, &s, &pub->y, &u2, &pub->p) < 0)
                goto nomem;
        if (bnMod(&w, &w, &pub->q) < 0)
                goto nomem;

	/* Compare result with r, should be equal */
	i = bnCmp(&w, &r) == 0;

	goto done;

fail:
	if (!i)
		i = PGPERR_SIG_TOOSHORT;
	goto done;
nomem:
	i = PGPERR_NOMEM;
	goto done;
done:
	bnEnd(&u2);
	bnEnd(&w);
	bnEnd(&s);
	bnEnd(&r);

	return i;
}

/*
 * Turn a PgpPubKey into the algorithm-specific parts of a public key.
 * A public key's DSA-specific part is:
 *
 *  0      2+i  MPI for prime
 * 2+i     2+t  MPI for order
 * 4+i+t   2+u	MPI for generator
 * 6+i+t+u 2+v	MPI for public key
 * 8+i+t+u+v
 */
static size_t
dsaPubBufferLength(struct PgpPubKey const *pubkey)
{
	struct DSApub const *pub = (struct DSApub *)pubkey->priv;

	return 8 + (bnBits(&pub->p)+7)/8 + (bnBits(&pub->q)+7)/8 +
		   (bnBits(&pub->g)+7)/8 + (bnBits(&pub->y)+7)/8;
}

static void
dsaPubToBuffer(struct PgpPubKey const *pubkey, byte *buf)
{
	struct DSApub const *pub = (struct DSApub *)pubkey->priv;
	unsigned off;

	off = 0;
	off += pgpBnPutPlain(&pub->p, buf+off);
	off += pgpBnPutPlain(&pub->q, buf+off);
	off += pgpBnPutPlain(&pub->g, buf+off);
	off += pgpBnPutPlain(&pub->y, buf+off);
}


/* A little helper function that's used twice */
static void
dsaFillPubkey(struct PgpPubKey *pubkey, struct DSApub *pub)
{
	pubkey->next	 = NULL;
	pubkey->pkAlg	 = PGP_PKALG_DSA;
	pubkey->priv	 = pub;
	pubkey->destroy  = dsaPubDestroy;
#if 0
	pubkey->id8      = dsaPubId8;
#endif
	pubkey->maxesk   = dsaMaxesk;
	pubkey->encrypt  = dsaEncrypt;
	pubkey->verify   = dsaVerify;
	pubkey->bufferLength  = dsaPubBufferLength;
	pubkey->toBuffer = dsaPubToBuffer;
}


/*
 * Turn the algorithm-specific parts of a public key into a PgpPubKey
 * structure.  A public key's DSA-specific part is:
 *
 *  0      2+i  MPI for prime
 * 2+i     2+t  MPI for order
 * 4+i+t   2+u	MPI for generator
 * 6+i+t+u 2+v	MPI for public key
 * 8+i+t+u+v
 */
struct PgpPubKey *
dsaPubFromBuf(byte const *buf, size_t size, int *error)
{
	struct PgpPubKey *pubkey;
	struct DSApub *pub;
	int err = PGPERR_NOMEM;
	unsigned i, t, u, v;
	int w;

	bnInit();

	w = pgpBnParse(buf, size, 4, &i, &t, &u, &v);
	if (w < 0) {
		*error = w;
		return NULL;
	}
	if (t <= i+2 || (buf[t-1] & 1) == 0) {	/* Too small or even prime p */
		*error = PGPERR_KEY_MPI;
		return NULL;
	}
	if (u <= t+2 || (buf[u-1] & 1) == 0) {	/* Too small or even order q */
		*error = PGPERR_KEY_MPI;
		return NULL;
	}
	pub = (struct DSApub *)pgpMemAlloc(sizeof(*pub));
	if (pub) {
		pubkey = (struct PgpPubKey *)pgpMemAlloc(sizeof(*pubkey));
		if (pubkey) {
			bnBegin(&pub->p);
			bnBegin(&pub->q);
			bnBegin(&pub->g);
			bnBegin(&pub->y);
			if (bnInsertBigBytes(&pub->p, buf+i+2, 0, t-i-2) >= 0
			 && bnInsertBigBytes(&pub->q, buf+t+2, 0, u-t-2) >= 0
			 && bnInsertBigBytes(&pub->g, buf+u+2, 0, v-u-2) >= 0
			 && bnInsertBigBytes(&pub->y, buf+v+2, 0, w-v-2) >= 0)
			{
				if (dsaKeyTooBig (pub, NULL)) {
					err = PGPERR_KEY_UNSUPP;
				} else {
					dsaFillPubkey(pubkey, pub);
					*error = 0;
					return pubkey;
				}
			}
			/* Failed = clean up and return NULL */
			bnEnd(&pub->p);
			bnEnd(&pub->q);
			bnEnd(&pub->g);
			bnEnd(&pub->y);
			pgpMemFree(pubkey);
		}
		pgpMemFree(pub);
	}
	*error = err;
	return NULL;
}

/*
 * Return the size of the public portion of a key buffer.
 */
int
dsaPubKeyPrefixSize(byte const *buf, size_t size)
{
	return pgpBnParse(buf, size, 4, NULL, NULL, NULL, NULL);
}


/** Secret key functions **/

static void
dsaSecDestroy(struct PgpSecKey *seckey)
{
	struct DSAsecPlus *sec = (struct DSAsecPlus *)seckey->priv;

	ASSERTDSA(seckey->pkAlg);
	bnEnd(&sec->s.p);
	bnEnd(&sec->s.q);
	bnEnd(&sec->s.g);
	bnEnd(&sec->s.y);
	bnEnd(&sec->s.x);
	memset(sec->cryptkey, 0, sec->ckalloc);
	pgpMemFree(sec->cryptkey);
	memset(sec, 0, sizeof(sec));
	pgpMemFree(sec);
	memset(seckey, 0, sizeof(seckey));
	pgpMemFree(seckey);
}

#if 0
static void
dsaSecId8(struct PgpSecKey const *seckey, byte *buf)
{
	struct DSAsecPlus const *sec = (struct DSAsecPlus *)seckey->priv;

	ASSERTDSA(seckey->pkAlg);
	bnExtractBigBytes(&sec->s.p, buf, 0, 8);
}
#endif

/*
 * Generate a PgpPubKey from a PgpSecKey
 */
static struct PgpPubKey *
dsaPubkey(struct PgpSecKey const *seckey)
{
	struct DSAsecPlus const *sec = (struct DSAsecPlus *)seckey->priv;
	struct PgpPubKey *pubkey;
	struct DSApub *pub;

	ASSERTDSA(seckey->pkAlg);
	pub = (struct DSApub *)pgpMemAlloc(sizeof(*pub));
	if (pub) {
		pubkey = (struct PgpPubKey *)pgpMemAlloc(sizeof(*pubkey));
		if (pubkey) {
			bnBegin(&pub->p);
			bnBegin(&pub->q);
			bnBegin(&pub->g);
			bnBegin(&pub->y);
			if (bnCopy(&pub->p, &sec->s.p) >= 0
			    && bnCopy(&pub->q, &sec->s.q) >= 0
			    && bnCopy(&pub->g, &sec->s.g) >= 0
			    && bnCopy(&pub->y, &sec->s.y) >= 0)
			{
				dsaFillPubkey(pubkey, pub);
				pubkey->pkAlg = seckey->pkAlg;
				memcpy(pubkey->keyID, seckey->keyID,
				       sizeof(pubkey->keyID));
				return pubkey;
			}
			/* Failed = clean up and return NULL */
			bnEnd(&pub->p);
			bnEnd(&pub->q);
			bnEnd(&pub->g);
			bnEnd(&pub->y);
			pgpMemFree(pubkey);
		}
		pgpMemFree(pub);
	}
	return NULL;
}

/*
 * Yes, there *is* a reason that this is a function and not a variable.
 * On a hardware device with an automatic timeout,
 * it actually might need to do some work to find out.
 */
static int
dsaIslocked(struct PgpSecKey const *seckey)
{
	struct DSAsecPlus const *sec = (struct DSAsecPlus *)seckey->priv;

	ASSERTDSA(seckey->pkAlg);
	return sec->locked;
}

/*
 * Try to decrypt the secret key wih the given passphrase.  Returns >0
 * if it was the correct passphrase. =0 if it was not, and <0 on error.
 * Does not alter the key even if it's the wrong passphrase and already
 * unlocked.  A NULL passphrae will work if the key is unencrypted.
 *
 * A (secret) key's DSA-specific part is:
 *
 *  0                2+u  MPI for prime p
 *  2+u              2+v  MPI for order q
 *  4+u+v            2+w  MPI for generator g
 *  6+u+v+w	     2+x  MPI for public key y
 *  8+u+v+w+x        1    Encryption algorithm (0 for none, 1 for IDEA)
 *  9+u+v+w+x        t    Encryption IV: 0 or 8 bytes
 *  9+t+u+v+w+x      2+y  MPI for x (discrete log of public key)
 * 11+t+u+v+w+x+y    2    Checksum
 * 13+t+u+v+w+x+y
 *
 * Actually, that's the old-style, if pgpS2KoldVers is true.
 * If it's false, the algoruthm is 255, and is followed by the
 * algorithm, then the (varaible-length, self-delimiting)
 * string-to-key descriptor.
 */

static int
dsaUnlock(struct PgpSecKey *seckey, struct PgpEnv const *env,
	  char const *phrase, size_t plen)
{
	struct DSAsecPlus *sec = (struct DSAsecPlus *)seckey->priv;
	struct BigNum x;
	struct PgpCfbContext *cfb = NULL;
	unsigned v;
	unsigned alg;
	unsigned checksum;
	int i;

	bnBegin(&x);

	ASSERTDSA(seckey->pkAlg);

	/* Check packet for basic consistency */
	i = pgpBnParse(sec->cryptkey, sec->cklen, 4, &v, NULL, NULL, NULL);
	if (i <= 0)
		goto fail;

	/* OK, read the public data */
	i = pgpBnGetPlain(&sec->s.p, sec->cryptkey+v, sec->cklen-v);
	if (i <= 0)
		goto fail;
	v += i;
	i = pgpBnGetPlain(&sec->s.q, sec->cryptkey+v, sec->cklen-v);
	if (i <= 0)
		goto fail;
	v += i;
	i = pgpBnGetPlain(&sec->s.g, sec->cryptkey+v, sec->cklen-v);
	if (i <= 0)
		goto fail;
	v += i;
	i = pgpBnGetPlain(&sec->s.y, sec->cryptkey+v, sec->cklen-v);
	if (i <= 0)
		goto fail;
	v += i;

	/* Get the encryption algorithm (cipher number).  0 == no encryption */
	alg  = sec->cryptkey[v];

	/* If the phrase is empty, set it to NULL */
	if (plen == 0)
		phrase = NULL;
	/*
	 * We need a pass if it is encrypted, and we cannot have a
	 * password if it is NOT encrypted.  I.e., this is a logical
	 * xor (^^)
	 */
	if (!phrase != !sec->cryptkey[v])
		goto badpass;

	i = pgpCipherSetup(sec->cryptkey + v, sec->cklen - v, phrase, plen,
	                   env, &cfb);
	if (i < 0)
		goto done;
	v += i;

	checksum = 0;
	i = pgpBnGetNew(&x, sec->cryptkey + v, sec->cklen - v, cfb, &checksum);
	if (i <= 0)
		goto badpass;
	v += i;
	if (bnCmp(&x, &sec->s.q) >= 0)
		goto badpass;	/* Wrong passphrase: x must be < q */

	/* Check that we ended in the right place */
	if (sec->cklen - v != 2) {
		i = PGPERR_KEY_LONG;
		goto fail;
	}
	checksum &= 0xffff;
	if (checksum != pgpChecksumGetNew(sec->cryptkey+v, cfb))
		goto badpass;

	/*
	 * Note that the "nomem" case calls bnEnd()
	 * more than once, but this is guaranteed harmless.
 	 */
	if (bnCopy(&sec->s.x, &x) < 0)
		goto nomem;

	i = 1;	/* Decrypted! */
	sec->locked = 0;
	goto done;

nomem:
	i = PGPERR_NOMEM;
	goto done;
fail:
	if (!i)
		i = PGPERR_KEY_SHORT;
	goto done;
badpass:
	i = 0;	/* Incorrect passphrase */
	goto done;
done:
	bnEnd(&x);
	if (cfb)
		pgpCfbDestroy (cfb);
	return i;
}

/*
 * Relock the key.
 */
static void
dsaLock(struct PgpSecKey *seckey)
{
	struct DSAsecPlus *sec = (struct DSAsecPlus *)seckey->priv;

	ASSERTDSA(seckey->pkAlg);
	sec->locked = 1;
	/* bnEnd is documented as also doing a bnBegin */
	bnEnd(&sec->s.x);
}

/*
 * Return the size of the buffer needed, worst-case, for the decrypted
 * output.
 */
static size_t
dsaMaxdecrypted(struct PgpSecKey const *seckey)
{
	(void)seckey;
	return PGPERR_PUBKEY_UNIMP;
}

/*
 * Try to decrypt the given esk.  If the key is locked, try the given
 * passphrase.  It may or may not leave the key unlocked in such a case.
 * (Some hardware implementations may insist on a password per usage.)
 */
static int
dsaDecrypt(struct PgpSecKey *seckey, struct PgpEnv const *env,
	   int esktype, byte const *esk, size_t esklen,
	   byte *key, size_t *keylen, char const *phrase,
	   size_t plen)
{
	(void)seckey;
	(void)env;
	(void)esktype;
	(void)esk;
	(void)esklen;
	(void)key;
	(void)keylen;
	(void)phrase;
	(void)plen;
	return PGPERR_PUBKEY_UNIMP;
}

static size_t
dsaMaxsig(struct PgpSecKey const *seckey, PgpVersion version)
{
	struct DSAsecPlus const *sec = (struct DSAsecPlus *)seckey->priv;

	(void)version;
	ASSERTDSA(seckey->pkAlg);
	return 2*((bnBits(&sec->s.q)+7)/8 + 2);
}

/*
 * Helper function: seed a RandomContext from a BigNum.
 * Be very sure to leave nothing in memory!
 */
static void
pgpRandomBnSeed(struct PgpRandomContext const *rc, struct BigNum const *bn)
{
	byte buf[32];	/* Big enough for 99.9% of all keys */
	unsigned bytes = (bnBits(bn) + 7)/8;
	unsigned off = 0;

	while (bytes > sizeof(buf)) {
		bnExtractLittleBytes(bn, buf, off, sizeof(buf));
		pgpRandomAddBytes(rc, buf, sizeof(buf));
		bytes -= sizeof(buf);
		off += sizeof(buf);
	}
	bnExtractLittleBytes(bn, buf, off, bytes);
	pgpRandomAddBytes(rc, buf, bytes);

	memset(buf, 0, sizeof(buf));
}

static int
dsaSign(struct PgpSecKey *seckey, struct PgpHash const *h, byte const *hash,
	byte *sig, size_t *siglen, struct PgpRandomContext const *rc,
	PgpVersion version)
{
	/* Calculate a DSA signature */

	struct DSAsecPlus *sec = (struct DSAsecPlus *)seckey->priv;
	struct BigNum r, s, bn, k;
	unsigned t;
	unsigned qbits;
	int i;
	struct PgpCipher const *ciph;
	struct PgpRandomContext *rc2;

	/* We don't need this argument, although other algorithms may... */
	(void)version;

	ASSERTDSA(seckey->pkAlg);
	/* We *only* sign SHA hashes */
	pgpAssert(h->type == PGP_HASH_SHA);

	if (sec->locked)
		return PGPERR_KEY_ISLOCKED;

	/*
	 * DSA requires a secret k.  This k is *very* important
	 * to keep secret.  Consider, the DSA signing equations are:
	 * r = (g^k mod p) mod q, and
	 * s = k^-1 * (H(m) + x*r) mod q,
	 * so if you know k (and, the signature r, s and H), then
	 * x = r^-1 * (k*s - H(m))
	 * If we ever pick two k values the same, then
	 * r = (g^k mod p) mod q is the same for both signatures, and
	 * s1 = k^-1 * (H1 + x * r)
	 * s2 = k^-1 * (H2 + x * r)
	 * k = (H1-H2) / (s1-s2)
	 * and proceed from there.
	 *
	 * So we need to make *very* sure there's no problem.  To make
	 * sure, we add a layer on top of the passed-in RNG.  We assume
	 * the passed-in RNG is good enough to never repeat (not a
	 * difficult task), and apply an additional X9.17 generator on
	 * top of that, seeded with the secret x, which is destroyed
	 * before leaving this function.
	 *
	 * In addition, we add entropy from the hash to the original RNG.
	 * This will prevent us from using the same k value twice if the
	 * messages are different.
	 */
	pgpRandomAddBytes(rc, hash, h->hashsize);
	ciph = pgpCipherByNumber(PGP_CIPHER_CAST5);
	if (!ciph)
		return PGPERR_BAD_CIPHERNUM;
	rc2 = pgpRandomCreateX9_17(ciph, rc);
	if (!rc2)
		return PGPERR_NOMEM;
	pgpRandomBnSeed(rc2, &sec->s.x);

	bnBegin(&r);
	bnBegin(&s);
	bnBegin(&bn);
	bnBegin(&k);

	/*
	 * Choose the random k value to be used for this signature.
	 * Make it a bit bigger than q so it is fairly uniform mod q.
	 */
	qbits = bnBits(&sec->s.q);
	if (pgpBnGenRand(&k, rc2, qbits+8, 0, 1, qbits) < 0 ||
	    bnMod(&k, &k, &sec->s.q) < 0)
		goto nomem;
	
	/* Raise g to k power mod p then mod q to get r */
	if (bnExpMod(&r, &sec->s.g, &k, &sec->s.p) < 0 ||
	    bnMod(&r, &r, &sec->s.q) < 0)
		goto nomem;
	
	/* r*x mod q into s */
	if (bnMul(&s, &r, &sec->s.x) < 0 ||
	    bnMod(&s, &s, &sec->s.q) < 0)
		goto nomem;

	/* Pack message hash M into buffer bn */
	/* TODO: complicated issues re packing, 160 bits is tight... */
	if (bnInsertBigBytes(&bn, hash, 0, h->hashsize) < 0)
		goto nomem;
	if (bnMod(&bn, &bn, &sec->s.q) < 0)
		goto nomem;

	/* Add into s */
	if (bnAdd(&s, &bn) < 0 ||
	    bnMod(&s, &s, &sec->s.q) < 0)
		goto nomem;

	/* Divide by k, mod q (k inverse held in bn) */
	if (bnInv(&bn, &k, &sec->s.q) < 0 ||
	    bnMul(&s, &s, &bn) < 0 ||
	    bnMod(&s, &s, &sec->s.q) < 0)
		goto nomem;

	/* That's it, now to pack r and then s into the buffer */
	t  = pgpBnPutPlain(&r, sig);
	t += pgpBnPutPlain(&s, sig+t);
	if (siglen)
		*siglen = (size_t)t;

	i = 0;
	goto done;

nomem:
	i = PGPERR_NOMEM;
	/* fall through */
done:
	pgpRandomDestroy(rc2);
	bnEnd(&k);
	bnEnd(&bn);
	bnEnd(&s);
	bnEnd(&r);
	return i;
}

/*
 * Re-encrypt a PgpSecKey with a new urn a PgpSecKey into a secret key.
 * A secret key is, after a non-specific prefix:
 *  0       1    Version (= 2 or 3)
 *  1       4    Timestamp
 *  5       2    Validity (=0 at present)
 *  7       1    Algorithm (=PGP_PKALG_DSA for DSA)
 * The following:
 *  0                2+u  MPI for prime p
 *  2+u              2+v  MPI for order q
 *  4+u+v            2+w  MPI for generator g
 *  6+u+v+w	     2+x  MPI for public key y
 *  8+u+v+w+x        1    Encryption algorithm (0 for none, 1 for IDEA)
 *  9+u+v+w+x        t    Encryption IV: 0 or 8 bytes
 *  9+t+u+v+w+x      2+y  MPI for x (discrete log of public key)
 * 11+t+u+v+w+x+y    2    Checksum
 * 13+t+u+v+w+x+y
 *
 * The Encryption algorithm is the cipher algorithm for the old-style
 * string-to-key conversion.  For the new type, it's 255, then a cipher
 * algorithm, then a string-to-key algorithm (variable-length),
 * then the encryption IV.  That's 16 bytes plus the string-to-key
 * conversion length.
 *
 * On initial key generation we rely on calling this with env=NULL being
 * OK if phrase=NULL.
 */

static int
dsaChangeLock(struct PgpSecKey *seckey, struct PgpEnv const *env,
	struct PgpRandomContext const *rc, char const *phrase, size_t plen)
{
	struct DSAsecPlus *sec = (struct DSAsecPlus *)seckey->priv;
	struct PgpStringToKey *s2k = NULL;	/* Shut up warnings */
	struct PgpCipher const *cipher = NULL;	/* Shut up warnings */
	struct PgpCfbContext *cfb = NULL;	/* This is realy needed */
	byte *p;
	int oldf = 0;				/* Shut up warnings */
	unsigned len;
	unsigned checksum;

	ASSERTDSA(seckey->pkAlg);
	if (sec->locked)
		return PGPERR_KEY_ISLOCKED;

	len = bnBytes(&sec->s.p) + bnBytes(&sec->s.q) + bnBytes(&sec->s.g) +
	      bnBytes(&sec->s.y) + bnBytes(&sec->s.x) + 13;
	if (phrase) {
		s2k = pgpS2Kdefault(env, rc);
		if (!s2k)
			return PGPERR_NOMEM;
		cipher = pgpCipherDefaultKey(env);
		pgpAssert(cipher);
		if (!cipher) {
			pgpS2Kdestroy(s2k);
			return PGPERR_NOMEM;
		}
		len += cipher->blocksize;
		cfb = pgpCfbCreate(cipher);
		if (!cfb) {
			pgpS2Kdestroy(s2k);
			return PGPERR_NOMEM;
		}
		oldf = pgpS2KisOldVers(s2k);
		if (!oldf)
			len += 1 + s2k->encodelen;
	}
	p = sec->cryptkey;
	if (len > sec->ckalloc) {
		p = (byte *)pgpMemRealloc(p, len);
		if (!p) {
			pgpCfbDestroy(cfb);
			pgpS2Kdestroy(s2k);
			return PGPERR_NOMEM;
		}
		sec->cryptkey = p;
		sec->ckalloc = (size_t)len;
	}
	sec->cklen = len;

	/* Okay, no more errors possible!   Start installing data */
	p += pgpBnPutPlain(&sec->s.p, p);
	p += pgpBnPutPlain(&sec->s.q, p);
	p += pgpBnPutPlain(&sec->s.g, p);
	p += pgpBnPutPlain(&sec->s.y, p);

	/* Encryption parameters */
	if (!phrase) {
		*p++ = 0;	/* Unencrypted */
	} else {
		if (oldf) {
			*p++ = cipher->type;
		} else {
			*p++ = 255;
			*p++ = cipher->type;
			memcpy(p, s2k->encoding, s2k->encodelen);
			p += s2k->encodelen;
		}
		/* Create IV */
		pgpRandomGetBytes(rc, p, cipher->blocksize);
		/* Use data buffer as temp holding space for key */
		pgpAssert(sec->ckalloc-cipher->blocksize >= cipher->keysize);
		pgpStringToKey(s2k, phrase, plen, p+cipher->blocksize,
			cipher->keysize);
		pgpCfbInit(cfb, p+cipher->blocksize, p);
		pgpS2Kdestroy(s2k);
		p += cipher->blocksize;
		/* Wipe key *immediately* */
		memset(p, 0, cipher->keysize);
	}

	/* Now install x, encrypted */
	checksum = 0;
	p += pgpBnPutNew(&sec->s.x, p, cfb, &checksum);
	pgpChecksumPutNew(checksum, p, cfb);
	p += 2;
	pgpAssert((ptrdiff_t)len == p - sec->cryptkey);

	if (cfb)
		pgpCfbDestroy(cfb);
	return 0;	/* Success */
}

static size_t
dsaSecBufferLength(struct PgpSecKey const *seckey)
{
	struct DSAsecPlus const *sec = (struct DSAsecPlus *)seckey->priv;

	return sec->cklen;
}

static void
dsaSecToBuffer(struct PgpSecKey const *seckey, byte *buf)
{
	struct DSAsecPlus const *sec = (struct DSAsecPlus *)seckey->priv;

	memcpy(buf, sec->cryptkey, sec->cklen);

	/* Return only algorithm-dependent portion */
}


/* Fill in secret key structure */
static void
dsaFillSecKey(struct PgpSecKey *seckey, struct DSAsecPlus *sec)
{
	seckey->pkAlg	     = PGP_PKALG_DSA;
	seckey->priv	     = sec;
	seckey->destroy      = dsaSecDestroy;
#if 0
	seckey->id8          = dsaSecId8;
#endif
	seckey->pubkey       = dsaPubkey;
	seckey->islocked     = dsaIslocked;
	seckey->unlock       = dsaUnlock;
	seckey->lock         = dsaLock;
	seckey->maxdecrypted = dsaMaxdecrypted;
	seckey->decrypt      = dsaDecrypt;
	seckey->maxsig       = dsaMaxsig;
	seckey->sign         = dsaSign;
	seckey->changeLock   = dsaChangeLock;
	seckey->bufferLength = dsaSecBufferLength;
	seckey->toBuffer     = dsaSecToBuffer;
}


struct PgpSecKey *
dsaSecFromBuf(byte const *buf, size_t size, int *error)
{
	struct PgpSecKey *seckey;
	struct DSAsecPlus *sec;
	int err = PGPERR_NOMEM;
	byte *cryptk;

	bnInit();
	cryptk = (byte *)pgpMemAlloc(size);
	if (cryptk) {
		sec = (struct DSAsecPlus *)pgpMemAlloc(sizeof(*sec));
		if (sec) {
			seckey = (struct PgpSecKey *)
				pgpMemAlloc(sizeof(*seckey));
			if (seckey) {
				memcpy(cryptk, buf, size);
				bnBegin(&sec->s.p);
				bnBegin(&sec->s.q);
				bnBegin(&sec->s.g);
				bnBegin(&sec->s.y);
				bnBegin(&sec->s.x);
				sec->cryptkey = cryptk;
				sec->cklen = sec->ckalloc = size;
				sec->locked = 1;
				/* We only need this to try unlocking... */
				seckey->pkAlg = PGP_PKALG_DSA;
				seckey->priv = sec;
				
				if (dsaUnlock(seckey, NULL, NULL, 0) >= 0) {
					if (dsaKeyTooBig (NULL, &sec->s)) {
						err = PGPERR_KEY_UNSUPP;
					} else {
						dsaFillSecKey(seckey, sec);
						*error = 0;
						return seckey;	/* Success! */
					}
				}

				/* Ka-boom.  Delete and free everything. */
				memset(cryptk, 0, size);
				memset(sec, 0, sizeof(*sec));
				pgpMemFree(seckey);
			}
			pgpMemFree(sec);
		}
		pgpMemFree(cryptk);
	}
	*error = err;
	return NULL;
}

#if 0	/* Disabled to avoid use of libm */
/*
 * Heuristic algorithm to estimate the size of the prime order for the
 * generator for DSA signatures.
 *
 * slowfactor is ln(ln(n))**(2/3).
 * Formula for work factor is exp(2.08*(ln n)**(1/3)*slowfactor), where
 * that 2.08 is sensitive to the algorithm.  This assumes some pretty good
 * version of NFS.
 * Change to use base 2 and we get:
 * 2**(2.656*(log2 n)**(1/3)*slowfactor).
 * We assume a DH exponent of 160 is about right for n of about 2**1000.
 * When we change the DH exponent by n bits we get 2**(n/2) increase in
 * work factor, so to find out how much we should change it, take the
 * power of 2 in the formula above, double it, and subtract the value
 * for n=1000, then add 160.
 * This leads to 5.3 * slowfactor * (log2 n)**(1/3) all minus 25 (-185.5+160).
 *
 * A simpler approximation holds slowfactor constant.  Varies from 3.5 at
 * bitsize of 1000 to 4.0 at 4000 bits, so I found that 4.5 made a good
 * conservative approximation for values in this range.  Then heuristic
 * formula becomes cube root of size of prime in bits, times 24, minus 80.
 * This can be calculated pretty well in int arithmetic if we want to.
 */
static unsigned
dsaOrderBits (unsigned primebits)
{
	unsigned size;
	double slowfactor;
	double logbits;
	
	logbits = log((double)primebits);
	slowfactor = exp((2./3.)*log(-.366 + logbits));
	size = 5.3 * slowfactor * exp(logbits/3.) - 25;

	return size >= 160 ? size : 160;
}
#endif


/*
 * Generate an DSA secret key with prime of the specified number of bits.
 * Make callbacks to progress function periodically.
 * Secret key is returned in the unlocked form, with no passphrase set.
 * fastgen tells us to use canned primes if available.
 *
 * PGP attempts to acquire enough true random entropy in the randpool to
 * make the keys it generates fully random and unpredictable, even if the
 * RNG used to generate them were later found to have some weaknesses.  With
 * RSA keys it gets as many bits as the size of the modulus since the sizes
 * of the secret primes p and q will add up to the size of the modulus.
 * (This is slight overkill since the entropy in a random prime is less
 * than the entropy of a random number because not all numbers are prime.)
 *
 * With discrete log based keys, DSA and ElGamal, only the private exponent
 * x needs to be kept secret.  However, the public values are generated at
 * the same time as x, and are seeded ultimately from the same randpool.
 * These values could theoretically leak information about the state of the
 * randpool when they were generated, and therefore about x.  This would
 * require a very powerful attack which will probably never be possible,
 * but we want to defend against it.  One approach would simply be to acquire
 * as much additional entropy as is needed for the public values, but that
 * is wasteful.  The public values don't need to be random, we just want them
 * to be different among users.
 *
 * Instead, we create a "firewall" between the randpool and the public
 * key values.  We instantiate a second PgpRandomContext which is not
 * based on the randpool but is a simple pseudo RNG, and seed it with
 * a fixed number of bits from the true RNG.  We choose enough bits
 * for the seeding that different keys will not share the same public
 * values.  Only this fixed number of bits reflects the state of the
 * randpool, so we acquire that many bits of additional entropy before
 * beginning the keygen.  This second RNG, rcdummy below and in the
 * ElGamal keygen, is used to generate the public values for the discrete
 * log key.
 */
struct PgpSecKey *
dsaSecGenerate(unsigned bits, Boolean fastgen,
	struct PgpRandomContext const *rc,
	int progress(void *arg, int c), void *arg, int *error)
{
	struct PgpSecKey *seckey = NULL;
	struct DSAsecPlus *sec;
	struct PgpRandomContext *rcdummy = NULL;
	struct BigNum h;
	struct BigNum e;
	unsigned qbits;
	int i;
	byte dummyseed[DSADUMMYBITS/8];

	*error = 0;

	/* Initialize local pointers (simplify cleanup below) */
	seckey = NULL;
	sec = NULL;
	bnBegin(&h);
	bnBegin(&e);

	/* Limit the size we will generate at this time */
	if (bits > MAX_DSA_PRIME_BITS) {
		*error = PGPERR_PUBKEY_TOOBIG;
		goto done;
	}

	/* Allocate data structures */
	seckey = (struct PgpSecKey *)pgpMemAlloc(sizeof(*seckey));
	if (!seckey)
		goto memerror;
	sec = (struct DSAsecPlus *)pgpMemAlloc(sizeof(*sec));
	if (!sec)
		goto memerror;
	
	bnBegin(&sec->s.p);
	bnBegin(&sec->s.q);
	bnBegin(&sec->s.g);
	bnBegin(&sec->s.y);
	bnBegin(&sec->s.x);

	/* Use fixed primes and generator if in our table */
	if (fastgen) {
		byte const *fixedp, *fixedq;
		size_t fixedplen, fixedqlen;
		if (pgpDSAfixed (bits, &fixedp, &fixedplen, &fixedq, &fixedqlen) > 0) {
			bnInsertBigBytes (&sec->s.q, fixedq, 0, fixedqlen);
			if (progress)
				progress(arg, ' ');
			bnInsertBigBytes (&sec->s.p, fixedp, 0, fixedplen);
			if (progress)
				progress(arg, ' ');
			qbits = bnBits (&sec->s.q);
			goto choose_g;
		}
	}

	/* Set up and seed local random number generator for p and q */
	rcdummy = pgpPseudoRandomCreate ();
	if (!rcdummy)
		goto memerror;
	pgpRandomGetBytes (rc, dummyseed, sizeof(dummyseed));
	pgpRandomAddBytes (rcdummy, dummyseed, sizeof(dummyseed));

	/*
	 * Choose a random starting place for q, in the high end of the range
	 */
	if (bits <= 1024)
		qbits = 160;	/* Follow the published standard */
	else
		qbits = pgpDiscreteLogExponentBits(bits);
	if (pgpBnGenRand(&sec->s.q, rcdummy, qbits, 0xFF, 1, qbits-9) < 0)
		goto nomem;
	/* And search for a prime */
	i = bnPrimeGen(&sec->s.q, NULL, progress, arg, 0);
	if (i < 0)
		goto nomem;
	if (progress)
		progress(arg, ' ');

	/* ...and now a random start for p (we discard qbits bits of it) */
	(void)bnSetQ(&sec->s.p, 0);
	if (pgpBnGenRand(&sec->s.p, rcdummy, bits, 0xC0, 1, bits-qbits) < 0)
		goto nomem;

	/* Temporarily double q */
	if (bnLShift(&sec->s.q, 1) < 0)
		goto nomem;

	/* Set p = p - (p mod q) + 1, i.e. congruent to 1 mod 2*q */
	if (bnMod(&e, &sec->s.p, &sec->s.q) < 0)
		goto nomem;
	if (bnSub(&sec->s.p, &e) < 0 || bnAddQ(&sec->s.p, 1) < 0)
		goto nomem;

	/* And search for a prime, 1+2kq for some k */
	i = bnPrimeGenStrong(&sec->s.p, &sec->s.q, progress, arg);
	if (i < 0)
		goto nomem;
	if (progress)
		progress(arg, ' ');

	/* Reduce q again */
	bnRShift(&sec->s.q, 1);

	/* May get here directly from above if fixed primes are used */
choose_g:

	/* Now hunt for a suitable g - first, find (p-1)/q */
	if (bnDivMod(&e, &h, &sec->s.p, &sec->s.q) < 0)
		goto nomem;
	/* e is now the exponent (p-1)/q, and h is the remainder (one!) */
	pgpAssert(bnBits(&h)==1);

	if (progress)
		progress(arg, '.');

	/* Search for a suitable h */
	if (bnSetQ(&h, 2) < 0 ||
	    bnTwoExpMod(&sec->s.g, &e, &sec->s.p) < 0)
		goto nomem;
	while (bnBits(&sec->s.g) < 2) {
		if (progress)
			progress(arg, '.');
		if (bnAddQ(&h, 1) < 0 ||
		    bnExpMod(&sec->s.g, &h, &e, &sec->s.p) < 0)
			goto nomem;
	}
	if (progress)
		progress(arg, ' ');

	/* Choose a random 0 < x < q of reasonable size as secret key */
	if (pgpBnGenRand(&sec->s.x, rc, qbits + 8, 0, 0, qbits) < 0 ||
	    bnMod(&sec->s.x, &sec->s.x, &sec->s.q) < 0)
		goto nomem;
	/* prob. failure < 2^-140 is awful unlikely... */
	pgpAssert(bnBits(&sec->s.x) > 20);

	/* And calculate g**x as public key */
	if (bnExpMod(&sec->s.y, &sec->s.g, &sec->s.x, &sec->s.p) < 0)
		goto nomem;

	/* And that's it... success! */

	/* Fill in structs */
	sec->cryptkey = NULL;
	sec->ckalloc = sec->cklen = 0;
	sec->locked = 0;
	dsaFillSecKey(seckey, sec);

	/* Fill in cryptkey structure, unencrypted */
	dsaChangeLock (seckey, NULL, NULL, NULL, 0);

	goto done;

nomem:
	bnEnd(&sec->s.p);
	bnEnd(&sec->s.q);
	bnEnd(&sec->s.g);
	bnEnd(&sec->s.y);
	bnEnd(&sec->s.x);
	/* Fall through */
memerror:
	pgpMemFree(seckey);
	pgpMemFree(sec);
	seckey = NULL;
	*error = PGPERR_NOMEM;
	/* Fall through */
done:
	bnEnd(&h);
	bnEnd(&e);

	if (rcdummy)
		pgpRandomDestroy (rcdummy);

	return seckey;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
