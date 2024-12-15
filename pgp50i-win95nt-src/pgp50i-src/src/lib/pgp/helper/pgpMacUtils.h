/*
* pgpMacUtils.h -- Various Macintosh utilities
*
* Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMacUtils.h,v 1.1.2.1 1997/06/07 09:50:09 mhw Exp $
*/

#ifndef PGPMACUTILS_H	/* [ */
#define PGPMACUTILS_H

#include "pgpErr.h"

#ifdef __cplusplus
extern "C" {
#endif

PGPError PGPExport		pgpErrorFromMacError(OSErr macError,
								PGPError defaultError);

#ifdef __cplusplus
}
#endif

#endif	/* ] PGPMACUTILS_H */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
