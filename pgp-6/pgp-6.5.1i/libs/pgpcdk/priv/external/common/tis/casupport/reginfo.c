/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: April 28, 1999 */

#include <ctype.h>
#include "reginfo.h"

/* field is mandatory for both RA and EE */
#define VRI_R_MANDATORY		1

/* value must be "end-user" */
#define VRI_R_VAL_END_USER		(1<<1)

/* value must be "yes" or "no" */
#define VRI_R_VAL_YES_OR_NO		(1<<2)

/* types is EMAIL rather than TEXT */
#define VRI_R_TYPE_EMAIL		(1<<4)

/* max length of string is 32 */
#define VRI_R_MAX_LEN_32		(1<<5)

/* value must be "yes", "no", or "pending" */
#define VRI_R_VAL_YES_NO_PENDING	(1<<6)

/* field is mandatory for EE */
#define VRI_R_MANDATORY_EE		(1<<7)

/* field is mandatory for RA */
#define VRI_R_MANDATORY_RA		(1<<8)

/* value must be "ipsec" */
#define VRI_R_VAL_IPSEC			(1<<9)

/* value must be "server" */
#define VRI_R_VAL_SERVER		(1<<10)

typedef struct {
    const char *name;
    int restrictions;
} vri_reginfo_t;

vri_reginfo_t VriPersonal[] = {
    { "corp_company",		VRI_R_MANDATORY },
    { "org_unit",		VRI_R_MANDATORY },
    { "cert_type",		VRI_R_MANDATORY | VRI_R_VAL_END_USER },
	{ "common_name",	VRI_R_MANDATORY },
    { "embed_email",		VRI_R_VAL_YES_OR_NO },
    { "mail_email",		VRI_R_MANDATORY | VRI_R_TYPE_EMAIL },
    { "jobTitle",		0 },
    { "employeeID",		0 },
    { "mailStop",		0 },
    { "additional_field4",	0 },
    { "additional_field5",	0 },
    { "additional_field6",	0 },
    { "challenge",		VRI_R_MANDATORY_EE | VRI_R_MAX_LEN_32 },
    { "authenticate",		VRI_R_MANDATORY_RA | VRI_R_VAL_YES_NO_PENDING },
    { NULL }
};

/* this is the current state as of 04/28/99 (from verisign crs profile dated
   April 19, 1999) */
vri_reginfo_t VriIPSEC[] = {
    { "corp_company",	VRI_R_MANDATORY },
    { "org_unit",		VRI_R_MANDATORY },
    { "cert_type",		VRI_R_MANDATORY | VRI_R_VAL_IPSEC },
	{ "common_name",	0 },
	{ "embed_email",	0 },
	{ "mail_email",		VRI_R_MANDATORY | VRI_R_TYPE_EMAIL },
    { "challenge",		VRI_R_MANDATORY_EE | VRI_R_MAX_LEN_32 },
    { "authenticate",	VRI_R_MANDATORY_RA | VRI_R_VAL_YES_NO_PENDING },
    { NULL }
};

vri_reginfo_t VriSecureServer[] = {
    { "corp_company",		VRI_R_MANDATORY },
    { "org_unit",		VRI_R_MANDATORY },
    { "cert_type",		VRI_R_MANDATORY | VRI_R_VAL_SERVER },
    { "challenge",		VRI_R_MANDATORY_EE | VRI_R_MAX_LEN_32 },
    { "authenticate",		VRI_R_MANDATORY_RA | VRI_R_VAL_YES_NO_PENDING },
    { NULL }
};

vri_reginfo_t VriRA[] = {
    { "cert_type",		VRI_R_MANDATORY | VRI_R_VAL_SERVER },
    { "tech_firstName",		VRI_R_MANDATORY },
    { "tech_lastName",		VRI_R_MANDATORY },
    { "tech_email",		VRI_R_MANDATORY | VRI_R_TYPE_EMAIL },
    { "challenge",		VRI_R_MANDATORY_EE | VRI_R_MAX_LEN_32 },
    { NULL }
};

static char vri_Hex[16] = "0123456789ABCDEF";

static int vri_ValidTextChar (unsigned char c)
{
    return (isalnum (c) || c == ' ' || (strchr ("'()+,-./:=?", c) != NULL));
}

static int
vri_ValidEmailChar (unsigned char c)
{
    return (isalnum (c) || strchr ("@:_+-[]\".", c) != NULL);
}

static char *
vri_AddChar (
	PKICONTEXT	*context,
	char		c,
	char		*encodedString,
	size_t		*encodedStringLen,
	size_t		*encodedStringSize)
{
    if (*encodedStringLen == *encodedStringSize)
    {
	*encodedStringSize += 4;
	PKIRealloc (context->memMgr,
		(void **) &encodedString,
		*encodedStringSize);
    }
    encodedString[*encodedStringLen] = c;
    (*encodedStringLen)++;
    return (encodedString);
}

static char *
vri_AddHexChar (
	PKICONTEXT *context,
	unsigned char c,
	char *encodedString,
	size_t *encodedStringLen,
	size_t *encodedStringSize)
{
    encodedString = vri_AddChar (context,
	    '%',
	    encodedString,
	    encodedStringLen,
	    encodedStringSize);
    encodedString = vri_AddChar (context,
	    vri_Hex[c / 16],
	    encodedString,
	    encodedStringLen,
	    encodedStringSize);
    encodedString = vri_AddChar (context,
	    vri_Hex[c % 16],
	    encodedString,
	    encodedStringLen,
	    encodedStringSize);
    return (encodedString);
}

/* application/x-www-form-urlencoded content-transfer-encoding
   (RFC1866 8.2.1) */

static int
vri_UrlEncode (
	PKICONTEXT *context,
	const char *inputString,
	size_t inputSize,
	char **encodedString)
{
    size_t encodedStringLen = 0;
    size_t encodedStringSize;
    unsigned i;
    
    encodedStringSize = inputSize + 1;
    *encodedString = PKIAlloc (context->memMgr, encodedStringSize);

    for (i=0; i<inputSize; i++)
    {
	if (strchr ("&+=%", inputString[i]))
	    *encodedString = vri_AddHexChar (context,
		    inputString[i],
		    *encodedString,
		    &encodedStringLen,
		    &encodedStringSize);
	else
	    *encodedString = vri_AddChar (context,
		    (char) ((inputString[i] != ' ') ? inputString[i] : '+'),
		    *encodedString,
		    &encodedStringLen,
		    &encodedStringSize);
	
    }
    *encodedString = vri_AddChar (context,
	    0,
	    *encodedString,
	    &encodedStringLen,
	    &encodedStringSize);
    return (VRI_E_OK);
}

int
vri_GenerateRegInfo (
	PKICONTEXT	*context,
	int		entityType,
	int		certRequestType,
	vri_ava_t	*ava,
	char		**regInfoString)
{
    vri_reginfo_t *table;
    int i, j;
    int r;
    size_t bytesNeeded;
    size_t regInfoStringLen = 0;
    char *encodedString;
    int (*testchar) (unsigned char);
    unsigned pvsize;
    
    *regInfoString = NULL;

    switch (certRequestType)
    {
	case VRI_CERT_PERSONAL:
	    table = VriPersonal;
	    break;
	case VRI_CERT_IPSEC:
	    table = VriIPSEC;
	    break;
	case VRI_CERT_SECURE_SERVER:
	    table = VriSecureServer;
	    break;
	case VRI_CERT_RA:
	    table = VriRA;
	    break;
	default:
	    return VRI_E_UNKNOWN_TYPE;
    }
    
    /* ensure all mandatory pairs are present */
    for (i = 0; table[i].name; i++)
    {
	for (j = 0; ava[j].type; j++)
	{
	    if (!strcmp (table[i].name, ava[j].type))
		break;
	}
	
	if (!ava[j].type &&
		((table[i].restrictions & VRI_R_MANDATORY) ||
		 ((table[i].restrictions & VRI_R_MANDATORY_EE) && (entityType == VRI_ENTITY_EE)) ||
		 ((table[i].restrictions & VRI_R_MANDATORY_RA) && (entityType == VRI_ENTITY_RA))))
	{
#ifdef TESTING
	    printf ("missing attribute... %s\n", table[i].name);
#endif /* TESTING */
	    return (VRI_E_MISSING_MANDATORY);
	}
    }

    for (i = 0; ava[i].type; i++)
    {
	for (j = 0; table[j].name; j++)
	{
	    if (!strcmp (table[j].name, ava[i].type))
		break;
	}

	if (!table[j].name)
	    return (VRI_E_AVA_TYPE);

	/* make sure the value satisfies the restrictions for the type */

	if ((table[j].restrictions & VRI_R_VAL_END_USER) &&
		strncmp (ava[i].value, "end-user", ava[i].size))
	    return (VRI_E_CERT_TYPE);

	if ((table[j].restrictions & VRI_R_VAL_IPSEC) &&
		strncmp (ava[i].value, "ipsec", ava[i].size))
	    return (VRI_E_CERT_TYPE);

	if ((table[j].restrictions & VRI_R_VAL_SERVER) &&
		strncmp (ava[i].value, "server", ava[i].size))
	    return (VRI_E_CERT_TYPE);

	if ((table[j].restrictions & VRI_R_VAL_YES_OR_NO) &&
		(strncmp (ava[i].value, "yes", ava[i].size)
		 && strncmp (ava[i].value, "no", ava[i].size)))
	    return (VRI_E_EMBED_EMAIL);

	if ((table[j].restrictions & VRI_R_MAX_LEN_32)
	    		&& ava[i].size > 32)
	    return (VRI_E_LENGTH);

	if ((table[j].restrictions & VRI_R_VAL_YES_NO_PENDING) &&
		(strncmp (ava[i].value, "yes", ava[i].size)
		 && strncmp (ava[i].value, "no", ava[i].size)
		 && strncmp (ava[i].value, "pending", ava[i].size)))
	    return (VRI_E_AUTHENTICATE);
	
	testchar = (table[j].restrictions & VRI_R_TYPE_EMAIL) ?
	    		vri_ValidEmailChar : vri_ValidTextChar;
	for (pvsize=0; pvsize<ava[i].size; pvsize++)
	{
	    if (! testchar (ava[i].value[pvsize]))
	    {
#ifdef TESTING
		printf ("invalid char: %c\n", ava[i].value[pvsize])
#endif
		return VRI_E_INVALID_CHAR;
	    }
	}

	r = vri_UrlEncode (context, ava[i].value, ava[i].size, &encodedString);
	if (r != VRI_E_OK)
	    return (r);
	
	bytesNeeded = strlen (ava[i].type) + strlen (encodedString) + 1;
	if (i)
	    bytesNeeded++;
	PKIRealloc (context->memMgr,
		(void **) regInfoString,
		regInfoStringLen + bytesNeeded + 1);
	(*regInfoString)[regInfoStringLen] = 0;

	if (i)
	    strcat (*regInfoString, "&");
	strcat (*regInfoString, ava[i].type);
	strcat (*regInfoString, "=");
	strcat (*regInfoString, encodedString);
	regInfoStringLen += bytesNeeded;
	PKIFree (context->memMgr, encodedString);
    }
    
    return (VRI_E_OK);
}

int vri_GeneratePKCSReq (
	PKICONTEXT		*context,
	const unsigned char	*pkcs10,
	size_t			pkcs10Len,
	const char		*regInfo,
	unsigned char		**pkcsReq,
	size_t			*pkcsReqLen)
{
    int		e = 0;
    PKIPKCSReq	*req = PKINewPKCSReq (context);

    if (regInfo)
    {
	req->regInfo = PKINewOCTET_STRING (context);
	PKIPutStrVal (context, req->regInfo, regInfo);
    }
    PKIPutOctVal (context, &req->endEntityInfo, pkcs10, pkcs10Len);
    *pkcsReqLen = PKISizeofPKCSReq (context, req, 1);
    *pkcsReq = PKIAlloc (context->memMgr, *pkcsReqLen);
    if (PKIPackPKCSReq (context, *pkcsReq, *pkcsReqLen, req, &e) != *pkcsReqLen)
    {
	PKIFree (context->memMgr, *pkcsReq);
	*pkcsReq = NULL;
	*pkcsReqLen = 0;
	PKIFreePKCSReq (context, req);
	return VRI_E_PACK; /* error packing PKCSReq ASN.1 */
    }
    PKIFreePKCSReq (context, req);
    return (VRI_E_OK);
}

/* vim:ts=4
 */
