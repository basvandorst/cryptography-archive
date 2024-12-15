/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLerror.c - convert error codes to strings and display in messagebox
	

	$Id: CLerror.c,v 1.11 1999/01/15 20:04:38 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// external globals
extern HINSTANCE g_hInst;

//	_____________________________________
//
//	get descriptive string for error code

PGPError PGPclExport 
PGPclErrorToString (
		INT		iCode, 
		LPSTR	szString, 
		UINT	uLen) 
{
	PGPError	err			= kPGPError_NoErr;
	UINT		uIDS;

	if (IsntPGPError (iCode)) return FALSE;

	switch (iCode) {
		case kPGPError_UserAbort :
		case kPGPError_Interrupted :
		case kPGPError_Win32_AlreadyOpen :
			return err;

		case kPGPError_Win32_NoSecretKeys :
			uIDS = IDS_ERR_NOSECRETKEYS;
			break;

		case kPGPError_Win32_Expired :
			uIDS = IDS_ERR_EXPIRED;
			break;

		case kPGPError_Win32_NoSigningKey :
			uIDS = IDS_ERR_NOSIGNINGKEYFORSEARCH;
			break;

		case kPGPError_Win32_ImageTooBig :
			uIDS = IDS_ERR_IMAGETOOBIG;
			break;

		case kPGPError_Win32_InvalidImage :
			uIDS = IDS_ERR_INVALIDIMAGE;
			break;

		case kPGPError_Win32_InvalidComctl32 :
			uIDS = IDS_ERR_INVALIDCOMMONCONTROLS;
			break;

		case kPGPError_Win32_NoRootCACertPrefs :
			uIDS = IDS_ERR_NOROOTCACERT;
			break;

		case kPGPError_Win32_NoCAServerPrefs :
			uIDS = IDS_ERR_NOCASERVER;
			break;

		case kPGPError_OutOfMemory :
			uIDS = IDS_ERR_OUTOFMEMORY;
			break;

		case kPGPError_CantOpenFile :
		case kPGPError_FileNotFound :		//???
			uIDS = IDS_ERR_NOFILE;
			break;

		case kPGPError_FilePermissions :
		case kPGPError_FileLocked :
			uIDS = IDS_ERR_FILEPERMISSIONS;
			break;

		case kPGPError_WriteFailed :
		case kPGPError_DiskFull :			//???
			uIDS = IDS_ERR_KEYIOWRITING;
			break;

		case kPGPError_IllegalFileOp:
			uIDS = IDS_ERR_INVALIDFILEOPERROR;
			break;

		case kPGPError_IncompatibleAPI :
			uIDS = IDS_ERR_INCOMPATIBLESDKAPI;
			break;

		case kPGPError_BadPassphrase :
		case kPGPError_MissingPassphrase :
			uIDS = IDS_ERR_WRONGKEYPASSPHRASE;
			break;

		case kPGPError_BadPacket :
			uIDS = IDS_ERR_CORRUPTPACKET;
			break;

		case kPGPError_DuplicateUserID :
			uIDS = IDS_ERR_DUPLICATEUSERID;
			break;

		case kPGPError_DuplicateCert :
			uIDS = IDS_ERR_DUPLICATECERT;
			break;

		case kPGPError_NoDecryptionKeyFound :
			uIDS = IDS_ERR_EXTRACTKEYNOTEXIST;
			break;

		case kPGPError_DetachedSignatureFound :
			uIDS = IDS_ERR_DETSIGFOUND;
			break;

		case kPGPError_KeyUnusableForEncryption :
			uIDS = IDS_ERR_SIGONLYKEY;
			break;

		case kPGPError_KeyUnusableForSignature :
			uIDS = IDS_ERR_ENCRYPTONLYKEY;
			break;

		case kPGPError_RandomSeedTooSmall :
		case kPGPError_OutOfEntropy :
			uIDS = IDS_ERR_RNGNOTSEEDED;
			break;

		case kPGPError_InputFile :
			uIDS = IDS_ERR_INPUTFILENOTEXIST;
			break;

		case kPGPError_OutputBufferTooSmall :
			uIDS = IDS_ERR_OUTPUTBUFFERTOOSMALL;
			break;

		case kPGPError_KeyInvalid :
			uIDS = IDS_ERR_CANNOTUSEUNTRUSTEDKEY;
			break;

		case kPGPError_FileOpFailed :
			uIDS = IDS_ERR_FILEOPFAILED;
			break;

		case kPGPError_ReadFailed :
		case kPGPError_EOF :
			uIDS = IDS_ERR_KEYRINGCORRUPT;
			break;

		case kPGPError_ServerKeyFailedPolicy :
			uIDS = IDS_ERR_KEYFAILEDPOLICY;
			break;

		case kPGPError_UnsupportedHKPFilter :
			uIDS = IDS_ERR_UNSUPPORTEDHTTPFILTER;
			break;

		default:
			uIDS = IDS_ERR_CATCHALL;
			break;
	}

	if (uIDS == IDS_ERR_CATCHALL) {
		CHAR	sz1[256];
		err = PGPGetClientErrorString (iCode, sizeof(sz1), sz1);
		if (IsntPGPError (err)) {
			CHAR	sz2[256];
			LoadString (g_hInst, IDS_ERRORPREFIX, sz2, sizeof(sz2));
			lstrcat (sz2, sz1);
			lstrcpyn (szString, sz2, uLen);
			szString[uLen-1] = '\0';
		}
		else {
			LoadString (g_hInst, uIDS, szString, uLen);
			wsprintf (sz1, szString, iCode);
			strncpy (szString, sz1, uLen);
			szString[uLen-1] = '\0';
		}
	}
	else LoadString (g_hInst, uIDS, szString, uLen);

	return kPGPError_UnknownError;
}

//	_____________________________________
//
//	putup error messagebox (if necessary)

PGPError PGPclExport 
PGPclErrorBox (
		HWND hWnd,
		INT iCode) 
{
	PGPError	err				= kPGPError_NoErr;
	CHAR		szMessage[512];
	CHAR		szCaption[64];

	err = PGPclErrorToString (iCode, szMessage, sizeof(szMessage));
	if (IsPGPError (err)) {
		LoadString (g_hInst, IDS_ERRORMESSAGECAPTION, szCaption, 
								sizeof(szCaption));
		MessageBox (hWnd, szMessage, szCaption, 
						MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|
						MB_TASKMODAL|MB_TOPMOST);
	}

	return err;
}

//	_____________________________________
//
//	get descriptive string for error code

PGPError PGPclExport 
PGPclEncDecErrorToString (
		INT		iCode, 
		LPSTR	szString, 
		UINT	uLen) 
{
	PGPError	err			= kPGPError_NoErr;
	UINT		uIDS;

	if (IsntPGPError (iCode)) return FALSE;

	switch (iCode) {
		case kPGPError_UserAbort :
		case kPGPError_Interrupted :
			return err;

		default:
			uIDS = IDS_ERR_CATCHALL;
			break;
	}

	if (uIDS == IDS_ERR_CATCHALL) {
		CHAR	sz1[256];
		err = PGPGetClientErrorString (iCode, sizeof(sz1), sz1);
		if (IsntPGPError (err)) {
			CHAR	sz2[256];
			LoadString (g_hInst, IDS_ERRORPREFIX, sz2, sizeof(sz2));
			lstrcat (sz2, sz1);
			lstrcpyn (szString, sz2, uLen);
			szString[uLen-1] = '\0';
		}
		else {
			LoadString (g_hInst, uIDS, szString, uLen);
			wsprintf (sz1, szString, iCode);
			strncpy (szString, sz1, uLen);
			szString[uLen-1] = '\0';
		}
	}
	else LoadString (g_hInst, uIDS, szString, uLen);

	return kPGPError_UnknownError;
}

//	_____________________________________
//
//	putup error messagebox (if necessary)

PGPError PGPclExport 
PGPclEncDecErrorBox (
		HWND hWnd,
		INT iCode) 
{
	PGPError	err				= kPGPError_NoErr;
	CHAR		szMessage[512];
	CHAR		szCaption[64];

	err = PGPclEncDecErrorToString (iCode, szMessage, sizeof(szMessage));
	if (IsPGPError (err)) {
		LoadString (g_hInst, IDS_ERRORMESSAGECAPTION, szCaption, 
								sizeof(szCaption));
		MessageBox (hWnd, szMessage, szCaption, 
						MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|
						MB_TASKMODAL|MB_TOPMOST);
	}

	return err;
}

