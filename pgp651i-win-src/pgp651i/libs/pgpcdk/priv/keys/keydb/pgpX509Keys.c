/*
 * X509 Key importing
 *
 * Copyright (C) 1996,1997,1998 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: pgpX509Keys.c,v 1.64 1999/05/19 22:34:03 hal Exp $
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
#include "pgpRngPars.h"
#include "pgpRngRead.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"
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

/* Use TIS ASN.1 compiler */
#include "pgpX509Cert_asn.h"

/* Whether or not to use short forms for X509 name conversions */
#ifndef PGP_X509SHORTNAMES
#define PGP_X509SHORTNAMES 1
#endif

#define CHKNONNULL(p,e)	if (IsNull(p)) { \
							(e) = kPGPError_OutOfMemory; \
							goto error; \
						}

#define CHKASNERR(e)	if ((e) != 0) { \
							if ((e) == kPGPASNError_ErrOutOfMemory) { \
								(e) = kPGPError_OutOfMemory; \
							} else { \
								(e) = k509Error_InvalidCertificateFormat; \
							} \
							goto error; \
						}

/* Error returns */
#define k509Error_NeededCertNotAvailable			-10
#define k509Error_SelfSignedCert					-20
#define k509Error_InvalidCertificateSignature		-30
#define k509Error_InvalidCertificateFormat			-40


/* Size limit on cert */
#define MAX_509_CERT_SIZE		65536

/* Commonly used tag values */
#define TAG_ZERO			0
#define TAG_INTEGER			2
#define TAG_BITSTRING		3
#define TAG_OCTETSTRING		4
#define TAG_NULL			5
#define TAG_OBJECT			6
#define TAG_SEQUENCE		16
#define	TAG_SET				17
#define TAG_PRINTABLE		19
#define TAG_T61STRING		20
#define TAG_IA5STRING		22
#define TAG_UTCTIME			23
#define TAG_GENERALIZEDTIME	24
#define TAG_CONSTRUCTED		32

/* Tagging values for CHOICE fields */
#define ASN_EXPLICIT		0x80|0x20
#define ASN_IMPLICIT		0x80
#define ASN_GENERALNAME_RFC822NAME		(1|ASN_EXPLICIT)
#define ASN_GENERALNAME_DNSNAME			(2|ASN_EXPLICIT)
#define ASN_GENERALNAME_DIRECTORYNAME	(4|ASN_EXPLICIT)
#define ASN_GENERALNAME_URI				(6|ASN_EXPLICIT)
#define ASN_GENERALNAME_IPADDRESS		(7|ASN_EXPLICIT)
#define ASN_GENERALNAME_REGISTEREDID	(8|ASN_EXPLICIT)
#define ASN_DPNAME_GENERALNAMES			(0|ASN_EXPLICIT)


#define elemsof(arr)  (sizeof(arr)/sizeof(arr[0]))

#define EXPECT(t, v)	if (t != v) { \
							err = kPGPError_X509InvalidCertificateFormat; \
							goto error; \
						}


/* Our inserted Description or OU value */
static char	s_pgpdescr[] = "PGPKeyCreation=0x";

/* RSA OID is (1, 2, 840, 113549, 1, 1, 1) */
static PGPByte const rsaoid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x01};
/* RSA with MD5 OID is (1, 2, 840, 113549, 1, 1, 4) */
static PGPByte const rsamd5oid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x04};
/* RSA with MD2 OID is (1, 2, 840, 113549, 1, 1, 2) */
static PGPByte const rsamd2oid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x02};
/* RSA with SHA OID is (1, 2, 840, 113549, 1, 1, 5) */
static PGPByte const rsashaoid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7,
							  0x0d, 0x01, 0x01, 0x05};

/* DSA OID is (1, 2, 840, 10040, 4, 1) */
static PGPByte const dsaoid[] = {0x2a, 0x86, 0x48, 0xce, 0x38, 0x04, 0x01};
/* DSA with SHA-1 OID is (1, 2, 840, 10040, 4, 3) */
static PGPByte const dsashaoid1[] = {0x2a, 0x86, 0x48, 0xce, 0x38, 0x04, 0x03};
/* Another variant (1 3 14 3 2 27) */
static PGPByte const dsashaoid2[] = {0x2b, 0x0e, 0x03, 0x02, 0x1b};

/* ElG OID is (1, 2, 840, 10046, 2, 1) */
static PGPByte const elgoid[] = {0x2a, 0x86, 0x48, 0xce, 0x3e, 0x02, 0x01};

/* CN (common-name) OID is (2, 5, 4, 3) */
static PGPByte const cnoid[] = {0x55, 0x04, 0x03};
/* RSA's OID for email address in RDN's: (1, 2, 840, 113549, 1, 9, 1) */
static PGPByte const emailoid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
									0x09, 0x01};
static const char emailname[] = "EMAIL";

/* Issuer, subject alternative names (2, 5, 29, 17) and (2, 5, 29, 18) */
static PGPByte const issaltnameoid[] = {0x55, 0x1d, 0x11};
static PGPByte const subaltnameoid[] = {0x55, 0x1d, 0x12};

/* Extensions */
/* Basic constraints (2, 5, 29, 19) */
static PGPByte const bconstraintsoid[] = {0x55, 0x1d, 0x13};
/* Key usage (2, 5, 29, 15) */
static PGPByte const keyusageoid[] = {0x55, 0x1d, 0x0f};
/* Issuer Distribution Point (2, 5, 29, 28) */
static PGPByte const issdistpoint[] = {0x55, 0x1d, 0x1c};
/* Certificate Distribution Point (2, 5, 29, 31) */
static PGPByte const certdistpoint[] = {0x55, 0x1d, 0x1f};

/* Need an OID for PGP extension */
/* PGP root is (1 3 6 1 4 1 3401 8), append (1 1) for us */
static PGPByte const pgpextensionoid[] = {0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04,
										 0x01, 0x9a, 0x49, 0x08, 0x01, 0x01};



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
	{TAG_GENERALIZEDTIME, "GeneralizedTime"}
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


/* ASN memory management */

	static void *
sASNAlloc( PGPASN_MemoryMgr *asnmgr, size_t requestSize)
{
	PGPMemoryMgrRef mgr = asnmgr->customValue;
	return PGPNewData( mgr, (PGPSize)requestSize, 0);
}

	static int
sASNReAlloc( PGPASN_MemoryMgr *asnmgr, void **allocation,
			 size_t newAllocationSize )
{
	PGPMemoryMgrRef mgr = asnmgr->customValue;
	return (int)PGPReallocData( mgr, allocation, newAllocationSize, 0);
}

	static int
sASNDeAlloc( PGPASN_MemoryMgr *asnmgr, void *allocation )
{
	(void) asnmgr;
	return (int)PGPFreeData( allocation );
}

	static void
sSetupASNCONTEXT (PGPMemoryMgrRef mgr, PGPASN_CONTEXT *asnctx,
				  PGPASN_MemoryMgr *asnmgr)
{
	asnctx->customValue = NULL;
	asnctx->memMgr = asnmgr;
	asnmgr->customValue = (void *)mgr;
	asnmgr->allocProc = sASNAlloc;
	asnmgr->reallocProc = sASNReAlloc;
	asnmgr->deallocProc = sASNDeAlloc;
}



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

#if 0
/* Unused */
/* Return the length of the DER length field for the specified length */
static PGPUInt32
_lenlen (PGPUInt32 length)
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
#endif

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


#if 0
/*
 * Convert an RFC2253 LDAP-format string to an X.509 PGPASN_Name
 * structure in *dname.
 * Any quoting used in input must be double quotes around
 * the whole value, as in O="Network Associates, Inc.".
 */
	static PGPError
sEncodeDN (PGPMemoryMgrRef mgr, PGPASN_CONTEXT *asnctx, char const *str,
	PGPASN_Name **pname)
{
	PGPASN_Name *name = NULL;
	PGPASN_RDNSequence *rdns ;
	PGPASN_RelativeDistinguishedName *rdn;
	PGPASN_AttributeTypeAndValue *atv;
	PGPASN_PrintableString *ps = NULL;	/* May be IA5String */
	PGPByte *psbuf = NULL;
	PGPSize psbufsize;
	char *s;
	char *eq, *cq, *com = NULL;
	PGPUInt32 i = 0;
	PGPSize slen;
	char *scopy;
	PGPBoolean email;
	PGPError err = kPGPError_NoErr;
	PGPByte oid[sizeof(emailoid)];
	PGPSize oidsize;

	*pname = NULL;

	slen = strlen(str);
	scopy = PGPNewData( mgr, slen+1, 0 );
	pgpCopyMemory( str, scopy, slen+1 );
	s = scopy;

	name = pgpasn_NewName (asnctx);
	CHKNONNULL( name, err );
	rdns = pgpasn_NewRDNSequence (asnctx);
	CHKNONNULL( rdns, err );
	name->CHOICE_field_type = PGPASN_ID_RDNSequence;
	name->data = rdns;

	while (*s) {
		if ((eq = strchr(s, '=')) == NULL) {
			err = kPGPError_BadParams;
			goto error;
		}
		*eq = '\0';
		if( strcmp(emailname, s) == 0 ) {
			email = TRUE;
		} else {
			email = FALSE;
			for (i=0; i<elemsof(dnnames); ++i) {
				if (strcmp(dnnames[i], s) == 0)
					break;
			}
			if (i >= elemsof(dnnames)) {
				err = kPGPError_BadParams;
				goto error;
			}
		}
		s = eq + 1;
		if (*s == '"') {
			if ((cq = strchr(++s, '"')) == NULL) {
				err = kPGPError_BadParams;
				goto error;
			}
			*cq = '\0';
		} else {
			cq = NULL;
			if ((com = strchr(s, ',')) == NULL)
				com = s + strlen(s);
			*com = '\0';
		}
		/* At this point, s points at the value, null terminated. */
		/* i holds the index of the attribute, or email is true */

		++rdns->n;
		rdn = pgpasn_NewRelativeDistinguishedName (asnctx);
		CHKNONNULL( rdn, err );
		rdns->elt[rdns->n++] = rdn;
		atv = pgpasn_NewAttributeTypeAndValue (asnctx);
		CHKNONNULL( atv, err );
		rdn->elt[0] = atv;
		rdn->n = 1;

		/* Insert attribute OID */
		if( email ) {
			oidsize = sizeof(emailoid);
			pgpCopyMemory( emailoid, oid, oidsize );
		} else {
			oidsize = sizeof(cnoid);
			pgpCopyMemory( cnoid, oid, oidsize );
			oid[oidsize-1] = i;
		}
		pgpasn_PutOctVal (asnctx, &atv->type, oid, oidsize);

		/* Insert attribute value */
		ps = pgpasn_NewPrintableString (asnctx);
		CHKNONNULL( ps, err );
		pgpasn_PutOctVal (asnctx, ps, s, strlen(s));
		psbufsize = pgpasn_SizeofPrintableString (asnctx, ps, TRUE);
		psbuf = PGPNewData( mgr, psbufsize, 0 );
		CHKNONNULL( psbuf, err );
		if( email )
			pgpasn_PackIA5String( asnctx, psbuf, psbufsize, ps, &err );
		else
			pgpasn_PackPrintableString( asnctx, psbuf, psbufsize, ps, &err );
		CHKASNERR( err );
		pgpasn_PutOctVal (asnctx, &atv->value, psbuf, psbufsize);
		PGPFreeData( psbuf );
		psbuf = NULL;
		pgpasn_FreePrintableString( asnctx, ps );
		ps = NULL;

		if (cq) {
			s = cq+2;
			if (s[-1] != ',') {
				err = kPGPError_BadParams;
				goto error;
			}
		} else {
			s = com+1;
		}
	}

	PGPFreeData( scopy );
	*pname = name;

	return kPGPError_NoErr;

 error:
	if (scopy)
		PGPFreeData( scopy );
	if (ps)
		pgpasn_FreePrintableString( asnctx, ps );
	if (psbuf)
		PGPFreeData( psbuf );
	if (name)
		pgpasn_FreeName( asnctx, name );
	return err;
}


/*
 * Convert an RFC2253 LDAP-format string to an allocated Distinguished
 * Name buffer.  Caller should do PGPFreeData on *pdname when done.  Any
 * quoting used in input must be double quotes around the whole value,
 * as in O="Network Associates, Inc.".
 */
	PGPError
PGPCreateDistinguishedName( PGPContextRef context, char const *str,
	PGPByte **pdname)
{
	PGPMemoryMgrRef mgr;
	PGPASN_CONTEXT  asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPASN_Name *name = NULL;
	PGPByte *dname = NULL;
	PGPSize dnamesize;
	PGPError err = kPGPError_NoErr;

	*pdname = NULL;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	err = sEncodeDN( mgr, asnctx, str, &name );
	if( IsPGPError( err ) )
		goto error;

	dnamesize = pgpasn_SizeofName (asnctx, name, TRUE);
	dname = PGPNewData( mgr, dnamesize, 0 );
	CHKNONNULL( dname, err );
	pgpasn_PackName( asnctx, dname, dnamesize, name, &err );
	CHKASNERR( err );

	pgpasn_FreeName( asnctx, name );

	*pdname = dname;
	return kPGPError_NoErr;

 error:
	if( name )
		pgpasn_FreeName( asnctx, name );
	if( dname )
		PGPFreeData( dname );
	return err;
}

#endif

	
/*
 * Add backslashes as needed for X.509 LDAP format conversion
 * Allocate s2 to be twice as big as s1, for safety.
 */
	static void
sEncodeLDAPString( const char *s1, char *s2 )
{
	PGPBoolean inQuotes = FALSE;
	PGPBoolean afterBackslash = FALSE;
	char c;

	while ((c = *s1++) != '\0') {
		if (c == '"' && !afterBackslash)
			inQuotes = !inQuotes;
		if (inQuotes) {
			if (c == '=' || c == '\\' || c == ',' || c == '+' ||
				c == '<' || c == '>' || c == ';' || c == ' ' || c == '#')
				*s2++ = '\\';
		}
		if (((c != '\\' || !inQuotes) && c != '"') || afterBackslash)
			*s2++ = c;
		afterBackslash = (c == '\\' && !afterBackslash);
	}
	*s2++ = '\0';
}

/*
 * Convert an RFC2253 LDAP-format string to an allocated Distinguished
 * Name buffer.  Caller should do PGPFreeData on *pdname when done.
 */
	PGPError
PGPCreateDistinguishedName( PGPContextRef context, char const *str,
	PGPByte **pdname, PGPSize *pdnamelen )
{
	PGPByte *dname = NULL;
	PGPSize dnamelen;
	char *str2 = NULL;
	PGPError err = kPGPError_NoErr;
	PGPMemoryMgrRef mgr = PGPGetContextMemoryMgr( context );

	PGPValidatePtr( pdname );
	PGPValidatePtr( pdnamelen );

	*pdname = NULL;
	*pdnamelen = 0;

	str2 = (char *)PGPNewData( mgr, 2*strlen(str) + 1, 0 );
	CHKNONNULL( str2, err );
	sEncodeLDAPString( str, str2 );

	err = X509CreateDistinguishedName( context, str2, &dname, &dnamelen );

	if( IsPGPError( err ) )
		goto error;

	*pdname = dname;
	*pdnamelen = dnamelen;

 error:
	if( IsPGPError( err ) && IsntNull(dname) )
		PGPFreeData( dname );
	if( str2 )
		PGPFreeData( str2 );
	return err;
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
 * If generalized is true, it is a GeneralizedTime structure, four digit
 * years.
 */
static PGPError
sDecodeUTCTime (PGPByte const *buf, PGPSize len, PGPBoolean generalized,
				PGPUInt32 *timestamp)
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
		if (i == 0 && generalized) {
			/* Two more digits */
			datefield[i] = datefield[i] * 100 + (buf[0]-'0') * 10
							+ (buf[1] - '0');
			buf += 2;
			len -= 2;
		}
	}
	if (len != 1  ||  buf[0] != 'Z'  ||  i < 5) {
		err = kPGPError_X509InvalidCertificateFormat;
		goto error;
	}
	
	/* Correct for Y2K conventions on UTCTime */
	if (!generalized)
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
sDecodeAlgorithmOID (PGPASN_CONTEXT *asnctx, PGPASN_OBJECT_ID *oid,
	PGPPublicKeyAlgorithm *keyalgorithm, PGPHashAlgorithm *hashalgorithm)
{
	PGPByte *buf = oid->val;
	PGPSize len = oid->len;
	PGPError err = kPGPError_NoErr;

	(void) asnctx;

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
	} else if (len == sizeof(rsashaoid) && memcmp (buf, rsashaoid, len) == 0) {
		if( IsntNull( hashalgorithm ) )
			*hashalgorithm = (PGPHashAlgorithm) kPGPHashAlgorithm_SHA;
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_RSA;
	} else if (len == sizeof(dsaoid) && memcmp (buf, dsaoid, len) == 0) {
		if( IsntNull( keyalgorithm ) )
			*keyalgorithm = kPGPPublicKeyAlgorithm_DSA;
	} else if ((len == sizeof(dsashaoid1) && memcmp (buf,dsashaoid1,len) == 0)
		  || (len == sizeof(dsashaoid2) && memcmp (buf,dsashaoid2,len) == 0)) {
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
 * Look for IP address or DNS name in subject alternative name extension
 * If ipbuf is NULL, set *iplength to necessary length; else store the
 * IP data into ipbuf, and treat *iplength as an input parameter telling
 * what the length needs to be.  (Presumably *iplength is always 4?)
 * If dnsbuf is NULL, set *dnslength to necessary length; else store the
 * DNS name into dnsbuf, and treat *dnslength as an input parameter telling
 * what the length needs to be.
 * If there is more than one IP or DNS we just return the first of each
 * type.
 */
static PGPError
sDecodeIPDNSName(PGPASN_CONTEXT *asnctx, PGPASN_Certificate *cert,
				 PGPByte *ipbuf, PGPSize *iplength,
				 PGPByte *dnsbuf, PGPSize *dnslength)
{
	PGPASN_Extensions *exts;
	PGPASN_Extension *ext;
	PGPASN_GeneralNames *gnames;
	PGPASN_GeneralName *gname;
	PGPASN_VariableBlock *vb;
	PGPBoolean seenIP=FALSE, seenDNS=FALSE;
	PGPInt32 i, j;
	PGPError err = kPGPError_NoErr;

	if (IsNull( ipbuf ) && IsntNull( iplength ) )
		*iplength = 0;
	if (IsNull( dnsbuf ) && IsntNull( dnslength ) )
		*dnslength = 0;

	exts = cert->tbsCertificate.extensions;
	if (IsNull(exts))
		return err;

	/* Check extensions for subject alternative name */
	for (i=0; i<exts->n && (!seenIP || !seenDNS); ++i) {
		ext = exts->elt[i];
		/* These are same size */
		if (ext->extnID.len == sizeof(subaltnameoid)
			&& memcmp(ext->extnID.val, subaltnameoid, ext->extnID.len)==0 ) {
			/* Subject alternative name, just as we wanted */
			pgpasn_UnpackGeneralNames( asnctx, &gnames, ext->extnValue.val,
									   ext->extnValue.len, &err );
			if (err != 0) {
				/* Skip if can't handle */
				err = 0;
				continue;
			}
			for (j=0; j<gnames->n && (!seenIP || !seenDNS); ++j) {
				gname = gnames->elt[j];
				if (gname->CHOICE_field_type == ASN_GENERALNAME_IPADDRESS
					&& !seenIP) {
					seenIP = TRUE;
					vb = gname->data;
					if (IsNull( ipbuf ) ) {
						if( IsntNull( iplength ) )
							*iplength = vb->len;
					} else {
						pgpCopyMemory( vb->val, ipbuf, vb->len );
					}
				} else if (gname->CHOICE_field_type == ASN_GENERALNAME_DNSNAME
						   && !seenDNS) {
					seenDNS = TRUE;
					vb = gname->data;
					if (IsNull( dnsbuf ) ) {
						if( IsntNull( dnslength ) )
							*dnslength = vb->len;
					} else {
						pgpCopyMemory( vb->val, dnsbuf, vb->len );
					}
				}
			}
			pgpasn_FreeGeneralNames( asnctx, gnames );
		}
	}
	return err;
}


/*
 * Convert from X.509 cert to PGP signature validity subpackets.
 * If vbuf is NULL, set *vlength to the necessary length.  Else
 * store the data into vbuf, and treat *vlength as an input parameter
 * telling what the length needs to be.
 * We create a pair of PGP signature subpackets expressing creation and
 * expiration dates.
 */
static PGPError
sDecodeValidity(PGPASN_CONTEXT *asnctx, PGPASN_Certificate *cert,
				PGPByte *vbuf, PGPSize *vlength)
{
	PGPASN_Validity *val = &cert->tbsCertificate.validity;
	PGPASN_CertificateValidityDate *cvd;
	PGPASN_UTCTime *utc;
	PGPUInt32 creationtime;
	PGPUInt32 expirationtime;
	PGPUInt32 o;
	PGPError err = kPGPError_NoErr;

	(void)asnctx;
	pgpAssert( IsntNull( vlength ) );

	cvd = &val->notBefore;
	utc = (PGPASN_UTCTime *)cvd->data;
	err = sDecodeUTCTime (utc->val, utc->len,
					(PGPBoolean)(cvd->CHOICE_field_type!=PGPASN_ID_UTCTime),
					&creationtime);
	if( IsPGPError( err ) )
		goto error;

	cvd = &val->notAfter;
	utc = (PGPASN_UTCTime *)cvd->data;
	err = sDecodeUTCTime (utc->val, utc->len,
					(PGPBoolean)(cvd->CHOICE_field_type!=PGPASN_ID_UTCTime),
					&expirationtime);
	if( IsPGPError( err ) )
		goto error;

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
 * Extract cert NotBefore time as a four byte time value, in bigendian form.
 */
static PGPError
sDecodeNotBefore4(PGPASN_CONTEXT *asnctx, PGPASN_Certificate *cert,
				  PGPByte *cbuf)
{
	PGPASN_Validity *val = &cert->tbsCertificate.validity;
	PGPASN_CertificateValidityDate *cvd;
	PGPASN_UTCTime *utc;
	PGPUInt32 creationtime;
	PGPError err = kPGPError_NoErr;

	(void)asnctx;

	cvd = &val->notBefore;
	utc = (PGPASN_UTCTime *)cvd->data;
	err = sDecodeUTCTime (utc->val, utc->len,
					(PGPBoolean)(cvd->CHOICE_field_type!=PGPASN_ID_UTCTime),
					&creationtime);
	if( IsPGPError( err ) )
		goto error;

	*cbuf++ = (creationtime >> 24) & 0xff;
	*cbuf++ = (creationtime >> 16) & 0xff;
	*cbuf++ = (creationtime >>  8) & 0xff;
	*cbuf++ = (creationtime >>  0) & 0xff;

error:

	return err;
}


/*
 * Convert from X.509 cert to PGP signature level subpacket, if any.
 * If slbuf is NULL, set *sllength to the necessary length.  Else
 * store the data into slbuf, and treat *sllength as an input parameter
 * telling what the length needs to be.
 * We optionally create a PGP sig-level subpacket if it is a CA certificate.
 */
static PGPError
sDecodeSigLevel(PGPContextRef context, PGPASN_CONTEXT *asnctx,
				PGPASN_Certificate *cert, PGPByte *slbuf, PGPSize *sllength)
{
	PGPASN_Extensions *exts = cert->tbsCertificate.extensions;
	PGPASN_Extension *ext;
	PGPASN_BasicConstraints *bc;
	PGPUInt32 o;
	PGPInt32 i;
	PGPInt32 level = 0;
	PGPBoolean gotCA = FALSE;
	RingPool *pool = pgpContextGetRingPool( context );
	PGPError err = kPGPError_NoErr;

	pgpAssert( IsntNull( sllength ) );

	/* Nothing to do if no extensions */
	if (!exts) {
		if( IsNull( slbuf ) ) {
			*sllength = 0;
		}
		return kPGPError_NoErr;
	}

	for (i=0; (i<exts->n) && !gotCA; ++i) {
		ext = exts->elt[i];
		if (ext->extnID.len == sizeof(bconstraintsoid)
			&& memcmp(ext->extnID.val, bconstraintsoid, ext->extnID.len)==0 ) {
			/* Have a BasicConstraints extension */
			pgpasn_UnpackBasicConstraints( asnctx, &bc, ext->extnValue.val,
										   ext->extnValue.len, &err );
			if (err != 0) {
				/* Skip if no good */
				err = 0;
				continue;
			}
			if ( IsntNull (bc->cA) && pgpasn_GetBoolVal( asnctx, bc->cA ) ) {
				/* Here we have it as a CA.  Assume maximum level */
				if (IsNull( bc->pathLenConstraint) ) {
					/* Max level if unconstrained */
					level = 255;
				} else {
					level = pgpasn_GetIntVal( asnctx, bc->pathLenConstraint,
											  &err );
					if( err ) {
						/* Skip the whole thing if malformed */
						err = 0;
						pgpasn_FreeBasicConstraints( asnctx, bc );
						continue;
					}
					/* In our units we use one level larger */
					level += 1;
				}
				gotCA = TRUE;
			}
			pgpasn_FreeBasicConstraints( asnctx, bc );
		}
	}

	if (!gotCA) {
		if( IsNull( slbuf ) ) {
			*sllength = 0;
		}
		return kPGPError_NoErr;
	}

	/* Here we need to create a sig level packet */
	if( IsNull( slbuf ) ) {
		/* One byte of len, one of type, one of level, one of trust */
		*sllength = 4;
		return kPGPError_NoErr;
	}

	pgpAssert (*sllength == 4);

	o = 0;
	slbuf[o++] = 3;	/* length */
	slbuf[o++] = SIGSUB_TRUST;
	slbuf[o++] = level;
	slbuf[o++] = ringTrustOldToExtern( pool, kPGPKeyTrust_Complete );

	pgpAssert (o == *sllength);

	return err;
}


/*
 * Convert from X.509 cert to PGP key usage subpacket, if any.
 * If kubuf is NULL, set *kulength to the necessary length.  Else
 * store the data into kubuf, and treat *kulength as an input parameter
 * telling what the length needs to be.
 * We optionally create a PGP sig-level subpacket if it is a CA certificate.
 */
static PGPError
sDecodeKeyUsage(PGPASN_CONTEXT *asnctx,
				PGPASN_Certificate *cert, PGPByte *kubuf, PGPSize *kulength)
{
	PGPASN_Extensions *exts = cert->tbsCertificate.extensions;
	PGPASN_Extension *ext;
	PGPASN_KeyUsage *ku;
	PGPUInt32 o;
	PGPInt32 i;
	PGPUInt32 kbits, kusage = 0;
	PGPBoolean gotKU = FALSE;
	PGPError err = kPGPError_NoErr;

	pgpAssert( IsntNull( kulength ) );

	/* Nothing to do if no extensions */
	if (!exts) {
		if( IsNull( kubuf ) ) {
			*kulength = 0;
		}
		return kPGPError_NoErr;
	}

	for (i=0; (i<exts->n) && !gotKU; ++i) {
		ext = exts->elt[i];
		if (ext->extnID.len == sizeof(keyusageoid)
			&& memcmp(ext->extnID.val, keyusageoid, ext->extnID.len)==0 ) {
			/* Have a KeyUsage extension */
			pgpasn_UnpackKeyUsage( asnctx, &ku, ext->extnValue.val,
								   ext->extnValue.len, &err );
			if (err != 0) {
				/* Skip if no good */
				err = 0;
				continue;
			}
			/* Coerce as an int value to get the bits */
			kbits = pgpasn_GetIntVal( asnctx,
									  (PGPASN_VariableBlock *)ku, &err );
			if (err != 0) {
				/* Skip if no good */
				err = 0;
				pgpasn_FreeKeyUsage( asnctx, ku );
				continue;
			}
			kusage = 0;
			if (kbits & (PGPASN_KeyUsage_digitalSignature |
						 PGPASN_KeyUsage_nonRepudiation))
				kusage |= SIGSUBF_KEYFLAG0_USAGE_SIGN;
			if (kbits & (PGPASN_KeyUsage_keyEncipherment |
						 PGPASN_KeyUsage_dataEncipherment |
						 PGPASN_KeyUsage_keyAgreement))
				kusage |= SIGSUBF_KEYFLAG0_USAGE_ENCRYPT_COMM |
						  SIGSUBF_KEYFLAG0_USAGE_ENCRYPT_STORAGE;
			if (kbits & (PGPASN_KeyUsage_keyCertSign |
						 PGPASN_KeyUsage_cRLSign))
				kusage |= SIGSUBF_KEYFLAG0_USAGE_CERTIFY;
			gotKU = TRUE;
			pgpasn_FreeKeyUsage( asnctx, ku );
		}
	}

	if (!gotKU) {
		if( IsNull( kubuf ) ) {
			*kulength = 0;
		}
		return kPGPError_NoErr;
	}

	/* Here we need to create a key usage packet */
	if( IsNull( kubuf ) ) {
		/* One byte of len, one of type, one of usage */
		*kulength = 3;
		return kPGPError_NoErr;
	}

	pgpAssert (*kulength == 3);

	o = 0;
	kubuf[o++] = 2;	/* length */
	kubuf[o++] = SIGSUB_KEYFLAGS;
	kubuf[o++] = kusage;

	pgpAssert (o == *kulength);

	return err;
}

/*
 * Extract PGP Key Creation field from cert, if available.
 * Return zero if not.
 * We try to encode it in the Description or, failing that, OrgUnit
 * fields of the subject DN.
 * If not there, we look for our custom extension.
 */
static PGPUInt32
sDecodeKeyCreation(PGPASN_CONTEXT *asnctx, PGPASN_Certificate *cert)
{
	PGPASN_RDNSequence *name;
	PGPASN_RelativeDistinguishedName *rdn;
	PGPASN_AttributeTypeAndValue *atv;
	PGPASN_DirectoryString *dstr;
	PGPASN_VariableBlock *vb;
	PGPASN_Extensions *exts;
	PGPASN_Extension *ext;
	PGPASN_PGPExtension *pe;
	PGPASN_UTCTime *utc;
	PGPUInt32 dateval = 0;
	PGPBoolean found;
	PGPInt32 i, j, k;
	char *bf;
	PGPError err = kPGPError_NoErr;

	found = FALSE;
	name = (PGPASN_RDNSequence *)cert->tbsCertificate.subject.data;
	/* Loop over elements of RDN sequence */
	for (i=0; i<name->n && !found; ++i) {
		rdn = name->elt[i];
		for (j=0; j<rdn->n && !found; ++j) {
			atv = rdn->elt[j];

			/* Look for OU or Description */
			if (atv->type.len == 3 &&
				atv->type.val[0] == cnoid[0] && atv->type.val[1] == cnoid[1] &&
				(atv->type.val[2] == 11 || atv->type.val[2] == 13)) {

				pgpasn_UnpackDirectoryString( asnctx, &dstr,
							atv->value.val, atv->value.len, &err );
				if (err != 0) {
					/* Skip if unprintable string */
					err = 0;
					continue;
				}
				vb = dstr->data;
				if (vb->len == sizeof(s_pgpdescr)-1 + 8 &&
					pgpMemoryEqual (vb->val, s_pgpdescr,
									sizeof(s_pgpdescr)-1) ) {
					/* Have a match, parse date value */
					found = TRUE;
					bf = (char *)vb->val + sizeof(s_pgpdescr)-1;
					for (k=0; k<8; ++k) {
						dateval = (dateval << 4) |
							((bf[k] <= '9') ? (bf[k] - '0')
							 : (bf[k] <= 'F') ? (bf[k] - 'A' + 10)
							 : (bf[k] - 'a' + 10));
					}
					pgpasn_FreeDirectoryString( asnctx, dstr );
					break;
				}
				pgpasn_FreeDirectoryString( asnctx, dstr );
			}
		}
	}

	if (found)
		return dateval;

	/* Now check extensions */
	
	exts = cert->tbsCertificate.extensions;
	if (IsNull(exts))
		return dateval;

	/* Check extensions for PGP special */
	for (i=0; i<exts->n && !found; ++i) {
		ext = exts->elt[i];
		if (ext->extnID.len == sizeof(pgpextensionoid)
			&& memcmp(ext->extnID.val, pgpextensionoid, ext->extnID.len)==0 ) {
			/* Have a match */
			pgpasn_UnpackPGPExtension( asnctx, &pe,
							ext->extnValue.val, ext->extnValue.len, &err );
			if (err != 0) {
				/* Skip if can't handle */
				err = 0;
				continue;
			}
			utc = (PGPASN_UTCTime *)pe->keyCreation.data;
			err = sDecodeUTCTime (utc->val, utc->len,
					(PGPBoolean)
					(pe->keyCreation.CHOICE_field_type!=PGPASN_ID_UTCTime),
					&dateval);
			if (err == kPGPError_NoErr)
				found = TRUE;
			pgpasn_FreePGPExtension( asnctx, pe );
		}
	}

	return dateval;
}




/*
 * Convert from X.509 cert to PGP style sig packet.
 * If sigbuf is NULL, set *siglength to the necessary length.  Else
 * store the data into sigbuf, and treat *siglength as an input parameter
 * telling what the length needs to be.
 */
static PGPError
sDecodeSig(PGPContextRef context, PGPASN_CONTEXT *asnctx,
		   PGPASN_Certificate *cert, PGPByte loversion,
		   PGPByte *sigBuf, PGPSize *sigLength)
{
	PGPSize certlen;
	PGPSize siglen;
	PGPSize extralen;
	PGPSize certlenlen;
	PGPSize validitylen;
	PGPSize siglevellen;
	PGPSize keyusagelen;
	PGPUInt32 o;
	PGPPublicKeyAlgorithm sigalg;
	PGPHashAlgorithm hashalg;
	PGPError err = kPGPError_NoErr;

	pgpAssert( IsntNull( sigLength ) );
	if( IsNull( sigBuf ) )
		*sigLength = 0;

	/* Get length of validity info */
	err = sDecodeValidity (asnctx, cert, NULL, &validitylen);
	if (IsPGPError(err))
		goto error;

	/* Get length of signing-level info */
	err = sDecodeSigLevel (context, asnctx, cert, NULL, &siglevellen);
	if (IsPGPError(err))
		goto error;

	/* Get length of key-usage info */
	err = sDecodeKeyUsage (asnctx, cert, NULL, &keyusagelen);
	if (IsPGPError(err))
		goto error;

	/* Decode signature algorithm and hash algorithm */
	err = sDecodeAlgorithmOID (asnctx, &cert->signatureAlgorithm.algorithm,
							   &sigalg, &hashalg);
	if( IsPGPError(err) )
		goto error;


	/* Calculate length of signature */
	certlen = pgpasn_SizeofCertificate( asnctx, cert, TRUE ) + 4;
	certlenlen = (certlen < 192) ? 1 : 2;
	extralen = validitylen + siglevellen + keyusagelen
				+ certlen + certlenlen;
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
	if (loversion <= 3)
		sigBuf[o++] = 0;		/* Use sigalgorithm of 0 because nonstandard */
	else
		sigBuf[o++] = 100;		/* 100 is more compatible with OpenPGP */
	sigBuf[o++] = hashalg;	/* But use true hash algorithm */

	sigBuf[o++] = (PGPByte) (extralen >> 8);
	sigBuf[o++] = (PGPByte) (extralen >> 0);
	err = sDecodeValidity (asnctx, cert, sigBuf + o, &validitylen);
	o += validitylen;
	err = sDecodeSigLevel (context, asnctx, cert, sigBuf + o, &siglevellen);
	o += siglevellen;
	err = sDecodeKeyUsage (asnctx, cert, sigBuf + o, &keyusagelen);
	o += keyusagelen;

	if (certlenlen == 1)
		sigBuf[o++] = certlen;
	else {
		sigBuf[o++] = (PGPByte) (192 + ((certlen-192) >> 8));
		sigBuf[o++] = (PGPByte) (certlen-192);
	}
	sigBuf[o++] = SIGSUB_NAI;
	sigBuf[o++] = SIGSUBSUB_X509;
	/* Store translation version bytes */
	sigBuf[o++] = SIGSUBSUB_X509_VERSION_HI;
	sigBuf[o++] = loversion;
	pgpasn_PackCertificate( asnctx, sigBuf+o, certlen-4, cert, &err );
	CHKASNERR( err );
	o += certlen - 4;

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
 * Convert from X.509 cert to PGP style name.  If issuer is true, we do
 * issuer's name, else subject name.
 * If namebuf is NULL, set *namelength to the necessary length.  Else
 * store the data into namebuf, and treat *namelength as an input parameter
 * telling what the length needs to be.
 * This versions uses long form for X509 names, in RFC2253 format.
 */
static PGPError
sDecodeNameLong(PGPASN_CONTEXT *asnctx, PGPASN_Certificate *cert,
			PGPBoolean issuer, PGPByte *nameBuf, PGPSize *nameLength)
{
	PGPASN_Name *cname;
	PGPASN_RDNSequence *name;
	PGPASN_RelativeDistinguishedName *rdn;
	PGPASN_AttributeTypeAndValue *atv;
	PGPASN_DirectoryString *dstr;
	PGPASN_VariableBlock *vb;

	PGPBoolean sizingPass;
	PGPBoolean seenName = FALSE;
	PGPError err = kPGPError_NoErr;
	char const *oidName;
	PGPUInt32 lNameLength;
	PGPUInt32 oidLength = 0;
	PGPInt32 i, j;
	PGPInt32 twice;
	static char const dfltname[] = "(Unknown X509 name)";

	sizingPass = (nameBuf==NULL);
	lNameLength = sizingPass ? 0 : *nameLength;

	cname = issuer ? &cert->tbsCertificate.issuer
				  : &cert->tbsCertificate.subject;
	name = (PGPASN_RDNSequence *)cname->data;

	/* Make two passes, doing common name last so it shows first */
	for (twice=0; twice<2; ++twice) {
		/* Loop over elements of RDN sequence */
		for (i=0; i<name->n; ++i) {
			rdn = name->elt[i];
			for (j=0; j<rdn->n; ++j) {
				atv = rdn->elt[j];

				oidName = NULL;

				if (atv->type.len == 3
				   && atv->type.val[0] == cnoid[0]
				   && atv->type.val[1] == cnoid[1]
				   && atv->type.val[2] < sizeof(dnnames)/sizeof(dnnames[0])) {
					/* Do CN if and only if twice==1 */
					if ((atv->type.val[2] == cnoid[2]) == twice) {
						oidName = dnnames[atv->type.val[2]];
						oidLength = strlen(oidName);
					}
				}
				else if (atv->type.len == sizeof(emailoid)
					  && memcmp(atv->type.val, emailoid, atv->type.len)==0 ) {
					if (!twice) {
						oidName = emailname;
						oidLength = strlen(oidName);
					}
				}

				if( IsntNull( oidName ) ) {
					/* Look at string for value */

					pgpasn_UnpackDirectoryString( asnctx, &dstr,
										atv->value.val, atv->value.len, &err );
					if (err != 0) {
						/* Skip if unprintable string */
						err = 0;
						continue;
					}
					vb = dstr->data;

					/* Skip our inserted PGPKeyCreation string */
					if (atv->type.len == 3
					   && atv->type.val[0] == cnoid[0]
					   && atv->type.val[1] == cnoid[1]
					   && (atv->type.val[2] == 11 || atv->type.val[2] == 13)) {
						if (vb->len == sizeof(s_pgpdescr)-1 + 8 &&
							pgpMemoryEqual (vb->val, s_pgpdescr,
											sizeof(s_pgpdescr)-1) ) {
							pgpasn_FreeDirectoryString( asnctx, dstr );
							continue;
						}
					}

					seenName = TRUE;
					if (sizingPass) {
						/* add 3 for equals, comma, space */
						lNameLength += strlen(oidName) + 3;
						lNameLength += sDNQuoteString(vb->val, vb->len,
													  NULL, TRUE);
					} else {
						/* Build name string in reverse order, per LDAP */
						lNameLength -= sDNQuoteString(vb->val, vb->len,
												nameBuf+lNameLength, TRUE);
						nameBuf[--lNameLength] = '=';
						lNameLength -= oidLength;
						pgpCopyMemory( oidName, nameBuf+lNameLength,
									   oidLength );
						if( lNameLength > 0 ) {
							nameBuf[--lNameLength] = ' ';
							nameBuf[--lNameLength] = ',';
						}
					}
					pgpasn_FreeDirectoryString( asnctx, dstr );
				}
			}
		}
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

	return err;
}



/*
 * Convert from X.509 cert to PGP style name.  If issuer is true, we do
 * issuer's name, else subject name.
 * If namebuf is NULL, set *namelength to the necessary length.  Else
 * store the data into namebuf, and treat *namelength as an input parameter
 * telling what the length needs to be.
 * This version uses short form for X509 names, in PGP format.
 * E.g. just "John Doe <john@doe.com>".  If it can't be converted in that
 * form, it uses the long form ("C=US, etc.").
 */
static PGPError
sDecodeName(PGPASN_CONTEXT *asnctx, PGPASN_Certificate *cert,
			PGPBoolean issuer, PGPByte *nameBuf, PGPSize *nameLength)
{
#if !PGP_X509SHORTNAMES

	return sDecodeNameLong(asnctx, cert, issuer, nameBuf, nameLength);

#else /* PGP_X509SHORTNAMES */

	PGPASN_Name *cname;
	PGPASN_RDNSequence *name;
	PGPASN_RelativeDistinguishedName *rdn;
	PGPASN_AttributeTypeAndValue *atv;
	PGPASN_DirectoryString *dstr;
	PGPASN_GeneralNames *gnames;
	PGPASN_GeneralName *gname;
	PGPASN_VariableBlock *vb;
	PGPASN_Extensions *exts = cert->tbsCertificate.extensions;
	PGPASN_Extension *ext;

	PGPByte const *altnameoid;
	PGPBoolean sizingPass;
	PGPBoolean seenName = FALSE;
	PGPBoolean seenEmail = FALSE;
	PGPUInt32 lNameLength;
	PGPInt32 i, j;
	PGPError err = kPGPError_NoErr;

	sizingPass = (nameBuf==NULL);
	lNameLength = sizingPass ? 0 : *nameLength;

	cname = issuer ? &cert->tbsCertificate.issuer
				  : &cert->tbsCertificate.subject;
	name = (PGPASN_RDNSequence *)cname->data;

	/* Loop over elements of RDN sequence until seen email and name */
	for (i=0; (i<name->n) && !(seenEmail && seenName); ++i) {
		rdn = name->elt[i];
		for (j=0; (j<rdn->n) && !(seenEmail && seenName); ++j) {
			atv = rdn->elt[j];

			/* Parse attribute type */
			if (atv->type.len == sizeof(cnoid)
				&& memcmp(atv->type.val, cnoid, atv->type.len)==0 ) {
				/* Common name, followed by a string */
				pgpasn_UnpackDirectoryString( asnctx, &dstr, atv->value.val,
											  atv->value.len, &err );
				if (err != 0) {
					/* Skip if unprintable string */
					err = 0;
					continue;
				}
				vb = dstr->data;
				seenName = TRUE;
				if (sizingPass) {
					lNameLength += vb->len;
				} else {
					/* Put common name at beginning of buffer */
					pgpAssert (lNameLength >= vb->len);
					pgpCopyMemory (vb->val, nameBuf, vb->len);
					/* Add a space if needed */
					if (lNameLength > vb->len)
						nameBuf[vb->len] = ' ';
				}
				pgpasn_FreeDirectoryString( asnctx, dstr );
			} else if (atv->type.len == sizeof(emailoid)
					   && memcmp(atv->type.val, emailoid, atv->type.len)==0 ) {
				/* Email address, followed by a string */
				pgpasn_UnpackDirectoryString( asnctx, &dstr, atv->value.val,
											  atv->value.len, &err );
				if (err != 0) {
					/* Skip if unprintable string */
					err = 0;
					continue;
				}
				vb = dstr->data;
				seenEmail = TRUE;
				if (sizingPass) {
					/* Add 2 for angle brackets */
					lNameLength += vb->len + 2;
				} else {
					/* Put email address at end of buffer */
					pgpAssert (lNameLength >= vb->len+2);
					nameBuf[lNameLength-1] = '>';
					pgpCopyMemory (vb->val, nameBuf+lNameLength-1-vb->len,
								   vb->len);
					nameBuf[lNameLength-vb->len-2] = '<';
				}
				pgpasn_FreeDirectoryString( asnctx, dstr );
			}
		}
	}

	/* Check extensions to see if email is there, if not found yet */
	if (seenName && !seenEmail && IsntNull(exts)) {
		for (i=0; (i<exts->n) && !seenEmail; ++i) {
			ext = exts->elt[i];
			altnameoid = issuer ? issaltnameoid : subaltnameoid;
			/* These are same size */
			if (ext->extnID.len == sizeof(issaltnameoid)
				&& memcmp(ext->extnID.val, altnameoid, ext->extnID.len)==0 ) {
				/* Alternative name, just as we wanted */
				pgpasn_UnpackGeneralNames( asnctx, &gnames, ext->extnValue.val,
										   ext->extnValue.len, &err );
				if (err != 0) {
					/* Skip if unprintable string */
					err = 0;
					continue;
				}
				for (j=0; (j<gnames->n) && !seenEmail; ++j) {
					gname = gnames->elt[j];
					if (gname->CHOICE_field_type != ASN_GENERALNAME_RFC822NAME)
						continue;
					/* Okay, we have an email address */
					vb = gname->data;
					seenEmail = TRUE;
					if (sizingPass) {
						/* Add 2 for angle brackets */
						lNameLength += vb->len + 2;
					} else {
						/* Put email address at end of buffer */
						pgpAssert (lNameLength >= vb->len+2);
						nameBuf[lNameLength-1] = '>';
						pgpCopyMemory (vb->val, nameBuf+lNameLength-1-vb->len,
									   vb->len);
						nameBuf[lNameLength-vb->len-2] = '<';
					}
				}
				pgpasn_FreeGeneralNames( asnctx, gnames );
			}
		}
	}

	if (!seenName || !seenEmail) {
		/* If we could not parse the name, use the long form */
		return sDecodeNameLong(asnctx, cert, issuer, nameBuf, nameLength);
	}

	if( sizingPass ) {
		/* Count a space between name and email if have both */
		if (seenName && seenEmail)
			++lNameLength;
		*nameLength = lNameLength;
	}

	return err;

#endif /* PGP_X509SHORTNAMES */
}



/*
 * Create a PGP public key packet corresponding to the certificate, or a
 * private key packet corresponding to the PrivateKeyInfo.  Only one of
 * these two pointers should be non-NULL.
 *
 * If keybuf is NULL, set *keylength to the necessary length.  Else
 * store the data into keybuf, and treat *keylength as an input parameter
 * telling what the length needs to be.
 */
static PGPError
sDecodeKey(PGPContextRef context, PGPASN_CONTEXT *asnctx,
	PGPASN_Certificate *cert, PGPASN_PrivateKeyInfo *pinfo,
	PGPByte *keyBuf, PGPSize *keyLength)
{
	PGPASN_SubjectPublicKeyInfo *spki =
					&cert->tbsCertificate.subjectPublicKeyInfo;
	PGPASN_DssParms			*dssp = NULL;
	PGPASN_DSAPublicKey		*dssk = NULL;
	PGPASN_ELGParms			*elgp = NULL;
	PGPASN_ELGPublicKey		*elgk = NULL;
	PGPASN_RSAKey			*rsak = NULL;
	PGPASN_PrivateKey		*pkey;
	PGPASN_RSAPrivateKey	*rsapriv = NULL;

	PGPPublicKeyAlgorithm keyalgorithm;
	PGPByte *bnbuf[6];		/* buf offset for start of X.509 bignum */
	PGPSize bnlen[6];		/* buf length for X.509 bignum */
	PGPSize bnlength[6];	/* length of PGP encoded bignum */
	PGPUInt32 pubelems=0;	/* Number of bignum elements that are public */
	PGPUInt32 i, j, o;
	PGPUInt16 checksum;
	PGPSize keybuflength;
	PGPByte *pubbuf = NULL;
	PGPSize publength;
	PGPByte keyversion;
	PGPByte const *matchkeybuf = NULL;
	PGPSize matchkeybuflength;
	RingObject *matchkey;
	PGPMemoryMgrRef mgr = PGPGetContextMemoryMgr( context );
	RingPool *pool = pgpContextGetRingPool( context );
	PGPByte hash20[20];
	PGPError err = kPGPError_NoErr;

	/* Init bignum arrays */
	pgpClearMemory (bnbuf, sizeof(bnbuf));
	pgpClearMemory (bnlen, sizeof(bnlen));
	pgpClearMemory (bnlength, sizeof(bnlength));

	if (IsntNull( pinfo ) ) {
		/* Do private key */

		/* Decode object identifier for RSA/DSS/ElGamal */
		err = sDecodeAlgorithmOID ( asnctx,
									&pinfo->privateKeyAlgorithm.algorithm,
									&keyalgorithm, NULL );
		if( IsPGPError(err) )
			goto error;

		if (keyalgorithm == kPGPPublicKeyAlgorithm_RSA) {
			pkey = &pinfo->privateKey;
			pgpasn_UnpackRSAPrivateKey( asnctx, &rsapriv,
										pkey->val, pkey->len, &err);
			CHKASNERR(err);

			/* modulus n */
			bnbuf[0] = rsapriv->modulus.val;
			bnlen[0] = rsapriv->modulus.len;
			err = sDecodeX509Integer (bnbuf[0], bnlen[0], NULL, &bnlength[0]);
			if( IsPGPError( err ) )
				goto error;

			/* public exponent e */
			bnbuf[1] = rsapriv->publicExponent.val;
			bnlen[1] = rsapriv->publicExponent.len;
			err = sDecodeX509Integer (bnbuf[1], bnlen[1], NULL, &bnlength[1]);
			if( IsPGPError( err ) )
				goto error;

			/* private exponent d */
			bnbuf[2] = rsapriv->privateExponent.val;
			bnlen[2] = rsapriv->privateExponent.len;
			err = sDecodeX509Integer (bnbuf[2], bnlen[2], NULL, &bnlength[2]);
			if( IsPGPError( err ) )
				goto error;

			/* smaller prime */
			bnbuf[3] = rsapriv->prime2.val;
			bnlen[3] = rsapriv->prime2.len;
			err = sDecodeX509Integer (bnbuf[3], bnlen[3], NULL, &bnlength[3]);
			if( IsPGPError( err ) )
				goto error;

			/* larger prime */
			bnbuf[4] = rsapriv->prime1.val;
			bnlen[4] = rsapriv->prime1.len;
			err = sDecodeX509Integer (bnbuf[4], bnlen[4], NULL, &bnlength[4]);
			if( IsPGPError( err ) )
				goto error;

			/* inverse u of smaller prime mod larger prime */
			bnbuf[5] = rsapriv->coefficient.val;
			bnlen[5] = rsapriv->coefficient.len;
			err = sDecodeX509Integer (bnbuf[5], bnlen[5], NULL, &bnlength[5]);
			if( IsPGPError( err ) )
				goto error;

			/* Number of these values which are public */
			pubelems = 2;

		} else if (keyalgorithm == kPGPPublicKeyAlgorithm_ElGamal) {
			pgpAssert (0);
		} else /* DSS */ {
			pgpAssert (0);
		}

	} else {
		/* Do public key */
		/* Decode object identifier for RSA/DSS/ElGamal */
		err = sDecodeAlgorithmOID ( asnctx, &spki->algorithm.algorithm,
									&keyalgorithm, NULL );
		if( IsPGPError(err) )
			goto error;

		if (keyalgorithm == kPGPPublicKeyAlgorithm_DSA) {
			pgpasn_UnpackDssParms ( asnctx, &dssp,
									spki->algorithm.parameters->val,
									spki->algorithm.parameters->len, &err );
			CHKASNERR( err );
			pgpasn_UnpackDSAPublicKey ( asnctx, &dssk,
										spki->subjectPublicKey.val,
										spki->subjectPublicKey.len, &err );
			CHKASNERR( err );

			/* prime p */
			bnbuf[0] = dssp->p.val;
			bnlen[0] = dssp->p.len;
			err = sDecodeX509Integer (bnbuf[0], bnlen[0], NULL, &bnlength[0]);
			if( IsPGPError( err ) )
				goto error;

			/* subprime q */
			bnbuf[1] = dssp->q.val;
			bnlen[1] = dssp->q.len;
			err = sDecodeX509Integer (bnbuf[1], bnlen[1], NULL, &bnlength[1]);
			if( IsPGPError( err ) )
				goto error;

			/* generator g */
			bnbuf[2] = dssp->g.val;
			bnlen[2] = dssp->g.len;
			err = sDecodeX509Integer (bnbuf[2], bnlen[2], NULL, &bnlength[2]);
			if( IsPGPError( err ) )
				goto error;

			/* public value y */
			bnbuf[3] = dssk->val;
			bnlen[3] = dssk->len;
			err = sDecodeX509Integer (bnbuf[3], bnlen[3], NULL, &bnlength[3]);
			if( IsPGPError( err ) )
				goto error;

			pubelems = 4;

		} else if (keyalgorithm == kPGPPublicKeyAlgorithm_ElGamal) {
			pgpasn_UnpackELGParms ( asnctx, &elgp,
									spki->algorithm.parameters->val,
									spki->algorithm.parameters->len, &err );
			CHKASNERR( err );
			pgpasn_UnpackELGPublicKey ( asnctx, &elgk,
										spki->subjectPublicKey.val,
										spki->subjectPublicKey.len, &err );
			CHKASNERR( err );

			/* prime p */
			bnbuf[0] = elgp->p.val;
			bnlen[0] = elgp->p.len;
			err = sDecodeX509Integer (bnbuf[0], bnlen[0], NULL, &bnlength[0]);
			if( IsPGPError( err ) )
				goto error;

			/* generator g */
			bnbuf[1] = elgp->g.val;
			bnlen[1] = elgp->g.len;
			err = sDecodeX509Integer (bnbuf[1], bnlen[1], NULL, &bnlength[1]);
			if( IsPGPError( err ) )
				goto error;

			/* public value y */
			bnbuf[2] = elgk->val;
			bnlen[2] = elgk->len;
			err = sDecodeX509Integer (bnbuf[2], bnlen[2], NULL, &bnlength[2]);
			if( IsPGPError( err ) )
				goto error;

			pubelems = 3;

		} else /* RSA */ {
			pgpasn_UnpackRSAKey ( asnctx, &rsak, spki->subjectPublicKey.val,
								  spki->subjectPublicKey.len, &err );
			CHKASNERR( err );

			/* modulus n */
			bnbuf[0] = rsak->modulus.val;
			bnlen[0] = rsak->modulus.len;
			err = sDecodeX509Integer (bnbuf[0], bnlen[0], NULL, &bnlength[0]);
			if( IsPGPError( err ) )
				goto error;

			/* exponent e */
			bnbuf[1] = rsak->exponent.val;
			bnlen[1] = rsak->exponent.len;
			err = sDecodeX509Integer (bnbuf[1], bnlen[1], NULL, &bnlength[1]);
			if( IsPGPError( err ) )
				goto error;
			pubelems = 2;
		}
	}

	keybuflength = 0;
	publength = 0;
	for (i=0; i<elemsof(bnlength); ++i) {
		if( bnlength[i] == 0 )
			break;
		if (i < pubelems)
			publength += bnlength[i];
		keybuflength += bnlength[i];
	}

	/*
	 * To better estimate keybuflength, we need to construct the numeric
	 * data portion and then see if it matches an incoming key.  Only then
	 * can we know what kind of key packet we are dealing with.
	 */

	pubbuf = PGPNewData (mgr, publength, 0);
	CHKNONNULL(pubbuf, err);
	o = 0;
	for (i=0; i<pubelems; ++i) {
		if( bnlength[i] == 0 )
			break;
		err = sDecodeX509Integer (bnbuf[i], bnlen[i],
								  pubbuf+o, &bnlength[i]);
		if( IsPGPError( err ) )
			goto error;
		o += bnlength[i];
	}
	pgpAssert (o == publength);

	/* Get hash of numeric data */
	pgpFingerprint20HashBuf (context, pubbuf, publength, hash20);

	/* See if this matches an existing key */
	matchkey = ringPoolFindKey20n (pool, hash20);

	if( IsNull( matchkey ) ) {
		/* Fall back on defaults */
		if (keyalgorithm == kPGPPublicKeyAlgorithm_RSA)
			keyversion = PGPVERSION_3;
		else
			keyversion = PGPVERSION_4;
	} else {
		RingSet *allset;
		allset = ringSetCreateUniversal (pool);
		if( IsNull( allset ) ) {
			err = ringPoolError(pool)->error;
			goto error;
		}
		matchkeybuf = ringFetchObject (allset, matchkey, &matchkeybuflength);
		ringSetDestroy( allset );
		if( IsNull( matchkeybuf ) ) {
			ringSetDestroy( allset );
			err = ringPoolError(pool)->error;
			goto error;
		}
		keyversion = matchkeybuf[0];
	}

	keybuflength += 6;	/* bytes preceding bignums */
	if( keyversion <= PGPVERSION_3)
			keybuflength += 2;	/* room for expiration date */

	if( IsntNull( pinfo ) ) {
		/*
		 * Can only import private if we have a matching key,
		 * because we can't synthesize a userid for it
		 */
		if( IsNull( matchkey ) ) {
			err = kPGPError_PublicKeyNotFound;
			goto error;
		}
		keybuflength += 3;		/* Private adds 1 enc byte + 2 csum */
	}

	if( IsNull( keyBuf ) ) {
		*keyLength = keybuflength;
	} else {
		pgpAssert (*keyLength == keybuflength);
		/* public key packet */
		o = 0;
		keyBuf[o++] = keyversion;
		/* 1-4 are timestamp */
		if (IsntNull( matchkey ) ) {
			pgpCopyMemory (matchkeybuf+1, keyBuf+o, 4);
		} else {
			PGPUInt32 creation = sDecodeKeyCreation( asnctx, cert );
			if( creation != 0 ) {
				keyBuf[o+0] = (creation >> 24) & 0xff;
				keyBuf[o+1] = (creation >> 16) & 0xff;
				keyBuf[o+2] = (creation >>  8) & 0xff;
				keyBuf[o+3] = (creation >>  0) & 0xff;
			} else {
				/* Use certificate creation date if all else fails */
				err = sDecodeNotBefore4( asnctx, cert, keyBuf+o );
				if (IsPGPError( err ) ) {
					pgpClearMemory (keyBuf+o, 4);
					err = kPGPError_NoErr;
				}
			}
		}
		o += 4;
		/* Next two are expiration if V3 packets */
		if (keyversion <= PGPVERSION_3) {
			if (IsntNull( matchkey ) ) {
				pgpCopyMemory (matchkeybuf+5, keyBuf+o, 2);
			} else {
				pgpClearMemory (keyBuf+o, 2);
			}
			o += 2;
		}
		keyBuf[o++] = keyalgorithm;
		pgpCopyMemory (pubbuf, keyBuf+o, publength);
		o += publength;
		PGPFreeData( pubbuf );
		pubbuf = NULL;

		if( IsntNull( pinfo ) ) {
			/* Save secret key fields */
			keyBuf[o++] = '\0';		/* Unencrypted secret data */
			checksum = 0;
			for (i=pubelems; i<elemsof(bnlength); ++i) {
				if( bnlength[i] == 0 )
					break;
				err = sDecodeX509Integer (bnbuf[i], bnlen[i],
										  keyBuf+o, &bnlength[i]);
				if( IsPGPError( err ) )
					goto error;
				for (j=0; j<bnlength[i]; ++j)
					checksum += keyBuf[o+j];
				o += bnlength[i];
			}
			keyBuf[o++] = (checksum >> 8) & 0xff;
			keyBuf[o++] = (checksum >> 0) & 0xff;
		}

		pgpAssert (o == keybuflength);
	}

error:

	if( IsntNull( pubbuf ) )
		PGPFreeData( pubbuf );
	if( IsntNull( dssp ) )
		pgpasn_FreeDssParms( asnctx, dssp );
	if( IsntNull( dssk ) )
		pgpasn_FreeDSAPublicKey( asnctx, dssk );
	if( IsntNull( elgp ) )
		pgpasn_FreeELGParms( asnctx, elgp );
	if( IsntNull( elgk ) )
		pgpasn_FreeELGPublicKey( asnctx, elgk );
	if( IsntNull( rsak ) )
		pgpasn_FreeRSAKey( asnctx, rsak );
	if( IsntNull( rsapriv ) )
		pgpasn_FreeRSAPrivateKey( asnctx, rsapriv );

	return err;
}


/*
 * Given an X509 cert, return a PGPKeySetRef containing a simple PGP key
 * with the same key material as the original cert.  The userid on the
 * key is one of the 509 subject distinguished name fields, the common name
 * if it exists, else one of the other fields.
 */
static PGPError
sDecode509(PGPContextRef context, PGPASN_CONTEXT *asnctx,
		   PGPASN_Certificate *cert, PGPKeySetRef *kset)
{
	PGPMemoryMgrRef mgr;
	PGPByte *pgpkeybuf = NULL;
	PGPSize keylen;
	PGPSize namelen;
	PGPSize siglen;
	PGPSize pgpkeylen;
	PGPUInt32 o;
	PGPKeySetRef keyset;
	PGPError err = kPGPError_NoErr;

	*kset = NULL;
	mgr = PGPGetContextMemoryMgr( context );

	/* Decode signature packet */
	err = sDecodeSig (context, asnctx, cert, SIGSUBSUB_X509_VERSION_LO,
					  NULL, &siglen);
	if (IsPGPError(err))
		goto error;

	/* Decode subject name */
	err = sDecodeName (asnctx, cert, FALSE, NULL, &namelen);
	if (IsPGPError(err))
		goto error;
	
	/* Decode key packet */
	err = sDecodeKey (context, asnctx, cert, NULL, NULL, &keylen);
	if (IsPGPError(err))
		goto error;

	/* Now create the PGP key */

	pgpkeylen = 3 + keylen + 3 + namelen + ((siglen>0)?(3+siglen):0);
	pgpkeybuf = (PGPByte *) PGPNewData( mgr, pgpkeylen, 0 );
	CHKNONNULL (pgpkeybuf, err);

	/* public key packet */
	o = 0;
	pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1),
	pgpkeybuf[o++] = (keylen >> 8) & 0xff;
	pgpkeybuf[o++] = (keylen >> 0) & 0xff;
	err = sDecodeKey (context, asnctx, cert, NULL, pgpkeybuf + o, &keylen);
	if (IsPGPError(err))
		goto error;
	o += keylen;

	/* userid packet */
	pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_NAME, 1);
	pgpkeybuf[o++] = (namelen >> 8) & 0xff;
	pgpkeybuf[o++] = (namelen >> 0) & 0xff;
	err = sDecodeName (asnctx, cert, FALSE, pgpkeybuf + o, &namelen);
	if (IsPGPError(err))
		goto error;
	o += namelen;

	/* sig packet */
	pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_SIG, 1);
	pgpkeybuf[o++] = (siglen >> 8) & 0xff;
	pgpkeybuf[o++] = (siglen >> 0) & 0xff;
	err = sDecodeSig (context, asnctx, cert, SIGSUBSUB_X509_VERSION_LO,
					  pgpkeybuf + o, &siglen);
	if (IsPGPError(err))
		goto error;
	o += siglen;

	pgpAssert (o == pgpkeylen);

	err = pgpImportKeyBinary( context, pgpkeybuf, pgpkeylen, &keyset );
	if( IsPGPError( err ) )
		goto error;

	*kset = keyset;
	/* Fall through */
error:

	if( IsntNull(pgpkeybuf) )
		PGPFreeData( pgpkeybuf );
	return err;
}


/* Entry point to import a 509 cert from the specified location */
	PGPError
pgpDecodeX509Cert( PGPByte *buf, PGPSize len, PGPContextRef context,
				   PGPKeySetRef *keys )
{
	PGPASN_Certificate *cert;
	PGPMemoryMgrRef	mgr;
	PGPASN_CONTEXT	asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPError		err = kPGPError_NoErr;
	
	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificate( asnctx, &cert, buf, len, &err );
	CHKASNERR( err )

	err = sDecode509( context, asnctx, cert, keys);

	pgpasn_FreeCertificate( asnctx, cert );

error:

	return err;
}

/* Entry point to import a SET OF X509 certs */
	PGPError
pgpDecodeX509CertSet(PGPByte *buf, PGPSize len, PGPContextRef context,
					 PGPKeySetRef *kset)
{
	PGPASN_Certificates *certs = NULL;
	PGPASN_Certificate *cert;
	PGPMemoryMgrRef	mgr;
	PGPASN_CONTEXT	asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPKeySetRef	newkset = NULL;
	PGPKeySetRef	tmpkset = NULL;
	PGPInt32		i;
	PGPError		err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);
	pgpAssert( IsntNull( kset ) );
	*kset = NULL;

	if( IsPGPError( err = PGPNewKeySet( context, &newkset ) ) )
		goto error;

	pgpasn_UnpackCertificates( asnctx, &certs, buf, len, &err );
	CHKASNERR( err );

	for (i=0; i<certs->n; ++i) {
		cert = certs->elt[i];
		err = sDecode509( context, asnctx, cert, &tmpkset );
		CHKASNERR( err );
		PGPAddKeys( tmpkset, newkset );
		PGPCommitKeyRingChanges( newkset );
		PGPFreeKeySet( tmpkset );
		tmpkset = NULL;
	}

error:
	if( IsntNull( certs ) )
		pgpasn_FreeCertificates( asnctx, certs );
	if( IsntNull( tmpkset ) )
		PGPFreeKeySet( tmpkset );

	if( IsntPGPError( err ) )
		*kset = newkset;
	else if( IsntNull( newkset ) )
		PGPFreeKeySet( newkset );

	return err;
}


/* Entry point to import a SET OF X509 CRLs */
	PGPError
pgpDecodeX509CRLSet(PGPByte *buf, PGPSize len,
					PGPContextRef context, PGPKeySet *kset)
{
	PGPASN_CertificateRevocationLists *crls = NULL;
	PGPASN_CertificateRevocationList *crl;
	PGPASN_Extension *ext;
	PGPASN_DistributionPointName *dpname;
	PGPASN_GeneralNames *gnames;
	PGPASN_IssuingDistributionPoint *dpt = NULL;
	PGPMemoryMgrRef	mgr;
	PGPASN_CONTEXT	asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	RingPool *pool = pgpContextGetRingPool( context );
	RingSet *newset = NULL;
	RingObject *crlkey;
	RingObject *crlobj;
	PGPPubKey *pubkey = NULL;
	PGPByte *pgpcrlbuf = NULL;
	PGPSize pgpcrllen;
	PGPByte *crlbuf = NULL;
	PGPSize crllen;
	PGPByte *namebuf = NULL;
	PGPSize namebuflen;
	PGPByte *dpoint = NULL;		/* Dynamically allocated */
	PGPSize dpointlen;
	PGPInt32		i, j;
	PGPError		err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificateRevocationLists( asnctx, &crls, buf, len, &err );
	CHKASNERR( err );

	newset = ringSetCreate( pool );
	if( IsNull( newset ) ) {
		err = ringPoolError(pool)->error;
		goto error;
	}

	for (i=0; i<crls->n; ++i) {
		crl = crls->elt[i];
		/* Look up CRL to see if we have issuing key */
		namebuflen = pgpasn_SizeofName( asnctx,
										&crl->tbsCertList.issuer, TRUE );
		namebuf = (PGPByte *) PGPNewData( mgr, namebuflen, 0 );
		CHKNONNULL( namebuf, err );
		pgpasn_PackName (asnctx, namebuf, namebuflen, &crl->tbsCertList.issuer,
						 &err );
		CHKASNERR( err );

		crlkey = ringPoolFindX509NamedSig (pool, namebuf, namebuflen);
		PGPFreeData( namebuf );
		namebuf = NULL;

		if( IsNull( crlkey ) ) {
			/* Don't have a key the CRL is for, skip it! */
			continue;
		}

		ringSetAddObject( newset, crlkey );

		/* Actualy the above call returns a sig on the key we want */
		pgpAssert( OBJISSIG( crlkey ) );
		while (!OBJISTOPKEY( crlkey ) )
			crlkey = crlkey->g.up;

		/* See if CRL is properly signed */
		pubkey = ringKeyPubKey(newset, crlkey, PGP_PKUSE_SIGN);
		if (!pubkey) {
			/* Can't verify this sig for some reason, try next */
			continue;
		}
		crllen = pgpasn_SizeofCertificateRevocationList( asnctx, crl, TRUE );
		crlbuf = (PGPByte *) PGPNewData( mgr, crllen, TRUE );
		CHKNONNULL( crlbuf, err );
		pgpasn_PackCertificateRevocationList( asnctx, crlbuf, crllen, crl,
											  &err );
		CHKASNERR( err );
		err = pgpX509VerifySignedObject( context, pubkey, crlbuf, crllen );
		PGPFreeData( crlbuf );
		crlbuf = NULL;
		pgpPubKeyDestroy( pubkey );
		pubkey = NULL;

		if( IsPGPError( err ) ) {
			/* Couldn't verify sig, skip CRL */
			err = kPGPError_NoErr;
			continue;
		}

		/* See if CRL has distribution point */
		dpoint = NULL;
		dpointlen = 0;
		if( IsntNull( crl->tbsCertList.crlExtensions ) ) {
			for (j=0; j<crl->tbsCertList.crlExtensions->n; ++j) {
				ext = crl->tbsCertList.crlExtensions->elt[j];
				if (ext->extnID.len == sizeof(issdistpoint)
					&& memcmp(ext->extnID.val, issdistpoint,
							  ext->extnID.len)==0 ) {
					/* Parse distribution point */
					pgpasn_UnpackIssuingDistributionPoint ( asnctx, &dpt,
								ext->extnValue.val, ext->extnValue.len, &err );
					if( err == 0 &&
								IsntNull( dpt->distributionPoint ) ) {
						dpname = dpt->distributionPoint;
						if( dpname->CHOICE_field_type ==
											ASN_DPNAME_GENERALNAMES) {
							gnames = dpname->data;
							dpointlen = pgpasn_SizeofGeneralNames( asnctx,
															gnames, TRUE);
							dpoint = PGPNewData( mgr, dpointlen, 0 );
							CHKNONNULL( dpoint, err );
							pgpasn_PackGeneralNames( asnctx, dpoint, dpointlen,
													gnames, &err );
							break;
						}
					}
					if( IsntNull( dpt ) )
						pgpasn_FreeIssuingDistributionPoint( asnctx, dpt );
					dpt = NULL;
				}
			}
		}

		pgpcrllen = pgpasn_SizeofCertificateRevocationList( asnctx,
															crl, TRUE );
		/* Two header bytes in CRL packet, plus optional dpoint data */
		pgpcrlbuf = (PGPByte *) PGPNewData( mgr,
							pgpcrllen+2+(dpointlen?dpointlen+4:0), 0 );
		CHKNONNULL( pgpcrlbuf, err );

		/* CRL data */
		pgpcrlbuf[0] = PGPVERSION_4;
		pgpcrlbuf[1] = dpointlen ? PGPCRLTYPE_X509DPOINT : PGPCRLTYPE_X509;
		/* Optional distribution point data */
		if (dpointlen) {
			pgpcrlbuf[2] = (dpointlen>>24) & 0xff;
			pgpcrlbuf[3] = (dpointlen>>16) & 0xff;
			pgpcrlbuf[4] = (dpointlen>> 8) & 0xff;
			pgpcrlbuf[5] = (dpointlen>> 0) & 0xff;
			pgpCopyMemory( dpoint, pgpcrlbuf+6, dpointlen );
			PGPFreeData( dpoint );
			dpoint = NULL; 
		}
		pgpasn_PackCertificateRevocationList( asnctx,
			pgpcrlbuf+2+(dpointlen?dpointlen+4:0), pgpcrllen, crl, &err );
		CHKASNERR( err );

		crlobj = ringCreateCRL( newset, crlkey, pgpcrlbuf,
								pgpcrllen+2+(dpointlen?dpointlen+4:0) );
		if( IsNull( crlobj ) ) {
			err = ringPoolError(pool)->error;
			goto error;
		}
		crlobj->r.trust = PGP_SIGTRUSTF_CHECKED_TRIED;
	}

	/* Add newly created objects to target keydb */
	ringSetFreeze( newset );
    err = kset->keyDB->add(kset->keyDB, newset);
	
error:
	if( IsntNull( pubkey ) )
		pgpPubKeyDestroy( pubkey );
	if( IsntNull( newset ) )
		ringSetDestroy( newset );
	if( IsntNull( pgpcrlbuf ) )
		PGPFreeData( pgpcrlbuf );
	if( IsntNull( namebuf ) )
		PGPFreeData( namebuf );
	if( IsntNull( crlbuf ) )
		PGPFreeData( crlbuf );
	if( IsntNull( dpoint ) )
		PGPFreeData( dpoint );
	if( IsntNull( crls ) )
		pgpasn_FreeCertificateRevocationLists( asnctx, crls );
	if( IsntNull( dpt ) )
		pgpasn_FreeIssuingDistributionPoint( asnctx, dpt );

	return err;
}


/*
 * Entry point to import a PKCS8 format private key into a new keyset.
 * There must be an existing public key in the ringpool already, because
 * pkcs-8 does not give us any userid info.
 */
	PGPError
pgpDecodePCKS8( PGPByte *buf, PGPSize len, PGPContextRef context,
				   PGPKeySetRef *keys )
{
	PGPASN_PrivateKeyInfo *pinfo = NULL;
	PGPMemoryMgrRef	 mgr;
	PGPASN_CONTEXT	 asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPByte			*pgpkeybuf = NULL;
	PGPSize			 keylen;
	PGPSize			 pgpkeylen;
	PGPUInt32		 o;
	PGPKeySetRef	 keyset;
	PGPError		 err = kPGPError_NoErr;
	
	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackPrivateKeyInfo( asnctx, &pinfo, buf, len, &err );
	CHKASNERR(err);

	/* Decode key packet */
	err = sDecodeKey (context, asnctx, NULL, pinfo, NULL, &keylen);
	if (IsPGPError(err))
		goto error;

	/* Import a bare PGP key - we already have a public version of it */

	pgpkeylen = 3 + keylen;
	pgpkeybuf = (PGPByte *) PGPNewData( mgr, pgpkeylen, 0 );
	CHKNONNULL (pgpkeybuf, err);

	/* public key packet */
	o = 0;
	pgpkeybuf[o++] = PKTBYTE_BUILD(PKTBYTE_SECKEY, 1),
	pgpkeybuf[o++] = (keylen >> 8) & 0xff;
	pgpkeybuf[o++] = (keylen >> 0) & 0xff;
	err = sDecodeKey (context, asnctx, NULL, pinfo, pgpkeybuf + o, &keylen);
	if (IsPGPError(err))
		goto error;
	o += keylen;

	pgpAssert (o == pgpkeylen);

	err = pgpImportKeyBinary( context, pgpkeybuf, pgpkeylen, &keyset );
	if( IsPGPError( err ) )
		goto error;

	*keys = keyset;
	/* Fall through */
error:

	if( IsntNull( pinfo ) )
		pgpasn_FreePrivateKeyInfo( asnctx, pinfo );

	if( IsntNull(pgpkeybuf) )
		PGPFreeData( pgpkeybuf );

	return err;
}




/* Return true if two PGPASN_Name objects are the same */
static PGPBoolean
sNamesEqual( PGPASN_Name *n1, PGPASN_Name *n2 )
{
	PGPASN_RDNSequence *rdns1, *rdns2;
	PGPASN_RelativeDistinguishedName *rdn1, *rdn2;
	PGPASN_AttributeTypeAndValue *atv1, *atv2;
	PGPInt32 i, j;
	
	rdns1 = n1->data;
	rdns2 = n2->data;

	if (rdns1->n != rdns2->n)
		return FALSE;
	for (i=0; i<rdns1->n; ++i) {
		rdn1 = rdns1->elt[i];
		rdn2 = rdns2->elt[i];
		if (rdn1->n != rdn2->n)
			return FALSE;
		for (j=0; j<rdn1->n; ++j) {
			atv1 = rdn1->elt[j];
			atv2 = rdn2->elt[j];
			if (atv1->type.len != atv2->type.len)
				return FALSE;
			if (atv1->value.len != atv2->value.len)
				return FALSE;
			if (!pgpMemoryEqual (atv1->type.val, atv2->type.val,
								 atv1->type.len))
				return FALSE;
			if (!pgpMemoryEqual (atv1->value.val, atv2->value.val,
								 atv2->value.len))
				return FALSE;
		}
	}
	return TRUE;
}


/*
 * Given a distinguished name (e.g. an issuer name from a cert) search
 * the certs list and try to find a cert which has the given name as a
 * subject.  Return the cert object.  Return 0 if no cert matches.
 */
static PGPError
sLookup509 (PGPASN_CONTEXT *asnctx, PGPByte *certs, PGPASN_Name *name,
			PGPASN_Certificate **subjectcert)
{
	PGPASN_Certificate *cert = NULL;
	PGPUInt32	totallen;
	PGPUInt32	certlen;
	PGPError	err = kPGPError_NoErr;

	*subjectcert = NULL;

	totallen = dolen3 (&certs);
	while (totallen > 0) {
		certlen = dolen3 (&certs);
		
		pgpasn_UnpackCertificate( asnctx, &cert, certs, certlen, &err );
		CHKASNERR( err );

		if( sNamesEqual ( &cert->tbsCertificate.subject, name ) ) {
			/* Have a match */
			*subjectcert = cert;
			return kPGPError_NoErr;
		}

		pgpasn_FreeCertificate( asnctx, cert );
		cert = NULL;

		totallen -= certlen + 3;
		certs += certlen;
	}
	return kPGPError_ItemNotFound;
error:

	if( IsntNull( cert ) )
		pgpasn_FreeCertificate( asnctx, cert );

	return err;
}


/*
 * Verify an X.509 signed object
 * Should be a SEQUENCE OF three parts: signed data, signature algorithm,
 * and signature.  This could be a cert or CRL, for example.
 */
PGPError
pgpX509VerifySignedObject (PGPContextRef context, PGPPubKey const *pubkey,
						   PGPByte *buf, PGPSize len)
{
	PGPMemoryMgrRef	mgr;
	PGPASN_CONTEXT	asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPASN_XCertificate *obj509 = NULL;
	PGPHashAlgorithm hashalg;
	PGPHashContextRef hc;
	PGPInt32 rslt;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackXCertificate( asnctx, &obj509, buf, len, &err );
	CHKASNERR( err );

	/* Figure out the hash algorithm to use */
	err = sDecodeAlgorithmOID (asnctx, &obj509->signatureAlgorithm.algorithm,
							   NULL, &hashalg);
	
	/* Hash data to check sig */
	hc = pgpHashCreate( PGPGetContextMemoryMgr( context ),
						pgpHashByNumber( hashalg ) );
	if( IsNull( hc ) ) {
		err = kPGPError_BadHashNumber;
		goto error;
	}
	PGPContinueHash( hc, obj509->tbsCertificate.val,
					 obj509->tbsCertificate.len );

	/* Verify the signature: rslt==1 means OK, else failure */
	rslt = pgpPubKeyVerify( pubkey,
							obj509->signature.val, obj509->signature.len,
							pgpHashGetVTBL(hc),
							(PGPByte *) pgpHashFinal(hc),
							kPGPPublicKeyMessageFormat_X509 );

	PGPFreeHashContext (hc);
	
	if( rslt != 1 ) {
		err = kPGPError_X509InvalidCertificateSignature;
	}

error:

	if( IsntNull( obj509 ) )
		pgpasn_FreeXCertificate( asnctx, obj509 );
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
	PGPMemoryMgrRef	mgr;
	PGPASN_CONTEXT	asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPASN_Certificate *cert;
	PGPASN_Certificate *parentcert;

	PGPBoolean matchedinroot;
	PGPByte *childcert;
	PGPUInt32 certchainlen;
	PGPUInt32 certlen;
	PGPKeySetRef parentkeyset;
	PGPKeyListRef keylist;
	PGPKeyIterRef keyiter;
	PGPKeyRef parentkey;
	PGPPubKey *pubkey = NULL;
	RingSet const *parentringset = NULL;
	PGPError err = kPGPError_NoErr;
	
	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	childcert = certchain;
	certchainlen = dolen3 (&childcert);
	certlen = dolen3 (&childcert);
	pgpasn_UnpackCertificate( asnctx, &cert, childcert, certlen, &err );
	CHKASNERR( err );
		
	do {
		/* See if we can match issuer in root list */
		err = sLookup509 (asnctx, rootcerts, &cert->tbsCertificate.issuer,
						  &parentcert);
		matchedinroot = IsntNull( parentcert );
		if( IsNull( parentcert ) ) {
			/* Failing that, see if we were given a parent cert */
			err = sLookup509 (asnctx, certchain, &cert->tbsCertificate.issuer,
							  &parentcert);
		}
		if( IsNull( parentcert ) ) {
			/* Unable to complete the search, no parent available */
			return (err == kPGPError_ItemNotFound) ?
								kPGPError_X509NeededCertNotAvailable : err;
		}

		/* Parse parent cert to get key data */
		err = sDecode509 (context, asnctx, parentcert, &parentkeyset);
		if (IsPGPError(err))
			goto error;

		/* Get just the one key out of it */
		PGPOrderKeySet( parentkeyset, kPGPAnyOrdering, &keylist );
		PGPNewKeyIter( keylist, &keyiter );
		PGPKeyIterNext( keyiter, &parentkey );
		PGPFreeKeyIter( keyiter );
		PGPFreeKeyList( keylist );
		
		/* Prepare to check signature */
		parentringset = pgpKeyDBRingSet (parentkey->keyDB);
		pubkey = ringKeyPubKey(parentringset, parentkey->key, PGP_PKUSE_SIGN);
		if (!pubkey) {
			err = ringSetError(parentringset)->error;
			goto error;
		}
			
		err = pgpX509VerifySignedObject( context, pubkey, childcert, certlen );
		pgpPubKeyDestroy( pubkey );
		pubkey = NULL;
		PGPFreeKeySet( parentkeyset );
		if( IsPGPError( err ) )
			goto error;

		/* Check for self-signed certificate after verifying sig */
		if( sNamesEqual( &cert->tbsCertificate.issuer,
						 &cert->tbsCertificate.subject ) ) {
			return kPGPError_X509SelfSignedCert;
		}

		/* Else try to validate parent cert */
		cert = parentcert;

	} while (!matchedinroot);

	return kPGPError_NoErr;	/* Success */

error:
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

/* Parse past signature bitstring returning raw data */

PGPError
pgpX509SignatureBitString( PGPByte *buf, PGPSize len,
	PGPByte **rawbuf, PGPSize *rawlen )
{
	PGPUInt32 tag;
	PGPError err = kPGPError_NoErr;

	tag = dotaglen(&buf, &len);
	EXPECT(tag, TAG_BITSTRING);
	EXPECT(*buf, 0);
	++buf;
	--len;
	*rawbuf = buf;
	*rawlen = len;

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
 * Given a buffer, return an X.509 cert.
 */
PGPError
pgpX509BufferToCert( PGPContextRef context, PGPByte *buf, PGPSize len,
					 PGPASN_Certificate **cert )
{
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificate( asnctx, cert, buf, len, &err );
	CHKASNERR( err );

error:

	return err;
}


/*
 * Free the X.509 cert structure allocated by pgpX509BufferToCert
 */
PGPError
pgpX509FreeCert( PGPContextRef context, PGPASN_Certificate *cert )
{
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_FreeCertificate( asnctx, cert );
	return err;
}


/*
 * Given a buffer, return the abbreviated "tbs" part of an X.509 cert
 */
PGPError
pgpX509BufferToXTBSCert( PGPContextRef context, PGPByte *buf, PGPSize len,
						 PGPASN_XTBSCertificate **xtbscert )
{
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPASN_XCertificate *xcert = NULL;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackXCertificate ( asnctx, &xcert, buf, len, &err );
	CHKASNERR(err);
	pgpasn_UnpackXTBSCertificate ( asnctx, xtbscert, xcert->tbsCertificate.val,
								   xcert->tbsCertificate.len, &err );
	CHKASNERR(err);

error:

	if( IsntNull(xcert) )
		pgpasn_FreeXCertificate( asnctx, xcert );
	return err;
}


/*
 * Free the X.509 abbreviated tbs cert structure.
 */
PGPError
pgpX509FreeXTBSCert( PGPContextRef context, PGPASN_XTBSCertificate *xtbscert )
{
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_FreeXTBSCertificate( asnctx, xtbscert );
	return err;
}


/*
 * Given a buffer, return an X.509 CRL.
 */
PGPError
pgpX509BufferToCRL( PGPContextRef context, PGPByte *buf, PGPSize len,
					 PGPASN_CertificateRevocationList **crl )
{
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificateRevocationList( asnctx, crl, buf, len, &err );
	CHKASNERR( err );

error:

	return err;
}


/*
 * Free the X.509 CRL structure allocated by pgpX509BufferToCRL
 */
PGPError
pgpX509FreeCRL( PGPContextRef context, PGPASN_CertificateRevocationList *crl )
{
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_FreeCertificateRevocationList( asnctx, crl );
	return err;
}



/*
 * Given an X.509 cert, fill a buffer with a PGP formatted
 * userid packet in it corresponding to the name in the cert.
 * Use the ringpool's reserve buffer.
 * The packet does not include the PGP headers, just the body.
 */
PGPError
pgpX509CertToNameBuffer (RingPool *pool, PGPASN_Certificate *cert,
	PGPByte loversion, PGPSize *pktlen)
{
	PGPByte *namebuf;
	PGPSize namelen;
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPContextRef context;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	(void)loversion;
	context = ringPoolContext(pool);
	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	err = sDecodeName (asnctx, cert, FALSE, NULL, &namelen);
	if (IsPGPError(err))
		goto error;
	namebuf = (PGPByte *)ringReserve(pool, namelen);
	CHKNONNULL( namebuf, err );
	err = sDecodeName (asnctx, cert, FALSE, namebuf, &namelen);
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
pgpX509CertToKeyBuffer (RingPool *pool, PGPASN_Certificate *cert,
	PGPByte loversion, PGPSize *pktlen)
{
	PGPByte *keybuf;
	PGPSize keylen;
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPContextRef context;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	(void)loversion;
	context = ringPoolContext(pool);
	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	err = sDecodeKey (context, asnctx, cert, NULL, NULL, &keylen);
	if (IsPGPError(err))
		goto error;
	keybuf = (PGPByte *)ringReserve(pool, keylen);
	CHKNONNULL( keybuf, err );
	err = sDecodeKey (context, asnctx, cert, NULL, keybuf, &keylen);
	if (IsPGPError(err))
		goto error;

	*pktlen = keylen;

error:

	return err;
}



/*
 * Given an X.509 cert, fill a buffer with a PGP formatted
 * sig packet in it corresponding to the signature in the cert.
 * Use the ringpool's reserve buffer.
 * The packet does not include the PGP headers, just the body.
 */
PGPError
pgpX509CertToSigBuffer (RingPool *pool, PGPASN_Certificate *cert,
	PGPByte loversion, PGPSize *pktlen)
{
	PGPSize siglen;
	PGPByte *sigbuf;
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPContextRef context;
	PGPMemoryMgrRef mgr;
	PGPError err = kPGPError_NoErr;

	context = ringPoolContext(pool);
	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	err = sDecodeSig(context, asnctx, cert, loversion, NULL, &siglen);
	if (IsPGPError(err))
		goto error;
	sigbuf = (PGPByte *)ringReserve(pool, siglen);
	CHKNONNULL( sigbuf, err );
	err = sDecodeSig (context, asnctx, cert, loversion, sigbuf, &siglen);
	if (IsPGPError(err))
		goto error;

	*pktlen = siglen;

error:

	return err;
}


/*
 * Given an X.509 XTBSCert, return the distribution point extension if
 * any, else NULL.  Buffer returned must be freed with PGPFreeData.
 */
PGPByte *
pgpX509XTBSCertToDistPoint (PGPASN_XTBSCertificate *xtbscert,
							PGPSize *pdpointlen)
{
	PGPASN_CRLDistPointsSyntax *dpts = NULL;
	PGPASN_DistributionPointName *dpname;
	PGPASN_GeneralNames *gnames;
	PGPByte *dpoint;
	PGPSize dpointlen;
	PGPASN_Extension *ext;
	PGPInt32 i;
	PGPMemoryMgrRef	mgr;
	PGPASN_CONTEXT asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPError err = kPGPError_NoErr;

	dpoint = NULL;
	dpointlen = 0;

	mgr = PGPGetDefaultMemoryMgr();
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	if( IsntNull( xtbscert->extensions ) ) {
		for (i=0; i<xtbscert->extensions->n; ++i) {
			ext = xtbscert->extensions->elt[i];
			if (ext->extnID.len == sizeof(certdistpoint)
				&& memcmp(ext->extnID.val, certdistpoint,
						  ext->extnID.len)==0 ) {
				/* Parse distribution point */
				pgpasn_UnpackCRLDistPointsSyntax ( asnctx, &dpts,
							ext->extnValue.val, ext->extnValue.len, &err );
				if( err == 0 && dpts->n >= 1 &&
							IsntNull( dpts->elt[0]->distributionPoint ) ) {
					dpname = dpts->elt[0]->distributionPoint;
					if( dpname->CHOICE_field_type == ASN_DPNAME_GENERALNAMES) {
						gnames = dpname->data;
						dpointlen = pgpasn_SizeofGeneralNames( asnctx, gnames,
															   TRUE);
						dpoint = PGPNewData( mgr, dpointlen, 0 );
						CHKNONNULL( dpoint, err );
						pgpasn_PackGeneralNames( asnctx, dpoint, dpointlen,
												gnames, &err );
						break;
					}
				}
				if( IsntNull( dpts ) )
					pgpasn_FreeCRLDistPointsSyntax( asnctx, dpts );
				dpts = NULL;
			}
		}
	}

 error:

	if( IsntNull( dpts ) )
		pgpasn_FreeCRLDistPointsSyntax( asnctx, dpts );

	if( IsntNull( dpoint ) && IsntNull( pdpointlen ) )
		*pdpointlen = dpointlen;

	return dpoint;
}



/*
 * Turn an X.509 Time value (CHOICE OF UTCTime or GeneralizedTime) into
 * PGP format
 */
PGPError
pgpX509DecodeTime (PGPASN_Time *tm, PGPUInt32 *pgptime)
{
	PGPASN_UTCTime *utc;
	PGPUInt32 utime;
	PGPError err = kPGPError_NoErr;

	utc = (PGPASN_UTCTime *)tm->data;
	err = sDecodeUTCTime (utc->val, utc->len,
					(PGPBoolean)(tm->CHOICE_field_type!=PGPASN_ID_UTCTime),
					&utime);
	if( IsPGPError( err ) )
		return err;

	if (pgptime)
		*pgptime = utime;

	return kPGPError_NoErr;
}


/*
 * Return an IssuerAndSerialNumber structure, a SEQUENCE consisting of
 * issuer name followed by certificate serial number.  Call with buf NULL
 * to find out how much space will be needed, then call with buf non-NULL
 * and pointing at sufficient space.
 */
PGPError
pgpX509CertToIASN (PGPByte *certbuf, PGPSize len, PGPByte *buf,
				   PGPSize *pktlen)
{
	PGPASN_XCertificate				*cert = NULL;
	PGPASN_XTBSCertificate			*cinfo = NULL;
	PGPASN_XIssuerAndSerialNumber	*iasn = NULL;
	PGPMemoryMgrRef					 mgr;
	PGPASN_CONTEXT					 asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr				 asnmem;
	PGPSize							 buflen = 0;
	PGPError						 err = kPGPError_NoErr;

	mgr = PGPGetDefaultMemoryMgr();
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	if( IsntNull( pktlen ) )
		*pktlen = 0;
	
	pgpasn_UnpackXCertificate ( asnctx, &cert, certbuf, len, &err );
	CHKASNERR(err);
	pgpasn_UnpackXTBSCertificate ( asnctx, &cinfo, cert->tbsCertificate.val,
								   cert->tbsCertificate.len, &err );
	CHKASNERR(err);

	iasn = pgpasn_NewXIssuerAndSerialNumber ( asnctx );
	CHKNONNULL(iasn, err);
	pgpasn_PutOctVal ( asnctx, &iasn->serialNumber, cinfo->serialNumber.val,
					   cinfo->serialNumber.len );
	pgpasn_PutOctVal ( asnctx, &iasn->issuer, cinfo->issuer.val,
					   cinfo->issuer.len );

	buflen = pgpasn_SizeofXIssuerAndSerialNumber ( asnctx, iasn, TRUE );
	if( IsntNull( buf ) ) {
		pgpasn_PackXIssuerAndSerialNumber ( asnctx, buf, buflen, iasn, &err );
		CHKASNERR(err);
	}

error:

	if( IsntNull( iasn ) )
		pgpasn_FreeXIssuerAndSerialNumber( asnctx, iasn );
	if( IsntNull( cinfo ) )
		pgpasn_FreeXTBSCertificate( asnctx, cinfo );
	if( IsntNull( cert ) )
		pgpasn_FreeXCertificate( asnctx, cert );

	if (IsntPGPError( err )  &&  IsntNull (pktlen) )
		*pktlen = buflen;

	return err;
}


/*
 * Return a name from the cert, either subject or issuer, in the long
 * form.  Call with buf NULL to find out how much space will be
 * needed, then call with buf non-NULL and pointing at sufficient
 * space.  (Must leave *pktlen with the value it had upon the first call.)
 */
PGPError
pgpX509CertToLongName (PGPByte *certbuf, PGPSize len, PGPBoolean doIssuer,
	PGPByte *buf, PGPSize *pktlen)
{
	PGPASN_Certificate				*cert = NULL;
	PGPMemoryMgrRef					 mgr;
	PGPASN_CONTEXT					 asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr				 asnmem;
	PGPSize							 buflen = 0;
	PGPError						 err = kPGPError_NoErr;

	buflen = *pktlen;

	mgr = PGPGetDefaultMemoryMgr();
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificate ( asnctx, &cert, certbuf, len, &err );
	CHKASNERR(err);

	err = sDecodeNameLong(asnctx, cert, doIssuer, buf, &buflen);

error:

	if( IsntNull( cert ) )
		pgpasn_FreeCertificate( asnctx, cert );

	if (IsntPGPError( err )  &&  IsntNull (pktlen) )
		*pktlen = buflen;

	return err;
}


/*
 * Return a name from the cert in binary form, DER encoded.
 * Call with buf NULL to find out how much space will be
 * needed, then call with buf non-NULL and pointing at sufficient
 * space.  (Must leave *pktlen with the value it had upon the first call.)
 */
PGPError
pgpX509CertToDName (PGPByte *certbuf, PGPSize len, PGPBoolean doIssuer,
	PGPByte *buf, PGPSize *pktlen)
{
	PGPASN_Certificate				*cert = NULL;
	PGPMemoryMgrRef					 mgr;
	PGPASN_Name						*name;
	PGPASN_CONTEXT					 asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr				 asnmem;
	PGPSize							 buflen = 0;
	PGPError						 err = kPGPError_NoErr;

	buflen = *pktlen;

	mgr = PGPGetDefaultMemoryMgr();
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificate ( asnctx, &cert, certbuf, len, &err );
	CHKASNERR(err);

	name = doIssuer ? &cert->tbsCertificate.issuer
					: &cert->tbsCertificate.subject;
	
	if( IsNull( buf ) ) {
		buflen = pgpasn_SizeofName (asnctx, name, TRUE);
		goto error;
	}

	pgpasn_PackName( asnctx, buf, buflen, name, &err );

error:

	if( IsntNull( cert ) )
		pgpasn_FreeCertificate( asnctx, cert );

	if (IsntPGPError( err )  &&  IsntNull (pktlen) )
		*pktlen = buflen;

	return err;
}


/*
 * Return IP address of DNS name from the cert, if they exist.  Call
 * with buf NULL to find out how much space will be needed, then call
 * with buf non-NULL and pointing at sufficient space.  (Must leave
 * *pktlen with the value it had upon the first call.)
 */
PGPError
pgpX509CertToIPDNS (PGPByte *certbuf, PGPSize len, PGPBoolean doIP,
	PGPByte *buf, PGPSize *pktlen)
{
	PGPASN_Certificate				*cert = NULL;
	PGPMemoryMgrRef					 mgr;
	PGPASN_CONTEXT					 asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr				 asnmem;
	PGPSize							 buflen = 0;
	PGPError						 err = kPGPError_NoErr;

	buflen = *pktlen;

	mgr = PGPGetDefaultMemoryMgr();
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	pgpasn_UnpackCertificate ( asnctx, &cert, certbuf, len, &err );
	CHKASNERR(err);

	err = sDecodeIPDNSName(asnctx, cert, doIP?buf:NULL, doIP?&buflen:NULL,
						   !doIP?buf:NULL, !doIP?&buflen:NULL);

error:

	if( IsntNull( cert ) )
		pgpasn_FreeCertificate( asnctx, cert );

	if (IsntPGPError( err )  &&  IsntNull (pktlen) )
		*pktlen = buflen;

	return err;
}



/*
 * Create a SubjectPublicKeyInfo structure holding the key material
 * from the specified PGP key.
 */
PGPError
pgpKeyToX509SPKI(PGPContextRef context,
	RingSet const *set, RingObject *key, PGPByte *keyBuf, PGPSize *keyLength)
{
	PGPASN_SubjectPublicKeyInfo *spki = NULL;
	PGPASN_DssParms			*dssp = NULL;
	PGPASN_DSAPublicKey		*dssk = NULL;
	PGPASN_ELGParms			*elgp = NULL;
	PGPASN_ELGPublicKey		*elgk = NULL;
	PGPASN_RSAKey			*rsak = NULL;

	PGPPublicKeyAlgorithm keyalgorithm;
	PGPByte pkalg;
	PGPByte const *bnbuf[4];	/* buf offset for start of bignum */
	PGPSize bnlen[4];			/* buf length for bignum */
	PGPInt32 i;
	PGPByte const *keyData;
	PGPSize keyDataLength;
	PGPSize keyDataOffset;
	PGPByte *tbuf = NULL;
	PGPSize tbuflen;
	PGPMemoryMgrRef mgr;
	PGPASN_CONTEXT  asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr asnmem;
	PGPError err = kPGPError_NoErr;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	spki = pgpasn_NewSubjectPublicKeyInfo (asnctx);

	keyData = ringFetchObject( set, key, &keyDataLength );
	CHKNONNULL(keyData, err);

	/* V4 keys are two bytes shorter than V3 */
	keyDataOffset = 6;
	if (keyData[0] <= PGPVERSION_3)
		keyDataOffset += 2;
	keyData += keyDataOffset;
	
	ringKeyID8(set, key, &pkalg, NULL);
	keyalgorithm = (PGPPublicKeyAlgorithm)pkalg;

	if (keyalgorithm == kPGPPublicKeyAlgorithm_DSA) {
		pgpasn_PutOctVal (asnctx, &spki->algorithm.algorithm, dsaoid,
							sizeof(dsaoid));

		/* Read four bignums */
		for( i=0; i<4; ++i ) {
			bnlen[i] = (((keyData[0] << 8) | keyData[1]) + 7) / 8;
			bnbuf[i] = keyData+2;
			keyData += 2+bnlen[i];
		}

		dssp = pgpasn_NewDssParms (asnctx);
		dssk = pgpasn_NewDSAPublicKey (asnctx);

		pgpasn_PutUIntBytes (asnctx, &dssp->p, bnbuf[0], bnlen[0]);
		pgpasn_PutUIntBytes (asnctx, &dssp->q, bnbuf[1], bnlen[1]);
		pgpasn_PutUIntBytes (asnctx, &dssp->g, bnbuf[2], bnlen[2]);

		pgpasn_PutUIntBytes (asnctx, dssk, bnbuf[3], bnlen[3]);

		tbuflen = pgpasn_SizeofDssParms (asnctx, dssp, TRUE);
		tbuf = PGPNewData( mgr, tbuflen, 0 );
		pgpasn_PackDssParms( asnctx, tbuf, tbuflen, dssp, &err );
		spki->algorithm.parameters = pgpasn_NewANY( asnctx );
		pgpasn_PutOctVal( asnctx, spki->algorithm.parameters, tbuf, tbuflen);
		PGPFreeData( tbuf );
		tbuf = NULL;

		tbuflen = pgpasn_SizeofDSAPublicKey (asnctx, dssk, TRUE);
		tbuf = PGPNewData( mgr, tbuflen, 0 );
		pgpasn_PackDSAPublicKey( asnctx, tbuf, tbuflen, dssk, &err );
		pgpasn_PutBitString( asnctx, &spki->subjectPublicKey, tbuf, tbuflen,
							 0 );
		PGPFreeData( tbuf );
		tbuf = NULL;

		pgpasn_FreeDssParms (asnctx, dssp);
		pgpasn_FreeDSAPublicKey (asnctx, dssk);
		dssp = NULL;
		dssk = NULL;
	} else if (keyalgorithm == kPGPPublicKeyAlgorithm_ElGamal) {
		pgpasn_PutOctVal (asnctx, &spki->algorithm.algorithm, elgoid,
							sizeof(elgoid));

		/* Read three bignums */
		for( i=0; i<3; ++i ) {
			bnlen[i] = (((keyData[0] << 8) | keyData[1]) + 7) / 8;
			bnbuf[i] = keyData+2;
			keyData += 2+bnlen[i];
		}

		elgp = pgpasn_NewELGParms (asnctx);
		elgk = pgpasn_NewELGPublicKey (asnctx);

		pgpasn_PutUIntBytes (asnctx, &elgp->p, bnbuf[0], bnlen[0]);
		pgpasn_PutUIntBytes (asnctx, &elgp->g, bnbuf[1], bnlen[1]);

		pgpasn_PutUIntBytes (asnctx, elgk, bnbuf[2], bnlen[2]);

		tbuflen = pgpasn_SizeofELGParms (asnctx, elgp, TRUE);
		tbuf = PGPNewData( mgr, tbuflen, 0 );
		pgpasn_PackELGParms( asnctx, tbuf, tbuflen, elgp, &err );
		spki->algorithm.parameters = pgpasn_NewANY( asnctx );
		pgpasn_PutOctVal( asnctx, spki->algorithm.parameters, tbuf, tbuflen);
		PGPFreeData( tbuf );
		tbuf = NULL;

		tbuflen = pgpasn_SizeofELGPublicKey (asnctx, elgk, TRUE);
		tbuf = PGPNewData( mgr, tbuflen, 0 );
		pgpasn_PackELGPublicKey( asnctx, tbuf, tbuflen, elgk, &err );
		pgpasn_PutBitString( asnctx, &spki->subjectPublicKey, tbuf, tbuflen,
							 0 );
		PGPFreeData( tbuf );
		tbuf = NULL;

		pgpasn_FreeELGParms (asnctx, elgp);
		pgpasn_FreeELGPublicKey (asnctx, elgk);
		elgp = NULL;
		elgk = NULL;
	} else if (keyalgorithm == kPGPPublicKeyAlgorithm_RSA) {
		static PGPByte nullbuf[2] = {0x05, 0x00};
		pgpasn_PutOctVal (asnctx, &spki->algorithm.algorithm, rsaoid,
							sizeof(rsaoid));
		/* Insert a null for the parameter */
		spki->algorithm.parameters = pgpasn_NewANY( asnctx );
		pgpasn_PutOctVal (asnctx, spki->algorithm.parameters, nullbuf,
						  sizeof(nullbuf));

		/* Read two bignums */
		for( i=0; i<2; ++i ) {
			bnlen[i] = (((keyData[0] << 8) | keyData[1]) + 7) / 8;
			bnbuf[i] = keyData+2;
			keyData += 2+bnlen[i];
		}

		rsak = pgpasn_NewRSAKey (asnctx);
		pgpasn_PutUIntBytes (asnctx, &rsak->modulus, bnbuf[0], bnlen[0]);
		pgpasn_PutUIntBytes (asnctx, &rsak->exponent, bnbuf[1], bnlen[1]);

		tbuflen = pgpasn_SizeofRSAKey (asnctx, rsak, TRUE);
		tbuf = PGPNewData( mgr, tbuflen, 0 );
		pgpasn_PackRSAKey( asnctx, tbuf, tbuflen, rsak, &err );
		pgpasn_PutBitString( asnctx, &spki->subjectPublicKey, tbuf, tbuflen,
							 0 );
		pgpasn_FreeRSAKey (asnctx, rsak);
		rsak = NULL;
		PGPFreeData( tbuf );
		tbuf = NULL;
	} else {
		err = kPGPError_UnknownPublicKeyAlgorithm;
		goto error;
	}

	if( IsNull( keyBuf ) ) {
		*keyLength = pgpasn_SizeofSubjectPublicKeyInfo (asnctx, spki, TRUE);
	} else {
		pgpasn_PackSubjectPublicKeyInfo( asnctx, keyBuf, *keyLength, spki,
										 &err );
		CHKASNERR(err);
	}
	pgpasn_FreeSubjectPublicKeyInfo( asnctx, spki );
	spki = NULL;

error:

	if( IsntNull( tbuf ) )
		PGPFreeData( tbuf );
	if( IsntNull( dssp ) )
		pgpasn_FreeDssParms( asnctx, dssp );
	if( IsntNull( dssk ) )
		pgpasn_FreeDSAPublicKey( asnctx, dssk );
	if( IsntNull( elgp ) )
		pgpasn_FreeELGParms( asnctx, elgp );
	if( IsntNull( elgk ) )
		pgpasn_FreeELGPublicKey( asnctx, elgk );
	if( IsntNull( rsak ) )
		pgpasn_FreeRSAKey( asnctx, rsak );
	if( IsntNull( spki ) )
		pgpasn_FreeSubjectPublicKeyInfo( asnctx, spki );

	return err;
}


/*
 * See whether the given ASN.1 object is in the list of objects, each of
 * type SEQUENCE
 */
PGPBoolean
pgpX509BufInSequenceList( PGPByte const *buf, PGPSize bufsize,
	PGPByte const *seqlist, PGPSize seqlistsize )
{
	PGPUInt32 tag;
	PGPSize len;
	PGPByte const *tseq;
	PGPSize seqitemsize;

	while (seqlistsize >= bufsize) 
	{
		tseq = seqlist;
		tag = dotaglen((PGPByte **)&tseq, &len);
		tseq += len;
		seqitemsize = tseq - seqlist;
		if( seqitemsize == bufsize &&
						pgpMemoryEqual( seqlist, buf, bufsize ) )
			return TRUE;
		seqlist += seqitemsize;
		seqlistsize -= seqitemsize;
	}
		
	return FALSE;
}

/*
 * Return the size of the given X.509 buffer of type SEQUENCE
 */
PGPSize
pgpX509BufSizeofSequence( PGPByte const *buf, PGPSize bufsize )
{
	PGPUInt32 tag;
	PGPSize len;
	PGPByte const *ibuf;

	/* Assume indefinite length buffers are whole thing */
	if( buf[1] == 0x80 )
		return bufsize;
	ibuf = buf;
	tag = dotaglen((PGPByte **)&buf, &len);
	if( tag != TAG_SEQUENCE )
		return 0;		/* Bad buffer */
	return len + buf - ibuf;
}



#if 0

#ifndef TISX509

/* Temporary functions for linking */


#if TESTASN1

/* PKCS-7 OID is: (1, 2, 840, 113549, 1, 7) */
#define PKCS7OID	0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07
static PGPByte const dataoid[] = {PKCS7OID, 0x01};
static PGPByte const signeddataoid[] = {PKCS7OID, 0x02};
static PGPByte const envelopeddataoid[] = {PKCS7OID, 0x03};
static PGPByte const signedandenvelopeddataoid[] = {PKCS7OID, 0x04};
static PGPByte const digesteddataoid[] = {PKCS7OID, 0x05};
static PGPByte const encrytpeddataoid[] = {PKCS7OID, 0x06};
/* MD5 (1 2 840 113549 2 5) */
static PGPByte const md5oid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02,
								 0x05};
#endif /* TESTASN1 */

    PGPError
X509InputCertificate (
            PGPContextRef       context,
            PGPByte            *input,      /* Input buffer */
            PGPSize             inputSize,
            PGPKeySetRef        keyset,     /* Where to find keys */
            PGPInputFormat      format,     /* Import format (CA) */
            PGPOptionListRef    passphrase, /* Passphrase for decrypting */
            PGPKeyRef          *decryptKey, /* Output decryption key */
            PGPKeyRef          *signKey,    /* Output signing key */
            PGPBoolean         *isSigned,   /* Output flag for signed msg */
            PGPBoolean         *sigChecked, /* Output that we have sig key */
            PGPBoolean         *sigVerified,/* Output flag for sig OK */
            PGPAttributeValue **extraData,  /* Output extra data */
            PGPUInt32          *extraDataCount,
            PGPByte           **certSeq,    /* Output buffer */
            PGPSize            *certSeqSize
    )
{
	(void)context;
	(void)input;
	(void)inputSize;
	(void)keyset;
	(void)format;
    (void)passphrase;
	(void)decryptKey;
	(void)signKey;
	(void)isSigned;
	(void)sigChecked;
	(void)sigVerified;
	(void)extraData;
	(void)extraDataCount;

	*certSeq = NULL;
	*certSeqSize = 0;
	return kPGPError_NoErr;
}

#if TESTASN1



	static PGPError
sX509CreateDN (PGPMemoryMgrRef mgr, PGPASN_CONTEXT *asnctx,
			   PGPASN_Name *name, char *commonName, char *emailAddress)
{
	PGPASN_RDNSequence				*rdns;
	PGPASN_AttributeTypeAndValue	*atv0, *atv1;
	PGPASN_IA5String				*ps;
	PGPByte							*psbuf;
	PGPSize							psbufsize;
	int								err = 0;

	/* Some kind of kludge needed here... */
	name->CHOICE_field_type = PGPASN_ID_RDNSequence;
	rdns = pgpasn_NewRDNSequence (asnctx);
	name->data = rdns;

	rdns->n = 2;
	rdns->elt[0] = pgpasn_NewRelativeDistinguishedName (asnctx);
	rdns->elt[1] = pgpasn_NewRelativeDistinguishedName (asnctx);

	rdns->elt[0]->n = 1;
	rdns->elt[1]->n = 1;
	atv0 = pgpasn_NewAttributeTypeAndValue (asnctx);
	atv1 = pgpasn_NewAttributeTypeAndValue (asnctx);
	rdns->elt[0]->elt[0] = atv0;
	rdns->elt[1]->elt[0] = atv1;

	pgpasn_PutOctVal (asnctx, &atv0->type, cnoid, sizeof(cnoid));
	ps = pgpasn_NewIA5String (asnctx);
	pgpasn_PutOctVal (asnctx, ps, commonName, strlen(commonName));
	psbufsize = pgpasn_SizeofIA5String (asnctx, ps, TRUE);
	psbuf = PGPNewData( mgr, psbufsize, 0 );
	pgpasn_PackIA5String( asnctx, psbuf, psbufsize, ps, &err );
	if ( err )
		return kPGPError_LazyProgrammer;
	pgpasn_PutOctVal (asnctx, &atv0->value, psbuf, psbufsize);
	PGPFreeData( psbuf );
	psbuf = NULL;
	pgpasn_FreeIA5String( asnctx, ps );

	pgpasn_PutOctVal (asnctx, &atv1->type, emailoid, sizeof(emailoid));
	ps = pgpasn_NewIA5String (asnctx);
	pgpasn_PutOctVal (asnctx, ps, emailAddress, strlen(emailAddress));
	psbufsize = pgpasn_SizeofIA5String (asnctx, ps, TRUE);
	psbuf = PGPNewData( mgr, psbufsize, 0 );
	pgpasn_PackIA5String( asnctx, psbuf, psbufsize, ps, &err );
	if ( err )
		return kPGPError_LazyProgrammer;
	pgpasn_PutOctVal (asnctx, &atv1->value, psbuf, psbufsize);
	PGPFreeData( psbuf );
	pgpasn_FreeIA5String( asnctx, ps );

	return kPGPError_NoErr;
}

	static PGPError
sX509CreateSPKI (PGPMemoryMgrRef mgr, PGPASN_CONTEXT *asnctx,
				 PGPASN_SubjectPublicKeyInfo *spki, PGPByte *keydata)
{
	PGPASN_RSAKey				*rsakey;
	PGPUInt32					modbits, modbytes;
	PGPUInt32					expbits, expbytes;
	PGPByte						*buf;
	PGPSize						bufsize;
	int							err = 0;

	pgpasn_PutOctVal (asnctx, &spki->algorithm.algorithm, rsaoid,
						sizeof(rsaoid));
	spki->algorithm.parameters = NULL;
	
	modbits = (keydata[0] << 8) | keydata[1];
	modbytes = (modbits + 7) / 8;
	expbits = (keydata[2+modbytes] << 8) | keydata[2+modbytes+1];
	expbytes = (expbits + 7) / 8;
	rsakey = pgpasn_NewRSAKey (asnctx);
	pgpasn_PutOctVal (asnctx, &rsakey->modulus, keydata + 2, modbytes);
	pgpasn_PutOctVal (asnctx, &rsakey->exponent, keydata + 2 + modbytes + 2,
						expbytes);

	bufsize = pgpasn_SizeofRSAKey (asnctx, rsakey, TRUE);
	buf = PGPNewData( mgr, bufsize, 0 );
	pgpasn_PackRSAKey( asnctx, buf, bufsize, rsakey, &err );
	pgpasn_PutBitString( asnctx, &spki->subjectPublicKey, buf, bufsize, 0 );
	pgpasn_FreeRSAKey (asnctx, rsakey);
	rsakey = NULL;
	PGPFreeData( buf );
	keydata = NULL;

	return kPGPError_NoErr;
}


#endif /* TESTASN1 */


/* Create the payload */
PGPError
X509CreateCertificateRequest (
		PGPContextRef       context,
		PGPKeyRef	        key,     	/* Key to export */
		PGPExportFormat     format,     /* Export format (CA) */
		PGPAttributeValue  *formatData, /* Formatting data */
		PGPUInt32           formatDataCount,
		PGPOptionListRef    passphrase, /* Passphrase if signing */
		PGPByte           **certReq,    /* Output buffer */
		PGPSize            *certReqSize /* Output buffer size */
)
{
#if !TESTASN1

	(void)context;
	(void)key;
	(void)format;
	(void)formatData;
	(void)formatDataCount;
    (void)passphrase;
	*certReqSize = 0;
	*certReq = NULL;

	return kPGPError_NoErr;

#else /* TESTASN1 */

	/* Create a PKCS-10, just for kicks */
	PGPMemoryMgrRef				mgr;
	PGPKeyListRef				klist;
	PGPKeyIterRef				kiter;
	PGPKeySetRef				keyset;
	PGPUserIDRef				userid;
	PGPUInt32					keyalg;
	char						*uid = NULL;
	char						*email;
	PGPSize						uidsize;
	PGPByte						*keydata = NULL;
	PGPSize						keydatasize;
	PGPHashContextRef			hashctx;
	PGPPrivateKeyContextRef		privctx;
	PGPByte						*sigbuf;
	PGPSize						sigbufsize;

	PGPASN_CONTEXT					asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr				asnmem;
	PGPASN_CertificationRequest		*cr;
	PGPASN_CertificationRequestInfo	*cri;
	PGPASN_Name						*name;
	PGPASN_SubjectPublicKeyInfo		*spki;
	PGPASN_Signature				*sig;
	PGPByte						*cridata;
	PGPSize						crisize;
	int							err = 0;

	(void)format;
	(void)formatData;
	(void)formatDataCount;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	PGPNewSingletonKeySet( key, &keyset );
	PGPOrderKeySet( keyset, kPGPAnyOrdering, &klist );
	PGPNewKeyIter( klist, &kiter );
	PGPKeyIterNext( kiter, &key );
	PGPKeyIterNextUserID( kiter, &userid );
	PGPFreeKeyIter( kiter );
	PGPFreeKeyList( klist );
	PGPFreeKeySet( keyset );

	PGPGetKeyNumber( key, kPGPKeyPropAlgID, &keyalg );
	PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName, 0, NULL, &uidsize );
	uid = PGPNewData( mgr, uidsize, 0);
	PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName, uidsize, uid,
							  &uidsize );
	PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, 0, NULL, &keydatasize );
	keydata = PGPNewData( mgr, keydatasize, 0);
	PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, keydatasize, keydata,
							 &keydatasize );

	/* Only supporting RSA for now */
	pgpAssert (keyalg == kPGPPublicKeyAlgorithm_RSA);

	/* some error checking would be used here if this weren't a hack! */
	email = strchr (uid, '<') + 1;
	*(strchr(email, '>')) = '\0';
	*(email-2) = '\0';

	/* Create the cert request */

	cr = pgpasn_NewCertificationRequest (asnctx);

	pgpasn_PutOctVal (asnctx, &cr->signatureAlgorithm.algorithm, rsamd5oid,
					  sizeof(rsamd5oid));
	cr->signatureAlgorithm.parameters = NULL;

	cri = &cr->certificationRequestInfo;
	name = &cri->subject;

	err = sX509CreateDN (mgr, asnctx, name, uid, email);
	if( err )
		return kPGPError_LazyProgrammer;
	PGPFreeData( uid );
	uid = NULL;

	pgpasn_PutIntVal (asnctx, &cri->version, 0);
	spki = &cri->subjectPublicKeyInfo;

	err = sX509CreateSPKI (mgr, asnctx, spki, keydata);
	if ( err )
		return kPGPError_LazyProgrammer;

	PGPFreeData( keydata );
	keydata = NULL;

	cri->attributes.n = 0;

	crisize = pgpasn_SizeofCertificationRequestInfo( asnctx, cri, TRUE );
	cridata = PGPNewData( mgr, crisize, 0);
	pgpasn_PackCertificationRequestInfo( asnctx, cridata, crisize, cri, &err );
	if ( err )
		return kPGPError_LazyProgrammer;

	err = PGPNewPrivateKeyContext( key, kPGPPublicKeyMessageFormat_X509,
								   &privctx,
								   ( passphrase ? passphrase
									 : PGPONullOption(context) ), 
								   PGPOLastOption(context) );
	if( IsPGPError( err ) )
		return err;

	PGPNewHashContext (mgr, kPGPHashAlgorithm_MD5, &hashctx);
	PGPContinueHash (hashctx, cridata, crisize);

	PGPFreeData( cridata );
	cridata = NULL;

	PGPGetPrivateKeyOperationSizes( privctx, NULL, NULL, &sigbufsize);
	sigbuf = PGPNewData( mgr, sigbufsize, 0 );
	PGPPrivateKeySign( privctx, hashctx, sigbuf, &sigbufsize);
	PGPFreePrivateKeyContext( privctx );
	privctx = NULL;

	pgpasn_UnpackSignature ( asnctx, &sig, sigbuf, sigbufsize, &err );
	if ( err )
		return kPGPError_LazyProgrammer;
	cr->signature = *sig;

	PGPFreeData( sigbuf );
	sigbuf = NULL;
	sig = NULL;

	*certReqSize = pgpasn_SizeofCertificationRequest( asnctx, cr, TRUE );
	*certReq = PGPNewData( mgr, *certReqSize, 0);
	pgpasn_PackCertificationRequest (asnctx, *certReq, *certReqSize, cr, &err);
	if ( err )
		return kPGPError_LazyProgrammer;

	return kPGPError_NoErr;
#endif /* TESTASN1 */
}

#if TESTASN1
/* Create a self signed X.509 cert */
	static PGPError
sX509CreateCert (PGPContextRef context, PGPASN_CONTEXT *asnctx,
				 PGPASN_Certificate *cert,
				 PGPKeyRef key, PGPOptionListRef passphrase)
{
	PGPMemoryMgrRef				mgr;
	PGPKeySetRef				keyset;
	PGPKeyListRef				klist;
	PGPKeyIterRef				kiter;
	PGPUserIDRef				userid;
	PGPUInt32					keyalg;
	char						*uid = NULL;
	char						*email;
	PGPSize						uidsize;
	PGPByte						*keydata = NULL;
	PGPSize						keydatasize;
	PGPHashContextRef			hashctx;
	PGPPrivateKeyContextRef		privctx;
	PGPByte						*sigbuf;
	PGPSize						sigbufsize;
	time_t						tm0;
	struct tm					*tm;
	char						timebuf[32];
	PGPByte						*tbsdata;
	PGPSize						tbssize;

	PGPASN_UTCTime				*t;
	PGPASN_TBSCertificate		*tbs;
	PGPASN_Signature			*sig;
	int							err = 0;
	
	mgr = PGPGetContextMemoryMgr( context );

	PGPNewSingletonKeySet( key, &keyset );
	PGPOrderKeySet( keyset, kPGPAnyOrdering, &klist );
	PGPNewKeyIter( klist, &kiter );
	PGPKeyIterNext( kiter, &key );
	PGPKeyIterNextUserID( kiter, &userid );
	PGPFreeKeyIter( kiter );
	PGPFreeKeyList( klist );
	PGPFreeKeySet( keyset );

	PGPGetKeyNumber( key, kPGPKeyPropAlgID, &keyalg );
	PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName, 0, NULL, &uidsize );
	uid = PGPNewData( mgr, uidsize, 0);
	PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName, uidsize, uid,
							  &uidsize );
	PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, 0, NULL, &keydatasize );
	keydata = PGPNewData( mgr, keydatasize, 0);
	PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, keydatasize, keydata,
							 &keydatasize );

	/* Only supporting RSA for now */
	pgpAssert (keyalg == kPGPPublicKeyAlgorithm_RSA);

	/* some error checking would be used here if this weren't a hack! */
	email = strchr (uid, '<') + 1;
	*(strchr(email, '>')) = '\0';
	*(email-2) = '\0';

	tbs = &cert->tbsCertificate;
	pgpasn_PutIntVal (asnctx, &tbs->serialNumber, 1);
	pgpasn_PutOctVal (asnctx, &tbs->signature.algorithm, rsamd5oid,
					  sizeof(rsamd5oid));

	err = sX509CreateDN (mgr, asnctx, &tbs->issuer, uid, email);
	if( err )
		return kPGPError_LazyProgrammer;
	err = sX509CreateDN (mgr, asnctx, &tbs->subject, uid, email);
	if( err )
		return kPGPError_LazyProgrammer;
	PGPFreeData( uid );
	uid = NULL;

	/* Make valid from previous midnight, for 48 hours */
	t = pgpasn_NewUTCTime (asnctx);
	tbs->validity.notBefore.CHOICE_field_type = PGPASN_ID_UTCTime;
	tbs->validity.notBefore.data = t;
	tm0 = time(0);
	tm = gmtime(&tm0);
	sprintf (timebuf, "%02d%02d%02d000000Z", tm->tm_year, tm->tm_mon+1,
			 tm->tm_mday);
	pgpasn_PutOctVal (asnctx, t, timebuf, strlen(timebuf));
	
	t = pgpasn_NewUTCTime (asnctx);
	tbs->validity.notAfter.CHOICE_field_type = PGPASN_ID_UTCTime;
	tbs->validity.notAfter.data = t;
	tm0 = time(0) + 48*60*60;
	tm = gmtime(&tm0);
	sprintf (timebuf, "%02d%02d%02d000000Z", tm->tm_year, tm->tm_mon+1,
			 tm->tm_mday);
	pgpasn_PutOctVal (asnctx, t, timebuf, strlen(timebuf));

	err = sX509CreateSPKI (mgr, asnctx, &tbs->subjectPublicKeyInfo, keydata);
	if ( err )
		return kPGPError_LazyProgrammer;

	PGPFreeData( keydata );
	keydata = NULL;

	tbssize = pgpasn_SizeofTBSCertificate( asnctx, tbs, TRUE );
	tbsdata = PGPNewData( mgr, tbssize, 0);
	pgpasn_PackTBSCertificate( asnctx, tbsdata, tbssize, tbs, &err );
	if ( err )
		return kPGPError_LazyProgrammer;

	err = PGPNewPrivateKeyContext( key, kPGPPublicKeyMessageFormat_X509,
								   &privctx,
								   ( passphrase ? passphrase
									 : PGPONullOption(context) ), 
								   PGPOLastOption(context) );
	if( IsPGPError( err ) )
		return err;

	PGPNewHashContext (mgr, kPGPHashAlgorithm_MD5, &hashctx);
	PGPContinueHash (hashctx, tbsdata, tbssize);

	PGPFreeData( tbsdata );
	tbsdata = NULL;

	PGPGetPrivateKeyOperationSizes( privctx, NULL, NULL, &sigbufsize);
	sigbuf = PGPNewData( mgr, sigbufsize, 0 );
	PGPPrivateKeySign( privctx, hashctx, sigbuf, &sigbufsize);
	PGPFreePrivateKeyContext( privctx );
	privctx = NULL;

	pgpasn_UnpackSignature ( asnctx, &sig, sigbuf, sigbufsize, &err );
	if ( err )
		return kPGPError_LazyProgrammer;
	/* Shallow copy */
	cert->signature = *sig;
	PGPFreeData( sig );

	PGPFreeData( sigbuf );
	sigbuf = NULL;
	sig = NULL;

	pgpasn_PutOctVal (asnctx, &cert->signatureAlgorithm.algorithm, rsamd5oid,
					  sizeof(rsamd5oid));

	return kPGPError_NoErr;
}


#endif /* TESTASN1 */

/* Package it in PKCS-7 form */
    PGPError
    X509PackageCertificateRequest (
            PGPContextRef       context,
            PGPByte            *certReqIn,     /* Input buffer */
            PGPSize             certReqInSize, /* Input buffer size */
            PGPKeyRef           encryptTo,     /* Key to encrypt to */
            PGPCipherAlgorithm  encryptAlg,    /* Encryption algorithm */
            PGPKeyRef           signWith,      /* Key to sign with */
            PGPOptionListRef    passphrase,    /* Passphrase if signing */
            PGPOutputFormat     format,        /* Export format (CA) */
            PGPByte           **certReqOut,    /* Output buffer */
            PGPSize            *certReqOutSize /* Output buffer size */
    )
{
#if !TESTASN1

	(void)context;
	(void)certReqIn;
	(void)certReqInSize;
	(void)encryptTo;
	(void)encryptAlg;
	(void)signWith;
    (void)passphrase;
	(void)format;

	*certReqOut = NULL;
	*certReqOutSize = 0;
	return kPGPError_NoErr;
#else /* TESTASN1 */
	/* Try packing into a PKCS-7 */
	PGPMemoryMgrRef				mgr;

	PGPASN_Certificate					*cert;
	PGPASN_ContentInfo					*ci0;
	PGPASN_Data							*data;
	PGPASN_Name							*namecopy;
	PGPASN_SignedData					*sdata;
	PGPASN_SignerInfo					*sinfo;
	PGPASN_IssuerAndSerialNumber		*iasn;
	PGPASN_Signature					*sig;

	PGPASN_CONTEXT				asnctx_static, *asnctx = &asnctx_static;
	PGPASN_MemoryMgr			asnmem;
	PGPByte						*databuf;
	PGPSize						databufsize;
	PGPByte						*namebuf;
	PGPSize						namebuflen;
	PGPByte						*sigbuf;
	PGPSize						sigbufsize;
	PGPHashContextRef			hashctx;
	PGPPrivateKeyContextRef		privctx;
	int							err = 0;

	(void)encryptTo;
	(void)encryptAlg;
	(void)format;

	mgr = PGPGetContextMemoryMgr( context );
	sSetupASNCONTEXT (mgr, asnctx, &asnmem);

	ci0 = pgpasn_NewContentInfo (asnctx);
	pgpasn_PutOctVal (asnctx, &ci0->contentType, dataoid, sizeof(dataoid));
	data = pgpasn_NewData (asnctx);
	pgpasn_PutOctVal (asnctx, data, certReqIn, certReqInSize);
	databufsize = pgpasn_SizeofData (asnctx, data, TRUE);
	databuf = PGPNewData( mgr, databufsize, 0);
	pgpasn_PackData( asnctx, databuf, databufsize, data, &err );
	if ( err )
		return kPGPError_LazyProgrammer;
	pgpasn_FreeData( asnctx, data );

	ci0->content = pgpasn_NewANY (asnctx);
	pgpasn_PutOctVal (asnctx, ci0->content, databuf, databufsize);
	PGPFreeData( databuf );
	databuf = NULL;

	if( IsntNull( signWith ) ) {
		sdata = pgpasn_NewSignedData (asnctx);
		pgpasn_PutIntVal (asnctx, &sdata->version, 1);

		sdata->digestAlgorithms.n = 1;
		sdata->digestAlgorithms.elt[0] =
				pgpasn_NewDigestAlgorithmIdentifier (asnctx);
		pgpasn_PutOctVal (asnctx, &sdata->digestAlgorithms.elt[0]->algorithm,
							md5oid, sizeof(md5oid) );
		
		sdata->contentInfo.contentType = ci0->contentType;
		sdata->contentInfo.content = ci0->content;
		/* Not clear how to clean this up */
		PGPFreeData (ci0);
		ci0 = NULL;

		cert = pgpasn_NewCertificate (asnctx);
		err = sX509CreateCert (context, asnctx, cert, signWith,
					(passphrase ? pgpCopyOptionList(passphrase) : NULL));
		if ( err )
			return kPGPError_LazyProgrammer;

		sdata->certificates = pgpasn_NewCertificates (asnctx);
		sdata->certificates->n = 1;
		sdata->certificates->elt[0] = cert;

		sdata->signerInfos.n = 1;
		sdata->signerInfos.elt[0] = sinfo = pgpasn_NewSignerInfo (asnctx);

		pgpasn_PutIntVal (asnctx, &sinfo->version, 1);
		
		iasn = &sinfo->issuerAndSerialNumber;

		pgpasn_PutOctVal (asnctx, &iasn->serialNumber,
							cert->tbsCertificate.serialNumber.val,
							cert->tbsCertificate.serialNumber.len);
		namebuflen = pgpasn_SizeofName (asnctx, &cert->tbsCertificate.subject,
										TRUE);
		namebuf = PGPNewData( mgr, namebuflen, 0 );
		pgpasn_PackName (asnctx, namebuf, namebuflen,
						 &cert->tbsCertificate.subject, &err);
		if ( err )
			return kPGPError_LazyProgrammer;
		pgpasn_UnpackName (asnctx, &namecopy, namebuf, namebuflen, &err);
		PGPFreeData( namebuf );
		namebuf = NULL;
		iasn->issuer = *namecopy;
		PGPFreeData (namecopy);

		pgpasn_PutOctVal (asnctx, &sinfo->digestAlgorithm.algorithm,
							md5oid, sizeof(md5oid) );
		
		pgpasn_PutOctVal (asnctx,&sinfo->digestEncryptionAlgorithm.algorithm,
							rsaoid, sizeof(rsaoid) );

		err = PGPNewPrivateKeyContext( signWith,
									   kPGPPublicKeyMessageFormat_X509,
									   &privctx,
									   ( passphrase ? passphrase
										 : PGPONullOption(context) ), 
									   PGPOLastOption(context) );
		if( IsPGPError( err ) )
			return err;

		PGPNewHashContext (mgr, kPGPHashAlgorithm_MD5, &hashctx);
		PGPContinueHash (hashctx, certReqIn, certReqInSize);

		PGPGetPrivateKeyOperationSizes( privctx, NULL, NULL, &sigbufsize);
		sigbuf = PGPNewData( mgr, sigbufsize, 0 );
		PGPPrivateKeySign( privctx, hashctx, sigbuf, &sigbufsize);
		PGPFreePrivateKeyContext( privctx );
		privctx = NULL;

		pgpasn_UnpackSignature ( asnctx, &sig, sigbuf, sigbufsize, &err );
		if ( err )
			return kPGPError_LazyProgrammer;
		pgpasn_PutOctVal (asnctx, &sinfo->encryptedDigest, sig->val,
							sig->len);
		pgpasn_FreeSignature( asnctx, sig );

		PGPFreeData( sigbuf );
		sigbuf = NULL;
		sig = NULL;


		databufsize = pgpasn_SizeofSignedData (asnctx, sdata, TRUE);
		databuf = PGPNewData( mgr, databufsize, 0);
		pgpasn_PackSignedData( asnctx, databuf, databufsize, sdata, &err );
		if ( err )
			return kPGPError_LazyProgrammer;
// crashes with this
//		pgpasn_FreeSignedData( sdata );

		ci0 = pgpasn_NewContentInfo (asnctx);
		pgpasn_PutOctVal (asnctx, &ci0->contentType, signeddataoid,
							sizeof(signeddataoid) );
		ci0->content = pgpasn_NewANY (asnctx);
		pgpasn_PutOctVal (asnctx, ci0->content, databuf, databufsize);
		PGPFreeData( databuf );
		databuf = NULL;
	}

	databufsize = pgpasn_SizeofContentInfo( asnctx, ci0, TRUE );
	databuf = PGPNewData( mgr, databufsize, 0);
	pgpasn_PackContentInfo( asnctx, databuf, databufsize, ci0, &err );
	pgpasn_FreeContentInfo( asnctx, ci0 );
	if ( err )
		return kPGPError_LazyProgrammer;

	*certReqOutSize = databufsize;
	*certReqOut = databuf;
	return kPGPError_NoErr;

#endif /* TESTASN1 */
}

#endif

#endif /* DEBUGX509 */
