/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPcomdlg.h - header file for PGP common dialog DLL
	

	$Id: PGPcmdlg.h,v 1.43 1997/10/28 21:05:19 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPcmdlg_h	/* [ */
#define Included_PGPcmdlg_h

#include "pflPrefTypes.h"
#include "..\..\shared\pgpGroups.h" //wjb

#ifdef _PGPCDDLL
# define PGPcdExport __declspec( dllexport )
#else
# define PGPcdExport __declspec( dllimport )
#endif

typedef struct _PGPcdGROUPFILE
{
	PFLContextRef	pflcontext;
	PFLFileSpecRef	pflfilespec;
	PGPGroupSetRef	groupset;
} PGPcdGROUPFILE;

typedef struct _PGPcdSERVEREVENT
{
	NMHDR		nmhdr;
	VOID*		pData;
	BOOL		cancel;
	LONG		step;
	LONG		total;
	CHAR		szmessage[256];
} PGPcdSERVEREVENT, *PPGPcdSERVEREVENT;

// keyserver notifications
#define PGPCOMDLG_SERVERDONE			0x0001
#define PGPCOMDLG_SERVERPROGRESS		0x0002
#define PGPCOMDLG_SERVERABORT			0x0003
#define PGPCOMDLG_SERVERERROR			0x0004

#define PGPCOMDLG_SERVERINFINITE		-1L

// options bits
#define PGPCOMDLG_ASCIIARMOR			0x0001
#define PGPCOMDLG_DETACHEDSIG			0x0002
#define PGPCOMDLG_PASSONLY				0x0004
#define PGPCOMDLG_WIPEORIG				0x0008

// disable bits
#define PGPCOMDLG_DISABLE_ASCIIARMOR    0x0001
#define PGPCOMDLG_DISABLE_AUTOMODE		0x0002
#define PGPCOMDLG_DISABLE_PASSONLY		0x0004
#define PGPCOMDLG_DISABLE_WIPEORIG		0x0008

// flag bits
#define PGPCOMDLG_RSAENCRYPT			0x0001
#define PGPCOMDLG_DECRYPTION			0x0002
#define PGPCOMDLG_ENCRYPTION			0x0004

// preferences pages
#define PGPCOMDLG_GENERALPREFS			0
#define PGPCOMDLG_KEYRINGPREFS			1
#define PGPCOMDLG_EMAILPREFS			2
#define PGPCOMDLG_KEYSERVERPREFS		3
#define PGPCOMDLG_ADVANCEDPREFS			4

// eval expire codes
#define PGPCOMDLG_ENCRYPTSIGNEXPIRED	1
#define PGPCOMDLG_ALLEXPIRED			2

#define PGPCOMDLG_DISPLAY_TIME			0L
#define PGPCOMDLG_DISPLAY_DATE			1L
#define PGPCOMDLG_DISPLAY_DURATION		2L

// broadcast message used to inform others of keyring changes
#define RELOADKEYRINGMSG	("PGPM_RELOADKEYRING")


#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------|
//	PGPcomdlgWrapBuffer -
//	Wrap text using platform independent word wrap code.
//	Output buffer must be freed with PGPcomdlgFreeWrapBuffer
//
//	Entry parameters :
//		szInText		- input buffer
//		wrapColumn		- column at which to wrap
//		pszOutText		- buffer to receive pointer to
//						  output buffer containing wrapped text
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgWrapBuffer (LPSTR szInText,
					 PGPUInt16 wrapColumn,
					 LPSTR* pszOutText);

//-------------------------------------------------|
//	PGPcomdlgFreeWrapBuffer -
//	Free previously-wrapped text buffer.
//
//	Entry parameters :
//		textBuffer		- buffer to free
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgFreeWrapBuffer (LPSTR textBuffer);

//-------------------------------------------------|
//	PGPcomdlgCloseLibrary -
//	Close down DLL and purge passphrase buffers (only
//  those used to sign disable/delete on server).
//
//	Entry parameters :
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgCloseLibrary (VOID);


//-------------------------------------------------|
//	PGPcomdlgOpenClientPreferences -
//	Open client preferences file and returns PrefRef to caller
//
//	Entry parameters :
//		pPrefRef	- pointer to buffer to receive PrefRef
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgOpenClientPrefs (PGPPrefRef* pPrefRef);


//-------------------------------------------------|
//	PGPcomdlgOpenAdminPreferences -
//	Open admin preferences file and returns PrefRef to caller
//
//	Entry parameters :
//		pPrefRef		- pointer to buffer to receive PrefRef
//		bLoadDefaults	- TRUE=>if file doesn't exist, then create
//						  and load default values
//						  FALSE=>if file doesn't exist, return error
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport
PGPcomdlgOpenAdminPrefs (PGPPrefRef* pPrefRef, 
							   BOOL bLoadDefaults);


//-------------------------------------------------|
//	PGPcomdlgCloseClientPrefs -
//	Closes, and optionally saves changes to, open preference
//  file.  This routine is used to close both client and admin
//  preference files. 
//
//	Entry parameters :
//		PrefRef	- PrefRef of file to close
//		bSave	- TRUE => save changes before closing
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgCloseClientPrefs (PGPPrefRef PrefRef, 
						   BOOL bSave);


//-------------------------------------------------|
//	PGPcomdlgCloseAdminPrefs -
//	Closes, and optionally saves changes to, open preference
//  file.  This routine is used to close both client and admin
//  preference files. 
//
//	Entry parameters :
//		PrefRef			- PrefRef of file to close
//		bSave			- TRUE => save changes before closing
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgCloseAdminPrefs (PGPPrefRef PrefRef, 
						  BOOL bSave);


//-------------------------------------------------|
//	PGPcomdlgGetKeyIDFromKey -
//	Get Key ID string from key
//
//	Entry parameters :
//		Key			- PGPKeyRef of key for which to get ID
//		bFull		- TRUE => get full (64bit) keyID
//		szID		- string buffer to receive keyID (must be at
//					  least 11 bytes long for "standard", 19 bytes
//					  for "full" keyID)
//		uLen		- length of szID in bytes
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgGetKeyIDFromKey (PGPKeyRef Key,
						  BOOL bFull,
						  LPSTR szID, 
						  UINT uLen); 

//-------------------------------------------------|
//	PGPcomdlgGetKeyFromKeyID -
//	Get key from Key ID string
//
//	Entry parameters :
//		context		- context ref
//		keyset		- keyset to find key in
//		szID		- string representation of key ID
//		alg			- algorithm of key 
//		Key			- buffer to receive keyref
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgGetKeyFromKeyID (PGPContextRef context,
						  PGPKeySetRef keyset,
						  LPSTR szID,
						  UINT alg,
						  PGPKeyRef* Key);

//-------------------------------------------------|
//	PGPcomdlgSyncKeySets -
//	sync keyset properties to resolve SDK problems
//
//	Entry parameters :
//		context		- context ref
//		keysetMain	- main keyset containing trust info
//		keysetNew	- new imported keyset to sync up
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgSyncKeySets (
		PGPContextRef context,
		PGPKeySetRef keysetMain,
		PGPKeySetRef keysetNew);

//-------------------------------------------------|
//	PGPcomdlgPassphraseQuality -
//	Calculates index of passphrase quality
//
//	Entry parameters :
//		szPhrase	- passphrase to be evaluated
//
//	Returns value in range [0,100] 
//

PGPError PGPcdExport
PGPcomdlgPassphraseQuality (LPSTR szPhrase);


//-------------------------------------------------|
//	PGPcomdlgHelpAbout -
//	Post Help About dialog with button for browser
//	launch to specified URL.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		szVersion	- string containing version information
//					  to be displayed
//					  NULL => use default version string
//		szLinkText	- text to be displayed on link button
//					  (should be <= ~20 chars),
//					  NULL => use default ("www.pgp.com")
//		szLink		- URL to which to jump
//					  NULL => use default ("http://www.pgp.com/")
//
//	Returns kPGPError_NoErr
//

PGPError PGPcdExport 
PGPcomdlgHelpAbout (HWND hWndParent, 
					LPSTR szVersion, 	
					LPSTR szLinkText, 
					LPSTR szLink);


//-------------------------------------------------|
//	PGPcomdlgHardSell -
//	Post hard sell dialog with button for browser
//	launch to PGP URL.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		pszText		- string containing text information
//					  to be displayed
//
//	Returns kPGPError_NoErr if no error
//

PGPError PGPcdExport 
PGPcomdlgHardSell (HWND hWndParent, 
				   LPSTR pszText); 	


//-------------------------------------------------|
//	PGPcomdlgNag -
//	Post nag dialog with button for browser
//	launch to specified URL.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		szLinkText	- text to be displayed on link button
//					  (should be <= ~20 chars),
//					  NULL => use default ("Order Now!")
//		szLink		- URL to which to jump
//					  NULL => use default ("http://www.pgp.com/")
//
//	Returns kPGPError_NoErr
//

PGPError PGPcdExport 
PGPcomdlgNag (HWND hWndParent, 
			  LPSTR szLinkText, 
			  LPSTR szLink);


//-------------------------------------------------|
//	PGPcomdlgCheckKeyRings -
//	check key rings for problems and query user if 
//	necessary.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		bForceQuery	- TRUE => always ask user 
//					  FALSE => only ask user if problems
//
//	Returns kPGPError_UserAbort if user presses the CANCEL
//	button.  Application should exit in this case.
//

PGPError PGPcdExport 
PGPcomdlgCheckKeyRings (PGPContextRef Context,
						HWND hWndParent, 
						BOOL bForceQuery);


//-------------------------------------------------|
//	PGPcomdlgPreferences -
//	post PGP preferences property sheets.
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
//					  use the above-defined constants
//
//	This function returns kPGPError_NoErr if all was OK or
//	kPGPError_UserAbort (if the user presses the cancel button)
//  or one of the above-defined error constants.
//

PGPError PGPcdExport 
PGPcomdlgPreferences (PGPContextRef Context, 
					  HWND hWndParent, 
					  INT startsheet);



//-------------------------------------------------|
//	PGPcomdlgRandom -
//	collect random data from keyboard/mouse.
//
//	Entry parameters :
//		Context		- PGP library context
//		hWndParent	- handle of parent window
//		uNeeded		- bits of entropy needed
//
//	This function returns either kPGPError_UserAbort (if the user
//  presses the cancel button) or kPGPError_NoErr or one 
//	of the above-defined error constants
//

PGPError PGPcdExport 
PGPcomdlgRandom (PGPContextRef Context,
				 HWND hWndParent, 
				 UINT uNeeded);


//-------------------------------------------------|
//	PGPcomdlgGetPhrase -
//	get passphrase from user.
//
//	Entry parameters :
//		context		- PGP library context
//		hWndParent	- handle of parent window
//		szPrompt	- message string to be displayed to user
//		pszPhrase	- pointer to receive address of buffer which
//					  will contain passphrase.  The caller should
//					  deallocate this buffer by calling PGPcomdlgFreePhrase 
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
//					  PGPCOMDLG_RSAENCRYPT => encrypting to RSA key
//					  PGPCOMDLG_DECRYPTION => display decryption dialog
//					  PGPCOMDLG_ENCRYPTION => display conventional encryption
//											  dialog
//
//	This function returns kPGPError_UserAbort if the user
//  presses the cancel button.  kPGPError_NoErr if no error.
//	Or one of the above-defined error constants.
//

PGPError PGPcdExport 
PGPcomdlgGetPhrase (
		PGPContextRef	context,
		HWND			hWndParent, 
		LPSTR			szPrompt,
		LPSTR*			pszPhrase, 
		PGPKeySetRef	KeySet,
		PGPKeyID*		pKeyIDs,
		UINT			uKeyCount,
		PGPKeyRef*		pKey, 
		UINT*			uOptions, 
		UINT			uFlags);

//-------------------------------------------------|
//	PGPcomdlgFreePhrase -
//	wipe and deallocate phrase buffer.
//
//	Entry parameters :
//		szPhrase	- passphrase buffer to wipe and deallocate
//

VOID PGPcdExport 
PGPcomdlgFreePhrase (LPSTR szPhrase);


//----------------------------------------------------|
//	get descriptive string for error code
//
//	Entry parameters :
//		iCode		- error code returned by pgp or simple library routine
//		szString	- buffer to be filled with descriptive error message
//		uLen		- length of buffer in bytes
//
//	This function evaluates iCode.  If iCode warrants an error
//	message, the buffer is filled with the message and the function
//	returns TRUE.  If no message is warranted, the function returns
//	FALSE.
//

BOOL PGPcdExport 
PGPcomdlgErrorToString (INT iCode, 
						LPSTR szString,  
						UINT uLen);


//----------------------------------------------------|
//	putup error messagebox (if necessary)
//
//	Entry parameters :
//		iCode		- error code returned by pgp or simple library routine
//
//	This function calls PGPcomdlgErrorToString to evaluate iCode.
//	If PGPcomdlgErrorToString returns TRUE, a message box is posted
//	containing the descriptive text.  Otherwise no messagebox is
//	displayed.  This function returns the value returned by 
//	PGPcomdlgErrorToString.
//

BOOL PGPcdExport 
PGPcomdlgErrorMessage (INT iCode);


//----------------------------------------------------|
//	Post Splash screen
//
//	Entry parameters :
//		Context			- PGP library context
//		hWndParent		- handle of parent window (if NULL,
//						  screen will not be dismissed until
//						  PGPcomdlgSetSplashParent is called 
//						  with a non-null value
//		uMilliseconds	- milliseconds to display splash
//

PGPError PGPcdExport 
PGPcomdlgSplash (PGPContextRef Context,
				 HWND hWndParent, 
				 UINT uMS);


//----------------------------------------------------|
//	Inform splash screen who its parent is
//
//	Entry parameters :
//		hWndParent		- handle of parent window
//

PGPError PGPcdExport 
PGPcomdlgSetSplashParent (HWND hWndParent);


//----------------------------------------------------|
//	Launch web browser and send to specified page
//
//	Entry parameters :
//		pszURL			- URL to open
//

PGPError PGPcdExport 
PGPcomdlgWebBrowse (LPSTR pszURL);

//----------------------------------------------------|
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
//

VOID PGPcdExport 
PGPcomdlgNotifyKeyringChanges (LPARAM lParam);


//----------------------------------------------------|
//	Returns TRUE if the beta or eval has expired, and displays a message box
//  Returns FALSE and does nothing if it's still valid
//
//	Entry parameters :
//		hwnd		Parent of window so we can show the MB.  
//					NULL is acceptable.
//

BOOL PGPcdExport 
PGPcomdlgIsExpired (HWND hwnd);


//-------------------------------------------------|
//	PGPcomdlgRSADSAMixWarning -
//	Post warning dialog 
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//
//	Returns kPGPError_NoErr, kPGPError_UserAbort, or PGPCOMDLG_NO
//

PGPError PGPcdExport 
PGPcomdlgRSADSAMixWarning (HWND hWndParent);


//----------------------------------------------------|
//	Returns TRUE if the evaluation has expired, and displays a message box
//  If the eval is partially expired, the nag screen will be displayed
//
//	Entry parameters :
//		hwnd		Parent of window so we can show the MB or nag screen.  
//					NULL is acceptable.
//		nIndex		Type of expiration to test: Encrypt/Sign or All

#if PGP_DEMO
	BOOL PGPcdExport PGPcomdlgEvalExpired (HWND hwnd, int nIndex);
#else
	#define	PGPcomdlgEvalExpired(x,y)	FALSE
#endif	// PGP_DEMO


//----------------------------------------------------|
//	Searches keyservers (as defined in prefs file) for all
//	keys in the specified keyset.  
//
//	Entry parameters :
//		hwndParent					parent window
//		keysetToUpdate				keyset with keys to update
//		pkeysetUpdated				buffer to receive PGPKeySetRef
//
//	returns kPGPError_NoErr.  
//
	
PGPError PGPcdExport 
PGPcomdlgUpdateKeySetFromServer (PGPContextRef context,
							   HWND hwndParent, 
							   PGPKeySetRef keysetToUpdate,
							   PGPKeySetRef* pkeysetUpdated);

//----------------------------------------------------|
//	Searches keyservers (as defined in prefs file) for the
//	specified userid string.
//
//	Entry parameters :
//		hwndParent					parent window
//		szUserID					string containing userid info
//		pkeysetFound				buffer to receive PGPKeySetRef
//
//	returns kPGPError_NoErr.  
//
	
PGPError PGPcdExport 
PGPcomdlgSearchServerForUserID (PGPContextRef context,
								HWND hwndParent, 
							    LPSTR szUserID,
								PGPKeySetRef* pkeysetFound);

//----------------------------------------------------|
//	Searches default keyserver for the keyids in the list.
//
//	Entry parameters :
//		hwndParent					parent window
//		pkeyidList					array of PGPKeyID
//		iNumKeyIDs					number of PGPKeyIDs in list
//		pkeysetFound				buffer to receive PGPKeySetRef
//
//	returns kPGPError_NoErr.  
//
	
PGPError PGPcdExport 
PGPcomdlgSearchServerForKeyIDs (PGPContextRef context,
							     HWND hwndParent, 
							     PGPKeyID* pkeyidList,
								 INT iNumKeyIDs,
								 PGPKeySetRef* pkeysetFound);

//----------------------------------------------------|
//	Sends keys in keyset to the specified keyserver
//
//	Entry parameters :
//		hwndParent					parent window
//		szServerURL					server to send to
//		keysetSigning				keyset with potential signing keys 
//										(used if signed request is required 
//										 by the server)
//		keysetToSend				keyset with keys to send
//
//	returns kPGPError_NoErr.  
//
	
PGPError PGPcdExport 
PGPcomdlgSendKeysToServer (PGPContextRef context,
						   HWND hwndParent, 
						   LPSTR szServerURL,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToSend);

//----------------------------------------------------|
//	Deletes keys in keyset from the specified keyserver
//
//	Entry parameters :
//		hwndParent					parent window
//		szServerURL					server to send to
//		space						pending or active bucket
//		keysetSigning				keyset with potential signing keys 
//										(used if signed request is required 
//										 by the server)
//		keysetToDelete				keyset with keys to delete
//
//	returns kPGPError_NoErr.  
//
	
PGPError PGPcdExport 
PGPcomdlgDeleteKeysFromServer (PGPContextRef context,
						   HWND hwndParent, 
						   LPSTR szServerURL,
						   INT space,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToDelete);

//----------------------------------------------------|
//	Disables keys in keyset on the specified keyserver
//
//	Entry parameters :
//		hwndParent					parent window
//		szServerURL					server to send to
//		space						pending or active bucket
//		keysetSigning				keyset with potential signing keys 
//										(used if signed request is required 
//										 by the server)
//		keysetToDisable				keyset with keys to disable
//
//	returns kPGPError_NoErr.  
//
	
PGPError PGPcdExport 
PGPcomdlgDisableKeysOnServer (PGPContextRef context,
						   HWND hwndParent, 
						   LPSTR szServerURL,
						   INT space,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToDisable);

//----------------------------------------------------|
//	Sends keys in keyset to the specified keyserver with
//	notifications
//
//	Entry parameters :
//		hWndToReceiveNotifications	window to get WM_NOTIFYs
//		szServerURL					server to send to
//		keysetSigning				keyset with potential signing keys 
//										(used if signed request is required 
//										 by the server)
//		keysetToSend				keyset with keys to send
//
//	returns kPGPError_NoErr.  Progress notifications and final
//	results are sent to window in form of WM_NOTIFY messages
//
	
PGPError PGPcdExport 
PGPcomdlgSendKeysToServerNotify (PGPContextRef context,
						   HWND hWndToReceiveNotifications, 
						   LPSTR szServerURL,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToSend);

//----------------------------------------------------|
//	Fills buffer with the path of the current PGP install.
//
//	Entry parameters :
//		szPath		buffer to receive string
//		uLen		length of buffer in bytes
//
//	returns kPGPError_NoErr if no error.
//
	
PGPError PGPcdExport 
PGPcomdlgGetPGPPath (LPSTR szPath, UINT uLen);


//----------------------------------------------------|
//	Fills specified combobox with server name strings.
//
//	Entry parameters :
//		hwndComboBox		handle of combobox to fill
//
//	returns kPGPError_NoErr if no error.
//
	
PGPError PGPcdExport 
PGPcomdlgFillServerComboBox (HWND hwndComboBox);


//----------------------------------------------------|
//	Opens standard groups file 
//
//	Entry parameters :
//		ppGroup			address of buffer to receive pointer 
//						to group file structure
//
//	returns kPGPError_NoErr if no error.
//

PGPError PGPcdExport 
PGPcomdlgOpenGroupFile (PGPcdGROUPFILE** ppGroup);


//----------------------------------------------------|
//	Saves groups file
//
//	Entry parameters :
//		pGroup			pointer to group file structure
//
//	returns kPGPError_NoErr if no error.
//

PGPError PGPcdExport
PGPcomdlgSaveGroupFile (PGPcdGROUPFILE* pGroup);


//----------------------------------------------------|
//	Closes groups file
//
//	Entry parameters :
//		pGroup			pointer to group file structure
//
//	returns kPGPError_NoErr if no error.
//

PGPError PGPcdExport 	
PGPcomdlgCloseGroupFile (PGPcdGROUPFILE* pGroup);


//----------------------------------------------------|
//	Copy user info strings to preferences file
//
//	Entry parameters :
//		szOwnerName			owner name
//		szCompanyName		company name
//		szLicenseNumber		license number
//
//	returns kPGPError_NoErr if no error.
//
	
PGPError PGPcdExport 
PGPcomdlgSetUserInfo (
		LPSTR	szOwnerName,
		LPSTR	szCompanyName,
		LPSTR	szLicenseNumber);


//----------------------------------------------------|
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
//
//	returns kPGPError_NoErr if no error.
//

PGPError PGPcdExport 
PGPcomdlgGetSDKFilePaths (
				LPSTR	pszPubRingPath,
				INT		iPubRingLen,
				LPSTR	pszPrivRingPath,
				INT		iPrivRingLen,
				LPSTR	pszRandSeedPath,
				INT		iRandSeedLen);


//----------------------------------------------------|
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
//
//	returns kPGPError_NoErr if no error.
//

PGPError PGPcdExport 
PGPcomdlgSetSDKFilePaths (
				LPSTR	pszPubRingPath,
				LPSTR	pszPrivRingPath,
				LPSTR	pszRandSeedPath,
				BOOL	bForceCreate);


//----------------------------------------------------|
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
//

PGPError PGPcdExport 
PGPcomdlgCheckSDKSupportForPKAlg (
				PGPPublicKeyAlgorithm	alg,
				PGPBoolean				mustEncrypt,
				PGPBoolean				mustSign);


//----------------------------------------------------|
//	Check whether SDK supports specified cipher algorithm
//
//	Entry parameters :
//		PGPCipherAlgorithm		SDK cipher algorithm constant
//
//	returns kPGPError_NoErr if SDK supports operations with
//	specified algorithm, kPGPError_FeatureNotAvailable otherwise.
//

BOOL PGPcdExport 
PGPcomdlgCheckSDKSupportForCipherAlg (
				PGPCipherAlgorithm		alg);


//----------------------------------------------------|
//	Initialize timedate classes
//
//	Entry parameters :
//		hinst			calling instance
//		
//	returns TRUE if no error.
//

BOOL PGPcdExport
PGPcomdlgInitTimeDateControl(HINSTANCE hinst);


//----------------------------------------------------|
//	Create a timedate control
//
//	Entry parameters :
//		hwndParent			owner window
//		hinst				calling instance
//		xPos				position of left side
//		yPos				position of top side
//		type				time, date, duration type
//
//	returns handle to control if no error.
//

HWND PGPcdExport
PGPcomdlgCreateTimeDateControl(	HWND hwndParent, 
								HINSTANCE hinst,
								LONG xPos,
								LONG yPos,
								LONG type);

//----------------------------------------------------|
//	convenient defines for timedate control
//

#define TD_GETHOUR		WM_USER + 1776
#define TD_GETMONTH		TD_GETHOUR
#define TD_GETMINUTE	WM_USER + 1777
#define TD_GETDAY		TD_GETMINUTE
#define TD_GETSECOND	WM_USER + 1778
#define TD_GETYEAR		TD_GETSECOND

#define TD_SETHOUR		WM_USER + 1779
#define TD_SETMONTH		TD_SETHOUR
#define TD_SETMINUTE	WM_USER + 1780
#define TD_SETDAY		TD_SETMINUTE
#define TD_SETSECOND	WM_USER + 1781
#define TD_SETYEAR		TD_SETSECOND

#define TD_SETHOURLIMIT		WM_USER + 1782
#define TD_SETMONTHLIMIT	TD_SETHOURLIMIT
#define TD_SETMINUTELIMIT	WM_USER + 1783
#define TD_SETDAYLIMIT		TD_SETMINUTELIMIT
#define TD_SETSECONDLIMIT	WM_USER + 1784
#define TD_SETYEARLIMIT		TD_SETSECONDLIMIT

#define TDN_TIMECHANGED		WM_USER + 1785


#define Time_GetHour(hwndCtl) \
((int)(DWORD)SendMessage((hwndCtl), TD_GETHOUR, 0L, 0L))
#define Duration_GetHour(hwndCtl) \
((int)(DWORD)SendMessage((hwndCtl), TD_GETHOUR, 0L, 0L))
#define Date_GetMonth(hwndCtl) \
((int)(DWORD)SendMessage((hwndCtl), TD_GETMONTH, 0L, 0L))

#define Time_GetMinute(hwndCtl)	\
((int)(DWORD)SendMessage((hwndCtl), TD_GETMINUTE, 0L, 0L))
#define Duration_GetMinute(hwndCtl)	\
((int)(DWORD)SendMessage((hwndCtl), TD_GETMINUTE, 0L, 0L))
#define Date_GetDay(hwndCtl) \
((int)(DWORD)SendMessage((hwndCtl), TD_GETDAY, 0L, 0L))

#define Time_GetSecond(hwndCtl)	\
((int)(DWORD)SendMessage((hwndCtl), TD_GETSECOND, 0L, 0L))
#define Duration_GetSecond(hwndCtl)	\
((int)(DWORD)SendMessage((hwndCtl), TD_GETSECOND, 0L, 0L))
#define Date_GetYear(hwndCtl) \
((int)(DWORD)SendMessage((hwndCtl), TD_GETYEAR, 0L, 0L))

#define Time_SetHour(hwndCtl, x) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETHOUR, (WPARAM)x, 0L))
#define Duration_SetHour(hwndCtl, x) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETHOUR, (WPARAM)x, 0L))
#define Date_SetMonth(hwndCtl, x) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETMONTH, (WPARAM)x, 0L))

#define Time_SetMinute(hwndCtl, x)	\
((int)(DWORD)SendMessage((hwndCtl), TD_SETMINUTE, (WPARAM)x, 0L))
#define Duration_SetMinute(hwndCtl, x) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETMINUTE, (WPARAM)x, 0L))
#define Date_SetDay(hwndCtl, x)	 \
((int)(DWORD)SendMessage((hwndCtl), TD_SETDAY, (WPARAM)x, 0L))

#define Time_SetSecond(hwndCtl, x)	\
((int)(DWORD)SendMessage((hwndCtl), TD_SETSECOND, (WPARAM)x, 0L))
#define Duration_SetSecond(hwndCtl, x)	\
((int)(DWORD)SendMessage((hwndCtl), TD_SETSECOND, (WPARAM)x, 0L))
#define Date_SetYear(hwndCtl, x) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETYEAR, (WPARAM)x, 0L))

#define Duration_SetHourLimits(hwndCtl,   hi,lo) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETHOURLIMIT, (WPARAM)hi,(LPARAM)lo))
#define Duration_SetMinuteLimits(hwndCtl, hi,lo) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETMINUTELIMIT, (WPARAM)hi,(LPARAM)lo))
#define Duration_SetSecondLimits(hwndCtl, hi,lo) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETSECONDLIMIT, (WPARAM)hi, (LPARAM)lo))

#define Date_SetMonthLimits(hwndCtl,   hi,lo) \
((int)(DWORD)SendMessage((hwndCtl),TD_SETMONTHLIMIT,(WPARAM)hi,(LPARAM)lo))
#define Date_SetDayLimits(hwndCtl, hi,lo) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETDAYLIMIT, (WPARAM)hi, (LPARAM)lo))
#define Date_SetYearLimits(hwndCtl, hi,lo) \
((int)(DWORD)SendMessage((hwndCtl), TD_SETYEARLIMIT, (WPARAM)hi, (LPARAM)lo))
 
//----------------------------------------------------|

#ifdef __cplusplus
}
#endif

#endif /* ] Included_PGPcmdlg_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
