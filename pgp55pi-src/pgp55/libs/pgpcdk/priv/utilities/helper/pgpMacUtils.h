/*
 * pgpMacUtils.h -- Various Macintosh utilities
 *
 * $Id: pgpMacUtils.h,v 1.7 1997/06/16 22:49:00 lloyd Exp $
 */

#ifndef Included_pgpMacUtils_h	/* [ */
#define Included_pgpMacUtils_h

#include <Types.h>

#include "pgpBase.h"
#include "pgpErrors.h"

PGP_BEGIN_C_DECLARATIONS

PGPError 		pgpErrorFromMacError(OSErr macError,
								PGPError defaultError);

PGP_END_C_DECLARATIONS

#endif	/* ] Included_pgpMacUtils_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
