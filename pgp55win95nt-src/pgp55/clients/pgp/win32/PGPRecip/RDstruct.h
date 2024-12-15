/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDstruct.h,v 1.22 1997/10/20 14:53:54 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDSTRUCT_h	/* [ */
#define Included_RDSTRUCT_h

#define KEYIDLENGTH 20 // Formerly 12
#define SIZELENGTH 30

#define PUIF_NOTFOUND 1
#define PUIF_MOVEME 2
#define PUIF_SELECTED 4
#define PUIF_MANUALLYDRAGGED 8
#define PUIF_LEFTLIST 16
#define PUIF_ADKWARNPLEASE 32
#define PUIF_SYSTEMKEY 64
#define PUIF_SIGNEDBYCORP 128
#define PUIF_GROUP 256
#define PUIF_HASRSA 512
#define PUIF_HASDSA 1024

#define ADKCLASS_MOVEWARNING 128

#define NUMCOLUMNS 4

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

typedef struct _ADKINFO
{
	void		*OwnerPUI;
	BOOL		ADKOK;
    PGPKeyRef	KeyRef;
	PGPKeyID KeyID;
	BYTE		ADKclass;
    struct      _ADKINFO *next;    
} ADKINFO, *PADKINFO;

typedef struct _USERKEYINFO
{
	DWORD		icon;
    DWORD       Algorithm;
    DWORD       Validity;
    DWORD       Trust;
    DWORD       Flags;
    char        UserId[kPGPMaxUserIDSize];
    PGPKeyRef	KeyRef;
    char        szSize[SIZELENGTH];
    PADKINFO    ADK;
    int         refCount;
	int			ADKEnforceCount;

	PGPGroupSetRef GroupSet;
	PGPGroupID  GroupID;
	int			NumGroupMembers;

    struct      _USERKEYINFO *next;
    
} USERKEYINFO, *PUSERKEYINFO;

typedef struct
{
// Admin Prefs
PGPBoolean bEnforceRemoteADK;
PGPBoolean bUseOutgoingADK;
PGPBoolean bEnforceOutgoingADK;
PGPBoolean bAllowConventionalEncryption;
PGPBoolean bWarnNotCorpSigned;

char		outADKKeyID[40];
UINT		outADKKeyAlgorithm;
char		corpKeyID[40];
UINT		corpKeyAlgorithm;

PGPKeyRef  RSAADKKey;
PGPKeyRef  CorpKey;

// Client Prefs
PGPBoolean bMarginalInvalid;
PGPBoolean bEncToSelf;
PGPBoolean bSynchNotFound;
PGPBoolean bWarnOnADK;

PGPcdGROUPFILE *Group;

} PREFS, *PPREFS;

typedef struct
{
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
PRECIPIENTDIALOGSTRUCT prds;
PUSERKEYINFO gUserLinkedList; 
HWND hwndRecipients;
HWND hwndUserIDs;
HWND hwndOptionASCII;
BOOL RSortAscending;
int RSortSub;
BOOL USortAscending;
int USortSub;
UINT AddUserRetVal;
PPREFS Prefs;
BOOL Reloading;
BOOL Pending;
HWND hwndRelay;
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
