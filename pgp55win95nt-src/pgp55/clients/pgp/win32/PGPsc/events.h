/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: events.h,v 1.16 1997/10/08 19:04:55 wjb Exp $
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
	HWND		    hwnd;
	HWND			hwndWorking;
	PGPFileSpecRef	fileRef;
	char			*fileName;
	PGPUInt32		sectionCount;
	PGPKeyID		*RecipientKeyIDArray;
	DWORD			dwKeyIDCount;
	char			*Action;
	PGPKeySetRef	OriginalKeySetRef;
	PGPKeyRef		*SignKey;
	unsigned short	PassCount;
	WORKSTRUCT		ws;
	char			*ConvPassPhrase;
	char			*PassPhrase;
	PGPKeySetRef	RecipientKeySet;
	BOOL			FoundPGPData;
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
