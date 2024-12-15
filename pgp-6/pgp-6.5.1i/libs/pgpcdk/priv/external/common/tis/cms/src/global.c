/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"

/* global constants */

/* RSA DSA id used in BSAFE 3.0 */
const unsigned char TC_ALG_RSA_DSA[] = {
  0x2b, 0x0e, 0x03, 0x02, 0x0c
};

/* RSA DSA with SHA1 for testing with BSAFE,
   NOTE: this is a BOGUS value just for testing!!! */
const unsigned char TC_ALG_RSA_DSA_SHA1[] = {
  0x2b, 0x0e, 0x03, 0x02, 0x7e
};

/* id-mosaicUpdatedSigAlgorithm, 2.16.840.1.101.2.1.1.19 */
const unsigned char TC_ALG_FORTEZZA_DSA[] = {
  0x60, 0x86, 0x48, 0x01, 0x65, 0x02, 0x01, 0x01, 0x13
};

/* id-mosaicKMandUpdSigAlgorithms, 2.16.840.1.101.2.1.1.20 */
const unsigned char TC_ALG_FORTEZZA_KEA_DSA[] = {
  0x60, 0x86, 0x48, 0x01, 0x65, 0x02, 0x01, 0x01, 0x14
};

/* id-mosaicUpdatedIntegrityAlgorithm, 2.16.840.1.101.2.1.1.21 */
const unsigned char TC_ALG_SHA[] = {
  0x60, 0x86, 0x48, 0x01, 0x65, 0x02, 0x01, 0x01, 0x15
};
