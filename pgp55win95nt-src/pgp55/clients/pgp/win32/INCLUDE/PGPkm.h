/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPkm.h - include file for PGP key manager DLL
	

	$Id: PGPkm.h,v 1.20 1997/11/05 16:16:42 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPkm_h	/* [ */
#define Included_PGPkm_h

#ifdef _PGPKMDLL
# define PGPkmExport __declspec( dllexport )
#else
# define PGPkmExport __declspec( dllimport )
#endif

// synchronization message
#define KM_M_REQUESTSDKACCESS	WM_APP+222
#define KM_M_UPDATEKEYINTREE	WM_APP+223

// options bits
#define KMF_READONLY		0x0001
#define KMF_ENABLECOMMITS	0x0002
#define KMF_NOVICEMODE		0x0004
#define KMF_ONLYSELECTKEYS	0x0008
#define KMF_ENABLEDROPIN	0x0010
#define KMF_ENABLEDRAGOUT	0x0020
#define KMF_MODALPROPERTIES	0x0040
#define KMF_ENABLERELOADS	0x0080
#define KMF_PENDINGBUCKET	0x0100
#define KMF_DISABLEKEYPROPS	0x0200
#define KMF_DISABLERSAOPS	0x0400
#define KMF_MARGASINVALID	0x0800

// action codes
#define KM_COPY				0x00000001
#define KM_PASTE			0x00000002
#define KM_DELETE			0x00000004
#define KM_SELECTALL		0x00000008
#define KM_COLLAPSEALL		0x00000010
#define KM_COLLAPSESEL		0x00000020
#define KM_EXPANDALL		0x00000040
#define KM_EXPANDSEL		0x00000080
#define KM_CERTIFY			0x00000100
#define KM_ENABLE			0x00000200
#define KM_DISABLE			0x00000400
#define KM_ADDUSERID		0x00000800
#define KM_REVOKE			0x00001000
#define KM_SETASDEFAULT		0x00002000
#define KM_SETASPRIMARY		0x00004000
#define KM_IMPORT			0x00008000
#define KM_EXPORT			0x00010000
#define KM_PROPERTIES		0x00020000
#define KM_SENDTOSERVER		0x00040000
#define KM_GETFROMSERVER	0x00080000
#define KM_DELETEFROMSERVER	0x00100000
#define KM_DISABLEONSERVER	0x00200000
#define KM_ADDTOMAIN		0x10000000
#define KM_UNSELECTALL		0x20000000

#define KM_MAINACTIONS		0x000FFFFF
#define KM_ALLACTIONS		0x300FFFFF

// column codes
#define KM_VALIDITY			0x00000001
#define KM_SIZE				0x00000002
#define KM_DESCRIPTION		0x00000004
#define KM_KEYID			0x00000008
#define KM_TRUST			0x00000010
#define KM_CREATION			0x00000020
#define KM_EXPIRATION		0x00000040
#define KM_ADK				0x00000080

// selection bits set in TLN_CONTEXTMENU notification
#define PGPKM_KEYFLAG		0x01
#define PGPKM_UIDFLAG		0x02
#define PGPKM_CERTFLAG		0x04

#define PGPKM_HELPFILE		0x0001
#define PGPKM_KEYSERVER		0x0002
#define PGPKM_OPTIONS		0x0004
#define PGPKM_DISABLEFLAGS	0x0008
#define PGPKM_COLUMNFLAGS	0x0010
#define PGPKM_STATUSBAR		0x0020
#define PGPKM_ALLITEMS		0x003F

// typedefs
typedef struct _KM FAR* HKEYMAN;
typedef struct {				// configuration information
	ULONG	ulMask;				//  mask bits specifying things to configure
	LPSTR	lpszHelpFile;		//  name of help file
	LPSTR	lpszPutKeyserver;	//  server to send keys to
	ULONG	ulOptionFlags;		//	logical OR of option bits (above)
	ULONG	ulDisableActions;	//  logical OR of actions (above) to disable
	ULONG	ulShowColumns;		//  logical OR of columns to always show
	ULONG	ulHideColumns;		//  logical OR of columns to always hide
	HWND	hWndStatusBar;		//  handle of status bar window
} KMCONFIG, *LPKMCONFIG;

#include "..\include\pgpWErr.h"

#ifdef __cplusplus
extern "C" {
#endif


//________________________________________________
//
//	PGPkmCreateKeyManager -
//	Creates empty keymanager window
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

HKEYMAN PGPkmExport 
PGPkmCreateKeyManager (PGPContextRef	Context,
					   HWND				hWndParent, 
					   INT				Id, 
					   INT				x, 
					   INT				y,
					   INT				nWidth, 
					   INT				nHeight);

//________________________________________________
//
//	PGPkmSetConfiguration -
//	Configures keymanager window
//
//	Entry parameters :
//		hKeyMan		- handle of key manager to configure
//		pKMConfig	- pointer to KMCONFIG struct
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmConfigure (HKEYMAN		hKeyMan, 
				LPKMCONFIG	pKMConfig);

//________________________________________________
//
//	PGPkmDestroyKeyManager -
//	Destroys keymanager window
//
//	Entry parameters :
//		hKeyMan			- handle of key manager to destroy
//		bSaveColumnInfo	- TRUE => save column info to pref file
//
//	Returns kPGPError_NoErr if successful
//

PGPError PGPkmExport 
PGPkmDestroyKeyManager (HKEYMAN hKeyMan,
						BOOL	bSaveColumnInfo);

//________________________________________________
//
//	PGPkmDefaultNotificationProc -
//	Handles default behavior for TreeList notifications
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//		lParam	- LPARAM from WM_NOTIFY message
//
//	Returns kPGPError_NoErr
//

PGPError PGPkmExport 
PGPkmDefaultNotificationProc (HKEYMAN	hKeyMan, 					  
							  LPARAM	lParam);

//________________________________________________
//
//	PGPkmSynchronizeThreadAccessToSDK -
//	synchronizes
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//
//	Returns kPGPError_NoErr
//

VOID PGPkmExport 
PGPkmSynchronizeThreadAccessToSDK (HKEYMAN	hKeyMan);


//________________________________________________
//
//	PGPkmUpdateKeyInTree -
//	update key following properties change
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//		key		- key to update
//
//	Returns kPGPError_NoErr
//

VOID PGPkmExport 
PGPkmUpdateKeyInTree (HKEYMAN hKeyMan, PGPKeyRef key);


//________________________________________________
//
//	PGPkmIsActionEnabled -
//	reports if specified action is enabled or not
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//		uAction	- action constant from above list
//
//	Returns TRUE if enabled, FALSE if not
//

BOOL PGPkmExport 
PGPkmIsActionEnabled (HKEYMAN	hKeyMan, 
					  ULONG		uAction);

//________________________________________________
//
//	PGPkmPerformAction -
//	requests that specified action be performed on
//	currently selected objects
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//		uAction	- action constant from above list
//
//	Returns kPGPError_NoErr if successful
//

PGPError PGPkmExport 
PGPkmPerformAction (HKEYMAN hKeyMan, 
					ULONG	uAction);

//________________________________________________
//
//	PGPkmGetManagerWindow -
//	returns HWND of keymanager window
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//
//	Returns HWND if successful, NULL if error.
//

HWND PGPkmExport 
PGPkmGetManagerWindow (HKEYMAN hKeyMan);

//________________________________________________
//
//	PGPkmLoadKeySets -
//	load keyset into manager
//
//	Entry parameters :
//		hKeyMan		- handle of key manager
//		pKeySetDisp	- keyset to display in window
//		pKeySetMain	- main keyset used for commits and trust calcs
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmLoadKeySet (HKEYMAN		hKeyMan, 
				 PGPKeySetRef	KeySetDisp,
				 PGPKeySetRef	KeySetMain);

//________________________________________________
//
//	PGPkmReLoadKeySet -
//	reload keyset into manager
//
//	Entry parameters :
//		hKeyMan		- handle of key manager
//		bExpandNew	- TRUE => expand any newly-found objects
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmReLoadKeySet (HKEYMAN hKeyMan, BOOL bExpandNew);


//________________________________________________
//
//	PGPkmSelectColumns -
//	post dialog which allows user to select columns to display
//
//	Entry parameters :
//		hKeyMan		- handle of key manager
//		bDialog		- TRUE => display dialog to allow user to select
//					  FALSE => save current settings only
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmSelectColumns (HKEYMAN hKeyMan, BOOL bDialog);


//________________________________________________
//
//	PGPkmSelectKey -
//	select the specified key
//
//	Entry parameters :
//		hKeyMan		- handle of key manager
//		key			- key to select
//		bDeselect	- TRUE => deselect all keys before selecting
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmSelectKey (HKEYMAN hKeyMan, PGPKeyRef key, BOOL bDeselect);


//________________________________________________
//
//	PGPkmQueryAddKeys -
//	displays and handles selective key import dialog
//
//	Entry parameters :
//		Context			- PGP library context
//		hWndParent		- handle of parent window
//		pKeySetToAdd	- keyset containing keys to add
//		pKeySetMain		- keyset to which keys will be added
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmQueryAddKeys (PGPContextRef	Context,
				   HWND				hWndParent, 
				   PGPKeySetRef		KeySetToAdd,			
				   PGPKeySetRef		KeySetMain);


#ifdef __cplusplus
}
#endif

#endif /* ] Included_PGPkm_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
