/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: October 16, 1998 */

#include "pkcsreq_asn.h"

/*
 * ERROR CODES
 */

/* successful completion */
#define VRI_E_OK		0

/* string contains invalid TEXT character */
#define VRI_E_INVALID_CHAR	-1

/* unknown type of client (not EE or RA) */
#define VRI_E_UNKNOWN_TYPE	-2

/* missing mandatory attribute/value pair */
#define VRI_E_MISSING_MANDATORY	-3

/* bad type for ava */
#define VRI_E_AVA_TYPE		-4

/* invalid value for cert_type ava */
#define VRI_E_CERT_TYPE		-5

/* invalid value for embed_email */
#define VRI_E_EMBED_EMAIL	-6

/* length too long */
#define VRI_E_LENGTH		-7

/* invalid value for `authenticate' ava */
#define VRI_E_AUTHENTICATE	-8

/* error packing PKCSReq ASN.1 */
#define VRI_E_PACK		-9

/*
 *  ENTITY TYPES
 */
enum {
    VRI_ENTITY_EE,	/* end entity (client) */
    VRI_ENTITY_RA	/* registration authority */
};

/*
 * CERT REQUEST TYPES
 */
enum {
    VRI_CERT_PERSONAL,
    VRI_CERT_IPSEC,
    VRI_CERT_SECURE_SERVER,
    VRI_CERT_RA
};

typedef struct {
    const char *type;
	size_t size;
    const char *value;
} vri_ava_t;

int
vri_GenerateRegInfo (
	PKICONTEXT	*context,		/* asn.1 compiler context */
	int		entityType,		/* type of entity making
						   request */
	int		certRequestType,	/* type of certificate being
						   requested */
	vri_ava_t	*ava,			/* arrary of AVA pairs.  Final
						   AVA should have type==NULL */
	char **regInfoString);			/* returned RegInfo string.
						   Application must free. */

int
vri_GeneratePKCSReq (
	PKICONTEXT		*context,
	const unsigned char	*pkcs10,
	size_t			pkcs10Len,
	const char		*regInfo,
	unsigned char		**pkcsReq,
	size_t			*pkcsReqLen);
