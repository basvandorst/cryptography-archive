/*
* pgpDES3.h - headers for TripleDES
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* By Richard Outerbridge
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpDES3.h,v 1.2.2.1 1997/06/07 09:49:53 mhw Exp $
*/

#ifndef PGPDES3_H
#define PGPDES3_H

#include "pgpCipher.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is the definition of the 3DES cipher, for use with the PGP
* Generic Cipher code.
*/

extern struct PgpCipher const cipher3DES;

#ifdef __cplusplus
}
#endif

#endif /* PGPDES3_H */
