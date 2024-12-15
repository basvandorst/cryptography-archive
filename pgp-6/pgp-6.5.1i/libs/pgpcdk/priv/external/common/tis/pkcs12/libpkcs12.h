/* Copyright (C) 1998 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: April 9, 1999 */

#include "pgpUtilities.h"

/* PBE algorithms from PKCS#12 */
/* TODO: only kPGPPBEAlgorithm_SHA3DES is currently supported */
typedef enum {
    kPGPPBEAlgorithm_Unknown,
    kPGPPBEAlgorithm_SHA3DES,	/* sha-1 w/ DES-EDE-3 CBC */
    kPGPPBEAlgorithm_SHADESEDE2,
    kPGPPBEAlgorithm_SHARC4_128,
    kPGPPBEAlgorithm_SHARC4_40,
    kPGPPBEAlgorithm_SHARC2_128,/* sha-1 w/ RC2/128 CBC */
    kPGPPBEAlgorithm_SHARC2_40	/* sha-1 w/ RC2/40 CBC */
} PGPPBEAlgorithm;

PGPError
PKCS12InputKey (
    PGPContextRef	context,	/* [IN] pgp context */
    const PGPByte	*der,		/* [IN] pkcs #12 data */
    PGPSize		dersize,	/* [IN] pkcs #12 data size */
    const PGPByte	*pass,		/* [IN] MAC/decrypt password */
    PGPSize		passlen,	/* [IN] password length */
    PGPByte		**privkey,	/* [OUT] ASN.1 PrivateKeyInfo */
    PGPSize		*privkeysize,	/* [OUT] size of PrivateKeyInfo */
    PGPByte		**cert,		/* [OUT] certificate matching the
					   private key stored in the message */
    PGPSize		*certSize);	/* [OUT] certificate size */

PGPError
PKCS12ExportKey (
    PGPContextRef	context,	/* [IN] PGP context */
    const PGPByte	*privKeyInfo,	/* [IN] DER of PKCS-8 PrivateKeyInfo */
    PGPSize		privKeyInfoSize,/* [IN] size in bytes of privKeyInfo */
    const PGPByte	*password,	/* [IN] password to use for
					   MAC/symmetric encryption */
    PGPSize		passwordSize,	/* [IN] length in bytes of password */
    const PGPByte	*certificate,	/* [IN] DER of X.509 Certificate */
    PGPSize		certificateSize,/* [IN] size in bytes of X.509 cert */
    int			macIterations,	/* [IN] number of iterations in
					   generation of MAC key */
    int			pbeIterations,	/* [IN] number of iterations in
					   generation of PBE key */
    PGPPBEAlgorithm	pbeAlgorithm,	/* [IN] PBE algorithm to use */
    PGPByte		**pkcs12der,	/* [OUT] PKCS-12 DER */
    PGPSize		*pkcs12derSize);/* [OUT] size in bytes of PKCS-12 DER */
