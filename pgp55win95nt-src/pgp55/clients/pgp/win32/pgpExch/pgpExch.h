/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpExch.h,v 1.4 1997/10/02 16:02:18 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpExch_h	/* [ */
#define Included_pgpExch_h

#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "PGPRecip.h"


// In ExchEncryptSign.cpp
extern PGPError EncryptSignBuffer(HWND hwnd, PGPContextRef context, 
								  void *pInput, DWORD dwInSize,
								  RECIPIENTDIALOGSTRUCT *prds, 
								  PGPOptionListRef *pSignOptions,
								  void **ppOutput, PGPSize *pOutSize, 
								  BOOL bEncrypt, BOOL bSign, 
								  BOOL bAttachment, BOOL bUseMIME);

extern PGPError EncryptSignFile(HWND hwnd, PGPContextRef context, 
								char *szInFile, RECIPIENTDIALOGSTRUCT *prds, 
								PGPOptionListRef *pSignOptions,
								char **pszOutFile, BOOL bEncrypt, BOOL bSign, 
								BOOL bUseMIME);

// In ExchDecryptVerify.cpp
extern PGPError DecryptVerifyBuffer(HWND hwnd,	PGPContextRef context, 
									void *pInput, DWORD dwInSize, 
									void *pAttachment, 
									DWORD dwAttachmentSize, BOOL bAutoAddKeys,
									void **ppOutput, PGPSize *pOutSize);

extern PGPError DecryptVerifyFile(HWND hwnd, PGPContextRef context, 
								  char *szInFile, char *szDataFile, 
								  BOOL bAutoAddKeys, char **pszOutFile);

// In ExchAddKey.cpp
extern PGPError AddKeyBuffer(HWND hwnd, PGPContextRef context, void* pInput,
							 DWORD dwInSize, BOOL bAutoAddKeys);

extern PGPError AddKeyFile(HWND hwnd, PGPContextRef context, char *szInFile,
						   BOOL bAutoAddKeys);


#endif /* ] Included_pgpExch_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


