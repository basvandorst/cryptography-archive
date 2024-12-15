/*
 * $Id: pgpMakeSig.c,v 1.25 1997/10/17 06:15:13 hal Exp $
 */

#include "pgpConfig.h"
#include <string.h>

#include "pgpDebug.h"
#include "pgpMakeSig.h"
#include "pgpHashPriv.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpPubKey.h"
#include "pgpRngPub.h"
#include "pgpSigSpec.h"
#include "pgpUsuals.h"

/* Forward declaration */
static int
pgpMakeSigSubs (PGPByte *buf, PGPSigSpec const *spec,
	 PGPRandomContext const *rc, PGPHashContext const *hc);

/* True if will need to be done as a V3 sig */
static int
pgpV3Sig (PGPSigSpec const *spec)
{
	return pgpSigSpecVersion( spec ) >= PGPVERSION_3;
}

/* The maximum size of the signature */
int
pgpMakeSigMaxSize(PGPSigSpec const *spec)
{
	size_t extralen;
	char const *regExp;

	if (pgpV3Sig (spec)) {
		/* If have preferred algorithms, use new format */
		size_t krkeylen;
		int i;
		pgpSigSpecPrefAlgs(spec, &extralen);
		extralen += 2 + 2 + 8;	/* counts two subpacket hdrs, keyid */
		extralen += 2 + 4;		/* Sig creation subpacket */
		if (pgpSigSpecSigExpiration(spec))
			extralen += 2 + 4;
		if (!pgpSigSpecExportable(spec))
			extralen += 2 + 1;
		if (!pgpSigSpecRevokable(spec))
			extralen += 2 + 1;
		if (pgpSigSpecTrustLevel(spec) != 0)
			extralen += 2 + 2;
		if (IsntNull( regExp = pgpSigSpecRegExp(spec) ) )
			extralen += 2 + strlen(regExp) + 1;
		if (pgpSigSpecKeyExpiration(spec))
			extralen += 2 + 4;
		for (i=0; pgpSigSpecAdditionalRecipientRequest (spec, &krkeylen, i);
					++i)
			extralen += 2 + krkeylen;
		for (i=0; pgpSigSpecRevocationKey (spec, &krkeylen, i); ++i)
			extralen += 2 + krkeylen;
		return 10 + extralen + pgpSecKeyMaxsig(pgpSigSpecSeckey(spec),
											   kPGPPublicKeyMessageFormat_PGP);
	} else {
		/* Old format */
		(void)pgpSigSpecExtra(spec, &extralen);
		return 14 + extralen + pgpSecKeyMaxsig(pgpSigSpecSeckey(spec),
											   kPGPPublicKeyMessageFormat_PGP);
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
pgpMakeSig (PGPByte *buf, PGPSigSpec const *spec,
	 PGPRandomContext const *rc, PGPHashContext const *hc)
{
	PGPSecKey *sec;
	PGPHashContext *temp_hc;
	size_t extralen;
	PGPByte const *extra;
	PGPByte const *hash;
	int i;
	size_t sigsize;
	
	/* Use subpacket sig formats if requesting preferred algs */
	if (pgpV3Sig (spec)) {
		return pgpMakeSigSubs (buf, spec, rc, hc);
	}

	sec = pgpSigSpecSeckey(spec);

	/* XXX should "die" gracefully here */
	pgpAssert (sec->sign);

	temp_hc = pgpHashCopy (hc);
	if (!temp_hc)
		return kPGPError_OutOfMemory;

	extra = pgpSigSpecExtra(spec, &extralen);
	pgpAssert(extralen < 256);
	PGPContinueHash (temp_hc, extra, extralen);

	hash = (PGPByte *) pgpHashFinal(temp_hc);
	/* Copy the first 2 bytes over while they're available */
	memcpy (buf+12+extralen, hash, 2);
	i = pgpSecKeySign(sec, pgpHashGetVTBL( hc ),
		 hash, buf+14+extralen, &sigsize, rc, kPGPPublicKeyMessageFormat_PGP);
	PGPFreeHashContext(temp_hc);
	if (i < 0) {
		buf[12+extralen] = 0;
		buf[13+extralen] = 0;
		return i;
	}
	
	/* Okay, build the signature packet - lots of magic numbers. */
	buf[0] = (PGPByte)PGPVERSION_2_6;		/* Force version 3 as format */
	buf[1] = extralen;
	memcpy(buf+2, extra, extralen);
	memcpy(buf+2+extralen, sec->keyID, 8);
	
	buf[10+extralen] = sec->pkAlg;
	buf[11+extralen] = pgpHashGetVTBL( hc )->algorithm;
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
 * Remember, if adding new subpackets here, that they must go in increasing
 * order of type value within each hashed/unhashed block.
 */
static int
pgpMakeSigSubs (PGPByte *buf, PGPSigSpec const *spec,
	 PGPRandomContext const *rc, PGPHashContext const *hc)
{
	PGPSecKey *sec;
	PGPHashContext *temp_hc;
	size_t extralen;
	unsigned l;
	unsigned unhashoffset;
	size_t alglen;
	size_t adkeylen;
	PGPByte const *extra;
	PGPByte const *hash;
	PGPByte const *algs;
	PGPByte const *adkey;
	char const *regExp;
	PGPUInt32 exp;
	int i;
	size_t sigsize;
	PGPByte postscript[6];
	
	sec = pgpSigSpecSeckey(spec);

	/* XXX should "die" gracefully here */
	pgpAssert (sec->sign);

	extra = pgpSigSpecExtra(spec, &extralen);
	pgpAssert(extralen == 5);
	
	/* Okay, build the signature packet - lots of magic numbers. */
	buf[0] = (PGPByte)PGPVERSION_3;
	buf[1] = extra[0];			/* Signature type */
	buf[2] = sec->pkAlg;
	buf[3] = pgpHashGetVTBL( hc )->algorithm;
	/* Fill in extralen after we figure out how much we need */
	l = 6;

	/* Signature creation */
	buf[l+1] = SIGSUB_CREATION;
	buf[l] = 5;				/* length */
	pgpCopyMemory (extra+1, buf+l+2, 4);
	l += 6;

	/* Sig expiration */
	if ((exp = pgpSigSpecSigExpiration (spec)) != 0) {
		buf[l+1] = SIGSUB_EXPIRATION;
		buf[l] = 1 + 4;
		buf[l+2] = (PGPByte)(exp >> 24);
		buf[l+3] = (PGPByte)(exp >> 16);
		buf[l+4] = (PGPByte)(exp >>  8);
		buf[l+5] = (PGPByte)(exp >>  0);
		l += 2 + 4;
	}

	/* Exportability information */
	if( !pgpSigSpecExportable(spec) ) {
		buf[l+1] = SIGSUB_EXPORTABLE;
		buf[l] = 1 + 1;
		buf[l+2] = pgpSigSpecExportable(spec);
		l += 1 + 2;
	}

	/* Trust information */
	if( pgpSigSpecTrustLevel(spec) > 0 ) {
		buf[l+1] = SIGSUB_TRUST;
		buf[l] = 1 + 2;
		buf[l+2] = pgpSigSpecTrustLevel(spec);
		buf[l+3] = pgpSigSpecTrustValue(spec);
		l += 2 + 2;
	}

	/* Regexp information */
	regExp = pgpSigSpecRegExp(spec);
	if( IsntNull( regExp ) ) {
		size_t regExpLength = strlen(regExp) + 1;
		buf[l+1] = SIGSUB_REGEXP;
		buf[l] = 1 + regExpLength;
		pgpCopyMemory( regExp, buf+l+2, regExpLength );
		l += regExpLength + 2;
	}

	/* Revokability information */
	if( !pgpSigSpecRevokable(spec) ) {
		buf[l+1] = SIGSUB_REVOKABLE;
		buf[l] = 1 + 1;
		buf[l+2] = pgpSigSpecRevokable(spec);
		l += 1 + 2;
	}

	/* Key expiration (self sig) */
	if ((exp = pgpSigSpecKeyExpiration (spec)) != 0) {
		buf[l+1] = SIGSUB_KEY_EXPIRATION;
		buf[l] = 1 + 4;
		buf[l+2] = (PGPByte)(exp >> 24);
		buf[l+3] = (PGPByte)(exp >> 16);
		buf[l+4] = (PGPByte)(exp >>  8);
		buf[l+5] = (PGPByte)(exp >>  0);
		l += 2 + 4;
	}

	/* Additional decryption fingerprint (self sig) */
	for (i=0; (adkey=pgpSigSpecAdditionalRecipientRequest(spec,&adkeylen,i))!=0;
		 ++i) {
		pgpAssert (adkeylen + 1 < 192);
		buf[l+1] = SIGSUB_KEY_ADDITIONAL_RECIPIENT_REQUEST;
		buf[l] = adkeylen + 1;
		pgpCopyMemory (adkey, buf+l+2, adkeylen);
		l += adkeylen + 2;
	}

	/* Preferred algorithms (self sig) */
	algs = pgpSigSpecPrefAlgs (spec, &alglen);
	if (alglen) {
		buf[l+1] = SIGSUB_PREFERRED_ALGS;
		buf[l] = alglen+1;
		pgpCopyMemory (algs, buf+l+2, alglen);
		l += alglen + 2;
	}

	/* Key revocation key fingerprint (self sig) */
	for (i=0; (adkey=pgpSigSpecRevocationKey(spec,&adkeylen,i)) != 0; ++i) {
		pgpAssert (adkeylen + 1 < 192);
		buf[l+1] = SIGSUB_KEY_REVOCATION_KEY;
		buf[l] = adkeylen + 1;
		pgpCopyMemory (adkey, buf+l+2, adkeylen);
		l += adkeylen + 2;
	}

	/* Now can go back and fill in first length field */
	buf[4] = (PGPByte)((l-6) >> 8);
	buf[5] = (PGPByte)((l-6) >> 0);

	/* Hash over the proper portion of the signature packet */
	temp_hc = pgpHashCopy (hc);
	if (!temp_hc)
		return kPGPError_OutOfMemory;

	PGPContinueHash (temp_hc, buf, l);
	
	/* Add hash "postscript" to ensure hashed data can't alias anything */
	postscript[0] = PGPVERSION_3;  /* Hash-convention version */
	postscript[1] = 0xff;		   /* 5th from end, != any sig type value */
	postscript[2] = (PGPByte)(l >> 24);
	postscript[3] = (PGPByte)(l >> 16);
	postscript[4] = (PGPByte)(l >>  8);
	postscript[5] = (PGPByte)(l >>  0);
	PGPContinueHash (temp_hc, postscript, sizeof(postscript));

	/* Fill in unhashed subpacket info now. */
	unhashoffset = l;
	l += 2;

	/* KeyID packet is not hashed, it is advisory! */
	buf[l+1] = SIGSUB_KEYID;
	buf[l] = 8 + 1;
	pgpCopyMemory (sec->keyID, buf+l+2, 8);
	l += 8 + 2;

	/* Now go back and fill in headers on unhashed portion */
	buf[unhashoffset]   = (l - unhashoffset - 2) >> 8;
	buf[unhashoffset+1] = (l - unhashoffset - 2) & 0xff;

	/* Now finish hash calculation, copy over 2 csum bytes, put in sig */
	hash = (PGPByte *) pgpHashFinal(temp_hc);
	pgpCopyMemory (hash, buf+l, 2);
	i = pgpSecKeySign(sec, pgpHashGetVTBL( hc ), hash, buf+l+2, &sigsize,
	                  rc, kPGPPublicKeyMessageFormat_PGP);
	PGPFreeHashContext(temp_hc);
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
pgpMakeSigHeaderMaxSize (PGPSigSpec const *spec)
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
pgpMakeSigHeader (PGPByte *buf, PGPSigSpec const *spec, PGPByte nest)
{
	PGPHashVTBL const *h;
	PGPSecKey const *seckey;
	PGPByte const *extra;
	size_t extralen;

	extra = pgpSigSpecExtra (spec, &extralen);
	h = pgpSigSpecHash (spec);
	seckey = pgpSigSpecSeckey (spec);

	/* XXX: This should be more graceful */
	pgpAssert (extra);
	pgpAssert (h);
	pgpAssert (seckey);
	pgpAssert (seckey->sign);

	buf[0] = (PGPByte)PGPVERSION_2_6;		/* Force version 3 as format */
	buf[1] = *extra;
	buf[2] = h->algorithm;
	buf[3] = seckey->pkAlg;
	memcpy(buf+4, seckey->keyID, 8);
	buf[12] = nest;

	return 13;
}

