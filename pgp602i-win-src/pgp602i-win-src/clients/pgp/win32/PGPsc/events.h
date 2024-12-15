/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: events.h,v 1.23.2.1.2.1 1998/11/12 03:13:36 heller Exp $
____________________________________________________________________________*/
#ifndef Included_EVENTS_h	/* [ */
#define Included_EVENTS_h

typedef struct _WORKSTRUCT {
	char			 TitleText[MAX_PATH+40];
	char			 StartText[40];
	char			 EndText[40];
	unsigned long	 sofar;
	unsigned long	 newsofar;
	unsigned long	 total;
	unsigned long	 maxpos;
	unsigned long	 curpos;
	BOOL			 CancelPending;
} WORKSTRUCT;

typedef struct _MYSTATE {
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
	WORKSTRUCT		ws;
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
