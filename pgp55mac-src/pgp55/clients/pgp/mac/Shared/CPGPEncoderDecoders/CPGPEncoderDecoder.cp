/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPEncoderDecoder.cp,v 1.38.8.1 1997/12/05 22:14:39 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "PGPKeys.h"
#include "PGPUtilities.h"
#include "MacFiles.h"
#include "MacMemory.h"
#include "MacStrings.h"
#include "MacErrors.h"
#include "PGPSharedEncryptDecrypt.h"
#include "PGPUserInterface.h"
#include "WarningAlert.h"
#include "pflPrefTypes.h"
#include "pgpOpenPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pflContext.h"
#include "pgpWordWrap.h"
#include "pgpMemoryIO.h"
#include "pgpVersionHeader.h"
#include "CSecureMemory.h"

#include "CTempFile.h"

#include "CPGPEncoderDecoder.h"


struct SDecodeData {
	Boolean				useCache;
	AnimatedCursorRef	cursor;
	Handle				handle;
};


// Statics
Boolean					CPGPEncoderDecoder::sCheckForMissingKeys	=	true;
CSignPassphraseCache *	CPGPEncoderDecoder::sSigningPassphraseCache	=	nil;
CSignPassphraseCache *	CPGPEncoderDecoder::sDecryptPassphraseCache	=	nil;



// Constants
const ResIDT	STRx_PGPEncoderDecoder				=	12121;
const SInt16	kNoPublicKeysID						=	1;
const SInt16	kNoPrivateKeysID					=	2;
const SInt16	kAddKeysID							=	3;

const ResIDT	STRx_PGPEncoderDecoderErrorStrings	=	12122;
const SInt16	kErrorStringID						=	1;
const SInt16	kErrorStringAdminPrefsNotFoundID	=	2;

const UInt32	kAllAlgorithms = 0xE0000000;
const UInt32	kIDEAAlgorithm = 0x80000000;
const UInt32	k3DESAlgorithm = 0x40000000;
const UInt32	kCAST5Algorithm = 0x20000000;

	void
CPGPEncoderDecoder::Init()
{
	sSigningPassphraseCache = new CSignPassphraseCache;
	sDecryptPassphraseCache = new CSignPassphraseCache;
}



	void
CPGPEncoderDecoder::Cleanup()
{
	delete sSigningPassphraseCache;
	delete sDecryptPassphraseCache;
}



	Boolean
CPGPEncoderDecoder::EncodeHandle(
	PGPContextRef		inContext,
	Handle				ioDataH,
	EEncodeOptions		inOptions,
	UInt32				inNumRecipients,
	const PGPKeySpec *	inRecipientList,
	Boolean				inUseCache)
{
	Boolean				result = false;
	OSErr				err;
	PGPError			pgpErr;
	AnimatedCursorRef	cursorRef = nil;
	PGPKeySetRef		defaultKeySet = kPGPInvalidRef;
	PGPKeySetRef		recipientSet = kPGPInvalidRef;
	PGPKeySetRef		newKeys = kPGPInvalidRef;
	PGPKeySetRef		selectedSet = kPGPInvalidRef;
	PGPKeyRef			signingKey = kPGPInvalidRef;
	PGPOptionListRef	optionList = kPGPInvalidRef;
	PGPPrefRef			prefRef = kPGPInvalidRef;
	PFLContextRef		pflContext = kPGPInvalidRef;
	PGPIORef			wrapInput = kPGPInvalidRef;
	PGPIORef			wrapOutput = kPGPInvalidRef;
	
	try {
		if (::GetHandleSize(ioDataH) > 0) {
			CSecureCString256			signingPassphrase;
			CSecureCString256			convEncryptPassphrase;
			UInt32						numPublicKeys = 1;
			UInt32						numPrivateKeys = 1;
	
			// Check for missing keys
			if (sCheckForMissingKeys) {
				err = CountKeysInDefaultKeyring(	inContext,
													&numPublicKeys,
													&numPrivateKeys);
				PGPThrowIfOSErr_(err);
				
				if ((numPublicKeys != 0) && (numPrivateKeys != 0)) {
					sCheckForMissingKeys = false;
				}
			}
			
			// Open default keyset
			pgpErr = PGPOpenDefaultKeyRings(	inContext,
												0,
												&defaultKeySet);
			PGPThrowIfPGPErr_(pgpErr);

			// If encrypting, get the recipients
			if ((inOptions & encodeOptions_Encrypt)
			&& (! (inOptions & encodeOptions_ConvEncrypt))) {
				PGPBoolean				didShowDialog;
				PGPRecipientOptions		options =
										kPGPRecipientOptionsHideFileOptions;
				PGPRecipientSettings	settings =
											kPGPRecipientSettingsDefault;
				
				if (numPublicKeys == 0) {
					WarningAlert(	kWACautionAlertType,
									kWAOKStyle,
									STRx_PGPEncoderDecoder,
									kNoPublicKeysID);
				}

				if (! inUseCache) {
					options |= kPGPRecipientOptionsAlwaysShowDialog;
				}
				pgpErr = PGPRecipientDialog(	inContext,
												defaultKeySet,
												inNumRecipients,
												inRecipientList,
												options,
												settings,
												&settings,
												&recipientSet,
												&didShowDialog,
												&newKeys);
				PGPThrowIfPGPErr_(pgpErr);

				if (PGPRefIsValid(newKeys)) {
					if (didShowDialog) {
						pgpErr = PGPSelectiveImportDialog(
									inContext,							
									CString(	STRx_PGPEncoderDecoder,
												kAddKeysID),							
									newKeys,							
									defaultKeySet,
									&selectedSet);
						if (pgpErr != kPGPError_UserAbort) {
							PGPThrowIfPGPErr_(pgpErr);
							pgpErr = PGPSharedAddKeysToDefaultKeyring(
										selectedSet);
							PGPThrowIfPGPErr_(pgpErr);
						}
					}
				}
				
				if (settings & kPGPRecipientSettingsConvEncrypt) {
					inOptions &= ~encodeOptions_Encrypt;
					inOptions |= encodeOptions_ConvEncrypt;
				} else {
					PGPUInt32		numKeys;
					pgpErr	= PGPCountKeys( recipientSet, &numKeys);
					PGPThrowIfPGPErr_(pgpErr);
				
					if ( numKeys == 0) {
						inOptions &= ~encodeOptions_Encrypt;
						if (! (inOptions & encodeOptions_Sign)) {
							PGPThrowOSErr_(userCanceledErr);
						}
					}
				}
			}
			
			// If conventionally encrypting, get passphrase
			if (inOptions & encodeOptions_ConvEncrypt) {
				pgpErr = PGPGetConventionalEncryptionPassphraseDialog(inContext,
								nil,
								convEncryptPassphrase);
				PGPThrowIfPGPErr_(pgpErr);
			}
			
			// If signing, get passphrase
			if (inOptions & encodeOptions_Sign) {
				PGPGetPassphraseSettings	userSettings;
				
				// Make sure we have a key to sign to
				if (numPrivateKeys == 0) {
					WarningAlert(	kWACautionAlertType,
									kWAOKStyle,
									STRx_PGPEncoderDecoder,
									kNoPrivateKeysID);
				}
				
				// Get passphrase
				if (! inUseCache) {
					sSigningPassphraseCache->Forget();
				}
				if (! sSigningPassphraseCache->GetPassphrase(defaultKeySet,
				signingPassphrase, &signingKey)) {
					pgpErr = PGPGetSigningPassphraseDialog(	inContext,
															defaultKeySet,
															nil,
															kPGPGetPassphraseOptionsHideFileOptions,
															userSettings,
															signingKey,
															signingPassphrase,
															&userSettings,
															&signingKey);
					PGPThrowIfPGPErr_(pgpErr);
					SetPassphraseCache(true, signingKey, signingPassphrase);
				}
				
				// If we are only signing, wrap the input
				if ((! (inOptions & encodeOptions_Encrypt))
				&& (! (inOptions &encodeOptions_ConvEncrypt))) {
					PGPBoolean	wordWrapEnabled;
					
					pgpErr = PFLNewContext(&pflContext);
					PGPThrowIfPGPErr_(pgpErr);
					pgpErr = PGPOpenClientPrefs(pflContext, &prefRef);
					PGPThrowIfPGPErr_(pgpErr);
					pgpErr = PGPGetPrefBoolean(	prefRef,
												kPGPPrefWordWrapEnable,
												&wordWrapEnabled);
					PGPThrowIfPGPErr_(pgpErr);
					
					if (wordWrapEnabled) {
						PGPUInt32		wordWrapWidth;
						PGPFileOffset	bufSize;
						PGPSize			unused;	
						
						{
							StHandleLocker	theLock(ioDataH);
							
							pgpErr = PGPGetPrefNumber(	prefRef,
														kPGPPrefWordWrapWidth,
														&wordWrapWidth);
							PGPThrowIfPGPErr_(pgpErr);
							pgpErr = PGPNewMemoryIOFixedBuffer(	pflContext,
											*ioDataH,
											::GetHandleSize(ioDataH),
											(PGPMemoryIORef*) &wrapInput);
							PGPThrowIfPGPErr_(pgpErr);
							pgpErr = PGPNewMemoryIO(pflContext,
											(PGPMemoryIORef*) &wrapOutput);
							PGPThrowIfPGPErr_(pgpErr);
							pgpErr = pgpWordWrapIO(	wrapInput,
													wrapOutput,
													wordWrapWidth,
													"\r");
							PGPThrowIfPGPErr_(pgpErr);
							PGPFreeIO(wrapInput);
							wrapInput = kPGPInvalidRef;
						}
						
						pgpErr = PGPIOSetPos(	wrapOutput,
												0);
						PGPThrowIfPGPErr_(pgpErr);
						pgpErr = PGPIOGetEOF(	wrapOutput,
												&bufSize);
						PGPThrowIfPGPErr_(pgpErr);
						::SetHandleSize(ioDataH, bufSize);
						PGPThrowIfMemError_();
						::HLock(ioDataH);
						pgpErr = PGPIORead(	wrapOutput,
											bufSize,
											*ioDataH,
											&unused);
						::HUnlock(ioDataH);
						PGPThrowIfPGPErr_(pgpErr);
						PGPFreeIO(wrapOutput);
						wrapOutput = kPGPInvalidRef;
					}
				}
			}

			// Load the cursor
			err = Get1AnimatedCursor(acur_BeachBall, &cursorRef);
			PGPThrowIfOSErr_(err);
			
			// Do the encrypting
			PGPSize				bufSize;
			void *				buf;

			optionList = CreateOptionList(	inContext,
											inOptions,
											recipientSet,
											signingKey,
											signingPassphrase,
											convEncryptPassphrase);
			::HLock(ioDataH);
			pgpErr = PGPEncode(	inContext,
								PGPOInputBuffer(	inContext,
													*ioDataH,
													::GetHandleSize(ioDataH)),
								PGPOAllocatedOutputBuffer(	inContext,
															&buf,
															MAX_PGPSize,
															&bufSize),
								PGPOEventHandler(	inContext,
													PGPEncodeEventHandler,
													cursorRef),
								PGPOSendNullEvents(	inContext,
													500),
								optionList,
								PGPOLastOption(inContext));
			::HUnlock(ioDataH);
			PGPThrowIfPGPErr_(pgpErr);
			
			::PtrToXHand(buf, ioDataH, bufSize);
			PGPFreeData(buf);
								
			result = true;
		}
	}
	
	catch (CComboError & comboError) {
		if (! comboError.IsCancelError()) {
			if (comboError.HavePGPError()) {
				ShowError(comboError.pgpErr);
			} else {
				ShowError(MacErrorToPGPError(comboError.err));
			}
		}
	}

	catch(...) {
		ShowError(kPGPError_UnknownError);
	}
	
	// Cleanup
	if (cursorRef != nil) {
		DisposeAnimatedCursor(cursorRef);
	}
	if (optionList != kPGPInvalidRef) {
		PGPFreeOptionList(optionList);
	}
	if (selectedSet != kPGPInvalidRef) {
		PGPFreeKeySet(selectedSet);
	}
	if (newKeys != kPGPInvalidRef) {
		PGPFreeKeySet(newKeys);
	}
	if (recipientSet != kPGPInvalidRef) {
		PGPFreeKeySet(recipientSet);
	}
	if (defaultKeySet != kPGPInvalidRef) {
		PGPFreeKeySet(defaultKeySet);
	}
	if (prefRef != kPGPInvalidRef) {
		PGPClosePrefFile(prefRef);
	}
	if (pflContext != kPGPInvalidRef) {
		PFLFreeContext(pflContext);
	}
	if (wrapInput != kPGPInvalidRef) {
		PGPFreeIO(wrapInput);
	}
	if (wrapOutput != kPGPInvalidRef) {
		PGPFreeIO(wrapOutput);
	}

	return result;
}



	Boolean
CPGPEncoderDecoder::DecodeHandle(
	PGPContextRef			inContext,
	Handle					inDataH,
	PGPKeySetRef			inKeySet,
	SDecodeSectionHandle &	outDecodeSectionsHandle,
	Boolean					inUseCache)
{
	Boolean				result = false;

	if (::GetHandleSize(inDataH) > 0) {
		OSErr				err;
		PGPError			pgpErr;
		UInt32				numPublicKeys = 1;
		UInt32				numPrivateKeys = 1;
		SDecodeData			decodeData = {inUseCache, nil, nil};

		// Check for missing keys
		try {
			if (sCheckForMissingKeys) {
				err = CountKeysInDefaultKeyring(	inContext,
													&numPublicKeys,
													&numPrivateKeys);
				PGPThrowIfOSErr_(err);
				
				if (numPublicKeys == 0) {
					WarningAlert(	kWACautionAlertType,
									kWAOKStyle,
									STRx_PGPEncoderDecoder,
									kNoPublicKeysID);
				} else if (numPrivateKeys == 0) {
					WarningAlert(	kWACautionAlertType,
									kWAOKStyle,
									STRx_PGPEncoderDecoder,
									kNoPrivateKeysID);
				} else {
					sCheckForMissingKeys = false;
				}
			}
			
			// Get the cursor
			err = Get1AnimatedCursor(acur_BeachBall, &decodeData.cursor);
			PGPThrowIfOSErr_(err);
			
			// Create Handle for storing decoded data
			decodeData.handle = ::NewHandle(0);
			PGPThrowIfMemFail_(decodeData.handle);
			
			::HLock(inDataH);
			::HLock(decodeData.handle);
			pgpErr = PGPDecode(	inContext,
								PGPOInputBuffer(	inContext,
													*inDataH,
													::GetHandleSize(inDataH)),
								PGPOEventHandler(	inContext,
													PGPDecodeEventHandler,
													&decodeData),
								PGPOSendNullEvents(	inContext,
													500),
								PGPOKeySetRef(	inContext,
												inKeySet),
								PGPOSendEventIfKeyFound(inContext, true),
								PGPOPassThroughIfUnrecognized(	inContext,
																true),
								PGPOLastOption(inContext));
			::HUnlock(decodeData.handle);
			::HUnlock(inDataH);
			PGPThrowIfPGPErr_(pgpErr);
			
			outDecodeSectionsHandle =
					(SDecodeSectionHandle) decodeData.handle;
			decodeData.handle = nil;
			result = true;
		}
		
		catch (CComboError & comboError) {
			if (! comboError.IsCancelError()) {
				if (comboError.HavePGPError()) {
					ShowError(comboError.pgpErr);
				} else {
					ShowError(MacErrorToPGPError(comboError.err));
				}
			}
		}

		catch(...) {
			ShowError(kPGPError_UnknownError);
		}
		
		// Cleanup
		if (decodeData.cursor != nil) {
			DisposeAnimatedCursor(decodeData.cursor);
		}
		if (decodeData.handle != nil) {
			::DisposeHandle(decodeData.handle);
		}
	}
	
	return result;
}



	void
CPGPEncoderDecoder::FreeDecodeSection(
	SDecodeSection *	inSection)
{
	if (inSection->buf != nil) {
		PGPFreeData(inSection->buf);
	}
	if (inSection->addKeysSet != kPGPInvalidRef) {
		PGPFreeKeySet(inSection->addKeysSet);
	}
}



	PGPOptionListRef
CPGPEncoderDecoder::CreateOptionList(
	PGPContextRef	inContext,
	EEncodeOptions	inOptions,
	PGPKeySetRef	inRecipients,
	PGPKeyRef		inSigningKey,
	char *			inSigningPassphrase,
	char *			inConvEncryptPassphrase)
{
	PGPOptionListRef	optionList;
	CString				commentString;
	PGPPrefRef			prefRef = kPGPInvalidRef;
	PGPError			pgpErr;
	PFLContextRef		pflContext = kPGPInvalidRef;
	
	try {
		// Add admin comment
		pgpErr = PFLNewContext(&pflContext);
		PGPThrowIfPGPErr_(pgpErr);
#if PGP_BUSINESS_SECURITY
		pgpErr = PGPOpenAdminPrefs(	pflContext,
									&prefRef );

		if (pgpErr != kPGPError_NoErr) {
			WarningAlert(	kWAStopAlertType,
							kWAOKStyle,
							STRx_PGPEncoderDecoderErrorStrings,
							kErrorStringID,
							CString(pgpErr),
							CString(	STRx_PGPEncoderDecoderErrorStrings,
										kErrorStringAdminPrefsNotFoundID));
			PGPThrowOSErr_(userCanceledErr);
		}
		PGPGetPrefStringBuffer(	prefRef,
								kPGPPrefComments,
								commentString.GetBufferSize(),
								commentString);
		PGPClosePrefFile(prefRef);
#endif
		// We need the client prefs for the rest of the options
		pgpErr = PGPOpenClientPrefs(	pflContext,
										&prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		
		// If there was no admin comment, add the client comment
		if (commentString.GetLength() == 0) {
			PGPGetPrefStringBuffer(	prefRef,
									kPGPPrefComment,
									commentString.GetBufferSize(),
									commentString);
		}

		pgpErr = PGPNewOptionList(inContext, & optionList);
		PGPThrowIfPGPErr_(pgpErr);
		if (commentString.GetLength() != 0) {
			PGPAppendOptionList(	inContext,
									optionList,
									PGPOCommentString(	inContext,
														commentString),
									PGPOLastOption(inContext));
		}
								
		// Add version string
		PGPAppendOptionList(	inContext,
								optionList,
								PGPOVersionString(	inContext,
													pgpVersionHeaderString),
								PGPOLastOption(inContext));
		
		// Add rest of options
		if ((inOptions & encodeOptions_Encrypt)
		&& (! (inOptions & encodeOptions_ConvEncrypt))) {
			PGPAppendOptionList(	inContext,
									optionList,
									PGPOEncryptToKeySet(	inContext,
															inRecipients),
									PGPOLastOption(inContext));

			PGPCipherAlgorithm *	allowedAlgorithms = nil;
			PGPSize					bufSize;
			
			PGPGetPrefData(	prefRef,
							kPGPPrefAllowedAlgorithmsList,
							&bufSize,
							&allowedAlgorithms);
			if (allowedAlgorithms != nil) {
				PGPAppendOptionList(
						inContext, optionList,
						PGPOPreferredAlgorithms(inContext,
								allowedAlgorithms,
								bufSize / sizeof( PGPCipherAlgorithm ) ),
								PGPOLastOption(inContext));
				PGPDisposePrefData(	prefRef,
									allowedAlgorithms);
			}
		}
		if (inOptions & encodeOptions_ConvEncrypt) {
			PGPUInt32	preferredAlgorithm;
			
			pgpErr = PGPGetPrefNumber(	prefRef,
										kPGPPrefPreferredAlgorithm,
										&preferredAlgorithm);
			if (pgpErr != kPGPError_NoErr) {
				preferredAlgorithm = kPGPCipherAlgorithm_CAST5;
			}
			PGPAppendOptionList(	
							inContext,
							optionList,
							PGPOConventionalEncrypt(inContext, 
								PGPOPassphrase(inContext,
										inConvEncryptPassphrase),
								PGPOLastOption(inContext)),
							PGPOCipherAlgorithm(	inContext,
								(PGPCipherAlgorithm) preferredAlgorithm),
							PGPOLastOption(inContext));
		}
		if (inOptions & encodeOptions_Sign) {
			PGPAppendOptionList(	inContext,
									optionList,
									PGPOSignWithKey(inContext,
													inSigningKey,
													PGPOPassphrase(	inContext,
														inSigningPassphrase),
													PGPOLastOption(inContext)),
									PGPOLastOption(inContext));
		}
		if (inOptions & encodeOptions_TextOutput) {
			PGPAppendOptionList(	inContext,
									optionList,
									PGPOArmorOutput(inContext, true),
									PGPOLastOption(inContext));
		}
		if (inOptions & encodeOptions_TreatInputAsText) {
			PGPAppendOptionList(	inContext,
									optionList,
									PGPODataIsASCII(inContext, true),
									PGPOLastOption(inContext));
		}
		if (inOptions & encodeOptions_ClearSign) {
			PGPAppendOptionList(	inContext,
									optionList,
									PGPOClearSign(inContext, true),
									PGPOLastOption(inContext));
		}
		PGPClosePrefFile(prefRef);
		PFLFreeContext(pflContext);
	}
	
	catch (...) {
		if (prefRef != kPGPInvalidRef) {
			PGPClosePrefFile(prefRef);
		}
		if (pflContext != kPGPInvalidRef) {
			PFLFreeContext(pflContext);
		}
		throw;
	}
	
	return optionList;
}



	void
CPGPEncoderDecoder::SetPassphraseCache(
	Boolean			inSigning,
	PGPKeyRef	 	inKey,
	char *			inPassphrase)
{
	PGPPrefRef				prefRef = kPGPInvalidRef;
	PFLContextRef			pflContext = kPGPInvalidRef;
	
	try {
		CSignPassphraseCache *	theCache;
		PGPBoolean				cacheEnabled;
		PGPUInt32				cacheDuration = 0;
		PGPError				pgpErr;
		
		// Get the cache values
		pgpErr = PFLNewContext(&pflContext);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPOpenClientPrefs(pflContext, &prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		
		if (inSigning) {
			theCache = sSigningPassphraseCache;
			pgpErr = PGPGetPrefBoolean(	prefRef,
										kPGPPrefSignCacheEnable,
										&cacheEnabled);
			PGPThrowIfPGPErr_(pgpErr);
			if (cacheEnabled) {
				pgpErr = PGPGetPrefNumber(	prefRef,
											kPGPPrefSignCacheSeconds,
											&cacheDuration);
				PGPThrowIfPGPErr_(pgpErr);
			}
		} else {
			theCache = sDecryptPassphraseCache;
			pgpErr = PGPGetPrefBoolean(	prefRef,
										kPGPPrefDecryptCacheEnable,
										&cacheEnabled);
			PGPThrowIfPGPErr_(pgpErr);
			if (cacheEnabled) {
				pgpErr = PGPGetPrefNumber(	prefRef,
											kPGPPrefDecryptCacheSeconds,
											&cacheDuration);
				PGPThrowIfPGPErr_(pgpErr);
			}
		}
		
		theCache->SetCacheSeconds(cacheDuration);
		theCache->Remember(inPassphrase, inKey);
	}
	
	catch (...) {
	}

	if (prefRef != kPGPInvalidRef) {
		PGPClosePrefFile(prefRef);
	}
	if (pflContext != kPGPInvalidRef) {
		PFLFreeContext(pflContext);
	}
}


	void
CPGPEncoderDecoder::ShowError(
	PGPError	inErr)
{
	CString		theErrorString;
	
	PGPGetErrorString(	inErr,
						theErrorString.GetBufferSize() + 1,
						theErrorString);
	WarningAlert(	kWAStopAlertType,
					kWAOKStyle,
					STRx_PGPEncoderDecoderErrorStrings,
					kErrorStringID,
					CString(inErr),
					theErrorString);
}



	PGPError
CPGPEncoderDecoder::PGPDecodeEventHandler(
	PGPContextRef	inContext,
	PGPEvent *		inEventP,
	PGPUserValue	inUserValue)
{
	PGPError				result = kPGPError_NoErr;
	static PGPInt32			section;
	static Boolean			allocated;
	static PGPUInt32		numMissingKeys = 0;
	static PGPKeySetRef		recipientKeySet = kPGPInvalidRef;
	SDecodeSectionHandle	decodeSectionH = 
							(SDecodeSectionHandle) ((SDecodeData *)
									inUserValue)->handle;
	
	AnimateCursor(((SDecodeData *) inUserValue)->cursor);
	
	switch (inEventP->type) {
		case kPGPEvent_InitialEvent:
		{
			section = -1;
			allocated = false;
		}
		break;
		
		
		case kPGPEvent_EndLexEvent:
		{
			allocated = false;
			if (recipientKeySet != kPGPInvalidRef) {
				PGPFreeKeySet(recipientKeySet);
				recipientKeySet = kPGPInvalidRef;
			}
		}
		break;
		
		
		case kPGPEvent_RecipientsEvent:
		{
			PGPUInt32	numFoundKeys = 0;
			
			if (PGPKeySetRefIsValid(inEventP->
			data.recipientsData.recipientSet)) {
				result = PGPIncKeySetRefCount(inEventP->
								data.recipientsData.recipientSet);
				if (IsntPGPError(result)) {
					result = PGPCountKeys(
								inEventP->data.recipientsData.recipientSet,
								&numFoundKeys );
				}
			}
			
			if (numFoundKeys > inEventP->data.recipientsData.keyCount) {
				numFoundKeys = inEventP->data.recipientsData.keyCount;
			}
				
			recipientKeySet = inEventP->data.recipientsData.recipientSet;
			numMissingKeys = inEventP->data.recipientsData.keyCount -
											numFoundKeys;
		}
		break;
		
		
		case kPGPEvent_PassphraseEvent:
		{
			PGPKeyRef			keyRef = kPGPInvalidRef;
			PGPKeyListRef		keyListRef = kPGPInvalidRef;
			PGPKeyIterRef		keyIterRef = kPGPInvalidRef;
			CSecureCString256	passphrase;

			try {
				if (! ((SDecodeData *) inUserValue)->useCache) {
					sDecryptPassphraseCache->Forget();
				}
				
				if (inEventP->data.passphraseData.fConventional) {
					result = PGPGetConventionalDecryptionPassphraseDialog(
									inContext,
									nil,
									passphrase);
					PGPThrowIfPGPErr_(result);
				} else {
					// Get passphrase
					if (! sDecryptPassphraseCache->GetPassphrase(inEventP->
					data.passphraseData.keyset, passphrase, &keyRef)) {
						// Get the key
						result = PGPOrderKeySet(	inEventP->
														data.passphraseData.
														keyset,
													kPGPKeyIDOrdering,
													&keyListRef);
						PGPThrowIfPGPErr_(result);
						result = PGPNewKeyIter(	keyListRef, &keyIterRef);
						PGPThrowIfPGPErr_(result);
						result = PGPKeyIterNext(keyIterRef, &keyRef);
						if (result == kPGPError_EndOfIteration) {
							result = kPGPError_NoDecryptionKeyFound;
						}
						PGPThrowIfPGPErr_(result);
						result = PGPGetDecryptionPassphraseDialog(
												inContext,
												nil,
												recipientKeySet,
												numMissingKeys,
												keyRef,
												passphrase,
												&keyRef);
						PGPThrowIfPGPErr_(result);
						SetPassphraseCache(false, keyRef, passphrase);
					}
				}
					
				// Add the passphrase to the job
				result = PGPAddJobOptions(	inEventP->job,
											PGPOPassphrase(	inContext,
															passphrase),
											PGPOLastOption(inContext));
				PGPThrowIfPGPErr_(result);
			}
				
			catch(...) {
			}
			
			if (keyIterRef != kPGPInvalidRef) {
				PGPFreeKeyIter(keyIterRef);
			}
			if (keyListRef != kPGPInvalidRef) {
				PGPFreeKeyList(keyListRef);
			}
		}
		break;
		
		
		case kPGPEvent_SignatureEvent:
		{
			if (! allocated) {
				section++;
				// Unlock long enough to grow the handle
				::HUnlock((Handle) decodeSectionH);
				::SetHandleSize(	(Handle) decodeSectionH,
									(section + 1) * sizeof(SDecodeSection));
				::HLock((Handle) decodeSectionH);
				(*decodeSectionH)[section].buf = nil;
				(*decodeSectionH)[section].addKeysSet = kPGPInvalidRef;
				(*decodeSectionH)[section].sigDataValid = false;
				allocated = true;
			}

			(*decodeSectionH)[section].sigData = inEventP->
					data.signatureData;
			(*decodeSectionH)[section].sigDataValid = true;		
		}
		break;
		
		
		case kPGPEvent_OutputEvent:
		{
			if (! allocated) {
				section++;
				// Unlock long enough to grow the handle
				::HUnlock((Handle) decodeSectionH);
				::SetHandleSize(	(Handle) decodeSectionH,
									(section + 1) * sizeof(SDecodeSection));
				::HLock((Handle) decodeSectionH);
				(*decodeSectionH)[section].buf = nil;
				(*decodeSectionH)[section].addKeysSet = kPGPInvalidRef;
				(*decodeSectionH)[section].sigDataValid = false;
				allocated = true;
			}
			
			result = PGPAddJobOptions(	inEventP->job,
										PGPOAllocatedOutputBuffer(inContext,
												&(*decodeSectionH)[section].
														buf,
												MAX_PGPSize,
												&(*decodeSectionH)[section].
														size),
										PGPOLastOption(inContext));
		}
		break;
		
		
		case kPGPEvent_KeyFoundEvent:
		{
			if (! allocated) {
				section++;
				// Unlock long enough to grow the handle
				::HUnlock((Handle) decodeSectionH);
				::SetHandleSize(	(Handle) decodeSectionH,
									(section + 1) * sizeof(SDecodeSection));
				::HLock((Handle) decodeSectionH);
				(*decodeSectionH)[section].buf = nil;
				(*decodeSectionH)[section].addKeysSet = kPGPInvalidRef;
				(*decodeSectionH)[section].sigDataValid = false;
				allocated = true;
			}

			result = PGPIncKeySetRefCount(inEventP->data.keyFoundData.keySet);
			if (result == kPGPError_NoErr) {
				(*decodeSectionH)[section].addKeysSet = inEventP->
						data.keyFoundData.keySet;
			}
		}
		break;
	}
	
	return result;
}
