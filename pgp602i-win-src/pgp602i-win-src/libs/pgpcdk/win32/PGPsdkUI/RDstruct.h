/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDstruct.h,v 1.15.10.1 1998/11/12 03:24:35 heller Exp $
____________________________________________________________________________*/
#ifndef Included_RDSTRUCT_h	/* [ */
#define Included_RDSTRUCT_h

#define KEYIDLENGTH 20 // Formerly 12
#define SIZELENGTH 30

#define ADKCLASS_MOVEWARNING 128

#define NUMCOLUMNS 3

typedef struct _liststruct
{
	HWND hwnd;
	HWND hwndlist;
	HWND hwndtext[NUMCOLUMNS];
	int colwidth[NUMCOLUMNS];
	float *colratio;
} LISTSTRUCT;

typedef struct _drawstruct
{
	PGPBoolean DisplayMarginal;
	PGPBoolean MarginalInvalid;
	DWORD barcolor;
	HBRUSH stdbarbrush;
	HBRUSH spcbarbrush;
	HPEN g_seltextpen;
	HPEN g_unseltextpen;
	HPEN hilightpen;
	HPEN shadowpen;
	HPEN buttonpen;

	HBRUSH barbgbrush;
	HBRUSH HighBrush;
	HBRUSH BackBrush;
	HFONT	hFont;
	HFONT	hItalic;
	HFONT	hStrikeOut;
	HIMAGELIST hIml;
} DRAWSTRUCT;

typedef struct _USERKEYINFO
{
	DWORD			icon;
	DWORD       	Trust;
	DWORD			Validity;
	DWORD			Algorithm;
	char        	UserId[kPGPMaxUserIDSize+1];
	char        	szSize[SIZELENGTH];
	struct PGPRecipientUser	*pru;

	struct _USERKEYINFO *next;
} USERKEYINFO, *PUSERKEYINFO;

typedef struct
{
PGPContextRef			context;
PGPtlsContextRef		tlsContext;
const PGPKeyServerSpec	*ksEntries;
PGPUInt32				numKSEntries;
char					*mWindowTitle;
PGPBoolean				mSearchBeforeDisplay;
PGPKeySetRef			*mNewKeys;
PGPOptionListRef		mDialogOptions;
PGPUInt32				mNumDefaultRecipients;
PGPRecipientSpec		*mDefaultRecipients;
PGPBoolean				mDisplayMarginalValidity;
PGPBoolean				mIgnoreMarginalValidity;
PGPGroupSetRef			mGroupSet;
PGPKeySetRef			mClientKeySet;
PGPKeySetRef			*mRecipientKeysPtr;
HWND					mHwndParent;
HWND					hwndRecDlg;
HINSTANCE				g_hInst;
HWND					hwndOptions;
HIMAGELIST hDragImage;
BOOL bDragging;      
HWND hwndDragFrom;
DRAWSTRUCT ds;
LISTSTRUCT lsUser;
LISTSTRUCT lsRec;
int g_ValRSA;
int g_ValDSA;
int g_Unknown;
int g_MultRSA;
int g_MultDSA;
PUSERKEYINFO gUserLinkedList; 
HWND hwndRecipients;
HWND hwndUserIDs;
HWND hwndOptionASCII;
BOOL RSortAscending;
int RSortSub;
BOOL USortAscending;
int USortSub;
UINT AddUserRetVal;
BOOL Reloading;
BOOL Pending;
HWND hwndRelay;
PGPRecipientsList	mRecipients;
} RECGBL, *PRECGBL;

HINSTANCE g_hinst;
UINT PGPM_RELOADKEYRING;
WNDPROC origListBoxProc;
 
typedef struct
{
int gIDH;  
BOOL *pWarnCheck;
char *gWarnText;
} WARNGBL, *PWARNGBL;


#endif /* ] Included_RDSTRUCT_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
