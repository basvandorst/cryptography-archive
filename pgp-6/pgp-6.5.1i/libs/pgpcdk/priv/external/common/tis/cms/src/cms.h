/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#ifndef _cms_h_
#define _cms_h_

#include "tc.h"
#include "debug.h"

/************************************
 * Internal Defines
 ************************************/
#define TC_TAG_NULL 0x05
#define TC_TAG_OBJECT_IDENTIFIER 0x06
#define TC_TAG_IA5STRING 0x16

#define TC_MAX_NAME 29

/************************************
 * Internal Structures
 ************************************/

struct tc_ava_entry_t {
    char *attributeType;
    unsigned char *oid;
    size_t oidlen;
    int asnType;
    struct tc_ava_entry_t *next;
};

/************************************
 * Internal Library Functions
 ************************************/

int tc_process_extensions (PKIExtensions *, 
			   void *cert,
			   TC_CONTEXT *);

int CopyExtList(
    TC_ExtensionList **newlist,
    TC_ExtensionList *oldlist,
    TC_CONTEXT *ctx);

int InitAVAList(TC_AVA_ENTRY **list, TC_MemoryMgr *);

void FreeAVAList(TC_AVA_ENTRY *list, TC_MemoryMgr *);

int tc_get_ava (unsigned char *, 
                unsigned char **, 
                size_t *, 
                PKIAttributeValue *);

int tc_set_ava (PKIAttributeTypeAndValue *,	/* structure to fill */
		const unsigned char *,		/* OID of type */
		size_t,				/* len of OID */
		unsigned char,			/* ASN.1 tag for value */
		const unsigned char *,		/* value */
		size_t,			/* len of value */
                TC_CONTEXT *ctx);

int tc_set_avader (PKIAttributeTypeAndValue *p,
		const unsigned char *type,
		size_t typelen,
		const unsigned char *val,
		size_t vallen,
		TC_CONTEXT *ctx);

int compiler2tc_error(int);

time_t tc_decode_UTCTime (PKIUTCTime *);

int tc_compare_time(PKITime *, PKITime *, TC_CONTEXT *ctx);

int tc_compare_serial(PKICertificateSerialNumber *,
		      PKICertificateSerialNumber *);

int tc_compare_GeneralName (PKIGeneralName *, PKIGeneralName *, TC_CONTEXT *);

int tc_compare_block (PKIVariableBlock *, PKIVariableBlock *);

int tc_set_pub_key (PKICertificate *, const unsigned char *, size_t);

int tc_set_alg (PKIAlgorithmIdentifier *, const unsigned char *, size_t,
		const unsigned char *, size_t,
		TC_CONTEXT *ctx);


int tc_check_crl (TC_CONTEXT *ctx, TC_CERT *cert, int flags);

/* destroys the LIST of handlers created by calls to tc_register_handler() */
void tc_free_handlers(TC_LIST **, TC_MemoryMgr *);

int CopyName(PKIName **to, PKIName *from, TC_CONTEXT *ctx);

void TrimWS(char *c);

time_t tc_decode_GeneralizedTime (PKIGeneralizedTime *p);

int tc_EncodeGeneralizedTime(TC_GeneralizedTime **p, time_t, TC_CONTEXT *);

int ParseStringRDN(
    PKIRelativeDistinguishedName *rdn,
    const char *name,
    TC_CONTEXT *context);

char *RDNToString(
     PKIRelativeDistinguishedName *rdnName,
     TC_CONTEXT *context);

#endif /* _cms_h_ */
