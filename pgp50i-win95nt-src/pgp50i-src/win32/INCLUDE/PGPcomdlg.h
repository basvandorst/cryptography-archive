//: PGPcomdlg.h - include file for PGP common dialog DLL
//
//	$Id: PGPcomdlg.h,v 1.7.2.1 1997/06/09 15:54:48 dgal Exp $
//
/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#ifndef _PGPCOMDLGH
#define _PGPCOMGLGH

// options bits
#define PGPCOMDLG_ASCIIARMOR			0x0001
#define PGPCOMDLG_DETACHEDSIG			0x0002

// flag bits
#define PGPCOMDLG_RSAENCRYPT			0x0001

// return codes
#define PGPCOMDLG_OK					0
#define PGPCOMDLG_CANCEL				5001
#define PGPCOMDLG_LIBERROR				5002
#define PGPCOMDLG_MEMERROR				5003
#define PGPCOMDLG_ALREADYOPEN			5004
#define PGPCOMDLG_NOSECRETKEYS			5005
#define PGPCOMDLG_NO					5006
#define PGPCOMDLG_EXPIRED				5007

// eval expire codes
#define PGPCOMDLG_ENCRYPTSIGNEXPIRED		1
#define PGPCOMDLG_ALLEXPIRED				2

// broadcast message used to inform others of keyring changes
#define RELOADKEYRINGMSG	("PGPM_RELOADKEYRING")


#ifdef __cplusplus
extern "C" {
#endif

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
//					  NULL => use default ("PGP.com")
//		szLink		- URL to which to jump
//					  NULL => use default ("http://www.pgp.com/")
//
//	Returns PGPCOMDLG_OK
//

UINT PGPExport PGPcomdlgHelpAbout (HWND hWndParent, LPSTR szVersion,
								   LPSTR szLinkText, LPSTR szLink);


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
//	Returns PGPCOMDLG_OK
//

UINT PGPExport PGPcomdlgNag (HWND hWndParent, LPSTR szLinkText, LPSTR szLink);


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
//	Returns PGPCOMDLG_CANCEL if user presses the CANCEL
//	button.  Application should exit in this case.
//

UINT PGPExport PGPcomdlgCheckKeyRings (HWND hWndParent, BOOL bForceQuery);


//-------------------------------------------------|
//	PGPcomdlgPreferences -
//	post PGP preferences property sheets.
//
//	This function posts the "PGP Preferences" property
//	sheet dialog boxes.  Appropriate calls are made into
//	the pgpkeydb library to set the preferences.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		iStartsheet	- zero-based index of property sheet
//                    page to display initially
//
//	This function returns PGPPREF_OK if all was OK or
//	PGPPREF_CANCEL (if the user presses the cancel button)
//  or one of the above-defined error constants.
//

UINT PGPExport PGPcomdlgPreferences (HWND hWndParent, INT iStartsheet);



//-------------------------------------------------|
//	PGPcomdlgRandom -
//	collect random data from keyboard/mouse.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		uNeeded		- bits of entropy needed
//
//	This function returns either PGPCOMDLG_CANCEL (if the user
//  presses the cancel button) or PGPCOMDLG_OK or one
//	of the above-defined error constants
//

UINT PGPExport PGPcomdlgRandom (HWND hWndParent, UINT uNeeded);


//-------------------------------------------------|
//	PGPcomdlgGetPhrase -
//	get passphrase from user.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		szPrompt	- message string to be displayed to user
//		pszPhrase	- pointer to receive address of buffer which
//					  will contain passphrase.  The caller should
//					  deallocate this buffer by calling PGPcomdlgFreePhrase
//		pKeySet		- pointer to PGPKeySet which contains current keyring
//					  NULL => hide key selection combo box
//		ppKey		- pointer to buffer to receive pointer to selected key.
//					  if buffer contains key on entry, this will be default.
//					  buffer should be set to NULL to use keyring default key.
//					  NULL => hide key selection combo box
//		szKeyID		- string containing keyid of key to use as default.
//					  may be empty or NULL.
//		uOptions	- pointer to buffer which contains and will receive
//					  options bits
//					  NULL => hide option checkboxes
//		uFlags		- flag bits
//					  PGPCOMDLG_RSAENCRYPT => encrypting to RSA key
//
//	This function returns PGPCOMDLG_CANCEL if the user
//  presses the cancel button.  PGPCOMDLG_OK if no error.
//	Or one of the above-defined error constants.
//

UINT PGPExport PGPcomdlgGetPhrase (HWND hWndParent, LPSTR szPrompt,
										LPSTR* pszPhrase, PGPKeySet* pKeySet,
										PGPKey** ppKey, LPSTR szKeyID,
										UINT* uOptions, UINT uFlags);


//-------------------------------------------------|
//	PGPcomdlgFreePhrase -
//	wipe and deallocate phrase buffer.
//
//	Entry parameters :
//		szPhrase	- passphrase buffer to wipe and deallocate
//

VOID PGPExport PGPcomdlgFreePhrase (LPSTR szPhrase);


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

BOOL PGPExport PGPcomdlgErrorToString (INT iCode, LPSTR szString,
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

BOOL PGPExport PGPcomdlgErrorMessage (INT iCode);


//----------------------------------------------------|
//	convert signature result code to message string
//
//	Entry parameters :
//		iSignatureCode	- code returned by signature checking routine
//		szSigner		- string representation of signer key
//		szSignatureDate	- string representation of signature date
//		szMessageBuffer - buffer to receive translated text message
//		uBufferLen		- length of buffer in bytes
//
//	This function converts code and signer/date strings into a
//	single message string.  Returns TRUE if successful, FALSE otherwise.
//

BOOL PGPExport PGPcomdlgTranslateSignatureResult (
							INT iSignatureCode, LPSTR szSigner,
							LPSTR szSignatureDate,
							LPSTR szMessageBuffer, UINT uBufferLen);


//----------------------------------------------------|
//	Post Splash screen
//
//	Entry parameters :
//		hWndParent		- handle of parent window (if NULL,
//						  screen will not be dismissed until
//						  PGPcomdlgSetSplashParent is called
//						  with a non-null value
//		uMilliseconds	- milliseconds to display splash
//

UINT PGPExport PGPcomdlgSplash (HWND hWndParent, UINT uMilliseconds);



//----------------------------------------------------|
//	Inform splash screen who its parent is
//
//	Entry parameters :
//		hWndParent		- handle of parent window
//

UINT PGPExport PGPcomdlgSetSplashParent (HWND hWndParent);



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

VOID PGPExport PGPcomdlgNotifyKeyringChanges (LPARAM lParam);


//----------------------------------------------------|
//	Returns TRUE if the beta or eval has expired, and displays a message box
//  Returns FALSE and does nothing if it's still valid
//
//	Entry parameters :
//		hwnd		Parent of window so we can show the MB.
//					NULL is acceptable.
//

BOOL PGPExport PGPcomdlgIsExpired (HWND hwnd);

//----------------------------------------------------|
//	Returns TRUE if the evaluation has expired, and displays a message box
//  If the eval is partially expired, the nag screen will be displayed
//
//	Entry parameters :
//		hwnd		Parent of window so we can show the MB or nag screen.
//					NULL is acceptable.
//		nIndex		Type of expiration to test: Encrypt/Sign or All

#ifdef EVAL_TIMEOUT
	BOOL PGPExport PGPcomdlgEvalExpired (HWND hwnd, int nIndex);
#else
	#define	PGPcomdlgEvalExpired(x,y)	FALSE
#endif	// EVAL_TIMEOUT


//-------------------------------------------------|
//	PGPcomdlgSignDSAEncryptRSAWarning -
//	Post warning dialog
//
//	Entry parameters :
//		hWnd	- handle of parent window
//
//	Returns PGPCOMDLG_OK, PGPCOMDLG_CANCEL, or PGPCOMDLG_NO
//

LRESULT PGPExport PGPcomdlgRSADSAMixWarning (HWND hWnd);


//-------------------------------------------------|
//	PGPcomdlgAdvertisement -
//	Post Help About dialog with button for browser
//	launch to specified URL.
//
//	Entry parameters :
//		hWndParent	- handle of parent window
//		uIndex		- index specifying which (!) ad to display
//
//	Returns PGPCOMDLG_OK
//

UINT PGPExport PGPcomdlgAdvertisement (HWND hWndParent, UINT uIndex);


#ifdef __cplusplus
}
#endif

#endif	//_PGPCOMDLGH
