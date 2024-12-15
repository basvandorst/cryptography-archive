/*
* pgpRngPars.c - parse structures from a keyring. Validates its
* inpout very carefully, for use by ringopen.c.
* Also includes routines to "unparse" and assemble
* keyringpackets.
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by Colin Plumb.
*
* $Id: pgpRngPars.c,v 1.6.2.4 1997/06/07 09:50:37 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "pgpDebug.h"
#include "pgpRngPars.h"
#include "pgpRngPriv.h"	/* For ringHashBuf, pgpFingerprint20HashBuf */
#include "pgpHash.h"
#include "pgpUsuals.h"
#include "pgpKeySpec.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpPktByte.h"

#ifndef NULL
#define NULL 0
#endif


/*
* Read a key from the buffer and extract vital statistics.
* If the KeyID cannot be determined, a fake keyID with a 32-bit CRC
* stuffed in the middle is created. This is designed to assist
* matching of bad keys.
*
* A key is:
* 0 Version: 1 byte
* 1 Timestamp: 4 bytes
* 5 Validity: 2 bytes (skipped if version PGPVERSION_3)
* 7 Algorithm: 1 byte
* [Algorithm-specific portion]
* 8 Some MP number from which we can extract key ID: modulus, etc.
* [Other algorithm-specific fields]
* [with possible extra data if it's a secret key]
*
* If "secretf" is set, additional data is allowed at the end of the
* packet. If it is clear, additional data results in PGPERR_KEY_LONG.
* (The format of the extra data is not checked.)
*/
int
ringKeyParse(byte const *buf, size_t len, byte *pkalg, byte keyID[8],
	word16 *keybits, word32 *tstamp, word16 *validity, int secretf)
{
	unsigned mlen, elen;
	int err;
	unsigned vsize;

	/* Defaults in case of error */
	if (keyID)
		memset(keyID, 0, 8);
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
		err = PGPERR_KEY_VERSION;
		goto fakeid;
	}
	/* PGPVERSION_3 keys have no validity field */
	if (buf[0] == PGPVERSION_3) {
		vsize = 0;
	} else {
		vsize = 2;
	}
	if (len < 5)
		goto fakeid_short;

	if (tstamp)
		*tstamp = (word32)((unsigned)buf[1] << 8 | buf[2]) << 16 |
			((unsigned)buf[3] << 8 | buf[4]);
	if (len < 5 + vsize)
		goto fakeid_short;
	if (validity && vsize)
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
		*keybits = (word16)mlen;

	if (mlen && len > 8 + vsize && buf[8 + vsize] >> ((mlen-1)&7) != 1) {
		err = PGPERR_KEY_MODMPI;
		goto fakeid;
	}
	mlen = (mlen + 7) / 8;
	if (len < 8 + vsize + mlen)
		goto fakeid_short;

	if (len < 10 + vsize + mlen)
		return PGPERR_KEY_SHORT;

	if (buf[5 + vsize] == PGP_PKALG_RSA ||
	buf[5 + vsize] == PGP_PKALG_RSA_ENC ||
		buf[5 + vsize] == PGP_PKALG_RSA_SIG) {
		/* Read keyID */
		if (keyID) {
			memcpy(keyID, buf + 8 + vsize + mlen - 8, 8);
			if (mlen < 8)
				memset(keyID, 0, 8 - mlen);
		}
		/* Sanity checks on the exponent */
		elen = (unsigned)buf[8+vsize+mlen] << 8 | buf[9+vsize+mlen];
		if (elen && len > 10+vsize+mlen &&
				buf[10+vsize+mlen] >> ((elen-1)&7) != 1)
			return PGPERR_KEY_EXPMPI;
		elen = (elen + 7) / 8;

		if (len < 10+vsize+mlen+elen)
			return PGPERR_KEY_SHORT;
		if ((buf[8 + vsize + mlen-1] & 1) == 0)
			return PGPERR_KEY_MODEVEN;
		if ((buf[10+vsize+mlen+elen-1] & 1) == 0)
			return PGPERR_KEY_EXPEVEN;
		if (len > 10+vsize+mlen+elen && !secretf)
			return PGPERR_KEY_LONG;
	} else if (keyID) {
			/*
		* Non-RSA keys use newer keyID algorithm.
		* Low bits of fingerprint20 hash of key, but we can't
		* use that function directly as key is not assembled yet.
				*/
		byte hash[20];
		if (secretf)
			len = ringKeyParsePublicPrefix(buf, len);
		err = pgpFingerprint20HashBuf(buf, len, hash);
		if (err < 0)
			return err;
		pgpAssert (err == sizeof(hash));
		memcpy(keyID, hash+sizeof(hash)-8, 8);
	}

	return 0;

fakeid_short:
	err = PGPERR_KEY_SHORT;
fakeid:
		/*
	* We couldn't read a KeyID. Create a bogus one.
	* The low 32 bits are 0 (which really stands out in a
	* keyring listing and is impossible in the usual case),
	* but the high 32 bits are a CRC of the packet, to reduce
	* reports of keyID collisions.
			*/
	if (keyID) {
		word32 fake = ringHashBuf(buf, len);
		keyID[0] = (byte)(fake >> 24);
		keyID[1] = (byte)(fake >> 16);
		keyID[2] = (byte)(fake >> 8);
		keyID[3] = (byte)fake;
	}
	return err;
}

/* Extract the modulus from a key and return it (buffer & length) */
byte const *
ringKeyParseModulus(byte const *buf, size_t len, unsigned *lenp)
{
	unsigned msize;
	unsigned vsize;

	*lenp = 0;
	if (buf[0] == PGPVERSION_3) {
		vsize = 0;
	} else {
		vsize = 2;
	}

	if (len < 8+vsize || !KNOWN_PGP_VERSION(buf[0]) ||
	(buf[5+vsize] != PGP_PKALG_RSA &&
	buf[5+vsize] != PGP_PKALG_RSA_ENC &&
		buf[5+vsize] != PGP_PKALG_RSA_SIG))
		return 0;

	/* Get bytes in modulus */
	msize = (unsigned)buf[6+vsize] << 8 | buf[7+vsize];
	msize = (msize + 7) / 8;

	*lenp = msize;
	return buf + 8 + vsize;
}

/* Extract the exponent from a key and return it (buffer & length) */
byte const *
ringKeyParseExponent(byte const *buf, size_t len, unsigned *lenp)
{
	unsigned size;
	unsigned vsize;

	*lenp = 0;

	if (buf[0] == PGPVERSION_3) {
		vsize = 0;
	} else {
		vsize = 2;
	}

	if (len < 10+vsize || !KNOWN_PGP_VERSION(buf[0]) ||
	(buf[5+vsize] != PGP_PKALG_RSA &&
	buf[5+vsize] != PGP_PKALG_RSA_ENC &&
		buf[5+vsize] != PGP_PKALG_RSA_SIG))
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
* which gets hashed. We use two different conventions, one for PGP 2.6
* and earlier, and the other for the later version packets. The latter
* includes the whole packet up through the variable part.
*
* buf		 Signature buffer pointer
* len		 buf length in bytes
* lenp		 Return pointer to length of extra buffer
*/
byte const *
ringSigParseExtra(byte const *buf, size_t len, unsigned *lenp)
{
	*lenp = 0;					/* In case of error */
	if (buf[0] == PGPVERSION_3) {
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
byte const *
ringSigParseInteger(byte const *buf, size_t len, unsigned *lenp)
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
	if (buf[0] != PGP_PKALG_RSA &&
	buf[0] != PGP_PKALG_RSA_ENC && buf[0] != PGP_PKALG_RSA_SIG)
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
ringSigParse3(byte const *buf, size_t len, byte *pkalg, byte keyID[8],
	word32 *tstamp, word16 *validity, byte *type, byte *hashalg,
	size_t *extralen, byte *version);
static int
ringSigParseSubpackets(byte const *buf, byte keyID[8],
	word32 *tstamp, word16 *validity);


/*
* Read a signature from the file, positioned just after the header.
* "len" is the length of the signature packet. Leaves the file
* pointed after the length.
*
* A signature is:
* 0 Version: 1 byte
* 1 Length of following material: 1 byte (must be 5!)
* 2 Signature type: 1 byte
* 3 Timestamp: 4 bytes
* 7 KeyID: 8 bytes
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
ringSigParse(byte const *buf, size_t len, byte *pkalg, byte keyID[8],
	word32 *tstamp, word16 *validity, byte *type, byte *hashalg,
	size_t *extralen, byte *version)
{
	unsigned l;	/* extralen */
	unsigned t;
	byte alg;

	/* Default settings in case of error */
	if (pkalg)
		*pkalg = 0;
	if (keyID)
		memset(keyID, 0, 8);
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

	if (len < 1)
		return PGPERR_SIG_SHORT;
	/* Forward compatibility */
	if (buf[0] == PGPVERSION_3) {
		return ringSigParse3(buf, len, pkalg, keyID, tstamp, validity, type,
							hashalg, extralen, version);
	}
	if (version)
		*version = buf[0];
	if (!KNOWN_PGP_VERSION(buf[0]))
		return PGPERR_SIG_VERSION;
	if (len < 2)
		return PGPERR_SIG_SHORT;
	l = buf[1];
	if (extralen)
		*extralen = l;
	if (l >= 1) {
		if (len < 3)
			return PGPERR_SIG_SHORT;
		if (type)
			*type = buf[2];
		if (l >= 5 && tstamp) {
			if (len < 7)
				return PGPERR_SIG_SHORT;
			*tstamp = (word32)((unsigned)buf[3]<<8|buf[4]) << 16 |
			((unsigned)buf[5]<<8|buf[6]);
		}
		if (l >= 7 && validity) {
			if (len < 9)
				return PGPERR_SIG_SHORT;
			*tstamp = (word16)buf[7]<<8 | buf[8];
		}
	}
	if (!extralen && l != 5)
		return PGPERR_SIG_EXTRALEN;
	if (len < l+10)
		return PGPERR_SIG_SHORT;
	if (keyID)
		memcpy(keyID, buf + 7, 8);
	if (len < l+11)
		return PGPERR_SIG_SHORT;
	alg = buf[l+10];
	if (pkalg)
		*pkalg = alg;
#if 0
	if (buf[l+10] != PGP_PKALG_RSA &&
	buf[l+10] != PGP_PKALG_RSA_ENC && buf[l+10] != PGP_PKALG_RSA_SIG)
			 return PGPERR_SIG_PKALG;
#endif
		if (len < l+12)
			 return PGPERR_SIG_SHORT;
		if (hashalg)
			 *hashalg = buf[l+11];
		if (len < l+16)
			 return PGPERR_SIG_SHORT;
		t = (unsigned)buf[l+14] << 8 | buf[l+15];
	if (t && len > l+16 && buf[l+16] >> ((t-1)&7) != 1)
		return PGPERR_SIG_MPI;
	l += 16 + (t+7)/8;
	if (alg == PGP_PKALG_RSA ||
	alg == PGP_PKALG_RSA_ENC || alg == PGP_PKALG_RSA_SIG) {
		if (len != l)
			return len < l ? PGPERR_SIG_SHORT : PGPERR_SIG_LONG;
	}
	return 0;
}

/*
* Version of ringSigParse for the signature packet formats being designed
* for a future version. We do not fully handle them here, but we do a
* basic parse on the sigs and we make sure there are no critical fields
* that we can't handle.
*
* Sig layout:
*
* Offset,Length Meaning
*	0		1	Version byte (=4)
*	1		1	Signature type (included in hash) (nested flag)
*	2		1	PK algorithm (1 = RSA) (included in hash)
*	3		1	Hash algorithm (1 = MD5) (included in hash)
*	4		2	Length of extra material included in hash (=y)
*	6		y	Subpackets (hashed)
* 6+y		2	Length of extra material not included in hash (=z)
* 8+y		z	Subpackets (unhashed)
*=====
*remainder not present on sig headers
*=====
*	8+y+z	2	 First 2 bytes of message digest (16-bit checksum)
*	10+y+z	2+w	MPI of PK-signed integer
*	12+y+z+w
*
*/
static int
ringSigParse3(byte const *buf, size_t len, byte *pkalg, byte keyID[8],
	word32 *tstamp, word16 *validity, byte *type, byte *hashalg,
	size_t *extralen, byte *version)
{
	unsigned	l;				/* extralen */
	unsigned	t;				/* bit count of first MPI */
	size_t		mpiend;			/* byte position past first MPI */
	byte		alg;			/* pk alg for sig */

	if (version)
		*version = buf[0];
	if (len < 8)
		return PGPERR_SIG_SHORT;
	if (type)
		*type = buf[1];
	alg = buf[2];
	if (pkalg)
		*pkalg = alg;
	if (hashalg)
		*hashalg = buf[3];
	l = (unsigned)buf[4]<<8 | buf[5];	 	/* extra material */
	if (extralen)
		*extralen = l;
	if (len < 6+l+2)
		return PGPERR_SIG_SHORT;
	l += (unsigned)buf[l+6]<<8 | buf[l+7];	/* unhashed extra material */
	if (len < 8+l+2+2)
		return PGPERR_SIG_SHORT;
	t = (unsigned)buf[l+10] << 8 | buf[l+11]; /* first MPI size in bits */
	mpiend = 8+l+2+2+(t+7)/8;
	if (len < mpiend)
		return PGPERR_SIG_SHORT;
	if (t && buf[l+12] >> ((t-1)&7) != 1)
		return PGPERR_SIG_MPI;
	/* Special checking for RSA sigs */
	if (alg == PGP_PKALG_RSA ||
	alg == PGP_PKALG_RSA_ENC || alg == PGP_PKALG_RSA_SIG) {
		if (len > mpiend)
			return PGPERR_SIG_LONG;
	}
			
	/* Now parse subpackets, pick up keyid, timestamp and validity info */
	return ringSigParseSubpackets (buf+4, keyID, tstamp, validity);
}


/*
* Parse the "extra data" on the new-style signature packets.
*
* This is a chain of subpackets, with the following format:
* Offset,Length Meaning
*	0	x(1-2)	Subpacket length (=y)
*	x	1	 Subpacket type (with critical flag)
*	x+1	y-1	Subpacket data
*
* We are only parsing a few of the subpackets now, returning an error
* if we see any critical ones that we don't handle. Also, we require
* there to be a keyid subpacket at present or it is an error.
*
* There are two such chains, each preceded by a two-byte count. The
* first is hashed and the second is not.
*/
static int
ringSigParseSubpackets(byte const *buf, byte keyID[8],
	word32 *tstamp, word16 *validity)
{
	unsigned	len;			/* Length of chain of subpackets */
	unsigned	sublen;				/* Length of subpacket */
	int 		type,	 		 	/* Type of subpacket */
				prevtype;		 	/* Type of previous subpacket */
	int			critical;	 		 /* Critical flag on subpacket */
	int			found_keyid = 0;	/* True if found a keyid subpacket */
	int			hashed;				/* True if parsing hashed subpackets */
	word32		creationtime = 0;	/* Creation time from subpacket */
	word32		expirationtime = 0;	/* Expiration time from subpacket */

	hashed = 1;
	/* Loop twice, once with hashed = 1 and once with hashed = 0 */
	do {
		prevtype = 0;
		len = (unsigned)buf[0]<<8 | buf[1];
		buf += 2;
		while (len) {
			if (len < 2)
				return PGPERR_SIG_BADEXTRA;
			/* Subpacket length may be one or two bytes */
			sublen = (unsigned)buf[0];
			if (sublen < 0xc0) {
				/* sublen is the length */
				len -= 1;
				buf += 1;
			} else if ((sublen & 0xe0) == 0xc0) {
				sublen &= 0x3f;
				sublen = (sublen << 8) + (unsigned)buf[1] + 192;
				len -= 2;
				buf += 2;
			} else {
				/* Badly formatted */
				return PGPERR_SIG_BADEXTRA;
			}
			if (len < 1)
				return PGPERR_SIG_BADEXTRA;
			type = buf[0];
			critical = type & SIGSUBF_CRITICAL;
			type &= ~SIGSUBF_CRITICAL;
			buf += 1;
			len -= 1;
			sublen -= 1;
			if (len < sublen)
				return PGPERR_SIG_BADEXTRA;
			/* Illegal to have critical packets in unhashed region */
			if (critical && !hashed)
				return PGPERR_SIG_BADEXTRA;
			/* Packets in order of nondecreasing type */
			if (type < prevtype)
				return PGPERR_SIG_BADEXTRA;
			switch (type) {
			case SIGSUB_CREATION:
				if (sublen != 4)
					return PGPERR_SIG_BADEXTRA;
				if (!tstamp && critical)
					return PGPERR_SIG_BADEXTRA;
				creationtime = (word32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									((unsigned)buf[2]<<8|buf[3]);
				if (tstamp)
					*tstamp = creationtime;
				break;
			case SIGSUB_EXPIRATION:
				if (sublen != 4 || !hashed)
					return PGPERR_SIG_BADEXTRA;
				/* We don't really handle this yet */
				if (critical)
					return PGPERR_SIG_BADEXTRA;
				expirationtime = (word32)((unsigned)buf[0]<<8|buf[1]) << 16 |
									((unsigned)buf[2]<<8|buf[3]);
				/* Store it as days. expirationtime is really valid period */
				if (validity && creationtime)
					*validity = (word16) (expirationtime / (24 * 3600));
				break;
			case SIGSUB_KEYID:
				/* Allow longer than 8 byte keyid's unless critical */
				if (sublen < 8)
					return PGPERR_SIG_BADEXTRA;
				if (critical && (!keyID || sublen != 8))
					return PGPERR_SIG_BADEXTRA;
				if (keyID)
					memcpy(keyID, buf + sublen - 8, 8);
				found_keyid = 1;
				break;
			default:
				if (critical)
					return PGPERR_SIG_BADEXTRA;
			}
			buf += sublen;
			len -= sublen;
			prevtype = type;
		}
		hashed = !hashed;
	} while (!hashed);			/* Two iterations */
	/* For now we must have a keyid. We can live without a creation time. */
	if (keyID && !found_keyid)
		return PGPERR_SIG_BADEXTRA;
	return PGPERR_OK;
}

/*
* Look for a particular subpacket associated with the given signature.
* See comments above for format of subpackets. Return pointer to the
* subpacket data, after the length and type field.
* Critical and hashed tell whether the packet was critical and/or in the
* hashed region, respectively (return parameters).
* We don't need to do consistency checking, it's already been checked.
* (I couldn't resist doing a bit anyway.)
*
* nth should be set to 0 to find first match, 1 for second, etc.
* Returns number of matches total in *pmatches.
*/
byte const *
ringSigFindSubpacket(byte const *buf, int subpacktype, unsigned nth,
	size_t *plen, int *pcritical, int *phashed, word32 *pcreation,
	unsigned *pmatches)
{
	unsigned	len;			/* Length of chain of subpackets */
	unsigned	sublen;				/* Length of subpacket */
	int 		type;	 		 	/* Type of subpacket */
	int			critical;	 		 /* Critical flag on subpacket */
	int			hashed;				/* True if parsing hashed subpackets */
	unsigned	nmatches = 0;		 /* Number of matches so far */
	byte const *pbuf = NULL;		/* Return buffer pointer */

	if (buf[0] < PGPVERSION_3)
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
				len -= 1;
				buf += 1;
			} else if ((sublen & 0xe0) == 0xc0) {
				sublen &= 0x3f;
				sublen = (sublen << 8) + (unsigned)buf[1] + 192;
				len -= 2;
				buf += 2;
			} else {
				/* Badly formatted */
				return NULL;
			}
			if (len < 1)
				return NULL;
			type = buf[0];
			critical = type & SIGSUBF_CRITICAL;
			type &= ~SIGSUBF_CRITICAL;
			buf += 1;
			len -= 1;
			sublen -= 1;

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
					*pcreation = (word32)((unsigned)buf[0]<<8|buf[1]) << 16 |
						((unsigned)buf[2]<<8|buf[3]);
				}
			}

			/* Skip to next packet */
			buf += sublen;
			len -= sublen;
		}
		hashed = !hashed;
	} while (!hashed);			/* Two iterations */
	if (pmatches)
		*pmatches = nmatches;
	return pbuf;				/* Will be non-null if we matched nth */
}


/*
* Return the length of the prefix of a secret key which is a public key,
* or 0 if it can't be determined. A key's prefix is:
* 0 1 - Version
* 1 4 - Creation time
* 5 2 - Vaidity period (days)
* 7 1 - Algorithm
* 8 ? - Algorithm-specific parameters
*/
size_t
ringKeyParsePublicPrefix(byte const *buf, size_t len)
{
	size_t size;
	unsigned vsize;

	if (buf[0] == PGPVERSION_3) {
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
ringKeyParseFingerprint16(byte const *kbuf, size_t klen, byte *fingerprint)
{
	struct PgpHash const *h;
	struct PgpHashContext *hc;
byte const *buf;
unsigned len;

	h = pgpHashByNumber (PGP_HASH_MD5);
	if (!h)
		return PGPERR_BAD_HASHNUM;
	hc = pgpHashCreate(h);
	if (!hc)
		return PGPERR_NOMEM;

buf = ringKeyParseModulus(kbuf, klen, &len);
if (!buf)
		goto error;
pgpHashUpdate(hc, buf, len);

buf = ringKeyParseExponent(kbuf, klen, &len);
if (!buf)
		goto error;
pgpHashUpdate(hc, buf, len);

	buf = pgpHashFinal(hc);
	memcpy(fingerprint, buf, h->hashsize);
	pgpHashDestroy(hc);

	return h->hashsize;	/* Success */
error:
	pgpHashDestroy(hc);
	memset(fingerprint, 0, 16);
	return ringKeyParse(kbuf, klen, NULL, NULL, NULL, NULL, NULL, 0);
	/* Pkalg KeyID KeybitTstampValid */
}


/* Return the size which will be used to create the key prefix buffer */
size_t
ringKeyBufferLength(struct PgpKeySpec const *ks, byte pkalg)
{
	(void)pkalg;
	if (pgpKeySpecVersion(ks)==PGPVERSION_3)
		return 6;	/* version(1)+timestamp(4)+pkalg(1) */
	else
		return 8;	/* version(1)+timestamp(4)+validity(2)+pkalg(1) */
}

/* Create the key prefix buffer */
int
ringKeyToBuffer(byte *buf, struct PgpKeySpec const *ks, byte pkalg)
{
	word32 tstamp;
	word16 validity;
	unsigned vsize;


	buf[0] = pgpKeySpecVersion(ks);
	if (buf[0] == PGPVERSION_3) {
		vsize = 0;
	} else {
		vsize = 2;
	}
	tstamp = pgpKeySpecCreation(ks);
	buf[1] = (byte)(tstamp>>24);
	buf[2] = (byte)(tstamp>>16);
	buf[3] = (byte)(tstamp>> 8);
	buf[4] = (byte)(tstamp );
	if (vsize) {
		validity = pgpKeySpecValidity(ks);
		buf[5] = (byte)(validity>>8);
		buf[6] = (byte)(validity );
	}
	buf[5+vsize] = pkalg;
	return 0;
}

/* Return the size of a prefix buffer for a secret key */
size_t
ringSecBufferLength(struct PgpKeySpec const *ks, byte pkalg)
{
	return ringKeyBufferLength(ks, pkalg);
}

/* Create the prefix buffer for a secret key */
int
ringSecToBuffer(byte *buf, struct PgpKeySpec const *ks, byte pkalg)
{
	return ringKeyToBuffer(buf, ks, pkalg);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
