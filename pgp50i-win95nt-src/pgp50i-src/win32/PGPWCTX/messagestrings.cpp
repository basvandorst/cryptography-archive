/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
//#include "globals.h"

//#include "stdafxs.h"

extern HINSTANCE	g_hmodThisDll; // Handle to this DLL itself.

#include "..\include\config.h"
#include "..\include\spgp.h"
#include "resource.h"

BOOL
SimplePGPErrorToString(char *ErrorMessage,
					   unsigned long ErrorMessageLen,
					   int SimpleResult)
{
	BOOL ReturnCode = TRUE; //Assume there is an error
	char TemporaryBuffer[128];

	assert(ErrorMessage);
	assert(ErrorMessageLen);

	switch(SimpleResult)
	{
		//We don't want to say anything on Cancels, or if we get here
		//on success, somehow:
		case SIMPLEPGPKEYSEL_CANCEL:
		case KERNEL_CANCEL_ON_BUSY:
		case KEYSEL_CANCEL_ON_BUSY:
		case SIMPLEPGPENCRYPTBUFFER_KEYSELCANCEL:
		case SIMPLEPGPSIGNBUFFER_KEYSELCANCEL:
		case SIMPLEPGPOPENPUBLICKEYRING_USERCANCELONTASKBUSY:
		case SIMPLEPGPOPENPRIVATEKEYRING_USERCANCELONTASKBUSY:
		//The kernel pops up a message for this one, so we don't want to
		//pop up a bonus box...
		case KERNEL_EXIT_OK:
		case KERNEL_EXIT_OK_NOSIG:
			ReturnCode = FALSE;  //There wasn't an error message
			break;

		case SIMPLEPGPRECEIVEBUFFER_INPUTBUFFERLENGTHISZERO:
			LoadString(g_hmodThisDll,
					   IDS_NO_PGP_DATA,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case SIMPLEPGPENCRYPTBUFFER_INPUTBUFFERLENGTHISZERO:
			LoadString(g_hmodThisDll,
					   IDS_NO_DATA_TO_ENCRYPT,
					   ErrorMessage,
					   ErrorMessageLen);

		case KERNEL_RSA_ENCR_ERROR:
			LoadString(g_hmodThisDll,
					   IDS_RSA_ENCR_ERROR,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case -100:
			strcpy(ErrorMessage,"No PGP encrypted/signed blocks in message.");
			break;

		case -101:
			strcpy(ErrorMessage,
				"Could not find default key to encrypt to self!\n"
				"Please choose a default key in the key manager.");
			break;


#if 0
		case SIMPLEPGPKEYSEL_KEYRINGFILENOTFOUND:
			LoadString(g_hmodThisDll,
					   IDS_KEYRING_FILE_NOT_FOUND,
					   ErrorMessage,
					   ErrorMessageLen);
			break;
#endif

		case KERNEL_SIGNATURE_ERROR:
			// Virtually infinite possibilities for this error code
			// 1) User pressed <ESC> at passphrase dlg
			// 2) User tried 10 times, still bad pwd
			// 3) "Can't find public part of sign-key"
			LoadString(g_hmodThisDll,
					    IDS_SIGNATURE_ERROR,
					    ErrorMessage,
					    ErrorMessageLen);
			break;

		case SIMPLEPGPSIGNBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE:
		case SIMPLEPGPSIGNBUFFER_NOTENOUGHMEMORYFOROUTPUTSTRUCTURE:
		case SIMPLEPGPENCRYPTBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE:
		case SIMPLEPGPENCRYPTBUFFER_NOTENOUGHMEMORYFOROUTPUTSTRUCTURE:
		case SIMPLEPGPRECEIVEBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE:
		case SIMPLEPGPRECEIVEBUFFER_NOTENOUGHMEMORYFOROUTPUTSTRUCTURE:
			// Insufficient memory
			LoadString(g_hmodThisDll,
					   IDS_OUT_OF_MEMORY,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case SIMPLEPGPSIGNBUFFER_FUNCTIONNOTENABLED:
		case SIMPLEPGPRECEIVEBUFFER_FUNCTIONNOTENABLED:
		case SIMPLEPGPENCRYPTBUFFER_ENCRYPTNOTSIGN_FUNCTIONNOTENABLED:
		case SIMPLEPGPENCRYPTBUFFER_ENCRYPTANDSIGN_FUNCTIONNOTENABLED:
			LoadString(g_hmodThisDll,
					   IDS_FUNCTION_NOT_ENABLED,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case SIMPLEPGPSIGNBUFFER_CANNOTUSEUNCERTIFIEDKEY:
			LoadString(g_hmodThisDll,
					   IDS_CANNOT_SIGN_WITH_UNCERTIFIED_KEY,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case SIMPLEPGPRECEIVEBUFFER_CANNOTUSEUNCERTIFIEDKEY:
			// Key(s) not certified
			LoadString(g_hmodThisDll,
					   IDS_CANNOT_DECRYPT_WITH_UNCERTIFIED_KEY,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case SIMPLEPGPENCRYPTBUFFER_CANNOTUSEUNCERTIFIEDKEY:
			// Key(s) not certified
			LoadString(g_hmodThisDll,
					   IDS_CANNOT_ENCRYPT_WITH_UNCERTIFIED_KEY,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		case KERNEL_INVALID_FILE_OP_ERROR:
			LoadString(g_hmodThisDll,
					   IDS_INVALID_FILE_OP_ERROR,
					   ErrorMessage,
					   ErrorMessageLen);
			break;

		default:
		//All other errors are internal problems (like NULL pointers
		//or buffers that are too small) and should never be returned.
			LoadString(g_hmodThisDll,
					    IDS_CATCHALL_ERROR_MESSAGE,
					    ErrorMessage,
					    ErrorMessageLen);

			sprintf(TemporaryBuffer,
					ErrorMessage,
					SimpleResult);
			strncpy(ErrorMessage, TemporaryBuffer, ErrorMessageLen);
			break;

	}

	return(ReturnCode);
}


BOOL
TranslateSignatureResult(char *SignatureResult,
						 char *Signer,
						 char *SignatureDate,
						 int SignatureCode)
{
	char ConditionalMessage[255] = "\0"; //This is for the cases where we
	//sometimes add some more info.
	BOOL ReturnCode = FALSE;

	assert(SignatureResult);
	assert(Signer);
	assert(SignatureDate);

	*SignatureResult = '\0';

	switch(SignatureCode)
	{
	    case SIGSTS_VERIFIED_UNTRUSTED:
			sprintf(ConditionalMessage,
					"\r\nWarning:  Because this key is not certified "
						"with enough trusted signatures, it cannot be known "
						"with high confidence that this message was "
						"actually signed by %s.",
					Signer);
			//Fall through

		case SIGSTS_VERIFIED:
			sprintf(SignatureResult,
					"Good signature from %s.\r\nSignature made on %s.\r\n",
					Signer,
					SignatureDate);
			strcat(SignatureResult, ConditionalMessage);
			break;

		case SIGSTS_NOTVERIFIED:
			strcpy(SignatureResult,
				   "Could not find a public key to verify the signature on "
					   "this message.");
			break;

		case SIGSTS_BADSIG:
			if(*Signer)
				sprintf(SignatureResult, "Bad signature from %s!", Signer);
			else
				strcpy(SignatureResult, "Unable to verify signature!");

			if(*SignatureDate)
			{
				sprintf(ConditionalMessage,
						"  Signature made on %s.\r\n",
						SignatureDate);
				strcat(SignatureResult, ConditionalMessage);
			}
			break;
	}

	if(*SignatureResult)
		ReturnCode = TRUE;

	return(ReturnCode);
}
