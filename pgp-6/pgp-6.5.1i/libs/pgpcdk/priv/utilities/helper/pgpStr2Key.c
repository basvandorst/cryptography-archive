/*
 * pgpStr2Key.c -- A prototype string-to-key framework.
 *
 * $Id: pgpStr2Key.c,v 1.26 1998/06/11 18:28:25 hal Exp $
 */

#include "pgpConfig.h"

#include <string.h>
#include <stddef.h>	/* For offsetof() */

#include "pgpDebug.h"
#include "pgpStr2Key.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpRandomX9_17.h"
#include "pgpEnv.h"
#include "pgpContext.h"

/* Number of chars to hash when creating default S2K */
#define SALT_CHARS_DEFAULT		32768

/* It turns out that they all use the same private context structure */
typedef struct StringToKeyPriv {
	PGPEnv const *env;
	PGPHashVTBL const *hash;
	PGPByte buf[1];	/* Variable-sized */
	DEBUG_STRUCT_CONSTRUCTOR( StringToKeyPriv )
} StringToKeyPriv ;

/* And it can be destroyed with a common routine */
static void
s2kDestroy(PGPStringToKey *s2k)
{
	
	StringToKeyPriv *priv = (StringToKeyPriv *)s2k->priv;
	
	PGPContextRef		cdkContext	= pgpenvGetContext( priv->env );
	
	pgpClearMemory( priv,  s2k->encodelen+offsetof(StringToKeyPriv, buf));
	pgpContextMemFree( cdkContext, priv);
	pgpClearMemory( s2k,  sizeof(*s2k));
	pgpContextMemFree( cdkContext, s2k);
}

static PGPStringToKey *
s2kAlloc(PGPEnv const *env, PGPHashVTBL const *h, unsigned size)
{
	StringToKeyPriv *priv;
	PGPStringToKey *s2k = NULL;
	PGPContextRef		cdkContext	= pgpenvGetContext( env );

	priv = (StringToKeyPriv *)
		pgpContextMemAlloc( cdkContext,
			offsetof(StringToKeyPriv, buf) + size, kPGPMemoryMgrFlags_Clear);
	if (priv) {
		s2k = (PGPStringToKey *)pgpContextMemAlloc( cdkContext,
			sizeof(*s2k), kPGPMemoryMgrFlags_Clear);
		if ( IsntNull( s2k ) ) {
			s2k->priv = priv;
			s2k->encoding = priv->buf;
			s2k->encodelen = size;
			s2k->destroy = s2kDestroy;
			priv->env = env;
			priv->hash = h;
			if (h)
				priv->buf[1] = h->algorithm;
		}
		else
		{
			pgpContextMemFree( cdkContext, priv );
			priv	= NULL;
		}
	}
	return s2k;
}

/* Allocate an array of "num" hash private buffers, all sharing the same hash*/
static void **
multiHashCreate(
	PGPContextRef	cdkContext,
	PGPHashVTBL const *h, unsigned num)
{
	void **v;
	void *p;
	unsigned i, j;
	PGPByte const b = 0;

	v = (void **)pgpContextMemAlloc( cdkContext,
		num * sizeof(*v), kPGPMemoryMgrFlags_Clear);
	if (!v)
		return NULL;

	for (i = 0; i < num; i++) {
		p = pgpContextMemAlloc( cdkContext,
			h->context_size, kPGPMemoryMgrFlags_Clear);
		if (!p) {
			while (i) {
				pgpClearMemory(v[--i], h->context_size);
				pgpContextMemFree( cdkContext, v[i]);
			}
			pgpContextMemFree( cdkContext, v);
			return NULL;
		}
		h->init(p);
		/* Initialze the PGPHashContext with leading null bytes */
		for (j = 0; j < i; j++)
			h->update(p, &b, 1);
		v[i] = p;
	}
	return v;
}

/* Update an array of hash private buffers, all sharing the same hash */
static void
multiHashUpdate(PGPHashVTBL const *h, void * const *v, unsigned num,
	PGPByte const *string, size_t len)
{
	while (num--)
		h->update(*v++, string, len);
}

/*
 * Extract the final combined string from an array of hash private buffers,
 * then wipe and free them.
 */
static void
multiHashFinal(
	PGPContextRef	cdkContext,
	PGPHashVTBL const *h, void **v, PGPByte *key, size_t klen)
{
	void **v0 = v;
	unsigned hsize = h->hashsize;

	while (klen > hsize) {
		memcpy(key, h->final(*v), hsize);
		key += hsize;
		klen -= hsize;
		pgpClearMemory(*v, h->context_size);
		pgpContextMemFree( cdkContext, *(v++) );
	}
	memcpy(key, h->final(*v), klen);
	pgpClearMemory(*v, h->context_size);
	pgpContextMemFree( cdkContext, *v);

	pgpContextMemFree( cdkContext, v0);
}

static int
s2kSimple(PGPStringToKey const *s2k, char const *str,
	size_t slen, PGPByte *key, size_t klen)
{
	unsigned num;
	StringToKeyPriv const *priv;
	PGPHashVTBL const *h;
	void **v;
	PGPContextRef		cdkContext;

	pgpAssert(s2k->encodelen == 2);

	if (!klen)
		return 0;	/* Okay, I guess... */

	priv = (StringToKeyPriv *)s2k->priv;
	h = priv->hash;
	pgpAssert(h->algorithm == priv->buf[1]);
	cdkContext	= pgpenvGetContext( priv->env );
	
	num = (klen-1)/h->hashsize + 1;
	v = multiHashCreate( cdkContext, h, num);
	if (!v)
		return kPGPError_OutOfMemory;
	multiHashUpdate (h, v, num, (PGPByte const *)str, slen);
	multiHashFinal( cdkContext, h, v, key, klen);

	return 0;
}


static int
s2kSalted(PGPStringToKey const *s2k, char const *str,
	size_t slen, PGPByte *key, size_t klen)
{
	unsigned num;
	StringToKeyPriv const *priv;
	PGPHashVTBL const *h;
	void **v;
	PGPContextRef		cdkContext	= NULL;

	pgpAssert(s2k->encodelen == 10);

	if (!klen)
		return 0;	/* Okay, I guess... */

	priv = (StringToKeyPriv *)s2k->priv;
	cdkContext	= pgpenvGetContext( priv->env );
	h = priv->hash;
	pgpAssert(h->algorithm == priv->buf[1]);
	num = (klen-1)/h->hashsize + 1;
	v = multiHashCreate( cdkContext, h, num);
	if (!v)
		return kPGPError_OutOfMemory;
	multiHashUpdate ( h, v, num, priv->buf+2, 8);
	multiHashUpdate ( h, v, num, (PGPByte const *)str, slen);
	multiHashFinal( cdkContext, h, v, key, klen);

	return 0;
}

/*
 * The count is stored as 4.4 bit normalized floating-point.  The high
 * 4 bits are the exponent (with a bias of 6), and the low 4 bits
 * are the mantissa.  0x12 corresponds to (16+0x2) << (0x1+6).
 * The minimum value is (16+0) << (0+6) = 0x400 = 1024.
 * The maximum is (16+0xf) << (0xf+6) = 0x3e00000 = 65011712.
 * These functions convert between the expanded count and a
 * floating-point approximation.
 */
#define EXPBIAS	6
static PGPUInt32
c_to_bytes(PGPByte c)
{
	return ((PGPUInt32)16 + (c & 15)) << ((c >> 4) + EXPBIAS);
}

static PGPByte
bytes_to_c(PGPUInt32 bytes)
{
	unsigned c;

	if (bytes <= (PGPUInt32)16 << EXPBIAS)
		return 0;
	if (bytes >= (PGPUInt32)31 << (15+EXPBIAS))
		return 0xff;
	/* Normalize mantissa to 32..63 */
	c = 0;
	for (bytes >>= EXPBIAS-1; bytes >= 64; bytes >>= 1)
		c++;
	/* Round to 4 bits by adding in low-order bit */
	bytes += (bytes & 1);
	/* Add exponent and mantissa */
	return (PGPByte)((c<<4) + ((unsigned)bytes >> 1));
}

static int
s2kIterSalt(PGPStringToKey const *s2k, char const *str,
	size_t slen, PGPByte *key, size_t klen)
{
	unsigned num;
	StringToKeyPriv const *priv;
	PGPHashVTBL const *h;
	PGPUInt32 bytes;
	void **v;
	PGPContextRef		cdkContext	= NULL;

	pgpAssert(s2k->encodelen == 11);

	if (!klen)
		return 0;	/* Okay, I guess... */

	priv = (StringToKeyPriv *)s2k->priv;
	cdkContext	= pgpenvGetContext( priv->env );
	h = priv->hash;
	pgpAssert(h->algorithm == priv->buf[1]);
	num = (klen-1)/h->hashsize + 1;
	v = multiHashCreate( cdkContext, h, num);
	if (!v)
		return kPGPError_OutOfMemory;
	/* Find the length of the material to hash */
	bytes = c_to_bytes(priv->buf[10]);
	/* Always hash a least the whole passphrase! */
	if (bytes < slen + 8)
		bytes = (PGPUInt32)(slen + 8);

	/* Hash len bytes of (salt, passphrase) repeated... */
	while (bytes > slen + 8) {
		multiHashUpdate (h, v, num, priv->buf+2, 8);
		multiHashUpdate (h, v, num, (PGPByte const *)str, slen);
		bytes -= slen + 8;
	}
	if (bytes <= 8) {
		multiHashUpdate (h, v, num, priv->buf+2, (size_t)bytes);
	} else {
		multiHashUpdate (h, v, num, priv->buf+2, 8);
		multiHashUpdate (h, v, num, (PGPByte const *)str,
				 (size_t)bytes-8);
	}
	multiHashFinal( cdkContext, h, v, key, klen);

	return 0;
}

static int
s2kLiteral(PGPStringToKey const *s2k, char const *str,
	size_t slen, PGPByte *key, size_t klen)
{
	(void)s2k;
	pgpAssert(s2k->encodelen == 1);

	(void) s2k;
	
	if (!klen)
		return 0;	/* Okay, I guess... */

	/* We will use the str literally, so sizes must match */
	if (klen != slen)
		return kPGPError_BadParams;

	pgpCopyMemory( str, key, klen );
	return 0;
}


/* Encoded as \000 + hash specifier */
PGPStringToKey *
pgpS2Ksimple(PGPEnv const *env, PGPHashVTBL const *h)
{
	PGPStringToKey *s2k;
	PGPByte *buff;

	s2k = s2kAlloc(env, h, 2);
	if (s2k) {
		s2k->stringToKey = s2kSimple;
		buff = ((StringToKeyPriv *)s2k->priv)->buf;
		buff[0] = kPGPStringToKey_Simple;
	}
	return s2k;
}

/* Encoded as \001 + hash specifier + salt8 */
PGPStringToKey *
pgpS2Ksalted(PGPEnv const *env, PGPHashVTBL const *h,
	     PGPByte const *salt8)
{
	PGPStringToKey *s2k;
	PGPByte *buff;

	s2k = s2kAlloc(env, h, 10);
	if (s2k) {
		s2k->stringToKey = s2kSalted;
		buff = ((StringToKeyPriv *)s2k->priv)->buf;
		buff[0] = kPGPStringToKey_Salted;
		memcpy(buff+2, salt8, 8);
	}
	return s2k;
}

/* Encoded as \003 + hash specifier + salt8 + (compressed) count */
static PGPStringToKey *
pgpS2Kiterintern(PGPEnv const *env, PGPHashVTBL const *h,
		 PGPByte const *salt8, PGPByte c)
{
	PGPStringToKey *s2k;
	PGPByte *buff;

	s2k = s2kAlloc(env, h, 11);
	if (s2k) {
		s2k->stringToKey = s2kIterSalt;
		buff = ((StringToKeyPriv *)s2k->priv)->buf;
		buff[0] = kPGPStringToKey_IteratedSalted;
		memcpy(buff+2, salt8, 8);
		buff[10] = c;
	}
	return s2k;
}
PGPStringToKey *
pgpS2Kitersalt(PGPEnv const *env, PGPHashVTBL const *h,
	       PGPByte const *salt8, PGPUInt32 bytes)
{
	return pgpS2Kiterintern(env, h, salt8, bytes_to_c(bytes));
}

/* Encoded as \004 */
PGPStringToKey *
pgpS2Kliteral(PGPEnv const *env)
{
	PGPStringToKey *s2k;
	PGPByte *buff;

	s2k = s2kAlloc(env, NULL, 1);
	if (s2k) {
		s2k->stringToKey = s2kLiteral;
		buff = ((StringToKeyPriv *)s2k->priv)->buf;
		buff[0] = kPGPStringToKey_Literal;
	}
	return s2k;
}

/* Encoded as \005, like literal but flags secret sharing */
PGPStringToKey *
pgpS2KliteralShared(PGPEnv const *env)
{
	PGPStringToKey *s2k;
	PGPByte *buff;

	s2k = s2kAlloc(env, NULL, 1);
	if (s2k) {
		s2k->stringToKey = s2kLiteral;
		buff = ((StringToKeyPriv *)s2k->priv)->buf;
		buff[0] = kPGPStringToKey_LiteralShared;
	}
	return s2k;
}


/*
 * Returns either an error <0 or the length >=0 of the encoded
 * string2key data found in the buffer.  If s2k is non-NULL, a
 * structure is allocated and returned.
 */
int
pgpS2Kdecode(PGPStringToKey **s2kp, PGPEnv const *env,
	     PGPByte const *buff, size_t len)
{
	PGPHashVTBL const *h;

	if (!len)
		return kPGPError_BadParams;
	switch (buff[0]) {
	case (int)kPGPStringToKey_Simple:
		if (len < 2)
			return kPGPError_BadParams;
		h = pgpHashByNumber( (PGPHashAlgorithm) buff[1]);
		if (!h)
			return kPGPError_BadHashNumber;
		if (!s2kp || (*s2kp = pgpS2Ksimple(env, h)) != 0)
			return 2;
		break;
	case (int)kPGPStringToKey_Salted:
		if (len < 10)
			return kPGPError_BadParams;
		h = pgpHashByNumber( (PGPHashAlgorithm) buff[1]);
		if (!h)
			return kPGPError_BadHashNumber;
		if (!s2kp || (*s2kp = pgpS2Ksalted(env, h, buff+2)) != 0)
			return 10;
		break;
	case (int)kPGPStringToKey_IteratedSalted:
		if (len < 11)
			return kPGPError_BadParams;
		h = pgpHashByNumber( (PGPHashAlgorithm) buff[1]);
		if (!h)
			return kPGPError_BadHashNumber;
		if (!s2kp || (*s2kp = pgpS2Kiterintern(env, h, buff+2,
						       buff[10])) != 0)
			return 11;
		break;
	case (int)kPGPStringToKey_Literal:
		if (len < 1)
			return kPGPError_BadParams;
		if (!s2kp || (*s2kp = pgpS2Kliteral(env)) != 0)
			return 1;
		break;
	case (int)kPGPStringToKey_LiteralShared:
		if (len < 1)
			return kPGPError_BadParams;
		if (!s2kp || (*s2kp = pgpS2KliteralShared(env)) != 0)
			return 1;
		break;
	default:
		return kPGPError_UnknownString2Key;
	}
	/* Tried to allocate and failed */
	return kPGPError_OutOfMemory;
}


/*
 * Return the default StringToKey based on the setting in the
 * environment.  This is expected to be the usual way to get
 * such structures.
 */
PGPStringToKey *
pgpS2Kdefault(PGPEnv const *env, PGPRandomContext const *rc)
{
	PGPByte salt[8];

	pgpAssert (rc);

	pgpRandomGetBytes(rc, salt, sizeof(salt));
	return pgpS2Kitersalt(env, pgpHashByNumber(kPGPHashAlgorithm_SHA),
		salt, SALT_CHARS_DEFAULT);
}

/*
 * Return the default StringToKey for PGP version 2.X
 */
PGPStringToKey *
pgpS2KdefaultV3(PGPEnv const *env, PGPRandomContext const *rc)
{
	/* Kludge for now */
	(void)rc;
	return pgpS2Ksimple(env, pgpHashByNumber(kPGPHashAlgorithm_MD5));
}

/*
 * Return a new StringToKey object of the specified type
 */
PGPStringToKey *
pgpS2Kcreate(PGPEnv const *env, PGPRandomContext const *rc,
	PGPStringToKeyType s2ktype)
{
	PGPHashAlgorithm hashalg = kPGPHashAlgorithm_SHA;

	if (s2ktype == kPGPStringToKey_Simple) {
		return pgpS2Ksimple(env, pgpHashByNumber(hashalg));
	} else if (s2ktype == kPGPStringToKey_Literal) {
		return pgpS2Kliteral(env);
	} else if (s2ktype == kPGPStringToKey_LiteralShared) {
		return pgpS2KliteralShared(env);
	} else {
		/* Need a salt */
		PGPByte salt[8];

		pgpAssert (rc);
		pgpRandomGetBytes(rc, salt, sizeof(salt));
		if (s2ktype == kPGPStringToKey_Salted) {
			return pgpS2Ksalted(env, pgpHashByNumber(hashalg), salt);
		} else if (s2ktype == kPGPStringToKey_IteratedSalted) {
			return pgpS2Kitersalt(env, pgpHashByNumber(hashalg),
								  salt, SALT_CHARS_DEFAULT);
		}
	}
	/* Unknown stringtokey type */
	return NULL;
}


/* Returns 1 if this is simple/md5, 0 otherwise */
int pgpS2KisOldVers (PGPStringToKey const *s2k)
{
	PGPByte const *ptr;

	if (s2k->encodelen != 2)
		return 0;

	ptr = s2k->encoding;
	if (ptr[0] == 0 && ptr[1] == kPGPHashAlgorithm_MD5)
		return 1;

	return 0;
}
