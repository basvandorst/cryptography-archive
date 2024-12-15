/* pgpDES3.h - headers for TripleDES
 *
 *	By Richard Outerbridge
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpDES3.h,v 1.1 1998/08/18 22:59:23 dgal Exp $
 */

#ifndef Included_pgpDES3_h
#define Included_pgpDES3_h

#include "pgpSymmetricCipherPriv.h"

PGP_BEGIN_C_DECLARATIONS

/* This is the definition of the 3DES cipher, for use with the PGP
 * Generic Cipher code.
 */
extern PGPCipherVTBL const cipher3DES;

PGP_END_C_DECLARATIONS

#endif /* Included_pgpDES3_h */