/* Copyright (C) 1998-9 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: March 10, 1999 */

#include "x509CMS.h"
#include "pkcsreq_asn.h"
#include "pkcsreq_oid.h"
#include "pgpX509Priv.h"

/* number of bytes for random serial number created when generated a
   temporary self-signed X.509v3 certificate */
#define x509SerialNumberSize 8

/* number of seconds the temporary self-signed X.509 certificate is valid */
#define x509CertificateValidityPeriod 86400 /* 24 hours */

/* retrieves the X.509 certificate associated with a PGP key */
PGPError
x509ExtractCertificate (
	PGPContextRef	context,
	PGPKeyRef		key,
	PKICONTEXT		*asnContext,
	PKICertificate	**cert)
{
	PGPError		err;
	PGPByte			*certData = NULL;
	PGPSize			certDataSize = 0;
	PGPKeySetRef	keyset;
	int				asnError = 0;

	*cert = NULL;

	err = PGPNewSingletonKeySet (key, &keyset);
	if (IsPGPError (err))
		return (err);
	err = PGPExportKeySet (keyset,
		PGPOAllocatedOutputBuffer (context, (void **) &certData, 4096, &certDataSize),
		PGPOExportFormat (context, kPGPExportFormat_X509Cert),
		PGPOLastOption (context));
	PGPFreeKeySet (keyset);
	if (IsPGPError (err))
	{
		if (certData)
			PGPFreeData (certData);
		return (err);
	}
	PKIUnpackCertificate (asnContext,
		cert,
		certData,
		certDataSize,
		&asnError);
	PGPFreeData (certData);
	/*
	 * Ignore error return, as cert will be null in that case and we will
	 * construct a new one.
	 */
	return kPGPError_NoErr;
}

/* create a self-signed certificate with the same dname as that stored in
   the given certificate request if specified */
static PGPError
x509CreateSelfSignedCertificate (
	PGPKeyRef				signWith,
	PKICONTEXT				*context,
	X509CMSCallbackData		*data,
	PGPByte					*certReq,
	PGPSize					certReqSize,
	PKICertificate			**cert)
{
	TC_CONTEXT			*tcContext;
	int					tcerr = 0;
	time_t				now;
	unsigned char		serial[x509SerialNumberSize];
	PKICertificationRequest	*req = NULL;
	PKIPKCSReq			*pkcsreq = NULL;
	TC_CERT				*tccert = NULL;
	PGPByte				*certData = NULL;
	PGPSize				certDataSize;
	PGPError			err;
	TC_Name				*name = NULL, *pname = NULL;
	char				*dn = NULL;
	PGPMemoryMgrRef		mem = PGPGetContextMemoryMgr (PGPGetKeyContext (signWith));
	X509SubjectPublicKeyInfo spki;

	*cert = NULL;

	/* initialize CMS */
	tcerr = tc_init_context (&tcContext,
		context->memMgr,
		x509CMSSignCallback,
		(void *) data,
		NULL,
		NULL);
	if (tcerr)
	{
		err = kPGPError_CMSInitialization;
		goto error_exit;
	}

	if (certReq)
	{
		/* unpack the request into its component parts and fetch the pkcs-10
		   body */
		PKIUnpackPKCSReq (context, &pkcsreq, certReq, certReqSize, &tcerr);
		if (tcerr)
		{
			err = kPGPError_LazyProgrammer;
			goto error_exit;
		}
		PKIUnpackCertificationRequest (context,
			&req,
			pkcsreq->endEntityInfo.val,
			pkcsreq->endEntityInfo.len,
			&tcerr);
		PKIFreePKCSReq (context, pkcsreq);
		if (tcerr)
		{
			err = kPGPError_LazyProgrammer;
			goto error_exit;
		}

		pname = &req->certificationRequestInfo.subject;
	}
	else
	{
		/* generate a simple DN based on the key user id */
		
		char buf[256], *p;
		PGPSize bufsize;
		PGPUserIDRef userid;
		
		err = PGPGetPrimaryUserID (signWith, &userid);
		if (IsPGPError (err))
			goto error_exit;
		err = PGPGetUserIDStringBuffer (userid, kPGPUserIDPropCommonName, sizeof (buf), buf, &bufsize);
		if (IsPGPError (err))
			goto error_exit;
		
		/* Make buf a C string */
		buf[bufsize] = 0;

		/* if this is in the form of an email address, erase the address
		   portion since it contains invalid chars for the common name type */
		if ((p = strchr (buf, '<')) != NULL)
		{
			*p-- = 0;
			while (p>buf && (*p == ' ' || *p == '\t'))
				*p-- = 0;
		}
		
		dn = PGPNewData (mem, bufsize + 4, 0);
		if (!dn)
		{
			err = kPGPError_OutOfMemory;
			goto error_exit;
		}
		strcpy (dn, "cn=");
		strcat (dn, buf);

		tcerr = tc_create_dname (&name, tcContext);
		if (tcerr)
		{
			err = kPGPError_LazyProgrammer;
			goto error_exit;
		}
		tcerr = tc_make_dname_fromstring (name, dn, tcContext);
		if (tcerr)
		{
			err = kPGPError_LazyProgrammer;
			goto error_exit;
		}
		pname = name;
	}

	err = x509CreateSubjectPublicKeyInfo (signWith,
		tcContext->certasnctx,
		&spki);
	if (IsPGPError (err))
		goto error_exit;

	/* generate a random serial number for this cert */
	err = PGPContextGetRandomBytes (PGPGetKeyContext (signWith),
		serial,
		x509SerialNumberSize);
	if (IsPGPError (err))
		goto error_exit;

	serial[0] &= ~0x80; /* ensure serial number is a positive integer */

	time (&now);
	tcerr = tc_create_cert (&tccert,
		TC_X509_VERSION_1,		/* version1 -- no extensions */
		serial,
		x509SerialNumberSize,
		spki.sigAlg,
		spki.sigAlgSize,
		spki.sigParm,
		spki.sigParmSize,
		pname,
		now, /* now */
		now + x509CertificateValidityPeriod,
		pname,
		NULL, /* extensions */
		spki.keyAlg,
		spki.keyAlgSize,
		spki.keyData,
		spki.keyDataSize,
		spki.keyParm,
		spki.keyParmSize,
		tcContext);
	if (tcerr)
		goto error_exit;
	tcerr = tc_pack_cert (&certData, &certDataSize, tccert, tcContext);
	if (tcerr)
		goto error_exit;
	PKIUnpackCertificate (context, cert, certData, certDataSize, &tcerr);

error_exit:

	x509FreeSubjectPublicKeyInfo (&spki);
	if (certData)
		PGPFreeData (certData);
	if (tccert)
		tc_free_cert (tccert, tcContext);
	if (name)
		tc_free_dname (name, tcContext);
	tc_free_context (tcContext);
	if (req)
		PKIFreeCertificationRequest (context, req);
	if (dn)
		PGPFreeData (dn);

	return (tcerr ? kPGPError_LazyProgrammer : kPGPError_NoErr);
}

static PGPError
x509HashPublicKey (
	PKICONTEXT				*context,
	PKISubjectPublicKeyInfo	*key,
	PGPByte					**hash,
	PGPSize					*hashsize)
{
	PGPByte		*s;
	PGPSize		ssize;
	PGPError		err;
	PGPHashContextRef	hashref = NULL;
	int			asnerror = 0;

	*hash = NULL;
	*hashsize = 0;

	ssize = PKISizeofSubjectPublicKeyInfo (context, key, 1);
	s = PGPNewData (context->memMgr->customValue, ssize, 0);
	PKIPackSubjectPublicKeyInfo (context, s, ssize, key, &asnerror);
	if (asnerror)
	{
		err = kPGPError_ASNPackFailure;
		goto error;
	}
	err = PGPNewHashContext (context->memMgr->customValue, kPGPHashAlgorithm_MD5, &hashref);
	if (IsPGPError (err))
		goto error;
	err = PGPContinueHash (hashref, s, ssize);
	if (IsPGPError (err))
		goto error;
	*hashsize = 16;
	*hash = PGPNewData (context->memMgr->customValue, 16, 0);
	err = PGPFinalizeHash (hashref, *hash);
	if (IsPGPError (err))
	{
		PGPFreeData (*hash);
		*hash = NULL;
		*hashsize = 0;
	}
error:
	if (hashref)
		PGPFreeHashContext (hashref);
	PGPFreeData (s);
	return err;
}

static PGPError
x509AddAuthAttribute (
	PKICONTEXT		*context,
	const unsigned char	*oid,
	size_t			oidsize,
	const unsigned char	*val,
	size_t			valsize,
	PKIAttributes		*attr)
{
	attr->elt[attr->n] = PKINewAttribute (context);
	PKIPutOctVal (context,
		&attr->elt[attr->n]->type,
		oid,
		oidsize);
	attr->elt[attr->n]->values.n = 1;
	attr->elt[attr->n]->values.elt[0] = PKINewANY (context);
	PKIPutOctVal (context,
		attr->elt[attr->n]->values.elt[0],
		val,
		valsize);
	attr->n++;
	return kPGPError_NoErr;
}

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
	int					result, asnError = 0, isGetCertInitial;
	int					msgtype, isCertReq, isGetCrl;
#ifdef X509_ENTRUST_HACK
	int					isEntrust;
	size_t				i;
#endif /* X509_ENTRUST_HACK */
	PGPByte				*inputData = certReqIn;
	PGPSize				inputDataSize = certReqInSize;
	X509CMSCallbackData	pgpData;
	PKICONTEXT			asnContext;
	PGPError			err;
	PGPMemoryMgrRef		mem = PGPGetContextMemoryMgr (context);
	PKICertificate		*sigCert = NULL;
	PKIAttributes		*attr = NULL;
	List				*includeCerts = NULL;

	(void) encryptAlg; /* unused */

	/* clear outputs */
	*certReqOut = NULL;
	*certReqOutSize = 0;

	isGetCertInitial = (format == kPGPOutputFormat_X509GetCertInitialInPKCS7 ||
		format == kPGPOutputFormat_NetToolsCAV1_GetCertInitialInPKCS7 ||
		format == kPGPOutputFormat_VerisignV1_GetCertInitialInPKCS7 ||
		format == kPGPOutputFormat_EntrustV1_GetCertInitialInPKCS7);

	isCertReq = (format == kPGPOutputFormat_X509CertReqInPKCS7 ||
		format == kPGPOutputFormat_VerisignV1_CertReqInPKCS7 ||
		format == kPGPOutputFormat_EntrustV1_CertReqInPKCS7); /* jason */

	isGetCrl = (format == kPGPOutputFormat_X509GetCRLInPKCS7 ||
		format == kPGPOutputFormat_NetToolsCAV1_GetCRLInPKCS7 ||
		format == kPGPOutputFormat_VerisignV1_GetCRLInPKCS7 ||
		format == kPGPOutputFormat_EntrustV1_GetCRLInPKCS7);

#ifdef X509_ENTRUST_HACK
	isEntrust = (format == kPGPOutputFormat_EntrustV1_CertReqInPKCS7 ||
		format == kPGPOutputFormat_EntrustV1_GetCertInitialInPKCS7 ||
		format == kPGPOutputFormat_EntrustV1_GetCRLInPKCS7);
#endif /* X509_ENTRUST_HACK */

	if (isGetCertInitial || isCertReq || isGetCrl)
	{
		memset (&pgpData, 0, sizeof (pgpData));
		pgpData.passphrase = passphrase;
		pgpData.context = context;

		memset (&asnContext, 0, sizeof (PKICONTEXT));
		asnContext.memMgr = &X509CMSMemoryMgr;
		asnContext.memMgr->customValue = mem;

#if 0
		if (encryptTo)
		{
			EncryptRecipient	 recipEntry;
			List		 recipientCertificates;
			const char		*encryptAlgString;

			switch (encryptAlg)
			{
				case kPGPCipherAlgorithm_3DES:
					encryptAlgString = SM_OID_ALG_3DES;
					break;
				default:
					return kPGPError_BadSessionKeyAlgorithm;
			}

			pgpData.key = encryptTo;

			memset (&recipientCertificates, 0, sizeof (List));
			recipientCertificates.data = (void *) &recipEntry;

			memset (&recipEntry, 0, sizeof (EncryptRecipient));

			err = PGPNewSingletonKeySet (encryptTo, &keyset);
			err = PGPExportKeySet (keyset,
				PGPOAllocatedOutputBuffer (context, (void **) &cert, 4096, &certLength),
				PGPOExportFormat (pgpData.context, kPGPExportFormat_X509Cert),
				PGPOLastOption (pgpData.context));
			PGPFreeKeySet (keyset);
			PKIUnpackCertificate (&asnContext,
				&recipEntry.certificate,
				cert,
				certLength,
				&asnError);
			PGPFreeData (cert);
			if (asnError)
			{
			}

			result = sm_EncryptMessage (certReqOut,
				certReqOutSize,
				certReqIn,
				certReqInSize,
				(signWith != NULL),
				SM_OID_CONTENT_DATA,
				encryptAlgString,
				&recipientCertificates,
				pkcs7EncryptCallback,
				(void *) &pgpData,
				&asnContext);
			if (result != PKCS7_OK)
			{
			}
			PKIFreeCertificate (&asnContext, recipEntry.certificate);
			PGPFreeData (cert);

			inputData = *certReqOut;
			inputDataSize = *certReqOutSize;
		}
#endif /* encryptTo */

		if (signWith)
		{
			/* algs hard hardcoded for now */
			const char	*digestAlgString;
			const char	*signatureAlgString;
			PGPInt32	keyAlgID;
			char		rsaSigParam[2] = { 0x05, 0x00 };
			char		*sigParam = NULL;
			size_t		sigParamSize = 0;
			char		md5param[2] = { 0x05, 0x00 };
			char		*digestParam = NULL;
			size_t		digestParamSize = 0;

			PKIINTEGER	*type;
			PKIOCTET_STRING	*nonce;
			PGPByte		*val;
			PGPSize		len;

			pgpData.key = signWith;

			/* determine pub key algorithm */
			err = PGPGetKeyNumber (signWith, kPGPKeyPropAlgID, &keyAlgID);
			if (IsPGPError (err))
				goto error_exit;
			switch (keyAlgID)
			{
				case kPGPPublicKeyAlgorithm_DSA:
					digestAlgString = SM_OID_ALG_SHA;
					signatureAlgString = SM_OID_ALG_DSA;
					break;
				case kPGPPublicKeyAlgorithm_RSA:
				case kPGPPublicKeyAlgorithm_RSASignOnly:
				case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
					digestAlgString = SM_OID_ALG_MD5;
					signatureAlgString = SM_OID_ALG_RSA;
					/* VeriSign requires the parameters for the digestAlg
					   and sigAlg to be explicitly encoded as ASN.1 NULL
					   rather than be omitted */
					sigParam = rsaSigParam;
					sigParamSize = sizeof rsaSigParam;
					digestParam = md5param;
					digestParamSize = sizeof md5param;
					break;
				default:
					err = kPGPError_UnknownPublicKeyAlgorithm;
					goto error_exit;
			}

			/* Verisign requires self-signed messages */
			if ( !(format==kPGPOutputFormat_VerisignV1_GetCertInitialInPKCS7 ||
				   format==kPGPOutputFormat_VerisignV1_GetCRLInPKCS7 ) )
			{
				err = x509ExtractCertificate (context,
					signWith,
					&asnContext,
					&sigCert);

				/* TODO: presumably we must check the error for the case where
				   no certificate for the key exists? */
				if (IsPGPError (err))
					goto error_exit;
			}

			if (!sigCert)
			{
				/* no certificate for this key, assume we need to generate
				   a temporary self-signed certificate for this message,
				   as prescribed by the VeriSign CRS specification */

				/* if this is a PKCSReq message, use the information in the
				   PKCS-10 request as the basis for the DN for the self-signed
				   cert.  otherwise generate a simple certificate. */
				err = x509CreateSelfSignedCertificate (signWith,
					&asnContext,
					&pgpData,
					isCertReq ? certReqIn : NULL,
					isCertReq ? certReqInSize : 0,
					&sigCert);
				if (IsPGPError (err))
					goto error_exit;
			}

			if( sigCert )
			{
				includeCerts = PGPNewData (mem, sizeof (List),
										   kPGPMemoryMgrFlags_Clear);
				if (!includeCerts)
				{
					err = kPGPError_OutOfMemory;
					goto error_exit;
				}
				includeCerts->data = (void *) sigCert;
			}

			/* ----- begin building up the pkcs-7 message ----- */

			attr = PKINewAttributes (&asnContext);

			/* add the message-type attribute */
			type = PKINewINTEGER (&asnContext);
			
			msgtype = isCertReq ? 19 /* PKCSReq */ : (isGetCertInitial ? 20 /* GetCertInitial */ : 22 /* GetCRL */);

#ifdef X509_ENTRUST_HACK
			if (isEntrust) {
				char str[3];
				
				i=0;
				if(msgtype>10)
					str[i++]=msgtype/10+'0';
				str[i++]=msgtype%10+'0';
				str[i]=0;
				PKIPutStrVal(&asnContext, type, str);
				len=PKISizeofPrintableString(&asnContext, type, 1);
				val=PGPNewData(mem,len,0);
				if(!val) {
					err=kPGPError_OutOfMemory;
					goto error_exit;
				}
				PKIPackPrintableString(&asnContext, val, len, type, &asnError);
			} else {
#endif /* X509_ENTRUST_HACK */
				PKIPutIntVal (&asnContext, type, msgtype);
				len = PKISizeofINTEGER (&asnContext, type, 1);
				val = PGPNewData (mem, len, 0);
				PKIPackINTEGER (&asnContext, val, len, type, &asnError);
#ifdef X509_ENTRUST_HACK
			}
#endif
			PKIFreeINTEGER (&asnContext, type);
			if (asnError)
				err = kPGPError_ASNPackFailure;

			if (IsPGPError (err)) {
				PGPFreeData (val);
				goto error_exit;
			}

			err = x509AddAuthAttribute (&asnContext,
				PKIat_pki_message_type_OID,
				PKIat_pki_message_type_OID_LEN,
				val,
				len,
				attr);

			PGPFreeData (val);

			if (IsPGPError (err))
				goto error_exit;

			/* add the sendernonce attribute */
			nonce = PKINewOCTET_STRING (&asnContext);
			nonce->len = 16;
			nonce->val = PGPNewData (mem, nonce->len, 0);
			err = PGPContextGetRandomBytes (context,
				nonce->val,
				nonce->len);
			if (IsPGPError (err))
			{
				PKIFreeOCTET_STRING (&asnContext, nonce);
				goto error_exit;
			}
			len = PKISizeofOCTET_STRING (&asnContext, nonce, 1);
			val = PGPNewData (mem, len, 0);
			PKIPackOCTET_STRING (&asnContext, val, len, nonce, &asnError);
			PKIFreeOCTET_STRING (&asnContext, nonce);
			if (!asnError)
				err = x509AddAuthAttribute (&asnContext,
					PKIat_pki_sendernonce_OID,
					PKIat_pki_sendernonce_OID_LEN,
					val,
					len,
					attr);
			else
				err = kPGPError_ASNPackFailure;

			PGPFreeData (val);

			if (IsPGPError (err))
				goto error_exit;

			/* add the transactionid attribute */
			err = x509HashPublicKey (&asnContext,
				&sigCert->tbsCertificate.subjectPublicKeyInfo,
				&val,
				&len);
			if (IsPGPError (err))
				goto error_exit;

			type = PKINewINTEGER (&asnContext);
#ifdef X509_ENTRUST_HACK
			if (isEntrust) {
				type->len=len*2;
				type->val=PGPNewData(mem, type->len + 1, 0);
				for(i=0;i<len;i++)
					sprintf(type->val + (2 * i), "%2.2X", (unsigned int) val[i]);
				PGPFreeData (val);

				len = PKISizeofPrintableString (&asnContext, type, 1);
				val = PGPNewData (mem, len, 0);
				PKIPackPrintableString (&asnContext, val, len, type, &asnError);
			} else {
#endif /* X509_ENTRUST_HACK */
				PKIPutUIntBytes (&asnContext, type, val, len);
				PGPFreeData (val);
				len = PKISizeofINTEGER (&asnContext, type, 1);
				val = PGPNewData (mem, len, 0);
				PKIPackINTEGER (&asnContext, val, len, type, &asnError);
#ifdef X509_ENTRUST_HACK
			}
#endif
			PKIFreeINTEGER (&asnContext, type);
			if (asnError)
				err = kPGPError_ASNPackFailure;
			if (IsPGPError (err)) {
				PGPFreeData (val);
				goto error_exit;
			}

			err = x509AddAuthAttribute (&asnContext,
				PKIat_pki_transactionid_OID,
				PKIat_pki_transactionid_OID_LEN,
				val,
				len,
				attr);

			PGPFreeData (val);

			if (IsPGPError (err))
				goto error_exit;

			result = sm_SignMessage (certReqOut,
				certReqOutSize,
				inputData,
				inputDataSize,
				0,			/* not nested, outer content type */
				encryptTo ? SM_OID_CONTENT_ENVELOPED_DATA : SM_OID_CONTENT_DATA,
				digestAlgString,
				(uchar *) digestParam,
				digestParamSize,
				signatureAlgString,
				(uchar *) sigParam,
				sigParamSize,
				sigCert,
				attr,		/* signed attributes */
				includeCerts,	/* certs to include in PKCS7 message */
				pkcs7HashCallback,
				(void *) &pgpData,
				pkcs7SignCallback,
				(void *) &pgpData,
				&asnContext);
			if (result != PKCS7_OK)
			{
				err = kPGPError_PKCS7SignFailure;
				goto error_exit;
			}
		}
	}
	else if (format == kPGPOutputFormat_NetToolsCAV1_CertReqInPKCS7) /* jason: what if we ever want */
	{																 /* to create a real PKCS7 cert? */
		/* no extra encoding required, just return what we were given */

		*certReqOutSize = certReqInSize;
		*certReqOut = PGPNewData (PGPGetContextMemoryMgr (context),
			*certReqOutSize,
			0);
		memcpy (*certReqOut, certReqIn, *certReqOutSize);
	}
	else
		return kPGPError_InvalidOutputFormat;

	err = kPGPError_NoErr;

error_exit:

	if (attr)
		PKIFreeAttributes (&asnContext, attr);
	if (sigCert)
		PKIFreeCertificate (&asnContext, sigCert);
	if (includeCerts)
		PGPFreeData (includeCerts);

	if (inputData != certReqIn)
	{
		/* free intermediate data */
		PKIFree (asnContext.memMgr, inputData);
	}

	if (IsPGPError (err))
	{
		/* clean up allocated data on failure */
		if (*certReqOut)
		{
			PGPFreeData (*certReqOut);
			*certReqOut = NULL;
			*certReqOutSize = 0;
		}
	}

	/* callback functions just make a copy of this since we could possible
	   use it more than once, so free the master copy */
	PGPFreeOptionList (passphrase);

	return (err); /* success */
}

/* vim:ts=4:sw=4:
 */
