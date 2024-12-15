/*____________________________________________________________________________
	pgpEncodePriv.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpEncodePriv.h,v 1.29.4.1.2.1 1998/11/12 03:21:41 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpEncodePriv_h	/* [ */
#define Included_pgpEncodePriv_h

#include "pgpPubTypes.h"
#include "pgpEncode.h"
#include "pgpFileMod.h"
#include "pgpFileRef.h"
#include "pgpOptionListPriv.h"
#include "pgpTextFilt.h"

#include "pgpEncPipe.h"			/* for pgpLiteralParams */

typedef PGPFileReadCallBack PGPProgressCallBack;


#define IsOp(x)		((PGPBoolean)((x).type!=kPGPOptionType_InvalidOption))
#define IsntOp(x)	((PGPBoolean)((x).type==kPGPOptionType_InvalidOption))



struct PGPJob
{
	PGPContextRef		context;
	PGPOptionListRef	newOptionList;
};
typedef struct PGPJob	PGPJob;


PGP_BEGIN_C_DECLARATIONS


PGPError 	pgpEncodeInternal(PGPContextRef context,
						PGPOptionListRef optionList);
PGPError 	pgpDecodeInternal(PGPContextRef context,
						PGPOptionListRef optionList);


/* Helper functions for Encode/Decode */

PGPError	pgpPumpMem(PGPPipeline *head, PGPByte *inBufPtr,
						PGPSize inBufLength, PGPProgressCallBack callBack,
						void *callBackArg);

PGPError	pgpWarnUser(PGPContextRef context, PGPOptionListRef optionList,
						PGPError errCode, void *warnArg);

PGPError	pgpGetMinValidity(PGPOptionListRef optionList,
						PGPValidity *failValidity, PGPValidity *warnValidity);

PGPError	pgpCheckKeyValidity(PGPContextRef context,
						PGPOptionListRef optionList, PGPKey *key,
						RingSet const *ringSet, PGPValidity failValidity,
						PGPValidity warnValidity, PGPKeySet *warnKeySet,
						PGPValidity *pValidity);

PGPError	pgpGetMemOutput(PGPPipeline *pipeBuf, PGPSize bufferSize,
						PGPSize *outputBufferLen);

PGPError	pgpGetVariableMemOutput(PGPPipeline *pipeBuf, PGPSize maxSize,
						PGPByte **outputBufferPtr, PGPSize *outputBufferLen);

PGPError	pgpGetPGPMIMEBodyOffset(PGPPipeline *pipeBuf,
						PGPOptionListRef optionList);

PGPLineEndType	pgpGetDefaultLineEndType (void);

PGPFileOpenFlags	pgpLocalEncodingToFlags(PGPUInt32 localEncode);

PGPError	pgpSetupCallback(PGPOptionListRef optionList,
						PGPEventHandlerProcPtr *func,
						PGPUserValue *userValue, PGPBoolean *fNullEvents);

PGPError	pgpSetupInput(PGPContextRef context, PGPOptionListRef optionList,
						PgpLiteralParams *literal, PGPRandomContext *rng,
						PGPBoolean inputMode, PGPBoolean detachedSignature,
						PFLConstFileSpecRef *inFileRef, PGPFileRead **pfrin,
						PGPFileDataType *inFileDataType,
						PGPByte **inBufPtr, PGPSize *inBufLength);

PGPError	pgpSetupOutputPipeline(PGPContextRef context,
						PGPOptionListRef optionList, PGPEnv *env,
						PGPBoolean fEncrypt, PGPBoolean fSign,
						PGPBoolean fDetachedSig, PGPBoolean fAppendOutput,
						PGPPipeline ***tail,
						PFLConstFileSpecRef *outFileRef, PGPFile **pfout,
						PGPByte **outBufPtr, PGPByte ***outBufPtrPtr,
						PGPSize *outBufLength, PGPSize **outBufUsed,
						PGPPipeline **outPipe);


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpEncodePriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
