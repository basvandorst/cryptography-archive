/*
 * pgpMakeSig.c
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpMakeSig.c,v 1.5.2.3 1997/06/07 09:51:27 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpMakeSig.h"
#include "pgpHash.h"
#include "pgpErr.h"
#include "pgpMem.h"
#include "pgpPubKey.h"
#include "pgpRngPub.h"
#include "pgpSigSpec.h"
#include "pgpUsuals.h"

/* Forward declaration */
static int
pgpMakeSigSubs (byte *buf, struct PgpSigSpec const *spec,
	 struct PgpRandomContext const *rc, struct PgpHashContext const *hc);

/* True if will need to be done as a V3 sig */
static int
pgpV3Sig (struct PgpSigSpec const *spec)
{
	size_t len;
	return pgpSigSpecPrefAlgs (spec, NULL)[0] != 0 ||
		pgpSigSpecKeyExpiration (spec) != 0 ||
		pgpSigSpecRecoveryKey(spec, (unsigned int *) &len, 0) != 0;
}

/* The maximum size of the signature */
int
pgpMakeSigMaxSize(struct PgpSigSpec const *spec)
{
	unsigned extralen;

	if (pgpV3Sig (spec)) {
		/* If have preferred algorithms, use new format */
		unsigned krkeylen;
		int i;
		pgpSigSpecPrefAlgs(spec, &extralen);
		extralen += 2 + 2 + 8;	/* counts two subpacket hdrs, keyid */
		extralen += 2 + 4;		/* Sig creation subpacket */
		if (pgpSigSpecKeyExpiration(spec))
			extralen += 2 + 4;
		for (i=0; pgpSigSpecRecoveryKey (spec, &krkeylen, i); ++i)
			extralen += 2 + krkeylen;
		return 10 + extralen + pgpSecKeyMaxsig(pgpSigSpecSeckey(spec),
											   pgpSigSpecVersion(spec));
	} else {
		/* Old format */
		(void)pgpSigSpecExtra(spec, &extralen);
		return 14 + extralen + pgpSecKeyMaxsig(pgpSigSpecSeckey(spec),
											   pgpSigSpecVersion(spec));
	}
}
	
/*
 * Given a buffer of at least "pgpMakeSigMaxSize" bytes, make a signature
 * into it and return the size of the signature, or 0.
 *
 * Format of signature packets:
 *
 *      Offset  Length  Meaning
 *       0      1       Version byte (= 2 or 3).
 *       1      1       Length of following material included in MD5 (x, = 5)
 *       2      1       Signature type
 *       3      4       32-bit timestamp of signature
 * -------- MD5 additional material stops here, at offset 7 ----------
 *       2+x    8       KeyID
 *      10+x    1       PK algorithm type (1 = RSA)
 *      11+x    1       MD algorithm type (1 = MD5)
 *      12+x    2       First 2 bytes of message digest (16-bit checksum)
 *      14+x    2+?     MPI of PK-encrypted integer
 */
int
pgpMakeSig (byte *buf, struct PgpSigSpec const *spec,
	 struct PgpRandomContext const *rc, struct PgpHashContext const *hc)
{
	struct PgpSecKey *sec;
	struct PgpHashContext *temp_hc;
	unsigned extralen;
	byte const *extra;
	byte const *hash;
	int i;
	PgpVersion version;
	size_t sigsize;
	
	/* Use subpacket sig formats if requesting preferred algs */
	if (pgpV3Sig (spec)) {
		return pgpMakeSigSubs (buf, spec, rc, hc);
	}

	sec = pgpSigSpecSeckey(spec);
	version = pgpSigSpecVersion(spec);

	/* XXX should "die" gracefully here */
	pgpAssert (sec->sign);

	temp_hc = pgpHashClone (hc);
	if (!temp_hc)
		return PGPERR_NOMEM;

	extra = pgpSigSpecExtra(spec, &extralen);
	pgpAssert(extralen < 256);
	pgpHashUpdate (temp_hc, extra, extralen);

	hash = pgpHashFinal(temp_hc);
	/* Copy the first 2 bytes over while they're available */
	memcpy (buf+12+extralen, hash, 2);
	i = pgpSecKeySign(sec, hc->hash, hash, buf+14+extralen, &sigsize,
	                  rc, version);
	pgpHashDestroy (temp_hc);
	if (i < 0) {
		buf[12+extralen] = 0;
		buf[13+extralen] = 0;
		return i;
	}
	
	/* Okay, build the signature packet - lots of magic numbers. */
	buf[0] = (byte)PGPVERSION_2_6;		/* Force version 3 as format */
	buf[1] = extralen;
	memcpy(buf+2, extra, extralen);
	memcpy(buf+2+extralen, sec->keyID, 8);
	
	buf[10+extralen] = sec->pkAlg;
	buf[11+extralen] = hc->hash->type;
	/* First 2 bytes of hash are already copied in */

	return (int)sigsize+14+(int)extralen;
}



/*
 * Subpacket Sig layout:
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
pgpMakeSigSubs (byte *buf, struct PgpSigSpec const *spec,
	 struct PgpRandomContext const *rc, struct PgpHashContext const *hc)
{
	struct PgpSecKey *sec;
	struct PgpHashContext *temp_hc;
	unsigned extralen;
	unsigned l;
	unsigned alglen;
	unsigned krkeylen;
	byte const *extra;
	byte const *hash;
	byte const *algs;
	byte const *krkey;
	word32 exp;
	int i;
	PgpVersion version;
	size_t sigsize;
	byte postscript[6];
	
	sec = pgpSigSpecSeckey(spec);
	version = pgpSigSpecVersion(spec);

	/* XXX should "die" gracefully here */
	pgpAssert (sec->sign);

	extra = pgpSigSpecExtra(spec, &extralen);
	pgpAssert(extralen == 5);
	
	/* Okay, build the signature packet - lots of magic numbers. */
	buf[0] = (byte)PGPVERSION_3;
	buf[1] = extra[0];			/* Signature type */
	buf[2] = sec->pkAlg;
	buf[3] = hc->hash->type;
	/* Fill in extralen after we figure out how much we need */
	l = 6;
	buf[l+1] = SIGSUB_CREATION;
	buf[l] = 5;				/* length */
	pgpCopyMemory (extra+1, buf+l+2, 4);
	l += 6;
	if ((exp = pgpSigSpecKeyExpiration (spec)) != 0) {
		buf[l+1] = SIGSUB_KEY_EXPIRATION;
		buf[l] = 5;
		buf[l+2] = (byte)(exp >> 24);
		buf[l+3] = (byte)(exp >> 16);
		buf[l+4] = (byte)(exp >>  8);
		buf[l+5] = (byte)(exp >>  0);
		l += 6;
	}
	/* Key recovery hash, for when we have that */
	for (i=0; (krkey=pgpSigSpecRecoveryKey(spec,&krkeylen,i)) != 0; ++i) {
		pgpAssert (krkeylen + 1 < 192);
		buf[l+1] = SIGSUB_KEY_RECOVERY_KEY;
		buf[l] = krkeylen + 1;
		pgpCopyMemory (krkey, buf+l+2, krkeylen);
		l += krkeylen + 2;
	}
	/* Preferred algorithms */
	algs = pgpSigSpecPrefAlgs (spec, &alglen);
	if (alglen) {
		buf[l+1] = SIGSUB_PREFERRED_ALGS;
		buf[l] = alglen+1;
		pgpCopyMemory (algs, buf+l+2, alglen);
		l += alglen + 2;
	}

	/* Now can go back and fill in first length field */
	buf[4] = (byte)((l-6) >> 8);
	buf[5] = (byte)((l-6) >> 0);

	/* Hash over the proper portion of the signature packet */
	temp_hc = pgpHashClone (hc);
	if (!temp_hc)
		return PGPERR_NOMEM;

	pgpHashUpdate (temp_hc, buf, l);
	
	/* Add hash "postscript" to ensure hashed data can't alias anything */
	postscript[0] = PGPVERSION_3;  /* Hash-convention version */
	postscript[1] = 0xff;		   /* 5th from end, != any sig type value */
	postscript[2] = (byte)(l >> 24);
	postscript[3] = (byte)(l >> 16);
	postscript[4] = (byte)(l >>  8);
	postscript[5] = (byte)(l >>  0);
	pgpHashUpdate (temp_hc, postscript, sizeof(postscript));

	/* Fill in unhashed subpacket info now.  First length, we know */
	buf[l] = 0;
	buf[l+1] = 8 + 2;
	l += 2;

	/* KeyID packet is not hashed, it is advisory! */
	buf[l+1] = SIGSUB_KEYID;
	buf[l] = 8 + 1;
	pgpCopyMemory (sec->keyID, buf+l+2, 8);
	l += 8 + 2;

	/* Now finish hash calculation, copy over 2 csum bytes, put in sig */
	hash = pgpHashFinal(temp_hc);
	pgpCopyMemory (hash, buf+l, 2);
	i = pgpSecKeySign(sec, hc->hash, hash, buf+l+2, &sigsize,
	                  rc, version);
	pgpHashDestroy (temp_hc);
	if (i < 0) {
		buf[l] = 0;
		buf[l+1] = 0;
		return i;
	}

	return (int)sigsize+l+2;
}



/* Routines for 1-pass signature headers */

/* Right now the header is always 13 bytes long */
int
pgpMakeSigHeaderMaxSize (struct PgpSigSpec const *spec)
{
	(void)spec;
	return 13;
}

/*
 * Given a buffer of appropriate length (currently 13), create a
 * single-pass signature header, which is a "one-pass" signature that
 * will have a real signature later on.  Don't forget that you'll need
 * the packet header (2 more bytes) before this.  The format is:
 *
 *      Offset  Length  Meaning
 *       0      1       Version byte (=3)
 *       1      1       Signature type
 *       2      1       Hash Algorithm
 *       3      1       PK Algorithm
 *       4      8       KeyID
 *       12     1       nested flag
 */
int
pgpMakeSigHeader (byte *buf, struct PgpSigSpec const *spec, byte nest)
{
	struct PgpHash const *h;
	struct PgpSecKey const *seckey;
	byte const *extra;
	unsigned extralen;

	extra = pgpSigSpecExtra (spec, &extralen);
	h = pgpSigSpecHash (spec);
	seckey = pgpSigSpecSeckey (spec);

	/* XXX: This should be more graceful */
	pgpAssert (extra);
	pgpAssert (h);
	pgpAssert (seckey);
	pgpAssert (seckey->sign);

	buf[0] = (byte)PGPVERSION_2_6;		/* Force version 3 as format */
	buf[1] = *extra;
	buf[2] = h->type;
	buf[3] = seckey->pkAlg;
	memcpy(buf+4, seckey->keyID, 8);
	buf[12] = nest;

	return 13;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
