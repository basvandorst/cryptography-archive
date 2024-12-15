/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: events.h,v 1.36.4.1 1999/06/03 03:43:07 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_EVENTS_h	/* [ */
#define Included_EVENTS_h

#include "ProgressDialog.h"
#include "PGPsda.h"

typedef struct _MYSTATE {
	PGPBoolean		bVerEncrypted;
	HPRGDLG			hPrgDlg;
	BOOL			bDoingSDA;
	PGPContextRef	context;
	PGPtlsContextRef tlsContext;
	PGPKeySetRef	KeySet;
	char			*OperationTarget;
	BOOL			FirstOperation;
	FILELIST		*ListHead;
	HANDLE			hSemaphore;
	DWORD			dwThreadID;
	DWORD			numDone;
	DWORD			Operation;
	char			*pInput;
	DWORD			dwInputSize;
	OUTBUFFLIST		*obl;		  // For decryption
	char			*pOutput;     // For encryption
	DWORD			dwOutputSize; // ...
	PGPError		err;
	BOOL			bDelFilePostWipe;
	char			*szAppName;

// SDA
	SDAHEADER		*SDAHeader;
// encrypt
	PRECIPIENTDIALOGSTRUCT prds;
	BOOL			Encrypt; 
	BOOL			Sign;
	char			*comment;
	PGPUInt32		PrefAlg;
	PGPCipherAlgorithm *allowedAlgs; 
	int				nNumAlgs;

	HWND			hwndWorking;
	PGPFileSpecRef	fileRef;
	char			*fileName;
	char			*verifyName[MAX_PATH+1];
	PGPUInt32		sectionCount;
	PGPKeyID		*RecipientKeyIDArray;
	DWORD			dwKeyIDCount;
	PGPKeyRef		SignKey;
	unsigned short	PassCount;
	char			*ConvPassPhrase;
	char			*PassPhrase;
	PGPKeySetRef	RecipientKeySet;
	BOOL			FoundPGPData;
	PGPByte			*PassKey;
	PGPUInt32		PassKeyLen;
	PGPKeySetRef    AddedKeys;
} MYSTATE;

PGPError myEvents(
	PGPContextRef context,
	PGPEvent *event,
	PGPUserValue userValue
	);


#endif /* ] Included_EVENTS_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
