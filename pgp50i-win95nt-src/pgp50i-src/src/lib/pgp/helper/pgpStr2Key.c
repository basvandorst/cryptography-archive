/*
* pgpStr2Key.c -- A prototype string-to-key framework.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpStr2Key.c,v 1.4.2.1 1997/06/07 09:50:12 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stddef.h>	 /* For offsetof() */

#include "pgpDebug.h"
#include "pgpStr2Key.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpRndom.h"
#include "pgpUsuals.h"

/* Number of chars to hash when creating default S2K */
#define SALT_CHARS_DEFAULT		32768

/* It turns out that they all use the same private context structure */
struct StringToKeyPriv {
	struct PgpEnv const *env;
	struct PgpHash const *hash;
	byte buf[1];	/* Variable-sized */
};

/* And it can be destroyed with a common routine */
static void
s2kDestroy(struct PgpStringToKey *s2k)
	{
			struct StringToKeyPriv *priv = (struct StringToKeyPriv *)s2k->priv;
			memset(priv, 0, s2k->encodelen+offsetof(struct StringToKeyPriv, buf));
			pgpMemFree(priv);
			memset(s2k, 0, sizeof(*s2k));
			pgpMemFree(s2k);
	}

static struct PgpStringToKey *
s2kAlloc(struct PgpEnv const *env, struct PgpHash const *h, unsigned size)
	{
			struct StringToKeyPriv *priv;
			struct PgpStringToKey *s2k = NULL;

priv = (struct StringToKeyPriv *)
	pgpMemAlloc(offsetof(struct StringToKeyPriv, buf) + size);
if (priv) {
			s2k = (struct PgpStringToKey *)pgpMemAlloc(sizeof(*s2k));
			if (s2k) {
				 s2k->priv = priv;
				 s2k->encoding = priv->buf;
				 s2k->encodelen = size;
							s2k->destroy = s2kDestroy;
							priv->env = env;
							priv->hash = h;
							priv->buf[1] = h->type;
					}
			}
			return s2k;
	}

/* Allocate an array of "num" hash private buffers, all sharing the same hash*/
static void **
multiHashCreate(struct PgpHash const *h, unsigned num)
	{
			void **v;
			void *p;
			unsigned i, j;
			byte const b = 0;

			v = (void **)pgpMemAlloc(num * sizeof(*v));
			if (!v)
				 return NULL;

			for (i = 0; i < num; i++) {
					p = pgpMemAlloc(h->context_size);
					if (!p) {
							while (i) {
								 memset(v[--i], 0, h->context_size);
								 pgpMemFree(v[i]);
							}
							pgpMemFree(v);
							return NULL;
					}
					h->init(p);
					/* Initialze the PgpHashContext with leading null bytes */
					for (j = 0; j < i; j++)
						h->update(p, &b, 1);
					v[i] = p;
			}
			return v;
	}

/* Update an array of hash private buffers, all sharing the same hash */
static void
multiHashUpdate(struct PgpHash const *h, void * const *v, unsigned num,
			byte const *string, size_t len)
	{
			while (num--)
				 h->update(*v++, string, len);
	}

/*
* Extract the final combined string from an array of hash private buffers,
* then wipe and free them.
*/
static void
multiHashFinal(struct PgpHash const *h, void **v, byte *key, size_t klen)
{
	void **v0 = v;
unsigned hsize = h->hashsize;

while (klen > hsize) {
			memcpy(key, h->final(*v), hsize);
			key += hsize;
			klen -= hsize;
			memset(*v, 0, h->context_size);
			pgpMemFree(*(v++));
	}
memcpy(key, h->final(*v), klen);
memset(*v, 0, h->context_size);
pgpMemFree(*v);

	pgpMemFree(v0);
}

static int
s2kSimple(struct PgpStringToKey const *s2k, char const *str,
			size_t slen, byte *key, size_t klen)
	{
			unsigned num;
			struct StringToKeyPriv const *priv;
			struct PgpHash const *h;
			void **v;

		pgpAssert(s2k->encodelen == 2);

		if (!klen)
			return 0;	/* Okay, I guess... */

		priv = (struct StringToKeyPriv *)s2k->priv;
		h = priv->hash;
		pgpAssert(h->type == priv->buf[1]);
		num = (klen-1)/h->hashsize + 1;
		v = multiHashCreate(h, num);
		if (!v)
			return PGPERR_NOMEM;
		multiHashUpdate (h, v, num, (byte const *)str, slen);
		multiHashFinal(h, v, key, klen);

		return 0;
	}


static int
s2kSalted(struct PgpStringToKey const *s2k, char const *str,
			size_t slen, byte *key, size_t klen)
	{
			unsigned num;
			struct StringToKeyPriv const *priv;
			struct PgpHash const *h;
			void **v;

		pgpAssert(s2k->encodelen == 10);

		if (!klen)
			return 0;	/* Okay, I guess... */

			priv = (struct StringToKeyPriv *)s2k->priv;
			h = priv->hash;
			pgpAssert(h->type == priv->buf[1]);
			num = (klen-1)/h->hashsize + 1;
			v = multiHashCreate(h, num);
			if (!v)
				 return PGPERR_NOMEM;
			multiHashUpdate (h, v, num, priv->buf+2, 8);
			multiHashUpdate (h, v, num, (byte const *)str, slen);
			multiHashFinal(h, v, key, klen);

			return 0;
	}

/*
* The count is stored as 4.4 bit normalized floating-point. The high
* 4 bits are the exponent (with a bias of 6), and the low 4 bits
* are the mantissa. 0x12 corresponds to (16+0x2) << (0x1+6).
* The minimum value is (16+0) << (0+6) = 0x400 = 1024.
* The maximum is (16+0xf) << (0xf+6) = 0x3e00000 = 65011712.
* These functions convert between the expanded count and a
* floating-point approximation.
*/
#define EXPBIAS	6
static word32
c_to_bytes(byte c)
{
	return ((word32)16 + (c & 15)) << ((c >> 4) + EXPBIAS);
}

static byte
bytes_to_c(word32 bytes)
{
			unsigned c;

			if (bytes <= (word32)16 << EXPBIAS)
				 return 0;
			if (bytes >= (word32)31 << (15+EXPBIAS))
				 return 0xff;
			/* Normalize mantissa to 32..63 */
			c = 0;
			for (bytes >>= EXPBIAS-1; bytes >= 64; bytes >>= 1)
				 c++;
			/* Round to 4 bits by adding in low-order bit */
			bytes += (bytes & 1);
			/* Add exponent and mantissa */
			return (byte)((c<<4) + ((unsigned)bytes >> 1));
	}

static int
s2kIterSalt(struct PgpStringToKey const *s2k, char const *str,
			size_t slen, byte *key, size_t klen)
	{
			unsigned num;
			struct StringToKeyPriv const *priv;
			struct PgpHash const *h;
			word32 bytes;
			void **v;

			pgpAssert(s2k->encodelen == 11);

			if (!klen)
				 return 0;	/* Okay, I guess... */

			priv = (struct StringToKeyPriv *)s2k->priv;
			h = priv->hash;
			pgpAssert(h->type == priv->buf[1]);
			num = (klen-1)/h->hashsize + 1;
			v = multiHashCreate(h, num);
			if (!v)
				 return PGPERR_NOMEM;
			/* Find the length of the material to hash */
			bytes = c_to_bytes(priv->buf[10]);
			/* Always hash a least the whole passphrase! */
			if (bytes < slen + 8)
				 bytes = (word32)(slen + 8);

			/* Hash len bytes of (salt, passphrase) repeated... */
			while (bytes > slen + 8) {
				 multiHashUpdate (h, v, num, priv->buf+2, 8);
				 multiHashUpdate (h, v, num, (byte const *)str, slen);
				 bytes -= slen + 8;
			}
			if (bytes <= 8) {
				 multiHashUpdate (h, v, num, priv->buf+2, (size_t)bytes);
			} else {
				 multiHashUpdate (h, v, num, priv->buf+2, 8);
				 multiHashUpdate (h, v, num, (byte const *)str,
				 		 (size_t)bytes-8);
			}
			multiHashFinal(h, v, key, klen);

			return 0;
	}

/* Encoded as \000 + hash specifier */
struct PgpStringToKey *
pgpS2Ksimple(struct PgpEnv const *env, struct PgpHash const *h)
{
			struct PgpStringToKey *s2k;
			byte *buff;

s2k = s2kAlloc(env, h, 2);
if (s2k) {
	s2k->stringToKey = s2kSimple;
	buff = ((struct StringToKeyPriv *)s2k->priv)->buf;
	buff[0] = 0;
}
return s2k;
}

/* Encoded as \001 + hash specifier + salt8 */
struct PgpStringToKey *
pgpS2Ksalted(struct PgpEnv const *env, struct PgpHash const *h,
	byte const *salt8)
{
			struct PgpStringToKey *s2k;
			byte *buff;

			s2k = s2kAlloc(env, h, 10);
			if (s2k) {
					s2k->stringToKey = s2kSalted;
					buff = ((struct StringToKeyPriv *)s2k->priv)->buf;
					buff[0] = 1;
					memcpy(buff+2, salt8, 8);
			}
			return s2k;
	}

/* Encoded as \003 + hash specifier + salt8 + (compressed) count */
static struct PgpStringToKey *
pgpS2Kiterintern(struct PgpEnv const *env, struct PgpHash const *h,
				 byte const *salt8, byte c)
	{
			struct PgpStringToKey *s2k;
			byte *buff;

			s2k = s2kAlloc(env, h, 11);
		if (s2k) {
					s2k->stringToKey = s2kIterSalt;
					buff = ((struct StringToKeyPriv *)s2k->priv)->buf;
					buff[0] = 3;
					memcpy(buff+2, salt8, 8);
					buff[10] = c;
			}
			return s2k;
	}
struct PgpStringToKey *
pgpS2Kitersalt(struct PgpEnv const *env, struct PgpHash const *h,
			byte const *salt8, word32 bytes)
	{
			return pgpS2Kiterintern(env, h, salt8, bytes_to_c(bytes));
	}

/*
* Returns either an error <0 or the length >=0 of the encoded
* string2key data found in the buffer. If s2k is non-NULL, a
* structure is allocated and returned.
*/
int
pgpS2Kdecode(struct PgpStringToKey **s2kp, struct PgpEnv const *env,
			byte const *buff, size_t len)
	{
			struct PgpHash const *h;
			int prefix;

			if (!len)
				 return PGPERR_BAD_STRING2KEY;
			if (buff[0] > 3 || buff[0] == 2)
				 return PGPERR_UNK_STRING2KEY;
			if (len < 2)
				 return PGPERR_BAD_STRING2KEY;
			h = pgpHashByNumber(buff[1]);
			if (!h)
				 return PGPERR_BAD_HASHNUM;
			switch (buff[0]) {
			case 0:
					if (!s2kp || (*s2kp = pgpS2Ksimple(env, h)) != 0)
						return 2;
					break;
				case 1:
					if (len < 10)
						return PGPERR_BAD_STRING2KEY;
					if (!s2kp || (*s2kp = pgpS2Ksalted(env, h, buff+2)) != 0)
						return 10;
					break;
				case 3:
					prefix = 11;
				if (len < 11)
						return PGPERR_BAD_STRING2KEY;
					if (!s2kp || (*s2kp = pgpS2Kiterintern(env, h, buff+2,
								 				buff[10]))
						!= 0)
							return 11;
					break;
			}
			/* Tried to allocate and failed */
			return PGPERR_NOMEM;
	}


/*
* Return the default StringToKey based on the setting in the
* environment. This is expected to be the usual way to get
* such structures.
*/
struct PgpStringToKey *
pgpS2Kdefault(struct PgpEnv const *env, struct PgpRandomContext const *rc)
{
			byte salt[8];

			pgpAssert (rc);

			pgpRandomGetBytes(rc, salt, sizeof(salt));
			return pgpS2Kitersalt(env, pgpHashByNumber(PGP_HASH_SHA),
				 salt, SALT_CHARS_DEFAULT);
}

/*
* Return the default StringToKey for PGP version 2.X
*/
struct PgpStringToKey *
pgpS2KdefaultV2(struct PgpEnv const *env, struct PgpRandomContext const *rc)
{
			/* Kludge for now */
			(void)rc;
			return pgpS2Ksimple(env, pgpHashByNumber(PGP_HASH_MD5));
}



/* Returns 1 if this is simple/md5, 0 otherwise */
int pgpS2KisOldVers (struct PgpStringToKey const *s2k)
{
			byte const *ptr;

			if (s2k->encodelen != 2)
				 return 0;

			ptr = s2k->encoding;
			if (ptr[0] == 0 && ptr[1] == PGP_HASH_MD5)
				 return 1;

			return 0;
}
