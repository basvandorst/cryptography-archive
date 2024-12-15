/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: AddKey.h,v 1.2 1999/03/10 02:28:58 heller Exp $
____________________________________________________________________________*/
#ifndef Included_AddKey_h	/* [ */
#define Included_AddKey_h

#include "pgpErrors.h"

#ifdef __cplusplus
extern "C" {
#endif

PGPError AddKeyBuffer(HWND hwnd, PGPContextRef context, 
					  PGPtlsContextRef tlsContext, void* pInput, 
					  DWORD dwInSize);

PGPError AddKeyFile(HWND hwnd, PGPContextRef context, 
					PGPtlsContextRef tlsContext, char *szInFile);

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
