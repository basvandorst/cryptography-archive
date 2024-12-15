/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PluginInfo.h,v 1.18 1999/04/26 19:13:24 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_PluginInfo_h	/* [ */
#define Included_PluginInfo_h

#include <windows.h>
#include "pgpUtilities.h"
#include "pgpTLS.h"

/* Data shared across and during window procs */

#define PLUGIN_INFO_PROP		"PGP plugin"
#define TOOLBAR_TYPE_PROP		"PGP toolbar type"

typedef enum _PluginAction
{
	PluginAction_None = 0,
	PluginAction_Encrypt,
	PluginAction_Reply
} PluginAction;

typedef struct _PluginInfo
{
	PGPContextRef pgpContext;
	PGPtlsContextRef tlsContext;
	PGPMemoryMgrRef memoryMgr;
	BOOL bLoaded;
	BOOL bOE5;
	UINT nPurgeCacheMsg;
	UINT nCopyDoneMsg;
	UINT nPasteDoneMsg;
	UINT nMsg;
	PluginAction nAction;
	WPARAM wParam;
	LPARAM lParam;
	HMENU hMainMenu;
	HMENU hToolsMenu;
	HMENU hPGPMenu;
	HWND hwndCurrent;
	HWND hStatusBar;
	HWND hToolbar;
	HWND hMenuToolbar;
	HWND hEdit;
	HWND hMessage;
	HWND hDlg;
	BOOL bEncrypt;
	BOOL bSign;
	BOOL bDisableMenus;
	char *szOldText;
	char *szReplyText;
	char *szOutput;
	UINT nPGPPosition;
	INT nPGPKeysButton;
	INT nPGPKeysImage;
	INT nPGPKeysString;
	INT nEncryptImage;
	INT nEncryptString;
	INT nSignImage;
	INT nSignString;
	INT nDecryptImage;
	INT nDecryptString;
	INT nButtonSizeX;
	INT nButtonSizeY;
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

#define IDC_OE5_PRINT					40116	
#define IDC_OE5_COPY					40124
#define IDC_OE5_SELECTALL				40125
#define IDC_OE5_PREVIOUS				40145
#define IDC_OE5_NEXT					40146
#define IDC_OE5_NEXTUNREADMESSAGE		40147
#define IDC_OE5_NEXTUNREADTHREAD		40148
#define IDC_OE5_REPLY					40176
#define IDC_OE5_REPLYTOALL				40177
#define IDC_OE5_REPLYTOGROUP			40178
#define IDC_OE5_FORWARD					40180
#define IDC_OE5_SENDLATER				40225
#define IDC_OE5_SAVE					40227
#define IDC_OE5_UNDO					40229
#define IDC_OE5_CUT						40230
#define IDC_OE5_PASTE					40231
#define IDC_OE5_ATTACHFILE				40237
#define IDC_OE5_ENCRYPT_SMIME			40260
#define IDC_OE5_SIGN_SMIME				40299
#define IDC_OE5_SENDMESSAGE				40411


/* Plug-in Command IDs */

#define IDC_ENCRYPT_PGP				30001
#define IDC_SIGN_PGP				30002
#define IDC_ENCRYPTNOW				30003
#define IDC_SIGNNOW					30004
#define IDC_ENCRYPTSIGNNOW			30005
#define IDC_PREFS					30010
#define IDC_PGPKEYS					30011
#define IDC_DECRYPT					30020
#define IDC_PGPMENU					30100


/* "Tools" submenu positions */

#define ENCRYPT_SMIME_POS	8
#define ENCRYPT_PGP_POS		9
#define SIGN_SMIME_POS		10
#define SIGN_PGP_POS		11

#define OE5_ENCRYPT_SMIME_POS	2
#define OE5_ENCRYPT_PGP_POS		3
#define OE5_SIGN_SMIME_POS		4
#define OE5_SIGN_PGP_POS		5


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

#define OE5_FILESAVEAS_MENU_POS			2
#define OE5_FILESTATIONERY_MENU_POS		4
#define OE5_FILEMOVE_MENU_POS			6
#define OE5_FILECOPY_MENU_POS			7
#define OE5_FILEPRINT_MENU_POS			10
#define OE5_VIEWTEXTSIZE_MENU_POS		3
#define OE5_VIEWENCODING_MENU_POS		4

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
