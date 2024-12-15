/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <string.h>

/* we don't want strings.h for WIN32, but for PGP use we should really check
   PGP_WIN32, so we have the extra check in the #if */
#if !PGP_WIN32 && !defined(WIN32)
#include <strings.h>
#endif

#include "tc.h"
#include "cms.h"

#include "cms_proto.h"

typedef struct OIDentifier {
    int oid_nelem;               /* number of sub-identifiers */
    unsigned long *oid_elements; /* the (ordered) list of sub-identifiers */
} OID_T, *OID_P;

/*****
*  prototypes 
*****/
static int HexStringToUchar(
	unsigned char **der,
	size_t *derlen,
	const char *hexstr,
	TC_MemoryMgr *mgr);

static int HexCharsToUchar(unsigned char *hex, char *string);

static char *AVAToString(
     PKIAttributeTypeAndValue *ava,
     TC_CONTEXT *context);

static int ParseStringRDNSeq(
    PKIRDNSequence *rdnSeq,
    const char *name,
    TC_CONTEXT *context);

static int ParseStringAVA(
	PKIAttributeTypeAndValue *ava,
	const char *name,
	TC_CONTEXT *context);

static int UndelimitString(
        unsigned char **converted, size_t *length, const char *orig,
	TC_MemoryMgr *mgr);

static int CreateAVADERFromString(
        unsigned char *string, 
	size_t strLen,
	TC_AVA_ENTRY *avaInfo,
	unsigned char **der,
	size_t *derLen,
	TC_CONTEXT *context);

/*****
*
* Table to define the default printable tag value, the DER OID, and 
* the value's data type for a distinguised name AVA.  Some values
* for non-string types are currently commented out until we can
* handle them.
*
* Values for printable tags are based on the values in:
*  RFC2256, "A Summary of the X.500(96) User Schema for use with LDAPv3"
*
* Values for DC came from:
*  RFC2247, "Using Domains in LDAP/X.500"
*
* Values for email address, unstructured name, and unstructured address
* came from:
*  PKCS #9 "Selected Attribute Types"
*
* Note: I have seen values for the following in print, but have not yet
* found the OID/type definitions
*	userid
*
*****/
static struct {
  const char *type;	/* the attribute type for a name */
  unsigned char oid[11];
  int oidlen;
  int  tag;	/* the base ASN.1 type (the default character set) */
} Types[] = {

/* common name */
{ "CN", { 0x55, 0x04, 0x03 }, 3, PKIID_PrintableString  },

/* surname */ 
{ "SN",  { 0x55, 0x04, 0x04 }, 3, PKIID_PrintableString },

{ "SerialNumber", { 0x55, 0x04, 0x05 }, 3, PKIID_PrintableString  },

/* country name */
{ "C",	{ 0x55, 0x04, 0x06 }, 3, PKIID_PrintableString  },

/* locality name */
{ "L",	{ 0x55, 0x04, 0x07 }, 3, PKIID_PrintableString },

/* state/province name */
{ "ST", { 0x55, 0x04, 0x08 }, 3, PKIID_PrintableString  },

/* street address */
{ "STREET", { 0x55, 0x04, 0x09 }, 3, PKIID_PrintableString  },

/* organization */
{ "O",  { 0x55, 0x04, 0x0a }, 3, PKIID_PrintableString  },

/* organization unit name */
{ "OU", { 0x55, 0x04, 0x0b }, 3, PKIID_PrintableString  },

{ "title",  { 0x55, 0x04, 0x0c }, 3, PKIID_PrintableString  },

{ "description", { 0x55, 0x04, 0x0d }, 3, PKIID_PrintableString  },

{ "businessCategory", { 55, 0x04, 0x0f }, 3, PKIID_PrintableString  },

/* postal address */
/* SEQ OF { "PA",  { 0x55, 0x04, 0x10 }, 3, ??? }, */

{ "postalCode", { 0x55, 0x04, 0x11 }, 3, PKIID_PrintableString  },

/* postOfficebox */
{ "POBOX", { 0x55, 0x04, 0x12 }, 3, PKIID_PrintableString  },

{ "physicalDeliveryOfficeName", { 0x55, 0x04, 0x13 }, 3, PKIID_PrintableString },

/* telephoneNumber */
{ "TN", { 0x55, 0x04, 0x14 }, 3, PKIID_PrintableString  },

/*  fax number */
/* SEQ { "FAX", { 0x55, 0x04, 0x17 }, 3, ???  }, */

{ "x121Address", { 0x55, 0x04, 0x18 }, 3, PKIID_NumericString  },

/* international ISDN number */
{ "ISDN", { 0x55, 0x04, 0x19 }, 3, PKIID_NumericString  },

{ "destinationIndicator", { 0x55, 0x04, 0x1b }, 3, PKIID_PrintableString  },

/* BIT STRING { "userPassword", { 0x55, 0x04, 0x23 }, 3, ??? },*/

{ "name", { 0x55, 0x04, 0x29 }, 3, PKIID_PrintableString  },

{ "givenName", { 0x55, 0x04, 0x2a }, 3, PKIID_PrintableString  },

{ "initials",  { 0x55, 0x04, 0x2b }, 3, PKIID_PrintableString  },

/* eg., 3rd */
{ "generationQualifier",  { 0x55, 0x04, 0x2c }, 3, PKIID_PrintableString  },

/*{ "x500UniqueIdentifier", { 0x55, 0x04, 0x2d }, 3, PKIID_BIT_STRING },*/

/* distinquished name qualifier */
{ "dnQualifier", { 0x55, 0x04, 0x2e }, 3, PKIID_PrintableString },

{ "houseIdentifier", { 0x55, 0x04, 0x33 }, 3, PKIID_PrintableString  },

/* directory management domain */
{ "dmdName", { 0x55, 0x04, 0x36 }, 3, PKIID_PrintableString  },

/* domain component, 0.9.2342.19200300.100.1.25 */
{ "DC", { 0x09, 0x92, 0x26, 0x89, 0x93, 0xf2, 0x2c, 0x64, 0x01, 0x19 }, 10, PKIID_IA5String },

/* e-mail name from pkcs-9 */
{ "emailAddress",  { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01 }, 9, PKIID_IA5String },

/* unstructued name from pkcs-9 */
{ "unstructuredName", { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x02 }, 9, PKIID_IA5String },

/* unstructed address from pkcs-9 */
{ "unstructuredAddress", { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x08 }, 9, PKIID_PrintableString },

{ NULL, { 0 }, 1, PKIID_NULL }

};

static int ValidASNType(int type)
{
    switch(type) {
    case TC_ASN_BOOLEAN:
    case TC_ASN_INTEGER:
    case TC_ASN_BIT_STRING :
    case TC_ASN_OCTET_STRING :
    case TC_ASN_NULL :
    case TC_ASN_OBJECT_ID :
    case TC_ASN_SEQUENCE :
    case TC_ASN_SET :
    case TC_ASN_NumericString :
    case TC_ASN_PrintableString :
    case TC_ASN_T61String :
    case TC_ASN_VideotexString :
    case TC_ASN_IA5String :
    case TC_ASN_UTCTime :
    case TC_ASN_GeneralizedTime :
    case TC_ASN_GraphicString :
    case TC_ASN_VisibleString :
    case TC_ASN_GeneralString :
    case TC_ASN_CHOICE :
        return 0;

    default:
        return -1;
    }
}


/*****
*
* InitAVAList
*
* Internal routine to set the default Dname AVA values in the
* context.
*
****/
int InitAVAList(TC_AVA_ENTRY **list, TC_MemoryMgr *memMgr)
{
    TC_AVA_ENTRY *entry;
    int i;

    if (list == NULL)
        return TC_E_INVARGS;
    *list = NULL;

    for (i = 0; Types[i].type; i++) {

        entry = TC_Alloc(memMgr, sizeof(TC_AVA_ENTRY));
        if (entry == NULL)
            return TC_E_NOMEMORY;

	entry->attributeType = TC_Alloc(memMgr, strlen(Types[i].type)+1 );
	if (entry->attributeType == NULL) {
	    TC_Free(memMgr, entry);
	    return TC_E_NOMEMORY;
	}
	strcpy(entry->attributeType, Types[i].type);

        entry->oid = TC_Alloc(memMgr, Types[i].oidlen);
        if (entry->oid == NULL) {
            /* TODO free whole list? */
	    TC_Free(memMgr, entry->attributeType);
	    TC_Free(memMgr, entry);
            return TC_E_NOMEMORY;
        }
        memcpy(entry->oid, Types[i].oid, Types[i].oidlen);
        entry->oidlen = Types[i].oidlen;
        entry->asnType = Types[i].tag;
        entry->next = *list;
        *list = entry;
    }

    return 0;

} /* InitAVAList */

void FreeAVAList(TC_AVA_ENTRY *list, TC_MemoryMgr *memMgr)
{
    TC_AVA_ENTRY *entry = list;
    TC_AVA_ENTRY *tmp;

    while (entry != NULL) {
	if (entry->attributeType != NULL)
	    TC_Free(memMgr, entry->attributeType);
	if (entry->oid != NULL)
	    TC_Free(memMgr, entry->oid);
	tmp = entry->next;
	TC_Free(memMgr, entry);
	entry = tmp;
    }

    return;
} /* FreeAVAList */

/*****
*
* tc_change_ava_entry
*
* Allows the user to update the default ASN type for an AVA
* in the context's list.
*
* parameters
*   input
*       oid - the AVA's oid
*       oidlen - length of above
*       newASNType - the new type for the AVA
*       context - pointer to an initialized context
*
* returns
*   TC_E_INVARGS
*   -1 (the provided oid is not in the list)
*   0
*****/
int tc_change_ava_entry(
        unsigned char *oid,
        size_t oidlen,
        int newASNType,
        TC_CONTEXT *context)
{
    TC_AVA_ENTRY *listentry = NULL;

    if (context == NULL || oid == NULL)
        return TC_E_INVARGS;

    if ( ValidASNType(newASNType) != 0)
        return TC_E_INVARGS;

    listentry = context->avaList;
    while(listentry) {

        if ((listentry->oidlen == oidlen) &&
            (memcmp(listentry->oid, oid, oidlen) == 0))
            break;

        listentry = listentry->next;
    }

    if (listentry == NULL)
        return  -1;

    listentry->asnType = newASNType;

    return 0;

} /* tc_change_ava_entry */


/*****
*
* tc_add_avatype
*
* Allows the user to add a new AVA definition to the list in the context.
* Then the user can provide string representations of a Dname with
* that AVA when using tc_make_dname_fromstring.  Also, Dname's containing
* that AVA will print successfully when using tc_extract_dname.
*
* parameters
*   input
*       oid - the oid for the AVA
*       oidlen - length of above
*       printableAttrName - the name to use for printable versions
*                   of this AVA (eg., "CN" or "unstructuredName")
*       asnType - the ASN.1 type for this AVA
*       context - pointer to an initialized context
*
* returns
*   TC_E_INVARGS
*   0
*****/
int tc_add_avatype(
        unsigned char *oid,
        size_t oidlen,
        const char *printableAttrName,
        int asnType,
        TC_CONTEXT *context)
{
    TC_AVA_ENTRY *entry;
    
    if (context == NULL || oid == NULL || oidlen == 0 ||
        printableAttrName == NULL)
        return TC_E_INVARGS;

    if ( ValidASNType(asnType) != 0)
        return TC_E_INVARGS;

    entry = TC_Alloc(context->memMgr, sizeof(TC_AVA_ENTRY));
    if (entry == NULL)
       return TC_E_NOMEMORY;

    entry->attributeType =
	TC_Alloc(context->memMgr, strlen(printableAttrName)+1 );
    if (entry->attributeType == NULL) {
	TC_Free(context->memMgr, entry);
	return TC_E_NOMEMORY;
    }
    strcpy(entry->attributeType, printableAttrName);

    entry->oid = TC_Alloc(context->memMgr, oidlen);
    if (entry->oid == NULL) {
	TC_Free(context->memMgr, entry->attributeType);
	TC_Free(context->memMgr, entry);
        return TC_E_NOMEMORY;
    }
    memcpy(entry->oid, oid, oidlen);
    entry->oidlen = oidlen;
    entry->asnType = asnType;
    entry->next = context->avaList;
    context->avaList = entry;

    return 0;

} /* tc_add_ava */


/*****
*
* Find the AVA entry in the context that has the printable
* name provided
*****/
static TC_AVA_ENTRY *lookupASNtype(
    const char *name,
    TC_CONTEXT *context)
{
  TC_AVA_ENTRY *avaEntry = context->avaList;

    while (avaEntry) {
        if (strcasecmp (name, avaEntry->attributeType) == 0)
            return avaEntry;
        avaEntry = avaEntry->next;
  }

  return NULL;
}

/*****
 *
 * Find an AVA entry in the context give the OID
 *
 *****/
static TC_AVA_ENTRY *lookupAVAByOID(
     PKIAttributeTypeAndValue *ava,
     TC_CONTEXT *context)
{
    TC_AVA_ENTRY *avaEntry = context->avaList;

    while(avaEntry) {
	if (ava->type.len == avaEntry->oidlen &&
	    memcmp(ava->type.val, avaEntry->oid, avaEntry->oidlen) == 0) 
	    return avaEntry;
	avaEntry = avaEntry->next;
    }
    return NULL;
} /* lookupAVAByOID */

/*****
 *
 * DERToOID
 *
 * a routine to convert from DER encoding to a list of integers 
 *
 *****/
static OID_T *DERToOID(
     unsigned char *derOid,
     size_t derOidLen,
     TC_MemoryMgr *mgr)
{
    unsigned long i;
    unsigned long *intElements;
    unsigned char *dataPtr, *endPtr;
    OID_T *oid = NULL;

    dataPtr = derOid;
    endPtr = dataPtr + derOidLen;

    /* Count the number of integer elements are in the OID,
       the first element will never be more than one byte
       so start with second DER value */
    for (i = 1; dataPtr < endPtr; i++) {
        if (*dataPtr == 0x80) /* incorrect format */
            return(oid);

        while (*dataPtr++ & 0x80)
            if (dataPtr > endPtr)
                return(oid);
    }

    oid = TC_Alloc(mgr, sizeof(OID_T));
    if (oid == NULL)
	return NULL;
    intElements = TC_Alloc(mgr, (i+1)*sizeof(unsigned long));
    if (intElements == NULL) {
	TC_Free(mgr, oid);
	return NULL;
    }

    oid->oid_elements = intElements;
    oid->oid_nelem = i;

    for (dataPtr = derOid; dataPtr < endPtr; ) {
        i = 0;
        do {
            i <<= 7;
            i |= *dataPtr & 0x7f;
        } while (*dataPtr++ & 0x80);
        if (intElements == oid->oid_elements) {
            *intElements++ = i / 40;
	    *intElements++ = i % 40;
	}
        else
            *intElements++ = i;
    }

    return oid;

} /* DERToOID */

/*****
 *
 * OidToString
 *
 * Convert a DER (hex) representation of an OID into a dotted
 * decimal string representataion.  It's assumed that the provided
 * string buffer is large enough.
 *
 *****/
static int OidToString(char *string, unsigned char *oid, size_t oidlen,
		       TC_MemoryMgr *mgr)
{
    int i;
    char temp[256];
    OID_T *intOID;

    if (!string)
	return -1;
    string[0] = '\0';

    intOID = DERToOID(oid, oidlen, mgr);
    if (intOID == NULL)
	return -1;

    for (i = 0; i < intOID->oid_nelem; i++) {

	if (i == intOID->oid_nelem-1)
	    sprintf(temp, "%ld", intOID->oid_elements[i]);
	else
	    sprintf(temp, "%ld.", intOID->oid_elements[i]);

	strcat(string, temp);
    }

    if (intOID->oid_elements != NULL)
	TC_Free(mgr, intOID->oid_elements);
    TC_Free(mgr, intOID);

    return 0;

} /* OidToString */

/*****
 *
 * CreateAVADERFromString
 *
 * Given the string value provided by the user and the AVA ASN.1 type it
 * should be, create the DER for the AVA's type field.  This uses the
 * compiler generated code for creating the DER, so that if the provided
 * string does not meet the requirements for the ASN.1 type, an error
 * will be reported.  For instance, if you provide an e-mail address for
 * a PrintableString type it will fail.
 *
 * TODO: a future version of this code should be able to dynamically change
 * the ASN.1 type used if the provided string is not allowed for the default
 * ASN.1 type in avaInfo.  So, following PKIX, if PrintableString doesn't
 * work, next try BMPString and finally use UTF8String.
 *
 * Parameters
 *   input
 *       string - the character data to use in the DER
 *       strLen
 *       avaInfo - contains the ASN.1 type for the AVA
 *       context - 
 *
 *   output
 *       der - filled in with the DER for the type field of the AVA or NULL
 *             if error
 *       derLen
 *
 * return
 *    0
 *    TC_E_INVARGS
 *    TC_E_NOMEMORY
 *    TC_E_BADNUMERICSTRING
 *    TC_E_BADPRINTABLESTRING
 *    TC_E_PARSE
 *    TC_E_BADDNAMESTRINGTYPE
 *****/
static int CreateAVADERFromString(
        unsigned char *string,
	size_t strLen,
	TC_AVA_ENTRY *avaInfo,
	unsigned char **der,
	size_t *derLen,
	TC_CONTEXT *context)
{
    int ret = 0;

    if (der == NULL || derLen == NULL || avaInfo == NULL || string == NULL)
	return TC_E_INVARGS;

    switch (avaInfo->asnType) {

    case PKIID_PrintableString:
	{
	    PKIPrintableString *asnstruct;

	    asnstruct = PKINewPrintableString(context->certasnctx);
	    if (asnstruct == NULL) {
		ret = TC_E_NOMEMORY;
		break;
	    }
	    PKIPutOctVal(context->certasnctx, asnstruct,
			 string, strLen);
	    *derLen = PKISizeofPrintableString(context->certasnctx,
					       asnstruct, 1);
	    *der = TC_Alloc(context->memMgr, *derLen);
	    if (*der == NULL) {
		PKIFreePrintableString(context->certasnctx, asnstruct);
		ret = TC_E_NOMEMORY;
		break;
	    }
	    (void)PKIPackPrintableString(context->certasnctx, *der, *derLen,
					 asnstruct, &ret);
	    if (ret != 0)
		ret = compiler2tc_error(ret);
	    PKIFreePrintableString(context->certasnctx, asnstruct);
	    break;
	}

    case PKIID_NumericString:
	{
	    PKINumericString *asnstruct;

	    asnstruct = PKINewNumericString(context->certasnctx);
	    if (asnstruct == NULL) {
		ret = TC_E_NOMEMORY;
		break;
	    }
	    PKIPutOctVal(context->certasnctx, asnstruct,
			 string, strLen);
	    *derLen = PKISizeofNumericString(context->certasnctx,
					       asnstruct, 1);
	    *der = TC_Alloc(context->memMgr, *derLen);
	    if (*der == NULL) {
                PKIFreeNumericString(context->certasnctx, asnstruct);
		ret = TC_E_NOMEMORY;
		break;
	    }
	    (void)PKIPackNumericString(context->certasnctx, *der, *derLen,
					 asnstruct, &ret);
	    if (ret != 0)
		ret = compiler2tc_error(ret);
	    PKIFreeNumericString(context->certasnctx, asnstruct);
	    break;
	}

    case PKIID_IA5String:
	{
	    PKIIA5String *asnstruct;

	    asnstruct = PKINewIA5String(context->certasnctx);
	    if (asnstruct == NULL) {
		ret = TC_E_NOMEMORY;
		break;
	    }
	    PKIPutOctVal(context->certasnctx, asnstruct,
			 string, strLen);
	    *derLen = PKISizeofIA5String(context->certasnctx,
					       asnstruct, 1);
	    *der = TC_Alloc(context->memMgr, *derLen);
	    if (*der == NULL) {
                PKIFreeIA5String(context->certasnctx, asnstruct);
		ret = TC_E_NOMEMORY;
		break;
	    }
	    (void)PKIPackIA5String(context->certasnctx, *der, *derLen,
					 asnstruct, &ret);
	    if (ret != 0)
		ret = compiler2tc_error(ret);
	    PKIFreeIA5String(context->certasnctx, asnstruct);
	    break;
	}

    case PKIID_T61String:
	{
	    PKIT61String *asnstruct;

	    asnstruct = PKINewT61String(context->certasnctx);
	    if (asnstruct == NULL) {
		ret = TC_E_NOMEMORY;
		break;
	    }
	    PKIPutOctVal(context->certasnctx, asnstruct,
			 string, strLen);
	    *derLen = PKISizeofT61String(context->certasnctx,
					       asnstruct, 1);
	    *der = TC_Alloc(context->memMgr, *derLen);
	    if (*der == NULL) {
                PKIFreeT61String(context->certasnctx, asnstruct);
		ret = TC_E_NOMEMORY;
		break;
	    }
	    (void)PKIPackT61String(context->certasnctx, *der, *derLen,
					 asnstruct, &ret);
	    if (ret != 0)
		ret = compiler2tc_error(ret);
	    PKIFreeT61String(context->certasnctx, asnstruct);
	    break;
	}


    case PKIID_VisibleString:
	{
	    PKIVisibleString *asnstruct;

	    asnstruct = PKINewVisibleString(context->certasnctx);
	    if (asnstruct == NULL) {
		ret = TC_E_NOMEMORY;
		break;
	    }
	    PKIPutOctVal(context->certasnctx, asnstruct,
			 string, strLen);
	    *derLen = PKISizeofVisibleString(context->certasnctx,
					       asnstruct, 1);
	    *der = TC_Alloc(context->memMgr, *derLen);
	    if (*der == NULL) {
                PKIFreeVisibleString(context->certasnctx, asnstruct);
		ret = TC_E_NOMEMORY;
		break;
	    }
	    (void)PKIPackVisibleString(context->certasnctx, *der, *derLen,
					 asnstruct, &ret);
	    if (ret != 0)
		ret = compiler2tc_error(ret);
	    PKIFreeVisibleString(context->certasnctx, asnstruct);
	    break;
	}

    /* the reset are currently not supported */
    default:
	ret = TC_E_BADDNAMESTRINGTYPE;
        break;

    } /* switch */

    if (ret != 0) {
	if (*der != NULL) {
	    TC_Free(context->memMgr, *der);
	    *der = NULL;
	}
	*derLen = 0;
    }

    return ret;

} /* CreateAVADERFromString */


/*****
*
* tc_create_dname
*
* Initialize a Name structure for Dname creation
*
* parameters
*   output
*       name - pointer to a Name pointer
*
* return
*   0 - okay
*   TC_E_INVARGS
*   TC_E_NOMEMORY
*****/
int tc_create_dname(TC_Name **dname, TC_CONTEXT *ctx)
{
    PKIName *localname;

    if (dname == NULL)
        return TC_E_INVARGS;
    *dname = NULL;

    localname = PKINewName(ctx->certasnctx);

    if (localname == NULL)
        return TC_E_NOMEMORY;

    localname->CHOICE_field_type = PKIID_RDNSequence;
    localname->data = TC_Alloc(ctx->memMgr, sizeof (PKIRDNSequence) );
    if (localname->data == NULL) {
        PKIFreeName(ctx->certasnctx, localname);
        return TC_E_NOMEMORY;
    }
    memset(localname->data, 0, sizeof (PKIRDNSequence) );
  
    *dname = localname;
    return 0;

} /* tc_create_dname */

int tc_free_dname(TC_Name *dname, TC_CONTEXT *ctx)
{
    if (dname == NULL)
        return TC_E_INVARGS;

    PKIFreeName(ctx->certasnctx, dname);

    return 0;

} /* tc_free_dname */

#define MEB_BUF_SIZE 1024

/*****
*
* tc_addAVAto_dname
*
* Add an AVA (attribute type and value) entry to a Dname.  Currently
* this only supports one AVA per Relative Distinguished Name.
* The AVA will be added to the end of the current dname's RDN list.
* To add multiple AVA's use tc_make_dname_fromstring().
*
* This assumes that the correct DER value for the AVA is being
* provided (as defined by the attributes OID).  It is the user's
* responsibility to make sure the correct (or allowed) ASN data
* types have been used. (eg., do not use SEQUENCE for country name,
* rather use PrintableString).  If incorrect DER data is provided,
* the DER created for the Dname may not parse.
*
* parameters
*   input
*       oid - the oid of the AVA to add
*       oidlen - length of above
*       der - the attribute's value
*       derlen - length of above
*
*   output
*       dname - the dname list to add the AVA entry to, updated
*           with provided data
*
* returns
*   0 - okay
*   TC_E_NOMEMORY
*   TC_E_INVARGS
*****/
int tc_addAVAto_dname(
        TC_Name *dname,
        unsigned char *oid,
        size_t oidlen,
        unsigned char *der,
        size_t derlen,
	TC_CONTEXT *ctx)
{
    PKIRDNSequence *rdnSeq;
    PKIRelativeDistinguishedName *rdnName;

    if (dname == NULL || oid == NULL || oidlen == 0 ||
        der == NULL || derlen == 0)
        return TC_E_INVARGS;

    rdnSeq = (PKIRDNSequence *)dname->data;

    if (rdnSeq->n == PKIMAX_RDNSequence)
	return TC_E_OTHER;

    rdnSeq->elt[rdnSeq->n] = 
                    TC_Alloc(ctx->memMgr,
			     sizeof(PKIRelativeDistinguishedName));
    if (rdnSeq->elt[rdnSeq->n] == NULL)
	  return TC_E_NOMEMORY;

    rdnName = (PKIRelativeDistinguishedName *)rdnSeq->elt[rdnSeq->n];

/* TODO: fix, assumes only 1 ava */
    rdnName->n = 1; 
    rdnName->elt[0] = TC_Alloc(ctx->memMgr, sizeof(PKIAttributeTypeAndValue) );
    if (rdnName->elt[0] == NULL) {
	TC_Free(ctx->memMgr, rdnSeq->elt[rdnSeq->n]);
	return TC_E_NOMEMORY;
    }
    memset(rdnName->elt[0], 0, sizeof(PKIAttributeTypeAndValue));

    (void)tc_set_avader(rdnName->elt[0],
		  oid,
		  oidlen,
		  der,
		  derlen,
		  ctx);

    rdnSeq->n++;

    return 0;

} /* tc_addAVAto_dname */

static void ReverseAVAOrder(PKIRDNSequence *seq)
{
    int i, j;
    void *temp;

    i = 0;
    j = seq->n - 1;

    for ( ; i < j; i++, j-- ) {

        temp = seq->elt[i];
        seq->elt[i] = seq->elt[j];
        seq->elt[j] = temp;
    }

    return;
}


void TrimWS(char *c)
{
    char *end;

    end = c + strlen(c) - 1;
    while (isspace((int) (*end))) {
	/* check to see if there is a delimited space at the end, don't
	   trim in this case and we're done */
	if (*(end-1) == '\\')
	    return;
	*end = '\0';
	end--;
    }
    return;
}

/*****
* tc_make_dname_fromstring
*
* Given a string, create a DNAME held in a list of entries.
* The provided string basically follows the syntax defined in
* RFC 2253.  The implementation does NOT support the quoted
* string syntax described in the RFC.
*
* examples:
*     OU=Test Certificate Authority , O=NETA\, Inc. , L=Internet
*     1.2.3.4=#0306020133020144, OU=CA+OU=BE, CN=John Doe
*
* The created list will be in the reverse order of the provided string.
* This assumes folks are used to the leaf->root, left->right
* ordering common with written LDAP dnames and DNS.
*
* If the value provided for the attribute type is a string, then the
* attribute OID and ASN data type used for the AVA in the dname
* will be based on the values in the provided context.  See
* tc_change_ava_entry and tc_add_ava for details on modifying the
* default list.  
*
* If the value provided for the attribute type is
* a dotted decimal OID, then it is assmed the data after the "="
* is "#" followed by a hex representation of valid DER data.
* Note that if the data is not DER, then the created Dname's DER
* will probably not successfully parse.
*
* If there is a failure in any particular RDN portion, the processing
* will stop.  All the RDN portions before the problem will be left in
* the dname object.  You must call tc_free_dname() to clean up the dname
* object.
*
* The call also allows one to call it multiple times with the same dname
* object. The string values will be entered into the dname object (in the
* order given) at the end, then the whole list will be reversed.  Not
* a very nice effect...  Maybe this can be changed so this call be use used
* with individual RDNs as the provided string???
*
* parameters
*   input
*	name - the original string name
*       context - a pointer to a valid context
*
*   output
*	dname - pointer to Name structure (created with tc_create_dname)
*               filled in
*
* return
*     0 - okay
*     TC_E_DNAMEPARSE
*     TC_E_NAMETOOLONG
*
*****/
int tc_make_dname_fromstring (
        TC_Name *dname, 
        const char *name,
        TC_CONTEXT *context)
{
    PKIRDNSequence *rdnSeq;
    int status;

    if (!dname || !name || !context)
	return TC_E_INVARGS;

    /* TODO: can probably get rid of this, tc_create_cert was
       changed and doesn't call this anymore... */
    /* If this wasn't a Name object created with tc_create_dname
     then we need to initialize it.  This happens, for example,
     internal to tc_create_cert. */
    if (dname->data == NULL) {
	dname->CHOICE_field_type = PKIID_RDNSequence;
	dname->data = TC_Alloc(context->memMgr, sizeof(PKIRDNSequence) );
	if (dname->data == NULL)
	    return TC_E_NOMEMORY;
	memset(dname->data, 0, sizeof(PKIRDNSequence) );
    }
    rdnSeq = (PKIRDNSequence *)dname->data;

    /* no error, just no name */
    if (name[0] == '\0')
	return 0;

    if ((status = ParseStringRDNSeq(rdnSeq, name, context)) != 0) {
	PKIDropInPlaceRDNSequence(context->certasnctx, rdnSeq);
	rdnSeq->n = 0;
	return status;
    }

    /* the list is created in reverse order of provided names */
    ReverseAVAOrder(rdnSeq);

    return 0;

} /* tc_make_dname_fromstring */
    
static int ParseStringRDNSeq(
    PKIRDNSequence *rdnSeq,
    const char *name,
    TC_CONTEXT *context)
{
    PKIRelativeDistinguishedName *rdnName;
    char *namePos, *rdnStart, *comma;
    char *buf;
    int status;

    if (!rdnSeq)
	return TC_E_INVARGS;

    /* see if the structure is already full */
    if (rdnSeq->n == PKIMAX_RDNSequence) {
	return TC_E_NAMETOOLONG;
    }

    buf = TC_Alloc(context->memMgr, strlen(name)+1);
    if (buf == NULL)
	return TC_E_NOMEMORY;
    strcpy(buf, name);
    namePos = buf;

    while(namePos != '\0' && rdnSeq->n < PKIMAX_RDNSequence) {

	SKIPWS(namePos);
	rdnStart = namePos;

	for(;;) {
	    comma = strchr(namePos, ',');

	    /* last RDN in string */
	    if (comma == NULL) {
		TrimWS(rdnStart);
		namePos = '\0';
		break;
	    } 

	    /* a delimited comma  */
	    else if ( *(comma-1) == '\\' ) {
                namePos = comma + 1;
                continue;
            }

            else {
                namePos = comma + 1;
		*comma = '\0';
                TrimWS(rdnStart);
                break;
            }
        } /* for(;;) */

        /* we have an RDN */
	rdnSeq->elt[rdnSeq->n] = 
	    TC_Alloc(context->memMgr, sizeof(PKIRelativeDistinguishedName));
        if (rdnSeq->elt[rdnSeq->n] == NULL) {
            /* rest of structure freed in top level routine */
	    TC_Free(context->memMgr, buf);
	    return TC_E_NOMEMORY;
        }
	memset(rdnSeq->elt[rdnSeq->n], 0,
	       sizeof(PKIRelativeDistinguishedName));
        rdnName = (PKIRelativeDistinguishedName *)rdnSeq->elt[rdnSeq->n];

        if ((status = ParseStringRDN(rdnName, rdnStart, context)) != 0) {
	    /* on error free the one we just allocated, rest
	       of structure freed in top level routine */
	    PKIFreeRelativeDistinguishedName(context->certasnctx,
					     rdnSeq->elt[rdnSeq->n]);
	    rdnSeq->elt[rdnSeq->n] = NULL;
	    TC_Free(context->memMgr, buf);
            return status;
	}

        rdnSeq->n++;
    } /* while rdnNames */

    if (rdnSeq->n >= PKIMAX_RDNSequence && namePos != '\0') {
	/* structure free'd at top level routine */
	TC_Free(context->memMgr, buf);
	return TC_E_NAMETOOLONG;
    }

    TC_Free(context->memMgr, buf);
    return 0;

} /* ParseStringRDNSeq */


int ParseStringRDN(
    PKIRelativeDistinguishedName *rdn,
    const char *name,
    TC_CONTEXT *context)
{
    PKIAttributeTypeAndValue *ava;
    char *rdnPos, *avaStart, *plus;
    char *buf;
    int status;

    if (!rdn)
	return TC_E_INVARGS;

    /* see if the structure is already full */
    if (rdn->n == PKIMAX_RelativeDistinguishedName) {
	return TC_E_NAMETOOLONG;
    }

    buf = TC_Alloc(context->memMgr, strlen(name)+1);
    if (buf == NULL)
	return TC_E_NOMEMORY;
    strcpy(buf, name);
    rdnPos = buf;

    while(rdnPos != '\0' && rdn->n < PKIMAX_RelativeDistinguishedName) {

	SKIPWS(rdnPos);
	avaStart = rdnPos;

	while(1) {
	    plus = strchr(rdnPos, '+');

	    if (plus == NULL) { /* last ava in string */
		TrimWS(avaStart);
		rdnPos = '\0';
		break;
	    }

	    else if (*(plus-1) == '\\') { /* delimited  */
                rdnPos = plus + 1;
                continue;
            }

            else {
                rdnPos = plus + 1;
		*plus = '\0';
                TrimWS(avaStart);
                break;
            }
        } /* while(0) */

        /* we have an AVA */
	rdn->elt[rdn->n] = TC_Alloc(context->memMgr,
				    sizeof(PKIAttributeTypeAndValue));
        if (rdn->elt[rdn->n] == NULL) {
	    TC_Free(context->memMgr, buf);
	    return TC_E_NOMEMORY;
        }
	memset(rdn->elt[rdn->n], 0, sizeof(PKIAttributeTypeAndValue));
	ava = (PKIAttributeTypeAndValue *)rdn->elt[rdn->n];

        if ((status = ParseStringAVA(ava, avaStart, context)) != 0) {
	    /* free the one we just allocated, rest of structure is freed
	       in top level routine */
	    PKIFreeAttributeTypeAndValue(context->certasnctx, rdn->elt[rdn->n]);
	    rdn->elt[rdn->n] = NULL;
	    TC_Free(context->memMgr, buf);
            return status;
	}

        rdn->n++;
    } /* while avas */

    if (rdn->n >= PKIMAX_RelativeDistinguishedName && rdnPos != '\0') {
	/* structure free'd at top level routine */
	TC_Free(context->memMgr, buf);
	return TC_E_NAMETOOLONG;
    }
    TC_Free(context->memMgr, buf);
    return 0;

} /* ParseStringRDN */

static int ParseStringAVA(
	PKIAttributeTypeAndValue *ava,
	const char *name,
	TC_CONTEXT *context)
{
    char *oidStr, *valStr, *equal, *dot;
    unsigned char *hexOID, *der;
    size_t hexOIDLen, derLen;
    TC_AVA_ENTRY *avaInfo;
    unsigned char *undelimitedStr;
    size_t length;
    char *buf;
    int status = 0;

    if (!ava)
        return TC_E_INVARGS;

    buf = TC_Alloc(context->memMgr, strlen(name)+1);
    if (buf == NULL)
	return TC_E_NOMEMORY;
    strcpy(buf, name);

    equal = strchr(buf, '=');
    if (equal == NULL) {
	TC_Free(context->memMgr, buf);
	return TC_E_DNAMEPARSE;
    }

    oidStr = buf;
    valStr = equal + 1;

    *equal = '\0';
    SKIPWS(oidStr);
    SKIPWS(valStr);
    TrimWS(oidStr);
    TrimWS(valStr);

    dot = strchr(oidStr, '.');
    if (dot != NULL) { /* its a dotted decimal OID */
	status = tc_create_oid(&hexOID, &hexOIDLen, oidStr, context);
	if (status != 0 || hexOID == NULL) {
	    TC_Free(context->memMgr, buf);
	    return TC_E_DNAMEPARSE;
	}
        if (valStr[0] != '#') {
	    TC_Free(context->memMgr, hexOID);
	    TC_Free(context->memMgr, buf);
	    return TC_E_DNAMEPARSE;
	}
	status = HexStringToUchar(&der, &derLen, valStr+1, context->memMgr);
	if (status != 0 || der == NULL) {
	    TC_Free(context->memMgr, hexOID);
	    TC_Free(context->memMgr, buf);
	    return status;
	}

	(void)tc_set_avader(ava,
		  hexOID,
		  hexOIDLen,
		  der,
		  derLen,
		  context);
	TC_Free(context->memMgr, hexOID);
	TC_Free(context->memMgr, der);
    }

    else { /* it must be an oid name in the context list */

	avaInfo = lookupASNtype(oidStr, context);
	if (avaInfo == NULL) {
	    TC_Free(context->memMgr, buf);
	    return TC_E_DNAMEPARSE;
	}

	if (UndelimitString(&undelimitedStr, &length,
			    valStr, context->memMgr) != 0) {
	    TC_Free(context->memMgr, buf);
	    return TC_E_DNAMEPARSE;
	}

	/* This call will return an error if the provided string value
	   is not correct for the ASN.1 type */
	/* TODO: idea, add a flag in context table that says whether
           ava value's ASN.1 type can be changed as needed, especially
	   if the type is DirectoryString. */
	status = CreateAVADERFromString(undelimitedStr, length, avaInfo,
					&der, &derLen, context);
	if (status != 0) {
	    TC_Free(context->memMgr, buf);
	    TC_Free(context->memMgr, undelimitedStr);
	    return status;
	}

	(void)tc_set_avader (ava,
		  avaInfo->oid,
		  avaInfo->oidlen,
		  der,
		  derLen,
		  context);

	TC_Free(context->memMgr, undelimitedStr);
	TC_Free(context->memMgr, der);

    }

    TC_Free(context->memMgr, buf);
    return 0;
} /* ParseStringAVA */

static int
UndelimitString(
     unsigned char **converted, size_t *length, const char *orig,
     TC_MemoryMgr *mgr)
{
    char *p = (char *)orig;
    unsigned char *new;
    size_t newlen = 0;

    if (!converted)
        return TC_E_INVARGS;
    *converted = NULL;

    new = TC_Alloc(mgr, strlen(orig));
    if (new == NULL)
        return TC_E_NOMEMORY;
    *converted = new;

    while (*p != '\0') {

	if (*p == '\\') {
	    p++;
	    switch(*p) {
		case '"':
		case '=':
		case ',':
		case '+':
		case '\\':
		case '<':
		case '>':
		case ';':
		case '#':
		case ' ':
			*new = *p;
			new++; p++;
			newlen++;
			break;

	        case '\0':
		        TC_Free(mgr, *converted);
			*converted = NULL;
			return TC_E_DNAMEPARSE;
			break;

		default:
		        if (HexCharsToUchar(new, p) != 0) {
			    TC_Free(mgr, *converted);
			    *converted = NULL;
			    return TC_E_DNAMEPARSE;
			}
			p += 2;
			new ++;
			newlen++;
			break;
	    } /* switch */
	} /* if delimiter */

	else {
	    *new = *p;
	    new++; p++;
	    newlen++;
	}

    } /* while chars */

    *length = newlen;
    return 0;
} /* UndelimitString */

static int HexCharsToUchar(
    unsigned char *hex, char *string)
{
    char buf[2];
    unsigned char c1 = 0, c2 = 0;

    if (!hex || !string)
        return TC_E_INVARGS;

    strncpy(buf, string, 2);

    buf[0] = toupper(buf[0]);

    if ( (buf[0] > 0x46) || (buf[0] < 0x30) ||
         ((buf[0] > 0x39) && (buf[0] < 0x41)) )
            return TC_E_DNAMEPARSE; /* Outside hex range */

    c1 = buf[0] - 0x30;
    if (buf[0] > 0x39)
	c1 = c1-7;

    buf[1] = toupper(buf[1]);

    if ( (buf[1] > 0x46) || (buf[1] < 0x30) ||
         ((buf[1] > 0x39) && (buf[1] < 0x41)) )
            return TC_E_DNAMEPARSE; /* Outside hex range */

    c2 = buf[1] - 0x30;
    if (buf[1] > 0x39)
	c2 = c2 - 7;

    c1 = (c1 << 4) | (0x00ff & c2);

    *hex = c1;
    return 0;

} /* HexCharsToUchar */

/*****
 *
 * HexStringToUchar
 *
 * Convert a NULL terminated string version of hexidecimal characters into
 * an unsigned char array of the actual hex characters.  Every
 * pair of hex string characters is converted into one unsigned char.
 * If the provided string has an odd number of characters, then the
 * string is prepended with "0" before conversion.
 *
 *****/
static int HexStringToUchar(
	unsigned char **der,
	size_t *derlen,
	const char *hexstr,
	TC_MemoryMgr *mgr)
{
    size_t stringLen = 0;
    char *localstring;
    size_t localstringlen;
    unsigned char *localder;
    size_t localderlen;
    unsigned char *ptr;
    unsigned int i;
    int status;

    if (!der || !hexstr || !derlen)
	return TC_E_INVARGS;
    *der = NULL;
    *derlen = 0;

    stringLen = strlen(hexstr);
    if (stringLen == 0) /* don't have chars to convert */
	return 0;

    /* see if its odd or even length */
    if ( (stringLen%2) == 0) {
	localderlen = stringLen/2;
	localstringlen = stringLen;
	localstring = TC_Alloc(mgr, stringLen + 1);
	if (localstring == NULL)
	    return TC_E_NOMEMORY;
	strcpy(localstring, (char *)hexstr);
    }
    else {
	localderlen = stringLen/2 + 1;
	localstringlen = stringLen + 1;
        /* allocate a local copy of string prepended with "0" */
        localstring = TC_Alloc(mgr,
		     stringLen + 2); /* 1 for prepended 0 and 1 for '\0' */
        if (localstring == NULL)
	    return TC_E_NOMEMORY;
	strcpy(localstring, "0");
	strcat(localstring, hexstr);
    }

    /* allocate local work space */
    localder = TC_Alloc(mgr, localderlen);
    if (localder == NULL) {
	TC_Free(mgr, localstring);
        return TC_E_NOMEMORY;
    }

    ptr = localder;
    for (i = 0; i < localstringlen; i += 2) {

	if ((status = HexCharsToUchar(ptr, localstring+i)) != 0) {
	    TC_Free(mgr, localstring);
	    TC_Free(mgr, localder);
	    return status;
	}
	ptr++;
    }

    *der = localder;
    *derlen = localderlen;

    TC_Free(mgr, localstring);

    return 0;

} /* HexStringToUchar */

/*****
 *
 * UcharToHexString
 *
 *****/
static char *UcharToHexString(
    unsigned char *buf,
    size_t buflen,
    TC_MemoryMgr *mgr)
{
    char *hexString;
    size_t hexStringLen;
    char temp[4];
    unsigned int i;

    /* two hex chars for each byte + '\0' */
    hexStringLen = buflen*2 +1;

    hexString = TC_Alloc(mgr, hexStringLen);
    if (hexString == NULL)
	return NULL;

    hexString[0] = '\0';

    for (i = 0; i < buflen; i++) {

	sprintf(temp, "%02x", buf[i]);
	strcat(hexString, temp);
    }

    return hexString;

} /* UcharToHexString */


/****
 *
 * DelimitString
 *
 * Add backslashes to delimit characters in text strings based
 * on the syntax defined in RFC 2253
 *
 *****/
static char *DelimitString(
     unsigned char *buf,
     size_t buflen,
     TC_MemoryMgr *mgr)
{
    char *delimString;
    char *delimStringPtr;
    unsigned int i,j;
    size_t delimStringLen = buflen;
    size_t numLeadingSpaces = 0;
    size_t numTrailingSpaces = 0;

    if (buflen == 0) {
	delimString = TC_Alloc(mgr, 1);
	if (delimString == NULL)
	    return NULL;
	*delimString = '\0';
	return delimString;
    }

    /*----- calc the additional size of the delimiters in the string ----*/
    /* We need to delimit leading and ending space characters, but not
       any other spaces */
    i = 0;
    while (buf[i] == ' ') {
	numLeadingSpaces++;
	i++;
    }

    /* check to see if the string is all spaces, if not then see if there
       are any trailing spaces */
    if (numLeadingSpaces != buflen) {
        i = buflen - 1;
        while (buf[i] == ' ' && (int)i >= 0) {
	    numTrailingSpaces++;
	    i--;
	}
    }
    delimStringLen = delimStringLen + numLeadingSpaces + numTrailingSpaces;

    for (i = 0; i < buflen; i++) {
	switch(buf[i]) {
	case '"':
	case '=':
	case ',':
	case '+':
	case '\\':
        case '<':
        case '>':
	case ';':
	case '#':
	    delimStringLen++;
	    break;
	default:
	    break;
	}
    } /* for */

    /* "+ 1" for null char at end */   
    delimStringLen++;

    delimString = TC_Alloc(mgr, delimStringLen);
    if (delimString == NULL)
	return NULL;
    delimStringPtr = delimString;

    /*----- now create actual delimited string ----*/

    /* delimit leading spaces */
    j = 0;
    while (buf[j] == ' ' && j < buflen) {
	*delimStringPtr = '\\';
	delimStringPtr++;
	*delimStringPtr = ' ';
	delimStringPtr++;
	j++;
    }

    for (i = j; i < buflen - numTrailingSpaces; i++) {
	switch(buf[i]) {
	case '"':
	case '=':
	case ',':
	case '+':
	case '\\':
        case '<':
        case '>':
	case ';':
	case '#':
	    *delimStringPtr = '\\';
	    delimStringPtr++;
	    *delimStringPtr = buf[i];
	    delimStringPtr++;
	    break;
	default:
	    *delimStringPtr = buf[i];
	    delimStringPtr++;
	    break;
	}
    } /* for */

    /* now do the trailing spaces */
    if (numTrailingSpaces != 0) {
	/* we'd better be at the trailing spaces in buf */
	if (buf[i] != ' ' || (buflen-i) != numTrailingSpaces) {
	    TC_Free(mgr, delimString);
	    return NULL;
	}
	while(buf[i] == ' ' && i < buflen) {
	    *delimStringPtr = '\\';
	    delimStringPtr++;
	    *delimStringPtr = ' ';
	    delimStringPtr++;
	    i++;
	}
    }
    

    *delimStringPtr = '\0';

    return delimString;
    
} /* DelimitString */


int CopyName(TC_Name **to, TC_Name *from, TC_CONTEXT *ctx)
{
    int status;
    int i, j;
    PKIName *localTo;
    PKIRDNSequence *to_rdnSeq, *from_rdnSeq;
    PKIRelativeDistinguishedName *to_rdnName, *from_rdnName;

    do {

        if (!to || !from) {
            status = TC_E_INVARGS;
            break;
        }

        if ((status = tc_create_dname(&localTo, ctx)) != 0)
            break;

        to_rdnSeq = (PKIRDNSequence *)localTo->data;
        from_rdnSeq = (PKIRDNSequence *)from->data;

        for ( i = 0; i < from_rdnSeq->n; i++) {

            to_rdnSeq->elt[i] = 
                    TC_Alloc(ctx->memMgr,
			     sizeof(PKIRelativeDistinguishedName));
            if (to_rdnSeq->elt[i] == NULL)
	        return TC_E_NOMEMORY;

            to_rdnName = (PKIRelativeDistinguishedName *)to_rdnSeq->elt[i];
            from_rdnName = (PKIRelativeDistinguishedName *)from_rdnSeq->elt[i];

	    for (j = 0; j < from_rdnName->n; j++) {
		to_rdnName->elt[j] =
		    TC_Alloc(ctx->memMgr, sizeof(PKIAttributeTypeAndValue));
		if (to_rdnName->elt[j] == NULL) {
		    TC_Free(ctx->memMgr, to_rdnSeq->elt[i]);
		    /* TODO: free rest of list... */
		    return TC_E_NOMEMORY;
		}
		memset(to_rdnName->elt[j], 0,
		       sizeof(PKIAttributeTypeAndValue));

		(void)tc_set_avader(to_rdnName->elt[j],
				    from_rdnName->elt[j]->type.val,
				    from_rdnName->elt[j]->type.len,
				    from_rdnName->elt[j]->value.val,
				    from_rdnName->elt[j]->value.len,
				    ctx);
	    } /* for each ava in the rdn */

	    to_rdnName->n = from_rdnName->n;

        } /* for each rdn in the sequence */

        to_rdnSeq->n = from_rdnSeq->n;

    } while (0);

    *to = localTo;
    return 0;
} /* CopyName */


/*****
*
* tc_get_dname
*
* Given a certificate or cert. request, return the pointer to the
* Name structure containing the requested Name.
*
* parameters
*   input
*       cert - a pointer to a CERT or a CertificateRequest
*       type - the desired name, one of:
*               TC_EXTRACT_ISSUER
*               TC_EXTRACT_CRLISSUER
*               TC_EXTRACT_SUBJECT
*               TC_EXTRACT_REQ_NAME
*
*   output
*       dname - a pointer to the Name requested or NULL on error
*
* returns
*   0 - okay
*   TC_E_???
*
*****/
int
tc_get_dname(
    TC_Name **dname,
    void *cert,
    int type,
    TC_CONTEXT *ctx)
{
    int status = 0;

    if (!cert || !dname)
	return TC_E_INVARGS;

    switch (type) {

    case TC_EXTRACT_ISSUER:
	status = CopyName(dname,
			  &((TC_CERT *) cert)->tbsCertificate->issuer,
			  ctx);
	break;
    case TC_EXTRACT_CRLISSUER:
	status = CopyName(dname,
			  &((TC_CertificateList *) cert)->tbsCertList.issuer,
			  ctx);
	break;
    case TC_EXTRACT_SUBJECT:
	status = CopyName(dname,
			  &((TC_CERT *) cert)->tbsCertificate->subject,
			  ctx);
	break;
    case TC_EXTRACT_REQ_NAME:
	status = CopyName(dname,
			  &((TC_CertificationRequest *) cert)->certificationRequestInfo.subject,
			  ctx);
	break;
    default:
        *dname = NULL;
	return TC_E_INVARGS;
    }

    return status;
} /* tc_get_dname */


/*****
 *
 * AVAToString
 *
 * Create a string representatio of an AVA. This follows the syntax
 * suggested in RFC 2253.
 *
 *****/
static char *AVAToString(
     PKIAttributeTypeAndValue *ava,
     TC_CONTEXT *context)
{
    TC_AVA_ENTRY *avaEntry = NULL;
    char *hexString;
    char *valString;
    char attrName[1024];
    unsigned char tag;
    size_t avaStringLen;
    char *avaString;
    unsigned char *avaData;
    size_t avaDataLen;
    size_t attrValLen;
    size_t attrNameLen;

    avaEntry = lookupAVAByOID(ava, context);

    /* get the string representation of the attribute type */
    if (avaEntry != NULL)
	strcpy(attrName, avaEntry->attributeType);
    else
	OidToString(attrName, ava->type.val, ava->type.len,
		    context->memMgr);
    attrNameLen = strlen(attrName);
    /* error check here... */

    (void)tc_get_ava(&tag, &avaData, &avaDataLen, &ava->value);

    /* if we have a non-printable value for the AVA, then
       convert the data into a hex string representation */
    if (avaEntry == NULL ||
	(tag != PKIID_NumericString &&
	 tag != PKIID_PrintableString &&
	 tag != PKIID_IA5String )) {
	hexString = UcharToHexString(avaData, avaDataLen, context->memMgr);
	if (hexString == NULL)
	    /* free stuff??? */
	    return NULL;
	valString = TC_Alloc(context->memMgr,
			  strlen(hexString) + 2); /* 2 is for "#" and '\0' */
	if (valString == NULL)
	    /* free stuff?? */
	    return NULL;
	strcpy(valString, "#");
	strcat(valString, hexString);
	TC_Free(context->memMgr, hexString);
    }
    /* otherwise use the value as a string */
    else {
	valString = DelimitString(avaData, avaDataLen, context->memMgr);
	if (valString == NULL)
	    /* free stuff?? */
	    return NULL;
    }
    attrValLen = strlen(valString);

    /* name + "=" + value + '\0' */
    avaStringLen = attrNameLen + 1 + attrValLen + 1;
    avaString = TC_Alloc(context->memMgr, avaStringLen);
    if (avaString == NULL)
	/* free stuff?? */
	return NULL;

    strcpy(avaString, attrName);
    strcat(avaString, "=");
    strcat(avaString, valString);

    TC_Free(context->memMgr, valString);

    return avaString;
} /* AVAToString */

/*****
 *
 * RDNToString
 *
 * Create a string representation of a Relative Distinguished Name.
 *
 *****/
char *RDNToString(
     PKIRelativeDistinguishedName *rdnName,
     TC_CONTEXT *context)
{
    int i;
    int newlen;
    PKIAttributeTypeAndValue *ava;
    char *rdnString = NULL;
    char *avaString;

    if (!rdnName)
	return NULL;

    if ((rdnString = TC_Alloc(context->memMgr, 1)) == NULL)
	return NULL;
    *rdnString = '\0';

    /* go through each AVA in the RDN name */
    for (i = 0; i < rdnName->n; i++) {

	ava = rdnName->elt[i];

	avaString = AVAToString(ava, context);
	if (avaString == NULL) {
	    TC_Free(context->memMgr, rdnString);
	    return NULL;
	}

	/* the "2" is for the '\0' and plus chars */
	newlen = strlen(rdnString) + strlen(avaString) + 2;

	TC_Realloc(context->memMgr, (void **)&rdnString, newlen);
	if (rdnString == NULL)
	    return NULL;

	strcat(rdnString, avaString);
        if (i != rdnName->n-1)
	    strcat(rdnString, "+");

	TC_Free(context->memMgr, avaString);

    } /* for each AVA */

    return rdnString;

} /* RDNToString */


/****
* tc_extractdname_fromcert
*
* Extract a subject or issuer dname from a certificate struct 
* or cert. request struct and return the string representation.
*
* returns
*   TC_E_INVARGS
*   TC_E_NOMEMORY
*   0 - okay
*/
int tc_extractdname_fromcert(
    char **dname, 
    void *cert, 
    int type,
    TC_CONTEXT *context)
{
    TC_Name *name;

    if (!dname)
	return TC_E_INVARGS;
    *dname = NULL;

    switch (type) {

    case TC_EXTRACT_ISSUER:
	name = &((TC_CERT *) cert)->tbsCertificate->issuer;
	break;
    case TC_EXTRACT_SUBJECT:
	name = &((TC_CERT *) cert)->tbsCertificate->subject;
	break;
    case TC_EXTRACT_REQ_NAME:
	name = &((TC_CertificationRequest *) cert)->certificationRequestInfo.subject;
	break;
    default:
	return TC_E_INVARGS;
    }

    return tc_extract_dname_string(dname, name, context);

} /* tc_extractdname_fromcert */

/*****
* tc_extract_dname_string
*
* Extract string representation of a dname from a Name structure.
* Follows the string format described in RFC 2253.
*
* returns
*   TC_E_INVARGS
*   TC_E_NOMEMORY
*   0 - okay
*/
int tc_extract_dname_string (
    char **stringname, 
    TC_Name *dname, 
    TC_CONTEXT *context)
{
    int i;
    int newlen;
    PKIRDNSequence *rdnSeq;
    PKIRelativeDistinguishedName *rdnName;
    char *rdnString;
    int error = 0;
    char *localdname;

    if (!stringname || !dname || !context)
	return TC_E_INVARGS;
    *stringname = NULL;

    rdnSeq = (PKIRDNSequence *)dname->data;

    localdname = TC_Alloc(context->memMgr, 1);
    if (localdname == NULL)
	return TC_E_NOMEMORY;
    *localdname = '\0';

    if (rdnSeq->n == 0) {
	*stringname = localdname;
	return 0;
    }

    /* go through each RDN in in rdnSeq, it is in reverse order because
       we want to create the LDAP convention for a string
       dname, leaf->root, left->right */
    for (i = rdnSeq->n-1; i >= 0; i--) {

	rdnName = rdnSeq->elt[i];

	rdnString = RDNToString(rdnName, context);
	if (rdnString == NULL) {
	    error = TC_E_DNAMEPARSE;
	    break;
	}

	/* the "2" is for '\0' and comma */
	newlen = strlen(localdname) + strlen(rdnString) + 2;
	TC_Realloc(context->memMgr, (void **)&localdname, newlen);
	if (localdname == NULL) {
	    error = TC_E_NOMEMORY;
	    break;
	}

	strcat(localdname, rdnString);
	if (i != 0)
	    strcat(localdname, ",");

	TC_Free(context->memMgr, rdnString);

    } /* for each rdnSeq */

    if (error != 0)
	TC_Free(context->memMgr, localdname);
    else
	*stringname = localdname;

    return error;

} /* tc_extract_dname_string */

/*****
 *
 * Given an attribute type (ASCII representation) and a dname, return the
 * value of the first AVA in the dname with that attribute type.  If there is
 * no attribute or the attribute is not a printable type (NumericString,
 * PrintableString, or IA5String), then return NULL.
 * The value returned is not delimited or changed in any way.
 *
 ****/
int
tc_get_attributeValue(
    char **value,
    TC_Name *dname,
    char *attributeType,
    TC_CONTEXT *context)
{
    TC_AVA_ENTRY *avaEntry = NULL;
    PKIRDNSequence *rdnSeq;
    PKIRelativeDistinguishedName *rdnName;
    PKIAttributeTypeAndValue *ava;
    unsigned char tag;
    unsigned char *avaData;
    size_t avaDataLen;
    int i, j;

    if (value == NULL || dname == NULL || attributeType == NULL || context == NULL)
	return TC_E_INVARGS;
    *value = NULL;

    rdnSeq = (PKIRDNSequence *)dname->data;
    for(i = 0; i < rdnSeq->n; i++ ) { 

	rdnName = rdnSeq->elt[i];
	for (j = 0; j < rdnName->n; j++ ) { 

	    ava = rdnName->elt[j];

	    avaEntry = lookupAVAByOID(ava, context);
	    if (avaEntry == NULL)
		break;

	    if (strcmp(attributeType, avaEntry->attributeType) != 0)
		break;

	    (void)tc_get_ava(&tag, &avaData, &avaDataLen, &ava->value);

	    /* if we have a non-printable value for the AVA, then skip it */
	    if (tag != PKIID_NumericString &&
		tag != PKIID_PrintableString &&
		tag != PKIID_IA5String)
		break;

	    /* "+1" for teminating null char */
	    *value = (char *)malloc(avaDataLen + 1);
	    if (*value == NULL)
		return TC_E_NOMEMORY;
	    memcpy(*value, avaData, avaDataLen);
	    memset(*value+avaDataLen, '\0', 1);

	    return 0;
	    
	} /* for each RDN */
    } /* for each RDNSeq */ 

    return 0;

} /* attributevalue */
