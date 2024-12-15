/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: PluginInfo.h,v 1.14.8.1 1998/11/12 03:12:45 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PluginInfo_h	/* [ */
#define Included_PluginInfo_h

#include <windows.h>
#include "pgpUtilities.h"
#include "pgpTLS.h"

/* Data shared across and during window procs */

typedef struct _PluginInfo
{
	PGPContextRef pgpContext;
	PGPtlsContextRef tlsContext;
	PGPMemoryMgrRef memoryMgr;
	BOOL bLoaded;
	UINT nPurgeCacheMsg;
	HMENU hMainMenu;
	HMENU hToolsMenu;
	HMENU hPGPMenu;
	HWND hwndCurrent;
	HWND hStatusBar;
	HWND hToolbar;
	HWND hEdit;
	HWND hMessage;
	BOOL bEncrypt;
	BOOL bSign;
	char *szReplyText;
	char *szOutput;
	UINT nPGPPosition;
	INT nPGPKeysButton;
	INT nPGPKeysImage;
	INT nPGPKeysString;
} PluginInfo;


/* Outlook Express Command IDs */

#define IDC_SAVE					110
#define IDC_UNDO					117
#define IDC_CUT						118
#define IDC_COPY					119
#define IDC_PASTE					120
#define IDC_SELECTALL				122
#define IDC_NEXT					139
#define IDC_PREVIOUS				140
#define IDC_REPLY					142
#define IDC_REPLYTOALL				143
#define IDC_REPLYTOGROUP			144
#define IDC_REPLYTOAUTHOR			145
#define IDC_FORWARD					146
#define IDC_NEXTUNREADARTICLE		151
#define IDC_PRINT					168	
#define IDC_SENDMESSAGE_MENU		207
#define IDC_ATTACHFILE				210
#define IDC_NEXTUNREADTHREAD		227
#define IDC_POSTARTICLE_MENU		234
#define IDC_REPLYTOGROUPANDAUTHOR	396
#define IDC_ENCRYPT_SMIME			461
#define IDC_SIGN_SMIME				460
#define IDC_NEXTUNREADMESSAGE		471
#define IDC_SENDLATER				674
#define IDC_POSTLATER				675
#define IDC_SENDMESSAGE_BUTTON		678
#define IDC_POSTARTICLE_BUTTON		679


/* Plug-in Command IDs */

#define IDC_ENCRYPT_PGP				40001
#define IDC_SIGN_PGP				40002
#define IDC_ENCRYPTNOW				40003
#define IDC_SIGNNOW					40004
#define IDC_ENCRYPTSIGNNOW			40005
#define IDC_PREFS					40010
#define IDC_PGPKEYS					40011
#define IDC_DECRYPT					40020


/* "Tools" submenu positions */

#define ENCRYPT_SMIME_POS	8
#define ENCRYPT_PGP_POS		9
#define SIGN_SMIME_POS		10
#define SIGN_PGP_POS		11


/* Other menu and submenu positions */

#define FILE_MENU_POS				0
#define FILESAVEAS_MENU_POS			0
#define FILESTATIONERY_MENU_POS		2
#define FILEMOVE_MENU_POS			5
#define FILECOPY_MENU_POS			6
#define FILEPRINT_MENU_POS			8
#define EDIT_MENU_POS				1
#define VIEW_MENU_POS				2
#define VIEWFONTS_MENU_POS			2
#define VIEWLANG_MENU_POS			3


/* Custom window messages and timers */

#define PASTE_REPLY_TIMER		262
#define WATCH_TOOLBAR_TIMER		263
#define CORRECT_TOOLBAR_TIMER	264
#define AUTO_DECRYPT_TIMER		265

#define TRANSPARENT_COLOR	RGB(255, 0, 0)


PluginInfo * CreatePluginInfo(HWND hwnd);
PluginInfo * GetPluginInfo(HWND hwnd);
void SavePluginInfo(HWND hwnd, PluginInfo *plugin);
void FreePluginInfo(PluginInfo *plugin);

void SetReplyText(char *szText);
char *GetReplyText(void);
void FreeReplyText(void);


#endif /* ] Included_PluginInfo_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
