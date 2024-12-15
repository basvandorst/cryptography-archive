/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPEncoderDecoder.h,v 1.14.8.1 1997/12/05 22:14:40 mhw Exp $
____________________________________________________________________________*/

#pragma once 


#include "PGPUserInterface.h"
#include "MacCursors.h"
#include "CString.h"
#include "PGPEncode.h"
#include "PassphraseCache.h"


typedef struct SDecodeSection {
	void *					buf;
	PGPSize					size;
	PGPKeySetRef			addKeysSet;
	PGPEventSignatureData	sigData;
	Boolean					sigDataValid;
} **SDecodeSectionHandle;

enum {
	encodeOptions_Encrypt			= (1L << 0),
	encodeOptions_Sign				= (1L << 1),
	encodeOptions_TextOutput		= (1L << 2),
	encodeOptions_TreatInputAsText	= (1L << 3),
	encodeOptions_ClearSign			= (1L << 4),
	encodeOptions_ConvEncrypt		= (1L << 5)
};

typedef OptionBits	EEncodeOptions;


class CPGPEncoderDecoder {
public:
	static void						Init();
	static void						Cleanup();
	
	static Boolean					EncodeHandle(PGPContextRef inContext,
										Handle ioDataH,
										EEncodeOptions inOptions,
										UInt32 inNumRecipients = 0,
										const PGPKeySpec * inRecipientList
											= nil,
										Boolean inUseCache = false);
	static Boolean					DecodeHandle(PGPContextRef inContext,
										Handle inDataH,
										PGPKeySetRef inKeySet,
										SDecodeSectionHandle &
												outDecodeSectionsHandle,
										Boolean inUseCache = false);
								
	static void						FreeDecodeSection(
										SDecodeSection * inSection);
	
protected:
	static Boolean					sCheckForMissingKeys;
	static CSignPassphraseCache *	sSigningPassphraseCache;
	static CSignPassphraseCache *	sDecryptPassphraseCache;
	
								
	static PGPOptionListRef			CreateOptionList(
										PGPContextRef inContext,
										EEncodeOptions inOptions,
										PGPKeySetRef inRecipients,
										PGPKeyRef inSigningKey,
										char * inSigningPassphrase,
										char * inConvEncryptPassphrase);
													
	static void						SetPassphraseCache(
										Boolean inSigning,
										PGPKeyRef inKey,
										char * inPassphrase);

	static void						ShowError(PGPError inErr);
	
	static PGPError					PGPEncodeEventHandler(
										PGPContextRef inContext,
										PGPEvent * inEventP,
										PGPUserValue inUserValue)
										{ AnimateCursor((AnimatedCursorRef)
												inUserValue);
										  return kPGPError_NoErr; }
	static PGPError					PGPDecodeEventHandler(
										PGPContextRef inContext,
										PGPEvent * inEventP,
										PGPUserValue inUserValue);
};

const ResIDT	acur_BeachBall						=	12121;

