/*
 * pgpKeyMisc.c
 * Miscellaneous helper functions for public key modules.
 * Including packing and unpacking for PKCS compatibility.
 *
 * $Id: pgpKeyMisc.c,v 1.32 1999/04/22 00:37:43 hal Exp $
 */


#include "pgpConfig.h"
#include <string.h>

#include <stdarg.h>

#include "pgpDebug.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "pgpCFBPriv.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpPublicKey.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpRandomX9_17.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"
#include "pgpUtilities.h"
#include "pgpEnv.h"

#ifndef PKCS_COMPAT
#define PKCS_COMPAT 1
#endif


/*
 * Given a number of bits in a modulus, compute the minimum exponent size
 * needed to provide equivalent security from small-exponent attacks.
 * Adding padding on top of this is not an entirely evil idea.
 *
 * This is based on a paper from Michael Wiener   | This extension to
 * on the difficulty of the two attacks, which    | the table is not
 * the following table:                           | part of the original.
 *                                                |
 *   Table 1: Subgroup Sizes to Match Field Sizes |   THIS FUNCTION
 *                                                |
 *  Size of p   Cost of each attack    Size of q  |  Output   Error
 *   (bits)     (instructions or        (bits)    |         (+ is safe)
 *               modular multiplies)              |
 *                                                |
 *     512           9 x 10^17           119      |   137      +18
 *     768           6 x 10^21           145      |   153      +8
 *    1024           7 x 10^24           165      |   169      +4
 *    1280           3 x 10^27           183      |   184      +1
 *    1536           7 x 10^29           198      |   198      +0
 *    1792           9 x 10^31           212      |   212      +0
 *    2048           8 x 10^33           225      |   225      +0
 *    2304           5 x 10^35           237      |   237      +0
 *    2560           3 x 10^37           249      |   249      +0
 *    2816           1 x 10^39           259      |   260      +1
 *    3072           3 x 10^40           269      |   270      +1
 *    3328           8 x 10^41           279      |   280      +1
 *    3584           2 x 10^43           288      |   289      +1
 *    3840           4 x 10^44           296      |   297      +1
 *    4096           7 x 10^45           305      |   305      +0
 *    4352           1 x 10^47           313      |   313      +0
 *    4608           2 x 10^48           320      |   321      +1
 *    4864           2 x 10^49           328      |   329      +1
 *    5120           3 x 10^50           335      |   337      +2
 *
 * This function fits a curve to this, which overestimates the size
 * of q required, but by a very small amount in the important 1000-4000
 * bit range.  It is a quadratic curve up to 3840 bits, and a linear
 * curve past that.  They are designed to be C(1) (have the same value
 * and the same slope) at the point where they meet.
 */
#define AN      1       /* a = -AN/AD/65536, the quadratic coefficient */
#define AD      3
#define M       8       /* Slope = M/256, i.e. 1/32 where linear starts */
#define TX      3840    /* X value at the slope point, where linear starts */
#define TY      297     /* Y value at the slope point, where linear starts */
/*
 * For a slope of M at the point (TX,TY), we only have one degree of
 * freedom left in a quadratic curve, so use the coefficient of x^2,
 * namely a, as that free parameter.
 *
 * y = -AN/AD*((x-TX)/256)^2 + M*(x-TX)/256 + TY
 *   = -AN*(x-TX)*(x-TX)/AD/256/256 + M*x/256 - M*TX/256 + TY
 *   = -AN*x*x/AD/256/256 + 2*AN*x*TX/AD/256/256 - AN*TX*TX/AD/256/256 \
 *      + M*x/256 - M*TX/256 + TY
 *   = -AN*(x/256)^2/AD + 2*AN*(TX/256)*(x/256)/AD + M*(x/256) \
 *      - AN*(TX/256)^2/AD - M*(TX/256) + TY
 *   = (AN*(2*TX/256 - x/256) + M*AD)*x/256/AD - (AN*(TX/256)/AD + M)*TX/256 \
 *      + TY
 *   = (AN*(2*TX/256 - x/256) + M*AD)*x/256/AD \
 *      - (AN*(TX/256) + M*AD)*TX/256/AD + TY
 *   =  ((M*AD + AN*(2*TX/256 - x/256))*x - (AN*(TX/256)+M*AD)*TX)/256/AD + TY
 *   =  ((M*AD + AN*(2*TX - x)/256)*x - (AN*(TX/256)+M*AD)*TX)/256/AD + TY
 *   =  ((M*AD + AN*(2*TX - x)/256)*x - (M*AD + AN*TX/256)*TX)/256/AD + TY
 *   =  (((256*M*AD+2*AN*TX-AN*x)/256)*x - (M*AD + AN*TX/256)*TX)/256/AD + TY
 *
 * Since this is for the range 0...TX, in order to avoid having any
 * intermediate results less than 0, we need one final rearrangement, and
 * a compiler can easily take the constant-folding from there...
 *
 *   =  TY + (((256*M*AD+2*AN*TX-AN*x)/256)*x - (M*AD + AN*TX/256)*TX)/256/AD
 *   =  TY - ((M*AD + AN*TX/256)*TX - ((256*M*AD+2*AN*TX-AN*x)/256)*x)/256/AD
 */

#define BITS(x) \
        TY - ((M*AD + AN*TX/256)*TX - ((256*M*AD+AN*2*TX-AN*x)/256)*x)/(AD*256)

unsigned
pgpDiscreteLogExponentBits(unsigned modbits)
{
	unsigned expbits;

	if (modbits > TX)
		expbits = M*modbits/256 - M*TX/256 + TY;    /* Linear */
	else
		expbits = BITS(modbits);       /* Quadratic */
	return expbits;
}

/* For the Q prime in the DSA, we round up to a multiple of 8 bits.
 * That way when we make the signature, we can bring our hash output over
 * in a multiple of bytes.
 */
unsigned
pgpDiscreteLogQBits(unsigned modbits)
{
	return ((pgpDiscreteLogExponentBits(modbits)+7)/8)*8;
}

#undef BITS
#undef TY
#undef TX
#undef M
#undef AD
#undef AN

/*
 * Fill the given bignum, from bytes high-1 through low (where 0 is
 * the least significant byte), with non-zero random data.
 */
static int
randomPad(PGPRandomContext const *rc, BigNum *bn,
	unsigned high, unsigned low)
{
	unsigned i, l;
	PGPByte padding[64];   /* This can be any size (>0) whatsoever */

	high -= low;
	while (high) {
		l = high < sizeof(padding) ? high : sizeof(padding);
		pgpRandomGetBytes(rc, padding, l);
                for (i = 0; i < l; i++) {       /* Replace all zero bytes */ 
                        while(padding[i] == 0)
				pgpRandomGetBytes(rc, padding+i, 1);
                }
		high -= l;
		if (bnInsertBigBytes(bn, padding, high+low, l) < 0)
			return kPGPError_OutOfMemory;
	}

	pgpClearMemory( padding,  sizeof(padding));
	return 0;
}

/*
 * Fill the given bignum, from bytes high-1 through low (where 0 is
 * the least significant byte), with all ones (0xFF) data.
 */
static int
onesPad(BigNum *bn, unsigned high, unsigned low)
{
	unsigned l;
	static PGPByte const padding[] = {
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255
	};

	high -= low;
	while (high) {
		l = high < sizeof(padding) ? high : sizeof(padding);
		high -= l;
		if (bnInsertBigBytes(bn, padding, high+low, l) < 0)
			return kPGPError_OutOfMemory;
	}
	return 0;
}


/*
 * The Basic Encoding Rules (of which the Distinguished Encoding Rules
 * are a simple minimal-sized subset) are supposed to be compact.  Humph.
 * Maybe they are, but the ASN.1 they're encoding is less than concise.
 * It's sequence(sequence(object_id(1.2.840.113549.2.5)),octet_string(...))
 */
static PGPByte const MD5_prefix[] = {
	0x30, /* Universal, Constructed, Sequence */
	0x20, /* Length 32 (bytes following) */
		0x30, /* Universal, Constructed, Sequence */
		0x0c, /* Length 12 */
			0x06, /* Universal, Primitive, object-identifier */
			0x08, /* Length 8 */
				0x2a, /* 42 = ISO(1)*40 + Member bodies(2) */
				0x86, 0x48, /* 840 = US (ANSI) */
				0x86, 0xF7, 0x0D, /* 113549 = RSADSI */
				0x02, /* 2 = Hash functions */
				0x05, /* 5 = MD5 */
			0x05, /* Universal, Primitive, NULL */
			0x00, /* Length 0 */
		0x04, /* Universal, Primitive, Octet string */
		0x10 /* Length 16 */
			/* 16 MD5 digest bytes go here */
};

/* This is the PGP 2.2 MD5 identification byte */
static PGPByte const pgp22_MD5_byte = 1;


/*
 * Wrap a PKCS wrapper around some data.
 * Type 1 is for signature, type 2 for encryption.
 *
 * Type 2 wrappers:
 *
 * If the modulus is n bytes long, with the most significant byte
 * being n-1 and the least significant, 0, the wrapper looks like:
 *
 * Position     Value   Function
 * n-1           0      This is needed to ensure that the padded number
 *                      is less than the modulus.
 * n-2           2      The padding type (non-zero random).
 * n-3..len+1   ???     Non-zero random padding bytes to "salt" the
 *                      output and prevent duplicate plaintext attacks.
 * len           0      Zero byte to mark the end of the padding
 * len-1..0     data    Supplied payload data.
 *
 *
 * In the case of PGP, the payload is a byte indicating the type of
 * conventional cipher, currently always set to 1 to indicate IDEA,
 * and a 16-byte IDEA key followed by a 16-bit checksum.
 *
 * Versions of PGP up to 2.2 generated a "broken" version of this format,
 * which had the components (except for the most significant zero byte)
 * in the opposite order.  This is controlled by PKCS_COMPAT.
 * There's no security difference, it's just nice if everyone does it
 * the same way.
 *
 * Position     Value
 * n-1           0
 * n-2..n-len-1 data    Supplied payload data.
 * n-len-2       0      Zero byte to mark the end of the padding
 * n-len-3..1   ???     Non-zero random padding bytes to "salt" the
 *                      output and prevent duplicate plaintext attacks.
 * 0             2      The padding type (non-zero random).
 *
 * On decryption, the fact that the first (most significant) byte of
 * payload data is 1 for the <= 2.2 format, and a padding type of 1
 * (fixed, all ones) is never used for public-key encryption lets
 * these be distinguished.
 *
 * There really should be several bytes of padding, although this
 * routine will not fail to encrypt unless it will not fit, even
 * with no padding bytes.
 *
 *
 * Type 1 padding:
 *
 *
 * The type is 1, and the padding is all 1's
 * (hex 0xFF).  In addition, if the data is a DER-padded MD5 hash, there's
 * an option for encoding it with the old PGP 2.2 format, in which case
 * that's all replaced by a 1 byte indicating MD5.
 *
 * When decrypting, distinguishing these is a bit trickier, since the
 * second most significant byte is 1 in both cases, but in general,
 * it could only cause confusion if the PGP hash were all 1's.
 *
 * To summarize, the formats are:
 *
 * Position     Value   Function
 * n-1           0      This is needed to ensure that the padded number
 *                      is less than the modulus.
 * n-2           1      The padding type (all ones).
 * n-3..len+1   255     All ones padding to ensure signatures are rare.
 * len           0      Zero byte to mark the end of the padding
 * len-1..x     ASN.1	The ASN.1 DER magic cookie (18 bytes)
 * x-1..0       data    The payload MD5 hash (16 bytes).
 *
 *
 * Position     Value
 * n-1           0
 * n-2           1	"This is MD5"
 * n-2..n-len-2 data    Supplied payload MD5 hash (len == 16).
 * n-len-2       0      Zero byte to mark the end of the padding
 * n-len-3..1   255     All ones padding.
 * 0             1      The padding type (all ones).
 *
 *
 * The reason for the all 1's padding is an extra consistency check.
 * A randomly invented signature will not decrypt to have the long
 * run of ones necessary for acceptance.
 *
 * Oh... the public key isn't needed to decrypt, but it's passed in
 * because a different glue library may need it for some reason.
 *
 * TODO: Have the caller put on the PKCS wrapper.  We can notice and
 * strip it off if we're trying to be compatible.
 */

int
pgpPKCSPack(BigNum *bn, PGPByte const *in, unsigned len, PGPByte padtype,
	unsigned bytes, PGPRandomContext const *rc)
{
	if (len+3 > bytes)
		return kPGPError_PublicKeyTooSmall;	/* data won't fit in pubkey! */

	/* Set the entire number to 0 to start */
	(void)bnSetQ(bn, 0);
	
	if (padtype == PKCS_PAD_ENCRYPTED) {
		/* Random padding */
		if (PKCS_COMPAT || in[0] != 1) {
			if (bnInsertBigBytes(bn, &padtype, bytes-2, 1) < 0 ||
			    randomPad(rc, bn, bytes-2, len+1) < 0 ||
			    bnInsertBigBytes(bn, in, 0, len) < 0)
				return kPGPError_OutOfMemory;
		} else {
			/* Old <= 2.2 broken format */
			if (bnInsertBigBytes(bn, in, bytes-len-1, len) < 0 ||
			    randomPad(rc, bn, bytes-len-2, 1) < 0 ||
			    bnInsertBigBytes(bn, &padtype, 0, 1) < 0)
				return kPGPError_OutOfMemory;
		}
	} else {
		pgpAssert (padtype == PKCS_PAD_SIGNED);
		/* Constant padding */
		if (PKCS_COMPAT
		    || len != sizeof(MD5_prefix)+16
		    || memcmp(in, MD5_prefix, sizeof(MD5_prefix)) != 0)
		{
			if (bnInsertBigBytes(bn, &padtype, bytes-2, 1) < 0 ||
			    onesPad(bn, bytes-2, len+1) < 0 ||
			    bnInsertBigBytes(bn, in, 0, len) < 0)
				return kPGPError_OutOfMemory;
		} else {
			/* Old <= 2.2 format - no DER prefix */
			len -= sizeof(MD5_prefix);
			in += sizeof(MD5_prefix);
			if (len+3+1 > bytes)
				return kPGPError_PublicKeyTooSmall;	/* won't fit */
			if (bnInsertBigBytes(bn, &pgp22_MD5_byte, bytes-1, 1)
					< 0 ||
			    bnInsertBigBytes(bn, in, bytes-len-2, len) < 0 ||
			    onesPad(bn, bytes-len-2, 1) < 0 ||
			    bnInsertBigBytes(bn, &padtype, 0, 1) < 0)
				return kPGPError_OutOfMemory;
		}
	}
	return 0;
}


/*
 * Searches bytes, beginning with start-1 and progressing to 0,
 * until one that is not 0xff is found.  The idex of the last 0xff
 * byte is returned (or start if start-1 is not 0xff.)
 */
static unsigned
bnSearchNonOneFromHigh(BigNum const *bn, unsigned start)
{
	PGPByte buf[16];	/* Size is arbitrary */
	unsigned l;
	unsigned i;

	while (start) {
		l = start < sizeof(buf) ? start : sizeof(buf);
		start -= l;
		bnExtractBigBytes(bn, buf, start, l);
		for (i = 0; i < l; i++) {
			if (buf[i] != 0xff) {
				pgpClearMemory( buf,  sizeof(buf));
				return start + l - i;
			}
		}
	}
	/* Nothing found */
	pgpClearMemory( buf,  sizeof(buf));
	return 0;
}

/*
 * Searches bytes, beginning with start and going up (towards more
 * significant bytes), until one that is not 0xff is found.  Since the
 * input number is finite, there must be zeros eventually.
 * The index of the first non-0xff byte is returned.
 */
static unsigned
bnSearchNonOneFromLow(BigNum const *bn, unsigned start)
{
	PGPByte buf[16];	/* Size is arbitrary */
	unsigned i;

	for (;;) {
		bnExtractBigBytes(bn, buf, start, sizeof(buf));
		i = sizeof(buf);
		start += i;
		do {
			if (buf[--i] != 0xff) {
				pgpClearMemory( buf,  sizeof(buf));
				return start - i - 1;
			}
		} while (i);
	}
	/*NOTREACHED*/
}



/*
 * Searches bytes, beginning with start-1 and progressing to 0,
 * until finding one that is zero, or the end of the array.
 * The index of the last non-zero byte is returned (0 if the array
 * is all non-zero, or start if start-1 is zero).
 */
static unsigned
bnSearchZeroFromHigh(BigNum const *bn, unsigned start)
{
	PGPByte buf[16];	/* Size is arbitrary */
	unsigned l;
	unsigned i;

	while (start) {
		l = start < sizeof(buf) ? start : sizeof(buf);
		start -= l;
		bnExtractBigBytes(bn, buf, start, l);
		for (i = 0; i < l; i++) {
			if (buf[i] == 0) {
				pgpClearMemory( buf,  sizeof(buf));
				return start + l - i;
			}
		}
	}
	/* Nothing found */
	pgpClearMemory( buf,  sizeof(buf));
	return 0;
}

/*
 * Searches bytes, beginning with start and going up (towards more
 * significant bytes), until one that is zero is found.  Since the
 * input number is finite, there must be zeros eventually.
 * The index of the first zero byte is returned.
 */
static unsigned
bnSearchZeroFromLow(BigNum const *bn, unsigned start)
{
	PGPByte buf[16];	/* Size is arbitrary */
	unsigned i;

	for (;;) {
		bnExtractBigBytes(bn, buf, start, sizeof(buf));
		i = sizeof(buf);
		start += i;
		do {
			if (buf[--i] == 0) {
				pgpClearMemory( buf,  sizeof(buf));
				return start - i - 1;
			}
		} while (i);
	}
	/*NOTREACHED*/
}

/*
 * Performs a PKCS unpack operation.  Returns a prefix of the unwrapped
 * data in the given buf.  Returns the length of the untruncated
 * data, which may exceed "len". Returns <0 on error.
 *
 * For the constant-padding (signature checking) case,
 * it recongizes the PGP 2.2 format, but not in all its generality;
 * only the one case (framing byte = 1, length = 16) which was ever
 * generated.  It fakes the DER prefix in that case.
 */
int
pgpPKCSUnpack(PGPByte *buf, unsigned len, BigNum *bn, PGPByte padtype,
	unsigned bytes)
{
	PGPByte tmp[2];

	bnExtractBigBytes(bn, tmp, bytes-2, 2);
	if (tmp[0] != 0) {
		pgpClearMemory( tmp,  2);
		return kPGPError_CorruptData;
	}

	if (padtype == PKCS_PAD_ENCRYPTED) {
		/* Decryption case, random padding */
		/* Special case: PGP <= 2.2 hack */
		bnExtractBigBytes(bn, tmp, 0, 1);
		if (tmp[1] == 1 && tmp[0] == padtype &&
		    bnSearchZeroFromLow(bn, 1) == bytes-1-1-16-2-1)
		{
			/* Aha, it's PGP <= 2.2 */
			if (len > 1+16+2)
				len = 1+16+2;
			bnExtractBigBytes(bn, buf, bytes-1-len, len);
			return 1+16+2;
		}

		/* Okay, assume it's PKCS.1 */
		if (tmp[1] != 2) {
			pgpClearMemory( tmp,  2);
			return kPGPError_CorruptData;
		}
		pgpClearMemory( tmp,  2);

		bytes = bnSearchZeroFromHigh(bn, bytes-2);
		if (bytes-- == 0)
			return kPGPError_CorruptData;
	} else {
		pgpAssert (padtype==PKCS_PAD_SIGNED);
		/* Signature check, constant padding */
		if (tmp[1] != padtype) {
			pgpClearMemory( tmp,  sizeof(tmp));
			return kPGPError_CorruptData;
		}

		/*
		 * Special-case hack: is is PGP <= 2.2 format?	This is
		 * identified by a least significant byte of 1, a byte
		 * of 0 between the 16-byte MD5 hash and the padding,
		 * and all ones padding.
		 */
		bnExtractBigBytes(bn, tmp, 0, 1);  /* Should be 1 if <= 2.2 */
		bnExtractBigBytes(bn, tmp+1, bytes-19, 1); /* zero if <= 2.2 */

		if (tmp[0] == padtype && tmp[1] == 0 &&
		    bnSearchNonOneFromLow(bn, 1) == bytes-19)
		{
			/* Aha, it's PGP <= 2.2 - fake up the DER prefix */
			if (len <= sizeof(MD5_prefix)) {
				memcpy(buf, MD5_prefix, len);
			} else {
				memcpy(buf, MD5_prefix, sizeof(MD5_prefix));
				buf += sizeof(MD5_prefix);
				len -= sizeof(MD5_prefix);
				if (len > 16)
					len = 16;
				bnExtractBigBytes(bn, buf, bytes-2-len, len);
			}
			return 16 + sizeof(MD5_prefix);
		}

		/* Okay, assume it's PKCS format thing. */

		bytes = bnSearchNonOneFromHigh(bn, bytes-2);
		if (bytes < 1)
			return kPGPError_CorruptData;
		bytes--;
		tmp[1] = 0;
		bnExtractBigBytes(bn, tmp, bytes, 1);
		if (tmp[0] != 0) {
			pgpClearMemory( tmp,  sizeof(tmp));
			return kPGPError_CorruptData;
		}
		/* Note: tmp isn't secret any more because it's a constant! */
	}

	/* Success! Return the data */
	if (len > bytes)
		len = bytes;
	bnExtractBigBytes(bn, buf, bytes-len, len);
	return bytes;
}

/*
 * Convert a big-endian byte buffer (with bit-count prefix) to an MPI.
 * Returns number of bytes read from buffer, or <= 0 on error.
 * (Returns 0 if the buffer is too short.)
 */
int
pgpBnGetPlain(BigNum *bn, PGPByte const *buf, unsigned size)
{
	unsigned t;

	if (size < 2)
		return 0;
	t = ((unsigned)buf[0] << 8) + buf[1];
	t = (t+7)/8;
	if (size < t+2)
		return 0;

	if (bnInsertBigBytes(bn, buf+2, 0, t) < 0)
		return kPGPError_OutOfMemory;
	return (int)t+2;
}

/* Read start of an X.509 object */
PGPByte
pgpBnX509TagLen(PGPByte const **buf, PGPUInt32 *length)
{
	PGPByte tag = *(*buf)++ & 0x1f;
	PGPUInt32 len = *(*buf)++;
	if (len & 0x80) {
		PGPUInt32 lenlen = len & 0x7f;
		len = 0;
		while (lenlen--) {
			len <<= 8;
			len |= *(*buf)++;
		}
	}
	*length = len;
	return tag;
}

PGPUInt32
pgpBnX509LenLen(PGPUInt32 length)
{
	if (length < 0x80)
		return 1;
	if (length < 0x100)
		return 2;
	if (length < 0x10000)
		return 3;
	if (length < 0x1000000)
		return 4;
	return 5;
}

/*
 * Convert a big-endian byte buffer to an MPI.  PGP format uses a
 * bitcount prefix.
 * Returns number of bytes read from buffer, or <= 0 on error.
 * (Returns 0 if the buffer is too short.)
 */
int
pgpBnGetFormatted(BigNum *bn, PGPByte const *buf, unsigned size,
	unsigned modbytes, PGPPublicKeyMessageFormat format)
{
	unsigned extra = 0;

	if (format == kPGPPublicKeyMessageFormat_PGP) {
		return pgpBnGetPlain( bn, buf, size );
	}

	if (format == kPGPPublicKeyMessageFormat_X509) {
		PGPUInt32 len;
		PGPByte const *bp = buf;
		if (pgpBnX509TagLen(&buf, &len) != X509_TAG_INTEGER)
			return kPGPError_MalformedKeyComponent;
		extra = buf - bp;
		size -= extra;
		if (len > size)
			return kPGPError_MalformedKeyComponent;
		modbytes = len;
	} else {
		pgpAssert( format == kPGPPublicKeyMessageFormat_PKCS1 ||
				   format == kPGPPublicKeyMessageFormat_IKE );
	}

	if (size < modbytes)
		return 0;

	if (bnInsertBigBytes(bn, buf, 0, modbytes) < 0)
		return kPGPError_OutOfMemory;
	return (int)(modbytes + extra);
}


/*
 * Helper function for key unlocking.
 * Convert a big-endian byte buffer to an MPI, with optional decryption and
 * checksums.  Accepts cfb == NULL to mean "unencrypted".
 * Returns number of bytes read from buffer, or <= 0 on error.
 * (Returns 0 if the buffer is too short.)
 *
 * If "old" is true, this does it in the PGP 2.x way, where the length
 * of the data is unencrypted and the CFB is resynced each step.
 * If "old" is false, this assumes that just everything is encrypted
 * and 
 */
int
pgpBnGet(BigNum *bn, PGPByte const *buf, unsigned size,
	PGPCFBContext *cfb, unsigned *checksump, int old)
{
	PGPByte tmp[64];	/* This can be any (non-zero) size >= 2 */
	unsigned t, l;

	if (!cfb) {
		int i;

		i = pgpBnGetPlain(bn, buf, size);
		if (i >= 0 && checksump) {
			for (t = 0; t < (unsigned)i; t++)
				*checksump += buf[t];
		}
		return i;
	}

	if (size < 2)
		return 0;
	if (old) {
		/* Length it bits is not encrypted */
		PGPCFBSync(cfb);
		if (checksump)
			*checksump += (unsigned)buf[0] + buf[1];
		t = ((unsigned)buf[0] << 8) + buf[1];
	} else {
		/* Length in bits is encrypted */
		pgpCFBDecryptInternal(cfb, buf, 2, tmp );
		if (checksump)
			*checksump += (unsigned)tmp[0] + tmp[1];
		t = ((unsigned)tmp[0] << 8) + tmp[1];
	}
	buf += 2;
	t = (t+7)/8;
	if (size < t+2)
		return 0;

	/*
	 * Descrypt and convert in pieces.  It's done from the
	 * most-significnt end to force allocation of the result
	 * number all at once rather than reallocating it bit by bit.
	 */
	size = t;
	while (size) {
		l = size < sizeof(tmp) ? size : sizeof(tmp);

		pgpCFBDecryptInternal(cfb, buf, l, tmp);
		size -= l;
		buf += l;
		if (bnInsertBigBytes(bn, tmp, size, l) < 0) {
			pgpClearMemory( tmp,  sizeof(tmp));
			return kPGPError_OutOfMemory;
		}

		/* Checksum */
		if (checksump) {
			do {
				*checksump += tmp[--l];
			} while (l);
		}
	}
	pgpClearMemory( tmp,  sizeof(tmp));

	return (int)t+2;
}

/*
 * Read the 2-byte simple checksum (as computed above) from the
 * buffer for comparison.  Old-style is unencrypted, new-style is
 * encrypted.
 */
unsigned
pgpChecksumGet(PGPByte const *buf, PGPCFBContext *cfb, int old)
{
	PGPByte tmp[2];
	unsigned checksum;

	if (old || !cfb) {
		checksum = ((unsigned)buf[0] << 8) + buf[1];
	} else {
		pgpCFBDecryptInternal(cfb, buf, 2, tmp);
		checksum = ((unsigned)tmp[0] << 8) + tmp[1];
		pgpClearMemory( tmp,  2);
	}
	return checksum;
}

/*
 * Convert an MPI to a big-endian byte buffer, with a length prefix.
 * Returns number of bytes put into buffer.
 */
unsigned
pgpBnPutPlain(BigNum const *bn, PGPByte *buf)
{
	unsigned t;

	t = bnBits(bn);
	buf[0] = (PGPByte)(t>>8 & 255);
	buf[1] = (PGPByte)(t & 255);
	t = (t+7)/8;
	bnExtractBigBytes(bn, buf+2, 0, t);
	return t+2;
}

/*
 * Convert an MPI to a big-endian byte buffer.  PGP format uses a
 * length prefix.
 * Returns number of bytes put into buffer.
 */
unsigned
pgpBnPutFormatted(BigNum const *bn, PGPByte *buf,
	unsigned modbytes, PGPPublicKeyMessageFormat format)
{
	unsigned extra = 0;

	if (format == kPGPPublicKeyMessageFormat_PGP) {
		return pgpBnPutPlain( bn, buf );
	}

	if (format == kPGPPublicKeyMessageFormat_X509) {
		/* Insert integer prefix in DER encoding */
		PGPUInt32 lenlen;
		PGPUInt32 len;
		if (bnBits(bn) == 8*modbytes)
			modbytes += 1;
		len = modbytes;
		*buf++ = X509_TAG_INTEGER;
		++extra;
		lenlen = pgpBnX509LenLen(len);
		if (--lenlen == 0) {
			*buf++ = len;
			++extra;
		} else {
			*buf++ = 0x80 | lenlen;
			++extra;
			len <<= 8 * (4-lenlen);
			while (lenlen--) {
				*buf++ = (PGPByte)(len >> 24);
				++extra;
				len <<= 8;
			}
		}
	}
	bnExtractBigBytes(bn, buf, 0, modbytes);
	return modbytes + extra;
}

/*
 * Helper function for ChangeLock.
 * Convert an MPI to a big-endian byte buffer, with optional encryption and
 * checksums.  Accepts cfb == NULL to mean "unencrypted".
 * Returns number of bytes put into buffer.
 */
unsigned
pgpBnPut(BigNum const *bn, PGPByte *buf,
	PGPCFBContext *cfb, unsigned *checksump, int old)
{
	unsigned t, u;

	t = pgpBnPutPlain(bn, buf);
	if (checksump)
		for (u = 0; u < t; u++)
			*checksump += buf[u];
	if (cfb) {
		if (old) {
			PGPCFBSync(cfb);
			pgpCFBEncryptInternal(cfb, buf+2, t-2, buf+2);
		} else {
			pgpCFBEncryptInternal(cfb, buf, t, buf);
		}
	}
	return t;
}

/*
 * Write the 2-byte simple checksum (as computed above) to the
 * buffer for comparison.  Old-style is unencrypted, new-style is
 * encrypted.
 */
void
pgpChecksumPut(unsigned checksum, PGPByte *buf, PGPCFBContext *cfb,
	int old)
{
	buf[0] = (PGPByte)(checksum>>8 & 255);
	buf[1] = (PGPByte)(checksum & 255);
	if (cfb && !old)
		pgpCFBEncryptInternal(cfb, buf, 2, buf);
}

/*
 * Generate a random bignum of the specified length, with the given
 * high and low 8 bits. "High" is merged into the high 8 bits of the
 * number.  For example, set it to 0x80 to ensure that the number is
 * exactly "bits" bits long (i.e. 2^(bits-1) <= bn < 2^bits).
 * "Low" is merged into the low 8 bits.  For example, set it to
 * 1 to ensure that you generate an odd number.
 */
int
pgpBnGenRand(BigNum *bn, PGPRandomContext const *rc,
	     unsigned bits, PGPByte high, PGPByte low, unsigned effective)
{
	unsigned char buf[64];
	unsigned bytes;
	unsigned l;
	int err;

	bnSetQ(bn, 0);

	/* Get high random bits */
	bytes = (bits+7) / 8;
	l = bytes < sizeof(buf) ? bytes : sizeof(buf);
	pgpRandomGetBytesEntropy(rc, buf, l, effective);

	/* Mask off excess high bits */
	buf[0] &= 255 >> (-(int)bits & 7);
	/* Merge in specified high bits */
	buf[0] |= high >> (-(int)bits & 7);
	if (bits & 7)
		buf[1] |= high << (bits & 7);

	for (;;) {
		bytes -= l;
		if (!bytes)	/* Last word - merge in low bits */
			buf[l-1] |= low;
		err = bnInsertBigBytes(bn, buf, bytes, l);
		if (!bytes || err < 0)
			break;
		l = bytes < sizeof(buf) ? bytes : sizeof(buf);
		pgpRandomGetBytesEntropy(rc, buf, l, 0);
	}

	/* Burn and return */
	pgpClearMemory( buf,  sizeof(buf));
	return err;
}

/*
 * Parse a buffer containing n mpi format numbers (two bytes of length in bits,
 * followed by data).  Make sure data is well formed and doesn't exceed
 * buffer length.  Take n pointers to offsets where the n numbers start
 * (pointers may be null but must not be left off arg list).
 * Return offset past last value, or negative for error.
 */
int
pgpBnParse(PGPByte const *buf, unsigned size, int n, ...)
{
	va_list ap;
	unsigned nb;
	unsigned off;
	unsigned *poff;

	va_start (ap, n);
	if (size < 2U*n)
		return kPGPError_KeyPacketTruncated;
	off = 0;
	while (n--) {
		poff = va_arg(ap, unsigned *);
		nb = ((unsigned)buf[0+off] << 8) + buf[1+off];
		if (!nb || buf[2+off] >> ((nb-1) & 7) != 1)
			return kPGPError_MalformedKeyComponent; /* Bad bit length */
		nb = (nb+7)/8;
		/* Need nb+2 bytes for this, plus 2*n for remainder */
		if (size-off < nb + 2 + 2*n)
			return kPGPError_KeyPacketTruncated;
		if (poff)
			*poff = off;
		off += nb+2;
	}
	va_end (ap);
	return off;
}

/*
 * Given a cipher algorithm descriptor in (buf,len) and a passphrase,
 * initialize the passed-in PGPCFBContext pointer and return the
 * number of bytes of descriptor used, or <0 on error.  (In which
 * case *cfbp is NULL.)  If hashedPhrase is true, the passphrase has
 * already been hashed using the s2k object and we use it literally
 * as the key.
 */
int
pgpCipherSetup(
PGPByte const *buf, unsigned len, char const *phrase, size_t plen,
	PGPBoolean hashedPhrase, PGPEnv const *env, PGPCFBContext **cfbp)
{
	PGPCipherVTBL const *cipher;
	PGPStringToKey *s2k;
	unsigned alg;
	int alglen;
	PGPByte key[PGP_CIPHER_MAXKEYSIZE];
	PGPContextRef	context	= pgpenvGetContext( env );

	/* Sanity check on lengths, otherwise we take forever */
	pgpAssert( plen < 0x10000U );
	pgpAssert( len < 0x10000U );

	/* First things first, in case of error... */
	*cfbp = NULL;

	if (len < 1)
		return kPGPError_KeyPacketTruncated;

	alg = buf[0] & 255;

	if (!alg)	/* The key isn't encrypted; just read it in */
		return 1;

	if (alg == 255) {
		/* New style, with a separate string-to-key */

		if (len == 1)
			return kPGPError_KeyPacketTruncated;
		alg = buf[1];
		alglen = pgpS2Kdecode(&s2k, env, buf+2, len-2);
		if (alglen < 0)
			return alglen;
		alglen += 2;
		if (len < (unsigned)alglen)
			return kPGPError_KeyPacketTruncated;
	} else {
		/* Old-style string-to-key */
		s2k = pgpS2Ksimple(env, pgpHashByNumber(kPGPHashAlgorithm_MD5));
		if (!s2k)
			return kPGPError_OutOfMemory;
		alglen = 1;
	}
	/* Okay now, do the conversion */
	cipher = pgpCipherGetVTBL( (PGPCipherAlgorithm)alg);
	if (!cipher) {
		pgpS2Kdestroy(s2k);
		return kPGPError_BadCipherNumber;
	}
	if (len < alglen + cipher->blocksize) {
		pgpS2Kdestroy(s2k);
		return kPGPError_KeyPacketTruncated;
	}
	*cfbp = pgpCFBCreate( PGPGetContextMemoryMgr( context ), cipher);
	if (!*cfbp) {
		pgpS2Kdestroy(s2k);
		return kPGPError_OutOfMemory;
	}
	pgpAssert(cipher->keysize <= sizeof(key));
	if (hashedPhrase) {
		pgpCopyMemory (phrase, key, plen);
	} else {
		pgpStringToKey(s2k, phrase, plen, key, cipher->keysize);
	}
	PGPInitCFB(*cfbp, key, buf + alglen);
	pgpClearMemory( key,  sizeof(key));
	pgpS2Kdestroy(s2k);

	return alglen + cipher->blocksize;
}
