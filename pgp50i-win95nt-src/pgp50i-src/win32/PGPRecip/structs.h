/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#define KEYIDLENGTH 20 // Formerly 12
#define USERIDLENGTH 512
#define SIZELENGTH 10

#define PUIF_NOTFOUND 1
#define PUIF_MOVEME 2
#define PUIF_MATCHEDWITHRECIPIENT 4
#define PUIF_MANUALLYDRAGGED 8
#define PUIF_LEFTLIST 16
#define PUIF_MULTIPLEFOUND 32
#define PUIF_MRKWARNPLEASE 64
#define PUIF_ALREADYINSERTEDMULTI 128
#define PUIF_DEFAULTKEY 256

#define MRKCLASS_MOVEWARNING 128

typedef struct _MRKINFO
{
    char        KeyId[KEYIDLENGTH];
	byte		mrkclass;
    BOOL        MRKFoundMatch;
    struct      _MRKINFO *next;
} MRKINFO, *PMRKINFO;

typedef struct _USERKEYINFO
{
    DWORD       Algorithm;
    DWORD       Validity;
    DWORD       Trust;
    DWORD       Flags;
    char        UserId[USERIDLENGTH];
    char        KeyId[KEYIDLENGTH];
    char        szSize[SIZELENGTH];
    PMRKINFO    mrk;
    int         refCount;
    int         iImage;
    struct      _USERKEYINFO *next;

} USERKEYINFO, *PUSERKEYINFO;

typedef struct
{
HIMAGELIST hDragImage;
BOOL bDragging;
HWND hwndDragFrom;
DWORD g_barcolor;
HBRUSH g_stdbarbrush;
HBRUSH g_spcbarbrush;
HPEN g_seltextpen;
HPEN g_unseltextpen;
HBRUSH barbgbrush;
HBRUSH HighBrush;
HBRUSH BackBrush;
HFONT g_hTLFont;
HFONT g_hTLFontItalic;
HFONT g_hTLFontStrike;
int g_ValRSA;
int g_ValDSA;
int g_Unknown;
int g_MultRSA;
int g_MultDSA;
HMENU hPopup;
PRECIPIENTDIALOGSTRUCT prds;
PUSERKEYINFO gUserLinkedList;
HIMAGELIST hImageList;
HWND hwndRecipients;
HWND hwndUserIDs;
HWND hwndOptionASCII;
BOOL RSortAscending;
int RSortSub;
BOOL USortAscending;
int USortSub;
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
