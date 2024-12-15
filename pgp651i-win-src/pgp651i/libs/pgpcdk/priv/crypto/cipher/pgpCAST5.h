/*
 * pgpCAST5.h - header file for pgpCAST5.c
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpCAST5.h,v 1.6 1997/06/13 01:52:54 lloyd Exp $
 */
#ifndef Included_pgpCAST5_h
#define Included_pgpCAST5_h

#include "pgpSymmetricCipherPriv.h"		/* for Cipher */

PGP_BEGIN_C_DECLARATIONS

/*
 * This is the definition of the CAST5 cipher, for use with the
 * PGP Generic Cipher code.
 */
extern PGPCipherVTBL const cipherCAST5;

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpCAST5_h */
