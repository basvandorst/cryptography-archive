/*
 * pgpIDEA.h - header file for pgpIDEA.c
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpIDEA.h,v 1.7 1997/10/10 18:08:13 lloyd Exp $
 */
#ifndef Included_pgpIDEA_h
#define Included_pgpIDEA_h


#include "pgpSDKBuildFlags.h"

#ifndef PGP_IDEA
#error you must define PGP_IDEA one way or the other
#endif


#if PGP_IDEA	/* [ */


#include "pgpSymmetricCipherPriv.h"		/* for Cipher */

PGP_BEGIN_C_DECLARATIONS

/*
 * This is the definition of the IDEA cipher, for use with the
 * PGP Generic Cipher code.
 */
extern PGPCipherVTBL const cipherIDEA;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpIDEA_h */



#endif /* ] PGP_IDEA */