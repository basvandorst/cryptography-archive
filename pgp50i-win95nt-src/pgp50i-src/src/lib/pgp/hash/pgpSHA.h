/*
* pgpSHA.h -- NIST Secure Hhash Algorithm
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpSHA.h,v 1.2 1996/12/19 20:26:35 mhw Exp $
*/

#ifndef PGPSHA_H
#define PGPSHA_H

#include "pgpHash.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct PgpHash const HashSHA;

#ifdef __cplusplus
}
#endif

#endif /* !PGPSHA_H */
