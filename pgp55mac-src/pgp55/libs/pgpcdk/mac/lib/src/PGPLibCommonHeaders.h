/*____________________________________________________________________________
	PGPLibCommonHeaders.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	

	$Id: PGPLibCommonHeaders.h,v 1.5 1997/09/30 21:02:31 lloyd Exp $
____________________________________________________________________________*/

#include "pgpSDKBuildFlags.h"

#define PGPTRUSTMODEL				0


#include "pgpConfig.h"

#if PGP_MACINTOSH	/* [ */


#include <ConditionalMacros.h>

#if GENERATINGPOWERPC
#define BNINCLUDE bnippc.h
#endif

#if GENERATING68K
#define BNINCLUDE bni68020.h
#endif

/*____________________________________________________________________________
	We use pgpLeaks and so do clients.  With the same magic field, if we
	allocate a pointer, and a client deallocates it with the clients pgpFree,
	the problem may not be detected. By changing the magic field here, our
	version of leaks will use a different magic than the clients thus
	forcing a clearer error message.
____________________________________________________________________________*/
#define kLeakItemMagic			'CMem'


#include "MacDebug.h"

#endif	/* ] */