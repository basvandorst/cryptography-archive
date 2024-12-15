/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPkm.h - include file for PGP key manager DLL
	

	$Id: PGPkm.h,v 1.39 1999/01/28 22:23:55 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPkm_h	/* [ */
#define Included_PGPkm_h

#ifdef _PGPKMDLL
# define PGPkmExport __declspec( dllexport )
#else
# define PGPkmExport __declspec( dllimport )
#endif

#include "pgpKeyServerPrefs.h"
#include "..\include\pgpWErr.h"

// synchronization message
#define KM_M_REQUESTSDKACCESS		WM_APP+222
#define KM_M_KEYPROPACTION			WM_APP+223

// reason codes
#define KMR_EXISTINGSPLITKEYDLGS	0x0001

// options bits
#define KMF_READONLY			0x0001
#define KMF_ENABLECOMMITS		0x0002
#define KMF_NOVICEMODE			0x0004
#define KMF_ONLYSELECTKEYS		0x0008
#define KMF_ENABLEDROPIN		0x0010
#define KMF_ENABLEDRAGOUT		0x0020
#define KMF_MODALPROPERTIES		0x0040
#define KMF_ENABLERELOADS		0x0080
#define KMF_PENDINGBUCKET		0x0100
#define KMF_DISABLEKEYPROPS		0x0200
#define KMF_MARGASINVALID		0x0800
#define KMF_DISABLESTATUSBAR	0x1000

// action codes
#define KM_COPY					0x00000001
#define KM_PASTE				0x00000002
#define KM_DELETE				0x00000004
#define KM_SELECTALL			0x00000008
#define KM_COLLAPSEALL			0x00000010
#define KM_COLLAPSESEL			0x00000020
#define KM_EXPANDALL			0x00000040
#define KM_EXPANDSEL			0x00000080
#define KM_CERTIFY				0x00000100
#define KM_ENABLE				0x00000200
#define KM_DISABLE				0x00000400
#define KM_ADDUSERID			0x00000800
#define KM_REVOKE				0x00001000
#define KM_SETASDEFAULT			0x00002000
#define KM_SETASPRIMARY			0x00004000
#define KM_IMPORT				0x00008000
#define KM_EXPORT				0x00010000
#define KM_PROPERTIES			0x00020000
#define KM_SENDTOSERVER			0x00040000
#define KM_GETFROMSERVER		0x00080000
#define KM_DELETEFROMSERVER		0x00100000
#define KM_DISABLEONSERVER		0x00200000
#define KM_RETRIEVECERTIFICATE	0x00400000
#define KM_ADDCERTIFICATE		0x00800000
#define KM_REVERIFYSIGS			0x01000000
#define KM_SPLITKEY				0x02000000
#define KM_ADDPHOTOID			0x04000000
#define KM_ADDREVOKER			0x08000000
#define KM_ADDTOMAIN			0x10000000
#define KM_UNSELECTALL			0x20000000

#define KM_ALLACTIONS			0x3FFFFFFF

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

// mask bits for KMCONFIGURE struct
#define PGPKM_HELPFILE		0x0001
#define PGPKM_KEYSERVER		0x0002
#define PGPKM_OPTIONS		0x0004
#define PGPKM_DISABLEFLAGS	0x0008
#define PGPKM_COLUMNFLAGS	0x0010
#define PGPKM_STATUSBAR		0x0020
#define PGPKM_SETHWNDPROC	0x0040
#define PGPKM_ALLITEMS		0x007F

// typedefs
typedef VOID (*HWNDLISTPROC) (HWND, BOOL, POINT*, HWND*);
typedef struct _KM FAR* HKEYMAN;
typedef struct {				// configuration information
	ULONG	ulMask;				//  mask bits specifying things to configure
	LPSTR	lpszHelpFile;		//  name of help file
	ULONG	ulOptionFlags;		//	logical OR of option bits (above)
	ULONG	ulDisableActions;	//  logical OR of actions (above) to disable
	ULONG	ulShowColumns;		//  logical OR of columns to always show
	ULONG	ulHideColumns;		//  logical OR of columns to always hide
	HWND	hWndStatusBar;		//  handle of status bar window

	PGPKeyServerEntry	keyserver;	//  keyserver to use for subsequent ops
} KMCONFIG, *LPKMCONFIG;

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
//		lpfnSetList	- callback to add/remove dialog windows from list
//		x			- x coordinate of manager window relative to parent
//		y			- y coordinate of manager window relative to parent
//		nWidth		- width of manager window
//		nHeight		- height of manager window
//		uFlags		- additional flags
//
//	Returns handle to keymanager
//

HKEYMAN PGPkmExport 
PGPkmCreateKeyManager (
		PGPContextRef		Context, 
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent,
		INT					iID, 
		HWNDLISTPROC		lpfnHwndListFunc,
		INT					x, 
		INT					y,
		INT					nWidth, 
		INT					nHeight);

#define PGPKM_SINGLESELECT		0x0001
#define PGPKM_SHOWSELECTION		0x0002

HKEYMAN PGPkmExport 
PGPkmCreateKeyManagerEx (
		PGPContextRef		Context,
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent, 
		INT					id, 
		HWNDLISTPROC		lpfnSetList,
		INT					x, 
		INT					y,
		INT					nWidth, 
		INT					nHeight,
		UINT				uFlags);

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
//	PGPkmProcessKeyPropMessage -
//	perform action requested by key props property sheet
//
//	Entry parameters :
//		hKeyMan	- handle of key manager
//		wParam	- wParam of message coming from prop sheet
//		lParam	- lParam of message coming from prop sheet
//
//	Returns kPGPError_NoErr
//

VOID PGPkmExport 
PGPkmProcessKeyPropMessage (HKEYMAN hKeyMan, WPARAM wParam, LPARAM lParam);


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
//	set columns to be displayed
//
//	Entry parameters :
//		hKeyMan			- handle of key manager
//		ulColumnFlags	- flags selecting columns to display
//		bRedraw			- TRUE=>forces redraw of display
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmSelectColumns (HKEYMAN hKeyMan, ULONG ulColumnFlags, BOOL bRedraw);


//________________________________________________
//
//	PGPkmSelectColumns -
//	get currently displayed columns
//
//	Entry parameters :
//		hKeyMan			- handle of key manager
//		pulColumnFlags	- buffer to receive flags of selected columns
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmGetSelectedColumns (HKEYMAN hKeyMan, ULONG* pulColumnFlags);


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
//	PGPkmGetSelectedKeys -
//	get a keyset of the currently selected keys
//
//	Entry parameters :
//		hKeyMan			- handle of key manager
//		pkeysetSelected	- pointer to buffer to receive keyset ref
//						  (must be freed when done)
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPkmExport 
PGPkmGetSelectedKeys (HKEYMAN hKeyMan, PGPKeySetRef* pkeysetSelected);


//________________________________________________
//
//	PGPkmOKToClose -
//	ask if OK to close down keymanager
//
//	Entry parameters :
//		hKeyMan		- handle of key manager
//		puReason	- buffer to receive reason for not closing
//
//	Returns TRUE if OK to close
//

BOOL PGPkmExport
PGPkmOKToClose (HKEYMAN hKeyMan, UINT* puReason);


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
