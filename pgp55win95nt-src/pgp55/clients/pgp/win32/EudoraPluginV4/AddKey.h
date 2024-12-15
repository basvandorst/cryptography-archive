/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	AddKey.h,v 1.3 1997/07/16 05:51:46 elrod Exp
____________________________________________________________________________*/
#ifndef Included_AddKey_h	/* [ */
#define Included_AddKey_h

#include "PGPErrors.h"

#define KEY_IN_BUFFER			0x0000001
#define KEY_IN_FILE				0x0000002

#ifdef __cplusplus
extern "C" {
#endif

PGPError AddKey(char* szBuffer, DWORD dwInSize, long flags);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_AddKey_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
