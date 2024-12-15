/*
 * pgpESK.c
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpESK.c,v 1.2.2.1 1997/06/07 09:51:24 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpESK.h"
#include "pgpPktList.h"
#include "pgpAnnotate.h"
#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"

#define ESKTYPE_CONV	(PGP_ESKTYPE_PASSPHRASE<<8)
#define ESKTYPE_NEWCONV ((PGP_ESKTYPE_PASSPHRASE<<8)+1)
#define ESKTYPE_PK	(PGP_ESKTYPE_PUBKEY<<8)
#define ESKTYPE_RSA	((PGP_ESKTYPE_PUBKEY<<8)+PGP_PKALG_RSA)

struct PgpESK {
	struct PktList pkt;
};

/*
 * A PK ESK is:
 *  0  1    Version
 *  1  8    KeyID
 *  9  1    PK alg
 * 10       Algorithm specific data
 */
static int
eskPubkeyValidate(byte const *buf, size_t len)
{
	unsigned bits;

	if (len < 1)
		return PGPERR_ESK_TOOSHORT;
	if (buf[0] != 2 && buf[0] != 3 && buf[0] != 4)	/* Version byte */
		return PGPERR_ESK_BADVERSION;
	if (len < 10)
		return PGPERR_ESK_TOOSHORT;
	switch (buf[9]) {	/* Algorithm Byte */
	  case PGP_PKALG_RSA:
		bits = ((unsigned)buf[10]<<8) + buf[11];
		if (len != 12+(bits+7)/8)
			return len < 12+(bits+7)/8 ? PGPERR_ESK_TOOSHORT :
						     PGPERR_ESK_TOOLONG;
		return ESKTYPE_RSA;
	}
	return ESKTYPE_PK + buf[9];
}

/*
 * A SK ESK is:
 *  0  1    Version
 *  1  1    Cipher
 *  2  x    StringToKey (x >= 1)
 * 2+x y    ESK (y >= 0)
 */
static int
eskConvkeyValidate (byte const *buf, size_t len)
{
	struct PgpCipher const *c;
	int ret;

	if (!len)
		return ESKTYPE_CONV;
	if (len < 3)
		return PGPERR_ESK_TOOSHORT;
	if (buf[0] != 4)	/* Version byte */
		return PGPERR_ESK_BADVERSION;

	/* Try to decode the ESK */
	c = pgpCipherByNumber (buf[1]);
	if (!c)
		return PGPERR_ESK_BADALGORITHM;

	/* Decode the string to key algorithm */
	ret = pgpS2Kdecode (NULL, NULL, buf+2, len-2);
	if (ret < 0)
		return ret;
	if (ret == 0)
		return PGPERR_ESK_BADALGORITHM;
	len -= 2+ret;
	if (len == 0)
		return ESKTYPE_NEWCONV;
	if (len < c->keysize+1)
		return PGPERR_ESK_TOOSHORT;
	if (len > c->keysize+1)
		return PGPERR_ESK_TOOLONG;
	return ESKTYPE_NEWCONV;
}

static struct PgpESK **
eskListTail(struct PgpESK **head)
{
	while (*head)
		head = (struct PgpESK **)&(*head)->pkt.next;
	return head;
}

/* Add an PgpESK to a list thereof */
int
pgpEskAdd(struct PgpESK **esklist, int type, byte const *buf, size_t len)
{
	struct PgpESK *esk;
	int i;

	switch (type) {
	  case PGPANN_SKCIPHER_ESK:
		i = eskConvkeyValidate(buf, len);
		if (i < 0)
			return i;
		break;
	  case PGPANN_PKCIPHER_ESK:
		i = eskPubkeyValidate(buf, len);
		if (i < 0)
			return i;
		break;
	  default:
		return PGPERR_ESK_BADTYPE;
	}
	esk = (struct PgpESK *)pgpPktListNew(i, buf, len);
	*eskListTail(esklist) = esk;
	return esk ? 0 : PGPERR_NOMEM;
}

/* A few trivial access functions */

void
pgpEskFreeList (struct PgpESK *esklist)
{
	pgpPktListFreeList((struct PktList *)esklist);
}

struct PgpESK *
pgpEskNext (struct PgpESK const *esk)
{
	if (esk)
		return (struct PgpESK *)esk->pkt.next;

	return NULL;
}

int
pgpEskType(struct PgpESK const *esk)
{
	return esk->pkt.type >> 8;
}

/* Returns PGP_PKALG_xxx */
int
pgpEskPKAlg(struct PgpESK const *esk)
{
	pgpAssert(esk->pkt.type >> 8 == PGP_ESKTYPE_PUBKEY);
	return esk->pkt.type & 255;
}

byte const *
pgpEskId8(struct PgpESK const *esk)
{
	pgpAssert(esk->pkt.type >> 8 == PGP_ESKTYPE_PUBKEY);
	return esk->pkt.buf+1;
}

/*
 * Return the size of the largest possible key that may
 * be produced as output from and eskXXdecrypt() function.
 */
size_t
pgpEskMaxKeySize (struct PgpESK const *esk)
{
	switch (pgpEskType (esk)) {
	case PGP_ESKTYPE_PASSPHRASE:
		return (size_t) (max ((int)25, (int)(((int)esk->pkt.len)-3)));
	case PGP_ESKTYPE_PUBKEY:
		/* XXX -- this only works with RSA, no?  Need a PgpSecKey
		   object to do better, though.*/
		return max (25, esk->pkt.len-10);
	}
	return 0;
}

/*
 * The "key" is from a pass phrase.  Returns the length of the key,
 * which is <algorithm identifier><key bits>.  The actual key is
 * obtained via a string-to-key operation.
 */
int
pgpEskConvDecrypt(struct PgpESK const *esk, struct PgpEnv const *env,
		  char const *pass, size_t plen, byte *buf)
{
	struct PgpStringToKey *s2k = NULL;
	struct PgpCfbContext *cfb = NULL;
	int ret = 0;

	switch (esk->pkt.type & 0xff) {
		struct PgpHash const *hash;
		struct PgpCipher const *c;
		byte *key;
		byte const *off;
		int len;
	case 0:
		/* Old-style (non ConvESK) */
		hash = pgpHashByNumber(PGP_HASH_MD5);
		if (!hash)
			return PGPERR_BAD_HASHNUM;
		c = pgpCipherByNumber (PGP_CIPHER_IDEA);
		if (!c)
			return PGPERR_BAD_CIPHERNUM;
		s2k = pgpS2Ksimple (env, hash);
		buf[0] = PGP_CIPHER_IDEA;
		ret = pgpStringToKey (s2k, pass, plen, buf+1, c->keysize);
		if (!ret)
			ret = c->keysize+1;
		break;
	case 1:
		c = pgpCipherByNumber (esk->pkt.buf[1]);
		if (!c)
			return PGPERR_BAD_CIPHERNUM;
		ret = pgpS2Kdecode (&s2k, env, esk->pkt.buf+2, esk->pkt.len-2);
		if (ret < 0)
			return ret;
		if (!s2k)
			return PGPERR_NOMEM;
		if (esk->pkt.len-2-ret != 0) {
			/* We have an ESK in the packet */
			off = esk->pkt.buf+2+ret; /* ESK starts here */
			len = esk->pkt.len-2-ret; /* ESK length */

			cfb = pgpCfbCreate (c);
			if (!cfb) {
				ret = PGPERR_NOMEM;
				break;
			}
			key = (byte *)pgpMemAlloc (c->keysize);
			if (!key) {
				ret = PGPERR_NOMEM;
				break;
			}
			ret = pgpStringToKey (s2k, pass, plen, key,
					      c->keysize);
			if (ret) {
				memset (key, 0, c->keysize);
				pgpMemFree (key);
				break;
			}
			pgpCfbInit (cfb, key, NULL);
			memset (key, 0, c->keysize);
			pgpMemFree (key);
			pgpCfbDecrypt (cfb, off, buf, len);
			pgpCfbWipe (cfb);
			ret = len;
		} else {
			/* this is just a key to use */
			buf[0] = c->type;
			ret = pgpStringToKey (s2k, pass, plen, buf+1,
					      c->keysize);
			if (!ret)
				ret = c->keysize+1;
		}
		break;
	default:
		ret = PGPERR_BADPARAM;
	}

	if (s2k)
		pgpS2Kdestroy (s2k);
	if (cfb)
		pgpCfbDestroy (cfb);

	return ret;
}

/*
 * Do public-key decryption - buf must be "big enough" as defined by
 * pgpSecKeyMaxDecrypted().  Returns the length of the decrypted key, or
 * <0 on error.
 */
int
pgpEskPKdecrypt(struct PgpESK const *esk, struct PgpEnv const *env,
		struct PgpSecKey *sec, byte *buf)
{
	size_t len;
	int err;

	pgpAssert (sec->decrypt);
	/* XXX should "die" gracefully, here */

	err = pgpSecKeyDecrypt(sec, env, pgpEskPKAlg(esk), esk->pkt.buf+10,
			       esk->pkt.len-10, buf, &len, NULL, 0);
	if (err)
		return err;
	return (int) len;
}
