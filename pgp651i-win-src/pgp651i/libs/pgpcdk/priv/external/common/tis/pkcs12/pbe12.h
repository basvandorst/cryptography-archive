/* Copyright (C) 1999 Network Associates, Inc.
   Author: Michael_Elkins@nai.com
   Last Edit: April 9, 1999 */

/* internal header file for pkcs-12 library */

#include "libpkcs12.h"

#include "pkcs12_asn.h"
#include "pkcs12_oid.h"

#include "x509CMS.h"

#include "pgpErrors.h"
#include "pgpMemoryMgr.h"
#include "pgpErrors.h"
#include "pgpPFLErrors.h"
#include "pgpHash.h"
#include "pgpHMAC.h"
#include "pgpCBC.h"

#define kPGPPBEIDEncryptKey	0x01
#define kPGPPBEIDIV		0x02
#define kPGPPBEIDMACKey		0x03

PGPError
pbe12GenerateRandomBits (
    PGPContextRef	context,
    PGPHashAlgorithm	alg,
    unsigned char	id,
    size_t		iterationCount,
    const unsigned char	*salt,
    size_t		saltSize,
    const unsigned char	*password,
    size_t		passwordSize,
    size_t		n,		/* [IN] n, num of random bytes */
    unsigned char	**A);		/* [OUT] random bit string */

PGPError
pbe12ASCIIToBMPString (
    PKICONTEXT		*context,
    const unsigned char	*ascii,
    size_t		asciilen,
    unsigned char	**bmp,
    size_t		*bmplen);
