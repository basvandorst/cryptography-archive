/*
 * pgpRC2.h - header file for pgpRC2.c
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpRC2.h,v 1.1 1999/05/15 06:14:48 hal Exp $
 */
#ifndef Included_pgpRC2_h
#define Included_pgpRC2_h

#include "pgpSymmetricCipherPriv.h"		/* for Cipher */

PGP_BEGIN_C_DECLARATIONS

/*
 * This is the definition of the 40- and 128-bit RC2 ciphers, for use
 * with the PGP Generic Cipher code.
 */
extern PGPCipherVTBL const cipherRC2_40;
extern PGPCipherVTBL const cipherRC2_128;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpRC2_h */
