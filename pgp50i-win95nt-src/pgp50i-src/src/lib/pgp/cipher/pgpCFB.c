/*
* pgpCFB.c - generic cipher feedback encryption, using pgpCipher.h ciphers.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpCFB.c,v 1.2.2.1 1997/06/07 09:49:50 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "pgpDebug.h"
#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpMem.h"
#include "pgpUsuals.h"

/*
* Initialize context.
* If key is NULL, the current key is not changed.
* if iv is NULL, the IV is set to all zero.
*/
void
pgpCfbInit(struct PgpCfbContext *cfb, byte const *key, byte const *iv)
{
		pgpAssert(cfb);
		pgpAssert(cfb->cipher);
		if (key)
			pgpCipherKey(cfb->cipher, key);
		memset(cfb->iv, 0, sizeof(cfb->iv));
		if (iv)
			memcpy(cfb->iv, iv, cfb->cipher->cipher->blocksize);
		cfb->bufleft = 0;
}

void
pgpCfbWipe(struct PgpCfbContext *cfb)
{
		struct PgpCipherContext *cipher;

		if (!cfb)
				return;

		cipher = cfb->cipher;
		if (cipher)
				pgpCipherWipe (cipher);

		memset(cfb, 0, sizeof(*cfb));
		cfb->cipher = cipher;
}

struct PgpCfbContext *
pgpCfbCreate(struct PgpCipher const *c)
{
		struct PgpCfbContext *cfb;
	struct PgpCipherContext *cipher;

		if (c->blocksize > PGP_CFB_MAXBLOCKSIZE)
			return 0;

			cfb = (struct PgpCfbContext *)pgpMemAlloc(sizeof(*cfb));
		if (cfb) {
				cipher = pgpCipherCreate(c);
				if (cipher) {
					memset (cfb, 0, sizeof (*cfb));
					cfb->cipher = cipher;
					return cfb;
				}
				pgpMemFree(cfb);
		}
		return 0;
}

struct PgpCfbContext *
pgpCfbCopy (struct PgpCfbContext const *cfb)
{
		struct PgpCfbContext *newcfb;
		struct PgpCipherContext *cipher;

		if (!cfb)
			return NULL;

		newcfb = (struct PgpCfbContext *)pgpMemAlloc (sizeof (*newcfb));
		if (!newcfb)
			return NULL;

		cipher = pgpCipherCopy (cfb->cipher);
		if (!cipher) {
			pgpMemFree (newcfb);
			return NULL;
		}

		memcpy (newcfb, cfb, sizeof (*cfb));
		newcfb->cipher = cipher;
		return newcfb;
}

void
pgpCfbDestroy(struct PgpCfbContext *cfb)
{
		if (cfb) {
				if (cfb->cipher) {
					pgpCipherDestroy(cfb->cipher);
					cfb->cipher = NULL;
				}
				pgpCfbWipe(cfb);
				pgpMemFree(cfb);
		}
}

/*
* Encrypt a buffer of data, using a block cipher in CFB mode.
* There are more compact ways of writing this, but this is
* written for speed.
*/
void
pgpCfbEncrypt(struct PgpCfbContext *cfb, byte const *src, byte *dest,
		size_t len)
{
		unsigned blocksize = cfb->cipher->cipher->blocksize;
		unsigned bufleft = cfb->bufleft;
		byte *bufptr = cfb->iv + blocksize-bufleft;

		pgpAssert(cfb);
		pgpAssert(cfb->cipher);

		/*
		* If there are no more bytes to encrypt that there are bytes
		* in the buffer, XOR them in and return.
		*/
		if (len <= bufleft) {
				cfb->bufleft = bufleft - len;
				while (len--) {
						*dest++ = *bufptr++ ^= *src++;
				}
				return;
		}
		len -= bufleft;
		/* Encrypt the first bufleft (0 to 7) bytes of the input by XOR
		* with the last bufleft bytes in the iv buffer.
		*/
		while (bufleft--) {
			*dest++ = (*bufptr++ ^= *src++);
		}
		/* Encrypt middle blocks of the input by cranking the cipher,
		* XORing blocksize-byte blocks, and repeating until the len
		* is blocksize or less.
		*/
		while (len > blocksize) {
				bufptr = cfb->iv;
				memcpy(cfb->prev, bufptr, blocksize);
				pgpCipherEncrypt(cfb->cipher, bufptr, bufptr);
				bufleft = blocksize;
				len -= blocksize;
				do {
					*dest++ = (*bufptr++ ^= *src++);
				} while (--bufleft);
		}
		/* Do the last 1 to blocksize bytes */
		bufptr = cfb->iv;
		memcpy(cfb->prev, bufptr, blocksize);
		pgpCipherEncrypt(cfb->cipher, bufptr, bufptr);
		cfb->bufleft = blocksize-len;
		do {
			*dest++ = (*bufptr++ ^= *src++);
		} while (--len);
}


/*
* Decrypt a buffer of data, using a cipher in CFB mode.
* There are more compact ways of writing this, but this is
* written for speed.
*/
void
pgpCfbDecrypt(struct PgpCfbContext *cfb, byte const *src, byte *dest,
		size_t len)
{
		unsigned blocksize = cfb->cipher->cipher->blocksize;
		unsigned bufleft = cfb->bufleft;
		static byte *bufptr;
		byte t;

		pgpAssert(cfb);
		pgpAssert(cfb->cipher);

		bufptr = cfb->iv + (blocksize-bufleft);
		if (len <= bufleft) {
				cfb->bufleft = bufleft - len;
				while (len--) {
						t = *bufptr;
						*dest++ = t ^ (*bufptr++ = *src++);
				}
				return;
		}
		len -= bufleft;
		while (bufleft--) {
			t = *bufptr;
			*dest++ = t ^ (*bufptr++ = *src++);
		}
		while (len > blocksize) {
				bufptr = cfb->iv;
				memcpy(cfb->prev, bufptr, 8);
				pgpCipherEncrypt(cfb->cipher, bufptr, bufptr);
				bufleft = blocksize;
				len -= blocksize;
				do {
					t = *bufptr;
					*dest++ = t ^ (*bufptr++ = *src++);
				} while (--bufleft);
		}
		bufptr = cfb->iv;
		memcpy(cfb->prev, bufptr, blocksize);
		pgpCipherEncrypt(cfb->cipher, bufptr, bufptr);
		cfb->bufleft = blocksize-len;
		do {
			t = *bufptr;
			*dest++ = t ^ (*bufptr++ = *src++);
		} while (--len);
}

/*
* Okay, explanation time:
* Phil invented a unique way of doing CFB that's sensitive to semantic
* boundaries within the data being encrypted. One way to phrase
* CFB en/decryption on an 8-byte block cipher is to say that you XOR
* the current 8 bytes with CRYPT(previous 8 bytes of ciphertext).
* Normally, you repeat this at 8-byte intervals, but Phil decided to
* resync things on the boundaries between elements in the stream being
* encrypted.
*
* That is, the last 4 bytes of a 12-byte field are en/decrypted using
* the first 4 bytes of CRYPT(previous 8 bytes of ciphertext), but then
* the last 4 bytes of that CRYPT computation are thrown away, and the
* first 8 bytes of the next field are en/decrypted using
* CRYPT(last 8 bytes of ciphertext). This is equivalent to using a
* shorter feedback length (if you're familiar with the general CFB
* technique) briefly, and doesn't weaken the cipher any (using shorter
* CFB lengths makes it stronger, actually), it just makes it a bit unusual.
*
* Anyway, to accomodate this behaviour, every time we do an
* encrpytion of 8 bytes of ciphertext to get 8 bytes of XOR mask,
* we remember the ciphertext. Then if we have to resync things
* after having processed, say, 2 bytes, we refill the iv buffer
* with the last 6 bytes of the old ciphertext followed by the
* 2 bytes of new ciphertext stored in the front of the iv buffer.
*/
void
pgpCfbSync(struct PgpCfbContext *cfb)
{
		unsigned blocksize = cfb->cipher->cipher->blocksize;
		unsigned bufleft = cfb->bufleft;

		if (bufleft) {
				memmove(cfb->iv+bufleft, cfb->iv, blocksize-bufleft);
				memcpy(cfb->iv, cfb->prev+blocksize-bufleft, bufleft);
				cfb->bufleft = 0;
		}
}

/*
* Cryptographically strong pseudo-random-number generator.
* The design is from Appendix C of ANSI X9.17, "Financial
* Institution Key Management (Wholesale)", with IDEA
* substituted for triple-DES.
*
* This generates one block (64 bits) of random number R, based on a
* key K, an initial vector V, and a time-dependent salt I. I need not
* be truly random, but should differ for each block of output R, and
* contain as much entropy as possible. In X9.17, I is encrypt(K,time()),
* where time() is the most accurate time available. This has I supplied
* (from the true random number pool, which is based on similar information)
* to the ideaRandCycle function.
*
* The operation of ideaRandCycle is straight from X9.17::
* R = encrypt(K, V^I)
* V = encrypt(K, R^I)
*
* One thing worth noting is that, given fixed values of K and V,
* there is an isomorphism between values of I and values of R.
* Thus, R always contains at least as much entropy as is in I;
* if I is truly completely random, it does not matter if K and/or V
* are known. Thus, if the supplied I (from the true random number pool)
* are good, the output of this is good.
*/


/*
* Fills in the supplied buffer with up to "count" pseudo-random bytes.
* Returns the number of bytes filled in. If less than "count",
* pgpCfbRandCycle will need to be called with a new random salt value
* before more bytes are available. pgpCfbRandBytes will return 0 until
* that is done.
*/
unsigned
pgpCfbRandBytes(struct PgpCfbContext *cfb, byte *dest, unsigned count)
{
		unsigned bufleft = cfb->bufleft;

		if (count > bufleft)
			count = bufleft;
		cfb->bufleft = bufleft - count;
		memcpy(dest, cfb->prev+cfb->cipher->cipher->blocksize-bufleft, count);
		return count;
}

/*
* Make more random bytes available from ideaRandBytes using the X9.17
* algorithm and the supplied random salt. Expects "cfb->cipher->blocksize"
* bytes of salt.
*/
void
pgpCfbRandCycle(struct PgpCfbContext *cfb, byte const *salt)
{
		unsigned i;
		struct PgpCipherContext *cipher = cfb->cipher;
		unsigned blocksize = cipher->cipher->blocksize;

		for (i = 0; i < blocksize; i++)
			cfb->iv[i] ^= salt[i];
		pgpCipherEncrypt(cipher, cfb->iv, cfb->prev);
		for (i = 0; i < blocksize; i++)
			cfb->iv[i] = cfb->prev[i] ^ salt[i];
		pgpCipherEncrypt(cipher, cfb->iv, cfb->iv);
		cfb->bufleft = blocksize;
}

/*
* "Wash" the random number state using an irreversible transformation
* based on the input buffer and the previous state.
*/
void
pgpCfbRandWash(struct PgpCfbContext *cfb, byte const *buf, unsigned len)
{
		struct PgpCipherContext *cipher = cfb->cipher;
		unsigned blocksize = cipher->cipher->blocksize;
		unsigned i, j;

		/* Wash the key (if supported) */
		if (cipher->cipher->wash)
			pgpCipherWash(cipher, buf, len);

		/*
		* Wash the IV - a bit ad-hoc, but it works. It's
		* basically a CBC-MAC.
		*/
		for (i = j = 0; i < len; i++) {
				cfb->iv[j] ^= buf[i];
				if (++j == blocksize) {
					pgpCipherEncrypt(cipher, cfb->iv, cfb->iv);
					j = 0;
				}
		}
		/* Pad with typical CBC padding indicating the length */
		while (j < blocksize)
			cfb->iv[j++] ^= (byte)len;
		pgpCipherEncrypt(cipher, cfb->iv, cfb->iv);

		/* Set to empty */
		cfb->bufleft = 0;
}
