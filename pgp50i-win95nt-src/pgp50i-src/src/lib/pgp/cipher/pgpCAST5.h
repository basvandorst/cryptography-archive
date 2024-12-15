/*
* pgpCAST5.h - header file for pgpCAST5.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpCAST5.h,v 1.2.2.1 1997/06/07 09:49:50 mhw Exp $
*/
#ifndef PGPCAST5_H
#define PGPCAST5_H

#include "pgpCipher.h"			/* for struct Cipher */

#ifdef __cplusplus
extern "C" {
#endif

/*
* This is the definition of the CAST5 cipher, for use with the
* PGP Generic Cipher code.
*/
extern struct PgpCipher const cipherCAST5;

#ifdef __cplusplus
}
#endif

#endif /* !PGPCAST5_H */
