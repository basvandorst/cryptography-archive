/*
* pgpRSAKey.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRSAKey.c,v 1.9.2.3 1997/06/09 23:46:35 quark Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if (!NO_RSA)

#include "pgpDebug.h"
#include "pgpKeyMisc.h"
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

#ifndef NULL
#define NULL 0
#endif

#define ASSERTRSA(alg) pgpAssert((ALGMASK(alg))==PGP_PKALG_RSA || \
			(ALGMASK(alg))==PGP_PKALG_RSA_ENC || \
			(ALGMASK(alg))==PGP_PKALG_RSA_SIG)
#define ASSERTRSASIG(alg) pgpAssert((ALGMASK(alg))==PGP_PKALG_RSA || \
				(ALGMASK(alg))==PGP_PKALG_RSA_SIG)
#define ASSERTRSAENC(alg) pgpAssert((ALGMASK(alg))==PGP_PKALG_RSA || \
				(ALGMASK(alg))==PGP_PKALG_RSA_ENC)

/** Public key functions **/

static void
rsaPubDestroy(struct PgpPubKey *pubkey)
	{
			struct RSApub *pub = (struct RSApub *)pubkey->priv;

			ASSERTRSA(pubkey->pkAlg);
		
			bnEnd(&pub->n);
			bnEnd(&pub->e);
			memset(pub, 0, sizeof(pub));
			pgpMemFree(pub);
			memset(pubkey, 0, sizeof(pubkey));
			pgpMemFree(pubkey);
	}

#if 0
static void
rsaPubId8(struct PgpPubKey const *pubkey, byte *buf)
	{
			struct RSApub const *pub = (struct RSApub *)pubkey->priv;

ASSERTRSA(pubkey->pkAlg);
bnExtractBigBytes(&pub->n, buf, 0, 8);
}
#endif

/* Return the largest possible PgpESK size for a given key */
static size_t
rsaMaxesk(struct PgpPubKey const *pubkey, PgpVersion version)
	{
			struct RSApub const *pub = (struct RSApub *)pubkey->priv;

			(void)version;
			ASSERTRSAENC(pubkey->pkAlg);
			return 2 + (bnBits(&pub->n)+7)/8;
	}

/*
* Given a buffer of at least "maxesk" bytes, make an PgpESK
* into it and return the size of the PgpESK, or <0.
*/
static int
rsaEncrypt(struct PgpPubKey const *pubkey, byte const *key,
size_t keylen, byte *esk, size_t *esklen,
struct PgpRandomContext const *rc, PgpVersion version)
{
struct RSApub const *pub = (struct RSApub *)pubkey->priv;
struct BigNum bn;
unsigned t;
int i;

/* We don't need these arguments, although other algorithms may... */
(void)rc;
(void)version;

ASSERTRSAENC(pubkey->pkAlg);
t = bnBits(&pub->n);
if (t > 0xffff)
	return PGPERR_PUBKEY_TOOBIG;
if (keylen > t)
	return PGPERR_PUBKEY_TOOSMALL; /* data too big for pubkey */

/* Add checksum to key, place temporarily in esk buffer */
t = 0;
esk[0] = key[0];
for (i = 1; i < (int)keylen; i++)
	t += esk[i] = key[i];
esk[keylen] = (byte)(t >> 8 & 255);
esk[keylen+1] = (byte)(t & 255);

			bnBegin(&bn);
			i = rsaPublicEncrypt(&bn, esk, keylen+2, pub, rc);
			if (i < 0) {
					bnEnd(&bn);
					memset(esk, 0, keylen+2);
					return i;
			}
		
			t = pgpBnPutPlain(&bn, esk);
			bnEnd(&bn);

			if (esklen)
				 *esklen = (size_t)t;
			return 0;
	}

/*
* Return 1 if (sig,siglen) is a valid MPI which signs
* hash, of type h. Check the DER-encoded prefix and the
* hash itself.
*/
static int
rsaVerify(struct PgpPubKey const *pubkey, int sigtype, byte const *sig,
			size_t siglen, struct PgpHash const *h, byte const *hash)
	{
			struct RSApub const *pub = (struct RSApub *)pubkey->priv;
			struct BigNum bn;
			byte buf[64];	/* largest hash size + DER prefix */
			int i;
			unsigned t;

(void)sigtype;

ASSERTRSASIG(pubkey->pkAlg);
t = ((unsigned)sig[0]<<8) + sig[1];
t = (t+7)/8;
if (siglen != t+2)
	return siglen < t+2 ? PGPERR_SIG_TOOSHORT
				: PGPERR_SIG_TOOLONG;
bnBegin(&bn);
if (bnInsertBigBytes(&bn, sig+2, 0, t) < 0) {
	bnEnd(&bn);
	return PGPERR_NOMEM;
}
i = rsaPublicDecrypt(buf, sizeof(buf), &bn, pub);
bnEnd(&bn);

if (i >= 0) {
				/* Check that the returned data is correct */
				t = h->DERprefixsize;
				i = (size_t)i <= sizeof(buf)
				&& (unsigned)i == h->hashsize + t
				&& memcmp(buf, h->DERprefix, t) == 0
				&& memcmp(buf+t, hash, h->hashsize) == 0;
		}
memset(buf, 0, sizeof(buf));
return i;
}

/*
* Turn a PgpPubKey into the algorithm-specific parts of a public key.
* A public key's RSA-specific part is:
*
* 0	2+i MPI for modulus
* 2+i	2+t MPI for exponent
* 4+i+t
*/
static size_t
rsaPubBufferLength(struct PgpPubKey const *pubkey)
{
	struct RSApub const *pub = (struct RSApub *)pubkey->priv;

	return 4 + (bnBits(&pub->n)+7)/8 + (bnBits(&pub->e)+7)/8;
}

static void
rsaPubToBuffer(struct PgpPubKey const *pubkey, byte *buf)
	{
			struct RSApub const *pub = (struct RSApub *)pubkey->priv;
			unsigned i, t;

			i = bnBits(&pub->n);
			pgpAssert(i <= 0xffff);
			buf[0] = (byte)(i >> 8);
			buf[1] = (byte)i;
			i = (i+7)/8;
			bnExtractBigBytes(&pub->n, buf+2, 0, i);
			t = bnBits(&pub->e);
			pgpAssert(t <= 0xffff);
			buf[2+i] = (byte)(t >> 8);
			buf[3+i] = (byte)t;
			t = (t+7)/8;
			bnExtractBigBytes(&pub->e, buf+4+i, 0, t);
	}


/* A little helper function that's used twice */
static void
rsaFillPubkey(struct PgpPubKey *pubkey, struct RSApub *pub)
	{
			pubkey->next	 = NULL;
			pubkey->pkAlg	= PGP_PKALG_RSA;
			pubkey->priv	 = pub;
			pubkey->destroy = rsaPubDestroy;
#if 0
			pubkey->id8 = rsaPubId8;
#endif
			pubkey->maxesk = rsaMaxesk;
			pubkey->encrypt = rsaEncrypt;
			pubkey->verify = rsaVerify;
			pubkey->bufferLength = rsaPubBufferLength;
			pubkey->toBuffer = rsaPubToBuffer;
	}


/*
* Turn the algorithm-specific parts of a public key into a PgpPubKey
* structure. A public key's RSA-specific part is:
*
* 0	2+i MPI for modulus
* 2+i	2+t MPI for exponent
* 4+i+t
*/
struct PgpPubKey *
rsaPubFromBuf(byte const *buf, size_t size, int *error)
	{
				struct PgpPubKey *pubkey;
				struct RSApub *pub;
				int err = PGPERR_NOMEM;
				unsigned i, t;

				bnInit();

				if (size < 4)
					return NULL;

				i = ((unsigned)buf[0] << 8) + buf[1];
				if (!i || buf[2] >> ((i-1) & 7) != 1) {
					*error = PGPERR_KEY_MODMPI;
					return NULL;	/* Bad bit length */
				}
				i = (i+7)/8;
				if (size < 4+i) {
					*error = PGPERR_KEY_SHORT;
					return NULL;
				}
				if ((buf[1+i] & 1) == 0) {	/* Too small or even modulus */
					*error = PGPERR_KEY_MODEVEN;
					return NULL;
				}
				t = ((unsigned)buf[2+i] << 8) + buf[3+i];
				if (!t || buf[4+i] >> ((t-1) & 7) != 1) {
					*error = PGPERR_KEY_EXPMPI;
					return NULL;	/* Bad bit length */
				}
				t = (t+7)/8;
				if (size < 4+i+t) {
					*error = PGPERR_KEY_SHORT;
					return NULL;
				}

				pub = (struct RSApub *)pgpMemAlloc(sizeof(*pub));
				if (pub) {
					pubkey = (struct PgpPubKey *)pgpMemAlloc(sizeof(*pubkey));
					if (pubkey) {
							bnBegin(&pub->n);
							bnBegin(&pub->e);
							if (bnInsertBigBytes(&pub->n, buf+2, 0, i) >= 0
							&& bnInsertBigBytes(&pub->e, buf+4+i, 0, t) >= 0)
							{
									if (rsaKeyTooBig (pub, NULL)) {
										err = PGPERR_KEY_UNSUPP;
									} else {
										rsaFillPubkey(pubkey, pub);
										*error = 0;
										return pubkey;
	}
}
/* Failed = clean up and return NULL */
bnEnd(&pub->n);
bnEnd(&pub->e);
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
rsaPubKeyPrefixSize(byte const *buf, size_t size)
{
	return pgpBnParse(buf, size, 2, NULL, NULL);
}



/** Secret key functions **/

static void
rsaSecDestroy(struct PgpSecKey *seckey)
	{
			struct RSAsecPlus *sec = (struct RSAsecPlus *)seckey->priv;

			ASSERTRSA(seckey->pkAlg);
			bnEnd(&sec->s.n);
			bnEnd(&sec->s.e);
			bnEnd(&sec->s.d);
			bnEnd(&sec->s.p);
			bnEnd(&sec->s.q);
			bnEnd(&sec->s.u);
			memset(sec->cryptkey, 0, sec->ckalloc);
			pgpMemFree(sec->cryptkey);
			memset(sec, 0, sizeof(sec));
			pgpMemFree(sec);
			memset(seckey, 0, sizeof(seckey));
			pgpMemFree(seckey);
	}

#if 0
static void
rsaSecId8(struct PgpSecKey const *seckey, byte *buf)
	{
			struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;

			ASSERTRSA(seckey->pkAlg);
			bnExtractBigBytes(&sec->s.n, buf, 0, 8);
}
#endif

/*
* Generate a PgpPubKey from a PgpSecKey
*/
static struct PgpPubKey *
rsaPubkey(struct PgpSecKey const *seckey)
	{
			struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;
			struct PgpPubKey *pubkey;
			struct RSApub *pub;

			ASSERTRSA(seckey->pkAlg);
			pub = (struct RSApub *)pgpMemAlloc(sizeof(*pub));
			if (pub) {
					pubkey = (struct PgpPubKey *)pgpMemAlloc(sizeof(*pubkey));
					if (pubkey) {
							bnBegin(&pub->n);
							bnBegin(&pub->e);
							if (bnCopy(&pub->n, &sec->s.n) >= 0
							&& bnCopy(&pub->e, &sec->s.e) >= 0)
							{
									rsaFillPubkey(pubkey, pub);
									pubkey->pkAlg = seckey->pkAlg;
									memcpy(pubkey->keyID, seckey->keyID,
									sizeof(pubkey->keyID));
									return pubkey;
							}
							/* Failed = clean up and return NULL */
							bnEnd(&pub->n);
							bnEnd(&pub->e);
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
rsaIslocked(struct PgpSecKey const *seckey)
	{
			struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;

			ASSERTRSA(seckey->pkAlg);
			return sec->locked;
	}


/*
* Try to decrypt the secret key wih the given passphrase. Returns >0
* if it was the correct passphrase. =0 if it was not, and <0 on error.
* Does not alter the key even if it's the wrong passphrase and already
* unlocked. A NULL passphrae will work if the key is unencrypted.
*
* A (secret) key's RSA-specific part is:
*
* 0	2+u MPI for modulus
* 2+u	2+v MPI for exponent
* 4+u+v	1 Encryption algorithm (0 for none, 1 for IDEA)
* 5+u+v	t Encryption IV: 0 or 8 bytes
* 5+t+u+v	2+w MPI for d
* 7+t+u+v+w	2+x MPI for p
* 9+t+u+v+w+x	2+y MPI for q
* 11+t+u+v+w+x+y	2+z MPI for u
* 13+t+u+v+w+x+y+z 2 Checksum
* 15+t+u+v+w+x+y+z
*
* Actually, that's the old-style, if pgpS2KoldVers is true.
* If it's false, the algoruthm is 255, and is followed by the
* algorithm, then the (varaible-length, self-delimiting)
* string-to-key descriptor.
*/

static int
rsaUnlock(struct PgpSecKey *seckey, struct PgpEnv const *env,
			char const *phrase, size_t plen)
	{
			struct RSAsecPlus *sec = (struct RSAsecPlus *)seckey->priv;
			struct BigNum d, p, q, u, bn;
			struct PgpCipher const *cipher;
			struct PgpCfbContext *cfb = NULL;		/* Necessary */
			struct PgpStringToKey *s2k;
			byte key[PGP_CIPHER_MAXKEYSIZE];
			unsigned v, t;
			unsigned alg;
			unsigned checksum;
			int i;

			ASSERTRSA(seckey->pkAlg);
			bnInit();

			if (sec->cklen < 5)
				 return PGPERR_KEY_SHORT;
			v = ((unsigned)sec->cryptkey[0] << 8) + sec->cryptkey[1];
			v = (v+7)/8;
			if (sec->cklen < 5+v)
				 return PGPERR_KEY_SHORT;
			if (bnInsertBigBytes(&sec->s.n, sec->cryptkey+2, 0, v) < 0)
				 return PGPERR_NOMEM;
			t = ((unsigned)sec->cryptkey[2+v] << 8) + sec->cryptkey[3+v];
			t = (t+7)/8;
			if (sec->cklen < 4+v+t)
				 return PGPERR_KEY_SHORT;
			if (bnInsertBigBytes(&sec->s.e, sec->cryptkey+4+v, 0, t) < 0)
				 return PGPERR_NOMEM;
			v += t + 5;
			if (sec->cklen < v)
				 return PGPERR_KEY_SHORT;

			/* Get the encryption algorithm (cipher number). 0 == no encryption */
			alg = sec->cryptkey[v-1];

			/* If the phrase is empty, set it to NULL */
			if (!phrase || plen == 0)
	phrase = NULL;
/*
* We need a pass if it is encrypted, and we cannot have a
* password if it is NOT encrypted. I.e., this is a logical
* xor (^^)
*/
if (!phrase != !alg)
	return 0;

if (!alg) {
	/* The key isn't encrypted; just read it in */
	cfb = NULL;
	} else {
			if (alg == 255) {
					/* New style, with a separate string-to-key */
					if (sec->cklen == v)
						return PGPERR_KEY_SHORT;
					alg = sec->cryptkey[v++];
					i = pgpS2Kdecode(&s2k, env, sec->cryptkey+v,
							 sec->cklen-v);
					if (i < 0)
						return i;
					v += i;
					if (sec->cklen < v)
						return PGPERR_KEY_SHORT;
			} else {
					/* Old-style string-to-key */
					s2k = pgpS2Ksimple(env, pgpHashByNumber(PGP_HASH_MD5));
					if (!s2k)
						return PGPERR_NOMEM;
			}
			/* Okay now, do the conversion */
			cipher = pgpCipherByNumber(alg);
			if (!cipher) {
				 pgpS2Kdestroy(s2k);
				 return PGPERR_BAD_CIPHERNUM;
			}
			if (sec->cklen < v+cipher->blocksize) {
				 pgpS2Kdestroy(s2k);
				 return PGPERR_KEY_SHORT;
			}
				cfb = pgpCfbCreate(cipher);
				if (!cfb) {
					pgpS2Kdestroy(s2k);
					return PGPERR_NOMEM;
				}
				pgpAssert(cipher->keysize <= sizeof(key));
			pgpStringToKey(s2k, phrase, plen, key, cipher->keysize);
			pgpCfbInit(cfb, key, sec->cryptkey+v);
			memset(key, 0, sizeof(key));
			pgpS2Kdestroy(s2k);
			v += cipher->blocksize;	/* Skip over IV */
	}
#if 0
		
		
		
	} else {			/* Old-style */
					/* The key is encrypted; create a cfb context */
					cipher = pgpCipherByNumber(alg);
					if (!cipher)
						return PGPERR_BAD_CIPHERNUM;
					if (sec->cklen < v+cipher->blocksize)
						return PGPERR_KEY_SHORT;
					cfb = pgpCfbCreate(cipher);
					if (!cfb)
						return PGPERR_NOMEM;
					s2k = pgpS2Ksimple(pgpHashByNumber(PGP_HASH_MD5));
					if (!s2k) {
						pgpCfbDestroy(cfb);
						return PGPERR_NOMEM;
					}

					hc = pgpHashCreate(pgpHashByNumber(PGP_HASH_MD5));
					if (!hc) {
						pgpCfbDestroy(cfb);
						return PGPERR_NOMEM;
					}
					if (cipher->keysize > hc->hash->hashsize) {
						pgpCfbDestroy(cfb);
						return PGPERR_BAD_CIPHERNUM;
					}
					pgpHashUpdate(hc, (byte const *)phrase, plen);
					pgpCfbInit(cfb, pgpHashFinal(hc), sec->cryptkey+v);
					pgpHashDestroy(hc);
					v += cipher->blocksize;
			}
#endif
	
checksum = 0;
bnBegin(&d);
bnBegin(&p);
bnBegin(&q);
bnBegin(&u);
bnBegin(&bn);
i = pgpBnGetOld(&d, sec->cryptkey + v, sec->cklen - v, cfb, &checksum);
if (i <= 0)
	goto fail;
v += i;
if (bnCmp(&d, &sec->s.n) >= 0)
	goto badpass;	 /* Wrong passphrase: d must be < n */
i = pgpBnGetOld(&p, sec->cryptkey + v, sec->cklen - v, cfb, &checksum);
if (i <= 0)
	goto fail;
if ((bnLSWord(&p) & 1) == 0)
	goto badpass;
v += i;
i = pgpBnGetOld(&q, sec->cryptkey + v, sec->cklen - v, cfb, &checksum);
if (i <= 0)
	goto fail;
if ((bnLSWord(&q) & 1) == 0)
	goto badpass;
v += i;

			/* Extremely high-powered check. Verify that p*q == n */
			if (bnMul(&bn, &p, &q) < 0)
			goto nomem;
		if (bnCmp(&bn, &sec->s.n) != 0)
			goto badpass;

		/* Verify that d*e == 1 mod p-1 */
		(void)bnSubQ(&p, 1);
		if (bnMul(&bn, &d, &sec->s.e) < 0 || bnMod(&bn, &bn, &p) < 0)
			goto nomem;
		if (bnCmpQ(&bn, 1) != 0)
			goto badpass;
		(void)bnAddQ(&p, 1);

		/* Verify that d*e == 1 mod q-1 */
		(void)bnSubQ(&q, 1);
		if (bnMul(&bn, &d, &sec->s.e) < 0 || bnMod(&bn, &bn, &q) < 0)
			goto nomem;
		if (bnCmpQ(&bn, 1) != 0)
			goto badpass;
		(void)bnAddQ(&q, 1);

		i = pgpBnGetOld(&u, sec->cryptkey + v, sec->cklen - v, cfb, &checksum);
		if (i <= 0)
			goto fail;
		v += i;

		/* Check that we ended in the right place */
		if (sec->cklen - v != 2) {
			i = PGPERR_KEY_LONG;
			goto fail;
		}
		checksum &= 0xffff;
		if (checksum != ((unsigned)sec->cryptkey[v]<<8) + sec->cryptkey[1+v])
			goto badpass;

		/* Verify that u = p^-1 mod q is less than q */
		if (bnCmp(&u, &q) >= 0)
			goto badpass;
		/* Verify that u * p == 1 mod q */
		if (bnMul(&bn, &p, &u) < 0 || bnMod(&bn, &bn, &q) < 0)
			goto nomem;
		if (bnCmpQ(&bn, 1) != 0)
			goto badpass;

		/*
			* Okay, we've verified every single value in the secret key,
			* against the public key, so it is *definitely* the right
			* secret key. Note that the "nomem" case calls bnEnd()
			* more than once, but this is guaranteed harmless.
*/
			bnEnd(&bn);
			if (bnCopy(&sec->s.d, &d) < 0)
				 goto nomem;
			bnEnd(&d);
			if (bnCopy(&sec->s.p, &p) < 0)
				 goto nomem;
			bnEnd(&p);
			if (bnCopy(&sec->s.q, &q) < 0)
				 goto nomem;
		bnEnd(&q);
		if (bnCopy(&sec->s.u, &u) < 0)
			goto nomem;
		bnEnd(&u);

		i = 1;	/* Decrypted! */
		sec->locked = 0;
		if (cfb)
			pgpCfbDestroy (cfb);
		return 1;	 /* Decrypted */

	nomem:
			i = PGPERR_NOMEM;
			goto done;
	badpass:
			i = 0;	/* Incorrect passphrase */
			goto done;
	fail:
			if (!i)
				 i = PGPERR_KEY_SHORT;
			goto done;
	done:
			if (cfb)
				 pgpCfbDestroy (cfb);
			bnEnd(&bn);
			bnEnd(&u);
			bnEnd(&q);
			bnEnd(&p);
			bnEnd(&d);
			return i;
	}

/*
* Relock the key.
*/
static void
rsaLock(struct PgpSecKey *seckey)
	{
			struct RSAsecPlus *sec = (struct RSAsecPlus *)seckey->priv;

			ASSERTRSA(seckey->pkAlg);
			sec->locked = 1;
			/* bnEnd is documented as also doing a bnBegin */
			bnEnd(&sec->s.d);
			bnEnd(&sec->s.p);
			bnEnd(&sec->s.q);
			bnEnd(&sec->s.u);
	}

/*
* Return the size of the buffer needed, worst-case, for the decrypted
* output. A trivially padded key (random padding length = 0)
* can just be 0 2 0 <key>.
*/
static size_t
rsaMaxdecrypted(struct PgpSecKey const *seckey)
{
	struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;
			size_t size;

			ASSERTRSAENC(seckey->pkAlg);
			size = (bnBits(&sec->s.n)+7)/8;
			return size < 3 ? 0 : size-3;
	}

/*
* Try to decrypt the given esk. If the key is locked, try the given
* passphrase. It may or may not leave the key unlocked in such a case.
* (Some hardware implementations may insist on a password per usage.)
*/
static int
rsaDecrypt(struct PgpSecKey *seckey, struct PgpEnv const *env,
			int esktype, byte const *esk, size_t esklen,
			byte *key, size_t *keylen, char const *phrase,
			size_t plen)
	{
			struct RSAsecPlus *sec = (struct RSAsecPlus *)seckey->priv;
			struct BigNum bn;
			int i, j;
			unsigned t;
			size_t max;

(void)esktype;

ASSERTRSAENC(seckey->pkAlg);
if (sec->locked) {
			i = rsaUnlock(seckey, env, phrase, plen);
			if (i <= 0)
				 return i ? i : PGPERR_KEY_ISLOCKED;
			pgpAssert(!sec->locked);
	}

			if (esklen < 2)
				 return PGPERR_ESK_TOOSHORT;
			t = ((unsigned)esk[0]<<8) + esk[1];
			t = (t+7)/8;
			if (esklen != t+2)
				 return esklen<t+2 ? PGPERR_ESK_TOOSHORT : PGPERR_ESK_TOOLONG;
			bnBegin(&bn);
			if (bnInsertBigBytes(&bn, esk+2, 0, t) < 0) {
				 bnEnd(&bn);
				 return PGPERR_NOMEM;
			}
			max = rsaMaxdecrypted(seckey);
			i = rsaPrivateDecrypt(key, max, &bn, &sec->s);
			bnEnd(&bn);
			if (i < 0)
				 return i;
			if ((size_t)i > max || i < 3)
				 return PGPERR_RSA_CORRUPT;
			/* Check checksum (should this be here?) */
			t = 0;
			for (j = 1; j < i-2; j++)
				 t += key[j];
			if (t != ((unsigned)key[i-2]<<8) + key[i-1])
				 return PGPERR_RSA_CORRUPT;
memset(key+i-2, 0, 2);

/* The actual key */
if (keylen)
	*keylen = (size_t)i-2;
return 0;
}

static size_t
rsaMaxsig(struct PgpSecKey const *seckey, PgpVersion version)
	{
			struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;

(void)version;
ASSERTRSASIG(seckey->pkAlg);
return (bnBits(&sec->s.n)+7)/8 + 2;
}

static int
rsaSign(struct PgpSecKey *seckey, struct PgpHash const *h, byte const *hash,
			byte *sig, size_t *siglen, struct PgpRandomContext const *rc,
			PgpVersion version)
	{
			struct RSAsecPlus *sec = (struct RSAsecPlus *)seckey->priv;
			struct BigNum bn;
			unsigned t;
			int i;

/* We don't need these arguments, although other algorithms may... */
(void)rc;
(void)version;

ASSERTRSASIG(seckey->pkAlg);
if (sec->locked)
	return PGPERR_KEY_ISLOCKED;

t = h->DERprefixsize;
if (t+h->hashsize > rsaMaxsig(seckey, version))
	return PGPERR_PUBKEY_TOOSMALL;
memcpy(sig, h->DERprefix, t);
memcpy(sig+t, hash, h->hashsize);
t += h->hashsize;

bnBegin(&bn);

i = rsaPrivateEncrypt(&bn, sig, t, &sec->s);
memset(sig, 0, t);
if (i >= 0) {
			t = bnBits(&bn);
			sig[0] = (byte)(t >> 8 & 255);
			sig[1] = (byte)(t & 255);
			t = (t+7)/8;
			bnExtractBigBytes(&bn, sig+2, 0, t);

			if (siglen)
				 *siglen = (size_t)t+2;
			i = 0;
	}
	bnEnd(&bn);
	return i;
}

/*
* Re-encrypt a PGpSecKey with a new urn a PgpSecKey into a secret key.
* A secret key is, after a non-specific prefix:
* 0 1 Version (= 2 or 3)
* 1 4 Timestamp
* 5 2 Validity (=0 at present)
* 7 1 Algorithm (=1 for RSA)
* The following:
* 0	2+u MPI for modulus
* 2+u	2+v MPI for exponent
* 4+u+v	1 Encryption algorithm (0 for none, 1 for IDEA)
* 5+u+v	t Encryption IV: 0 or 8 bytes
* 5+t+u+v	2+w MPI for d
* 7+t+u+v+w	2+x MPI for p
* 9+t+u+v+w+x	2+y MPI for q
* 11+t+u+v+w+x+y	2+z MPI for u
* 13+t+u+v+w+x+y+z 2 Checksum (big-endian sum of all the bytes)
* 15+t+u+v+w+x+y+z
*
* The Encryption algorithm is the cipher algorithm for the old-style
* string-to-key conversion. For the new type, it's 255, then a cipher
* algorithm, then a string-to-key algorithm (variable-length),
* then the encryption IV. That's 16 bytes plus the string-to-key
* conversion length.
*
* On initial key generation we rely on calling this with env=NULL being
* OK if phrase=NULL.
*/

int
rsaChangeLock(struct PgpSecKey *seckey, struct PgpEnv const *env,
			struct PgpRandomContext const *rc, char const *phrase, size_t plen)
	{
		struct RSAsecPlus *sec = (struct RSAsecPlus *)seckey->priv;
		struct PgpStringToKey *s2k = NULL;	/* Shut up warnings */
		struct PgpCipher const *cipher = NULL;	/* Shut up warnings */
	struct PgpCfbContext *cfb = NULL;		/* This is realy needed */
	byte *p;
	int oldf = 0;					/* Shut up warnings */
	unsigned len;
	unsigned checksum;

	ASSERTRSA(seckey->pkAlg);
	if (sec->locked)
	return PGPERR_KEY_ISLOCKED;

			len = bnBytes(&sec->s.n) + bnBytes(&sec->s.e) +
			bnBytes(&sec->s.d) + bnBytes(&sec->s.p) +
			bnBytes(&sec->s.q) + bnBytes(&sec->s.u) + 15;
			if (phrase) {
				 s2k = pgpS2KdefaultV2(env, rc);
				 if (!s2k)
				 	return PGPERR_NOMEM;
				 cipher = pgpCipherDefaultKeyV2(env);
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

			/* Okay, no more errors possible! Start installing data */
			p += pgpBnPutPlain(&sec->s.n, p);
			p += pgpBnPutPlain(&sec->s.e, p);

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

			/* Now install d, p, q and u, encrypted */
			checksum = 0;
			p += pgpBnPutOld(&sec->s.d, p, cfb, &checksum);
			p += pgpBnPutOld(&sec->s.p, p, cfb, &checksum);
			p += pgpBnPutOld(&sec->s.q, p, cfb, &checksum);
			p += pgpBnPutOld(&sec->s.u, p, cfb, &checksum);
			pgpChecksumPutOld(checksum, p, cfb);
			p += 2;
			pgpAssert((ptrdiff_t)len == p - sec->cryptkey);

			if (cfb)
				 pgpCfbDestroy(cfb);
			return 0;	/* Success */
	}

static size_t
rsaSecBufferLength(struct PgpSecKey const *seckey)
	{
	struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;

	return sec->cklen;
}

static void
rsaSecToBuffer(struct PgpSecKey const *seckey, byte *buf)
	{
			struct RSAsecPlus const *sec = (struct RSAsecPlus *)seckey->priv;

			memcpy(buf, sec->cryptkey, sec->cklen);

			/* Return only algorithm-dependent portion */
#if 0
			if (ver > PGPVERSION_2_6)
				 ver = PGPVERSION_2_6;
			else if (ver < PGPVERSION_2)
				 ver = PGPVERSION_2;
			buf[0] = (byte)ver;
			buf[1] = (byte)(tstamp >> 24);
			buf[2] = (byte)(tstamp >> 16);
			buf[3] = (byte)(tstamp >> 8);
			buf[4] = (byte)tstamp;
			/* Round validity up to the nearest day = 60*60*24 seconds */
			validity = (validity + 86399)/86400;
			buf[5] = (byte)(validity >> 8);
			buf[6] = (byte)validity;
			buf[7] = PGP_PKALG_RSA;
#endif
}

#if 0
/*
* Format of signature packets:
*
* Offset Length Meaning
* 0 1 Version byte (= 2 or 3).
* 1 1 x, Length of following material included in MD5 (=5)
* 2 1 Signature type (=0 or 1)
* 3 4 32-bit timestamp of signature
* -------- MD5 additional material stops here, at offset 2+x ---------
*	2+x	8	KeyID
*	10+x	1	PK algorithm type (1 = RSA)
*	11+x	1	MD algorithm type (1 = MD5)
*	12+x	2	First 2 bytes of message digest (16-bit checksum)
*	14+x	2+y	MPI of PK-encrypted integer
*	16+x+y
*/
static int
rsaSigValidate(int type, byte const *sig, size_t len)
	{
			unsigned extra, bits;
			struct PgpHash const *hash;

			if (len < 1)
				 return PGPERR_SIG_TOOSHORT;
			if (sig[0] != PGPVERSION_2 && sig[0] != PGPVERSION_2_6)
				 return PGPERR_SIG_BADVERSION;
			if (len < 2)
				 return PGPERR_SIG_TOOSHORT;
			extra = sig[1];
			if (len < 11+extra)
				 return PGPERR_SIG_TOOSHORT;
			if (sig[10+extra] != PGP_PKALG_RSA)
				 return PGPERR_SIG_BADALGORITHM;
			hash = hashByNumber(sig[11+extra]);
			if (!hash)
				 return PGPERR_SIG_BADALGORITHM;
			if (len < 16+extra)
				 return PGPERR_SIG_TOOSHORT;
			bits = ((unsigned)sig[14+extra]<<8) + sig[15+extra];
			if (len != 16+extra+(bits+7)/8)
				 return len < 16+extra+(bits+7)/8 ? PGPERR_SIG_TOOSHORT :
				 PGPERR_SIG_TOOLONG;
			if (bits && sig[16+extra] >> (bits & 7) != 1)
				 return PGPERR_SIG_BITSWRONG;
			return 0;
	}
#endif /* 0 */


/* Fill in secret key structure */
void
rsaFillSecKey(struct PgpSecKey *seckey, struct RSAsecPlus *sec)
	{
			seckey->pkAlg			= PGP_PKALG_RSA;
			seckey->priv			= sec;
			seckey->destroy		= rsaSecDestroy;
#if 0
			seckey->id8		= rsaSecId8;
#endif
			seckey->pubkey		= rsaPubkey;
			seckey->islocked		= rsaIslocked;
			seckey->unlock		= rsaUnlock;
			seckey->lock		= rsaLock;
			seckey->maxdecrypted = rsaMaxdecrypted;
			seckey->decrypt = rsaDecrypt;
			seckey->maxsig = rsaMaxsig;
			seckey->sign = rsaSign;
seckey->changeLock = rsaChangeLock;
seckey->bufferLength = rsaSecBufferLength;
seckey->toBuffer = rsaSecToBuffer;
}


struct PgpSecKey *
rsaSecFromBuf(byte const *buf, size_t size, int *error)
	{
			struct PgpSecKey *seckey;
			struct RSAsecPlus *sec;
			int err = PGPERR_NOMEM;
			byte *cryptk;

			bnInit();
			cryptk = (byte *)pgpMemAlloc(size);
			if (cryptk) {
					sec = (struct RSAsecPlus *)pgpMemAlloc(sizeof(*sec));
					if (sec) {
							seckey = (struct PgpSecKey *)
								 pgpMemAlloc(sizeof(*seckey));
							if (seckey) {
									memcpy(cryptk, buf, size);
									bnBegin(&sec->s.n);
									bnBegin(&sec->s.e);
									bnBegin(&sec->s.d);
									bnBegin(&sec->s.p);
									bnBegin(&sec->s.q);
									bnBegin(&sec->s.u);
									sec->cryptkey = cryptk;
									sec->cklen = sec->ckalloc = size;
									sec->locked = 1;
									/* We only need this to try unlocking... */
									seckey->pkAlg = PGP_PKALG_RSA;
									seckey->priv = sec;
								
									if (rsaUnlock(seckey, NULL, NULL, 0) >= 0) {
											if (rsaKeyTooBig (NULL, &sec->s)) {
												 err = PGPERR_KEY_UNSUPP;
											} else {
													rsaFillSecKey(seckey, sec);
													*error = 0;
													return seckey;	 /* Success! */
											}
									}

							/* Ka-boom. Delete and free everything. */
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

#endif /*!NO_RSA*/
