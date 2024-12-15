/*
 * pgpMacUtils.c -- Various Macintosh utilities
 *
 * $Id: pgpMacUtils.c,v 1.8 1997/08/25 18:17:59 lloyd Exp $
 */

#include "pgpConfig.h"

#if PGP_MACINTOSH	/* [ */

#include "MacErrors.h"
#include "pgpErrors.h"
#include "pgpMacUtils.h"


	PGPError
pgpErrorFromMacError(
	OSErr		macError,
	PGPError	defaultError)
{
	PGPError	err	= kPGPError_NoErr;
	
	if (macError == noErr)
		return kPGPError_NoErr;
	
	err	= MacErrorToPGPError( macError );
	if ( err == kPGPError_UnknownError )
	{
		pgpAssert( defaultError != kPGPError_NoErr );
		err	= defaultError;
	}
	
	return err;
}

#else

#error you shouldn't be compiling this file unless you're on a Mac

#endif	PGP_MACINTOSH /* ] */
/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
