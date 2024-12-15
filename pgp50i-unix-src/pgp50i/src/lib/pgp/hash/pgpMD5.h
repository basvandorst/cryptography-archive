/*
* pgpMD5.h -- The MD-5 Message Digest.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpMD5.h,v 1.2 1996/12/19 20:26:35 mhw Exp $
*/

#ifndef PGPMD5_H
#define PGPMD5_H

#include "pgpHash.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct PgpHash const HashMD5;

#ifdef __cplusplus
}
#endif

#endif /* !PGPMD5_H */
