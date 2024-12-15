/*
 * pgpElGKey.c -- ElGamal encryption
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpElGKey.c,v 1.9.2.3 1997/06/07 09:51:25 mhw Exp $
 */

/* Experiment with El Gamal signatures */
#undef ELGSIGS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpElGKey.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "bnprime.h"
#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpFixedKey.h"
#include "bngermain.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "bnprime.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"

#ifndef NULL
#define NULL 0
#endif

#define ASSERTELG(alg) pgpAssert((ALGMASK(alg))==PGP_PKALG_ELGAMAL)

struct ELGpub {
	struct BigNum p;		/* Public prime */
	struct BigNum g;		/* Public generator */
	struct BigNum y;		/* Public key, g**x mod p */
};

struct ELGsec {
	struct BigNum p;		/* Copy of public parameters */
	struct BigNum g;
	struct BigNum y;
	struct BigNum x;		/* Secret key, discrete log of y */
};

/* A PgpSecKey's priv points to this, an ELGsec plus the encrypted form... */
struct ELGsecPlus {
	struct ELGsec s;
	byte *cryptkey;
	size_t ckalloc, cklen;
	int locked;
};

/** Public key functions **/

static void
elgPubDestroy(struct PgpPubKey *pubkey)
{
	struct ELGpub *pub = (struct ELGpub *)pubkey->priv;

	ASSERTELG(pubkey->pkAlg);
	
	bnEnd(&pub->p);
	bnEnd(&pub->g);
	bnEnd(&pub->y);
	memset(pub, 0, sizeof(pub));
	pgpMemFree(pub);
	memset(pubkey, 0, sizeof(pubkey));
	pgpMemFree(pubkey);
}

#if 0
static void
elgPubId8(struct PgpPubKey const *pubkey, byte *buf)
{
	pgpAssert(0);
	struct ELGpub const *pub = (struct ELGpub *)pubkey->priv;
	byte *keybuf;
	size_t keybuflen;
	struct PgpHash const *h;
	struct PgpHashContext *hc;

	pgpAssert (pubkey->pkAlg == PGP_PKALG_ELG);

	/* Unfortunately we have no way of indicating failure */
	h = pgpHashByNumber (PGP_HASH_SHA);
	pgpAssert(h);
	hc = pgpHashCreate(h);
	pgpAssert(hc);

	keybuflen = elgPubBufferLength(pubkey);
	keybuf = pgpMemAlloc(keybuflen);
	pgpAssert(keybuf);
	elgPubToBuffer(pubkey, keybuf);

	pgpHashUpdate(hc, keybuf, keybuflen);
	memcpy (buf, pgpHashFinal(hc), 8);
	pgpHashDestroy(hc);
}
#endif

/*
 * Return the largest possible PgpESK size for a given key.
 * Must hold two numbers up to p in size.
 */
static size_t
elgMaxesk(struct PgpPubKey const *pubkey, PgpVersion version)
{
	struct ELGpub const *pub = (struct ELGpub *)pubkey->priv;

	(void)version;
	ASSERTELG(pubkey->pkAlg);
	return 2*(2 + (bnBits(&pub->p)+7)/8);
}


#if 0
/*
 * Heuristic algorithm to estimate the size of the random exponent chosen
 * for El Gamal encryption.
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
 * for n=1000, then add 160.  This leads to 5.3 * slowfactor * (log2
 * n)**(1/3) all minus 25 (-185.5+160).
 *
 * A simpler approximation holds slowfactor constant.  Varies from 3.5 at
 * bitsize of 1000 to 4.0 at 4000 bits, so I found that 4.5 made a good
 * conservative approximation for values in this range.  Then heuristic
 * formula becomes cube root of size of prime in bits, times 24, minus 80.
 * This can be calculated pretty well in int arithmetic if we want to.
 */
static unsigned
elgExpBits (unsigned primebits)
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
 * Given a buffer of at least "maxesk" bytes, make an PgpESK
 * into it and return the size of the PgpESK, or <0.
 *
 * ElGamal encryption is a simple variant on non-interactove
 * Diffie-Hellman.  The recipient publishes g, p, and y = g**x mod p.
 * the sender picks a random xx, computes yy = g**xx mod p, and
 * the shared secret z = y**xx mod p = yy**x mod p = g**(x*xx) mod p, then
 * then sends z*m, where m is the message.  (Padded in the usual
 * PKCS manner.)
 */
static int
elgEncrypt(struct PgpPubKey const *pubkey, byte const *key,
           size_t keylen, byte *esk, size_t *esklen,
           struct PgpRandomContext const *rc, PgpVersion version)
{
	struct ELGpub const *pub = (struct ELGpub *)pubkey->priv;
	struct BigNum xx;		/* Random exponent */
	struct BigNum yy;		/* g**xx mod p */
	struct BigNum z;		/* y**xx mod p */
	unsigned t;
	unsigned xxbits, pbytes;
	int i;

	/* We don't need this argument, although other algorithms may... */
	(void)version;
	
	bnBegin(&xx);
	bnBegin(&yy);
	bnBegin(&z);

	ASSERTELG(pubkey->pkAlg);
	t = bnBits(&pub->p);
	if (t > 0xffff)
		return PGPERR_PUBKEY_TOOBIG;
	pbytes = (t + 7) / 8;
	if (keylen > pbytes)
		return PGPERR_PUBKEY_TOOSMALL; /* data too big for pubkey */

	/* Add checksum to key, place temporarily in esk buffer */
	t = 0;
	esk[0] = key[0];
	for (i = 1; i < (int)keylen; i++)
		t += esk[i] = key[i];
	esk[keylen] = (byte)(t >> 8 & 255);
	esk[keylen+1] = (byte)(t & 255);

	/* Choose the random xx value to be used for this encryption */
	xxbits = pgpDiscreteLogExponentBits(bnBits(&pub->p))*3/2;
	if (pgpBnGenRand(&xx, rc, xxbits, 0, 0, xxbits) < 0)
		goto nomem;

	/* Do the two exponentiations necessary to compute yy and z */
	if (bnExpMod(&yy, &pub->g, &xx, &pub->p) < 0 ||
	    bnExpMod(&z,  &pub->y, &xx, &pub->p) < 0)
		goto nomem;

	/* Re-use xx to hold the PKCS-padded conventional key */
	if (pgpPKCSPack(&xx, esk, keylen+2, PKCS_PAD_ENCRYPTED, pbytes, rc)<0)
		goto nomem;
	pgpAssert (bnCmp(&xx, &pub->p) < 0);

	/* Compute xx*z mod p, the encrypted session key we will transmit */
	if (bnMul(&z, &z, &xx) < 0 ||
	    bnMod(&z, &z, &pub->p) < 0)
		goto nomem;

	/* Pack the two values into the esk, first yy and then k*z */
	t  = pgpBnPutPlain(&yy, esk);
	t += pgpBnPutPlain(&z, esk+t);
	if (esklen)
		*esklen = (size_t)t;
	
	i = 0;
	goto done;
nomem:
	i = PGPERR_NOMEM;
done:
	bnEnd(&z);
	bnEnd(&yy);
	bnEnd(&xx);
	return i;
}

#ifdef ELGSIGS
/*
 * Helper routine to pack a hash into a buffer with DER prefix
 * Return size of packed data
 */
static unsigned
elgPack (byte *buf, unsigned buflen, struct PgpHash const *h, byte const *hash)
{
	unsigned t = h->DERprefixsize + h->hashsize;
	pgpAssert (t <= buflen);
	memcpy(buf, h->DERprefix, h->DERprefixsize);
	memcpy(buf+h->DERprefixsize, hash, h->hashsize);
	return t;
}
#endif

/*
 * Return 1 if (sig,siglen) is a valid MPI which signs
 * (hash, hashlen).
 * Not implementing ElGamal signatures, using DSS.
 */
static int
elgVerify(struct PgpPubKey const *pubkey, int sigtype, byte const *sig,
	size_t siglen, struct PgpHash const *h, byte const *hash)
{
#ifndef ELGSIGS
	(void)pubkey;
	(void)sigtype;
	(void)sig;
	(void)siglen;
	(void)h;
	(void)hash;
	return PGPERR_PUBKEY_UNIMP;
#else
	/* Check an El Gamal signature */
	struct ELGpub const *pub = (struct ELGpub *)pubkey->priv;
	struct BigNum a,b,yaab;
	byte buf[64];	/* largest hash size + DER prefix */
	int i;
	size_t off;
	unsigned t;

	(void)sigtype;

	bnBegin(&a);
	bnBegin(&b);
	bnBegin(&yaab);

	ASSERTELG(pubkey->pkAlg);

	/* sig holds two values.  Get first, a, from sig. */
	off = 0;
	i = pgpBnGetPlain(&a, sig+off, siglen-off);
	if (i <= 0)
		goto fail;
	/* Get 2nd value, b, from SIG */
	off += i;
	i = pgpBnGetPlain(&b, sig+off, siglen-off);
	if (i <= 0)
		goto fail;
	off += i;
	if (off != siglen) {
		i = PGPERR_SIG_TOOLONG;
		goto done;
	}
	
	/* Compute y**a * a**b, put in a */
	if (bnExpMod(&yaab, &pub->y, &a, &pub->p) < 0 ||
	    bnExpMod(&a, &a, &b, &pub->p)         < 0 ||
	    bnMul(&yaab, &a, &yaab)               < 0 ||
	    bnMod(&yaab, &yaab, &pub->p)          < 0)
		goto nomem;

	/* Reconstruct PKCS packed hash as b */
	t = elgPack(buf, sizeof(buf), h, hash);
	i = pgpPKCSPack(&b, buf, t, PKCS_PAD_SIGNED, (bnBits(&pub->p)+7)/8,
			(struct PgpRandomContext const *)NULL);
	memset(buf, 0, t);
	if (i < 0)
		goto nomem;

	/* Calculate g**M, leave in a */
	if (bnExpMod(&a, &pub->g, &b, &pub->p) < 0)
		goto nomem;

	/* Compare y**a * a**b with g**M, should be equal */
	i = bnCmp(&a, &yaab) == 0;
	goto done;
nomem:
	i = PGPERR_NOMEM;
	goto done;
fail:
	if (!i)
		i = PGPERR_SIG_TOOSHORT;
	goto done;
done:
	bnEnd(&yaab);
	bnEnd(&b);
	bnEnd(&a);

	return i;
#endif
}

/*
 * Turn a PgpPubKey into the algorithm-specific parts of a public key.
 * A public key's ELG-specific part is:
 *
 *  0      2+i  MPI for prime
 * 2+i     2+t  MPI for generator
 * 4+i+t   2+u	MPI for public key
 * 6+i+t+u
 */
static size_t
elgPubBufferLength(struct PgpPubKey const *pubkey)
{
	struct ELGpub const *pub = (struct ELGpub *)pubkey->priv;

	return 6 + (bnBits(&pub->p)+7)/8 + (bnBits(&pub->g)+7)/8 +
		(bnBits(&pub->y)+7)/8;
}

static void
elgPubToBuffer(struct PgpPubKey const *pubkey, byte *buf)
{
	struct ELGpub const *pub = (struct ELGpub *)pubkey->priv;
	unsigned off;

	off = 0;
	off += pgpBnPutPlain(&pub->p, buf+off);
	off += pgpBnPutPlain(&pub->g, buf+off);
	off += pgpBnPutPlain(&pub->y, buf+off);
}


/* A little helper function that's used twice */
static void
elgFillPubkey(struct PgpPubKey *pubkey, struct ELGpub *pub)
{
	pubkey->next	 = NULL;
	pubkey->pkAlg	 = PGP_PKALG_ELGAMAL;
	pubkey->priv	 = pub;
	pubkey->destroy  = elgPubDestroy;
#if 0
	pubkey->id8      = elgPubId8;
#endif
	pubkey->maxesk   = elgMaxesk;
	pubkey->encrypt  = elgEncrypt;
	pubkey->verify   = elgVerify;
	pubkey->bufferLength  = elgPubBufferLength;
	pubkey->toBuffer = elgPubToBuffer;
}


/*
 * Turn the algorithm-specific parts of a public key into a PgpPubKey
 * structure.  A public key's ELG-specific part is:
 *
 *  0      2+i  MPI for prime
 * 2+i     2+t  MPI for generator
 * 4+i+t   2+u	MPI for public key
 * 6+i+t+u
 */
struct PgpPubKey *
elgPubFromBuf(byte const *buf, size_t size, int *error)
{
	struct PgpPubKey *pubkey;
	struct ELGpub *pub;
	unsigned i, t, u;
	int v;

	bnInit();

	v = pgpBnParse(buf, size, 3, &i, &t, &u);
	if (v < 0) {
		*error = v;
		return NULL;
	}
	if ((buf[t-1] & 1) == 0) {	/* Too small or even prime p */
		*error = PGPERR_KEY_MPI;
		return NULL;
	}

	pub = (struct ELGpub *)pgpMemAlloc(sizeof(*pub));
	if (pub) {
		pubkey = (struct PgpPubKey *)pgpMemAlloc(sizeof(*pubkey));
		if (pubkey) {
			bnBegin(&pub->p);
			bnBegin(&pub->g);
			bnBegin(&pub->y);
			if (bnInsertBigBytes(&pub->p, buf+i+2, 0, t-i-2) >= 0
			    && bnInsertBigBytes(&pub->g, buf+t+2, 0,
						u-t-2) >= 0
			    && bnInsertBigBytes(&pub->y, buf+u+2, 0,
						v-u-2) >= 0)
			{
				elgFillPubkey(pubkey, pub);

				*error = 0;
				return pubkey;
			}
			/* Failed = clean up and return NULL */
			bnEnd(&pub->p);
			bnEnd(&pub->g);
			bnEnd(&pub->y);
			pgpMemFree(pubkey);
		}
		pgpMemFree(pub);
	}
	*error = PGPERR_NOMEM;
	return NULL;
}

/*
 * Return the size of the public portion of a key buffer.
 */
int
elgPubKeyPrefixSize(byte const *buf, size_t size)
{
	return pgpBnParse(buf, size, 3, NULL, NULL, NULL);
}


/** Secret key functions **/

static void
elgSecDestroy(struct PgpSecKey *seckey)
{
	struct ELGsecPlus *sec = (struct ELGsecPlus *)seckey->priv;

	ASSERTELG(seckey->pkAlg);
	bnEnd(&sec->s.p);
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
elgSecId8(struct PgpSecKey const *seckey, byte *buf)
{
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;

	ASSERT (seckey->pkAlg);
	bnExtractBigBytes(&sec->s.p, buf, 0, 8);
}
#endif

/*
 * Generate a PgpPubKey from a PgpSecKey
 */
static struct PgpPubKey *
elgPubkey(struct PgpSecKey const *seckey)
{
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;
	struct PgpPubKey *pubkey;
	struct ELGpub *pub;

	ASSERTELG(seckey->pkAlg);
	pub = (struct ELGpub *)pgpMemAlloc(sizeof(*pub));
	if (pub) {
		pubkey = (struct PgpPubKey *)pgpMemAlloc(sizeof(*pubkey));
		if (pubkey) {
			bnBegin(&pub->p);
			bnBegin(&pub->g);
			bnBegin(&pub->y);
			if (bnCopy(&pub->p, &sec->s.p) >= 0
			    && bnCopy(&pub->g, &sec->s.g) >= 0
			    && bnCopy(&pub->y, &sec->s.y) >= 0)
			{
				elgFillPubkey(pubkey, pub);
				pubkey->pkAlg = seckey->pkAlg;
				memcpy(pubkey->keyID, seckey->keyID,
				       sizeof(pubkey->keyID));
				return pubkey;
			}
			/* Failed = clean up and return NULL */
			bnEnd(&pub->p);
			bnEnd(&pub->g);
			bnEnd(&pub->y);
			pgpMemFree(pubkey);
		}
		pgpMemFree(pub);
	}
	return NULL;
}

/*
 * Yes, there *is* a reason that this is a function and no a variable.
 * On a hardware device with an automatic timeout,
 * it actually might need to do some work to find out.
 */
static int
elgIslocked(struct PgpSecKey const *seckey)
{
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;

	ASSERTELG(seckey->pkAlg);
	return sec->locked;
}

/*
 * Try to decrypt the secret key wih the given passphrase.  Returns >0
 * if it was the correct passphrase. =0 if it was not, and <0 on error.
 * Does not alter the key even if it's the wrong passphrase and already
 * unlocked.  A NULL passphrae will work if the key is unencrypted.
 *
 * A (secret) key's ELG-specific part is:
 *
 *  0                2+u  MPI for prime
 *  2+u              2+v  MPI for generator
 *  4+u+v	     2+w  MPI for public key
 *  6+u+v+w          1    Encryption algorithm (0 for none, 1 for IDEA)
 *  7+u+v+w          t    Encryption IV: 0 or 8 bytes
 *  7+t+u+v+w        2+x  MPI for x (discrete log of public key)
 *  9+t+u+v+w+x      2    Checksum
 * 11+t+u+v+w+x
 *
 * Actually, that's the old-style, if pgpS2KoldVers is true.
 * If it's false, the algorithm is 255, and is followed by the
 * algorithm, then the (varaible-length, self-delimiting)
 * string-to-key descriptor.
 */

static int
elgUnlock(struct PgpSecKey *seckey, struct PgpEnv const *env,
	  char const *phrase, size_t plen)
{
	struct ELGsecPlus *sec = (struct ELGsecPlus *)seckey->priv;
	struct BigNum x;
	struct PgpCfbContext *cfb = NULL;
	unsigned v;
	unsigned alg;
	unsigned checksum;
	int i;

	ASSERTELG(seckey->pkAlg);
	bnInit();

	/* Check packet for basic consistency */
	i = pgpBnParse(sec->cryptkey, sec->cklen, 3, &v, NULL, NULL, NULL);
	if (i < 0)
		return i;

	/* OK, read the public data */
	i = pgpBnGetPlain(&sec->s.p, sec->cryptkey+v, sec->cklen-v);
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
	if (!phrase != !alg)
		return 0;

	i = pgpCipherSetup(sec->cryptkey + v, sec->cklen - v, phrase, plen,
	                   env, &cfb);
	if (i < 0)
		goto done;
	v += i;

	checksum = 0;
	bnBegin(&x);
	i = pgpBnGetNew(&x, sec->cryptkey + v, sec->cklen - v, cfb, &checksum);
	if (i <= 0)
		goto badpass;
	v += i;
	if (bnCmp(&x, &sec->s.p) >= 0)
		goto badpass;	/* Wrong passphrase: x must be < p */

	/* Check that we ended in the right place */
	if (sec->cklen - v != 2) {
		i = PGPERR_KEY_LONG;
		goto done;
	}
	checksum &= 0xffff;
	if (checksum != pgpChecksumGetNew(sec->cryptkey + v, cfb))
		goto badpass;

	/*
	 * Note that the "nomem" case calls bnEnd()
	 * more than once, but this is guaranteed harmless.
 	 */
	if (bnCopy(&sec->s.x, &x) < 0)
		goto nomem;
	bnEnd(&x);

	i = 1;	/* Decrypted! */
	sec->locked = 0;
	if (cfb)
		pgpCfbDestroy (cfb);
	return 1;	/* Decrypted */

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
elgLock(struct PgpSecKey *seckey)
{
	struct ELGsecPlus *sec = (struct ELGsecPlus *)seckey->priv;

	ASSERTELG(seckey->pkAlg);
	sec->locked = 1;
	/* bnEnd is documented as also doing a bnBegin */
	bnEnd(&sec->s.x);
}

/*
 * Return the size of the buffer needed, worst-case, for the decrypted
 * output.  A trivially padded key (random padding length = 0)
 * can just be 0 2 0 <key>.
 */
static size_t
elgMaxdecrypted(struct PgpSecKey const *seckey)
{
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;
	size_t size;

	ASSERTELG(seckey->pkAlg);
	size = (bnBits(&sec->s.p)+7)/8;
	return size < 3 ? 0 : size-3;
}

/*
 * Try to decrypt the given esk.  If the key is locked, try the given
 * passphrase.  It may or may not leave the key unlocked in such a case.
 * (Some hardware implementations may insist on a password per usage.)
 */
static int
elgDecrypt(struct PgpSecKey *seckey, struct PgpEnv const *env,
	   int esktype, byte const *esk, size_t esklen,
	   byte *key, size_t *keylen, char const *phrase,
	   size_t plen)
{
	struct ELGsecPlus *sec = (struct ELGsecPlus *)seckey->priv;
	struct BigNum k, yy;
	int i, j;
	unsigned t, pbytes;
	size_t len;
	size_t off;

	(void)esktype;

	ASSERTELG(seckey->pkAlg);
	if (sec->locked) {
		i = elgUnlock(seckey, env, phrase, plen);
		if (i <= 0)
			return i ? i : PGPERR_KEY_ISLOCKED;
		pgpAssert(!sec->locked);
	}

	bnBegin(&k);
	bnBegin(&yy);

	/* ESK holds two values.  Get first, yy, from ESK. */
	off = 0;
	i = pgpBnGetPlain(&yy, esk+off, esklen-off);
	if (i <= 0)
		goto fail;
	/* Get 2nd value, k, from ESK */
	off += i;
	i = pgpBnGetPlain(&k, esk+off, esklen-off);
	if (i <= 0)
		goto fail;
	off += i;
	if (off != esklen) {
		i = PGPERR_ESK_TOOLONG;
		goto done;
	}
	
	/* Calculate yy = yy**x mod p, which is what is multiplied by k */
	if (bnExpMod(&yy, &yy, &sec->s.x, &sec->s.p) < 0)
		goto nomem;

	/* Set k = k / (new)yy , revealing PKCS padded session key */
	if (bnInv(&yy, &yy, &sec->s.p) < 0 ||
		bnMul(&k, &k, &yy) < 0 ||
		bnMod(&k, &k, &sec->s.p) < 0) {
			goto nomem;
	}
	bnEnd(&yy);
	
	/* k is now suitable for unpacking */
	pbytes = (bnBits(&sec->s.p) + 7) / 8;
	len = elgMaxdecrypted(seckey);
	i = pgpPKCSUnpack (key, len, &k, PKCS_PAD_ENCRYPTED, pbytes);
	bnEnd(&k);
	if (i < 0)
		return i;

	/* Check checksum (should this be here?) */
	t = 0;
	for (j = 1; j < i-2; j++)
		t += key[j];
	if (t != ((unsigned)key[i-2]<<8) + key[i-1])
		return PGPERR_PK_CORRUPT;
	memset(key+i-2, 0, 2);

	/* The actual key */
	if (keylen)
		*keylen = (size_t)i-2;
	
	return 0;

fail:
	if (!i)
		i = PGPERR_ESK_TOOSHORT;
	goto done;
nomem:
	i = PGPERR_NOMEM;
	goto done;
done:
	bnEnd(&yy);
	bnEnd(&k);
	return 0;
}

static size_t
elgMaxsig(struct PgpSecKey const *seckey, PgpVersion version)
{
#ifndef ELGSIGS
	(void)seckey;
	(void)version;
	return PGPERR_PUBKEY_UNIMP;
#else
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;

	(void)version;
	ASSERTELG(seckey->pkAlg);
	return 2*((bnBits(&sec->s.p)+7)/8 + 2);
#endif
}

static int
elgSign(struct PgpSecKey *seckey, struct PgpHash const *h, byte const *hash,
	byte *sig, size_t *siglen, struct PgpRandomContext const *rc,
	PgpVersion version)
{
#ifndef ELGSIGS
	(void)seckey;
	(void)h;
	(void)hash;
	(void)sig;
	(void)siglen;
	(void)rc;
	(void)version;
	return PGPERR_PUBKEY_UNIMP;
#else
	/* Calculate an El Gamal signature */

	struct ELGsecPlus *sec = (struct ELGsecPlus *)seckey->priv;
	struct BigNum pm1, x1, a, bn, xk, xa;
	unsigned t;
	unsigned x1bits;
	int i;

	/* We don't need this argument, although other algorithms may... */
	(void)version;

	ASSERTELG(seckey->pkAlg);
	if (sec->locked)
		return PGPERR_KEY_ISLOCKED;
	if (h->DERprefixsize + h->hashsize  > elgMaxsig(seckey, version))
		return PGPERR_PUBKEY_TOOSMALL;

	bnBegin(&pm1);
	bnBegin(&x1);
	bnBegin(&a);
	bnBegin(&bn);
	bnBegin(&xk);
	bnBegin(&xa);

	/* Some calculations done mod p-1 */
	if (bnCopy (&pm1, &sec->s.p) < 0 ||
	    bnSubQ (&pm1, 1) < 0)
		goto nomem;

	/*
	 * Choose the random x1 value to be used for this signature.
	 * Make sure it is relatively prime to p-1
	 */
	x1bits = pgpDiscreteLogExponentBits(bnBits(&sec->s.p))*3/2;
	do {
		if (pgpBnGenRand(&x1, rc, x1bits, 0, 1, x1bits-1) < 0 ||
		    bnGcd(&a, &x1, &pm1) < 0)
			goto nomem;
	} while (bnBits(&a) > 1);

	/* Raise g to x1 power to get a */
	if (bnExpMod(&a, &sec->s.g, &x1, &sec->s.p) < 0)
		goto nomem;
	
	/* Calculate x**-1 mod p-1, keep in x1 */
	if (bnInv(&x1, &x1, &pm1) < 0)
		goto nomem;

	/* Pack message hash M into buffer bn (use sig temporarily) */
	t = elgPack(sig, elgMaxsig(seckey, version), h, hash);
	if (pgpPKCSPack(&bn, sig, t, PKCS_PAD_SIGNED,
			(bnBits(&sec->s.p)+7)/8, rc) < 0)
		goto nomem;

	/* Calculate b = (M-xa)/k mod p-1, put it in bn */
	if (bnMul(&xa, &sec->s.x, &a) < 0 ||
	    bnMod(&xa, &xa, &pm1) < 0)
		goto nomem;

	if (bnCmp(&bn, &xa) < 0) {
		if (bnAdd(&bn, &pm1) < 0)
			goto nomem;
	}
	bnSub(&bn, &xa);
	if (bnMul(&bn, &bn, &x1) < 0 ||
	    bnMod(&bn, &bn, &pm1) < 0)
		goto nomem;

	/* Success, now just pack into sig buffer */
	t  = pgpBnPutPlain(&a, sig);
	t += pgpBnPutPlain(&bn, sig+t);
	if (siglen)
		*siglen = (size_t)t;

	i = 0;
	goto done;

nomem:
	i = PGPERR_NOMEM;
	/* fall through */
done:
	bnEnd(&xa);
	bnEnd(&bn);
	bnEnd(&pm1);
	bnEnd(&x1);
	bnEnd(&a);
	bnEnd(&bn);
	bnEnd(&xk);
	return i;
#endif
}

/*
 * Re-encrypt a PgpSecKey with a new passphrase.
 * A secret key is, after a non-specific prefix:
 *  0       1    Version (= 2 or 3)
 *  1       4    Timestamp
 *  5       2    Validity (=0 at present)
 *  7       1    Algorithm (=PGP_PKALG_ELGAMAL for ELG)
 * The following:
 *  0                2+u  MPI for prime
 *  2+u              2+v  MPI for generator
 *  4+u+v	     2+w  MPI for public key
 *  6+u+v+w          1    Encryption algorithm (0 for none, 1 for IDEA)
 *  7+u+v+w          t    Encryption IV: 0 or 8 bytes
 *  7+t+u+v+w        2+x  MPI for x (discrete log of public key)
 *  9+t+u+v+w+x      2    Checksum
 * 11+t+u+v+w+x
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
elgChangeLock(struct PgpSecKey *seckey, struct PgpEnv const *env,
	struct PgpRandomContext const *rc, char const *phrase, size_t plen)
{
	struct ELGsecPlus *sec = (struct ELGsecPlus *)seckey->priv;
	struct PgpStringToKey *s2k = NULL;	/* Shut up warnings */
	struct PgpCipher const *cipher = NULL;	/* Shut up warnings */
	struct PgpCfbContext *cfb = NULL;	/* This is realy needed */
	byte *p;
	int oldf = 0;				/* Shut up warnings */
	unsigned len;
	unsigned checksum;

	ASSERTELG(seckey->pkAlg);
	if (sec->locked)
		return PGPERR_KEY_ISLOCKED;

	len = bnBytes(&sec->s.p) + bnBytes(&sec->s.g) +
	      bnBytes(&sec->s.y) + bnBytes(&sec->s.x) + 11;
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
elgSecBufferLength(struct PgpSecKey const *seckey)
{
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;

	return sec->cklen;
}

static void
elgSecToBuffer(struct PgpSecKey const *seckey, byte *buf)
{
	struct ELGsecPlus const *sec = (struct ELGsecPlus *)seckey->priv;

	memcpy(buf, sec->cryptkey, sec->cklen);

	/* Return only algorithm-dependent portion */
}


/* Fill in secret key structure */
static void
elgFillSecKey(struct PgpSecKey *seckey, struct ELGsecPlus *sec)
{
	seckey->pkAlg	     = PGP_PKALG_ELGAMAL;
	seckey->priv	     = sec;
	seckey->destroy      = elgSecDestroy;
#if 0
	seckey->id8          = elgSecId8;
#endif
	seckey->pubkey       = elgPubkey;
	seckey->islocked     = elgIslocked;
	seckey->unlock       = elgUnlock;
	seckey->lock         = elgLock;
	seckey->maxdecrypted = elgMaxdecrypted;
	seckey->decrypt      = elgDecrypt;
	seckey->maxsig       = elgMaxsig;
	seckey->sign         = elgSign;
	seckey->changeLock   = elgChangeLock;
	seckey->bufferLength = elgSecBufferLength;
	seckey->toBuffer     = elgSecToBuffer;
}


struct PgpSecKey *
elgSecFromBuf(byte const *buf, size_t size, int *error)
{
	struct PgpSecKey *seckey;
	struct ELGsecPlus *sec;
	byte *cryptk;

	bnInit();
	cryptk = (byte *)pgpMemAlloc(size);
	if (cryptk) {
		sec = (struct ELGsecPlus *)pgpMemAlloc(sizeof(*sec));
		if (sec) {
			seckey = (struct PgpSecKey *)
				pgpMemAlloc(sizeof(*seckey));
			if (seckey) {
				memcpy(cryptk, buf, size);
				bnBegin(&sec->s.p);
				bnBegin(&sec->s.g);
				bnBegin(&sec->s.y);
				bnBegin(&sec->s.x);
				sec->cryptkey = cryptk;
				sec->cklen = sec->ckalloc = size;
				sec->locked = 1;
				/* We only need this to try unlocking... */
				seckey->pkAlg = PGP_PKALG_ELGAMAL;
				seckey->priv = sec;
				
				if (elgUnlock(seckey, NULL, NULL, 0) >= 0) {
					elgFillSecKey(seckey, sec);
					*error = 0;
					return seckey;	/* Success! */
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
	*error = PGPERR_NOMEM;
	return NULL;
}

/* Generate super-strong primes? (Warning: slow!) */
#ifndef ELG_GERMAIN
#define ELG_GERMAIN 0
#endif

/*
 * Generate an ELG secret key with prime of the specified number of bits.
 * Make callbacks to progress function periodically.
 * Secret key is returned in the unlocked form, with no passphrase set.
 * fastgen tells us to use canned primes if available.
 *
 * If ELG_GERMAIN is set to 1, we generate p such that (p-1)/2 is also
 * prime.  This takes long time.  (Pseudoprimality tests take time
 * cubic in the number of bits, and the number of tests needed is
 * linear in the number of bits, so it's quartic overall.  Searching
 * for Sophier Germain primes makes it quintic(!), although the constant
 * factor improves to make up for a lot of that.
 *
 * The alternative (which is really just as safe, really) is to generate
 * primes which have a large prime factor q about 10 bits shorter than
 * the requested length.  We will guarantee that 2 is a generator of a
 * subgroup with period at least q.
 *
 * If it is OK, we could speed it up more by generating multiple primes
 * qi such that p = 2*k*q1*q2*q3*...*qn + 1, and where each qi is greater
 * than 2**160 (or whatever exponent value we are using).  Again, once we
 * verify that 2's period is > 2k we know it as at least min(qi), which
 * should be long enough for prevention of discrete log attacks.
 *
 * A bit of theory: the average density of primes around n is 1/ln(n),
 * so the average gap between primes is ln(n).  However, the maximum
 * gap is ln(n)^2.  The fact that we're searching in steps other than
 * 1 doesn't matter - it'll take an average of ln(n) steps.
 *
 * So to produce a prime of the desired size, we should have p/q at
 * least ln(p) and to guarantee it, we need p/q = ln(p)^2.  This
 * means that we want
 * log2(ln(p))           < log2(p/q)         < log2(ln(p)^2)
 * log2(0.693*log2(p))   < log2(p) - log2(q) < 2 * log2(0.693 * log2(p))
 * log2(log2(p)) - 0.529 < log2(p) - log2(q) < 2 * log2(log2(p)) - 1.058
 *
 * At this point, it's safe to start getting crude, because if we're
 * only counting bits, 2^(bits(x)-1) <= x < 2^bits(x) <= 2*x, or
 * bits(x)-1 <= log2(x) < bits(x) <= log2(x)+1.  Another way of looking
 * at all this is that log2(x) is bits(x) - 0.5 +/- 0.5.
 * So let's split the difference and use 1.5 * bits(bits(p)) - 1 as the
 * difference in bits between p and q.
 *
 * See the discussion preceding the DSA keygen routine dsaSecGenerate
 * in pgpDSAKey.c for an explanation of the rcdummy random number
 * generator below.  It serves to limit leakage of the state of the
 * randpool into the public values generated as part of the key.
 */
struct PgpSecKey *
elgSecGenerate(unsigned bits, Boolean fastgen,
	struct PgpRandomContext const *rc,
	int progress(void *arg, int c), void *arg, int *error)
{
	struct PgpSecKey *seckey;
	struct ELGsecPlus *sec;
	struct PgpRandomContext *rcdummy = NULL;
	unsigned bits2;
#if !ELG_GERMAIN
	unsigned lengthdiff;
	struct BigNum q, h, e;
	int i;
#endif
	byte dummyseed[ELGDUMMYBITS/8];

	*error = 0;

	/* Initialize local pointers (simplify cleanup below) */
	seckey = NULL;
	sec = NULL;

	/* Allocate data structures */
	seckey = (struct PgpSecKey *)pgpMemAlloc(sizeof(*seckey));
	if (!seckey)
		goto memerror;
	sec = (struct ELGsecPlus *)pgpMemAlloc(sizeof(*sec));
	if (!sec)
		goto memerror;
	
	bnBegin(&sec->s.p);
	bnBegin(&sec->s.g);
	bnBegin(&sec->s.y);
	bnBegin(&sec->s.x);

	/* Use a fixed prime and generator if in our table */
	if (fastgen) {
		byte const *fixedp, *fixedg;
		size_t fixedplen, fixedglen;
		if (pgpElGfixed (bits, &fixedp, &fixedplen, &fixedg, &fixedglen) > 0) {
			if (progress)
				progress(arg, ' ');
			bnInsertBigBytes (&sec->s.p, fixedp, 0, fixedplen);
			if (progress)
				progress(arg, ' ');
			bnInsertBigBytes (&sec->s.g, fixedg, 0, fixedglen);
			goto choose_x;
		}
	}

	/* Set up local random number generator for p and q */
	rcdummy = pgpPseudoRandomCreate ();
	if (!rcdummy)
		goto memerror;
	pgpRandomGetBytes (rc, dummyseed, sizeof(dummyseed));
	pgpRandomAddBytes (rcdummy, dummyseed, sizeof(dummyseed));

#if ELG_GERMAIN
	/* Strong ("sophie germain") prime search */

	/* Find p - choose a starting place */
	if (pgpBnGenRand(&sec->s.p, rcdummy, bits, 0xC0, 3, bits-4) < 0)
		goto nomem;

	/* And search for a prime */
	if (bnGermainPrimeGen(&sec->s.p, 1, progress, arg) < 0)
		goto nomem;

	/* We have chosen p so 2 is a good choice for generator */
	if (bnSetQ(&sec->s.g, 2) < 0)
		goto nomem;

	/* Choose a random x of reasonable size as secret key */
	expbits = elgExpBits(bits);
	if (pgpBnGenRand(&sec->s.x, rc, expbits, 0, 0, expbits) < 0)
		goto nomem;

	/* And calculate g**x as public key */
	if (bnTwoExpMod(&sec->s.y, &sec->s.x, &sec->s.p) < 0)
		goto nomem;
#else /* !ELG_GERMAIN - the faster version */
	bnBegin(&q);
	bnBegin(&h);
	bnBegin(&e);

	/*
	 * Choose a random starting place for q, a bit less than p.
	 * (See function header comment above for the theory behind this.)
	 */
	lengthdiff = 0;
	for (bits2 = bits; bits2; bits2 >>= 1)
		lengthdiff++;
	lengthdiff += (lengthdiff+1)/2;
	bits2 = bits - lengthdiff;

	if (pgpBnGenRand(&q, rcdummy, bits2, 0x80, 1, bits2-2) < 0)
		goto nomem;
	/* And search for a prime */
	i = bnPrimeGen(&q, NULL, progress, arg, 0);
	if (i < 0)
		goto nomem;
	if (progress)
		progress(arg, ' ');

	/* ...and now a random start for p */
	(void)bnSetQ(&sec->s.p, 0);
	if (pgpBnGenRand(&sec->s.p, rcdummy, bits, 0xC0, 1, bits-bits2-3) < 0)
		goto nomem;

	/* Double q to make it a suitable stride */
	if (bnLShift(&q, 1) < 0)
		goto nomem;

	/* Set p = p - (p mod 2q) + 1, i.e. congruent to 1 mod 2q */
	if (bnMod(&h, &sec->s.p, &q) < 0)
		goto nomem;
	if (bnSub(&sec->s.p, &h) < 0 || bnAddQ(&sec->s.p, 1) < 0)
		goto nomem;

	/* This loop is very rarely executed */
retry:
	/* And search for a prime, 1+2kq for some k */
	i = bnPrimeGenStrong(&sec->s.p, &q, progress, arg);
	if (i < 0)
		goto nomem;
	if (progress)
		progress(arg, ' ');

	/* Now check two as g: first, find (p-1)/q = 2*((p-1)/(2*q)) */
	if (bnDivMod(&e, &h, &sec->s.p, &q) < 0 || bnLShift(&e, 1) < 0)
		goto nomem;
	/* e is now (p-1)/q, and h is the remainder (one!) */
	pgpAssert (bnBits(&h) == 1);

	/*
	 * Make sure 2**((p-1)/q) mod p is not small.  This should imply
	 * that the period of 2 as a generator has at least q as a factor,
	 * meaning it is very big.
	 * With (p-1)/q as small as it is, the chances are *excellent*
	 * that it will work.  If (p-1)/q were less than a few hundred
	 * 2**((p-1)/q) would be less than p and coundn't possibly be 1.
	 */
	if (bnTwoExpMod(&h, &e, &sec->s.p) < 0)
		goto nomem;
	if (bnBits(&h) < 2) {
		if (progress)
			progress(arg, ' ');
		goto retry;
	}
	bnEnd(&e);
	bnEnd(&h);
	bnEnd(&q);
#endif
	/* We have done things so 2 is a good choice for generator */
	if (bnSetQ(&sec->s.g, 2) < 0)
		goto nomem;

	/* May get here directly from above if fixed primes are used */
choose_x:

	/* Choose a random x of reasonable size as secret key */
	bits2 = pgpDiscreteLogExponentBits(bits)*3/2;
	if (pgpBnGenRand(&sec->s.x, rc, bits2, 0, 0, bits2) < 0)
		goto nomem;

	/* And calculate g**x as public key */
	if (bnExpMod(&sec->s.y, &sec->s.g, &sec->s.x, &sec->s.p) < 0)
		goto nomem;
	

	/* And that's it... success! */

	/* Fill in structs */
	sec->cryptkey = NULL;
	sec->ckalloc = sec->cklen = 0;
	sec->locked = 0;
	elgFillSecKey(seckey, sec);

	/* Fill in cryptkey structure, unencrypted */
	elgChangeLock (seckey, NULL, NULL, NULL, 0);

	goto done;

nomem:
#if !ELG_GERMAIN
	bnEnd(&e);
	bnEnd(&h);
	bnEnd(&q);
#endif
	bnEnd(&sec->s.p);
	bnEnd(&sec->s.g);
	bnEnd(&sec->s.y);
	bnEnd(&sec->s.x);
	/* Fall through */
memerror:
	pgpMemFree(seckey);
	pgpMemFree(sec);
	seckey = NULL;
	*error = PGPERR_NOMEM;

done:
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
