/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPerror.c - convert error codes to strings and display in messagebox
	

	$Id: CDerror.c,v 1.19 1997/10/22 18:07:19 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"

// external globals
extern HINSTANCE g_hInst;

//	_____________________________________
//
//	get descriptive string for error code

BOOL 
PGPcdExport PGPcomdlgErrorToString (
		INT		iCode, 
		LPSTR	szString, 
		UINT	uLen) 
{
	PGPError	err			= kPGPError_NoErr;
	UINT		uIDS;

	if (IsntPGPError (iCode)) return FALSE;

	switch (iCode) {
		case kPGPError_Win32_NoSecretKeys :
			uIDS = IDS_ERR_NOSECRETKEYS;
			break;

		case kPGPError_Win32_Expired :
			uIDS = IDS_ERR_EXPIRED;
			break;

		case kPGPError_Win32_NoSigningKey :
			uIDS = IDS_ERR_NOSIGNINGKEYFORSEARCH;
			break;

		case kPGPError_UserAbort :
		case kPGPError_Interrupted :
		case kPGPError_Win32_AlreadyOpen :
			return FALSE;

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
		err = PGPGetErrorString (iCode, sizeof(sz1), sz1);
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

	return TRUE;
}

//	_____________________________________
//
//	putup error messagebox (if necessary)

BOOL PGPcdExport 
PGPcomdlgErrorMessage (INT iCode) 
{
	CHAR szMessage[512];
	CHAR szCaption[64];

	if (PGPcomdlgErrorToString (iCode, szMessage, sizeof(szMessage))) {
		LoadString (g_hInst, IDS_ERRORMESSAGECAPTION, szCaption, 
								sizeof(szCaption));
		MessageBox (NULL, szMessage, szCaption, 
						MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|
						MB_TASKMODAL|MB_TOPMOST);
		return TRUE;
	}
	return FALSE;
}

