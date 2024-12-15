/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: DecryptVerify.h,v 1.3.2.2.2.1 1998/11/12 03:14:09 heller Exp $
____________________________________________________________________________*/
#ifndef Included_DecryptVerify_h	/* [ */
#define Included_DecryptVerify_h

#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpTLS.h"

#ifdef __cplusplus
extern "C" {
#endif

PGPError DecryptVerifyBuffer(HINSTANCE hInst, HWND hwnd, 
							 PGPContextRef context, 
							 PGPtlsContextRef tlsContext, 
							 char *szName, char *szModule,
							 void *pInput, DWORD dwInSize,
							 BOOL bMIME, void **ppOutput, PGPSize *pOutSize,
							 BOOL *FYEO);

PGPError DecryptVerifyFile(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
						   PGPtlsContextRef tlsContext, 
						   char *szName, char *szModule,
						   char *szInFile, BOOL bMIME, BOOL bBinary,
						   char **pszOutFile, void **ppOutput, 
						   PGPSize *pOutSize, BOOL *FYEO);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_DecryptVerify_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


