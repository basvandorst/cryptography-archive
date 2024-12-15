/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: EncryptSign.h,v 1.3.2.1.2.1 1998/11/12 03:14:10 heller Exp $
____________________________________________________________________________*/
#ifndef Included_EncryptSign_h	/* [ */
#define Included_EncryptSign_h

#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpTLS.h"

#ifdef __cplusplus
extern "C" {
#endif

PGPError EncryptSignBuffer(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
						   PGPtlsContextRef tlsContext, 
						   char *szName, char *szModule,
						   void *pInput, DWORD dwInSize, 
						   RECIPIENTDIALOGSTRUCT *prds,
						   PGPOptionListRef mimeOptions,
						   PGPOptionListRef *pSignOptions,
						   void **ppOutput, PGPSize *pOutSize, BOOL bEncrypt, 
						   BOOL bSign, BOOL bBinary);

PGPError EncryptSignFile(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
						 PGPtlsContextRef tlsContext, 
						 char *szName, char *szModule, char *szInFile, 
						 RECIPIENTDIALOGSTRUCT *prds, 
						 PGPOptionListRef mimeOptions,
						 PGPOptionListRef *pSignOptions, char *szOutFile, 
						 BOOL bEncrypt, BOOL bSign, BOOL bBinary);

BOOL WrapBuffer(char **pszOutText,
				char *szInText,
				PGPUInt16 wrapLength);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_EncryptSign_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


