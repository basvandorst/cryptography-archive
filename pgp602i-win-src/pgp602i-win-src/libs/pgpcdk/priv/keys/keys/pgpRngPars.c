/*
 * pgpRngPars.c - parse structures from a keyring.  Validates its
 * inpout very carefully, for use by ringopen.c.
 * Also includes routines to "unparse" and assemble keyring packets.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPars.c,v 1.37 1998/06/11 18:28:11 hal Exp $
 */
#include "pgpConfig.h"

#include <string.h>

#include "pgpDebug.h"
#include "pgpRngPars.h"
#include "pgpRngPriv.h"	/* For ringHashBuf, pgpFingerprint20HashBuf */
#include "pgpHashPriv.h"
#include "pgpUsuals.h"
#include "pgpKeySpec.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpPktByte.h"
#include "pgpSigSpec.h"
#include "pgpMem.h"

#ifndef NULL
#define NULL 0
#endif


/*
 * Read a key from the buffer and extract vital statistics.
 * If the KeyID cannot be determined, a fake keyID with a 32-bit CRC
 * stuffed in the middle is created.  This is designed to assist
 * matching of bad keys.
 *
 * A key is:
 *  0 Version: 1 byte
 *  1 Timestamp: 4 bytes
 *  5 Validity: 2 bytes  (skipped if version PGPVERSION_4)
 *  7 Algorithm: 1 byte
 * [Algorithm-specific portion]
 *  8 Some MP number from which we can extract key ID: modulus, etc.
 * [Other algorithm-specific fields]
 * [with possible extra data if it's a secret key]
 *
 * If "secretf" is set, additional data is allowed at the end of the
 * packet.  If it is clear, additional data results in kPGPError_KEY_LONG.
 * (The format of the extra data is not checked.)
 */
int
ringKeyParse(PGPContextRef context, PGPByte const *buf, size_t len,
		PGPByte *pkalg, PGPByte keyID[8], PGPUInt16 *keybits,
		PGPUInt32 *tstamp, PGPUInt16 *validity, PGPByte *v3, int secretf)
{
	unsigned mlen, elen;
	int err;
	unsigned vsize;

	/* Defaults in case of error */
	if (v3)
		*v3 = 0;
	if (keyID)
		pgpClearMemory( keyID,  8);
	if (keybits)
		*keybits = 0;
	if (tstamp)
		*tstamp = 0;
	if (validity)
		*validity = 0;
	if (pkalg)
		*pkalg = 0;

	if (len < 1)
		goto fakeid_short;

	if (!KNOWN_PGP_VERSION(buf[0])) {
		err = kPGPError_UnknownKeyVersion;
		goto fakeid;
	}
	/* PGPVERSION_4 keys have no validity field */
	if (buf[0] == PGPVERSION_4) {
		*v3 = FALSE;
		vsize = 0;
	} else {
		*v3 = TRUE;
		vsize = 2;
	}
	if (len < 5)
		goto fakeid_short;

	if (tstamp)
		*tstamp = (PGPUInt32)((unsigned)buf[1] << 8 | buf[2]) << 16 |
			((unsigned)buf[3] << 8 | buf[4]);
	if (len < 5 + vsize)
		goto fakeid_short;
	if (validity  &&  vsize)
		*validity = (unsigned)buf[5] << 8 | buf[6];
	if (len < 6 + vsize)
		goto fakeid_short;
	if (pkalg)
		*pkalg = buf[5 + vsize];
	if (len < 8 + vsize)
		goto fakeid_short;

	/* Get bytes in modulus */
	mlen = (unsigned)buf[6 + vsize] << 8 | buf[7 + vsize];
	if (keybits)
		*keybits = (PGPUInt16)mlen;

	if (mlen && len > 8 + vsize && buf[8 + vsize] >> ((mlen-1)&7) != 1) {
		err = kPGPError_MalformedKeyModulus;
		goto fakeid;
	}
	mlen = (mlen + 7) / 8;
	if (len < 8 + vsize + mlen)
		goto fakeid_short;

	if (len < 10 + vsize + mlen)
		return kPGPError_KeyPacketTruncated;

	if (buf[5 + vsize] == kPGPPublicKeyAlgorithm_RSA ||
	    buf[5 + vsize] == kPGPPublicKeyAlgorithm_RSAEncryptOnly ||
		buf[5 + vsize] == kPGPPublicKeyAlgorithm_RSASignOnly) {
		/* Sanity checks on the exponent */
		elen = (unsigned)buf[8+vsize+mlen] << 8 | buf[9+vsize+mlen];
		if (elen && len > 10+vsize+mlen &&
				buf[10+vsize+mlen] >> ((elen-1)&7) != 1)
			return kPGPError_MalformedKeyExponent;
		elen = (elen + 7) / 8;

		if (len < 10+vsize+mlen+elen)
			return kPGPError_KeyPacketTruncated;
		if ((buf[8 + vsize + mlen-1] & 1) == 0)
			return kPGPError_RSAPublicModulusIsEven;
		if ((buf[10+vsize+mlen+elen-1] & 1) == 0)
			return kPGPError_RSAPublicExponentIsEven;
		if (len > 10+vsize+mlen+elen && !secretf)
			return kPGPError_KEY_LONG;
	}
	if (keyID) {
		if (buf[0] < PGPVERSION_4) {
			/* Old keyid rules, low 64 bits of modulus */
			memcpy(keyID, buf + 8 + vsize + mlen - 8, 8);
			if (mlen < 8)
				pgpClearMemory( keyID,  8 - mlen);
		} else {
			/*
			 * PGPVERSION_4 (aka V4) keys use newer keyID algorithm.
			 * Low bits of fingerprint20 hash of key, but we can't
			 * use that function directly as key is not assembled yet.
			 */
			PGPByte hash[20];
			if (secretf)
				len = ringKeyParsePublicPrefix(buf, len);
			err = pgpFingerprint20HashBuf(context, buf, len, hash);
			if (err < 0)
				return err;
			pgpAssert (err == sizeof(hash));
			memcpy(keyID, hash+sizeof(hash)-8, 8);
		}
	}

	return 0;

fakeid_short:
	err = kPGPError_KeyPacketTruncated;
fakeid:
	/*
	 * We couldn't read a KeyID.  Create a bogus one.
	 * The low 32 bits are 0 (which really stands out in a
	 * keyring listing and is impossible in the usual case),
	 * but the high 32 bits are a CRC of the packet, to reduce
	 * reports of keyID collisions.
	 */
	if (keyID) {
		PGPUInt32 fake = ringHashBuf(buf, len);
		keyID[0] = (PGPByte)(fake >> 24);
		keyID[1] = (PGPByte)(fake >> 16);
		keyID[2] = (PGPByte)(fake >> 8);
		keyID[3] = (PGPByte)fake;
	}
	return err;
}

/* Extract the modulus from a key and return it (buffer & length) */
PGPByte const *
ringKeyParseModulus(PGPByte const *buf, size_t len, unsigned *lenp)
{
	unsigned msize;
	unsigned vsize;

	*lenp = 0;
	if (buf[0] == PGPVERSION_4) {
		vsize = 0;
	} else {
		vsize = 2;
	}

	if (len < 8+vsize || !KNOWN_PGP_VERSION(buf[0]) || 
	    (buf[5+vsize] != kPGPPublicKeyAlgorithm_RSA &&
	     buf[5+vsize] != kPGPPublicKeyAlgorithm_RSAEncryptOnly &&
		 buf[5+vsize] != kPGPPublicKeyAlgorithm_RSASignOnly))
		return 0;

	/* Get bytes in modulus */
	msize = (unsigned)buf[6+vsize] << 8 | buf[7+vsize];
	msize = (msize + 7) / 8;

	*lenp = msize;
	return buf + 8 + vsize;
}

/* Extract the exponent from a key and return it (buffer & length) */
PGPByte const *
ringKeyParseExponent(PGPByte const *buf, size_t len, unsigned *lenp)
{
	unsigned size;
	unsigned vsize;

	*lenp = 0;

	if (buf[0] == PGPVERSION_4) {
		vsize = 0;
	} else {
		vsize = 2;
	}

	if (len < 10+vsize || !KNOWN_PGP_VERSION(buf[0]) || 
	    (buf[5+vsize] != kPGPPublicKeyAlgorithm_RSA &&
	     buf[5+vsize] != kPGPPublicKeyAlgorithm_RSAEncryptOnly &&
		 buf[5+vsize] != kPGPPublicKeyAlgorithm_RSASignOnly))
		return NULL;

	/* Get bytes in modulus */
	size = (unsigned)buf[6+vsize] << 8 | buf[7+vsize];
	size = (size + 7) / 8;

	if (len < 10 + vsize + size)
		return NULL;

	/* Skip modulus */
	size += 8 + vsize;
	buf += size;
	len -= size;

	/* Get bytes in exponent */
	size = (unsigned)buf[0] << 8 | buf[1];
	size = (size + 7) / 8;

	if (len < 2 + size)
		return NULL;

	*lenp = size;
	return buf + 2;
}

/*
 * Return a pointer to the "extra" part of the signature packet, the part
 * which gets hashed.  We use two different conventions, one for PGP 2.6
 * and earlier, and the other for the later version packets.  The latter
 * includes the whole packet up through the variable part.
 *
 * buf		Signature buffer pointer
 * len		buf length in bytes
 * lenp		Return pointer to length of extra buffer
 */
PGPByte const *
ringSigParseExtra(PGPByte const *buf, size_t len, unsigned *lenp)
{
	*lenp = 0;					/* In case of error */
	if (buf[0] == PGPVERSION_4) {
		unsigned extralen;
		if (len < 6)			/* Too short */
			return NULL;
		extralen = (unsigned)buf[4]<<8 | buf[5];
		if (len < 6 + extralen)
			return NULL;		/* Too short */
		*lenp = 6 + extralen;
		return buf;
	} else {
		/* Format from version 2.6 and earlier */
		if (len < 2 || !KNOWN_PGP_VERSION(buf[0]) || len < 2u + buf[1])
			return NULL;	/* Too short or bad version */
		*lenp = buf[1];
		return buf+2;
	}
}

/* Extract the RSA integer from a sig and return it (buffer & length) */
PGPByte const *
ringSigParseInteger(PGPByte const *buf, size_t len, unsigned *lenp)
{
	unsigned size;

	*lenp = 0;

	if (len < 2 || !KNOWN_PGP_VERSION(buf[0]))
		return NULL;	/* Too short or bad version */
	if (len < 16u + buf[1])
		return NULL;	/* Too short */
	/* Skip forward to PK algorithm */
	len -= 10 + buf[1];
	buf += 10 + buf[1];
	if (buf[0] != kPGPPublicKeyAlgorithm_RSA &&
	    buf[0] != kPGPPublicKeyAlgorithm_RSAEncryptOnly &&
	    	buf[0] != kPGPPublicKeyAlgorithm_RSASignOnly)
		return NULL;	/* Not RSA */

	/* Get bytes in modulus */
	size = (unsigned)buf[4] << 8 | buf[5];
	size = (size + 7) / 8;

	if (len < 6 + size)
		return NULL;

	*lenp = size;
	return buf + 6;
}

/* Forward reference */
static int
ringSigParse3(PGPByte const *buf, size_t len, PGPByte *pkalg, PGPByte keyID[8],
	PGPUInt32 *tstamp, PGPUInt16 *validity, PGPByte *type, PGPByte *hashalg,
	size_t *extralen, PGPByte *version, PGPBoolean *exportable,
	PGPBoolean *revocable, PGPByte *trustLevel, PGPByte *trustValue,
	PGPBoolean *hasRegExp, PGPBoolean *isX509);
static int
ringSigParseSubpackets(PGPByte const *buf, PGPByte keyID[8],
	PGPUInt32 *tstamp, PGPUInt16 *validity, PGPBoolean *exportable,
	PGPBoolean *revocable, PGPByte *trustLevel, PGPByte *trustValue,
	PGPBoolean *hasRegExp, PGPBoolean *isX509);


/*
 * Read a signature from the file, positioned just after the header.
 * "len" is the length of the signature packet.  Leaves the file
 * pointed after the length.
 *
 * A signature is:
 *  0 Version: 1 byte
 *  1 Length of following material: 1 byte (must be 5!)
 *  2 Signature type: 1 byte
 *  3 Timestamp: 4 bytes
 *  7 KeyID: 8 bytes
 * 15 Public-key algorithm: 1 byte
 * 16 Hash algorithm: 1 byte
 * 17 First 2 bytes of hash: 2 bytes
 * 19 MPI of signature (header)
 * 21 MPI of signature (data)
 *
 * This reads up to the keyID (15 bytes) into a buffer,
 * then sets up the various fields.
 */
int
ringSigParse(PGPByte const *buf, size_t len, PGPByte *pkalg, PGPByte keyID[8],
	PGPUInt32 *tstamp, PGPUInt16 *validity, PGPByte *type, PGPByte *hashalg,
	size_t *extralen, PGPByte *version, PGPBoolean *exportable,
	PGPBoolean *revocable, PGPByte *trustLevel, PGPByte *trustValue,
	PGPBoolean *hasRegExp, PGPBoolean *isX509)
{
	unsigned l;	/* extralen */
	unsigned t;
	PGPByte alg;

	/* Default settings in case of error */
	if (pkalg)
		*pkalg = 0;
	if (keyID)
		pgpClearMemory( keyID,  8);
	if (tstamp)
		*tstamp = 0;
	if (validity)
		*validity = 0;
	if (type)
		*type = 255;
	if (hashalg)
		*hashalg = 255;
	if (extralen)
		*extralen = 0;
	if (version)
		*version = 0;
	if (exportable)
		*exportable = TRUE;
	if (revocable)
		*revocable = TRUE;
	if (trustLevel)
		*trustLevel = 0;
	if (trustValue)
		*trustValue = 0;
	if (hasRegExp)
		*hasRegExp = FALSE;
	if (isX509)
		*isX509 = FALSE;

	if (len < 1)
		return kPGPError_TruncatedSignature;
	/* Forward compatibility */
	if (buf[0] == PGPVERSION_4) {
		return ringSigParse3(buf, len, pkalg, keyID, tstamp, validity, type,
							 hashalg, extralen, version, exportable,
							 revocable, trustLevel, trustValue, hasRegExp,
							 isX509);
	}
	if (version)
		*version = buf[0];
	if (!KNOWN_PGP_VERSION(buf[0]))
		return kPGPError_UnknownSignatureVersion;
	if (len < 2)
		return kPGPError_TruncatedSignature;
	l = buf[1];
	if (extralen)
		*extralen = l;
	if (l >= 1) {
		if (len < 3)
			return kPGPError_TruncatedSignature;
		if (type)
			*type = buf[2];
		if (l >= 5 && tstamp) {
			if (len < 7)
				return kPGPError_TruncatedSignature;
			*tstamp = (PGPUInt32)((unsigned)buf[3]<<8|buf[4]) << 16 |
			                  ((unsigned)buf[5]<<8|buf[6]);
		}
		if (l >= 7 && tstamp) {
			if (len < 9)
				return kPGPError_TruncatedSignature;
			*tstamp = (PGPUInt16)buf[7]<<8 | buf[8];
		}
	}
	if (!extralen && l != 5)
		return kPGPError_ExtraSignatureMaterial;
	if (len < l+10)
		return kPGPError_TruncatedSignature;
	if (keyID)
		memcpy(keyID, buf + 7, 8);
	if (len < l+11)
		return kPGPError_TruncatedSignature;
	alg = buf[l+10];
	if (pkalg)
		*pkalg = alg;
#if 0
	if (buf[l+10] != kPGPPublicKeyAlgorithm_RSA &&
	    buf[l+10] != kPGPPublicKeyAlgorithm_RSAEncryptOnly &&
	    	buf[l+10] != kPGPPublicKeyAlgorithm_RSASignOnly)
		return kPGPError_UnknownSignatureAlgorithm;
#endif
	if (len < l+12)
		return kPGPError_TruncatedSignature;
	if (hashalg)
		*hashalg = buf[l+11];
	if (len < l+16)
		return kPGPError_TruncatedSignature;
	t = (unsigned)buf[l+14] << 8 | buf[l+15];
	if (t && len > l+16 && buf[l+16] >> ((t-1)&7) != 1)
		return kPGPError_MalformedSignatureInteger;
	l += 16 + (t+7)/8;
	if (alg == kPGPPublicKeyAlgorithm_RSA ||
	    alg == kPGPPublicKeyAlgorithm_RSAEncryptOnly ||
	    alg == kPGPPublicKeyAlgorithm_RSASignOnly) {
		if (len != l)
			return len < l ? kPGPError_TruncatedSignature : kPGPError_SIG_LONG;
	}
	return 0;
}

/*
 * Version of ringSigParse for the signature packet formats being designed
 * for a future version.  We do not fully handle them here, but we do a
 * basic parse on the sigs and we make sure there are no critical fields
 * that we can't handle.
 *
 * Sig layout:
 *
 * Offset,Length  Meaning
 *	0		1	Version byte (=4)
 *	1		1	Signature type (included in hash) (nested flag)
 *	2		1	PK algorithm (1 = RSA) (included in hash)
 *	3		1	Hash algorithm (1 = MD5) (included in hash)
 *	4		2	Length of extra material included in hash (=y)
 *	6		y	Subpackets (hashed)
 *  6+y		2	Length of extra material not included in hash (=z)
 *  8+y		z	Subpackets (unhashed)
 *=====
 *remainder not present on sig headers
 *=====
 *	8+y+z	2	First 2 bytes of message digest (16-bit checksum)
 *	10+y+z	2+w	MPI of PK-signed integer
 *	12+y+z+w
 *
 */
static int
ringSigParse3(PGPByte const *buf, size_t len, PGPByte *pkalg, PGPByte keyID[8],
	PGPUInt32 *tstamp, PGPUInt16 *validity, PGPByte *type, PGPByte *hashalg,
	size_t *extralen, PGPByte *version, PGPBoolean *exportable,
	PGPBoolean *revocable, PGPByte *trustLevel, PGPByte *trustValue,
	PGPBoolean *hasRegExp, PGPBoolean *isX509)
{
	unsigned	l;				/* extralen */
	unsigned	t;				/* bit count of first MPI */
	size_t		mpiend;			/* byte position past first MPI */
	PGPByte		alg;			/* pk alg for sig */

	if (version)
		*version = buf[0];
	if (len < 8)
		return kPGPError_TruncatedSignature;
	if (type)
		*type = buf[1];
	alg = buf[2];
	if (pkalg)
		*pkalg = alg;
	if (hashalg)
		*hashalg = buf[3];
	l = (unsigned)buf[4]<<8 | buf[5];		/* extra material */
	if (extralen)
		*extralen = l;
	if (len < 6+l+2)
		return kPGPError_TruncatedSignature;
	l += (unsigned)buf[l+6]<<8 | buf[l+7];	/* unhashed extra material */
	if (len < 8+l+2+2)
		return kPGPError_TruncatedSignature;
	t = (unsigned)buf[l+10] << 8 | buf[l+11]; /* first MPI size in bits */
	mpiend = 8+l+2+2+(t+7)/8;
	if (len < mpiend)
		return kPGPError_TruncatedSignature;
	if (t && buf[l+12] >> ((t-1)&7) != 1)
		return kPGPError_MalformedSignatureInteger;
	/* Special checking for RSA sigs */
	if (alg == kPGPPublicKeyAlgorithm_RSA ||
	    alg == kPGPPublicKeyAlgorithm_RSAEncryptOnly ||
		alg == kPGPPublicKeyAlgorithm_RSASignOnly) {
		if (len > mpiend)
			return kPGPError_SIG_LONG;
	}
			
	/* Now parse subpackets, pick up keyid, timestamp and validity info */
	return ringSigParseSubpackets (buf+4, keyID, tstamp, validity, exportable,
		revocable, trustLevel, trustValue, hasRegExp, isX509);
}


/*
 * Parse the "extra data" on the new-style signature packets.
 *
 * This is a chain of subpackets, with the following format:
 * Offset,Length  Meaning
 *	0	x(1-2)	Subpacket length (=y)
 *	x	1	Subpacket type (with critical flag)
 *	x+1	y-1	Subpacket data
 *
 * We are only parsing a few of the subpackets now, returning an error
 * if we see any critical ones that we don't handle.  Also, we require
 * there to be a keyid subpacket at present or it is an error.
 *
 * There are two such chains, each preceded by a two-byte count.  The
 * first is hashed and the second is not.
 */
static int
ringSigParseSubpackets(PGPByte const *buf, PGPByte keyID[8],
	PGPUInt32 *tstamp, PGPUInt16 *validity, PGPBoolean *exportable,
	PGPBoolean *revocable, PGPByte *trustLevel, PGPByte *trustValue,
	PGPBoolean *hasRegExp, PGPBoolean *isX509)
{
	unsigned	len;			    /* Length of chain of subpackets */
	unsigned	sublen;				/* Length of subpacket */
	unsigned	offset;				/* Offset to next subpacket */
	int 		type;				/* Type of subpacket */
	int			critical;			/* Critical flag on subpacket */
	int			found_X509 = 0;		/* True if found an X509 subpacket */
	int			found_keyid = 0;	/* True if found a keyid subpacket */
	int			hashed;				/* True if parsing hashed subpackets */
	int			hashedExportable=0; /* True if exportable in hashed portion */
	PGPUInt32		creationtime = 0;	/* Creation time from subpacket */
	PGPUInt32		expirationtime = 0;	/* Expiration time from subpacket */

	hashed = 1;
	/* Loop twice, once with hashed = 1 and once with hashed = 0 */
	do {
		len = (unsigned)buf[0]<<8 | buf[1];
		buf += 2;
		while (len) {
			if (len < 2)
				return kPGPError_ExtraDateOnSignature;
			/* Subpacket length may be one or two bytes */
			sublen = (unsigned)buf[0];
			if (sublen < 0xc0) {
				/* sublen is the length */
				offset = sublen;
				len -= 1;
				buf += 1;
			} else if ((sublen & 0xe0) == 0xc0) {
				sublen &= 0x3f;
				sublen = (sublen << 8) + (unsigned)buf[1] + 192;
				offset = sublen;
				len -= 2;
				buf += 2;
			} else if (sublen == 0xff) {
				if (len < 5)
					return kPGPError_ExtraDateOnSignature;
				sublen = (((((buf[1]<<8)|buf[2])<<8)|buf[3])<<8)|buf[4];
				offset = sublen;
				len -= 5;
				buf += 5;
			} else {
				/*
				 * Variable length subpacket; add up sizes of parts.
				 * We don't consolidate them, so these can't be used for
				 * the types we are scanning for below.
				 */
				unsigned partlen;
				PGPByte const *bp;
				PGPBoolean final = FALSE;
				len -= 1;
				buf += 1;
				sublen = 1 << (sublen & 0x1f);
				partlen = sublen;
				bp = buf + partlen;
				while (!final) {
					partlen = (unsigned)*bp++;
					if (partlen < 0xe0) {
						final = TRUE;
						if (partlen >= 0xc0)
							partlen = ((partlen&0x3f) << 8)
												+ (unsigned)*bp++ + 192;
					} else {
						partlen = 1 << (partlen & 0x1f);
					}
					sublen += partlen;
					bp += partlen;
				}
				offset = bp - buf;
			}
			if (len < 1)
				return kPGPError_ExtraDateOnSignature;
			type = buf[0];
			critical = type & SIGSUBF_CRITICAL;
			type &= ~SIGSUBF_CRITICAL;
			buf += 1;
			len -= 1;
			sublen -= 1;
			offset -= 1;
			if (len < sublen)
				return kPGPError_ExtraDateOnSignature;
			/* Illegal to have critical packets in unhashed region */
			if (critical && !hashed)
				return kPGPError_ExtraDateOnSignature;
			switch (type) {
			case SIGSUB_CREATION:
				if (sublen != 4)
					return kPGPError_ExtraDateOnSignature;
				if (!tstamp && critical)
					return kPGPError_ExtraDateOnSignature;
				creationtime = (PGPUInt32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									   ((unsigned)buf[2]<<8|buf[3]);
				if (tstamp)
					*tstamp = creationtime;
				break;
			case SIGSUB_EXPIRATION:
				if (sublen != 4 || !hashed)
					return kPGPError_ExtraDateOnSignature;
				if (!validity && critical)
					return kPGPError_ExtraDateOnSignature;
				expirationtime = (PGPUInt32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									   ((unsigned)buf[2]<<8|buf[3]);
				/* Store it as days.  expirationtime is really valid period */
				if (validity && creationtime)
					*validity = (PGPUInt16) (expirationtime / (24 * 3600));
				break;
			case SIGSUB_KEYID:
				/* Allow longer than 8 byte keyid's unless critical */
				if (sublen < 8)
					return kPGPError_ExtraDateOnSignature;
				if (critical && (!keyID || sublen != 8))
					return kPGPError_ExtraDateOnSignature;
				if (keyID)
					memcpy(keyID, buf + sublen - 8, 8);
				found_keyid = 1;
				break;
			case SIGSUB_EXPORTABLE:
				/* Allow exportability attribute not to be hashed */
				if (sublen != 1)
					return kPGPError_ExtraDateOnSignature;
				/* hashed version overrides unhashed */
				if (hashedExportable && !hashed)
					return kPGPError_ExtraDateOnSignature;
				if (IsntNull( exportable ) ) {
					*exportable = *buf != 0;
					hashedExportable = hashed;
				}
				break;
			case SIGSUB_REVOCABLE:
				if (sublen != 1 || !hashed)
					return kPGPError_ExtraDateOnSignature;
				if (IsntNull( revocable ) )
					*revocable = *buf != 0;
				break;
			case SIGSUB_TRUST:
				/* Packet format not fully decided, use 1st two bytes */
				if (sublen < 2 || !hashed)
					return kPGPError_ExtraDateOnSignature;
				if (IsntNull( trustLevel ) )
					*trustLevel = buf[0];
				if (IsntNull( trustValue ) )
					*trustValue = buf[1];
				break;
			case SIGSUB_REGEXP:
				if (!hashed)
					return kPGPError_ExtraDateOnSignature;
				if (IsntNull( hasRegExp ) )
					*hasRegExp = TRUE;
				break;
			case SIGSUB_X509:
				if (!hashed)
					return kPGPError_ExtraDateOnSignature;
				if (IsntNull( isX509 ) )
					*isX509 = TRUE;
				found_X509 = TRUE;
				break;
			default:
				if (critical)
					return kPGPError_ExtraDateOnSignature;
			}
			buf += offset;
			len -= offset;
		}
		hashed = !hashed;
	} while (!hashed);			/* Two iterations */
	/* We must have a keyid, unless it is an X509 key */
	if (keyID && !found_keyid && !found_X509)
		return kPGPError_ExtraDateOnSignature;
	return kPGPError_NoErr;
}

/* 
 * Look for a particular subpacket associated with the given signature.
 * See comments above for format of subpackets.  Return pointer to the
 * subpacket data, after the length and type field.
 * Critical and hashed tell whether the packet was critical and/or in the
 * hashed region, respectively (return parameters).
 * We don't need to do consistency checking, it's already been checked.
 *
 * This may alter the contents of buf, so should only be called once on
 * any given buffer.
 *
 * nth should be set to 0 to find first match, 1 for second, etc.
 * Returns number of matches total in *pmatches.
 */
PGPByte const *
ringSigFindSubpacket(PGPByte *buf, int subpacktype, unsigned nth,
	PGPSize *plen, int *pcritical, int *phashed, PGPUInt32 *pcreation,
	unsigned *pmatches)
{
	unsigned	len;			    /* Length of chain of subpackets */
	unsigned	sublen;				/* Length of subpacket */
	unsigned	offset;				/* Offset to next subpacket */
	int 		type;				/* Type of subpacket */
	int			critical;			/* Critical flag on subpacket */
	int			hashed;				/* True if parsing hashed subpackets */
	unsigned	nmatches = 0;		/* Number of matches so far */
	PGPByte const *pbuf = NULL;		/* Return buffer pointer */

	if (buf[0] < PGPVERSION_4)
		return NULL;

	subpacktype &= ~SIGSUBF_CRITICAL;
	buf += 4;
	hashed = 1;
	/* Loop twice, once with hashed = 1 and once with hashed = 0 */
	do {
		len = (unsigned)buf[0]<<8 | buf[1];
		buf += 2;
		while (len) {
			if (len < 2)
				return NULL;
			/* Subpacket length may be one or two bytes */
			sublen = (unsigned)buf[0];
			if (sublen < 0xc0) {
				/* sublen is the length */
				offset = sublen;
				len -= 1;
				buf += 1;
			} else if ((sublen & 0xe0) == 0xc0) {
				sublen &= 0x3f;
				sublen = (sublen << 8) + (unsigned)buf[1] + 192;
				offset = sublen;
				len -= 2;
				buf += 2;
			} else if (sublen == 0xff) {
				if (len < 5)
					return NULL;
				sublen = (((((buf[1]<<8)|buf[2])<<8)|buf[3])<<8)|buf[4];
				offset = sublen;
				len -= 5;
				buf += 5;
			} else {
				/* Variable length subpacket; concatenate data */
				unsigned partlen;
				PGPByte *bp;
				PGPBoolean final = FALSE;
				len -= 1;
				buf += 1;
				sublen = 1 << (sublen & 0x1f);
				partlen = sublen;
				bp = buf + partlen;
				while (!final) {
					partlen = (unsigned)*bp++;
					if (partlen < 0xe0) {
						final = TRUE;
						if (partlen >= 0xc0)
							partlen = ((partlen&0x3f) << 8)
												+ (unsigned)*bp++ + 192;
					} else {
						partlen = 1 << (partlen & 0x1f);
					}
					pgpCopyMemory (bp, buf+sublen, partlen);
					sublen += partlen;
					bp += partlen;
				}
				offset = bp - buf;
			}
			if (len < 1)
				return NULL;
			type = buf[0];
			critical = type & SIGSUBF_CRITICAL;
			type &= ~SIGSUBF_CRITICAL;
			buf += 1;
			len -= 1;
			sublen -= 1;
			offset -= 1;

			if (type == subpacktype) {
				/* Match! */
				if (nmatches == nth) {
					if (pcritical)
						*pcritical = critical;
					if (phashed)
						*phashed = hashed;
					if (plen)
						*plen = sublen;
					pbuf = buf;
				}
				nmatches += 1;
			}

			if (type == SIGSUB_CREATION) {
				pgpAssert (sublen == 4);
				if (pcreation) {
					*pcreation = (PGPUInt32)((unsigned)buf[0]<<8|buf[1]) << 16 |
						((unsigned)buf[2]<<8|buf[3]);
				}
			}

			/* Skip to next packet */
			buf += offset;
			len -= offset;
		}
		hashed = !hashed;
	} while (!hashed);			/* Two iterations */
	if (pmatches)
		*pmatches = nmatches;
	return pbuf;				/* Will be non-null if we matched nth */
}


#define SFLAGS(critical,hashed) \
	(((critical)?kPGPSigFlags_Critical:0)|((hashed)?0:kPGPSigFlags_Unhashed))

/*
 * Add info from subpackets to a sigspec structure.
 */
PGPError
ringSigSubpacketsSpec(PGPSigSpec *spec, PGPByte const *buf)
{
	unsigned	len;			    /* Length of chain of subpackets */
	unsigned	sublen;				/* Length of subpacket */
	unsigned	offset;				/* Offset to next subpacket */
	int 		type;				/* Type of subpacket */
	int			critical;			/* Critical flag on subpacket */
	int			hashed;				/* True if parsing hashed subpackets */
	PGPUInt32	dat;

	pgpAssert (buf[0] >= PGPVERSION_4);
	buf += 4;

	hashed = 1;
	/* Loop twice, once with hashed = 1 and once with hashed = 0 */
	do {
		len = (unsigned)buf[0]<<8 | buf[1];
		buf += 2;
		while (len) {
			if (len < 2)
				return kPGPError_ExtraDateOnSignature;
			/* Subpacket length may be one or two bytes */
			sublen = (unsigned)buf[0];
			if (sublen < 0xc0) {
				/* sublen is the length */
				offset = sublen;
				len -= 1;
				buf += 1;
			} else if ((sublen & 0xe0) == 0xc0) {
				sublen &= 0x3f;
				sublen = (sublen << 8) + (unsigned)buf[1] + 192;
				offset = sublen;
				len -= 2;
				buf += 2;
			} else if (sublen == 0xff) {
				if (len < 5)
					return kPGPError_ExtraDateOnSignature;
				sublen = (((((buf[1]<<8)|buf[2])<<8)|buf[3])<<8)|buf[4];
				offset = sublen;
				len -= 5;
				buf += 5;
			} else {
				/*
				 * Variable length subpacket; add up sizes of parts.
				 * We don't consolidate them, so these can't be used for
				 * the types we are scanning for below.
				 */
				unsigned partlen;
				PGPByte const *bp;
				PGPBoolean final = FALSE;
				len -= 1;
				buf += 1;
				sublen = 1 << (sublen & 0x1f);
				partlen = sublen;
				bp = buf + partlen;
				while (!final) {
					partlen = (unsigned)*bp++;
					if (partlen < 0xe0) {
						final = TRUE;
						if (partlen >= 0xc0)
							partlen = ((partlen&0x3f) << 8)
												+ (unsigned)*bp++ + 192;
					} else {
						partlen = 1 << (partlen & 0x1f);
					}
					sublen += partlen;
					bp += partlen;
				}
				offset = bp - buf;
			}
			if (len < 1)
				return kPGPError_ExtraDateOnSignature;
			type = buf[0];
			critical = type & SIGSUBF_CRITICAL;
			type &= ~SIGSUBF_CRITICAL;
			buf += 1;
			len -= 1;
			sublen -= 1;
			offset -= 1;
			if (len < sublen)
				return kPGPError_ExtraDateOnSignature;
			/* Illegal to have critical packets in unhashed region */
			if (critical && !hashed)
				return kPGPError_ExtraDateOnSignature;
			switch (type) {
			case SIGSUB_CREATION:
				if (sublen != 4)
					return kPGPError_ExtraDateOnSignature;
				dat = (PGPUInt32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									   ((unsigned)buf[2]<<8|buf[3]);
				pgpSigSpecSetTimestamp (spec, dat);
				break;
			case SIGSUB_EXPIRATION:
				if (sublen != 4)
					return kPGPError_ExtraDateOnSignature;
				dat = (PGPUInt32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									   ((unsigned)buf[2]<<8|buf[3]);
				pgpSigSpecSetSigExpiration (spec, SFLAGS(critical, hashed),
											dat);
				break;
			case SIGSUB_KEY_EXPIRATION:
				if (sublen != 4)
					return kPGPError_ExtraDateOnSignature;
				dat = (PGPUInt32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									   ((unsigned)buf[2]<<8|buf[3]);
				pgpSigSpecSetKeyExpiration (spec, SFLAGS(critical, hashed),
											dat);
				break;
			case SIGSUB_KEYID:
				/* This was processed beforehand to set up the sigspec */
				if (sublen != 8)
					return kPGPError_ExtraDateOnSignature;
				break;
			case SIGSUB_EXPORTABLE:
				/* Allow exportability attribute not to be hashed */
				if (sublen != 1)
					return kPGPError_ExtraDateOnSignature;
				pgpSigSpecSetExportable (spec, SFLAGS(critical, hashed),
											(PGPBoolean)(buf[0]!=0));
				break;
			case SIGSUB_REVOCABLE:
				if (sublen != 1)
					return kPGPError_ExtraDateOnSignature;
				pgpSigSpecSetRevocable (spec, SFLAGS(critical, hashed),
											(PGPBoolean)(buf[0]!=0));
				break;
			case SIGSUB_TRUST:
				/* Packet format not fully decided, use 1st two bytes */
				if (sublen != 2)
					return kPGPError_ExtraDateOnSignature;
				pgpSigSpecSetTrustLevel (spec, SFLAGS(critical, hashed),
										 (PGPBoolean)(buf[0]!=0),
										 (PGPBoolean)(buf[1]!=0));
				break;
			case SIGSUB_REGEXP:
				pgpSigSpecSetRegExp (spec, SFLAGS(critical, hashed),
									 (char *)buf);
				break;
			case SIGSUB_PREFERRED_ENCRYPTION_ALGS:
				pgpSigSpecSetPrefAlgs (spec, SFLAGS(critical, hashed),
									 buf, sublen);
				break;
			case SIGSUB_KEY_ADDITIONAL_RECIPIENT_REQUEST:
				pgpSigSpecSetAdditionalRecipientRequest (spec,
							SFLAGS(critical, hashed), buf, sublen);
				break;
			case SIGSUB_KEY_REVOCATION_KEY:
				pgpSigSpecSetRevocationKey (spec,
							SFLAGS(critical, hashed), buf, sublen);
				break;
			default:
				pgpSigSpecSetPacket (spec, SFLAGS(critical, hashed),
									 buf-1, sublen+1);
				break;
			}
			buf += offset;
			len -= offset;
		}
		hashed = !hashed;
	} while (!hashed);			/* Two iterations */
	return kPGPError_NoErr;
}




/*
 * Return data for nth subpacket in an attribute object.
 * Data is in buf.
 * Format is as for signature subpacket portion:
 * A chain of subpackets, with the following format:
 * Offset,Length  Meaning
 *	0	x(1-2)	Subpacket length (=y)
 *	x	1	Subpacket type (with critical flag)
 *	x+1	y-1	Subpacket data
 *
 * This may alter the contents of buf, so should only be called once on
 * any given buffer.
 *
 * nth should be set to 0 to find first subpacket, 1 for second, etc.
 * Returns type of subpacket and its size.
 */
PGPByte const *
ringAttrSubpacket(PGPByte *buf, PGPSize len, PGPUInt32 nth,
	PGPUInt32 *subpacktype, PGPSize *plen)
{
	PGPSize		sublen;				/* Length of subpacket */
	PGPSize		offset;				/* Offset to next subpacket */
	PGPUInt32	type;				/* Type of subpacket */
	PGPUInt32	npackets = 0;		/* Number of matches so far */

	while (len) {
		if (len < 2)
			return NULL;
		/* Subpacket length may be one or two bytes, or variable-length */
		sublen = (PGPSize)buf[0];
		if (sublen < 0xc0) {
			/* sublen is the length */
			offset = sublen;
			len -= 1;
			buf += 1;
		} else if ((sublen & 0xe0) == 0xc0) {
			sublen &= 0x3f;
			sublen = (sublen << 8) + (PGPSize)buf[1] + 192;
			offset = sublen;
			len -= 2;
			buf += 2;
		} else if (sublen == 0xff) {
			sublen = (((((buf[1]<<8)|buf[2])<<8)|buf[3])<<8)|buf[4];
			offset = sublen;
			len -= 5;
			buf += 5;
		} else {
			/* Variable length subpacket; concatenate data */
			PGPSize partlen;
			PGPByte *bp;
			PGPBoolean final = FALSE;
			len -= 1;
			buf += 1;
			sublen = 1 << (sublen & 0x1f);
			partlen = sublen;
			bp = buf + partlen;
			while (!final) {
				partlen = (PGPSize)*bp++;
				if (partlen < 0xe0) {
					final = TRUE;
					if (partlen >= 0xc0)
						partlen = ((partlen&0x3f) << 8) + (PGPSize)*bp++ + 192;
				} else {
					partlen = 1 << (partlen & 0x1f);
				}
				pgpCopyMemory (bp, buf+sublen, partlen);
				sublen += partlen;
				bp += partlen;
			}
			offset = bp - buf;
		}
		if (len < 1)
			return NULL;
		type = buf[0];
		buf += 1;
		len -= 1;
		sublen -= 1;
		offset -= 1;

		if (npackets++ == nth) {
			if (plen)
				*plen = sublen;
			if (subpacktype)
				*subpacktype = type;
			return buf;
		}
	}

	/* Get here if fewer than n+1 subpackets */
	return NULL;
}

/* Return the number of subpackets in the buffer */
PGPUInt32
ringAttrCountSubpackets(PGPByte const *buf, PGPSize len)
{
	PGPSize		sublen;				/* Length of subpacket */
	PGPSize		offset;				/* Offset to next subpacket */
	PGPUInt32	type;				/* Type of subpacket */
	PGPUInt32	npackets = 0;		/* Number of matches so far */

	while (len) {
		if (len < 2)
			return 0;
		/* Subpacket length may be 1/2/4 bytes, or variable-length */
		sublen = (PGPSize)buf[0];
		if (sublen < 0xc0) {
			/* sublen is the length */
			offset = sublen;
			len -= 1;
			buf += 1;
		} else if ((sublen & 0xe0) == 0xc0) {
			sublen &= 0x3f;
			sublen = (sublen << 8) + (PGPSize)buf[1] + 192;
			offset = sublen;
			len -= 2;
			buf += 2;
		} else if (sublen == 0xff) {
			sublen = (((((buf[1]<<8)|buf[2])<<8)|buf[3])<<8)|buf[4];
			offset = sublen;
			len -= 5;
			buf += 5;
		} else {
			/* Variable length subpacket */
			PGPSize partlen;
			PGPByte const *bp;
			PGPBoolean final = FALSE;
			len -= 1;
			buf += 1;
			sublen = 1 << (sublen & 0x1f);
			partlen = sublen;
			bp = buf + partlen;
			while (!final) {
				partlen = (PGPSize)*bp++;
				if (partlen < 0xe0) {
					final = TRUE;
					if (partlen >= 0xc0)
						partlen = ((partlen&0x3f) << 8) + (PGPSize)*bp++ + 192;
				} else {
					partlen = 1 << (partlen & 0x1f);
				}
				sublen += partlen;
				bp += partlen;
			}
			offset = bp - buf;
		}
		if (len < 1)
			return 0;
		type = buf[0];
		buf += 1;
		len -= 1;
		sublen -= 1;
		offset -= 1;

		++npackets;
	}

	return npackets;
}


/*
 * Return the length of the prefix of a secret key which is a public key,
 * or 0 if it can't be determined.  A key's prefix is:
 * 0 1 - Version
 * 1 4 - Creation time
 * 5 2 - Vaidity period (days)
 * 7 1 - Algorithm
 * 8 ? - Algorithm-specific parameters
 */
size_t
ringKeyParsePublicPrefix(PGPByte const *buf, size_t len)
{
	size_t size;
	unsigned vsize;

	if (buf[0] == PGPVERSION_4) {
		vsize = 0;
	} else {
		vsize = 2;
	}

	/* Check version bytes */
	if (!KNOWN_PGP_VERSION(buf[0]) || len < 6+vsize)
		return 0;
	size = pgpPubKeyPrefixSize(buf[5+vsize], buf+6+vsize, len-6-vsize);
	if (size)
		size += 6+vsize;
	return size;
}

int
ringKeyParseFingerprint16(PGPContextRef context, PGPByte const *kbuf,
						  size_t klen, PGPByte *fingerprint)
{
	PGPHashVTBL const *h;
	PGPHashContext *hc;
	PGPByte const *buf;
	unsigned len;
    PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( context );

	h = pgpHashByNumber (kPGPHashAlgorithm_MD5);
	if (!h)
		return kPGPError_BadHashNumber;
	hc = pgpHashCreate( memoryMgr, h);
	if (!hc)
		return kPGPError_OutOfMemory;

        buf = ringKeyParseModulus(kbuf, klen, &len);
        if (!buf)
		goto error;
        PGPContinueHash(hc, buf, len);

        buf = ringKeyParseExponent(kbuf, klen, &len);
        if (!buf)
		goto error;
        PGPContinueHash(hc, buf, len);

	buf = (PGPByte *) pgpHashFinal(hc);
	memcpy(fingerprint, buf, h->hashsize);
	PGPFreeHashContext(hc);

	return h->hashsize;	/* Success */
error:
	PGPFreeHashContext(hc);
	pgpClearMemory( fingerprint,  16);
	return ringKeyParse(context, kbuf, klen, NULL, NULL, NULL, NULL, NULL,
						NULL, 0);
	/*                              Pkalg KeyID KeybitTstampValid */
}


/* Return the size which will be used to create the key prefix buffer */
size_t
ringKeyBufferLength(PGPKeySpec const *ks, PGPByte pkalg)
{
	(void)pkalg;
	if (pgpKeySpecVersion(ks)==PGPVERSION_4)
		return 6;	/* version(1)+timestamp(4)+pkalg(1) */
	else
		return 8;	/* version(1)+timestamp(4)+validity(2)+pkalg(1) */
}

/* Create the key prefix buffer */
int
ringKeyToBuffer(PGPByte *buf, PGPKeySpec const *ks, PGPByte pkalg)
{
	PGPUInt32 tstamp;
	PGPUInt16 validity;
	unsigned vsize;


	buf[0] = pgpKeySpecVersion(ks);
	if (buf[0] == PGPVERSION_4) {
		vsize = 0;
	} else {
		vsize = 2;
	}
	tstamp = pgpKeySpecCreation(ks);
	buf[1] = (PGPByte)(tstamp>>24);
	buf[2] = (PGPByte)(tstamp>>16);
	buf[3] = (PGPByte)(tstamp>> 8);
	buf[4] = (PGPByte)(tstamp    );
	if (vsize) {
		validity = pgpKeySpecValidity(ks);
		buf[5] = (PGPByte)(validity>>8);
		buf[6] = (PGPByte)(validity   );
	}
	buf[5+vsize] = pkalg;
	return 0;
}

/* Return the size of a prefix buffer for a secret key */
size_t
ringSecBufferLength(PGPKeySpec const *ks, PGPByte pkalg)
{
	return ringKeyBufferLength(ks, pkalg);
}

/* Create the prefix buffer for a secret key */
int
ringSecToBuffer(PGPByte *buf, PGPKeySpec const *ks, PGPByte pkalg)
{
	return ringKeyToBuffer(buf, ks, pkalg);
}

