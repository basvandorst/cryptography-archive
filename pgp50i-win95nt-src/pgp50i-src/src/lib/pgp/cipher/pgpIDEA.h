/*
* pgpIDEA.h - header file for pgpIDEA.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpIDEA.h,v 1.2.2.1 1997/06/07 09:49:54 mhw Exp $
*/
#ifndef PGPIDEA_H
#define PGPIDEA_H

	#include "pgpCipher.h" 	/* for struct Cipher */

#ifdef __cplusplus
extern "C" {
#endif

/*
* This is the definition of the IDEA cipher, for use with the
* PGP Generic Cipher code.
*/
extern struct PgpCipher const cipherIDEA;

#ifdef __cplusplus
}
#endif

#endif /* !PGPIDEA_H */
