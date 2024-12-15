/*
 * pgpIDEA.h - header file for pgpIDEA.c
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpIDEA.h,v 1.1 1998/08/18 22:59:24 dgal Exp $
 */
#ifndef Included_pgpIDEA_h
#define Included_pgpIDEA_h

#include "pgpSymmetricCipherPriv.h"		/* for Cipher */

PGP_BEGIN_C_DECLARATIONS

/*
 * This is the definition of the IDEA cipher, for use with the
 * PGP Generic Cipher code.
 */
extern PGPCipherVTBL const cipherIDEA;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpIDEA_h */



