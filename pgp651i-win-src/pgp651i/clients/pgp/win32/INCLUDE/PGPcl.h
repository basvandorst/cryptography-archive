/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPcl.h - header file for PGP ClientLib DLL
	

	$Id: PGPcl.h,v 1.71 1999/04/13 17:29:52 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_PGPcl_h	/* [ */
#define Included_PGPcl_h

#include "pflPrefTypes.h"
#include "pgpGroups.h" 
#include "pgpKeyServerPrefs.h"

#ifdef _PGPCLDLL
# define PGPclExport __declspec( dllexport )
#else
# define PGPclExport __declspec( dllimport )
#endif

#ifdef __cplusplus
extern "C" {
#endif


//	_______________________________________________________
//
//	Library Initialization/Cleanup 
//
//	Open library and initialize

PGPError PGPclExport 
PGPclInitLibrary (PGPContextRef context);

//	Close down DLL and purge passphrase buffers.

PGPError PGPclExport 
PGPclCloseLibrary (VOID);


//	_______________________________________________________
//
//	Word wrap convenience functions 
//
//	Wrap text using platform independent word wrap code.
//	Output buffer must be freed with PGPclFreeWrapBuffer
//
//	Entry parameters :
//		szInText		- input buffer
//		wrapColumn		- column at which to wrap
//		pszOutText		- buffer to receive pointer to
//						  output buffer containing wrapped text

PGPError PGPclExport 
PGPclWrapBuffer (
		LPSTR		szInText,
		PGPUInt16	wrapColumn,
		LPSTR*		pszOutText);

//	Free previously-wrapped text buffer.
//
//	Entry parameters :
//		textBuffer		- buffer to free

PGPError PGPclExport 
PGPclFreeWrapBuffer (LPSTR textBuffer);


//	_______________________________________________________
//
//	Miscellaneous common dialogs and UI elements

//	Post PGP preferences property sheets.
//
//	This function posts the "PGP Preferences" property
//	sheet dialog boxes.  Appropriate calls are made into
//	the pgpkeydb library to set the preferences.
//
//	Entry parameters :
//		Context		- PGP library context
//		hWndParent	- handle of parent window
//		iStartsheet	- zero-based index of property sheet 
//                    page to display initially.
//					  use the below-defined constants
//		keysetMain	- main keyset, if available.  If not
//					  available, NULL is OK.
//
//	This function returns kPGPError_UserAbort (if the user 
//	presses the cancel button)

//	preferences pages (used for "iStartsheet")
#define PGPCL_GENERALPREFS			0
#define PGPCL_KEYRINGPREFS			1
#define PGPCL_EMAILPREFS			2
#define PGPCL_HOTKEYPREFS			3
#define PGPCL_KEYSERVERPREFS		4
#define PGPCL_CAPREFS				5
#define PGPCL_ADVANCEDPREFS			6

PGPError PGPclExport 
PGPclPreferences (
		PGPContextRef	Context, 
		HWND			hWndParent, 
		INT				iStartsheet,
		PGPKeySetRef	keysetMain);

//	Get descriptive string for error code for encode/decode modules
//
//	Entry parameters :
//		iCode		- error code returned by pgp or simple library routine
//		szString	- buffer to be filled with descriptive error message
//		uLen		- length of buffer in bytes
//
//	This function evaluates iCode.  If iCode warrants an error
//	message, the buffer is filled with the message and the function
//	returns kPGPError_UnknownError.  If no message is warranted, 
//	the function returns kPGPError_NoErr.

PGPError PGPclExport 
PGPclEncDecErrorToString (
		INT		iCode, 
		LPSTR	szString,  
		UINT	uLen);

//	Putup error messagebox (if necessary) for encode/decode modules
//
//	Entry parameters :
//		hWnd		- handle of parent (NULL is OK)
//		iCode		- error code returned by pgp or simple library routine
//
//	This function calls PGPclErrorToString to evaluate iCode.
//	If PGPclErrorToString returns kPGPError_UnknownError, a message 
//  box is posted containing the descriptive text.  Otherwise no 
//	messagebox is displayed.  This function returns the value returned 
//	by PGPclErrorToString.

PGPError PGPclExport 
PGPclEncDecErrorBox (
		HWND	hWnd,
		INT		iCode);

//	Get descriptive string for error code
//
//	Entry parameters :
//		iCode		- error code returned by pgp or simple library routine
//		szString	- buffer to be filled with descriptive error message
//		uLen		- length of buffer in bytes
//
//	This function evaluates iCode.  If iCode warrants an error
//	message, the buffer is filled with the message and the function
//	returns kPGPError_UnknownError.  If no message is warranted, 
//	the function returns kPGPError_NoErr.

PGPError PGPclExport 
PGPclErrorToString (
		INT		iCode, 
		LPSTR	szString,  
		UINT	uLen);

//	Putup error messagebox (if necessary)
//
//	Entry parameters :
//		hWnd		- handle of parent (NULL is OK)
//		iCode		- error code returned by pgp or simple library routine
//
//	This function calls PGPclErrorToString to evaluate iCode.
//	If PGPclErrorToString returns kPGPError_UnknownError, a message 
//  box is posted containing the descriptive text.  Otherwise no 
//	messagebox is displayed.  This function returns the value returned 
//	by PGPclErrorToString.

PGPError PGPclExport 
PGPclErrorBox (
		HWND	hWnd,
		INT		iCode);

//	Collect entropy from keyboard/mouse.
//
//	Entry parameters :
//		Context		- PGP library context
//		hWndParent	- handle of parent window
//		uNeeded		- bits of entropy needed
//
//	This function returns kPGPError_UserAbort (if the user
//  presses the cancel button).


PGPError PGPclExport 
PGPclRandom (
		PGPContextRef	Context,
		HWND			hWndParent, 
		UINT			uNeeded);

//	Post Help|About dialog with button for browser launch to specified URL.
//
//	Entry parameters :
//		context		- current PGP context 
//		hWndParent	- handle of parent window
//		szVersion	- string containing version information
//					  to be displayed
//					  NULL => use default version string
//		szLinkText	- text to be displayed on link button
//					  (should be <= ~20 chars),
//					  NULL => use default ("www.pgpi.com")
//		szLink		- URL to which to jump
//					  NULL => use default ("http://www.pgpi.com/")

PGPError PGPclExport 
PGPclHelpAbout (
		PGPContextRef	context,
		HWND			hWndParent, 
		LPSTR			szVersion, 	
		LPSTR			szLinkText, 
		LPSTR			szLink);

//	Post nag dialog with button for browser launch to specified URL.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		szLinkText	- text to be displayed on link button
//					  (should be <= ~20 chars),
//					  NULL => use default ("Order Now!")
//		szLink		- URL to which to jump
//					  NULL => use default ("http://www.pgpi.com/")

PGPError PGPclExport 
PGPclNag (
		HWND	hWndParent, 
		LPSTR	szLinkText, 
		LPSTR	szLink);

//	Post Splash screen
//
//	Entry parameters :
//		hWndParent		- handle of parent window (if NULL,
//						  screen will not be dismissed until
//						  PGPclSetSplashParent is called 
//						  with a non-null value)
//		uMilliseconds	- milliseconds to display splash

PGPError PGPclExport 
PGPclSplash (
		PGPContextRef	Context,
		HWND			hWndParent, 
		UINT			uMS);

//	Inform splash screen who its parent is
//
//	Entry parameters :
//		hWndParent		- handle of parent window

PGPError PGPclExport 
PGPclSetSplashParent (HWND hWndParent);

//	display and handle selective key import dialog
//
//	Entry parameters :
//		hWndParent		- handle of parent window
//		KeySetToAdd	- keyset containing keys to add
//		KeySetMain		- keyset to which keys will be added
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPclExport 
PGPclQueryAddKeys (
		PGPContextRef		Context,
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent, 
		PGPKeySetRef		KeySetToAdd,			
		PGPKeySetRef		KeySetMain);

//	display keyset and allow user to select keys
//	- original calling parameters
//
//	Entry parameters :
//		hWndParent		- handle of parent window
//		pszPrompt		- prompt string
//		KeySetToDisplay	- keyset containing keys to show
//		KeySetMain		- backing keyset 
//		pKeySetSelected	- buffer to receive keyset ref of selected keys
//		
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPclExport 
PGPclSelectKeys (
		PGPContextRef		Context,
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent, 
		LPSTR				pszPrompt,
		PGPKeySetRef		KeySetToDisplay,			
		PGPKeySetRef		KeySetMain,
		PGPKeySetRef*		pKeySetSelected);

//	display keyset and allow user to select keys
//	- extended calling parameters -- added option flags
//
//	Entry parameters :
//		hWndParent		- handle of parent window
//		pszPrompt		- prompt string
//		KeySetToDisplay	- keyset containing keys to show
//		KeySetMain		- backing keyset 
//		uFlags			- option flags (see below)
//		pKeySetSelected	- buffer to receive keyset ref of selected keys
//		
//
//	Returns kPGPError_NoErr if no error
//

// option flag bits
#define PGPCL_SINGLESELECTION		0x0001

PGPError PGPclExport 
PGPclSelectKeysEx (
		PGPContextRef		Context,
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent, 
		LPSTR				pszPrompt,
		PGPKeySetRef		KeySetToDisplay,			
		PGPKeySetRef		KeySetMain,
		UINT				uFlags,
		PGPKeySetRef*		pKeySetSelected);

//	display X.509 certs and allow user to select cert
//
//	Entry parameters :
//		hWndParent		- handle of parent window
//		pszPrompt		- prompt string
//		KeySetMain		- backing keyset 
//		uFlags			- option flags (see below)
//		pkeyCert		- buffer to receive cert keyref
//		psigCert		- buffer to receive cert sigref
//		
//
//	Returns kPGPError_NoErr if no error
//

// option flag bits
#define PGPCL_CANSIGNKEYSONLY		0x0001
#define PGPCL_NOSPLITKEYS			0x0002
#define PGPCL_CACERTSONLY			0x0004

PGPError PGPclExport 
PGPclSelectX509Cert (
		PGPContextRef		context,
		HWND				hwndParent, 
		LPSTR				pszPrompt,
		PGPKeySetRef		keysetMain,
		UINT				uFlags,
		PGPKeyRef*			pkeyCert,
		PGPSigRef*			psigRef);

//	Post RSA/DSA mix warning dialog 
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//
//	Returns kPGPError_NoErr, kPGPError_UserAbort, or PGPCL_NO
//

PGPError PGPclExport 
PGPclRSADSAMixWarning (HWND hWnd, PGPBoolean *pbNeverShowAgain);

//	Check for product expiration 
//
//	Returns kPGPError_FeatureNotAvailable if the beta or eval has expired,
//  and displays a message box.  Returns kPGPError_NoErr and does nothing 
//	if it's still valid.
//
//	Entry parameters :
//		hwnd		- Parent of window so we can show the MB.  
//					  NULL is acceptable.

PGPError PGPclExport 
PGPclIsExpired (HWND hwnd);

//	Check for evaluation product expiration 
//
//	Returns kPGPError_FeatureNotAvailable if the evaluation has expired, 
//  and displays a message box.  If the eval is partially expired, the 
//  nag screen will be displayed.
//
//	Entry parameters :
//		hwnd		Parent of window so we can show the MB or nag screen.  
//					NULL is acceptable.
//		nIndex		Type of expiration to test: Encrypt/Sign or All

#define PGPCL_ENCRYPTSIGNEXPIRED	1
#define PGPCL_ALLEXPIRED			2

#if PGP_DEMO
	BOOL PGPclExport PGPclEvalExpired (HWND hwnd, int nIndex);
#else
	#define	PGPclEvalExpired(x,y)	kPGPError_NoErr
#endif	// PGP_DEMO



//	_______________________________________________________
//
//	Group files convenience functions 

typedef struct _PGPclGROUPFILE
{
	PGPFileSpecRef	filespec;
	PGPGroupSetRef	groupset;
} PGPclGROUPFILE;

//	Opens standard groups file 
//
//	Entry parameters :
//		ppGroup			address of buffer to receive pointer 
//						to group file structure

PGPError PGPclExport 
PGPclOpenGroupFile (
		PGPContextRef		Context,
		PGPclGROUPFILE**	ppGroup);

//	Saves groups file
//
//	Entry parameters :
//		pGroup			pointer to group file structure

PGPError PGPclExport
PGPclSaveGroupFile (PGPclGROUPFILE* pGroup);

//	Closes groups file
//
//	Entry parameters :
//		pGroup			pointer to group file structure

PGPError PGPclExport 	
PGPclCloseGroupFile (PGPclGROUPFILE* pGroup);


//	_______________________________________________________
//
//	Preference files convenience functions 

//	Query if this is a "Client" installation
//

BOOL PGPclExport 
PGPclIsClientInstall (VOID);

//	Query if this is a "Admin" installation
//

BOOL PGPclExport 
PGPclIsAdminInstall (VOID);

//	Get the path where client prefs files are located
//
//	Entry parameters :
//		pszPath		- buffer to receive path string
//		uLen		- length of buffer in bytes

PGPError PGPclExport
PGPclGetClientPrefsPath (
		LPSTR	pszPath, 
		UINT	uLen);

//	Open client preferences file and returns PrefRef to caller
//
//	Entry parameters :
//		memMgr		- memory manager to user for allocating fileref
//		pPrefRef	- pointer to buffer to receive PrefRef

PGPError PGPclExport 
PGPclOpenClientPrefs (
		PGPMemoryMgrRef	memMgr,
		PGPPrefRef*		pPrefRef);

//	______________________________________________
//
//  The following functions are wrappers for
//  the pfl prefs routines. These are needed so
//  groupwise plugin can access prefs (Delphi)
//  using code in the PGPcl DLL.

PGPError PGPclGetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean *data);

PGPError PGPclSetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean data);

PGPError PGPclGetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 *data);

PGPError PGPclSetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 data);

PGPError PGPclGetPrefStringAlloc(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   char **string);

PGPError PGPclGetPrefStringBuffer(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPSize maxSize,
								char *string);

PGPError PGPclSetPrefString(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  const char *string);

PGPError PGPclGetPrefData(PGPPrefRef prefRef, 
						PGPPrefIndex prefIndex, 
						PGPSize *dataLength, 
						void **inBuffer);

PGPError PGPclGetPrefFileSpec(PGPPrefRef prefRef,
								PFLFileSpecRef *prefFileSpec);

PGPError PGPclSetPrefData(PGPPrefRef prefRef, 
						PGPPrefIndex prefIndex, 
						PGPSize dataLength, 
						const void *outBuffer);

PGPError PGPclRemovePref(PGPPrefRef prefRef, 
					   PGPPrefIndex prefIndex);

PGPError PGPclDisposePrefData(PGPPrefRef prefRef, 
							void *dataBuffer);

//	Close, and optionally save changes to, open client preference
//  file.  
//
//	Entry parameters :
//		PrefRef	- PrefRef of file to close
//		bSave	- TRUE => save changes before closing

PGPError PGPclExport 
PGPclCloseClientPrefs (
		PGPPrefRef	PrefRef, 
		BOOL		bSave);

//	Open admin preferences file and returns PrefRef to caller
//
//	Entry parameters :
//		memMgr			- memory manager to user for allocating fileref
//		pPrefRef		- pointer to buffer to receive PrefRef
//		bLoadDefaults	- TRUE=>if file doesn't exist, then create
//						  and load default values
//						  FALSE=>if file doesn't exist, return error

PGPError PGPclExport
PGPclOpenAdminPrefs (
		PGPMemoryMgrRef	memMgr,
		PGPPrefRef*		pPrefRef, 
		BOOL			bLoadDefaults);

//	Close, and optionally save changes to, open admin preference
//  file.  
//
//	Entry parameters :
//		PrefRef			- PrefRef of file to close
//		bSave			- TRUE => save changes before closing

PGPError PGPclExport 
PGPclCloseAdminPrefs (
		PGPPrefRef	PrefRef, 
		BOOL		bSave);

//	Get root CA key and sig that are specified by client prefs
//
//	Entry parameters :
//		prefref			- valid prefref for client prefs
//		keyset			- main keyset on which root CA key is located
//		pkeyCert		- pointer to buffer to receive key
//		psigCert		- pointer to buffer to receive sig

PGPError PGPclExport
PGPclGetRootCACertPrefs (
		PGPContextRef	context,
		PGPPrefRef		prefref, 
		PGPKeySetRef	keyset,
		PGPKeyRef*		pkeyCert,
		PGPSigRef*		psigCert);

//	_______________________________________________________
//
//	Miscellaneous convenience functions 

//	Determines if we are running with Admin priviledges
//	Note: returns TRUE if not running under NT

BOOL PGPclExport
PGPclLoggedInAsAdministrator (VOID);

//	Fills buffer with the path of the current PGP install.
//
//	Entry parameters :
//		szPath		buffer to receive string
//		uLen		length of buffer in bytes
	
PGPError PGPclExport 
PGPclGetPGPPath (
		LPSTR	szPath, 
		UINT	uLen);

//	Launch web browser and send to specified page
//
//	Entry parameters :
//		pszURL			- URL to open
//

PGPError PGPclExport 
PGPclWebBrowse (LPSTR pszURL);

//	Sync keysets to resolve trust info discrepancies
//
//	Entry parameters :
//		context		- context ref
//		keysetMain	- main keyset containing trust info
//		keysetNew	- newly-imported keyset to sync with main

PGPError PGPclExport 
PGPclSyncKeySets (
		PGPContextRef	context,
		PGPKeySetRef	keysetMain,
		PGPKeySetRef	keysetNew);

//	Convert info in SYSTEMTIME structure to number of days from today
//
//	Entry parameters :
//		pst			address of SYSTEMTIME structure containing data
//					to convert
//		piDays		address of buffer to receive number of days from
//					today.  Negative numbers are prior to today.

PGPError PGPclExport
PGPclSystemTimeToDays (
		SYSTEMTIME*	pst, 
		INT*		piDays);

//	Check whether SDK supports specified Public Key algorithm
//
//	Entry parameters :
//		PGPPublicKeyAlgorithm	SDK public key algorithm constant
//		mustEncrypt				TRUE=>SDK must support encryption with
//									this algorithm
//		mustSign				TRUE=>SDK must support signing with
//									this algorithm
//
//	returns kPGPError_NoErr if SDK supports operations with
//	specified algorithm, kPGPError_FeatureNotAvailable otherwise.

PGPError PGPclExport 
PGPclCheckSDKSupportForPKAlg (
		PGPPublicKeyAlgorithm	alg,
		PGPBoolean				mustEncrypt,
		PGPBoolean				mustSign);

//	Check whether SDK supports specified cipher algorithm
//
//	Entry parameters :
//		PGPCipherAlgorithm		SDK cipher algorithm constant
//
//	returns kPGPError_NoErr if SDK supports operations with
//	specified algorithm, kPGPError_FeatureNotAvailable otherwise.

PGPError PGPclExport 
PGPclCheckSDKSupportForCipherAlg (PGPCipherAlgorithm alg);

//	Broadcast message that indicates that the keyring
//	has been changed and that others should reload from
//	disk.
//
//	Entry parameters :
//		lParam		- 32 value which is passed along as the LPARAM
//					  of the broadcast message.  Current usage
//					  is to set this to your process ID or your
//					  window handle so that you can ignore 
//					  your own messages, if you want.  Set to
//					  zero to ensure all recipients process message.

//	broadcast message used to inform others of keyring changes
#define RELOADKEYRINGMSG	("PGPM_RELOADKEYRING")

VOID PGPclExport 
PGPclNotifyKeyringChanges (LPARAM lParam);

//	Broadcast message that indicates that the keyserver prefs
//	have been changed and that others should reload from
//	disk.
//
//	Entry parameters :
//		lParam		- 32 value which is passed along as the LPARAM
//					  of the broadcast message.  Current usage
//					  is to set this to your process ID or your
//					  window handle so that you can ignore 
//					  your own messages, if you want.  Set to
//					  zero to ensure all recipients process message.

//	broadcast message used to inform others of keyring changes
#define RELOADPREFSMSG	("PGPM_RELOADPREFS")

VOID PGPclExport 
PGPclNotifyPrefsChanges (LPARAM lParam);

//	Broadcast message that indicates that the prefs
//	have been changed and that others should reload from
//	disk.
//
//	Entry parameters :
//		lParam		- 32 value which is passed along as the LPARAM
//					  of the broadcast message.  Current usage
//					  is to set this to your process ID or your
//					  window handle so that you can ignore 
//					  your own messages, if you want.  Set to
//					  zero to ensure all recipients process message.

//	broadcast message used to inform others of keyring changes
#define RELOADKEYSERVERPREFSMSG	("PGPM_RELOADKEYSERVERPREFS")

VOID PGPclExport 
PGPclNotifyKeyserverPrefsChanges (LPARAM lParam);

//	Copy user info strings to preferences file
//
//	Entry parameters :
//		szOwnerName			owner name
//		szCompanyName		company name
//		szLicenseNumber		license number
	
PGPError PGPclExport 
PGPclSetUserInfo (
		LPSTR szOwnerName,
		LPSTR szCompanyName,
		LPSTR szLicenseNumber);

//	Query SDK for keyring and randseed file paths
//
//	Entry parameters :
//		pszPubRingPath		buffer to receive public keyring file name
//		iPubRingLen			length of buffer in bytes
//		pszPrivRingPath		buffer to receive private keyring file name
//		iPrivRingLen		length of buffer in bytes
//		pszRandSeedPath		buffer to receive random seed file name
//		iRandSeedLen		length of buffer in bytes
//
//	Note: any or all buffer pointers can be NULL -- they will be ignored.

PGPError PGPclExport 
PGPclGetSDKFilePaths (
		LPSTR	pszPubRingPath,
		INT		iPubRingLen,
		LPSTR	pszPrivRingPath,
		INT		iPrivRingLen,
		LPSTR	pszRandSeedPath,
		INT		iRandSeedLen);

//	Use SDK to set keyring and randseed file paths
//
//	Entry parameters :
//		pszPubRingPath		buffer containing public keyring file name
//		pszPrivRingPath		buffer containing private keyring file name
//		pszRandSeedPath		buffer containing random seed file name
//		bForceCreate		TRUE => call PGPOpenDefaultKeyRings with "Create"
//									flag to force creation of files
//							FALSE => do not call PGPOpenDefaultKeyRings
//
//	Note: any or all buffer pointers can be NULL -- they will be ignored.

PGPError PGPclExport 
PGPclSetSDKFilePaths (
		LPSTR	pszPubRingPath,
		LPSTR	pszPrivRingPath,
		LPSTR	pszRandSeedPath,
		BOOL	bForceCreate);

//	Startup the WinNT memlocking driver.  If called under Win95, just
//	returns kPGPError_NoErr.  If called under WinNT, returns 
//  kPGPError_NoErr if driver already started, or driver is successfully
//  started.  Returns kPGPError_UnknownError if driver cannot be started.

PGPError PGPclExport
PGPclStartMemLockDriver (VOID);

//	Get key from Key ID string
//
//	Entry parameters :
//		context		- context ref
//		keyset		- keyset to find key in
//		szID		- string representation of key ID
//		alg			- algorithm of key 
//		Key			- buffer to receive keyref

PGPError PGPclExport 
PGPclGetKeyFromKeyID (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		LPSTR			szID,
		UINT			uAlg,
		PGPKeyRef*		pkey);

//	_______________________________________________________
//
//	Keyserver convenience functions 


#define PGPCL_SPECIFIEDSERVER   0
#define PGPCL_USERIDBASEDSERVER	1	// determine server based on userid
#define	PGPCL_DEFAULTSERVER		2	// use default keyserver
#define PGPCL_ROOTSERVER		3	// use root keyserver

//	Searches keyserver prefs for specifed keyserver and replaces
//	authentication key.  
//
//	Entry parameters :
//		keyserver					keyserver data to put in prefs file
	
PGPError PGPclExport 
PGPclSyncKeyserverPrefs (
		PGPContextRef		context,
		PGPKeyServerEntry*	keyserver);

//	Searches keyservers (as defined in prefs file) for all
//	keys in the specified keyset.  
//
//	Entry parameters :
//		hwndParent			parent window
//		keysetToUpdate		keyset with keys to update
//		uServer				one of above-defined server constants
//		keysetMain			keyset to which tls key is added
//		pkeysetUpdated		buffer to receive PGPKeySetRef of updated keys
	
PGPError PGPclExport 
PGPclUpdateKeySetFromServer (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPKeySetRef		keysetToUpdate,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetUpdated);

//	Searches keyservers (as defined in prefs file) for the
//	specified userid string.
//
//	Entry parameters :
//		hwndParent			parent window
//		szUserID			string containing userid info
//		uServer				one of above-defined server constants
//		keysetMain			keyset to which tls key is added
//		pkeysetFound		buffer to receive PGPKeySetRef of found keys
	
PGPError PGPclExport 
PGPclSearchServerForUserID (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		LPSTR				szUserID,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetFound);

//	Searches default keyserver for the keyids in the list.
//
//	Entry parameters :
//		hwndParent			parent window
//		pkeyidList			array of PGPKeyID
//		iNumKeyIDs			number of PGPKeyIDs in list
//		uServer				one of above-defined server constants
//		keysetMain			keyset to which tls key is added
//		pkeysetFound		buffer to receive PGPKeySetRef of found keys
	
PGPError PGPclExport 
PGPclSearchServerForKeyIDs (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPKeyID*			pkeyidList,
		INT					iNumKeyIDs,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetFound);

//	Searches default keyserver using the specified filter.
//
//	Entry parameters :
//		hwndParent			parent window
//		filter				filter to use for search
//		uServer				one of above-defined server constants
//		keysetMain			keyset to which tls key is added
//		pkeysetFound		buffer to receive PGPKeySetRef of found keys
	
PGPError PGPclExport 
PGPclSearchServerWithFilter (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPFilterRef		filter,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetFound);

//	Sends keys in keyset to the specified keyserver
//
//	Entry parameters :
//		hwndParent			parent window
//		szServerURL			server to send to
//		uServer				one of above-defined server constants
//							or zero to use pkeyserver
//		pkeyserver			server to send to (if uServer == 0)
//		keysetMain			keyset to which tls key is added and 
//								which contains signing keys
//		keysetToSend		keyset with keys to send
	
PGPError PGPclExport 
PGPclSendKeysToServer (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		UINT				uServer,
		PGPKeyServerEntry*	pkeyserver,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef		keysetToSend);

//	Sends keys in keyset to the root keyserver with notifications
//
//	Entry parameters :
//		hWndToReceiveNotifications	window to get WM_NOTIFYs
//		szServerURL					server to send to
//		keysetMain					keyset to which tls key is added and 
//										which contains signing keys
//		keysetToSend				keyset with keys to send
//
//	returns kPGPError_NoErr.  Progress notifications and final
//	results are sent to window in form of WM_NOTIFY messages
	
//  keyserver notifications
#define PGPCL_SERVERDONE			0x0001
#define PGPCL_SERVERPROGRESS		0x0002
#define PGPCL_SERVERABORT			0x0003
#define PGPCL_SERVERERROR			0x0004

typedef struct _PGPclSERVEREVENT
{
	NMHDR		nmhdr;
	VOID*		pData;
	BOOL		cancel;
	LONG		step;
	LONG		total;
	CHAR		szmessage[256];
} PGPclSERVEREVENT, *PPGPclSERVEREVENT;

#define PGPCL_SERVERINFINITE		-1L

PGPError PGPclExport 
PGPclSendKeysToRootServerNotify (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hWndToReceiveNotifications, 
		PGPKeySetRef		keysetMain,
		PGPKeySetRef		keysetToSend);

//	Deletes keys in keyset from the specified keyserver
//
//	Entry parameters :
//		hwndParent			parent window
//		szServerURL			server to send to
//		space				pending or active bucket
//		keysetMain			keyset to which tls key is added and 
//								which contains signing keys
//		keysetToDelete		keyset with keys to delete
	
PGPError PGPclExport 
PGPclDeleteKeysFromServer (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPKeyServerEntry*	pkeyserver,
		INT					space,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef		keysetToDelete);

//	Disables keys in keyset on the specified keyserver
//
//	Entry parameters :
//		hwndParent			parent window
//		szServerURL			server to send to
//		space				pending or active bucket
//		keysetMain			keyset to which tls key is added and 
//								which contains signing keys
//		keysetToDisable		keyset with keys to disable
	
PGPError PGPclExport 
PGPclDisableKeysOnServer (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPKeyServerEntry*	pkeyserver,
		INT					space,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef		keysetToDisable);

//	Downloads a new groupset from the root server
//
//	Entry parameters :
//		hwndParent			parent window
//		keysetMain			keyset to which tls key is added and 
//								which contains signing keys
//		groupsetDownloaded	newly downloaded groupset
	
PGPError PGPclExport 
PGPclGetGroupsFromRootServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPGroupSetRef*			groupsetDownloaded);

//	Downloads a new groupset from the root server
//
//	Entry parameters :
//		hwndParent			parent window
//		keysetMain			keyset to which tls key is added and 
//								which contains signing keys
//		groupsetToSend		groupset to send to root server
	
PGPError PGPclExport 
PGPclSendGroupsToRootServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPGroupSetRef			groupsetToSend);

//	Create and send certificate request to CA server
//
//	Entry parameters :
//		hwndParent			parent window
//		keysetMain			main keyset
//		userid				userid for which request will be made
//		keysetKey			keyset containing single key to request
//								certificate for
	
PGPError PGPclExport 
PGPclSendCertificateRequestToServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPUserIDRef			userid,
		PGPKeySetRef			keysetKey);

//	Create and send certificate request to CA server with notifications
//
//	Entry parameters :
//		hwndToNotify		window to receive progress notifications
//		keysetMain			main keyset
//		userid				userid for which request will be made
//		keysetKey			keyset containing single key to request
//								certificate for
//		pszPassPhrase		passphrase of key in keysetKey
	
PGPError PGPclExport 
PGPclSendCertificateRequestToServerNotify (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndToNotify, 
		PGPKeySetRef			keysetMain,
		PGPUserIDRef			userid,
		PGPKeySetRef			keysetKey,
		LPSTR					pszPassPhrase);

//	Get the previously-requested certificate from the CA server
//
//	Entry parameters :
//		hwndParent			parent window
//		keysetKey			keyset containing single key for whic request
//								was previously made
//		userid				userid to retrieve certificate for
//		pkeysetCert			buffer to receive PGPKeySetRef of found cert
	
PGPError PGPclExport 
PGPclRetrieveCertificateFromServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPKeySetRef			keysetKey,
		PGPUserIDRef			userid,
		PGPKeySetRef*			pkeysetCert);

//	Create and send certificate request to CA server
//
//	Entry parameters :
//		hwndParent			parent window
//		keysetMain			main keyset containing CA root key
	
PGPError PGPclExport 
PGPclGetCertificateRevocationsFromServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain);

//	Create certificate request AV list
//
//	Entry parameters :
//		hwnd				parent window
//		bForceDlg			TRUE=>forces dialog to appear
//		userid				userid of key for which cert req is made 
//								or kInvalidPGPUserID
//		serverclass			class of CA server or kPGPKeyServerClass_Invalid
//		pAVlist				receives pointer to AVlist
//		pNumAVs				receives number of AVs in list

PGPError PGPclExport
PGPclGetCACertRequestAVList (
		HWND					hwnd,
		PGPContextRef			context,
		PGPBoolean				bForceDlg,
		PGPUserIDRef			userid,
		PGPKeyServerClass		serverclass,
		PGPAttributeValue**		ppAVlist,
		PGPUInt32*				pNumAVs);

//	Free AV list previously returned by PGPclGetCACertRequestAVList
//
//	Entry parameters :
//		pAVlist				pointer to AVlist
//		NumAVs				number of AVs in list

PGPError PGPclExport
PGPclFreeCACertRequestAVList (
		PGPAttributeValue*		pAVlist,
		PGPUInt32				NumAVs);

//	return a string description of a given attribute type
//
//	Entry parameters :
//		attr				attribute
//		psz					string buffer to fill
//		uLen				size of string buffer
//
//	Note: returns kPGPError_ItemNotFound if no string available for attribute

PGPError PGPclExport
PGPclGetAVListAttributeString (
		PGPAVAttribute		attr,
		LPSTR				psz,
		UINT				uLen);

//	_______________________________________________________
//
//	Split key sharing functions 

//	Send key share file to remote computer
//
//	Entry parameters :
//		context		- PGP library context
//		hwndParent	- handle of parent window
//		keysetMain	- main keyset (should contain decryption
//						and authentication keys)

PGPError PGPclExport
PGPclSendShares (
		PGPContextRef		context, 
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent,
		PGPKeySetRef		keysetMain);


//	Reconstitute key by collecting key shares
//
//	Entry parameters :
//		context		- PGP library context
//		hwndParent	- handle of parent window
//		keysetMain	- main keyset (should contain decryption
//						and authentication keys)
//		key			- key to reconstitute
//		ppPasskeyBuffer		- pointer to buffer to receive address 
//							  of passkey buffer
//		piPasskeyLength		- pointer to buffer to receive length

PGPError PGPclExport
PGPclReconstituteKey (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent,
		PGPKeySetRef		keysetMain,
		PGPKeyRef			key,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength);


//	Allow user to confirm remote authentication key
//
//	Entry parameters :
//		hwndParent		- handle of parent window
//		pszServer		- string with name of remote host
//		keyAuth			- authentication key to confirm
//		tlsCipher		- cipher suite number
//		keysetMain		- main keyset to which keyAuth may be added
//		uFlags			- flags controlling text display
//
//	returns kPGPError_NoErr or kPGPError_UserAbort

#define PGPCL_SHOWAUTHENTICATION	0x0000
#define	PGPCL_AUTHRECONSTITUTING	0x0001
#define PGPCL_AUTHNEWKEY			0x0002
#define PGPCL_AUTHUNEXPECTEDKEY		0x0003
#define PGPCL_AUTHEXPECTEDKEY		0x0004

PGPError PGPclExport 
PGPclConfirmRemoteAuthentication (
		PGPContextRef			context,
		HWND					hwndParent, 
		LPSTR					pszServer,
		PGPKeyRef				keyAuth,
		PGPtlsCipherSuiteNum	tlsCipher,
		PGPKeySetRef			keysetMain,
		UINT					uFlags);

//	_______________________________________________________
//
//	Passphrase functions 

//	Get passphrase from user.
//
//	Entry parameters :
//		context		- PGP library context
//		hWndParent	- handle of parent window
//		szPrompt	- message string to be displayed to user
//		pszPhrase	- pointer to receive address of buffer which
//					  will contain passphrase.  The caller should
//					  deallocate this buffer by calling PGPclFreePhrase 
//		KeySet		- KeySet containing keys to display in combo or list box
//					  NULL => hide key selection combo box
//		pKeyIDs		- additional keyids to tag onto end of listbox in 
//					  decryption dialog
//		uKeyCount	- total number of keys to display (only necessary when
//					  keyids are to be displayed, otherwise can be zero)
//		pKey		- pointer to buffer to receive ref to selected key. 
//					  if buffer contains key on entry, this will be default.
//					  buffer should be set to NULL to use keyring default key.
//					  NULL => hide key selection combo box
//		uOptions	- pointer to buffer which contains and will receive 
//					  options bits
//					  NULL => hide option checkboxes
//		uFlags		- flag bits
//					  PGPcl_RSAENCRYPT => encrypting to RSA key
//					  PGPcl_DECRYPTION => display decryption dialog
//					  PGPcl_ENCRYPTION => display conventional encryption
//											  dialog
//
//	This function returns kPGPError_UserAbort if the user
//  presses the cancel button.  

// options bits
#define PGPCL_ASCIIARMOR			0x0001
#define PGPCL_DETACHEDSIG			0x0002
#define PGPCL_PASSONLY				0x0004
#define PGPCL_WIPEORIG				0x0008
#define PGPCL_FYEO					0x0010
#define PGPCL_SDA					0x0020

// disable bits
#define PGPCL_DISABLE_ASCIIARMOR    0x0001
#define PGPCL_DISABLE_AUTOMODE		0x0002
#define PGPCL_DISABLE_PASSONLY		0x0004
#define PGPCL_DISABLE_WIPEORIG		0x0008
#define PGPCL_DISABLE_FYEO			0x0010
#define PGPCL_DISABLE_SDA			0x0020

// flag bits
#define PGPCL_RSAENCRYPT			0x0001
#define PGPCL_DECRYPTION			0x0002
#define PGPCL_ENCRYPTION			0x0004
#define PGPCL_KEYPASSPHRASE         0x0008
#define PGPCL_REJECTSPLITKEYS		0x0010

PGPError PGPclExport 
PGPclGetPhrase (
		PGPContextRef	context,
		PGPKeySetRef	MainKeySet,
		HWND			hWndParent, 
		LPSTR			szPrompt,
		LPSTR*			ppszPhrase, 
		PGPKeySetRef	KeySet,
		PGPKeyID*		pKeyIDs,
		UINT			uKeyCount,
		PGPKeyRef*		pKey, 
		UINT*			puOptions, 
		UINT			uFlags,
		PGPByte**		ppPasskeyBuffer,
		PGPUInt32*		piPasskeyLength,
		PGPUInt32		MinLength,
		PGPUInt32		MinQuality,
		PGPtlsContextRef tlsContext,
		PGPKeySetRef	*AddedKeys,
		char			*szTitle
		) ;

//	Recipient dialog glue call
//
//  Interfaces to PGPsdkUI calls.

#define CurrentPGPrecipVersion 'DUKE'

typedef struct _recipientdialogstruct
{
	DWORD			Version;
	HWND			hwndParent;
	char *			szTitle;
	PGPContextRef	Context;
	PGPKeySetRef 	OriginalKeySetRef;
	PGPKeySetRef 	SelectedKeySetRef;
	char **			szRecipientArray;
	DWORD			dwNumRecipients;
	DWORD			dwOptions;
	DWORD			dwFlags;
	DWORD			dwDisableFlags;
	PGPtlsContextRef tlsContext;
	PGPKeySetRef	AddedKeys;
} RECIPIENTDIALOGSTRUCT, *PRECIPIENTDIALOGSTRUCT;

UINT PGPclExport PGPclRecipientDialog(PRECIPIENTDIALOGSTRUCT prds);

//	Wipe and deallocate phrase buffer.
//
//	Entry parameters :
//		szPhrase	- passphrase buffer to wipe and deallocate
//

VOID PGPclExport 
PGPclFreePhrase (LPSTR szPhrase);

//___________________________________
//
//	passphrase caching routines

//
//	PGPclGetCachedDecryptionPhrase
//	This routine is called to get either cached phrase 
//	(if available) or prompt user for phrase.

PGPError PGPclExport
PGPclGetCachedDecryptionPhrase (
		PGPContextRef		context, 
		PGPtlsContextRef	tlsContext,
		PGPKeySetRef		keysetMain,
		HWND				hwnd, 
		LPSTR				szPrompt, 
		BOOL				bForceUserInput,
		LPSTR*				pszBuffer,
		PGPKeySetRef		keysetEncryptedTo,
		PGPKeyID*			pkeyidEncryptedTo,
		UINT				uKeyIDCount,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength,
		PGPKeySetRef*		pkeysetAdded,
		char *				szTitle); 

//
//	PGPclGetCachedSigningPhrase
//	This routine is called to get either signing cached phrase 
//	(if available) or prompt user for phrase.

PGPError PGPclExport
PGPclGetCachedSigningPhrase (
		PGPContextRef		context, 
		PGPtlsContextRef	tlsContext,
		HWND				hwnd,
		LPSTR				szPrompt,
		BOOL				bForceUserInput, 
		LPSTR*				pszBuffer, 
		PGPKeySetRef		keysetSigning,
		PGPKeyRef*			pkeySigning,
		PGPHashAlgorithm*	pulHashAlg,
		UINT*				puOptions, 
		UINT				uFlags,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength,
		PGPKeySetRef*		pkeysetAdded,
		char *				szTitle);

#define PGPCL_DECRYPTIONCACHE	0x0001
#define PGPCL_SIGNINGCACHE		0x0002

//
//	PGPclPurgeCachedPassphrase
//	Called to purge phrases from cache.  
//	dwFlags is the logical OR of PGPCL_DECRYPTIONCACHE and
//	PGPCL_SIGNINGCACHE.

BOOL PGPclExport
PGPclPurgeCachedPassphrase (DWORD dwFlags); 

//
//	PGPclFreeCachedPhrase
//	Called to wipe and free the phrase returned by
//	PGPGetCachedPhrase.

VOID PGPclExport
PGPclFreeCachedPhrase (LPSTR szPhrase); 

//
//	PGPclQueryDecryptCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPclExport 
PGPclQueryDecryptionCacheSecsRemaining (VOID); 

//
//	PGPclQuerySignCacheSecsRemaining
//	Returns number of seconds remaining until cache expires

INT PGPclExport
PGPclQuerySigningCacheSecsRemaining (VOID); 


//	Broadcast message that indicates that the receiving module
//	should purge its passphrase cache(s)
//
//	Entry parameters :
//		wParam		- logical OR of PGPCL_DECRYPTIONCACHE and
//					  PGPCL_SIGNINGCACHE.
//		lParam		- 32 value which is passed along as the LPARAM
//					  of the broadcast message.  Current usage
//					  is to set this to your process ID or your
//					  window handle so that you can ignore 
//					  your own messages, if you want.  Set to
//					  zero to ensure all recipients process message.

//	broadcast message used to inform others of keyring changes
#define PURGEPASSPHRASECACEHMSG	("PGPM_PURGEPASSPHRASECACHE")

VOID PGPclExport 
PGPclNotifyPurgePassphraseCache (
		WPARAM wParam,
		LPARAM lParam);

//	__________________________________________________________
//
//  Check to see if it's time to auto-update keys
//
//	Entry parameters :
//		memoryMgr					Memory manager ref
//		bResetDates					TRUE=>set "last updated" date to today
//		pbUpdateAllKeys				Pointer to boolean, TRUE if it's
//									time to update all keys
//		pbUpdateTrustedIntroducers	Pointer to boolean, TRUE if it's
//									time to update trusted introducers
//		pbUpdateCRL					Pointer to boolean, TRUE if it's 
//									time to update CA CRLs

PGPError PGPclExport
PGPclCheckAutoUpdate(PGPMemoryMgrRef memoryMgr, 
					 PGPBoolean  bResetDates,
					 PGPBoolean* pbUpdateAllKeys,
					 PGPBoolean* pbUpdateTrustedIntroducers,
					 PGPBoolean* pbUpdateCRL);

//	__________________________________________________________
//
//  Look up an unknown signer's key on the keyserver
//
//	Entry parameters :
//		context			context ref
//		hwnd			handle to parent window
//		event			event data pointer
//		signingKeyID	Key ID of unknown signer
//		pbGotKeys		Pointer to boolean, TRUE if the key(s)
//						of the unknown signer were successfully fetched

PGPError PGPclExport
PGPclLookupUnknownSigner(PGPContextRef context,
						 PGPKeySetRef KeySetMain,
						 PGPtlsContextRef tlsContext,
						 HWND hwnd,
						 PGPEvent *event,
						 PGPKeyID signingKeyID,
						 PGPBoolean *pbGotKeys);
//	_______________________________________________________

#ifdef __cplusplus
}
#endif

#endif /* ] Included_PGPcl_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
