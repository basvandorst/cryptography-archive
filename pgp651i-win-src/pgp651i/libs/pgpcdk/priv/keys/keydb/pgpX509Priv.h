/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpX509Priv.h,v 1.25 1999/05/19 22:34:04 hal Exp $
____________________________________________________________________________*/

#ifndef Included_pgpX509Priv_h
#define Included_pgpX509Priv_h

#include "pgpRngPriv.h"
#include "pgpX509Cert_asn.h"

PGP_BEGIN_C_DECLARATIONS

/* Internal functions */

PGPError		pgpX509BufferToCert (PGPContextRef context, PGPByte *buf,
						PGPSize len, PGPASN_Certificate **cert);

PGPError		pgpX509FreeCert (PGPContextRef context,
						PGPASN_Certificate *cert);

PGPError		pgpX509BufferToXTBSCert (PGPContextRef context, PGPByte *buf,
						PGPSize len, PGPASN_XTBSCertificate **xtbscert);

PGPError		pgpX509FreeXTBSCert (PGPContextRef context,
						PGPASN_XTBSCertificate *xtbscert);

PGPError		pgpX509BufferToCRL (PGPContextRef context, PGPByte *buf,
						PGPSize len, PGPASN_CertificateRevocationList **crl);

PGPError		pgpX509FreeCRL (PGPContextRef context,
						PGPASN_CertificateRevocationList *crl);

PGPError		pgpX509CertToSigBuffer (RingPool *pool,
						PGPASN_Certificate *cert, PGPByte loversion,
						PGPSize *pktlen);

PGPError		pgpX509CertToKeyBuffer (RingPool *pool,
						PGPASN_Certificate *cert, PGPByte loversion,
						PGPSize *pktlen);

PGPError		pgpX509CertToNameBuffer (RingPool *pool,
						PGPASN_Certificate *cert, PGPByte loversion,
						PGPSize *pktlen);

PGPError		pgpX509CertInfoPartition( PGPByte *buf, PGPSize len,
						PGPByte **serialbuf, PGPSize *seriallen,
						PGPByte **sigalgbuf, PGPSize *sigalglen,
						PGPByte **issuerbuf, PGPSize *issuerlen,
						PGPByte **validitybuf, PGPSize *validitylen,
						PGPByte **subjectbuf, PGPSize *subjectlen,
						PGPByte **publickeyinfobuf, PGPSize *publickeyinfolen);

PGPError		pgpX509SignatureBitString( PGPByte *buf, PGPSize len,
						PGPByte **rawbuf, PGPSize *rawlen );

PGPError		pgpX509CertPartition( PGPByte *buf, PGPSize len,
						PGPByte **certinfobuf, PGPSize *certinfolen,
						PGPByte **sigalgbuf, PGPSize *sigalglen,
						PGPByte **sigbuf, PGPSize *siglen );

PGPError		pgpX509DecodeTime( PGPASN_Time *tm, PGPUInt32 *pgptime );

PGPError		pgpX509CertToIASN( PGPByte *cert, PGPSize len,
						PGPByte *buf, PGPSize *pktlen );

PGPError		pgpX509CertToLongName( PGPByte *cert, PGPSize len,
						PGPBoolean doIssuer, PGPByte *buf, PGPSize *pktlen );

PGPError		pgpX509CertToDName( PGPByte *cert, PGPSize len,
						PGPBoolean doIssuer, PGPByte *buf, PGPSize *pktlen );

PGPError		pgpX509CertToIPDNS( PGPByte *cert, PGPSize len,
						PGPBoolean doIP, PGPByte *buf, PGPSize *pktlen );

PGPError		pgpDecodeX509Cert( PGPByte *buf, PGPSize len,
						PGPContextRef context, PGPKeySetRef *kset);

PGPError		pgpDecodeX509CertSet( PGPByte *buf, PGPSize len,
						PGPContextRef context, PGPKeySetRef *kset);

PGPError		pgpDecodeX509CRLSet( PGPByte *buf, PGPSize len,
						PGPContextRef context, PGPKeySetRef kset);

PGPError		pgpDecodePCKS8( PGPByte *buf, PGPSize len,
						PGPContextRef context, PGPKeySetRef *keys );

PGPError		pgpX509VerifySignedObject (PGPContextRef context,
						PGPPubKey const *pubkey, PGPByte *buf, PGPSize len);

PGPError		pgpKeyToX509SPKI(PGPContextRef context, RingSet const *set,
						RingObject *key, PGPByte *keyBuf, PGPSize *keyLength);

PGPBoolean		pgpX509BufInSequenceList( PGPByte const *buf, PGPSize bufsize,
						PGPByte const *seqlist, PGPSize seqlistsize );

PGPSize			pgpX509BufSizeofSequence( PGPByte const *buf, PGPSize bufsize);

PGPByte *		pgpX509XTBSCertToDistPoint (PGPASN_XTBSCertificate *xtbscert,
						PGPSize *pdpointlen);


/* Interface functions to TIS CMS layer */

#include "pgpOptionList.h"

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
            PGPByte           **certSet,    /* Cert output buffer */
            PGPSize            *certSetSize,
            PGPByte           **crlReqOut,  /* CRL output buffer */
            PGPSize            *crlReqOutSize
    );

/* Create the payload */
    PGPError
    X509CreateCertificateRequest (
            PGPContextRef       context,
            PGPKeyRef           key,        /* Key to export */
            PGPExportFormat     format,     /* Export format (CA) */
            PGPAttributeValue  *formatData, /* Formatting data */
            PGPUInt32           formatDataCount,
            PGPOptionListRef    passphrase, /* Passphrase if signing */
            PGPByte           **certReq,    /* Output buffer */
            PGPSize            *certReqSize /* Output buffer size */
    );


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
            PGPByte           **certReqOut,    /* Cert output buffer */
            PGPSize            *certReqOutSize /* Cert output buffer size */
    );


/* Create a GetCRL message */
PGPError
X509CreateCRLRequest (
    PGPContextRef       context,
    PGPKeyRef           key,            /* [IN] key for CA we request from */
    PGPByte             *crldistp,      /* [IN] asn.1 crl dist point ext(opt)*/
    PGPSize             crldistpsize,   /* [IN] size of crldistp */
    PGPExportFormat     format,         /* [IN] what type of CA */
    time_t              validity,       /* [IN] CRL for which date */
    PGPByte             **request,      /* [OUT] der encoded GetCRL msg */
    PGPSize             *requestsize    /* [OUT] size of der encoding */
	);


/* Create a distinguished name buffer */
PGPError
X509CreateDistinguishedName (
	PGPContextRef       context,
	char const			*str,		/* LDAP format string */
	PGPByte           **dname,    /* Output buffer */
	PGPSize            *dnamesize /* Output buffer size */
);


/* Import a PKCS-12 key */
PGPError
PKCS12InputKey (
    PGPContextRef	context,	/* [IN] pgp context */
    const PGPByte	*der,		/* [IN] pkcs #12 data */
    PGPSize		dersize,	/* [IN] pkcs #12 data size */
    const PGPByte	*pass,		/* [IN] MAC/decrypt password */
    PGPSize		passlen,	/* [IN] password length */
    PGPByte		**privkey,	/* [OUT] ASN.1 PrivateKeyInfo */
    PGPSize		*privkeysize,	/* [OUT] size of PrivateKeyInfo */
    PGPByte		**cert,
    PGPSize		*certSize);



PGP_END_C_DECLARATIONS

#endif /* pgpX509Priv.h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
