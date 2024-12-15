/*____________________________________________________________________________
	pgpDecode.c
	High level decode functionality
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpDecode.c,v 1.97.2.2.2.1 1998/11/12 03:21:37 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"	/* or pgpConfig.h in the CDK */

#include <string.h>

/* Public headers */
#include "pgpPubTypes.h"
#include "pgpContext.h"
#include "pgpEncode.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpMem.h"

/* Private headers */
#include "pgpSDKBuildFlags.h"
#include "pgpDebug.h"
#include "pgpEncodePriv.h"
#include "pgpAnnotate.h"
#include "pgpArmor.h"
#include "pgpBufMod.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpConvKey.h"
#include "pgpDecPipe.h"
#include "pgpDevNull.h"
#include "pgpEnv.h"
#include "pgpESK.h"
#include "pgpEventPriv.h"
#include "pgpFIFO.h"
#include "pgpFile.h"
#include "pgpFileMod.h"
#include "pgpFileRef.h"
#include "pgpFileSpec.h"
#include "pgpFileType.h"
#include "pgpHash.h"
#include "pgpKeyDB.h"
#include "pgpKeyIDPriv.h"
#include "pgpKDBInt.h"
#include "pgpMem.h"
#include "pgpMemMod.h"
#include "pgpOptionList.h"
#include "pgpPipeline.h"
#include "pgpPubKey.h"
#include "pgpRandomPoolPriv.h"
#include "pgpRngPub.h"
#include "pgpSig.h"
#include "pgpTextFilt.h"
#include "pgpTrstPkt.h"
#include "pgpUI.h"
#include "pgpVMemMod.h"

#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

/************************** Types and Constants ***************************/


/* All state information for pgpDecode is kept in a struct like this */
struct PGPDecodeJob_
{
	PGPContextRef		 context;		/* Context pointer */
	PGPOptionListRef	 optionList;	/* List of all our options */
	PGPOptionListRef	 newOptionList;	/* New options from user callback */
	PGPUICb              ui;			/* Callback functions */
	PGPError			 err;			/* Error */
	PGPEnv				*env;			/* Environment for low-level fns */
	PGPRandomContext	*rng;			/* Random state */
	PGPOption			 op;			/* Selected option from list */
	PGPBoolean			 fUsedDetachedSigOp;  /* Have used detachedsig op */
	PGPPipeline			*head, **tail;	/* Low level pipeline */
	PGPPipeline			*outPipe;		/* Dynamic memory output module */
	PGPFile				*pfout;			/* Output file handle */
	PGPPipeline			*outKey;		/* Key data output module */
	PGPPipeline			*prevStarOutput;/* Deferred tail of output pipeline */
	PGPPipeline		   **prevOutput;	/* Address of pointer to pipe tail */
	PGPPipeline			*prevOutPipe;	/* Dynamic memory mod for prevoutput */
	PGPFile				*prevPFout;		/* Output file handle for prevoutput */
	PGPBoolean			 fPrevOutput;	/* Have set prevOutput */
	PFLConstFileSpecRef	 inFileRef;		/* Input filename handle */
	PGPFileRead			*pfrin;			/* Input file reading structure */
	PGPByte				*inBufPtr;		/* Input buffer pointer */
	PGPSize				 inBufLength;	/* Size of input buffer */
	PFLFileSpecRef		 outFileRef;	/* Output filename handle */
	PGPByte				*outBufPtr;		/* Output buffer pointer */
	PGPByte			   **outBufPtrPtr;	/* Dynamically allocated buf ptr */
	PGPSize				 outBufMaxLength; /* Allocated size of outBufPtr */
	PGPSize				*outBufUsedLength; /* Amount output to outBufPtr */
	PGPBoolean			 outDiscard;	/* True if want to discard output */
	PGPBoolean			 fixedOutput;	/* Use same output throughout */
	PGPBoolean			 fAppendOutput;	/* Append output to buffer or file */
	PGPEventHandlerProcPtr func;		/* Pointer to user callback func */
	PGPUserValue		 userValue;		/* Arg to callback func */
	PGPBoolean			 fNullEvents;	/* True if user wants null events */
	PGPUInt32			 localEncodeFlags; /* Macbinary etc. for output */
	PGPLineEndType		 lineEnd;		/* Line endings for text output */
	PGPKeySetRef		 keySet;		/* Keyset to check sigs, decrypt */
	PGPByte				*passPhrase;	/* Pass phrase from user */
	PGPSize				 passLength;	/* Length of passPhrase */
	PGPBoolean			 hashedPhrase;	/* True if given passkey */
	PGPUInt32			sectionNumber;	/* Number of section we are on */
	PGPAnalyzeType		analyzeType;	/* Type of section we found */
	PGPInt32			analyzeState;	/* kAnalyze status */
	PGPInt32			scopeLevel;	/* Nesting scope */
	PGPInt32			scopeSegment;	/* scopeLevel at start of segment */
	PGPSize				sectOffset;		/* Offset in input to start of sect */
	PGPBoolean			passThrough;	/* True if outputing without change */
	PGPFifoContext		*passThroughFifo; /* Accumulate some passthrough data*/
	PGPByte				literalType;	/* type of literal packet */
	PGPBoolean			recurse;		/* Recursing on decode */
};
typedef struct PGPDecodeJob_ PGPDecodeJob;

/* analyzeState values */
enum {
	kAnalyzeWaiting = 1,
	kAnalyzeGotType,
	kAnalyzeSegmentEndWait
};


/*********** Functions to set up data structures for pipeline *************/


/* Parse output specifications */

/*
 * Note that any outFileRef returned will be a fresh copy, which the
 * caller is responsible for freeing after use.
 */
	static PGPError
pgpSetupOutput(
	PGPOptionListRef	  optionList,
	PGPEnv				 *env,
	PFLFileSpecRef		 *outFileRef,	/* Output params */
	PGPByte				**outBufPtr,
	PGPByte			   ***outBufPtrPtr,
	PGPSize				 *outBufLength,
	PGPSize				**outBufUsed,
	PGPBoolean			 *outDiscard,
	PGPUInt32			 *localEncodeFlags,
	PGPLineEndType		 *lineEnd,
	PGPBoolean			 *pAppendOutput
	)
{
	PGPOption			 op;			/* Selected option from list */
	PGPError			 err;			/* Error return code */
	PGPUInt32			 localEncode;	/* Enum for macbinary, etc. */
	PFLConstFileSpecRef	 lOutFileRef;	/* Local copy of outfileref */
	PGPFileSpecRef	 	 lOutFileRefPGP;/* Another local copy of outfileref */
	PGPUInt32			 fDiscard;		/* Discard output option flag */
	PGPUInt32			 fAppendOutput;	/* Append to output flag */

	(void) env;

	/* Init return data to default states */
	pgpa( pgpaAddrValid( outFileRef, PFLConstFileSpecRef ) );
	pgpa( pgpaAddrValid( outBufPtr, PGPByte * ) );
	pgpa( pgpaAddrValid( outBufLength, PGPSize ) );
	pgpa( pgpaAddrValid( outBufUsed, PGPSize * ) );
	pgpa( pgpaAddrValid( outDiscard, PGPBoolean ) );
	pgpa( pgpaAddrValid( localEncodeFlags, PGPUInt32 ) );
	pgpa( pgpaAddrValid( lineEnd, PGPLineEndType ) );
	*outFileRef = NULL;
	*outBufPtr = NULL;
	*outBufLength = 0;
	*outBufUsed = NULL;
	*outDiscard = FALSE;
	*localEncodeFlags = 0;
	*lineEnd = pgpGetDefaultLineEndType ();
	*pAppendOutput = FALSE;

	/* Test for append flag */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_AppendOutput, FALSE,
						 "%d", &fAppendOutput ) ) )
		goto error;
	*pAppendOutput = (PGPBoolean)fAppendOutput;

	/* See if there is an output file specified */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_OutputFileRef, &op ) ) )
		goto error;

	if( IsOp( op ) ) {
		/* Have an output file specified */
		if( IsPGPError( err = pgpOptionPtr( &op, (void **)&lOutFileRefPGP ) ) )
			goto error;
		lOutFileRef = (PFLConstFileSpecRef) lOutFileRefPGP;
		if( IsPGPError( err = PFLCopyFileSpec( lOutFileRef, outFileRef ) ) )
			goto error;
	}

	/* See if there is an output buffer specified */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_OutputBuffer, &op ) ) )
		goto error;

	if( IsOp( op ) ) {
		/* Have an output buffer specified */
		if( IsntNull( *outFileRef ) ) {
			pgpDebugMsg( "Error: multiple output options" );
			err = kPGPError_BadParams;
			goto error;
		}
	
		if( IsPGPError( err = pgpOptionPtrLengthPtr( &op, (void **)outBufPtr,
								outBufLength, (void **)outBufUsed ) ) )
			goto error;

	}

	/* Check for variable-sized output buffer specification */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_OutputAllocatedBuffer, &op ) ) )
		goto error;

	if( IsOp( op ) ) {
		/* Have an output buffer specified */
		if( IsntNull( *outFileRef ) || IsntNull( *outBufPtr ) ) {
			pgpDebugMsg( "Error: multiple output options" );
			err = kPGPError_BadParams;
			goto error;
		}
	
		if( IsPGPError( err = pgpOptionPtrLengthPtr( &op,
				(void **)outBufPtrPtr, outBufLength, (void **)outBufUsed ) ) )
			goto error;
	}

	/* Check for request to discard output (send to devnull module) */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_DiscardOutput, FALSE,
						 "%d", &fDiscard ) ) )
		goto error;
	if( fDiscard ) {
		/* User wants to go to /dev/null */
		if( IsntNull( *outFileRef ) || IsntNull( *outBufPtr ) ||
			IsntNull( *outBufPtrPtr ) ) {
			pgpDebugMsg( "Error: multiple output options" );
			err = kPGPError_BadParams;
			goto error;
		}
		*outDiscard = TRUE;
	}

	/* Read output local encoding and line endings */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
						  kPGPOptionType_LocalEncoding, &op ) ) )
			goto error;
	if( IsOp( op ) ) {
		if( IsPGPError( err = pgpOptionUInt( &op, &localEncode ) ) )
			goto error;
		*localEncodeFlags = pgpLocalEncodingToFlags( localEncode );
	}
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
						  kPGPOptionType_OutputLineEndType, &op ) ) )
		goto error;
	if( IsOp( op ) ) {
		PGPUInt32 uintLineEnd;
		if( IsPGPError( err = pgpOptionUInt( &op, &uintLineEnd ) ) )
			goto error;
		*lineEnd = (PGPLineEndType) uintLineEnd;
	}

	return kPGPError_NoErr;

error:

	*outFileRef = NULL;
	*outBufPtr = NULL;
	*outBufPtrPtr = NULL;
	*outBufLength = 0;
	*outDiscard = FALSE;
	*localEncodeFlags = 0;
	*lineEnd = pgpGetDefaultLineEndType();
	return err; 
}


/* Get keyring set from user if specified */

	static PGPError
pgpSetupKeySet(
	PGPOptionListRef	  optionList,
	PGPKeySetRef		 *keySet		/* Output params */
	)
{
	PGPOption			 op;			/* Selected option from list */
	PGPError			 err;			/* Error return code */

	/* Init return data to default states */
	pgpa( pgpaAddrValid( keySet, PGPKeySetRef ) );
	*keySet = NULL;

	/* See if there is a keyset specified */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_KeySetRef, &op ) ) )
		goto error;

	if( IsOp( op ) ) {
		if( IsPGPError( err = pgpOptionPtr( &op, (void **)keySet ) ) )
			goto error;
	}

	return kPGPError_NoErr;

error:
	*keySet = NULL;
	return err;
}
	
/*
 * Get passphrase from user.  We make a copy of the passphrase because if
 * it is specified from a callback, when we free the optionlist it will
 * go away.
 */

	static PGPError
pgpSetupDecodePassphrase(
	PGPContextRef	 	  context,		/* Input params */
	PGPOptionListRef	  optionList,
	PGPByte				**passPhrase,	/* Output params */
	PGPSize				 *passLength,
	PGPBoolean			 *hashedPhrase
	)
{
	void				*vPassPhrase;	/* Pointer to option pphrase */
	PGPByte				*lPassPhrase;	/* Local allocated passphrase */
	PGPSize				 lPassLength;	/* Local copy of pphrase length */
	PGPError			 err;			/* Error return code */

	/* Init return data to default states */
	pgpa( pgpaAddrValid( passPhrase, PGPByte * ) );
	pgpa( pgpaAddrValid( passLength, PGPSize ) );
	pgpa( pgpaAddrValid( passLength, PGPBoolean ) );
	*passPhrase = NULL;
	*passLength = (PGPSize)0;
	*hashedPhrase = FALSE;

	/* See if there is a pass phrase specified */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &vPassPhrase, &lPassLength ) ) )
		goto error;
	if( IsNull( vPassPhrase ) ) {
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							 kPGPOptionType_Passkey, FALSE,
							 "%p%l", &vPassPhrase, &lPassLength ) ) )
			goto error;
		if( IsntNull( vPassPhrase ) )
			*hashedPhrase = TRUE;
	}
	if( IsntNull( vPassPhrase ) ) {
		lPassPhrase = (PGPByte *)
			PGPNewSecureData(PGPGetContextMemoryMgr(context), lPassLength, 0);
		if( IsNull( lPassPhrase ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
		pgpCopyMemory( vPassPhrase, lPassPhrase, lPassLength );
		*passPhrase = lPassPhrase;
		*passLength = lPassLength;
	}
	return kPGPError_NoErr;

error:
	*passPhrase = NULL;
	*passLength = (PGPSize)0;
	return err;
}


/*
 * Handle options requesting that we pass clearsigned data and/or keys
 * through to the output, unchanged.  We still do the signature checking
 * on signed text, and we still do the key callbacks.  This just affects
 * what goes into the output.
 */

	static PGPError
pgpSetupPassThrough(
	PGPContextRef	 	  context,		/* Input params */
	PGPOptionListRef	  optionList,
	PGPPipeline			 *pipeHead,
	PGPFifoContext		**fifo,			/* Output params */
	PGPBoolean			*recurse
	)
{
	PGPUInt32			 fPassClear;		/* True if passthroughcleartext */
	PGPUInt32			 fPassKeys;			/* True if passthroughkeys */
	PGPUInt32			 fRecurse;			/* True if recursivelydecode */
	PGPByte				 boolFlag = TRUE;	/* Flag for turning on modes */
	PGPError			 err;				/* Error return code */

	/* Init return data to default states */
	pgpa( pgpaAddrValid( fifo, PGPByte * ) );
	pgpa( pgpaAddrValid( recurse, PGPBoolean ) );
	*fifo = NULL;
	*recurse = FALSE;

	/* Read our passthroughoptions */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_PassThroughClearSigned, FALSE,
						 "%d", &fPassClear ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_PassThroughKeys, FALSE,
						 "%d", &fPassKeys ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_RecursivelyDecode, FALSE,
						 "%d", &fRecurse ) ) )
		goto error;
	*recurse = (PGPBoolean)fRecurse;

	fPassClear |= fRecurse;		/* Recurse needs passthroughclearsigned */

	if( fPassClear || fPassKeys ) {
		*fifo = pgpFifoCreate( context, &pgpByteFifoDesc );
		if (*fifo == NULL) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
	}
	
	if( fPassClear ) {
		pipeHead->annotate( pipeHead, NULL, PGPANN_PASSTHROUGH_CLEARSIGN,
							&boolFlag, 1 );
	}

	if( fPassKeys ) {
		pipeHead->annotate( pipeHead, NULL, PGPANN_PASSTHROUGH_KEYS,
							&boolFlag, 1 );
	}

	return kPGPError_NoErr;

error:
	if( IsntNull( *fifo ) ) {
		pgpFifoDestroy( &pgpByteFifoDesc, *fifo );
		*fifo = NULL;
	}
	return err;
}


/* Burn a passphrase from the job state structure, if any */
	static void
pgpBurnDecodePassphrase ( PGPDecodeJob *s )
{
	if( IsntNull( s->passPhrase ) ) {
		pgpClearMemory( s->passPhrase, s->passLength );
		PGPFreeData( s->passPhrase );
		s->passPhrase = NULL;
		s->passLength = 0;
	}
}


/*********************** Subroutines for callbacks *************************/


/* Close an existing output pipeline and do appropriate cleanup */

	static PGPError
pgpCloseOutput(
	PGPDecodeJob		 *s,
	PGPPipeline			**output
	)
{
	PGPError			  err;

	if( IsntNull( *output ) ) {
		(*output)->sizeAdvise (*output, 0);
		if( IsntNull( s->outPipe ) ) {
			/* Return buffer sizes to user */
			if( IsntNull( s->outBufPtrPtr ) ) {
				/* Dynamically allocated buffer - tell user size & position */
				if( IsPGPError( err = pgpGetVariableMemOutput( s->outPipe,
								s->outBufMaxLength, s->outBufPtrPtr,
								s->outBufUsedLength ) ) )
					goto error;
			} else {
				/* Fixed size buffer - tell user actual size used */
				pgpAssert( IsntNull( s->outBufPtr ) );
				if( IsPGPError( err = pgpGetMemOutput( s->outPipe,
							s->outBufMaxLength, s->outBufUsedLength ) ) )
					goto error;
			}
			s->outPipe = NULL;
		}
		(*output)->teardown (*output);
		*output = NULL;
		if( s->pfout ) {
			pgpFileClose( s->pfout );
			s->pfout = NULL;
		}
	}
	return kPGPError_NoErr;
error:
	return err;
}


/* Close an existing buffer saving a key and do appropriate cleanup */

	static PGPError
pgpCloseKeyOutput(
	PGPDecodeJob		*s
	)
{
	PGPOption			 op;			/* Selected option from list */
	PGPByte				*bufPtr;		/* Buffer pointer for key data */
	PGPSize				 bufLength;		/* Length of key buffer */
	PGPKeySet			*keySet;		/* Keyset for key data */
	PGPKeySet			*importKeySet;	/* Keyset we are adding to for user */
	PGPError			 err;			/* Error code */
	PGPContextRef		cdkContext;

	pgpAssertAddrValid( s, PGPDecodeJob );
	cdkContext	= s->context;

	keySet = NULL;
	importKeySet = NULL;
	bufPtr = NULL;

	/* Get buffer and length where key data is */
	pgpAssert( IsntNull( s->outKey ) );
	if( IsPGPError( err = pgpGetVariableMemOutput( s->outKey,
					(PGPSize)~0, &bufPtr, &bufLength ) ) )
		goto error;

	/* Close down output portion of pipeline */
	s->outKey = NULL;
	pgpAssert( s->fPrevOutput );
	pgpAssert( IsntNull( s->prevOutput ) );
	pgpAssert( IsNull( s->outPipe ) );
	if( IsPGPError( err = pgpCloseOutput(s,  s->prevOutput ) ) )
		goto error;

	/* Translate into a keyset holding any imported key(s) */
	if( IsPGPError( err = pgpImportKeyBinary( s->context,
											  bufPtr, bufLength, &keySet ) ) )
		goto error;
	PGPFreeData( bufPtr );
	bufPtr = NULL;

	/* See if there is a keyset specified for adding keys to */
	if( IsPGPError( err = pgpSearchOptionSingle( s->optionList,
							 kPGPOptionType_ImportKeysTo, &op ) ) )
		goto error;
	if( IsOp( op ) ) {
		/* Add keys to user-specified keyset */
		if( IsPGPError( err = pgpOptionPtr( &op, (void **)&importKeySet ) ) )
			goto error;
		pgpa(pgpaPGPKeySetValid(importKeySet));
		if( !pgpKeySetIsValid(importKeySet) ) {
			pgpDebugMsg( "Error: invalid ImportKeysTo keyset" );
			err = kPGPError_BadParams;
			goto error;
		}
		if( IsPGPError( err = PGPAddKeys( keySet, importKeySet) ) )
			goto error;
		if( IsPGPError( err = PGPCommitKeyRingChanges( importKeySet ) ) )
			goto error;
		PGPFreeKeySet( keySet );
		keySet = NULL;
	} else {
		/* Else see if we should ask user what to do with keys */
		PGPUInt32 fKeyEvents;
		if( IsPGPError( err = pgpFindOptionArgs( s->optionList,
							 kPGPOptionType_SendEventIfKeyFound, FALSE,
							 "%d", &fKeyEvents ) ) )
			goto error;
		if( fKeyEvents ) {
			err = pgpEventKeyFound( s->context, &s->newOptionList,
						s->func, s->userValue, keySet );
			pgpCleanupOptionList( &s->newOptionList );
			if( IsPGPError( err ) )
				goto error;
		}
		/* User has dealt with it if he wants, now we can delete data */
		PGPFreeKeySet( keySet );
		keySet = NULL;
	}
	return kPGPError_NoErr;
error:
	if( IsntNull( bufPtr ) )
		PGPFreeData( bufPtr );
	if( IsntNull( keySet ) )
		PGPFreeKeySet( keySet );
	return err;
}	


/* 
 * Use when we first open output in passthrough mode.  We drain the
 * contents of the passthrough fifo (which holds header data and such
 * which we read before we got the begin annotation).
 */
static void
sDrainPassThroughFifo( PGPDecodeJob *s )
{
	PGPByte const *	ptr;
	PGPSize			length;
	PGPError		err;

	pgpAssert( IsntNull( s->prevStarOutput ) );
	pgpAssert( IsntNull( s->passThroughFifo ) );

	ptr = pgpFifoPeek(&pgpByteFifoDesc, s->passThroughFifo, &length);
	while (length != 0) {
		s->prevStarOutput->write( s->prevStarOutput, ptr, length, &err );
		pgpFifoSeek(&pgpByteFifoDesc, s->passThroughFifo, length);
		ptr = pgpFifoPeek(&pgpByteFifoDesc, s->passThroughFifo, &length);
	}
}


/*
 * Output state diversion.  We sometimes want to suspend output to the
 * current pipeline temporarily while we deal with a key or non-pgp data.
 * We save the current state in the "prev" class of variables, and later
 * restore them.
 */

static void
sRestoreOutputState( PGPDecodeJob *s, PGPPipeline **output )
{
	/*
	 * In order for our cleanup in pgpDecodeInternal to work, the output
	 * tail pointer from the module which calls us must always be the same.
	 * The following assertion helps test for that.  (During pgpDecodeInternal
	 * we have no access to the output tail pointer, so we will assume that
	 * the prevOutput value matches what is needed.)
	 */
	pgpAssert( s->prevOutput == output );
	*output = s->prevStarOutput;
	s->outPipe = s->prevOutPipe;
	s->pfout = s->prevPFout;
	s->prevStarOutput = NULL;
	s->prevOutPipe = NULL;
	s->prevPFout = NULL;
	s->fPrevOutput = FALSE;
}

static void
sSaveOutputState( PGPDecodeJob *s, PGPPipeline **output )
{
	pgpAssert( s->prevOutput == output );
	s->prevStarOutput = *output;
	s->prevOutPipe = s->outPipe;
	s->prevPFout = s->pfout;
	s->fPrevOutput = TRUE;
	s->outPipe = NULL;
	s->pfout = NULL;
	*output = NULL;
}





/***************** Callback functions from Aurora library *******************/

static PGPError pgpDecodeDoCommit(void *arg, PGPInt32 scope);


/*
 * The low-level Aurora library calls our analyze and commit functions
 * for each recursive level of handling of a given message segment.
 * We only want to give one of these to the user.  Accordingly we have
 * three states as far as getting segment-type information:
 *
 *	- Waiting for segment type info
 *	- Got segment type info, waiting to give to user
 * 	- Gave segment info to user, waiting for end of segment
 *
 * We give segment info to the user from the commit call, because that
 * is our opportunity to skip a section, which the user is allowed to do
 * on the analyze call.
 */
	static PGPError
pgpDecodeHandleAnnotation(
	void				*arg,
	PGPPipeline			*origin,
	PGPInt32			 type,
	PGPByte const		*string,
	PGPSize				 size
	)
{
	PGPDecodeJob		*s;				/* Parameters for callbacks */
	PGPUInt32			 passThroughNonPGP;
	PGPError			 err = kPGPError_NoErr;

	s = (PGPDecodeJob *) arg;
	(void) origin;
	(void) string;
	(void) size;

	/* Keep track of our nesting status */
	PGP_SCOPE_DEPTH_UPDATE(s->scopeLevel, type);

	/* Handle passthrough data */
	if (type == PGPANN_CLEARDATA) {
		/* Passthrough data gets buffered or handed to suspended output */
		s->passThrough = TRUE;
		if( IsntNull( s->prevStarOutput ) ) {
			s->prevStarOutput->write( s->prevStarOutput,
									  string, size, &err );
		} else {
			/* We get a few lines of header before the data starts,
			 * so we buffer that here and will drain it in
			 * NewOutput.
			 */
			pgpAssert( IsntNull( s->passThroughFifo ) );
			pgpFifoWrite(&pgpByteFifoDesc, s->passThroughFifo,
						 string, size);
		}
	} else if( type == PGPANN_LITERAL_TYPE ) {
		/* Remember type in case we want to recurse into it */
		pgpAssert (size == 1);
		s->literalType = *(PGPByte *)string;
	} else if( s->analyzeState == kAnalyzeWaiting ) {
		/* If looking for the start of a segment, figure out the type */
		switch (type) {
		case PGPANN_ARMOR_BEGIN:
			/* Just get the section offset for later callback */
			if( size == sizeof(s->sectOffset) )
				s->sectOffset = *(PGPSize *)string;
			break;
		case PGPANN_PGPKEY_BEGIN:
			s->analyzeType = kPGPAnalyze_Key;
			s->analyzeState = kAnalyzeGotType;
			s->scopeSegment = s->scopeLevel;
			break;
		case PGPANN_CIPHER_BEGIN:
			s->analyzeType = kPGPAnalyze_Encrypted;
			s->analyzeState = kAnalyzeGotType;
			s->scopeSegment = s->scopeLevel;
			break;
		case PGPANN_COMPRESSED_BEGIN:
		/* Signed may be a detached or a regular sig, need another anno */
		/*		case PGPANN_SIGNED_BEGIN: */
		case PGPANN_SIGNED_SIG:
		case PGPANN_SIGNED_SIG2:
			s->analyzeType = kPGPAnalyze_Signed;
			s->analyzeState = kAnalyzeGotType;
			s->scopeSegment = s->scopeLevel;
			break;
		case PGPANN_SIGNED_SEP:
			s->analyzeType = kPGPAnalyze_DetachedSignature;
			s->analyzeState = kAnalyzeGotType;
			s->scopeSegment = s->scopeLevel;
			break;
		case PGPANN_CLEARSIG_BEGIN:
			/*
			 * Clearsig doesn't go through binary parser, so we do some
			 * shortcuts here.  Call commit directly.  We can't currently
			 * skip clearsigned messages though.  Note commit changes
			 * analyzeState.
			 */
			s->analyzeType = kPGPAnalyze_Signed;
			s->analyzeState = kAnalyzeGotType;
			s->scopeSegment = s->scopeLevel;
			if( size == sizeof(s->sectOffset) )
				s->sectOffset = *(PGPSize *)string;
			err = pgpDecodeDoCommit(arg, type);
			if (err == PGPANN_PARSER_EATIT || err == PGPANN_PARSER_RECURSE)
				err = kPGPError_NoErr;
			break;
		case PGPANN_NONPGP_BEGIN:
			/*
			 * Non-PGP data is handled differently.  Call commit directly.
			 * (Skip not supported though.)  Note commit changes
			 * analyzeState.
			 */
			if( IsPGPError( pgpFindOptionArgs( s->optionList,
							kPGPOptionType_PassThroughIfUnrecognized, FALSE,
							"%d", &passThroughNonPGP ) ) )
				goto error;
			if( passThroughNonPGP ) {
				s->analyzeType = kPGPAnalyze_Unknown;
				s->analyzeState = kAnalyzeGotType;
				s->scopeSegment = s->scopeLevel;
				if( size == sizeof(s->sectOffset) )
					s->sectOffset = *(PGPSize *)string;
				err = pgpDecodeDoCommit(arg, type);
				if (err == PGPANN_PARSER_EATIT || err == PGPANN_PARSER_RECURSE)
					err = kPGPError_NoErr;
			}
			break;
		}
	} else if( s->analyzeState == kAnalyzeSegmentEndWait ) {
		/*
		 * Handle end of segment.
		 * Special treatment for clearsig: it goes CLEARSIG_BEGIN,
		 * CLEARSIG_END, ARMOR_BEGIN, ARMOR_END.  We skip the CLEARSIG_END
		 * so we terminate the scope at the ARMOR_END.
		 */
		if( type == PGPANN_PGPKEY_END ) {
			/* On close of PGPkey segment, deal with key data */
			/* May be nested within an encrypted segment so check here */
			if( IsPGPError( err = pgpCloseKeyOutput( s ) ) )
				goto error;
			pgpAssert( s->fPrevOutput );
			sRestoreOutputState( s, s->prevOutput );
		}
		if( s->scopeLevel < s->scopeSegment && type != PGPANN_CLEARSIG_END) {
			/* Close any open output so client can see his data */
			if( !s->fixedOutput && IsntNull( s->prevOutput ) ) {
				/* If finished with a segment, close it */
				if( IsPGPError( err = pgpCloseOutput(s,  s->prevOutput )))
					goto error;
			}
			s->passThrough = FALSE;		/* End of segment */
			err = pgpEventEndLex( s->context, &s->newOptionList,
								  s->func, s->userValue, s->sectionNumber++ );
			pgpCleanupOptionList( &s->newOptionList );
			s->analyzeState = kAnalyzeWaiting;
			if( IsPGPError( err ) )
				goto error;
			/* Handle one-section mode */
			if( s->sectionNumber == 1 ) {
				PGPUInt32 fOnlyOne;
				if( IsPGPError( err = pgpFindOptionArgs( s->optionList,
									kPGPOptionType_DecodeOnlyOne, FALSE,
									"%d", &fOnlyOne ) ) )
					goto error;
				if( fOnlyOne ) {
					err = kPGPError_Interrupted;
				}
			}
		}
	}
	/* Detect bad parsing situations */
	if (type == PGPANN_ESK_TOO_BIG
		|| type == PGPANN_SIGNATURE_TOO_BIG
		|| type == PGPANN_PACKET_SHORT
		|| type == PGPANN_PACKET_TRUNCATED
		|| type == PGPANN_SCOPE_TRUNCATED) {
		err = kPGPError_BadPacket;
		goto error;
	} else if (type == PGPANN_ARMOR_TOOLONG
		|| type == PGPANN_ARMOR_BADLINE
		|| type == PGPANN_ARMOR_NOCRC
		|| type == PGPANN_ARMOR_CRCCANT
		|| type == PGPANN_ARMOR_CRCBAD) {
		err = kPGPError_AsciiParseIncomplete;
		goto error;
	}
	/* Fall through */
error:
	return err;
}


/*
 * Display a message as appropriate.
 * Nothing is appropriate for us, as we have no UI.  Assert an error
 * if it happens.  (Perhaps better simply to ignore it.)
 */
	static PGPError
pgpDecodeShowMessage(
	void				*arg,
	PGPInt32			 type,
	PGPInt32			 msg,
	PGPUInt32			 numargs,
	...
	)
{
	PGPDecodeJob		*s;				/* Parameters for callbacks */

	s = (PGPDecodeJob *) arg;

	(void)type;
	(void)msg;
	(void)numargs;
	return kPGPError_NoErr;
}


/*
 * Decide whether to work on a section or to skip it.
 */
	static PGPError
pgpDecodeDoCommit(
	void				*arg,
	PGPInt32			 scope
	)
{
	PGPDecodeJob		*s;				/* Parameters for callbacks */
	PGPError			 err;			/* Error code */

	s = (PGPDecodeJob *) arg;

	if( s->analyzeState == kAnalyzeGotType ) {
		/* Tell user of results, let him say if we should continue */
		s->analyzeState = kAnalyzeSegmentEndWait;

		err = pgpEventBeginLex( s->context, &s->newOptionList,
								s->func, s->userValue, s->sectionNumber,
								s->sectOffset );
		pgpCleanupOptionList( &s->newOptionList );
		if( IsPGPError( err ) )
			return err;

		/* Give user analysis callback, let him tell us to skip section */
		err = pgpEventAnalyze( s->context, &s->newOptionList,
								s->func, s->userValue, s->analyzeType );
		pgpCleanupOptionList( &s->newOptionList );
		if( err == kPGPError_SkipSection ) {
			return PGPANN_PARSER_EATIT;
		}
		if( IsPGPError( err ) )
			return err;
	}
	/* Don't recurse into literal recursion packets unless set up for it */
	/* Must be in recurse and passthrough mode to do it, else don't recurse */
	if (scope == PGPANN_LITERAL_BEGIN && s->literalType == PGP_LITERAL_RECURSE
		&& (s->passThroughFifo == NULL || !s->recurse))
		return PGPANN_PARSER_PROCESS;
	return PGPANN_PARSER_RECURSE;
}

/* Set up the output pipeline as appropriate */

	static PGPError
pgpDecodeSetupNewOutput(
	void				 *arg,
	PGPPipeline			**output,
	PGPInt32			  type,
	char const			 *suggested_name
	)
{
	PGPDecodeJob		 *s;			/* Parameters for callbacks */
	PGPError			  err;			/* Error code */
	PGPUInt32			  outputCount;	/* How many output options we got */
	PGPFileOpenFlags	  fileFlags;	/* Flags for opening output file */
	PGPFileType			  fileType;		/* Type field for output file */
	PGPByte				 *charMap;		/* Charmap for output text filter */
	PGPUInt32			  passThroughNonPGP;
	PGPUInt32			 dummyLocalEncodeFlags;
	PGPLineEndType		 dummyLineEnd;
	PGPContextRef		cdkContext	= NULL;
	PGPMemoryMgrRef		memoryMgr	= NULL;
	PGPBoolean			fFYEO = FALSE;
	
	s = (PGPDecodeJob *) arg;
	err = kPGPError_NoErr;
	cdkContext	= s->context;
	memoryMgr	= PGPGetContextMemoryMgr( cdkContext );

	/*
	 * We need to adjust the pipeline at other stages, so we will save
	 * the output pointer.  For those to work, it must always be the
	 * same whenever we are called.  (The output pointer is the _address_
	 * of a pointer to the output portion of the pipeline, not the pointer
	 * to the pipeline itself.)
	 */
	if( IsNull( s->prevOutput ) )
		s->prevOutput = output;
	pgpAssert( s->prevOutput == output );

	/* If have a prevoutput pipeline saved, switch back to that first. */
	if( s->fPrevOutput ) {
		if( IsPGPError( err = pgpCloseOutput( s, output ) ) )
			goto error;
		sRestoreOutputState( s, output );
	}

	if( type == PGPANN_PGPKEY_BEGIN && !s->passThrough ) {
		/*
		 * Here we are adding a key.  We must copy the data somewhere,
		 * and then convert it to a key set and then add it.
		 * We handle keys in passthrough mode below.
		 */
		sSaveOutputState( s, output );
		if ( IsNull( s->outKey = pgpVariableMemModCreate( cdkContext,
					output, ~(PGPSize)0 ) ) ) {
			/* Guess at the cause */
			err = kPGPError_OutOfMemory;
		}
		/* Return with success or failure */
		goto error;
	}

	if( type == PGPANN_NONPGP_BEGIN ) {
		if( IsPGPError( pgpFindOptionArgs( s->optionList,
						kPGPOptionType_PassThroughIfUnrecognized, FALSE,
						"%d", &passThroughNonPGP ) ) )
			goto error;
		if( !passThroughNonPGP ) {
			/*
			 * Trash non-PGP data if not requesting passthrough.
			 * We will use the prevOutput mechanism to go back to what our
			 * output was before when we are through with it.
			 */
			sSaveOutputState( s, output );
			if ( IsNull( pgpDevNullCreate ( cdkContext, output ) ) ) {
				/* Guess at the cause */
				err = kPGPError_OutOfMemory;
			}
			/* Return with success or failure */
			goto error;
		}
	}

	/* If have fixed output, just leave it open */
	if( s->fixedOutput && IsntNull( *output ) ) {
		/* Leave things open */
		goto checkpassthrough;
	}

	/* Close previous output if any */
	if( IsPGPError( err = pgpCloseOutput( s, output ) ) )
		goto error;
		
	if( !s->fixedOutput ) {
		/* Get output for next part from user */
		pgpAssert( IsntNull( s->func ) );
		fFYEO = IsntNull( suggested_name ) &&
				0==strcmp( suggested_name, "_CONSOLE" );
		/* Turn empty names and magic ones into a null suggested_name */
		if( IsntNull( suggested_name ) &&
			( fFYEO || *suggested_name == '\0' ) )
			suggested_name = NULL;
		if( IsPGPError (err = pgpEventOutput( s->context, &s->newOptionList,
							  s->func, s->userValue,
							  type, suggested_name, fFYEO ) ) )
			goto error;
		/* Parse new output information if any */
		if( IsntNull( s->outFileRef ) ) {
			PFLFreeFileSpec( s->outFileRef );
			s->outFileRef = NULL;
		}
		err = pgpSetupOutput( s->newOptionList,
							s->env, &s->outFileRef,
							&s->outBufPtr, &s->outBufPtrPtr,
							&s->outBufMaxLength, &s->outBufUsedLength,
							&s->outDiscard, &dummyLocalEncodeFlags,
							&dummyLineEnd, &s->fAppendOutput );
		pgpCleanupOptionList( &s->newOptionList );
		if( IsPGPError( err ) )
			goto error;
		/* Make sure he gave us exactly one output option */
		outputCount = !!IsntNull( s->outFileRef ) +
					  !!IsntNull( s->outBufPtr ) +
					  !!IsntNull( s->outBufPtrPtr ) +
					  !!s->outDiscard;
		if( outputCount == 0 ) {
			pgpDebugMsg( "Error: no output options" );
			err = kPGPError_BadParams;
			goto error;
		} else if( outputCount > 1 ) {
			pgpDebugMsg( "Error: multiple output options" );
			err = kPGPError_BadParams;
			goto error;
		}
	}

	/* No support for other types than text or binary now */
	if( type == PGPANN_NONPGP_BEGIN ) {
		/* Treat non-PGP data similarly to text input data */
		type = PGP_LITERAL_TEXT;
	} else if( type != PGP_LITERAL_TEXT ) {
		type = PGP_LITERAL_BINARY;
	}

	if( type==PGP_LITERAL_TEXT ) {
		/* Convert to local line endings if appropriate */
		charMap = (PGPByte *)pgpenvGetPointer( s->env, PGPENV_CHARMAPTOLATIN1,
											   NULL );
		output = pgpTextFiltCreate( s->context,
			output, charMap, 0, s->lineEnd );
	}

	/* Now open output modules as needed */
	if( IsntNull( s->outFileRef ) ) {
		/* Open output file */
		pgpAssert((s->localEncodeFlags & ~(kPGPFileOpenMaybeLocalEncode
									  | kPGPFileOpenNoMacBinCRCOkay)) == 0);

		/* Create output file pipeline */
		fileFlags = s->fAppendOutput ? kPGPFileOpenStdAppendFlags :
									   kPGPFileOpenStdWriteFlags ;
		fileFlags |=  s->localEncodeFlags;
	#if PGP_MACINTOSH
		/* always force decoding of MacBinary */
		fileFlags	|= kPGPFileOpenForceLocalEncode;
	#endif
		fileType =  type == PGP_LITERAL_TEXT ?
								kPGPFileTypeDecryptedText :
								kPGPFileTypeDecryptedBin;
		s->pfout = pgpFileRefOpen( cdkContext,
						(PFLConstFileSpecRef)s->outFileRef,
						fileFlags, fileType, &err );
		if( IsNull( s->pfout ) )
			goto error;
		
		/* Set up output pipeline */
		if( IsNull( pgpFileWriteCreate( cdkContext,
				output, s->pfout, 0 ) ) ) {
			/* Guess at the cause */
			err = kPGPError_OutOfMemory;
			pgpFileClose( s->pfout );
			goto error;
		}
	} else if( IsntNull( s->outBufPtr ) ) {
		/* Open fixed-size memory buffer */
		if ( IsNull( s->outPipe = pgpMemModCreate( cdkContext, output,
						(char *)s->outBufPtr, s->outBufMaxLength ) ) ) {
			/* Guess at the cause */
			err = kPGPError_OutOfMemory;
			goto error;
		}
		if( s->fAppendOutput ) {
			/* Skip past existing buffer contents */
			if( IsPGPError( err = (s->outPipe)->annotate( s->outPipe, NULL,
					 PGPANN_MEM_PREPEND,
					 (unsigned char *)s->outBufPtr, *s->outBufUsedLength ) ) )
				goto error;
		}
	} else if( IsntNull( s->outBufPtrPtr ) ) {
		/* Open variable-size memory buffer */
		/* Use secure mode if a FYEO buffer */
		s->outPipe = fFYEO	? pgpSecureVariableMemModCreate( cdkContext,
												output, s->outBufMaxLength )
							: pgpVariableMemModCreate( cdkContext,
												output, s->outBufMaxLength );
		
		if ( IsNull( s->outPipe ) ) {
			/* Guess at the cause */
			err = kPGPError_OutOfMemory;
			goto error;
		}
		if( s->fAppendOutput && *s->outBufUsedLength != 0 ) {
			/* Prepend existing buffer contents */
			if( IsPGPError( err = (s->outPipe)->annotate( s->outPipe, NULL,
					 PGPANN_MEM_PREPEND, (unsigned char *)*s->outBufPtrPtr,
					 *s->outBufUsedLength ) ) )
				goto error;
			/* Free buffer now that we have captured it */
			PGPFreeData( *s->outBufPtrPtr );
			*s->outBufPtrPtr = NULL;
		}
	} else if( s->outDiscard ) {
		/* Pass to the bit bucket */
		if ( IsNull( pgpDevNullCreate ( cdkContext, output ) ) ) {
			/* Guess at the cause */
			err = kPGPError_OutOfMemory;
			goto error;
		}
	} else {
		/* Checked for this case above */
		pgpAssert( 0 );
	}

checkpassthrough:
	/* Passthrough mode means we want to use data from annotations for
	 * output rather than the data coming down the pipe.  Save the output
	 * we've set up, and then convert output pipe to /dev/null for clearsign
	 * or to a memmod buffer for keys.
	 */
	if( s->passThrough ) {
		/*
		 * Handle passthrough for clearsign and keys.
		 * May have changed output temporarily for textfilt setup, so restore
		 * from prevOutput
		 */
		output = s->prevOutput;
		sSaveOutputState( s, output );
		sDrainPassThroughFifo( s );
		if( s->analyzeType == kPGPAnalyze_Key ) {
			if ( IsNull( s->outKey = pgpVariableMemModCreate( s->context,
						output, ~(PGPSize)0 ) ) ) {
				/* Guess at the cause */
				err = kPGPError_OutOfMemory;
			}
		} else {
			if ( IsNull( pgpDevNullCreate ( cdkContext, output ) ) ) {
				/* Guess at the cause */
				err = kPGPError_OutOfMemory;
			}
		}
	}

	return kPGPError_NoErr;
error:
	return err;
}


/*
 * Look for input file when we find a detached signature.
 */
	static PGPError
pgpDecodeFindAltInput(
	void				*arg,
	PGPPipeline			*head
	)
{
	PGPDecodeJob		*s;				/* Parameters for callbacks */
	PFLConstFileSpecRef		 detFileRef;	/* Input fileref */
	PGPFileRead			*pfrdet;		/* Input PGPFile pointer */
	PGPByte				*detBufPtr;		/* Input buffer pointer */
	PGPSize				 detBufLength;	/* Input buffer length */
	PGPOption			 op;			/* Selected option from list */
	PGPOptionListRef	 setupOp;		/* Optlist for setting up input */
	PGPBoolean			 freshOp;		/* Got fresh op from user */
	PGPError			 err;			/* Error codes */
	PGPFileDataType		 inFileDataType;	/* Unused */
	
	(void) head;

	s = (PGPDecodeJob *) arg;
	freshOp = FALSE;

	/* See if there was a detached-signature option specified */
	if( IsPGPError( err = pgpSearchOptionSingle( s->optionList,
							 kPGPOptionType_DetachedSignature, &op ) ) )
		goto error;
	if( !s->fUsedDetachedSigOp && IsOp( op )  && IsntNull( op.subOptions ) ) {
		/* Only use the detachedsig op once, after that we will ask user */
		s->fUsedDetachedSigOp = TRUE;
		setupOp = op.subOptions;
	} else {
		/* Ask user for detached signature input */

		if( IsNull( s->func ) ) {
			err = kPGPError_DetachedSignatureFound;
			goto error;
		}

		if( IsPGPError (err = pgpEventDetachedSignature( s->context,
								&s->newOptionList, s->func, s->userValue ) ) )
			goto error;
		if( IsPGPError( err = pgpSearchOptionSingle( s->newOptionList,
								kPGPOptionType_DetachedSignature, &op ) ) )
			goto error;
		if( IsOp( op )  && IsntNull( op.subOptions ) ) {
			setupOp = op.subOptions;
		} else {
			err = kPGPError_DetachedSignatureFound;
			goto error;
		}
		freshOp = TRUE;
	}

	/* Set up input file or buffer */
	err = pgpSetupInput( s->context, setupOp, NULL, NULL, TRUE, TRUE,
						 &detFileRef, &pfrdet, &inFileDataType, &detBufPtr,
						 &detBufLength );
	if( IsPGPError( err ) )
		goto error;
	
	/* Now pump the sig-check data through the pipes */
	if( detFileRef ) {
		/* File input */
		err = pgpFileReadPump( pfrdet, head );
		pgpFileReadDestroy( pfrdet );
		if( IsPGPError( err ) )
			goto error;
	} else {
		/* Buffer input */
		if( IsPGPError( err = pgpPumpMem( head, detBufPtr,
						detBufLength, NULL, NULL ) ) )
			goto error;
	}
	err = kPGPError_NoErr;
	/* Fall through */
error:
	/* Discard after one usage */
	if( freshOp ) {
		pgpCleanupOptionList( &s->newOptionList );
	}
	return err;
}


/*
 * Given the signature structure, sig, verify it against the hash
 * to see if this signature is valid.  This requires looking up the
 * public key in the keyring and validating the key.
 *
 * Returns 0 on success or an error code.
 */
	static PGPError
pgpDecodeVerifySig(
	void				*arg,
	PGPSig const		*sig,
	PGPByte const		*hash
	)
{
#if PGP_VERIFY_DISABLE /* [ */

	(void)arg;
	(void)sig;
	(void)hash;
	return kPGPError_FeatureNotAvailable;

#else /* PGP_VERIFY_DISABLE */  /* ]  [ */

	PGPDecodeJob		*s;				/* Parameters for callbacks */
	PGPKeyRef			 signKey;		/* Signing key as PGPKey */
	PGPKeySetRef		 alternateKeySet; /* Extra keyset to try lookup */
	RingObject			*ringKey;		/* Public key ring object */
	RingSet const		*ringSet;		/* Set holding ringKey */
	PGPPubKey			*pubkey;		/* Public key for verify */
	PGPKeyID			keyid;			/* KeyID from signature */
	PGPPublicKeyAlgorithm	pkalg;		/* Public key alg from signature */
	PGPTime				 timeStamp;		/* Issuance time of sig */
	PGPValidity			 failValidity,	/* Fail on keys less valid */
						 warnValidity;	/* Warn on keys less valid */
	PGPBoolean			 fRevoked=FALSE,/* Status of signing key */
						 fExpired=FALSE,
						 fDisabled=FALSE,
						 fValidityThreshold=TRUE;
	PGPValidity			 keyValidity=kPGPValidity_Unknown;
	PGPError			 err;			/* Error code */
	
	s = (PGPDecodeJob *) arg;
	pubkey = NULL;
	alternateKeySet = NULL;

	if( IsNull( s->func ) ) {
		/* If can't report results, don't bother to do anything */
		err = kPGPError_NoErr;
		goto error;
	}

	/* Get info about sig */
	err	= pgpGetSigKeyID ( sig, &keyid );
	if ( IsPGPError( err ) )
		goto error;
		
	pkalg = (PGPPublicKeyAlgorithm)pgpSigPKAlg (sig);

	timeStamp = pgpSigTimestamp (sig);

retrykeyset:

	if (pkalg >= kPGPPublicKeyAlgorithm_First &&
					pkalg <= kPGPPublicKeyAlgorithm_Last) {
		if( IsNull( s->keySet ) ) {
			err = kPGPError_MissingKeySet;
		} else {
			err = PGPGetKeyByKeyID ( s->keySet, &keyid, pkalg, &signKey);
		}
		if( IsPGPError( err ) && IsntNull( alternateKeySet ) ) {
			err = PGPGetKeyByKeyID ( alternateKeySet, &keyid, pkalg, &signKey);
		}
	}

	if( IsPGPError( err ) ||
			pkalg < kPGPPublicKeyAlgorithm_First ||
			pkalg > kPGPPublicKeyAlgorithm_Last) {
		/* Don't have signing key; notify him */
		err = pgpEventSignature( s->context, &s->newOptionList,
							  s->func, s->userValue, &keyid, NULL,
							  FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
							  keyValidity, timeStamp );
		if( IsntNull( alternateKeySet ) )
			PGPFreeKeySet (alternateKeySet);
		pgpSetupKeySet (s->newOptionList, &alternateKeySet);
		if( IsntNull( alternateKeySet ) ) {
			PGPIncKeySetRefCount( alternateKeySet );
			pgpCleanupOptionList( &s->newOptionList );
			goto retrykeyset;
		}
		pgpCleanupOptionList( &s->newOptionList );
		goto error;
	}

	if( IsPGPError( err = pgpGetKeyRingObject( signKey, FALSE, &ringKey ) ) )
		goto error;
	if( IsPGPError( err = pgpGetKeyRingSet( signKey, FALSE, &ringSet ) ) )
		goto error;
	
	pubkey = ringKeyPubKey( ringSet, ringKey, PGP_PKUSE_SIGN );

	if (pubkey && !pubkey->verify) {
		/* Make sure we can use this key */
		pgpPubKeyDestroy (pubkey);
		pubkey = NULL;
	}
	
	if (!pubkey) {
		/* Can't verify for some reason... notify him */
		err = pgpEventSignature( s->context, &s->newOptionList,
							  s->func, s->userValue, &keyid, signKey,
							  FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
							  keyValidity, timeStamp );
		if( IsntNull( alternateKeySet ) )
			PGPFreeKeySet (alternateKeySet);
		pgpSetupKeySet (s->newOptionList, &alternateKeySet);
		if( IsntNull( alternateKeySet ) ) {
			PGPIncKeySetRefCount( alternateKeySet );
			pgpCleanupOptionList( &s->newOptionList );
			goto retrykeyset;
		}
		pgpCleanupOptionList( &s->newOptionList );
		goto error;
	}
	
	/* Determine validity of signing key */
	if( IsPGPError( err = pgpGetMinValidity( s->optionList,
					&failValidity, &warnValidity ) ) )
		goto error;
	err = pgpCheckKeyValidity( s->context, s->optionList,
					signKey, ringSet, failValidity, warnValidity, NULL,
					&keyValidity );

	if( err == kPGPError_KeyInvalid ) {
		fValidityThreshold = FALSE;
	} else if( err == kPGPError_KeyRevoked ||
			   err == kPGPError_KeyExpired ||
			   err == kPGPError_KeyDisabled ) {
		fValidityThreshold = FALSE;
		if( IsPGPError( err = PGPGetKeyBoolean( signKey, kPGPKeyPropIsRevoked,
												&fRevoked ) ) )
			goto error;
		if( IsPGPError( err = PGPGetKeyBoolean( signKey, kPGPKeyPropIsDisabled,
												&fDisabled ) ) )
			goto error;
		if( IsPGPError( err = PGPGetKeyBoolean( signKey, kPGPKeyPropIsExpired,
												&fExpired ) ) )
			goto error;
	} else if( err != kPGPError_NoErr ) {
		goto error;
	}

	/* Check signature, returns 1 if OK */
	err = pgpSigCheck (sig, pubkey, hash);
	pgpPubKeyDestroy (pubkey);
	pubkey = NULL;
	if( err != 1 ) {
		if( IsPGPError( err ) )
			goto error;
		/* Notify of bad signature and exit */
		err = pgpEventSignature( s->context, &s->newOptionList,
							  s->func, s->userValue, &keyid, signKey,
							  TRUE, FALSE, fDisabled, fRevoked, fExpired,
							  fValidityThreshold, keyValidity, timeStamp );
		pgpCleanupOptionList( &s->newOptionList );
		goto error;
	}
		
	err = pgpEventSignature( s->context, &s->newOptionList,
						  s->func, s->userValue, &keyid, signKey,
						  TRUE, TRUE, fDisabled, fRevoked, fExpired,
						  fValidityThreshold, keyValidity, timeStamp );
	pgpCleanupOptionList( &s->newOptionList );

	/* Fall through */
error:
	if ( IsntNull( pubkey ) )
		pgpPubKeyDestroy( pubkey );
	if ( IsntNull( alternateKeySet ) )
		PGPFreeKeySet( alternateKeySet );
	return err;

#endif /* PGP_VERIFY_DISABLE */ /* ] */
}


/*
 * given a list of Encrypted Session Keys (esklist), try to decrypt
 * them to get the session key.  Fills in keylen with the length of
 * the session key buffer.
 *
 * Returns 0 on success or PGPANN_PARSER_EATIT on failure.
 */
	static PGPError
pgpDecodeDecryptESK(
	void				*arg,
	PGPESK const		*esklist,
	PGPByte				*key,
	PGPSize				*keylen,
	int				   (*tryKey)(void *arg, PGPByte const *key, PGPSize keylen),
	void				*tryarg
	)
{
#if PGP_DECRYPT_DISABLE /* [ */

	(void) arg;
	(void) esklist;
	(void) key;
	(void) keylen;
	(void) tryKey;
	(void) tryarg;
	return kPGPError_FeatureNotAvailable;

#else /* PGP_DECRYPT_DISABLE */  /* ]  [ */

	PGPDecodeJob		*s;				/* Parameters for callbacks */
	PGPESK const		*esk;			/* ESK being tested */
	PGPKeyRef			 decKey;		/* PGPKey for decryption key */
	PGPKeySet			*decKeySet1;	/* Keyset with just decKey */
	PGPKeySet			*decKeySet;		/* Keyset with all decryption keys */
	RingObject			*ringKey;		/* Secret key ring object */
	RingSet const		*ringSet;		/* Lowlevel keyring set for key */
	PGPSecKey			*seckey;		/* Secret key */
	PGPPublicKeyAlgorithm	 pkalg;			/* Pubkey algorithm from ESK */
	PGPBoolean			 success;		/* True if had a successful decrypt */
	PGPInt32			 klen;			/* Return code from key func */
	PGPError			 err;			/* Error from pgplib */
	PGPUInt32			 passPhraseCount; /* Count of possible pphrases */
	PGPUInt32			 keyIDCount;	/* # deckeys unavail */
	PGPKeyID			*keyIDArray;	/* Array of ptrs to keyids */
	PGPCipherAlgorithm	 cipheralg;		/* Decryption algorithm */

	s = (PGPDecodeJob *) arg;
	decKeySet = NULL;
	decKeySet1 = NULL;
	seckey = NULL;
	keyIDArray = NULL;
	
	/* First loop over all ESK's to set up recipient list */
	passPhraseCount = 0;
	keyIDCount = 0;
	if( IsntNull( s->keySet ) ) {
		if( IsPGPError( err = PGPNewEmptyKeySet( s->keySet, &decKeySet ) ) )
			goto error;
	}
	for( esk = esklist; IsntNull( esk ); esk = pgpEskNext( esk ) ) {
		switch( pgpEskType( esk ) ) {
		case PGP_ESKTYPE_PASSPHRASE:
			passPhraseCount += 1;
			break;
		case PGP_ESKTYPE_PUBKEY:
		{
			PGPKeyID			keyid;			/* Key ID from ESK */
			void *				vKeyIDArray;
	
			err	= pgpGetEskKeyID( esk, &keyid);
			if ( IsPGPError( err ) )
				goto error;

			/* Save keyids in list */
			vKeyIDArray = keyIDArray;
			err = pgpContextMemRealloc( s->context,
				   &vKeyIDArray,
				   (keyIDCount+1) * sizeof(keyIDArray[ 0 ]), 0 );
			if( IsPGPError( err ) )
				goto error;
			keyIDArray = (PGPKeyID *)vKeyIDArray;
			keyIDArray[keyIDCount++] = keyid;
				
			pkalg = (PGPPublicKeyAlgorithm)pgpEskPKAlg (esk);

			decKey = NULL;
			if( IsntNull( s->keySet ) &&
						pkalg >= kPGPPublicKeyAlgorithm_First &&
						pkalg <= kPGPPublicKeyAlgorithm_Last) {
				(void)PGPGetKeyByKeyID ( s->keySet, &keyid, pkalg, &decKey);
			}

			if( IsNull( decKey ) ) {
				/* We don't have the decryption key */
			} else {
				/* Accumulate into recipient key set */
				PGPKeySetRef newKeySet;
				if( IsPGPError( err = PGPNewSingletonKeySet( decKey,
											&decKeySet1 ) ) )
					goto error;
				if( IsPGPError( err = PGPUnionKeySets(
										decKeySet1, decKeySet, &newKeySet ) ) )
					goto error;
				PGPFreeKeySet( decKeySet );
				decKeySet = newKeySet;
				PGPFreeKeySet( decKeySet1 );
				decKeySet1 = NULL;
			}
			break;
		}
		
		default:
			pgpAssert(0);
			break;
		}
	}

	/* Notify user about decryption keys */
	if( IsPGPError( err = pgpEventRecipients( s->context, &s->newOptionList,
						  s->func, s->userValue, decKeySet, keyIDArray,
						  passPhraseCount, keyIDCount ) ) )
		goto error;
	pgpCleanupOptionList( &s->newOptionList );
	if( IsntNull( decKeySet ) ) {
		PGPFreeKeySet( decKeySet );
		decKeySet = NULL;
	}

	/* Loop over all ESK's trying to find one we can decrypt */
	/* decKeySet will hold decryption keys that we have secrets for */
	success = FALSE;
	if( IsntNull( s->keySet ) ) {
		if( IsPGPError( err = PGPNewEmptyKeySet( s->keySet, &decKeySet ) ) )
			goto error;
	}

	while( !success ) {
		for( esk = esklist; IsntNull( esk ) && !success;
			 								esk = pgpEskNext( esk ) ) {
			switch( pgpEskType( esk ) ) {
			case PGP_ESKTYPE_PASSPHRASE:
				/* Try the pass phrase */
				klen = pgpEskConvDecrypt (esk, s->env,
								  (char *)s->passPhrase, s->passLength, key);
				if( klen < 0 ) {
					err = klen;
					goto error;
				}
				/* Else klen is length of key */
				*keylen = klen;

				/* Returns 0 on success, nonzero on failure */
				if( tryKey (tryarg, key, *keylen) == 0 ) {
					/* Success */
					success = 1;
				}
				break;

			case PGP_ESKTYPE_PUBKEY:
			{
				PGPKeyID			keyid;		/* Key ID from ESK */
			
				/* Look up key from ESK */
				err = pgpGetEskKeyID( esk, &keyid );
				if ( IsPGPError( err ) )
					goto error;
					
				pkalg = (PGPPublicKeyAlgorithm)pgpEskPKAlg (esk);

				/* If don't have key, go on to next one */
				decKey = NULL;
				if( IsntNull( s->keySet ) &&
						pkalg >= kPGPPublicKeyAlgorithm_First &&
						pkalg <= kPGPPublicKeyAlgorithm_Last) {
					(void)PGPGetKeyByKeyID( s->keySet, &keyid, pkalg, &decKey);
				}
				
				if( IsNull( decKey ) )
					break;

				if( IsPGPError( err = pgpGetKeyRingSet( decKey, FALSE,
														&ringSet ) ) )
					goto error;

				/*
				 * Use lower-level function to look up proper subkey.
				 * If key has more than one subkey then ringSecSecKey won't
				 * choose the right one if we give it a top level key.
				 */
				ringKey = ringKeyById8 (ringSet, (PGPByte)pkalg,
										pgpGetKeyBytes (&keyid));
				if( IsNull( ringKey ) )
					break;

				seckey = ringSecSecKey( ringSet, ringKey, PGP_PKUSE_ENCRYPT );

				/* See if have a good decryption key */
				if (seckey && !seckey->decrypt) {
					/* A matching secret key of a type which can't decrypt? */
					pgpSecKeyDestroy (seckey);
					seckey = NULL;
				}

				if( IsNull( seckey ) )
					break;

				/* Try to unlock decryption key with passphrase */
				err = pgpSecKeyUnlock (seckey, s->env,
								(char *)s->passPhrase, s->passLength,
								s->hashedPhrase );
				if (err <= 0) {
					/* Pass phrase failed to unlock.  Try next ESK. */
					PGPKeySetRef newKeySet;
					pgpSecKeyDestroy (seckey);
					seckey = NULL;
					/* Accumulate signing keys into decKeySet */
					if( IsPGPError( err = PGPNewSingletonKeySet( decKey,
														&decKeySet1 ) ) )
						goto error;
					if( IsPGPError( err = PGPUnionKeySets(
									   decKeySet1, decKeySet, &newKeySet ) ) )
						goto error;
					PGPFreeKeySet( decKeySet );
					decKeySet = newKeySet;
					PGPFreeKeySet( decKeySet1 );
					decKeySet1 = NULL;
					break;
				}

				/* Try decrypting the ESK */
				err = pgpEskPKdecrypt (esk, s->env, seckey, key);
				*keylen = err;
				pgpSecKeyDestroy (seckey);
				seckey = NULL;
				if (err <= 0) {
					/* Failed to decrypt, indicates corrupt ESK */
					if (err != kPGPError_CAPIUnsupportedKey)
						err = kPGPError_CorruptSessionKey;
					goto error;
				}

				/* Now try the decrypted ESK against the rest of the message */
				/* (save cipheralgorithm, first byte of key buffer) */
				cipheralg = (PGPCipherAlgorithm)key[0];
				err = tryKey (tryarg, key, *keylen);
				if (err) {
					/* Failure, indicates corrupt ESK */
					err = kPGPError_CorruptSessionKey;
					goto error;
				}
				
				/* Else it worked OK */
				if( IsPGPError( err = pgpEventDecryption( s->context,
									  &s->newOptionList, s->func,
									  s->userValue, cipheralg ) ) )
					goto error;
				pgpCleanupOptionList( &s->newOptionList );
				
				success = 1;
				break;
			}

			default:
				/* Unknown ESK type */
				pgpAssert (0);
				break;
			}
		}
		if( !success ) {
			/* Get new passphrase from user */
			if( IsNull( s->func ) ) {
				/* If no callback, just skip undecryptable messages */
				err = PGPANN_PARSER_EATIT;
				goto error;
			}
			if( IsPGPError( err = pgpEventPassphrase( s->context,
								  &s->newOptionList, s->func,
								  s->userValue,
								  (PGPBoolean)(passPhraseCount>0),
								  decKeySet ) ) )
				goto error;
			pgpBurnDecodePassphrase( s );
			err = pgpSetupDecodePassphrase( s->context,
				  s->newOptionList, &s->passPhrase, &s->passLength,
				  &s->hashedPhrase );
			pgpCleanupOptionList( &s->newOptionList );
			if( IsPGPError( err ) )
				goto error;
			/* If he specified no passphrase, we will eat the message */
			if( IsNull( s->passPhrase ) ) {
				err = PGPANN_PARSER_EATIT;
				goto error;
			}
		}
	}

	err = kPGPError_NoErr;

error:
	if( IsntNull( decKeySet ) ) {
		PGPFreeKeySet( decKeySet );
		decKeySet = NULL;
	}
	if( IsntNull( decKeySet1 ) ) {
		PGPFreeKeySet( decKeySet1 );
		decKeySet1 = NULL;
	}
	if( IsntNull( seckey ) ) {
		pgpSecKeyDestroy( seckey );
		seckey = NULL;
	}
	if( IsntNull( keyIDArray ) ) {
		pgpContextMemFree( s->context, keyIDArray );
		keyIDArray = NULL;
	}

	return err;

#endif /* PGP_DECRYPT_DISABLE */ /* ] */
}


/************************** Main decode function ****************************/



/* Local callback function */

	static PGPError
decodeLocalCallBack (
	void				*arg,
	PGPFileOffset		soFar,
	PGPFileOffset		total
	)
{
	PGPError			  err = kPGPError_NoErr;
	PGPDecodeJob		 *s = ( PGPDecodeJob * ) arg;

	if( IsntNull( s->func )  &&  s->fNullEvents ) {
		err = pgpEventNull( s->context, &s->newOptionList, s->func,
							s->userValue, soFar, total );
		pgpCleanupOptionList( &s->newOptionList );
	}
	return err;
}


static const PGPOptionType decodeOptionSet[] = {
	kPGPOptionType_InputFileRef,
	kPGPOptionType_InputBuffer,
	kPGPOptionType_OutputFileRef,
	kPGPOptionType_OutputBuffer,
	kPGPOptionType_OutputAllocatedBuffer,
	kPGPOptionType_AppendOutput,
	kPGPOptionType_DiscardOutput,
	kPGPOptionType_LocalEncoding,
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey,
	kPGPOptionType_DetachedSignature,
	kPGPOptionType_FailBelowValidity,
	kPGPOptionType_WarnBelowValidity,
	kPGPOptionType_OutputLineEndType,
	kPGPOptionType_EventHandler,
	kPGPOptionType_SendNullEvents,
	kPGPOptionType_ImportKeysTo,
	kPGPOptionType_SendEventIfKeyFound,
	kPGPOptionType_DecodeOnlyOne,
	kPGPOptionType_PassThroughIfUnrecognized,
	kPGPOptionType_PassThroughClearSigned,
	kPGPOptionType_PassThroughKeys,
	kPGPOptionType_KeySetRef,
	kPGPOptionType_RecursivelyDecode,
	kPGPOptionType_X509Encoding   /* allow calling from pgpimportkeyset */
};

/* Main entry point for this module */

	PGPError
pgpDecodeInternal(
	PGPContextRef		context,
	PGPOptionListRef	optionList
	)
{
	PGPDecodeJob		 jobState,		/* State in a struct */
						*s=&jobState;	/* Use s-> to access all state  */
	PGPFileDataType		 inFileDataType;	/* Unused */
	
	/* Initialize pointers to NULL for easier error cleanup */
	pgpClearMemory( s, sizeof( *s ) );
	s->context = context;
	s->optionList = optionList;
	s->tail = &s->head;
	s->analyzeState = kAnalyzeWaiting;

	if (IsPGPError( s->err = pgpCheckOptionsInSet( optionList,
						decodeOptionSet, elemsof( decodeOptionSet ) ) ) )
		return s->err;

	/* Get copies of info from context */
	s->env = pgpContextGetEnvironment( s->context );
	s->rng = pgpContextGetX9_17RandomContext ( s->context );
	
	/* Setup the UI callback functions & args */
	s->ui.message		= pgpDecodeShowMessage;
	s->ui.doCommit		= pgpDecodeDoCommit;
	s->ui.newOutput		= pgpDecodeSetupNewOutput;
	s->ui.needInput		= pgpDecodeFindAltInput;
	s->ui.sigVerify		= pgpDecodeVerifySig;
	s->ui.eskDecrypt	= pgpDecodeDecryptESK;
	s->ui.annotate		= pgpDecodeHandleAnnotation;

	/* Set up callback pointers and data */
	if( IsPGPError( s->err = pgpSetupCallback( s->optionList,
							 &s->func, &s->userValue, &s->fNullEvents ) ) )
		goto error;
	s->err = pgpEventInitial( s->context, &s->newOptionList,
							  s->func, s->userValue );
	pgpCleanupOptionList( &s->newOptionList );
	if( IsPGPError( s->err ) )
		goto error;

	/* Set up input file or buffer */
	if( IsPGPError( s->err = pgpSetupInput( s->context, s->optionList,
					NULL, NULL, FALSE, FALSE, &s->inFileRef, &s->pfrin,
					&inFileDataType, &s->inBufPtr, &s->inBufLength ) ) )
		goto error;


	/* Get keyset, passphrase data from user */
	if( IsPGPError( s->err = pgpSetupKeySet( s->optionList,
									&s->keySet ) ) )
		goto error;
	if( IsPGPError( s->err = pgpSetupDecodePassphrase( s->context,
						s->optionList, &s->passPhrase, &s->passLength,
						&s->hashedPhrase ) ) )
		goto error;

	/* Parse output information if any */
	if( IsPGPError( s->err = pgpSetupOutput( s->optionList,
							s->env, &s->outFileRef,
							&s->outBufPtr, &s->outBufPtrPtr,
							&s->outBufMaxLength, &s->outBufUsedLength,
							&s->outDiscard, &s->localEncodeFlags,
							&s->lineEnd, &s->fAppendOutput ) ) )
		goto error;

	/* If he set up output already, keep it throughout.  Otherwise we need
	 * to see a callback function. */
	s->fixedOutput = IsntNull( s->outFileRef ) || IsntNull( s->outBufPtr ) ||
					  IsntNull( s->outBufPtrPtr ) || s->outDiscard;

	if( !s->fixedOutput && IsNull( s->func ) ) {
		pgpDebugMsg( "Error: no output options" );
		s->err = kPGPError_BadParams;
		goto error;
	}

	/* Set up pipeline */
	s->tail = pgpDecryptPipelineCreate ( context,
				&s->head, s->env, NULL, &s->ui, s);

	/* Handle clearsign and key passthroughs (nonpgp is in NewOutput) */
	if( IsPGPError( s->err = pgpSetupPassThrough( s->context, s->optionList,
												  s->head,
												  &s->passThroughFifo,
												  &s->recurse ) ) )
		goto error;

	/* Now pump the data through the pipes */
	if( s->inFileRef ) {
		/* File input */
		if( IsntNull( s->func ) && s->fNullEvents ) {
			pgpFileReadSetCallBack( s->pfrin, decodeLocalCallBack, s );
		}
		s->err = pgpFileReadPump( s->pfrin, s->head );
		pgpFileReadDestroy( s->pfrin );
		s->pfrin = NULL;
		if( IsPGPError( s->err ) )
			goto error;
		s->head->sizeAdvise( s->head, 0 );
	} else {
		/* Buffer input */
		if( IsntNull( s->func ) && s->fNullEvents ) {
			s->err = pgpPumpMem( s->head, s->inBufPtr, s->inBufLength,
								 decodeLocalCallBack, s );
		} else {
			s->err = pgpPumpMem( s->head, s->inBufPtr, s->inBufLength,
								 NULL, NULL );
		}
		if( IsPGPError( s->err ) )
			goto error;
	}

	/* Clean up if we had diverted the output in the last section */
	if( s->fPrevOutput ) {
		(*s->prevOutput)->teardown( *s->prevOutput );
		sRestoreOutputState( s, s->prevOutput );
	}

	/* Get output buffer bytes-used info if appropriate */
	if( s->outPipe ) {
		if( IsntNull( s->outBufPtrPtr ) ) {
			/* Dynamically allocated buffer - tell user size & position */
			if( IsPGPError( s->err = pgpGetVariableMemOutput( s->outPipe,
							s->outBufMaxLength, s->outBufPtrPtr,
							s->outBufUsedLength ) ) )
				goto error;
		} else {
			/* Fixed size buffer - tell user actual size used */
			pgpAssert( IsntNull( s->outBufPtr ) );
			if( IsPGPError( s->err = pgpGetMemOutput( s->outPipe,
						s->outBufMaxLength, s->outBufUsedLength ) ) )
				goto error;
		}
		s->outPipe = NULL;
	}

	/* Now we can tear down the pipeline */
	s->head->teardown( s->head );
	s->head = NULL;
	if( IsntNull( s->pfout ) ) {
		pgpFileClose( s->pfout );
		s->pfout = NULL;
	}

	/* Done, clean up and return */
	s->err = kPGPError_NoErr;

error:

	if( IsntNull( s->head ) ) {
		s->head->teardown( s->head );
		s->head = NULL;
	}
	if( IsntNull( s->pfrin ) ) {
		pgpFileReadDestroy( s->pfrin );
		s->pfrin = NULL;
	}
	if( IsntNull( s->pfout ) ) {
		pgpFileClose( s->pfout );
		s->pfout = NULL;
	}
	if( IsntNull( s->outFileRef ) ) {
		PFLFreeFileSpec( s->outFileRef );
		s->outFileRef = NULL;
	}

	if( IsntNull( s->passThroughFifo ) ) {
		pgpFifoDestroy( &pgpByteFifoDesc, s->passThroughFifo );
		s->passThroughFifo = NULL;
	}

	/* Burn passphrase from job structure, if any */
	pgpBurnDecodePassphrase( s );

	/* Interruption error is used to abort early in the process */
	if( s->err == kPGPError_Interrupted )
		s->err = kPGPError_NoErr;

	/* Notify user via callback of error if requested */
	if( IsPGPError( s->err ) && IsntNull( s->func ) ) {
		(void)pgpEventError( s->context, &s->newOptionList, s->func,
							 s->userValue, s->err, NULL );
		pgpCleanupOptionList( &s->newOptionList );
	}
		
	(void)pgpEventFinal( s->context, &s->newOptionList, s->func,
						 s->userValue );
	pgpCleanupOptionList( &s->newOptionList );

	return s->err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	comment-column: 40
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
