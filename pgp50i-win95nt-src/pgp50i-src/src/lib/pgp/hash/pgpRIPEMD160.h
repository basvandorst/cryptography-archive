/*
* pgpRIPEMD160.h -- RIPEMD-160 message digest
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpRIPEMD160.h,v 1.1.2.2 1997/05/21 19:06:48 mhw Exp $
*/

#ifndef PGPRIPEMD160_H
#define PGPRIPEMD160_H

#include "pgpHash.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct PgpHash const HashRIPEMD160;

#ifdef __cplusplus
}
#endif

#endif /* !PGPRIPEMD160_H */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
