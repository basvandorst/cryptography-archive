/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPgm.h - include file for PGP group manager DLL
	

	$Id: PGPgm.h,v 1.8 1997/10/20 15:23:51 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPgm_h	/* [ */
#define Included_PGPgm_h

#if 0
#ifdef _PGPGMDLL
# define PGPgmExport __declspec( dllexport )
#else
# define PGPgmExport __declspec( dllimport )
#endif
#else
# define PGPgmExport
#endif

// options bits
#define GMF_NOVICEMODE		0x0001
#define GMF_MARGASINVALID	0x0002

// action codes
#define GM_COPY				0x00000001
#define GM_PASTE			0x00000002
#define GM_DELETE			0x00000004
#define GM_SELECTALL		0x00000008
#define GM_COLLAPSEALL		0x00000010
#define GM_COLLAPSESEL		0x00000020
#define GM_EXPANDALL		0x00000040
#define GM_EXPANDSEL		0x00000080
#define GM_PROPERTIES		0x00000100
#define GM_NEWGROUP			0x00000200
#define GM_LOCATEKEYS		0x00000400
#define GM_GETFROMSERVER	0x00001000
#define GM_IMPORTGROUPS		0x00002000
#define GM_UNSELECTALL		0x10000000

#define GM_MAINACTIONS		0x00003FFF
#define GM_ALLACTIONS		0x10003FFF

// column codes
#define GM_VALIDITY			0x00000001
#define GM_DESCRIPTION		0x00000002

// selection bits set in TLN_CONTEXTMENU notification
#define PGPGM_GROUPFLAG		0x01
#define PGPGM_KEYFLAG		0x02

// typedefs
typedef struct _GM FAR* HGROUPMAN;
typedef struct {				// configuration information
	HKEYMAN			hKM;			//handle of associated keymanager
	PGPKeySetRef	keysetMain;		//main keyset
	LPSTR			lpszHelpFile;	//  name of help file
	ULONG			ulOptionFlags;	//	logical OR of option bits (above)
	ULONG			ulDisableActions;	//  logical OR of actions (above)
	HWND			hWndStatusBar;	//  handle of status bar window
} GMCONFIG, *LPGMCONFIG;

#include "..\include\pgpWErr.h"

#ifdef __cplusplus
extern "C" {
#endif


//________________________________________________
//
//	PGPgmCreateGroupManager -
//	Creates empty group manager window
//
//	Entry parameters :
//		Context		- PGP library context
//		hWndParent	- handle of parent window
//		id			- window ID to assign to manager (used in notifications)
//		x			- x coordinate of manager window relative to parent
//		y			- y coordinate of manager window relative to parent
//		nWidth		- width of manager window
//		nHeight		- height of manager window
//
//	Returns handle to keymanager
//

HGROUPMAN PGPgmExport 
PGPgmCreateGroupManager (PGPContextRef	Context,
					   HWND				hWndParent, 
					   INT				Id, 
					   INT				x, 
					   INT				y,
					   INT				nWidth, 
					   INT				nHeight);

//________________________________________________
//
//	PGPgmConfigure -
//	Configures group manager window
//
//	Entry parameters :
//		hGroupMan	- handle of group manager to configure
//		pGMConfig	- pointer to GMCONFIG struct
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPgmExport 
PGPgmConfigure (HGROUPMAN	hGroupMan, 
				LPGMCONFIG	pGMConfig);

//________________________________________________
//
//	PGPgmDestroyGroupManager -
//	Destroys group manager window
//
//	Entry parameters :
//		hGroupMan - handle of group manager to destroy
//
//	Returns kPGPError_NoErr if successful
//

PGPError PGPgmExport 
PGPgmDestroyGroupManager (HGROUPMAN hGroupMan);

//________________________________________________
//
//	PGPgmDefaultNotificationProc -
//	Handles default behavior for TreeList notifications
//
//	Entry parameters :
//		hGroupMan	- handle of group manager
//		lParam		- LPARAM from WM_NOTIFY message
//
//	Returns kPGPError_NoErr
//

PGPError PGPgmExport 
PGPgmDefaultNotificationProc (HGROUPMAN	hGroupMan, 					  
							  LPARAM	lParam);

//________________________________________________
//
//	PGPgmIsActionEnabled -
//	reports if specified action is enabled or not
//
//	Entry parameters :
//		hGroupMan	- handle of group manager
//		uAction		- action constant from above list
//
//	Returns TRUE if enabled, FALSE if not
//

BOOL PGPgmExport 
PGPgmIsActionEnabled (HGROUPMAN	hGroupMan, 
					  ULONG		uAction);

//________________________________________________
//
//	PGPgmPerformAction -
//	requests that specified action be performed on
//	currently selected objects
//
//	Entry parameters :
//		hGroupMan	- handle of group manager
//		uAction		- action constant from above list
//
//	Returns kPGPError_NoErr if successful
//

PGPError PGPgmExport 
PGPgmPerformAction (HGROUPMAN	hGroupMan, 
					ULONG		uAction);

//________________________________________________
//
//	PGPgmGetManagerWindow -
//	returns HWND of group manager window
//
//	Entry parameters :
//		hGroupMan	- handle of group manager
//
//	Returns HWND if successful, NULL if error.
//

HWND PGPgmExport 
PGPgmGetManagerWindow (HGROUPMAN hGroupMan);

//________________________________________________
//
//	PGPgmLoadGroups -
//	load groups into manager
//
//	Entry parameters :
//		hGroupMan		- handle of group manager
//
//	Returns kPGPError_NoErr if no error
//

PGPError 
PGPgmLoadGroups (HGROUPMAN hGroupMan); 

//________________________________________________
//
//	PGPgmReLoadGroups -
//	reload groups into manager
//
//	Entry parameters :
//		hGroupMan		- handle of group manager
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPgmExport 
PGPgmReLoadGroups (HGROUPMAN hGroupMan);

//________________________________________________
//
//	PGPgmNewGroup -
//	create new recipient group
//
//	Entry parameters :
//		hGroupMan		- handle of group manager
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPgmExport
PGPgmNewGroup (HGROUPMAN hGroupMan);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_PGPgm_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
