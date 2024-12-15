/*
 * X509 Key importing
 *
 * Copyright (C) 1996,1997,1998 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpX509Keys.c,v 1.15.2.1.2.1 1998/11/12 03:22:07 heller Exp $
 */

#include "pgpConfig.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>
#include <ctype.h>

#include "pgpKDBInt.h"
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpRngPub.h"
#include "pgpRngPriv.h"
#include "pgpRngRead.h"
#include "pgpContext.h"

#include "pgpX509Priv.h"
#include "pgpHashPriv.h"
#include "pgpPubKey.h"
#include "pgpOptionListPriv.h"
#include "pgpPktByte.h"
#include "pgpPipeline.h"
#include "pgpFileMod.h"
#include "pgpEncodePriv.h"
#include "pgpVMemMod.h"
#include "pgpAnnotate.h"
#include "pgpSigSpec.h"
#include "pgpTimeDate.h"

/* Whether or not to use short forms for X509 name conversions */
#ifndef PGP_X509SHORTNAMES
#define PGP_X509SHORTNAMES 1
#endif


/* Error returns */
#define k509Error_NeededCertNotAvailable			-10
#define k509Error_SelfSignedCert					-20
#define k509Error_InvalidCertificateSignature		-30
#define k509Error_InvalidCertificateFormat			-40


/* Size limit on cert */
#define MAX_509_CERT_SIZE		65536

/* Commonly used tag values */
#define TAG_ZERO		0
#define TAG_INTEGER		2
#define TAG_BITSTRING	3
#define TAG_OCTETSTRING	4
#define TAG_NULL		5
#define TAG_OBJECT		6
#define TAG_SEQUENCE	16
#define	TAG_SET			17
#define TAG_PRINTABLE	19
#define TAG_T61STRING	20
#define TAG_IA5STRING	22
#define TAG_UTCTIME		23


#define elemsof(arr)  (sizeof(arr)/sizeof(arr[0]))

#define EXPECT(t, v)	if (t != v) { \
							err = kPGPError_X509InvalidCertificateFormat; \
							goto error; \
						}

/* RSA OID is (1, 2, 840, 113549, 1, 1, 1) */
static PGPByte const rsaoid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x01};
/* RSA with MD5 OID is (1, 2, 840, 113549, 1, 1, 4) */
static PGPByte const rsamd5oid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x04};
/* RSA with MD2 OID is (1, 2, 840, 113549, 1, 1, 2) */
static PGPByte const rsamd2oid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x02};
/* DSA OID is (1, 2, 840, 10040, 4, 1) */
static PGPByte const dsaoid[] = {0x2a, 0x86, 0x48, 0xce, 0x38, 0x04, 0x01};
/* DSA with SHA-1 OID is (1, 2, 840, 10040, 4, 3) */
static PGPByte const dsashaoid[] = {0x2a, 0x86, 0x48, 0xce, 0x38, 0x04, 0x03};

/* ElG OID is (1, 2, 840, 10046, 2, 1) */
static PGPByte const elgoid[] = {0x2a, 0x86, 0x48, 0xce, 0x3e, 0x02, 0x01};

/* CN (common-name) OID is (2, 5, 4, 3) */
static PGPByte const cnoid[] = {0x55, 0x04, 0x03};
/* RSA's OID for email address in RDN's: (1, 2, 840, 113549, 1, 9, 1) */
static PGPByte const emailoid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
									0x09, 0x01};
static const char emailname[] = "EMAIL";


/* Table to convert DN field OID's to letter codes per RFC2253 */
/* Indexed by the last value in a (2, 5, 4, X) OID */
static const char *dnnames[] = {
	NULL, NULL, NULL,
	"CN",			/* 3 == commonName */
	NULL, NULL,
	"C",			/* 6 == countryName */
	"L",			/* 7 == localityName */
	"ST",			/* 8 == stateOrProvinceName */
	"STREET",		/* 9 == streetAddress */
	"O",			/* 10 == organizationName */
	"OU",			/* 11 == organizationalUnitName */
};


#if PRINTDEBUG509
/* Used for debugging, to print out the tags as we see them */
static struct {
	int tag;
	char *name;
} const tagnames[] = {
	{TAG_INTEGER,		"Integer"},
	{TAG_BITSTRING,		"Bitstring"},
	{TAG_OCTETSTRING,	"Octetstring"},
	{TAG_NULL,			"Null"},
	{TAG_OBJECT,		"Object"},
	{TAG_SEQUENCE,		"Sequence"},
	{TAG_SET,			"Set"},
	{TAG_PRINTABLE,		"Printablestring"},
	{TAG_T61STRING,		"T61string"},
	{TAG_IA5STRING,		"IA5string"},
	{TAG_UTCTIME,		"UTCTime"}
};
#endif



#if PRINTDEBUG509
static void
printtagname (int tag)
{
	int i;
	int printed = 0;
	printf ("0x%02x", tag);
	for (i=0; i<sizeof(tagnames)/sizeof(tagnames[0]); ++i) {
		if (tagnames[i].tag == tag) {
			printf (" (%s)", tagnames[i].name);
			printed = 1;
			break;
		}
	}
	if (!printed)
		printf ("(<Unknown>)");
}
#endif

/* Return bit number of highest bit in byte */
static int
hibit(int n)
{
	int bit = 7;
	int val = 0x80;

	if (n == 0)
		return -1;
	while ((val & n) == 0) {
		val >>= 1;
		--bit;
	}
	return bit;
}


/* Read start of an X.509 object */
static int
dotaglen(PGPByte **buf, PGPSize *length)
{
	PGPByte tag = *(*buf)++ & 0x1f;
	PGPSize len = *(*buf)++;
	if (len & 0x80) {
		PGPByte lenlen = len & 0x7f;
		len = 0;
		while (lenlen--) {
			len <<= 8;
			len |= *(*buf)++;
		}
	}
	*length = len;
#if PRINTDEBUG509
printf ("tag ");
printtagname(tag);
printf (", length 0x%x\n", len);
#endif
	return tag;
}

/* Read a three byte value and skip past it */
static int
dolen3(PGPByte **buf)
{
	int len;
	len = *(*buf)++;
	len <<= 8;
	len |= *(*buf)++;
	len <<= 8;
	len |= *(*buf)++;
	return len;
}


/*
 * Given an X509 integer (pointing past the tag/length), return a PGP
 * format MPI number.  If the mpbuf pointer is NULL, just return the len.
 * If non-NULL, *mplen must equal the required length.
 */
static PGPError
sDecodeX509Integer (PGPByte const *buf, PGPSize len,
		PGPByte *mpbuf, PGPSize *mplen)
{
	PGPUInt32 bits;
	
	/* Skip past any zero padding */
	while (*buf == '\0' && len > 0) {
		++buf;
		--len;
	}
	
	/* Must be at least 1 byte long */
	if (len == 0)
		return kPGPError_X509InvalidCertificateFormat;

	/* Set MPI with bitcount prefix */
	if( IsNull( mpbuf ) ) {
		*mplen = len + 2;
		return kPGPError_NoErr;
	}

	pgpAssert (len + 2 == *mplen);
		
	bits = (len-1) * 8 + hibit(*buf) + 1;
	mpbuf[0] = (PGPByte)(bits >> 8);
	mpbuf[1] = (PGPByte)(bits >> 0);
	pgpCopyMemory( buf, mpbuf+2, len);

	return kPGPError_NoErr;
}


/*
 * Decode the given UTCTime buffer into a standard PGP timestamp
 */
static PGPError
sDecodeUTCTime (PGPByte const *buf, PGPSize len, PGPUInt32 *timestamp)
{
	PGPUInt32 datefield[6];		/* year, month, day, hour, min, second */
	PGPUInt32 i;
	PGPUInt32 tstamp;
	PGPError err = kPGPError_NoErr;

	pgpAssert( IsntNull( timestamp ) );
	*timestamp = 0;

	pgpClearMemory (datefield, sizeof(datefield));

	for (i=0; i<elemsof(datefield); ++i) {
		if (len < 2 || !isdigit(buf[0]))
			break;
		if (!isdigit(buf[1])) {
			err = kPGPError_X509InvalidCertificateFormat;
			goto error;
		}
		datefield[i] = (buf[0]-'0') * 10 + (buf[1] - '0');
		buf += 2;
		len -= 2;
	}
	if (len != 1  ||  buf[0] != 'Z'  ||  i < 5) {
		err = kPGPError_X509InvalidCertificateFormat;
		goto error;
	}
	
	/* Correct for Y2K problem */
	datefield[0] += (datefield[0] > 50) ? 1900 : 2000;

	/* Get seconds since Jan 1, 1970 */
	tstamp = pgpDateFromYMD (datefield[0], datefield[1], datefield[2]);
	tstamp = (((tstamp*24 + datefield[3]) * 60) + datefield[4]) * 60
		+ datefield[5];

	*timestamp = tstamp;

error:

	return err;
}


/* 
 * TRUE if given DN string needs quoting per RFC1779.
 * Also return the number of internal backslashes needed.
 */
static PGPBoolean
sNeedsDNQuoting(PGPByte const *buf, PGPSize length, PGPUInt32 *nslashes)
{
	PGPByte c='\0', prevc='\0';
	PGPUInt32 nslash = 0;
	PGPBoolean outerquote = FALSE;

	if (*buf==' ')
		outerquote = TRUE;
	while (length--) {
		prevc = c;
		c = *buf++;
		if (c==',' || c=='=' || c=='\r' || c=='+' || c=='<'
			|| c=='>' || c=='#' || c==';')
			outerquote = TRUE;
		if (c==' ' && prevc==' ')
			outerquote = TRUE;
		if (c=='"' || c=='\\')
			++nslash;
	}
	if (prevc==' ')
		outerquote = TRUE;
	if (IsntNull (nslashes) )
		*nslashes = nslash;
	return outerquote;
}


/*
 * If out is non-NULL, apply RFC1779 DNS quoting to buf, outputting to out.
 * In either case, return the length of what the output string would be.
 * If the rev flag is set, out points at the end of the output buffer rather
 * than at the beginning.
 */
static PGPUInt32
sDNQuoteString(PGPByte const *buf, PGPSize len, PGPByte *out, PGPBoolean rev)
{
	PGPUInt32 outlen;
	PGPBoolean needouterquote;
	PGPByte c;

	needouterquote = sNeedsDNQuoting( buf, len, &outlen);
	outlen += len + (needouterquote ? 2 : 0);
	if (IsNull( out ))
		return outlen;
	if (rev)
		out -= outlen;
	if (needouterquote)
		*out++ = '"';
	while (len--) {
		c = *buf++;
		if (c=='"' || c=='\\')
			*out++ = '\\';
		*out++ = c;
	}
	if (needouterquote)
		*out++ = '"';
	return outlen;
}


/*
 * Called pointing at the OID for an algorithm identifier, see if it is one
 * we know.
 */
static PGPError
sDecodeAlgorithmOID (PGPByte const *buf, PGPSize len,
	PGPPublicKeyAlgorithm *keyalgorithm, PGPHashAlgorithm *hashalgorithm)
{
	PGPError err = kPGPError_NoErr;

	if( IsntNull( keyalgorithm ) )
		*keyalgorithm = (PGPPublicKeyAlgorithm) 0;

	if( IsntNull( hashalgorithm ) )
		*hashalgorithm = (PGPHashAlgorithm) 0;

	if (len == sizeof(rsaoid) && memcmp (buf, rsaoid, len) == 0) {
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_RSA;
	} else if (len == sizeof(rsamd5oid) && memcmp (buf, rsamd5oid, len) == 0) {
		if( IsntNull( hashalgorithm ) )
			*hashalgorithm = kPGPHashAlgorithm_MD5;
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_RSA;
	} else if (len == sizeof(rsamd2oid) && memcmp (buf, rsamd2oid, len) == 0) {
		if( IsntNull( hashalgorithm ) )
			*hashalgorithm = (PGPHashAlgorithm) kPGPHashAlgorithm_MD2;
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_RSA;
	} else if (len == sizeof(dsaoid) && memcmp (buf, dsaoid, len) == 0) {
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_DSA;
	} else if (len == sizeof(dsashaoid) && memcmp (buf, dsashaoid, len) == 0) {
		if( IsntNull( hashalgorithm ) )
			*hashalgorithm = kPGPHashAlgorithm_SHA;
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_DSA;
	} else if (len == sizeof(elgoid) && memcmp (buf, elgoid, len) == 0) {
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_ElGamal;
	} else {
		err = kPGPError_X509InvalidCertificateFormat;
	}
	return err;
}

/*
 * Call this after reading the tag and length for the SEQUENCE which holds
 * the validity period
 * If vbuf is NULL, set *vlength to the necessary length.  Else
 * store the data into vbuf, and treat *vlength as an input parameter
 * telling what the length needs to be.
 * We create a pair of PGP signature subpackets expressing creation and
 * expiration dates.
 */
static PGPError
sDecodeValidity(PGPByte *buf, PGPSize length, PGPByte *vbuf,
	 PGPSize *vlength)
{
	PGPSize len;
	PGPByte tag;
	PGPUInt32 creationtime;
	PGPUInt32 expirationtime;
	PGPUInt32 o;
	PGPError err;

	(void) length;
	pgpAssert( IsntNull( vlength ) );

	/* Need to enhance to handle GENERALIZEDTIME */

	/* Should be notbefore UTCTIME */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_UTCTIME);

	err = sDecodeUTCTime (buf, len, &creationtime);
	if( IsPGPError( err ) )
		goto error;
	buf += len;

	/* Should be notafter UTCTIME */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_UTCTIME);

	err = sDecodeUTCTime (buf, len, &expirationtime);
	if( IsPGPError( err ) )
		goto error;
	buf += len;

	if (expirationtime < creationtime) {
		err = kPGPError_X509InvalidCertificateFormat;
		goto error;
	}
	expirationtime -= creationtime;

	/* Two subpackets, each with 1 byte of len, 1 byte of type, 4 of time */
	if( IsNull( vbuf ) ) {
		*vlength = 2 * 6;
		return kPGPError_NoErr;
	}

	pgpAssert (*vlength == 2 * 6);

	o = 0;
	vbuf[o++] = 5;	/* length */
	vbuf[o++] = SIGSUB_CREATION;
	vbuf[o++] = (PGPByte)(creationtime >> 24);
	vbuf[o++] = (PGPByte)(creationtime >> 16);
	vbuf[o++] = (PGPByte)(creationtime >>  8);
	vbuf[o++] = (PGPByte)(creationtime >>  0);

	vbuf[o++] = 5;	/* length */
	vbuf[o++] = SIGSUB_EXPIRATION;
	vbuf[o++] = (PGPByte)(expirationtime >> 24);
	vbuf[o++] = (PGPByte)(expirationtime >> 16);
	vbuf[o++] = (PGPByte)(expirationtime >>  8);
	vbuf[o++] = (PGPByte)(expirationtime >>  0);

	pgpAssert (o == *vlength);

error:

	return err;
}


/*
 * Call this pointing AT the start of the cert to
 * create a PGP signature packet which embodies the cert.
 * If sigbuf is NULL, set *siglength to the necessary length.  Else
 * store the data into sigbuf, and treat *siglength as an input parameter
 * telling what the length needs to be.
 */
static PGPError
sDecodeSig(PGPByte *buf, PGPByte *sigBuf, PGPSize *sigLength)
{
	PGPByte *buf_validity;
	PGPSize len_validity;
	PGPByte *certbuf;
	PGPSize certlen;
	PGPByte *buf2;
	PGPUInt32 tag;
	PGPSize siglen;
	PGPSize extralen;
	PGPSize certlenlen;
	PGPSize validitylen;
	PGPSize len;
	PGPUInt32 o;
	PGPPublicKeyAlgorithm sigalg;
	PGPHashAlgorithm hashalg;
	PGPError err = kPGPError_NoErr;

	pgpAssert( IsntNull( sigLength ) );
	if( IsNull( sigBuf ) )
		*sigLength = 0;

	/* Save pointers to cert buf and len */
	certbuf = buf;

	/* Should be the overall certificate SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	certlen = len + buf - certbuf;

	/* Should be certificate info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Save pointer to signature algorithm identifier after cert info */
	buf2 = buf + len;

	/* Parse cert info to get validity period */

	/* May be X509 version number */
	tag = dotaglen(&buf, &len);
	if (tag == TAG_ZERO) {
		buf += len;
		tag = dotaglen(&buf, &len);
	}

	/* Should be serial number INTEGER */
	EXPECT(tag, TAG_INTEGER);
	buf += len;

	/* Should be signature algorithm identifier SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be issuer name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be validity SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf_validity = buf;
	len_validity = len;
	err = sDecodeValidity (buf, len, NULL, &validitylen);
	if (IsPGPError(err))
		goto error;

	/* Get type of signature */
	buf = buf2;

	/* Should be signature algorithm identifier SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Should be signature algorithm OBJECT */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_OBJECT);

	err = sDecodeAlgorithmOID (buf, len, &sigalg, &hashalg);
	if( IsPGPError(err) )
		goto error;
	buf += len;

	/* Calculate length of signature */
	certlenlen = (certlen+1 < 192) ? 1 : 2;
	extralen = validitylen + certlen + 1 + certlenlen;
	siglen = 8 + extralen + 2 + 3;

	if( IsNull( sigBuf ) ) {
		*sigLength = siglen;
		return kPGPError_NoErr;
	}

	pgpAssert (*sigLength = siglen);

	/* Lay out signature packet */
	
	o = 0;
	sigBuf[o++] = PGPVERSION_4;
	sigBuf[o++] = PGP_SIGTYPE_KEY_GENERIC;
	sigBuf[o++] = 0;		/* Use sigalgorithm of 0 to show nonstandard */
	sigBuf[o++] = hashalg;	/* But use true hash algorithm */

	sigBuf[o++] = (PGPByte) (extralen >> 8);
	sigBuf[o++] = (PGPByte) (extralen >> 0);
	err = sDecodeValidity (buf_validity, len_validity,
						   sigBuf + o, &validitylen);
	o += validitylen;

	if (certlenlen == 1)
		sigBuf[o++] = certlen + 1;
	else {
		sigBuf[o++] = (PGPByte) (192 + ((certlen+1-192) >> 8));
		sigBuf[o++] = (PGPByte) (certlen+1-192);
	}
	sigBuf[o++] = SIGSUB_X509;
	pgpCopyMemory (certbuf, sigBuf+o, certlen);
	o += certlen;

	/* No non-hashed data (yet) */
	sigBuf[o++] = 0;
	sigBuf[o++] = 0;

	/* End up with dummy 2-byte checksum and dummy MPI for old parsers */
	sigBuf[o++] = 0;
	sigBuf[o++] = 0;
	sigBuf[o++] = 0;
	sigBuf[o++] = 1;
	sigBuf[o++] = 1;

	pgpAssert (o == siglen);

error:
	return err;
}


/*
 * Call this after reading the tag and length for the SEQUENCE which holds
 * the distinguished name.
 * If namebuf is NULL, set *namelength to the necessary length.  Else
 * store the data into namebuf, and treat *namelength as an input parameter
 * telling what the length needs to be.
 * This versions uses long form for X509 names, in RFC2253 format.
 */
static PGPError
sDecodeNameLong(PGPByte *buf, PGPSize length, PGPByte *nameBuf,
	 PGPSize *nameLength)
{
	PGPByte *endname = buf + length;
	PGPSize len;
	PGPByte tag;
	PGPBoolean sizingPass;
	PGPBoolean seenName = FALSE;
	PGPError err;
	char const *oidName;
	PGPUInt32 lNameLength;
	PGPUInt32 oidLength = 0;
	static char const dfltname[] = "(Unknown X509 name)";

	sizingPass = (nameBuf==NULL);
	lNameLength = sizingPass ? 0 : *nameLength;

	while (buf < endname) {
		/* Should be SET */
		tag = dotaglen (&buf, &len);
		EXPECT(tag, TAG_SET);

		/* Set just holds a SEQUENCE */
		tag = dotaglen (&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* Sequence starts with an OID */
		tag = dotaglen (&buf, &len);
		EXPECT(tag, TAG_OBJECT);

		oidName = NULL;
		if (len == 3 &&
			(buf)[0] == cnoid[0] && (buf)[1] == cnoid[1] &&
			(buf)[2] < sizeof(dnnames)/sizeof(dnnames[0])) {
			oidName = dnnames[(buf)[2]];
			oidLength = strlen(oidName);
		}
		else if (len == sizeof(emailoid)
				 && memcmp(buf, emailoid, len)==0 ) {
			oidName = emailname;
			oidLength = strlen(oidName);
		}
		buf += len;

		/* Next comes the printablestring (may be variants like IA5) */
		tag = dotaglen (&buf, &len);
		/* EXPECT(tag, TAG_PRINTABLE); */

		if( IsntNull( oidName ) ) {
			seenName = TRUE;
			if (sizingPass) {
				/* add 3 for equals, comma, space */
				lNameLength += strlen(oidName) + 3;
				lNameLength += sDNQuoteString(buf, len, NULL, TRUE);
			} else {
				/* Build name string in reverse order, per LDAP */
				lNameLength -= sDNQuoteString(buf, len,
											 nameBuf+lNameLength, TRUE);
				nameBuf[--lNameLength] = '=';
				lNameLength -= oidLength;
				pgpCopyMemory( oidName, nameBuf+lNameLength, oidLength );
				if( lNameLength > 0 ) {
					nameBuf[--lNameLength] = ' ';
					nameBuf[--lNameLength] = ',';
				}
			}
		}
		buf += len;
	}
	if( sizingPass ) {
		if (lNameLength == 0) {
			/* No name fields we recognized */
			*nameLength = sizeof(dfltname);
		} else {
			lNameLength -= 2;	/* Counted an initial ", " */
			*nameLength = lNameLength;
		}
	} else {
		/* lNameLength should have counted down to zero */
		if( !seenName ) {
			pgpAssert (lNameLength == sizeof(dfltname));
			pgpCopyMemory (dfltname, nameBuf, lNameLength);
		} else {
			pgpAssert (lNameLength == 0);
		}
	}

	return kPGPError_NoErr;

error:
	return err;
}



/*
 * Call this after reading the tag and length for the SEQUENCE which holds
 * the distinguished name.
 * If namebuf is NULL, set *namelength to the necessary length.  Else
 * store the data into namebuf, and treat *namelength as an input parameter
 * telling what the length needs to be.
 * This version uses short form for X509 names, in PGP format.
 * E.g. just "John Doe <john@doe.com>".
 */
static PGPError
sDecodeName(PGPByte *buf, PGPSize length, PGPByte *nameBuf,
	 PGPSize *nameLength)
{
#if !PGP_X509SHORTNAMES

	return sDecodeNameLong(buf, length, nameBuf, nameLength);

#else /* PGP_X509SHORTNAMES */

	PGPByte *endname = buf + length;
	PGPByte *inibuf = buf;
	PGPSize len;
	PGPByte tag;
	PGPError err;
	PGPBoolean sizingPass;
	PGPBoolean seenName = FALSE;
	PGPBoolean seenEmail = FALSE;
	PGPUInt32 lNameLength;

	sizingPass = (nameBuf==NULL);
	lNameLength = sizingPass ? 0 : *nameLength;

	while (buf < endname) {
		/* Should be SET */
		tag = dotaglen (&buf, &len);
		EXPECT(tag, TAG_SET);

		/* Set just holds a SEQUENCE */
		tag = dotaglen (&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* Sequence starts with an OID */
		tag = dotaglen (&buf, &len);
		EXPECT(tag, TAG_OBJECT);

		if (len == sizeof(cnoid)
				&& memcmp(buf, cnoid, len)==0 ) {
			/* Next comes the printablestring (may be variants like IA5) */
			buf += len;
			tag = dotaglen (&buf, &len);
			/* EXPECT(tag, TAG_PRINTABLE); */

			seenName = TRUE;
			if (sizingPass) {
				lNameLength += len;
			} else {
				/* Put common name at beginning of buffer */
				pgpAssert (lNameLength >= len);
				pgpCopyMemory (buf, nameBuf, len);
				/* Add a space if needed */
				if (lNameLength > len)
					nameBuf[len] = ' ';
			}
		}
		else if (len == sizeof(emailoid)
				 && memcmp(buf, emailoid, len)==0 ) {
			/* Next comes the printablestring (may be variants like IA5) */
			buf += len;
			tag = dotaglen (&buf, &len);
			/* EXPECT(tag, TAG_PRINTABLE); */

			seenEmail = TRUE;
			if (sizingPass) {
				/* Add 2 for angle brackets */
				lNameLength += len + 2;
			} else {
				/* Put email address at end of buffer */
				pgpAssert (lNameLength >= len+2);
				nameBuf[lNameLength-1] = '>';
				pgpCopyMemory (buf, nameBuf+lNameLength-1-len, len);
				nameBuf[lNameLength-len-2] = '<';
			}
		} else {
			/* Some other field of the DN which we are ignoring */
			buf += len;
			tag = dotaglen (&buf, &len);
		}
		buf += len;
	}

	if (!seenName && !seenEmail) {
		/* If we could not parse the name, use the long form */
		return sDecodeNameLong(inibuf, length, nameBuf, nameLength);
	}

	if( sizingPass ) {
		/* Count a space between name and email if have both */
		if (seenName && seenEmail)
			++lNameLength;
		*nameLength = lNameLength;
	}

	return kPGPError_NoErr;

error:
	return err;

#endif /* PGP_X509SHORTNAMES */
}



/*
 * Call this after reading the tag and length for the SEQUENCE which holds
 * the subjectpublickeyinfo.
 * If keybuf is NULL, set *keylength to the necessary length.  Else
 * store the data into keybuf, and treat *keylength as an input parameter
 * telling what the length needs to be.
 */
static PGPError
sDecodeKey(PGPByte *buf, PGPSize length, PGPByte *keyBuf, PGPSize *keyLength)
{
	PGPUInt32 tag;
	PGPByte *buf2;
	PGPSize len;
	PGPPublicKeyAlgorithm keyalgorithm;
	PGPByte *bnbuf[4];		/* buf offset for start of X.509 bignum */
	PGPSize bnlen[4];		/* buf length for X.509 bignum */
	PGPSize bnlength[4];	/* length of PGP encoded bignum */
	PGPUInt32 i, o;
	PGPSize keybuflength;
	PGPError err = kPGPError_NoErr;

	(void) length;

	/* Init bignum arrays */
	pgpClearMemory (bnbuf, sizeof(bnbuf));
	pgpClearMemory (bnlen, sizeof(bnlen));
	pgpClearMemory (bnlength, sizeof(bnlength));

	/* Should be algorithm identifier for key data SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf2 = buf + len;

	/* Go within algorithm identifier to determine key type */

	/* Should be object identifier for RSA/DSS/ElGamal */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_OBJECT);
	err = sDecodeAlgorithmOID (buf, len, &keyalgorithm, NULL);
	if( IsPGPError(err) )
		goto error;
	buf += len;

	if (keyalgorithm == kPGPPublicKeyAlgorithm_DSA
			|| keyalgorithm == kPGPPublicKeyAlgorithm_ElGamal) {
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* Should be prime p INTEGER */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_INTEGER);
		
		bnbuf[0] = buf;
		bnlen[0] = len;
		err = sDecodeX509Integer (buf, len, NULL, &bnlength[0]);
		if( IsPGPError( err ) )
			goto error;
		buf += len;

		/* Should be generator g INTEGER if ElG, or subprime q for DSA */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_INTEGER);
		
		bnbuf[1] = buf;
		bnlen[1] = len;
		err = sDecodeX509Integer (buf, len, NULL, &bnlength[1]);
		if( IsPGPError( err ) )
			goto error;
		buf += len;

		if (keyalgorithm == kPGPPublicKeyAlgorithm_DSA) {
			/* Should be generator g INTEGER */
			tag = dotaglen(&buf, &len);
			EXPECT(tag, TAG_INTEGER);

			bnbuf[2] = buf;
			bnlen[2] = len;
			err = sDecodeX509Integer (buf, len, NULL, &bnlength[2]);
			if( IsPGPError( err ) )
				goto error;
			buf += len;
		}
	} else /* RSA */ {
		/* Ignore any RSA parameters */
	}

	/* Back up to subject public key info */
	buf = buf2;

	/* Should be key data as an encoded BITSTRING */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_BITSTRING);
	
	/* Go within bitstring and decode the public key data */

	/* Bit string should be multiple of 8 bits, have a zero at the top */
	EXPECT(*buf, 0);
	++buf;
	--len;

	if (keyalgorithm == kPGPPublicKeyAlgorithm_DSA) {
		/* Should be public y value INTEGER */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_INTEGER);

		bnbuf[3] = buf;
		bnlen[3] = len;
		err = sDecodeX509Integer (buf, len, NULL, &bnlength[3]);
		if( IsPGPError( err ) )
			goto error;
		buf += len;

	} else if( keyalgorithm == kPGPPublicKeyAlgorithm_ElGamal) {
		/* Should be public y value INTEGER */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_INTEGER);

		bnbuf[2] = buf;
		bnlen[2] = len;
		err = sDecodeX509Integer (buf, len, NULL, &bnlength[2]);
		if( IsPGPError( err ) )
			goto error;
		buf += len;

	} else /* RSA */ {
		/* Should be RSA public key SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* Go within RSA public key */

		/* Should be modulus INTEGER */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_INTEGER);

		bnbuf[0] = buf;
		bnlen[0] = len;
		err = sDecodeX509Integer (buf, len, NULL, &bnlength[0]);
		if( IsPGPError( err ) )
			goto error;
		buf += len;

		/* Should be exponent INTEGER */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_INTEGER);

		bnbuf[1] = buf;
		bnlen[1] = len;
		err = sDecodeX509Integer (buf, len, NULL, &bnlength[1]);
		if( IsPGPError( err ) )
			goto error;
		buf += len;
	}

	keybuflength = 6; /* bytes preceding bignums */

	for (i=0; i<elemsof(bnlength); ++i) {
		if( bnlength[i] == 0 )
			break;
		keybuflength += bnlength[i];
	}

	if( IsNull( keyBuf ) ) {
		*keyLength = keybuflength;
	} else {
		pgpAssert (*keyLength == keybuflength);
		/* public key packet */
		o = 0;
		keyBuf[o++] = PGPVERSION_4;		/* Version */
		/* 1-4 are timestamp */
		while (o < 5)
			keyBuf[o++] = 0;
		keyBuf[o++] = keyalgorithm;
		for (i=0; i<elemsof(bnlength); ++i) {
			if( bnlength[i] == 0 )
				break;
			err = sDecodeX509Integer (bnbuf[i], bnlen[i],
									  keyBuf+o, &bnlength[i]);
			if( IsPGPError( err ) )
				goto error;
			o += bnlength[i];
		}
		pgpAssert (o == keybuflength);
	}

error:

	return err;
}


/*
 * Given an X509 cert, return a PGPKeySetRef containing a simple PGP key
 * with the same key material as the original cert.  The userid on the
 * key is one of the 509 subject distinguished name fields, the common name
 * if it exists, else one of the other fields.
 */
static PGPError
sDecode509(PGPByte *buf, PGPContextRef context, PGPKeySetRef *kset)
{
	PGPByte tag;
	PGPSize len;
	PGPByte *pgpkeybuf = NULL;
	PGPByte *buf_name, *buf_key, *buf_sig;
	PGPSize len_name, len_key;
	PGPSize keylen;
	PGPSize namelen;
	PGPSize siglen;
	PGPSize pgpkeylen;
	PGPUInt32 o;
	PGPKeySetRef keyset;
	PGPError err;

	*kset = NULL;

	/* Ignore errors from sig, we may just leave it off */
	buf_sig = buf;
	(void)sDecodeSig (buf, NULL, &siglen);

	/* Should be the overall certificate SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Should be certificate info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* May be X509 version number */
	tag = dotaglen(&buf, &len);
	if (tag == TAG_ZERO) {
		buf += len;
		tag = dotaglen(&buf, &len);
	}

	/* Should be serial number INTEGER */
	EXPECT(tag, TAG_INTEGER);
	buf += len;

	/* Should be signature algorithm identifier SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be issuer name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
#if 0
	/* For debugging, verify issuer name */
	err = sDecodeName (buf, len, NULL, &namelen);
	if (IsPGPError(err))
		goto error;
	namebuf = (PGPByte *) pgpContextMemAlloc( context, namelen, 0);
	if (IsNull(namebuf)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	err = sDecodeName (buf, len, namebuf, &namelen);
	if (IsPGPError(err))
		goto error;
	/* Peek at it here if debugging, then trash it */
	pgpContextMemFree( context, namebuf );
	namebuf = NULL;
#else
	buf += len;
#endif

	/* Should be validity SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be subject name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Decode subject name */
	buf_name = buf;
	len_name = len;
	err = sDecodeName (buf, len, NULL, &namelen);
	if (IsPGPError(err))
		goto error;
	buf += len;

	/* Should be subject public key info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Decode subject public key info */
	buf_key = buf;
	len_key = len;
	err = sDecodeKey (buf, len, NULL, &keylen);
	if (IsPGPError(err))
		goto error;

	/* Now create the PGP key */

	pgpkeylen = 3 + keylen + 3 + namelen + ((siglen>0)?(3+siglen):0);
	pgpkeybuf = (PGPByte *) pgpContextMemAlloc( context, pgpkeylen, 0);
	if (IsNull(pgpkeybuf)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	/* public key packet */
	o = 0;
	pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1),
	pgpkeybuf[o++] = (keylen >> 8) & 0xff;
	pgpkeybuf[o++] = (keylen >> 0) & 0xff;
	err = sDecodeKey (buf_key, len_key, pgpkeybuf + o, &keylen);
	if (IsPGPError(err))
		goto error;
	o += keylen;

	/* userid packet */
	pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_NAME, 1);
	pgpkeybuf[o++] = (namelen >> 8) & 0xff;
	pgpkeybuf[o++] = (namelen >> 0) & 0xff;
	err = sDecodeName (buf_name, len_name, pgpkeybuf + o, &namelen);
	if (IsPGPError(err))
		goto error;
	o += namelen;

	/* sig packet */
	if( siglen > 0 ) {
		pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_SIG, 1);
		pgpkeybuf[o++] = (siglen >> 8) & 0xff;
		pgpkeybuf[o++] = (siglen >> 0) & 0xff;
		err = sDecodeSig (buf_sig, pgpkeybuf + o, &siglen);
		if (IsPGPError(err))
			goto error;
		o += siglen;
	}

	pgpAssert (o == pgpkeylen);

	err = pgpImportKeyBinary( context, pgpkeybuf, pgpkeylen, &keyset );
	if( IsPGPError( err ) )
		goto error;

	*kset = keyset;
	/* Fall through */
error:

	if( IsntNull(pgpkeybuf) )
		pgpContextMemFree( context, pgpkeybuf );
	return err;
}


/* Entry point to import a 509 cert from the specified location */
	PGPError
pgpImportX509Certificate( PGPContextRef context, PGPKeySetRef *keys,
	PGPOptionListRef optionList)
{
	PGPError		err;
	PGPPipeline	   *head = NULL;
	PGPFileRead		*pfrin;
	PFLConstFileSpecRef	fileRef;
	PGPByte		   *bufPtr;
	PGPSize			bufLength;
	PGPFileDataType	inFileDataType;
	
	err = pgpSetupInput( context, optionList, NULL, NULL, FALSE, FALSE,
						 &fileRef, &pfrin, &inFileDataType, &bufPtr, &bufLength );
	if( IsPGPError( err ) )
		goto error;

	if( IsntNull( pfrin ) ) {
		/* Read whole file into memory buffer */
		pgpVariableMemModCreate( context, &head, MAX_509_CERT_SIZE );
		if( IsNull( head ) ) {
			pgpFileReadDestroy( pfrin );
			return kPGPError_OutOfMemory;
		}
		err = pgpFileReadPump( pfrin, head );
		pgpFileReadDestroy( pfrin );
		if( IsPGPError( err ) ) {
			head->teardown( head );
			return err;
		}
		err = head->annotate( head, NULL, PGPANN_MEM_BUFSIZE,
							  (PGPByte *)&bufLength, sizeof( bufLength ) );
		if( IsPGPError( err ) ) {
			head->teardown( head );
			return err;
		}
		
		err = head->annotate( head, NULL, PGPANN_MEM_MEMBUF,
							  (PGPByte *)&bufPtr, sizeof( bufPtr ) );
		head->teardown( head );
		if( IsPGPError( err ) ) {
			return err;
		}
	}

	/* Now data is in bufPtr, of length bufLength */
	err = sDecode509( bufPtr, context, keys);
	if( IsntNull( pfrin ) ) {
		PGPFreeData( bufPtr );
	}

error:
	return err;
}


/* Given a distinguished name (e.g. an issuer name from a cert) search
 * the certs list and try to find a cert which has the given name as a
 * subject.  Return the cert length and a pointer to the start of the cert
 * (to the SEQUENCE which is the whole cert).  Return 0 if no cert matches.
 */
static PGPError
sLookup509 (PGPByte *certs, PGPByte *dname, PGPByte **rbuf, PGPUInt32 *rlen)
{
	PGPUInt32	totallen;
	PGPUInt32	certlen;
	PGPSize		len;
	PGPByte		tag;
	PGPSize		dnamelen;
	PGPByte		*buf;
	PGPByte		*subjname;
	PGPError	err;

	*rbuf = NULL;
	*rlen = 0;

	/* Get length of dname */
	buf = dname;
	tag = dotaglen(&buf, &dnamelen);
	EXPECT(tag, TAG_SEQUENCE);
	dnamelen += buf - dname;

	totallen = dolen3 (&certs);
	while (totallen > 0) {
		certlen = dolen3 (&certs);
		
		/* Parse down to find subject name */
		buf = certs;

		/* Should be the overall certificate SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* Should be certificate info SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* May be X509 version number */
		tag = dotaglen(&buf, &len);
		if (tag == TAG_ZERO) {
			buf += len;
			tag = dotaglen(&buf, &len);
		}

		/* Should be serial number INTEGER */
		EXPECT(tag, TAG_INTEGER);
		buf += len;

		/* Should be signature algorithm identifier SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		buf += len;

		/* Should be issuer name SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		buf += len;

		/* Should be validity SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		buf += len;

		/* Should be subject name SEQUENCE */
		subjname = buf;
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		len += buf - subjname;

		if (dnamelen==len && memcmp (subjname, dname, len) == 0) {
			/* Have a match */
			*rbuf = certs;
			*rlen = certlen;
			return kPGPError_NoErr;
		}

		totallen -= certlen + 3;
		certs += certlen;
	}
	return kPGPError_ItemNotFound;
error:
	return err;
}


/*
 * Verify X.509 certificate chain.
 *
 * We have a certain number of root certificates, plus some in the chain.
 * Call with a TLS cert chain buffer.  This has a three byte buffer length,
 * followed with 1 or more repetitions of a three byte certificate length
 * plus a certificate.  The certificates are from most local to most
 * global.  We are also passed a list of root certificates in the same form.
 * Return 0 on success, error code on error.
 */
PGPError
PGPVerifyX509CertificateChain (PGPContextRef context,
							   PGPByte *certchain, PGPByte *rootcerts)
{
	PGPBoolean matchedinroot;
	PGPByte *buf;
	PGPByte *buf2;
	PGPByte *hashedbuf;
	PGPByte *sigbuf;
	PGPByte *parentcert;
	PGPByte *childcert;
	PGPByte *issuerbuf;
	PGPUInt32 hashedlen;
	PGPSize len;
	PGPUInt32 certchainlen;
	PGPUInt32 certlen;
	PGPUInt32 parentlen;
	PGPUInt32 sigbuflen;
	PGPUInt32 tag;
	PGPInt32 rslt;
	PGPHashContextRef certhash = NULL;
	PGPKeySetRef parentkeyset;
	PGPKeyListRef keylist;
	PGPKeyIterRef keyiter;
	PGPKeyRef parentkey;
	PGPError err;
	PGPHashAlgorithm hashalg;
	PGPPubKey *pubkey = NULL;
	RingSet const *parentringset = NULL;
	


	childcert = certchain;
	certchainlen = dolen3 (&childcert);
	certlen = dolen3 (&childcert);
	do {
		/* Parse down to find issuer name */
		buf = childcert;

		/* Should be the overall certificate SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);

		/* Should be certificate info SEQUENCE */
		hashedbuf = buf;
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		hashedlen = len + buf - hashedbuf;

		/* May be X509 version number */
		tag = dotaglen(&buf, &len);
		if (tag == TAG_ZERO) {
			buf += len;
			tag = dotaglen(&buf, &len);
		}

		/* Should be serial number INTEGER */
		EXPECT(tag, TAG_INTEGER);
		buf += len;

		/* Should be signature algorithm identifier SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		buf2 = buf + len;
		/* Get hash algorithm for signature from it */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_OBJECT);
		err = sDecodeAlgorithmOID (buf, len, NULL, &hashalg);
		buf = buf2;

		/* Should be issuer name SEQUENCE */
		issuerbuf = buf;
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		len += buf - issuerbuf;

		/* See if we can match issuer in root list */
		err = sLookup509 (rootcerts, issuerbuf, &parentcert, &parentlen);
		matchedinroot = (parentlen != 0);
		if (!parentlen) {
			/* Failing that, see if we were given a parent cert */
			err = sLookup509 (certchain, issuerbuf, &parentcert, &parentlen);
			
		}
		if (!parentlen) {
			/* Unable to complete the search, no parent available */
			return (err == kPGPError_ItemNotFound) ?
								kPGPError_X509NeededCertNotAvailable : err;
		}

		/* Find signature data */
		buf = hashedbuf + hashedlen;

		/* Should be signature algorithm identifier SEQUENCE */
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_SEQUENCE);
		buf += len;
		
		/* Should be signature data as an encoded BITSTRING */
		sigbuf = buf;
		tag = dotaglen(&buf, &len);
		EXPECT(tag, TAG_BITSTRING);
		sigbuflen = len + (buf - sigbuf);

		/* Parse parent cert to get key data */
		err = sDecode509 (parentcert, context, &parentkeyset);
		if (IsPGPError(err))
			goto error;

		/* Get just the one key out of it */
		PGPOrderKeySet( parentkeyset, kPGPAnyOrdering, &keylist );
		PGPNewKeyIter( keylist, &keyiter );
		PGPKeyIterNext( keyiter, &parentkey );
		PGPFreeKeyIter( keyiter );
		PGPFreeKeyList( keylist );
		
		/* Hash data in child cert */
		certhash = pgpHashCreate( PGPGetContextMemoryMgr( context ),
								  pgpHashByNumber( hashalg ) );
		if( IsNull( certhash ) ) {
			err = kPGPError_X509InvalidCertificateFormat;
			goto error;
		}
		PGPContinueHash(certhash, hashedbuf, hashedlen);

		/* Prepare to check signature */
		parentringset = pgpKeyDBRingSet (parentkey->keyDB);
		pubkey = ringKeyPubKey(parentringset, parentkey->key, PGP_PKUSE_SIGN);
		if (!pubkey) {
			err = ringSetError(parentringset)->error;
			goto error;
		}
			
		/* Verify the signature: rslt==1 means OK, else failure */
		rslt = pgpPubKeyVerify( pubkey, sigbuf, sigbuflen,
								pgpHashGetVTBL(certhash),
								(PGPByte *) pgpHashFinal(certhash),
								kPGPPublicKeyMessageFormat_X509 );

		PGPFreeHashContext (certhash);
		certhash = NULL;
		pgpPubKeyDestroy( pubkey );
		pubkey = NULL;
		PGPFreeKeySet( parentkeyset );

		if( rslt != 1 ) {
			return kPGPError_X509InvalidCertificateSignature;
		}

		if (childcert == parentcert) {
			return kPGPError_X509SelfSignedCert;
		}

		/* Else try to validate parent cert */
		childcert = parentcert;
		certlen = parentlen;

	} while (!matchedinroot);

	return kPGPError_NoErr;	/* Success */

error:
	if( IsntNull( certhash ) )
		PGPFreeHashContext(certhash);
	if( IsntNull( pubkey ) )
		pgpPubKeyDestroy( pubkey );
	return err;
}



/****   Internal Functions   ****/


/* Parse a certificate into its three parts, returning pointers and size */

PGPError
pgpX509CertPartition( PGPByte *buf, PGPSize len,
	PGPByte **certinfobuf, PGPSize *certinfolen,
	PGPByte **sigalgbuf, PGPSize *sigalglen,
	PGPByte **sigbuf, PGPSize *siglen )
{
	PGPByte *buf2;
	PGPSize buf2len;
	PGPUInt32 tag;
	PGPError err = kPGPError_NoErr;

	/* Prefix, prepare output with default values */
	PGPValidatePtr( buf );
	if( IsntNull (certinfobuf) )
		*certinfobuf = NULL;
	if( IsntNull (certinfolen) )
		*certinfolen = 0;
	if( IsntNull (sigalgbuf) )
		*sigalgbuf = NULL;
	if( IsntNull (sigalglen) )
		*sigalglen = 0;
	if( IsntNull (sigbuf) )
		*sigbuf = NULL;
	if( IsntNull (siglen) )
		*siglen = 0;
	
	/* Should start with the overall certificate SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Should be certificate info SEQUENCE, which is certinfo */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (certinfobuf) )
		*certinfobuf = buf2;
	if( IsntNull (certinfolen) )
		*certinfolen = buf2len;

	/* Should be signature algorithm identifier SEQUENCE */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (sigalgbuf) )
		*sigalgbuf = buf2;
	if( IsntNull (sigalglen) )
		*sigalglen = buf2len;

	/* Should be signature data as an encoded BITSTRING */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_BITSTRING);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (sigbuf) )
		*sigbuf = buf2;
	if( IsntNull (siglen) )
		*siglen = buf2len;

error:
	return err;
}

/* Parse certificate info portion into parts, returning pointers and size */

PGPError
pgpX509CertInfoPartition( PGPByte *buf, PGPSize len,
	PGPByte **serialbuf, PGPSize *seriallen,
	PGPByte **sigalgbuf, PGPSize *sigalglen,
	PGPByte **issuerbuf, PGPSize *issuerlen,
	PGPByte **validitybuf, PGPSize *validitylen,
	PGPByte **subjectbuf, PGPSize *subjectlen,
	PGPByte **publickeyinfobuf, PGPSize *publickeyinfolen )
{
	PGPByte *buf2;
	PGPSize buf2len;
	PGPUInt32 tag;
	PGPError err = kPGPError_NoErr;

	/* Prefix, prepare output with default values */
	PGPValidatePtr( buf );
	if( IsntNull (serialbuf) )
		*serialbuf = NULL;
	if( IsntNull (seriallen) )
		*seriallen = 0;
	if( IsntNull (sigalgbuf) )
		*sigalgbuf = NULL;
	if( IsntNull (sigalglen) )
		*sigalglen = 0;
	if( IsntNull (issuerbuf) )
		*issuerbuf = NULL;
	if( IsntNull (issuerlen) )
		*issuerlen = 0;
	if( IsntNull (validitybuf) )
		*validitybuf = NULL;
	if( IsntNull (validitylen) )
		*validitylen = 0;
	if( IsntNull (subjectbuf) )
		*subjectbuf = NULL;
	if( IsntNull (subjectlen) )
		*subjectlen = 0;
	if( IsntNull (publickeyinfobuf) )
		*publickeyinfobuf = NULL;
	if( IsntNull (publickeyinfolen) )
		*publickeyinfolen = 0;
	
	/* Should start with the overall certificate info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* May be X509 version number */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	if (tag == TAG_ZERO) {
		buf += len;
		buf2 = buf;
		tag = dotaglen(&buf, &len);
	}

	/* Should be serial number INTEGER */
	EXPECT(tag, TAG_INTEGER);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (serialbuf) )
		*serialbuf = buf2;
	if( IsntNull (seriallen) )
		*seriallen = buf2len;

	/* Should be signature algorithm identifier SEQUENCE */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (sigalgbuf) )
		*sigalgbuf = buf2;
	if( IsntNull (sigalglen) )
		*sigalglen = buf2len;

	/* Should be issuer name SEQUENCE */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (issuerbuf) )
		*issuerbuf = buf2;
	if( IsntNull (issuerlen) )
		*issuerlen = buf2len;

	/* Should be validity SEQUENCE */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (validitybuf) )
		*validitybuf = buf2;
	if( IsntNull (validitylen) )
		*validitylen = buf2len;

	/* Should be subject name SEQUENCE */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (subjectbuf) )
		*subjectbuf = buf2;
	if( IsntNull (subjectlen) )
		*subjectlen = buf2len;

	/* Should be subject public key info SEQUENCE */
	buf2 = buf;
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;
	buf2len = buf - buf2;

	if( IsntNull (publickeyinfobuf) )
		*publickeyinfobuf = buf2;
	if( IsntNull (publickeyinfolen) )
		*publickeyinfolen = buf2len;

error:
	return err;
}



/*
 * Given an X.509 cert, fill a buffer with a PGP formatted
 * userid packet in it corresponding to the name in the cert.
 * Use the ringpool's reserve buffer.
 * The packet does not include the PGP headers, just the body.
 */
PGPError
pgpX509CertToNameBuffer (RingPool *pool, PGPByte *buf, PGPSize len,
	PGPSize *pktlen)
{
	PGPUInt32 tag;
	PGPByte *namebuf;
	PGPSize namelen;
	PGPError err = kPGPError_NoErr;

	/* Should be the overall certificate SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Should be certificate info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* May be X509 version number */
	tag = dotaglen(&buf, &len);
	if (tag == TAG_ZERO) {
		buf += len;
		tag = dotaglen(&buf, &len);
	}

	/* Should be serial number INTEGER */
	EXPECT(tag, TAG_INTEGER);
	buf += len;

	/* Should be signature algorithm identifier SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be issuer name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be validity SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be subject name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	err = sDecodeName (buf, len, NULL, &namelen);
	if (IsPGPError(err))
		goto error;
	namebuf = (PGPByte *)ringReserve(pool, namelen);
	if (IsNull(namebuf)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	err = sDecodeName (buf, len, namebuf, &namelen);
	if (IsPGPError(err))
		goto error;

	*pktlen = namelen;

error:

	return err;
}


/*
 * Given an X.509 cert, fill a buffer with a PGP formatted
 * key packet in it corresponding to the key data in the cert.
 * Use the ringpool's reserve buffer.
 * The packet does not include the PGP headers, just the body.
 */
PGPError
pgpX509CertToKeyBuffer (RingPool *pool, PGPByte *buf, PGPSize len,
	PGPSize *pktlen)
{
	PGPUInt32 tag;
	PGPByte *keybuf;
	PGPSize keylen;
	PGPError err = kPGPError_NoErr;

	/* Should be the overall certificate SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* Should be certificate info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	/* May be X509 version number */
	tag = dotaglen(&buf, &len);
	if (tag == TAG_ZERO) {
		buf += len;
		tag = dotaglen(&buf, &len);
	}

	/* Should be serial number INTEGER */
	EXPECT(tag, TAG_INTEGER);
	buf += len;

	/* Should be signature algorithm identifier SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be issuer name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be validity SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be subject name SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);
	buf += len;

	/* Should be subject public key info SEQUENCE */
	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_SEQUENCE);

	err = sDecodeKey (buf, len, NULL, &keylen);
	if (IsPGPError(err))
		goto error;
	keybuf = (PGPByte *)ringReserve(pool, keylen);
	if (IsNull(keybuf)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	err = sDecodeKey (buf, len, keybuf, &keylen);
	if (IsPGPError(err))
		goto error;

	*pktlen = keylen;

error:

	return err;
}



/*
 * Given an X.509 cert, fill a buffer with a PGP formatted
 * userid packet in it corresponding to the signature in the cert.
 * Use the ringpool's reserve buffer.
 * The packet does not include the PGP headers, just the body.
 */
PGPError
pgpX509CertToSigBuffer (RingPool *pool, PGPByte *buf, PGPSize len,
	PGPSize *pktlen)
{
	PGPSize siglen;
	PGPByte *sigbuf;
	PGPError err = kPGPError_NoErr;

	(void) len;

	err = sDecodeSig(buf, NULL, &siglen);
	if (IsPGPError(err))
		goto error;
	sigbuf = (PGPByte *)ringReserve(pool, siglen);
	if (IsNull(sigbuf)) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	err = sDecodeSig (buf, sigbuf, &siglen);
	if (IsPGPError(err))
		goto error;

	*pktlen = siglen;

error:

	return err;
}
