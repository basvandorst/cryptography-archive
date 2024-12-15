/* Copyright (C) 1998-9 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: May 13, 1999 */

#include "x509CMS.h"
#include "reginfo.h"
#include "pgpX509Priv.h"
#include "cert_oid.h"

/* pkcs-9 14 */
static PGPByte PKIid_ce_rsaExtensions_OID[] = {
	0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x0e
};
#define PKIid_ce_rsaExtensions_OID_LEN 9

/* returns the number of bytes required for the encoded version of the
   DN string */
static PGPSize
x509EncodedStringLength (const char *s, PGPSize slen)
{
	PGPSize bytes = 0;

	for (; slen > 0; s++, slen--, bytes++)
	{
		if (*s == '=' || *s == '\\' || *s == ',' || *s == '+')
			bytes++;
	}
	return (bytes);
}

/* encode a DN string for input to CMS.  assumes that the caller has allocated
   enough space to encode the string using the x509EncodedStringLength()
   function */
static PGPError
x509EncodeString (const char *src, PGPSize srclen, char *dest)
{
	for (; srclen > 0; src++, dest++, srclen--)
	{
		if (*src == '=' || *src == '\\' || *src == ',' || *src == '+')
			*dest++ = '\\';
		*dest = *src;
	}
	*dest = 0;
	return kPGPError_NoErr;
}

enum {
	kPGPPersonalAttr_CommonName,
	kPGPPersonalAttr_SerialNumber,
	kPGPPersonalAttr_Description
};

static PGPError
x509AppendDNAttribute(
	PGPMemoryMgrRef	mem,
	const char		*sep,
	const char		*type,
	const char		*val,
	PGPSize			valsize,
	char			**dn,
	PGPSize			*dnsize)
{
	*dnsize += strlen(type) + 2 + x509EncodedStringLength(val,valsize);
	PGPReallocData(mem, (void **)dn, *dnsize, kPGPMemoryMgrFlags_Clear);
	strcat(*dn, type);
	strcat(*dn, "=");
	x509EncodeString(val,valsize,*dn+strlen(*dn));
	strcat(*dn, sep);
	return kPGPError_NoErr;
}

static PGPError
x509CreateDistinguishedName (
	PGPMemoryMgrRef		mem,
	PGPExportFormat		caType,
	PGPAttributeValue	*formatData,
	PGPUInt32			formatDataCount,
	char				**dnString)
{
	const char	*dnType = NULL;
	PGPSize		bytesNeeded = 1;	/* 1 for nul char */
	PGPSize		i, j;
	char		*p;
	
	/* used when processing Entrust DN's */
	const char	*personaltype[3] = { "CN", "SerialNumber", "description" };
	char		*personal[3] = { NULL, NULL, NULL };
	size_t		personalsizes[3] = { 0, 0, 0 };

	for(i = 0; i < formatDataCount; i++)
	{
		switch(formatData[i].attribute)
		{
			case kPGPAVAttribute_CommonName:
				if(caType==kPGPExportFormat_EntrustV1_CertReq) /* jason */
				{
					personal[kPGPPersonalAttr_CommonName]=formatData[i].value.pointervalue;
					personalsizes[kPGPPersonalAttr_CommonName]=formatData[i].size;
					dnType=NULL;
				}
				else
					dnType = "CN";
				break;
			case kPGPAVAttribute_Email:
				dnType = "emailAddress";
				/* do a check to make sure we don't also have "embed_email=no"
				   for the reginfo field */
				for (j = 0; j < formatDataCount; j++)
				  {
					if (formatData[j].attribute == kPGPAVAttribute_EmbedEmail)
					  {
						if (!formatData[j].value.booleanvalue)
							dnType = NULL; /* don't include it */
						break;
					  }
				  }
				break;
			case kPGPAVAttribute_OrganizationName:
				dnType = "O";
				break;      
			case kPGPAVAttribute_OrganizationalUnitName:
				dnType = "OU";
				break;
			case kPGPAVAttribute_SurName:
				dnType = "SN";
				break;
			case kPGPAVAttribute_SerialNumber:
				if(caType==kPGPExportFormat_EntrustV1_CertReq) /* jason */
				{
					personal[kPGPPersonalAttr_SerialNumber]=formatData[i].value.pointervalue;
					personalsizes[kPGPPersonalAttr_SerialNumber]=formatData[i].size;
					dnType=NULL;
				}
				else
					dnType = "SerialNumber";
				break;
			case kPGPAVAttribute_Country:
				dnType = "C";
				break;
			case kPGPAVAttribute_Locality:
				dnType = "L";
				break;
			case kPGPAVAttribute_State:
				dnType = "ST";
				break;
			case kPGPAVAttribute_StreetAddress:
				dnType = "STREET";
				break;
			case kPGPAVAttribute_Title:
				dnType = "title";
				break;
			case kPGPAVAttribute_Description:
				if(caType==kPGPExportFormat_EntrustV1_CertReq) /* jason */
				{
					personal[kPGPPersonalAttr_Description]=formatData[i].value.pointervalue;
					personalsizes[kPGPPersonalAttr_Description]=formatData[i].size;
					dnType=NULL;
				}
				else
					dnType = "description";
				break;
			case kPGPAVAttribute_PostalCode:
				dnType = "postalCode";
				break;
			case kPGPAVAttribute_POBOX:
				dnType = "POBOX";
				break;
			case kPGPAVAttribute_PhysicalDeliveryOfficeName:
				dnType = "physicalDeliveryOfficeName";
				break;
			case kPGPAVAttribute_TelephoneNumber:
				dnType = "TN";
				break;
			case kPGPAVAttribute_X121Address:
				dnType = "x121Adress";
				break;
			case kPGPAVAttribute_ISDN:
				dnType = "ISDN";
				break;
			case kPGPAVAttribute_DestinationIndicator:
				dnType = "destinationIndicator";
				break;
			case kPGPAVAttribute_Name:
				dnType = "name";
				break;
			case kPGPAVAttribute_GivenName:
				dnType = "givenName";
				break;
			case kPGPAVAttribute_Initials:
				dnType = "initials";
				break;
				/* case kPGPAVAttribute_GenerationQualifier:
				   dnType = "generationQualifier";
				   break; */
			case kPGPAVAttribute_HouseIdentifier:
				dnType = "houseIdentifier";
				break;
			case kPGPAVAttribute_DirectoryManagementDomain:
				dnType = "dmdName";
				break;
			case kPGPAVAttribute_DomainComponent:
				dnType = "DC";
				break;
			case kPGPAVAttribute_UnstructuredName:
				dnType = "unstructuredName";
				break;
			case kPGPAVAttribute_UnstructuredAddress:
				dnType = "unstructuredAddress";
				break;

				/* VeriSign CRS RegInfo entries, skip these for now so we don't
				   generate a bogus error */
			case kPGPAVAttribute_Challenge:
			case kPGPAVAttribute_CertType:
			case kPGPAVAttribute_EmployeeID:
			case kPGPAVAttribute_MailStop:
			case kPGPAVAttribute_AdditionalField4:
			case kPGPAVAttribute_AdditionalField5:
			case kPGPAVAttribute_AdditionalField6:
			case kPGPAVAttribute_Authenticate:
			case kPGPAVAttribute_EmbedEmail:

				/* ExtensionReq attributes, skip these */
			case kPGPAVAttribute_RFC822Name:
			case kPGPAVAttribute_DNSName:
			case kPGPAVAttribute_AnotherName:
			case kPGPAVAttribute_IPAddress:
			case kPGPAVAttribute_CertificateExtension:
				dnType = NULL;
				break;

			default:
				return (kPGPError_X509AttributeNotSupported);
		}
		if(dnType && formatData[i].size > 0)
			x509AppendDNAttribute(mem,",",dnType,formatData[i].value.pointervalue,formatData[i].size,dnString,&bytesNeeded);
	}

	if(caType==kPGPExportFormat_EntrustV1_CertReq) /* jason */
	{
		for(i=0;i<=kPGPPersonalAttr_Description;i++)
			if(personal[i] && personalsizes[i] > 0)
				x509AppendDNAttribute(mem,"+",personaltype[i],personal[i],
					personalsizes[i],dnString,&bytesNeeded);
	}

	p = *dnString + strlen(*dnString) - 1;
	*p = 0;
	return(kPGPError_NoErr);
}

static PGPError
x509FormatDSAKey (
	PGPMemoryMgrRef	mgr,
	PKICONTEXT	*asnContext,
	PGPKeyRef	key,
	PGPByte		**y,
	PGPSize		*ySize,
	PGPByte		**pqg,
	PGPSize		*pqgSize)
{
	PKIINTEGER	*py;
	PKIDss_Parms	*ppqg;

	int asnError = 0;

	PGPByte *keydata, *kd;
	PGPSize keydatasize;

	int pbits, pbytes;
	int qbits, qbytes;
	int gbits, gbytes;
	int ybits, ybytes;

	PGPByte *pdata, *qdata, *gdata, *ydata;

	PGPError err;

	/* Read algorithm-specific key data into keydata buffer */
	err = PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, 0, NULL,
		&keydatasize );
	if (err != kPGPError_BufferTooSmall && IsPGPError (err))
		return err;
	keydata = PGPNewData( mgr, keydatasize, 0);
	err = PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, keydatasize,
		keydata,
		&keydatasize );
	if (IsPGPError (err))
	{
		PGPFreeData (keydata);
		return err;
	}

	/* Parse keydata buffer */
	kd = keydata;

	pbits = (kd[0] << 8) | kd[1];
	pbytes = (pbits + 7) / 8;
	pdata = kd + 2;
	kd += 2 + pbytes;

	qbits = (kd[0] << 8) | kd[1];
	qbytes = (qbits + 7) / 8;
	qdata = kd + 2;
	kd += 2 + qbytes;

	gbits = (kd[0] << 8) | kd[1];
	gbytes = (gbits + 7) / 8;
	gdata = kd + 2;
	kd += 2 + gbytes;

	ybits = (kd[0] << 8) | kd[1];
	ybytes = (ybits + 7) / 8;
	ydata = kd + 2;
	kd += 2 + ybytes;

	/* This leaves pbytes, qbytes, gbytes, ybytes holding the length
           of the data buffers, with
	   pdata, qdata, gdata, ydata pointing at the raw numeric data
	   (with no leading zeros) */

	/* clear outputs */
	*y = NULL;
	*ySize = 0;
	*pqg = NULL;
	*pqgSize = 0;

	if ((ppqg = PKINewDss_Parms (asnContext)) == NULL)
	{
		PGPFreeData (keydata);
		return (kPGPError_OutOfMemory);
	}
	PKIPutUIntBytes (asnContext, &ppqg->p, pdata, pbytes);
	PKIPutUIntBytes (asnContext, &ppqg->q, qdata, qbytes);
	PKIPutUIntBytes (asnContext, &ppqg->g, gdata, gbytes);
	*pqgSize = PKISizeofDss_Parms (asnContext, ppqg, 1);
	*pqg = PGPNewData (mgr, *pqgSize, 0);
	PKIPackDss_Parms (asnContext, *pqg, *pqgSize, ppqg, &asnError);
	PKIFreeDss_Parms (asnContext, ppqg);
	if (asnError)
	{
		PGPFreeData (keydata);
		return kPGPError_LazyProgrammer;
	}

	py = PKINewINTEGER (asnContext);
	PKIPutUIntBytes (asnContext, py, ydata, ybytes);
	*ySize = PKISizeofINTEGER (asnContext, py, 1);
	*y = PGPNewData (mgr, *ySize, 0);
	PKIPackINTEGER (asnContext, *y, *ySize, py, &asnError);
	PKIFreeINTEGER (asnContext, py);
	if (asnError)
	{
		PGPFreeData (keydata);
		return kPGPError_LazyProgrammer;
	}

	PGPFreeData (keydata);

	return (kPGPError_NoErr);
}

static PGPError
x509FormatRSAKey (
	PGPMemoryMgrRef	mgr,
	PKICONTEXT	*asnContext,
	PGPKeyRef	key,
	PGPByte		**keyData,
	PGPSize		*keyDataSize,
	PGPByte		**paramData,
	PGPSize		*paramDataSize)
{
	PGPByte	*keydata, *kd;
	PGPSize	keydatasize;
	PGPByte	*e, *n;
	int		ebytes, nbytes;
	int		ebits, nbits;
	PKIRSAKey	*rsaKey;
	int		asnError = 0;
	PGPError	err;

	*keyData = NULL;
	*keyDataSize = 0;
	*paramData = NULL;
	*paramDataSize = 0;

	err = PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, 0, NULL,
		&keydatasize );
	if (err != kPGPError_BufferTooSmall && IsPGPError (err))
		return err;
	keydata = PGPNewData( mgr, keydatasize, 0);
	err = PGPGetKeyPropertyBuffer( key, kPGPKeyPropKeyData, keydatasize,
		keydata,
		&keydatasize );
	if (IsPGPError (err))
	{
		PGPFreeData (keydata);
		return err;
	}

	kd = keydata;

	nbits = (kd[0] << 8) | kd[1];
	nbytes = (nbits + 7) / 8;
	n = kd + 2;
	kd += 2 + nbytes;

	ebits = (kd[0] << 8) | kd[1];
	ebytes = (ebits + 7) / 8;
	e = kd + 2;

	/* clear outputs */
	*keyData = NULL;
	*keyDataSize = 0;

	/* pack n and e in an ASN.1 structure */
	rsaKey = PKINewRSAKey (asnContext);
	if (!rsaKey)
	{
		PGPFreeData (keydata);
		return kPGPError_OutOfMemory;
	}
	PKIPutUIntBytes (asnContext, &rsaKey->modulus, n, nbytes);
	PKIPutUIntBytes (asnContext, &rsaKey->exponent, e, ebytes);

	PGPFreeData (keydata);

	*keyDataSize = PKISizeofRSAKey (asnContext, rsaKey, 1);
	*keyData = PGPNewData (mgr, *keyDataSize, 0);
	if (!*keyData)
	{
		PKIFreeRSAKey (asnContext, rsaKey);
		return kPGPError_OutOfMemory;
	}

	PKIPackRSAKey (asnContext, *keyData, *keyDataSize, rsaKey, &asnError);
	PKIFreeRSAKey (asnContext, rsaKey);
	if (asnError)
		return kPGPError_LazyProgrammer;

	/* encoded as ASN.1 NULL value */
	*paramDataSize = 2;
	*paramData = PGPNewData (mgr, 2, 0);
	(*paramData)[0] = 0x05;
	(*paramData)[1] = 0x00;

	return kPGPError_NoErr;
}

static PGPError
x509CompileRegInfo (
	PKICONTEXT		*context,
	PGPAttributeValue	*formatData,
	PGPSize			formatDataCount,
	vri_ava_t		**av)
{
	int avCount = 0;
	size_t i;

	*av = PKIAlloc (context->memMgr, sizeof (vri_ava_t) * (formatDataCount + 1));
	for (i = 0; i < formatDataCount; i++)
	{
		switch (formatData[i].attribute)
		{
			case kPGPAVAttribute_Challenge:
				(*av)[avCount].type = "challenge";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_CertType:
				(*av)[avCount].type = "cert_type";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_CommonName:
				(*av)[avCount].type = "common_name";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_EmployeeID:
				(*av)[avCount].type = "employeeID";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_MailStop:
				(*av)[avCount].type = "mailStop";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_AdditionalField4:
				(*av)[avCount].type = "additional_field4";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_AdditionalField5:
				(*av)[avCount].type = "additional_field5";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_AdditionalField6:
				(*av)[avCount].type = "additional_field6";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_Authenticate:
				(*av)[avCount].type = "authenticate";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_EmbedEmail:
				(*av)[avCount].type = "embed_email";
				(*av)[avCount].value = formatData[i].value.booleanvalue ?
											"yes" : "no";
				(*av)[avCount].size = strlen((*av)[avCount].value);
				++avCount;
				break;

				/* stock X.500 attributes we also use here */
			case kPGPAVAttribute_OrganizationName:
				(*av)[avCount].type = "corp_company";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_OrganizationalUnitName:
				(*av)[avCount].type = "org_unit";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_Title:
				(*av)[avCount].type = "jobTitle";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			case kPGPAVAttribute_Email:
				(*av)[avCount].type = "mail_email";
				(*av)[avCount].size = formatData[i].size;
				(*av)[avCount++].value = formatData[i].value.pointervalue;
				break;
			default:
				break; /* do nothing */
		}
	}

	/* add termination record */
	(*av)[avCount].type = NULL;
	(*av)[avCount].size = 0;
	(*av)[avCount++].value = NULL;

	/* resize to actual size used */
	PKIRealloc (context->memMgr, (void **) av, sizeof (vri_ava_t) * avCount);

	return kPGPError_NoErr;
}

static PGPError
x509CRSToPGPError (int err)
{
	switch (err)
	{
		case VRI_E_MISSING_MANDATORY:
			return kPGPError_CRSMissingRequiredAttribute;
		case VRI_E_INVALID_CHAR:
			return kPGPError_CRSInvalidCharacter;
		case VRI_E_AVA_TYPE:
			return kPGPError_CRSInvalidAttributeType;
		case VRI_E_CERT_TYPE:
			return kPGPError_CRSInvalidCertType;
		case VRI_E_LENGTH:
			return kPGPError_CRSInvalidAttributeValueLength;
		case VRI_E_AUTHENTICATE:
			return kPGPError_CRSInvalidAuthenticateValue;
		default:
			return kPGPError_LazyProgrammer;
	}
}

#if 1
const PGPByte x509TestNameOid[] = { 0x2a, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
#define x509TestNameOidLen 8
#endif

static PGPError
x509AddGeneralName (
	PKICONTEXT *pki, /* [IN] */
	PGPByte tag, /* [IN] */
	const PGPByte *data, /* [IN] */
	PGPSize datasize, /* [IN] */
	PKIGeneralNames *gn) /* [OUT] */
{
	PKIAnotherName *on = NULL;

	gn->elt[gn->n] = PKINewGeneralName (pki);
	gn->elt[gn->n]->CHOICE_field_type = 0xA0 | tag;

	if (tag == 0)
	{
		on = PKINewAnotherName (pki);
		PKIPutOctVal (pki, &on->type_id, x509TestNameOid, x509TestNameOidLen);
		PKIPutOctVal (pki, &on->value, data, datasize);
		gn->elt[gn->n]->data = (void *) on;
	}
	else
	{
		PKIOCTET_STRING *os=PKINewOCTET_STRING(pki);
		PKIPutOctVal(pki,os,data,datasize);
		gn->elt[gn->n]->data = (void *) os;
	}
	gn->n++;
	return kPGPError_NoErr;
}

static PGPError
x509AddExtensionReq (
	PGPAttributeValue	*format,/* [IN] */
	PGPSize				formatcount, /* [IN] */
	TC_CONTEXT			*ctx, /* [IN] */
	TC_Attributes		*attr) /* [OUT] */
{
	int			asnerr = 0;
	PGPSize		n;
	PGPError	err;
	PGPByte *der = NULL;
	PGPSize dersize;
	PKIExtensions *ext=NULL;
	PKICONTEXT *pki = ctx->certasnctx;
	PKIGeneralNames *gn = PKINewGeneralNames (pki);
	
	ext = PKINewExtensions (pki);

	/* see if RFC822Name, DNSName, IPAddress or AnotherName are specified */
	for (n = 0; n < formatcount; n++)
	{
	  /* skip empty fields since they are not valid in ASN.1 */
	  if (format[n].size > 0)
		{
		  if (format[n].attribute == kPGPAVAttribute_RFC822Name)
			  x509AddGeneralName (pki, 1, format[n].value.pointervalue,
				  format[n].size, gn);
		  else if (format[n].attribute == kPGPAVAttribute_DNSName)
			  x509AddGeneralName (pki, 2, format[n].value.pointervalue,
				  format[n].size, gn);
#if 0 /* TODO: not finished */
		  else if (format[n].attribute == kPGPAVAttribute_AnotherName)
			  x509AddGeneralName (pki, 0, format[n].value.pointervalue,
				  format[n].size, gn);
#endif
		  else if (format[n].attribute == kPGPAVAttribute_IPAddress)
			  x509AddGeneralName (pki, 7, format[n].value.pointervalue,
				  format[n].size, gn);
		  else if (format[n].attribute == kPGPAVAttribute_CertificateExtension)
			{
			  PKIExtension *t;

			  PKIUnpackExtension (pki, &t, format[n].value.pointervalue,
				  format[n].size, &asnerr);
			  if (asnerr)
				{
				  err = kPGPError_InvalidCertificateExtension;
				  goto ERROR;
				}
			  ext->elt[ext->n] = t;
			  ext->n++;
			}
		}
	}

	if (gn->n)
	  {
		dersize = PKISizeofGeneralNames (pki, gn, TRUE);
		der = PKIAlloc (pki->memMgr, dersize);
		if (!der)
		  {
			err = kPGPError_OutOfMemory;
			goto ERROR;
		  }
		PKIPackGeneralNames (pki, der, dersize, gn, &asnerr);
		if (asnerr)
		  {
			err = kPGPError_ASNPackFailure;
			goto ERROR;
		  }

		ext->elt[ext->n] = PKINewExtension (pki);
		PKIPutOctVal (pki, &ext->elt[ext->n]->extnID,
			PKIid_ce_subjectAltName_OID, PKIid_ce_subjectAltName_OID_LEN);
		ext->elt[ext->n]->extnValue.val = der;
		ext->elt[ext->n]->extnValue.len = dersize;
		ext->n++;
	}

	if (ext->n)
	{
		dersize = PKISizeofExtensions (pki, ext, TRUE);
		der = PKIAlloc (pki->memMgr, dersize);
		if (!der)
		{
			err = kPGPError_OutOfMemory;
			goto ERROR;
		}
		PKIPackExtensions (pki, der, dersize, ext, &asnerr);
		if (asnerr)
		{
			err = kPGPError_ASNPackFailure;
			goto ERROR;
		}
		asnerr = tc_add_attribute (attr,
			PKIid_ce_rsaExtensions_OID,
			PKIid_ce_rsaExtensions_OID_LEN,
			der,
			dersize,
			ctx);
		if (asnerr)
		{
			err = kPGPError_LazyProgrammer;
			goto ERROR;
		}
	}

	err = kPGPError_NoErr;

ERROR:
	if (gn)
		PKIFreeGeneralNames (ctx->certasnctx,gn);
	if (ext)
		PKIFreeExtensions (pki,ext);
	if (der)
		PGPFreeData (der);
	return err;
}

PGPError x509CreateSubjectPublicKeyInfo (
	PGPKeyRef keyref,
	PKICONTEXT *asnContext,
	X509SubjectPublicKeyInfo *info)
{
	PGPError err;
	PGPInt32 keyAlgID;
	PGPMemoryMgrRef mem = PGPGetContextMemoryMgr (PGPGetKeyContext (keyref));
	const PGPByte rsaparm[2] = { 0x05, 0x00 };

	memset (info, 0, sizeof (X509SubjectPublicKeyInfo));

	/* determine which type of key we have */
	err = PGPGetKeyNumber (keyref, kPGPKeyPropAlgID, &keyAlgID);
	if (IsPGPError (err))
		return err;

	/* format the key and any parameters for PKCS-10 */
	if (keyAlgID == kPGPPublicKeyAlgorithm_DSA)
	{
		err = x509FormatDSAKey (mem,
			asnContext,
			keyref,
			&info->keyData,
			&info->keyDataSize,
			&info->keyParm,
			&info->keyParmSize);

		info->keyAlg = TC_ALG_DSA;
		info->keyAlgSize = TC_ALG_DSA_LEN;
		info->sigAlg = TC_ALG_DSA_SHA1;
		info->sigAlgSize = TC_ALG_DSA_SHA1_LEN;
	}
	else if (keyAlgID == kPGPPublicKeyAlgorithm_RSA  ||
		keyAlgID == kPGPPublicKeyAlgorithm_RSAEncryptOnly ||
		keyAlgID == kPGPPublicKeyAlgorithm_RSASignOnly)
	{
		err = x509FormatRSAKey (mem,
			asnContext,
			keyref,
			&info->keyData,
			&info->keyDataSize,
			&info->keyParm,
			&info->keyParmSize);

		info->keyAlg = TC_ALG_RSA;
		info->keyAlgSize = TC_ALG_RSA_LEN;
		info->sigAlg = TC_ALG_RSA_MD5;
		info->sigAlgSize = TC_ALG_RSA_MD5_LEN;
		info->sigParmSize = sizeof rsaparm;
		info->sigParm = PGPNewData (mem, info->sigParmSize, 0);
		memcpy (info->sigParm, rsaparm, info->sigParmSize);
	}
	else
	{
		/* we don't support whatever type of key this is */
		err = kPGPError_UnknownPublicKeyAlgorithm;
	}

	return err;
}

void
x509FreeSubjectPublicKeyInfo (X509SubjectPublicKeyInfo *info)
{
	if (info->keyData)
		PGPFreeData (info->keyData);
	if (info->keyParm)
		PGPFreeData (info->keyParm);
	if (info->sigParm)
		PGPFreeData (info->sigParm);
}

/* Create the payload */
PGPError
X509CreateCertificateRequest (
	PGPContextRef       context,
	PGPKeyRef           keyref,     /* Key to export */
	PGPExportFormat     format,     /* Export format (CA) */
	PGPAttributeValue  *formatData, /* Formatting data */
	PGPUInt32           formatDataCount,
	PGPOptionListRef    passphrase, /* Passphrase if signing */
	PGPByte           **certReq,    /* Output buffer */
	PGPSize            *certReqSize /* Output buffer size */
							 )
{
	TC_CONTEXT				*tcContext;
	PKICONTEXT				asnContext;
	TC_CertificationRequest	*certRequest = NULL;
	TC_Name					*subjectName = NULL;
	PGPMemoryMgrRef			mem = PGPGetContextMemoryMgr (context);
	X509CMSCallbackData		callbackData;
	char					*dnString = NULL;
	int						tcError;
	PGPError				err;
	X509SubjectPublicKeyInfo	spki;

	TC_Attributes			*attr = NULL;
	char					*reginfo = NULL;

	/* clear outputs */
	*certReq = NULL;
	*certReqSize = 0;

	/* initialize callback data */
	memset (&callbackData, 0, sizeof (callbackData));
	callbackData.passphrase = passphrase;
	callbackData.context = context;
	callbackData.key = keyref;

	/* initialize ASN.1 compiler */
	memset (&asnContext, 0, sizeof (asnContext));
	asnContext.memMgr = &X509CMSMemoryMgr;
	asnContext.memMgr->customValue = (void *) mem;

	memset (&spki, 0, sizeof spki);

	/* initialize CMS */
	tcError = tc_init_context (&tcContext,
		asnContext.memMgr,
		x509CMSSignCallback,
		&callbackData,
		NULL,
		NULL);
	if (tcError)
		return kPGPError_CMSInitialization;

	/* convert AVA array into printable string suitable for input to CMS */
	err = x509CreateDistinguishedName (mem,
		format,
		formatData,
		formatDataCount,
		&dnString);
	if (IsPGPError (err))
		goto error;

	tcError = tc_create_dname (&subjectName, tcContext);
	if (tcError)
	{
		err = kPGPError_LazyProgrammer;
		goto error;
	}
	tcError = tc_make_dname_fromstring (subjectName, dnString, tcContext);
	PGPFreeData (dnString);
	if (tcError)
	{
		err = kPGPError_InvalidDistinguishedName;
		goto error;
	}

	err = x509CreateSubjectPublicKeyInfo (keyref, &asnContext, &spki);
	if (IsPGPError (err))
		goto error;

	/* some CAs will barf on pkcs-10 with no attributes, so add a signing-time
	   to ensure we have at least one present */
	tcError = tc_create_attrlist (&attr, tcContext);
	tcError = tc_set_signing_time (attr, tcContext);

	/* determine if we need to add the ExtensionReq attribute */
	err = x509AddExtensionReq (formatData, formatDataCount, tcContext, attr);
	if (IsPGPError (err))
		goto error;

	tcError = tc_create_request (&certRequest,
		0, /* pkcs-10 1.5 requires version number 0 */
		spki.sigAlg,
		spki.sigAlgSize,
		spki.sigParm,
		spki.sigParmSize,
		subjectName,
		spki.keyAlg,
		spki.keyAlgSize,
		spki.keyData,	/* public key material */
		spki.keyDataSize,
		spki.keyParm,	/* key parameters.*/
		spki.keyParmSize,
		attr,	/* attributes */
		tcContext);

	if (tcError)
	{
		/* TODO: should this return a finer grain of error code? */
		err = kPGPError_CertRequestCreationFailure;
		goto error;
	}

	tcError = tc_pack_request (certReq, certReqSize, certRequest, tcContext);
	if (tcError)
	{
		err = kPGPError_CertRequestCreationFailure;
		goto error;
	}

	/* since we could have more than one crypto operation, the callbacks
	   make a copy of this, so free the master copy now */
	PGPFreeOptionList (passphrase);

	/* create CRS wrapper */
	if (format == kPGPExportFormat_VerisignV1_CertReq) /* jason */
	{
		vri_ava_t	*av;
		int			certType = -1;
		size_t		i;

		/* determine what type of certificate we are requesting */
		for (i = 0; i < formatDataCount; i++)
			if (formatData[i].attribute == kPGPAVAttribute_CertType)
			{
				if (!strncmp ((char *) formatData[i].value.pointervalue, "end-user",
							  formatData[i].size))
					certType = VRI_CERT_PERSONAL;
				else if (!strncmp ((char *) formatData[i].value.pointervalue, "ipsec",
								   formatData[i].size))
					certType = VRI_CERT_IPSEC;
				else if (!strncmp ((char *) formatData[i].value.pointervalue, "server",
								   formatData[i].size))
					certType = VRI_CERT_SECURE_SERVER;
				break;
			}
		if (certType == -1)
		{
			/* missing or bad cert_type */
			err = kPGPError_CRSInvalidCertType;
			goto error;
		}

		x509CompileRegInfo (&asnContext, formatData, formatDataCount, &av);
		tcError = vri_GenerateRegInfo (&asnContext,
			VRI_ENTITY_EE,
			certType,
			av, 
			&reginfo);
		PGPFreeData (av);
		if (tcError)
		{
			err = x509CRSToPGPError (tcError);
			goto error;
		}
	}

	if (format == kPGPExportFormat_VerisignV1_CertReq ||
		format == kPGPExportFormat_EntrustV1_CertReq) /* jason */
	{
		PGPByte *p10 = *certReq;
		PGPSize p10len = *certReqSize;

		tcError = vri_GeneratePKCSReq (&asnContext, 
			p10,
			p10len,
			reginfo,
			certReq,
			certReqSize);
		if (reginfo)
			PGPFreeData (reginfo);
		PGPFreeData (p10);
		if (tcError)
		{
			err = kPGPError_LazyProgrammer;
			goto error;
		}
	}
	err = kPGPError_NoErr;
error:
	x509FreeSubjectPublicKeyInfo (&spki);

	if (certRequest)
		tc_free_request (certRequest, tcContext);
	if (subjectName)
		tc_free_dname (subjectName, tcContext);
	if (attr)
		tc_free_attrlist (attr, tcContext);
	if (tcContext)
		tc_free_context (tcContext);

	if (IsPGPError (err))
	{
		if (*certReq)
		{
			PGPFreeData(*certReq);
			*certReq=NULL;
		}
		*certReqSize = 0;
	}
	return err;
}



/* Create a distinguished name buffer */
PGPError
X509CreateDistinguishedName (
	PGPContextRef       context,
	char const			*str,		/* LDAP format string */
	PGPByte           **dname,    /* Output buffer */
	PGPSize            *dnamesize /* Output buffer size */
)
{
	TC_CONTEXT				*tcContext = NULL;
	PKICONTEXT				asnContext;
	PKIName					*name = NULL;
	PGPMemoryMgrRef			mem = PGPGetContextMemoryMgr (context);
	PGPError				err = kPGPError_NoErr;

	*dname = NULL;

	/* initialize ASN.1 compiler */
	memset (&asnContext, 0, sizeof (asnContext));
	asnContext.memMgr = &X509CMSMemoryMgr;
	asnContext.memMgr->customValue = (void *) mem;

	/* initialize CMS */
	err = tc_init_context (&tcContext,
		asnContext.memMgr,
		NULL,
		NULL,
		NULL,
		NULL);
	if (err)
		return kPGPError_CMSInitialization;

	err = tc_create_dname (&name, tcContext);
	if (err)
	{
		err = kPGPError_LazyProgrammer;
		goto error;
	}
	err = tc_make_dname_fromstring (name, str, tcContext);
	if (err)
	{
		err = kPGPError_InvalidDistinguishedName;
		goto error;
	}

	*dnamesize = PKISizeofName (&asnContext, name, 1);
	*dname = PKIAlloc (asnContext.memMgr, *dnamesize);
	if( *dname == NULL )
	{
		err = kPGPError_OutOfMemory;
		goto error;
	}
	PKIPackName (&asnContext, *dname, *dnamesize, name, &err);
	PKIFreeName (&asnContext, name);
	name = NULL;
	if (err)
	{
		err = kPGPError_InvalidDistinguishedName;
		goto error;
	}

 error:

	if (name)
		PKIFreeName (&asnContext, name);
	if (tcContext)
		tc_free_context (tcContext);

	if (IsPGPError (err))
	{
		if (*dname)
		{
			PGPFreeData(*dname);
			*dname=NULL;
		}
		*dnamesize = 0;
	}
	return err;


}


/* vim:ts=4:sw=4:
 */
