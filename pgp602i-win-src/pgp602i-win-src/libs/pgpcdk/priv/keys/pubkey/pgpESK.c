/*
 * $Id: pgpESK.c,v 1.29 1998/04/02 05:37:32 hal Exp $
 */

#include "pgpConfig.h"

#include "pgpDebug.h"
#include "pgpESK.h"
#include "pgpPktList.h"
#include "pgpAnnotate.h"
#include "pgpCFBPriv.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"
#include "pgpEnv.h"
#include "pgpKeyIDPriv.h"

#define ESKTYPE_CONV	(PGP_ESKTYPE_PASSPHRASE<<8)
#define ESKTYPE_NEWCONV ((PGP_ESKTYPE_PASSPHRASE<<8)+1)
#define ESKTYPE_PK	(PGP_ESKTYPE_PUBKEY<<8)
#define ESKTYPE_RSA	((PGP_ESKTYPE_PUBKEY<<8)+kPGPPublicKeyAlgorithm_RSA)

struct PGPESK
{
	PktList pkt;
} ;

/*
 * A PK ESK is:
 *  0  1    Version
 *  1  8    KeyID
 *  9  1    PK alg
 * 10       Algorithm specific data
 */
static int
eskPubkeyValidate(PGPByte const *buf, size_t len)
{
	unsigned bits;

	if (len < 1)
		return kPGPError_BadSessionKeySize;
	if (buf[0] != 2 && buf[0] != 3 && buf[0] != 4)	/* Version byte */
		return kPGPError_UnknownVersion;
	if (len < 10)
		return kPGPError_BadSessionKeySize;
	switch (buf[9]) {	/* Algorithm Byte */
	  case kPGPPublicKeyAlgorithm_RSA:
		bits = ((unsigned)buf[10]<<8) + buf[11];
		if (len != 12+(bits+7)/8)
			return len < 12+(bits+7)/8 ? kPGPError_BadSessionKeySize :
						     kPGPError_BadSessionKeySize;
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
eskConvkeyValidate (PGPByte const *buf, size_t len)
{
	PGPCipherVTBL const *c;
	int ret;

	if (!len)
		return ESKTYPE_CONV;
	if (len < 3)
		return kPGPError_BadSessionKeySize;
	if (buf[0] != 4)	/* Version byte */
		return kPGPError_UnknownVersion;

	/* Try to decode the ESK */
	c = pgpCipherGetVTBL ( (PGPCipherAlgorithm)buf[1]);
	if (!c)
		return kPGPError_BadSessionKeyAlgorithm;

	/* Decode the string to key algorithm */
	ret = pgpS2Kdecode (NULL, NULL, buf+2, len-2);
	if (ret < 0)
		return ret;
	if (ret == 0)
		return kPGPError_BadSessionKeyAlgorithm;
	len -= 2+ret;
	if (len == 0)
		return ESKTYPE_NEWCONV;
	/* Note that message may be encrypted with different algorithm than ESK */
	return ESKTYPE_NEWCONV;
}

static PGPESK **
eskListTail(PGPESK **head)
{
	while (*head)
		head = (PGPESK **)&(*head)->pkt.next;
	return head;
}

/* Add an PGPESK to a list thereof */
int
pgpEskAdd(
	PGPContextRef	cdkContext,
	PGPESK **esklist, int type, PGPByte const *buf, size_t len)
{
	PGPESK *esk;
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
		return kPGPError_BadParams;
	}
	esk = (PGPESK *)pgpPktListNew( cdkContext, i, buf, len);
	*eskListTail(esklist) = esk;
	return esk ? 0 : kPGPError_OutOfMemory;
}

/* A few trivial access functions */

void
pgpEskFreeList (PGPESK *esklist)
{
	pgpPktListFreeList((PktList *)esklist);
}

PGPESK *
pgpEskNext (PGPESK const *esk)
{
	if (esk)
		return (PGPESK *)esk->pkt.next;

	return NULL;
}

int
pgpEskType(PGPESK const *esk)
{
	return esk->pkt.type >> 8;
}

/* Returns PGP_PKALG_xxx */
int
pgpEskPKAlg(PGPESK const *esk)
{
	pgpAssert(esk->pkt.type >> 8 == PGP_ESKTYPE_PUBKEY);
	return esk->pkt.type & 255;
}

	static PGPByte const *
pgpEskId8(PGPESK const *esk)
{
	pgpAssert(esk->pkt.type >> 8 == PGP_ESKTYPE_PUBKEY);
	return esk->pkt.buf+1;
}


	PGPError
pgpGetEskKeyID(
	PGPESK const *	esk,
	PGPKeyID *		outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= pgpNewKeyIDFromRawData( pgpEskId8( esk ), 8, outRef );
	
	return( err );
}


/*
 * Return the size of the largest possible key that may
 * be produced as output from and eskXXdecrypt() function.
 */
size_t
pgpEskMaxKeySize (PGPESK const *esk)
{
	switch (pgpEskType (esk)) {
	case PGP_ESKTYPE_PASSPHRASE:
		return (size_t) (pgpMax ((int)25, (int)(((int)esk->pkt.len)-3)));
	case PGP_ESKTYPE_PUBKEY:
		/* XXX -- this only works with RSA, no?  Need a PGPSecKey
		   object to do better, though.*/
		return pgpMax (25, esk->pkt.len-10);
	}
	return 0;
}

/*
 * The "key" is from a pass phrase.  Returns the length of the key,
 * which is <algorithm identifier><key bits>.  The actual key is
 * obtained via a string-to-key operation.
 */
int
pgpEskConvDecrypt(PGPESK const *esk, PGPEnv const *env,
		  char const *pass, size_t plen, PGPByte *buf)
{
	PGPStringToKey *s2k = NULL;
	PGPCFBContext *cfb = NULL;
	int ret = 0;
	PGPContextRef		cdkContext	= pgpenvGetContext( env );

	switch (esk->pkt.type & 0xff) {
		PGPHashVTBL const *hash;
		PGPCipherVTBL const *c;
		PGPByte *key;
		PGPByte const *off;
		int len;
	case 0:
		/* Old-style (non ConvESK) */
		hash = pgpHashByNumber(kPGPHashAlgorithm_MD5);
		if (!hash)
			return kPGPError_BadHashNumber;
		c = pgpCipherGetVTBL (kPGPCipherAlgorithm_IDEA);
		if (!c)
			return kPGPError_BadCipherNumber;
		s2k = pgpS2Ksimple (env, hash);
		buf[0] = kPGPCipherAlgorithm_IDEA;
		ret = pgpStringToKey (s2k, pass, plen, buf+1, c->keysize);
		if (!ret)
			ret = c->keysize+1;
		break;
	case 1:
		c = pgpCipherGetVTBL ( (PGPCipherAlgorithm)esk->pkt.buf[1]);
		if (!c)
			return kPGPError_BadCipherNumber;
		ret = pgpS2Kdecode (&s2k, env, esk->pkt.buf+2, esk->pkt.len-2);
		if (ret < 0)
			return ret;
		if (!s2k)
			return kPGPError_OutOfMemory;
		if (esk->pkt.len-2-ret != 0) {
			/* We have an ESK in the packet */
			off = esk->pkt.buf+2+ret; /* ESK starts here */
			len = esk->pkt.len-2-ret; /* ESK length */

			cfb = pgpCFBCreate ( PGPGetContextMemoryMgr( cdkContext ), c);
			if (!cfb) {
				ret = kPGPError_OutOfMemory;
				break;
			}
			key = (PGPByte *)pgpContextMemAlloc( cdkContext,
				c->keysize, kPGPMemoryMgrFlags_Clear);
			if (!key) {
				ret = kPGPError_OutOfMemory;
				break;
			}
			ret = pgpStringToKey (s2k, pass, plen, key,
					      c->keysize);
			if (ret) {
				pgpClearMemory( key,  c->keysize);
				pgpContextMemFree( cdkContext, key);
				break;
			}
			PGPInitCFB (cfb, key, NULL);
			pgpClearMemory( key,  c->keysize);
			pgpContextMemFree( cdkContext, key);
			PGPCFBDecrypt (cfb, off, len, buf);
			pgpCFBWipe (cfb);
			ret = len;
		} else {
			/* this is just a key to use */
			buf[0] = c->algorithm;
			ret = pgpStringToKey (s2k, pass, plen, buf+1,
					      c->keysize);
			if (!ret)
				ret = c->keysize+1;
		}
		break;
	default:
		ret = kPGPError_BadParams;
	}

	if (s2k)
		pgpS2Kdestroy (s2k);
	if (cfb)
		PGPFreeCFBContext(cfb);

	return ret;
}

/*
 * Do public-key decryption - buf must be "big enough" as defined by
 * pgpSecKeyMaxDecrypted().  Returns the length of the decrypted key, or
 * <0 on error.
 */
int
pgpEskPKdecrypt(PGPESK const *esk, PGPEnv const *env,
		PGPSecKey *sec, PGPByte *buf)
{
	size_t len;
	int err;

	pgpAssert (sec->decrypt);
	/* XXX should "die" gracefully, here */

	err = pgpSecKeyDecrypt(sec, env, esk->pkt.buf+10, esk->pkt.len-10,
						   buf, &len, NULL, 0, kPGPPublicKeyMessageFormat_PGP);
	if (err)
		return err;
	return (int) len;
}
