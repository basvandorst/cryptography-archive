/*____________________________________________________________________________
	pgpEncSubr.c
	Helper subroutines for encode/decode API
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpEncSubr.c,v 1.59 1999/04/23 00:36:44 hal Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"	/* or pgpConfig.h in the CDK */

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* Public headers */
#include "pgpPubTypes.h"
#include "pgpContext.h"
#include "pgpEncode.h"
#include "pgpErrors.h"
#include "pgpFileRef.h"
#include "pgpKeys.h"
#include "pgpMem.h"

/* Private headers */
#include "pgpDebug.h"
#include "pgpAnnotate.h"
#include "pgpDevNull.h"
#include "pgpEncodePriv.h"
#include "pgpEnv.h"
#include "pgpEventPriv.h"
#include "pgpMemMod.h"
#include "pgpPipeline.h"
#include "pgpPrsAsc.h"
#include "pgpRandomContext.h"
#include "pgpRngPub.h"
#include "pgpVMemMod.h"
#include "pgpUtilities.h"


/******************* Misc functions for encode/decode ********************/

/*
 * Convert the user flags used to specify local encoding into a flag value
 * from pgpFileRef.h
 */
	PGPFileOpenFlags
pgpLocalEncodingToFlags( PGPUInt32 localEncode )
{
	PGPFileOpenFlags	flags = (PGPFileOpenFlags)0;

	/* Wrapper function ensured that only one of these was given */
	if( localEncode & kPGPLocalEncoding_Auto )
		flags = kPGPFileOpenMaybeLocalEncode;
	else if( localEncode & kPGPLocalEncoding_Force )
		flags = kPGPFileOpenForceLocalEncode;

	/* Add optional modifier flags */
	if( localEncode & kPGPLocalEncoding_NoMacBinCRCOkay )
		flags |= kPGPFileOpenNoMacBinCRCOkay;

	return flags;
}



/******************* Helper functions for encode/decode ********************/



/*
 * Pump data from the memory buffer at inBufPtr, of size InBufLength,
 * into head.  Do the terminating sizeAdvise but don't do the teardown.
 */

	PGPError
pgpPumpMem(
	PGPPipeline			*head,
	PGPByte				*inBufPtr,
	PGPSize				 inBufLength,
	PGPProgressCallBack	 callBack,
	void				 *callBackArg
	)
{
	PGPSize				totalSize;
	PGPError			err = kPGPError_NoErr;
	PGPSize				written;

	totalSize = inBufLength;

	err = head->sizeAdvise( head, inBufLength );
	if( IsPGPError( err ) )
		return err;
	err = head->annotate (head, NULL, PGPANN_FILE_BEGIN, NULL, 0);
	if( IsPGPError( err ) )
		return err;
	err = head->sizeAdvise( head, inBufLength );

	while( inBufLength>0 && IsntPGPError( err ) ) {
		written = head->write( head, inBufPtr,
							   pgpMin(inBufLength, 4096), &err );
		inBufPtr += written;
		inBufLength -= written;
		if( IsPGPError( err ) )
			break;
		if( IsntNull( callBack ) )
			err = callBack( callBackArg, totalSize - inBufLength, totalSize );
	}
	if( IsntPGPError( err ) )
		err = head->sizeAdvise( head, 0 );
	if( IsntPGPError( err ) )
		err = head->annotate (head, NULL, PGPANN_FILE_END, NULL, 0);
	if( IsntPGPError( err ) )
		err = head->sizeAdvise( head, 0 );
	return err;
}


/* Send a warning event to the user if he is set up for events */

	PGPError
pgpWarnUser(
	PGPContextRef	 	 context,		/* Input parameters */
	PGPOptionListRef	 optionList,
	PGPError			 errCode,
	void				*warnArg
	)
{
	PGPError			 err;
	PGPEventHandlerProcPtr handler;
	PGPUserValue		 arg;
	PGPOptionListRef	dummyOptionList = NULL;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_EventHandler, FALSE,
						 "%p%p", &handler, &arg ) ) )
		goto error;

	if( IsntNull( handler ) ) {
		if( IsPGPError( err = pgpEventWarning( context, &dummyOptionList,
								  handler, arg, errCode, warnArg ) ) )
			goto error;
		pgpCleanupOptionList( &dummyOptionList );
	}
	return kPGPError_NoErr;

error:
	return err;
}


/* Set fail-below validity values for failure and warning */

	PGPError
pgpGetMinValidity(
	PGPOptionListRef	 optionList,
	PGPValidity			*failValidity,	/* Output parameters */
	PGPValidity			*warnValidity
	)
{
	PGPError			 err;
	PGPOption			 op;
	PGPUInt32			temp;

	/* Init return pointers */
	pgpa( pgpaAddrValid( failValidity, PGPValidity ) );
	pgpa( pgpaAddrValid( warnValidity, PGPValidity ) );
	/* XXX Need to fix these based on trust model and other aspects */
	*failValidity = kPGPValidity_Unknown;
	*warnValidity = kPGPValidity_Marginal;

	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
						  kPGPOptionType_FailBelowValidity, &op ) ) )
		goto error;
	if( IsOp( op ) ) {
		
		if( IsPGPError( err = pgpOptionUInt( &op, &temp ) ) )
			goto error;
		*failValidity	= (PGPValidity)temp;
	}
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
						  kPGPOptionType_WarnBelowValidity, &op ) ) )
		goto error;
	if( IsOp( op ) ) {
		if( IsPGPError( err = pgpOptionUInt( &op, &temp ) ) )
			goto error;
		*warnValidity	= (PGPValidity)temp;
	}
	return kPGPError_NoErr;

error:
	return err;
}

	


/* Check a key for encryption or signature-verification validity.  Return
   an error if it is no good, no error to proceed */

	PGPError
pgpCheckKeyValidity(
	PGPContextRef	 	 context,		/* Input parameters */
	PGPOptionListRef	 optionList,
	PGPKey				*key,
	RingSet const		*ringSet,
	PGPValidity			 failValidity,
	PGPValidity			 warnValidity,
	PGPKeySet			*warnKeySet,	/* Output parameters */
	PGPValidity			*pValidity
	)
{
	PGPValidity			 validity;
	PgpTrustModel		 pgptrustmodel;
	PGPError			 err;
	PGPBoolean			 fRevoked,
						 fExpired,
						 fDisabled;

	(void) context;
	(void) optionList;

	if( IsntNull( pValidity ) )
		*pValidity = kPGPValidity_Unknown;

	pgptrustmodel = pgpTrustModel (ringSetPool (ringSet));

	/* First calculate validity level */
	if (pgptrustmodel == PGPTRUST0) {
		if( IsPGPError( err = PGPGetKeyNumber( key, kPGPKeyPropValidity,
											   (PGPInt32 *)&validity ) ) )
			goto error;
	} else {
		/* Not yet supported */
		if( IsPGPError( err = PGPGetKeyNumber( key, kPGPKeyPropValidity,
											   (PGPInt32 *)&validity ) ) )
			goto error;
	}

	if( IsntNull( pValidity ) )
		*pValidity = validity;

	if( IsPGPError( err = PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked,
											&fRevoked ) ) )
		goto error;
	if( fRevoked ) {
		err = kPGPError_KeyRevoked;
		goto error;
	}

	if( IsPGPError( err = PGPGetKeyBoolean( key, kPGPKeyPropIsDisabled,
											&fDisabled ) ) )
		goto error;
	if( fDisabled ) {
		err = kPGPError_KeyDisabled;
		goto error;
	}

	if( IsPGPError( err = PGPGetKeyBoolean( key, kPGPKeyPropIsExpired,
											&fExpired ) ) )
		goto error;
	if( fExpired ) {
		err = kPGPError_KeyExpired;
		goto error;
	}

	if( validity < failValidity ) {
		/* Return an error */
		err = kPGPError_KeyInvalid;
		goto error;
	} else if( validity < warnValidity && IsntNull( warnKeySet ) ) {
		/* Send a warning */
		PGPKeySet *tmpKeySet;
		if( IsPGPError( err = PGPNewSingletonKeySet( key,
													 &tmpKeySet ) ) )
			goto error;
		if( IsNull( tmpKeySet ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
		if( IsPGPError( err = PGPAddKeys( tmpKeySet, warnKeySet ) ) )
			goto error;
		if( IsPGPError( err = PGPFreeKeySet( tmpKeySet ) ) )
			goto error;
	}

	/* Everything is OK, proceed */
	return kPGPError_NoErr;

error:
	return err;
}

/*
 * Set *outputBufferLen from pipeBuf.
 * bufferSize is the allocated size of the buffer, and we return an error
 * if we exceeded the allocated size.  (We don't actually write beyond
 * the end of the buffer, of course.)
 */

	PGPError
pgpGetMemOutput(
	PGPPipeline			*pipeBuf,
	PGPSize				 bufferSize,
	PGPSize				*outputBufferLen
	)
{
	pgpa( pgpaAddrValid( pipeBuf, PGPPipeline ) );

	pipeBuf->annotate( pipeBuf, NULL,
					   PGPANN_MEM_BYTECOUNT, (PGPByte *)outputBufferLen,
					   sizeof( *outputBufferLen ) );
	if( *outputBufferLen > bufferSize )
		return kPGPError_OutputBufferTooSmall;
	return kPGPError_NoErr;
}


/*
 * Set *outputBufferPtr and *outputBufferLen from pipeBuf.  maxSize
 * was our limit on the allocated size of the buffer, and we return an
 * error if we exceeded the max size.  (We don't actually write more data
 * than that into our dynamically allocated buffer, though.)  The difference
 * from the above is that in this case the buffer was dynamically allocated
 * while in the previous routine it was a static buffer we were given.
 */

	PGPError
pgpGetVariableMemOutput(
	PGPPipeline			*pipeBuf,
	PGPSize				 maxSize,
	PGPByte			   **outputBufferPtr,
	PGPSize				*outputBufferLen
	)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpa( pgpaAddrValid( pipeBuf, PGPPipeline ) );

	err	= pipeBuf->annotate( pipeBuf, NULL,
				 PGPANN_MEM_BYTECOUNT, (PGPByte *)outputBufferLen,
				 sizeof( *outputBufferLen ) );
	if ( IsntPGPError( err ) )
	{
		pipeBuf->annotate( pipeBuf, NULL,
			PGPANN_MEM_MEMBUF, (PGPByte *)outputBufferPtr,
			sizeof( *outputBufferPtr ) );
	}
	
	if ( IsntPGPError( err ) )
	{
		if( *outputBufferLen > maxSize )
			err	= kPGPError_OutputBufferTooSmall;
	}
		
	return err;
}


/*
 * See if optionList specifies PGPMIME encoding, and if so, get MIME body
 * offset and return it to caller.
 */
	PGPError
pgpGetPGPMIMEBodyOffset(
	PGPPipeline			*pipeBuf,
	PGPOptionListRef	 optionList)
{
	PGPSize				*mimeBodyOffPtr;
	PGPSize				 mimeBodyOff;
	char				*mimeSeparator;
	PGPUInt32	 		 mimeHeaderLines;
	PGPUInt32			 mimeFlag;
	PGPUInt32			 lineEndFlag;
	PGPLineEndType		 lineEnd;
	PGPUInt32			 uintLineEnd;
	PGPError			 err = kPGPError_NoErr;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
							 kPGPOptionType_PGPMIMEEncoding, FALSE,
							 "%d%p%p", &mimeFlag, &mimeBodyOffPtr,
									   &mimeSeparator) ) )
		goto error;
	if( mimeFlag ) {
		if( IsntNull( mimeBodyOffPtr ) ) {
			if( IsPGPError( err = pipeBuf->annotate( pipeBuf, NULL,
					 PGPANN_PGPMIME_HEADER_SIZE,
					 (unsigned char *)&mimeBodyOff, sizeof(mimeBodyOff) ) ) )
			   goto error;
			if( IsPGPError( err = pgpFindOptionArgs( optionList,
								  kPGPOptionType_OutputLineEndType, FALSE,
								  "%b%d", &lineEndFlag, &uintLineEnd ) ) )
				goto error;
			if( lineEndFlag )
				lineEnd = (PGPLineEndType)uintLineEnd;
			else
				lineEnd = pgpGetDefaultLineEndType ();
			if( lineEnd == kPGPLineEnd_CRLF ) {
				if( IsPGPError( err = pipeBuf->annotate( pipeBuf, NULL,
									PGPANN_PGPMIME_HEADER_LINES,
									(unsigned char *)&mimeHeaderLines,
									sizeof(mimeHeaderLines) ) ) )
					goto error;
				mimeBodyOff += mimeHeaderLines;
			}
			*mimeBodyOffPtr = mimeBodyOff;
		}
		if( IsntNull( mimeSeparator ) ) {
			if( IsPGPError( err = pipeBuf->annotate( pipeBuf, NULL,
					 PGPANN_PGPMIME_SEPARATOR,
					 (unsigned char *)mimeSeparator,
					 kPGPMimeSeparatorSize ) ) )
				goto error;
		}
	}
error:
	return err;
}




/* Line ending control */

	PGPLineEndType
pgpGetDefaultLineEndType ()
{
#if defined(PGP_MSDOS) || defined(PGP_WIN32)
	return kPGPLineEnd_CRLF;
#elif PGP_MACINTOSH
	return kPGPLineEnd_CR;
#else
	return kPGPLineEnd_LF;
#endif
}




/* Parse an optionlist structure and extract callback info */

	PGPError
pgpSetupCallback(
	PGPOptionListRef	  optionList,
	PGPEventHandlerProcPtr *func,		/* Output params */
	PGPUserValue		 *userValue,
	PGPBoolean			 *fNullEvents
	)
{
	PGPError			  err;
	PGPBoolean			  lNullEvents;

	/* Init return data to default states */
	pgpa( pgpaAddrValid( func, PGPEventHandlerProcPtr ) );
	pgpa( pgpaAddrValid( userValue, PGPUserValue ) );
	pgpa( pgpaAddrValid( fNullEvents, PGPBoolean ) );
	*func = NULL;
	*userValue = (PGPUserValue)0;
	*fNullEvents = FALSE;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_EventHandler, FALSE,
						 "%p%p", func, userValue ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_SendNullEvents, FALSE,
						 "%b", &lNullEvents ) ) )
		goto error;
	if( lNullEvents ) {
		/* Wants notification of null events */
		if( IsNull (*func) ) {
			pgpDebugMsg(
				   "Error: Null events requested without event handler" );
			err = kPGPError_BadParams;
			goto error;
		}
		*fNullEvents = TRUE;
	}
	return kPGPError_NoErr;

error:
	*func = NULL;
	*userValue = (PGPUserValue)0;
	*fNullEvents = FALSE;

	return err;
}

		

/* Open input specification if there is one.  Optionally set up filename
 * literal structure, and seed RNG.
 */

	PGPError
pgpSetupInput(
	PGPContextRef	 	  context,		/* Input params */
	PGPOptionListRef	  optionList,
	PgpLiteralParams	 *literal,
	PGPRandomContext	 *rng,
	PGPBoolean			  inputMode,
	PGPBoolean			  detachedSignature,
	PFLConstFileSpecRef	 *inFileRef,	/* Output params */
	PGPFileRead         **pfrin,
	PGPFileDataType		 *inFileDataType,
	PGPByte				**inBufPtr,
	PGPSize				 *inBufLength
	)
{
	PGPOption			 op;			/* Selected option from list */
	PGPError			 err;			/* Error return code */
	static char			 fyeomagic[] = "_CONSOLE"; /* Display-only mode */
	PGPUInt32			 localEncode;	/* Special input encoding (macbin) */
	PGPFileOpenFlags	 localEncodeFlags;	/* Flag form of localEncode */
	PGPByte const		*peekBuf;		/* For RNG seeding from file */
	PGPSize				 peekLength;	/* Length of peekBuf */

	/* Init return data to default states */
	pgpa( pgpaAddrValid( inFileRef, PFLConstFileSpecRef ) );
	pgpa( pgpaAddrValid( pfrin, PGPFileRead * ) );
	pgpa( pgpaAddrValid( inFileDataType, PGPFileDataType ) );
	pgpa( pgpaAddrValid( inBufPtr, PGPByte * ) );
	pgpa( pgpaAddrValid( inBufLength, PGPSize ) );
	*inFileRef = NULL;
	*pfrin = NULL;
	*inFileDataType = kPGPFileDataType_Unknown;
	*inBufPtr = NULL;
	*inBufLength = 0;
	if( IsntNull( literal) ) {
		pgpa( pgpaAddrValid( literal, PgpLiteralParams ) );
		literal->filename = NULL;
		literal->timestamp = ( PGPUInt32 ) 0;
	}

	if( IsntNull( literal ) ) {
		/* First check "for your eyes only" mode, flag with magic filename */
		PGPUInt32 fFYEO;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							 kPGPOptionType_ForYourEyesOnly, FALSE,
							 "%d", &fFYEO ) ) )
			goto error;
		if( fFYEO ) {
			literal->filename = pgpAllocCString( context, fyeomagic );
		}
	}

	/* See if there is an input file specified */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_InputFileRef, &op ) ) )
		goto error;
	if( IsOp( op ) ) {
		/* File input */
		if( IsPGPError( err = pgpOptionPtr( &op, (void **)inFileRef ) ) )
			goto error;
		if( IsntNull( literal ) ) {
			if( IsNull( literal->filename ) ) {
				char *filerefname;
				void *vfilerefname;
				if( IsPGPError( err = pgpFindOptionArgs( optionList,
									 kPGPOptionType_InputFileName, FALSE,
									 "%p", &vfilerefname ) ) )
					goto error;
				filerefname = (char *)vfilerefname;
				if( IsntNull( filerefname ) ) {
					literal->filename = pgpAllocCString( context,
														 filerefname );
				} else {
					filerefname = pgpGetFileRefName( *inFileRef );
					literal->filename = pgpAllocCString( context,
														 filerefname );
					PGPFreeData( filerefname );
				}
			}
		}
		localEncodeFlags = (PGPFileOpenFlags)0;
		if( inputMode ) {
			if( IsPGPError( err = pgpSearchOptionSingle( optionList,
									 kPGPOptionType_LocalEncoding, &op ) ) )
				goto error;
			if( IsOp( op ) ) {
				if( IsPGPError( err = pgpOptionUInt( &op, &localEncode ) ) )
					goto error;
				localEncodeFlags = pgpLocalEncodingToFlags( localEncode );
			}
		}
		localEncodeFlags |= kPGPFileOpenReadPerm;
		if( detachedSignature) {
			localEncodeFlags |= kPGPFileOpenLocalEncodeHashOnly;
		}

		/* Open input file */
		*pfrin = pgpFileRefReadCreate( context, *inFileRef,
							localEncodeFlags, &err, inFileDataType );
		if( IsNull( *pfrin ) ) {
			goto error;
		}
		if( IsntNull( rng ) ) {
			/* Add some data from the file to the RNG for more entropy */
			peekBuf = pgpFileReadPeek( *pfrin, &peekLength );
			pgpRandomAddBytes( rng, peekBuf, peekLength );
		}
	}

	/* See if there is a buffer specified */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_InputBuffer, &op ) ) )
		goto error;
	if( IsOp( op ) ) {
		/* Buffer input */
		if ( IsntNull( *inFileRef ) ) {
			/* Error if buffer input also specified */
			pgpDebugMsg( "Error: multiple input options" );
			err = kPGPError_BadParams;
			goto error;
		}
		if( IsntNull( literal ) ) {
			/*
			 * Fake up filename if FYEO mode above didn't set it.
			 * Earlier versions of PGP used the fake filename "stdin".
			 * However this cannot be distinguished from the case where
			 * a file with that name is encrypted.  We will change to using
			 * an empty string for this case, if no name was specified.
			 */
			if( IsNull( literal->filename ) ) {
				char *filerefname;
				void *vfilerefname;
				if( IsPGPError( err = pgpFindOptionArgs( optionList,
									 kPGPOptionType_InputFileName, FALSE,
									 "%p", &vfilerefname ) ) )
					goto error;
				filerefname = (char *)vfilerefname;
				if( IsNull( filerefname ) ) {
					filerefname = (char *)"";
				}
				literal->filename = pgpAllocCString( context, filerefname );
			}
		}
		if( IsPGPError( err = pgpOptionPtrLength( &op,
							   (void **)inBufPtr, inBufLength ) ) )
			goto error;
		if( IsntNull( rng ) ) {
			/* Pre-seed RNG with some entropy from buffer */
			pgpRandomAddBytes( rng, *inBufPtr, pgpMin( 1024, *inBufLength ) );
		}
	}

	/* Must have some input */
	if ( IsNull( *inBufPtr ) && IsNull( *inFileRef ) ) {
		pgpDebugMsg( "Error: no input options" );
		err = kPGPError_BadParams;
		goto error;
	}
	return kPGPError_NoErr;

error:
	*inFileRef = NULL;
	if( IsntNull( *pfrin ) ) {
		pgpFileReadDestroy( *pfrin );
		*pfrin = NULL;
	}
	*inBufPtr = NULL;
	*inBufLength = 0;
	if( IsntNull( literal ) ) {
		if( IsntNull( literal->filename ) ) {
			pgpContextMemFree( context, (char *)literal->filename );
			literal->filename = NULL;
		}
	}
	return err;
}


/*
 * Copy all of input to a buffer, from optionlist.  Leaves it in a
 * buffer if it is already there.  Returns *mustfree=TRUE if the
 * caller must free the buffer after using it, which will be the case
 * if we must allocate one.
 */
	PGPError
pgpSetupInputToBuffer(
	PGPContextRef	 	  context,		/* Input params */
	PGPOptionListRef	  optionList,
	PGPByte				**bufPtr,		/* Output params */
	PGPSize				 *bufLength,
	PGPBoolean			 *mustFree
	)
{
	PFLConstFileSpecRef	fileRef;
	PGPFileRead		   *pfrin = NULL;
	PGPPipeline		   *head = NULL;
	PGPFileDataType		inFileDataType;
	PGPError			err = kPGPError_NoErr;

	pgpAssert( IsntNull( bufPtr ) );
	pgpAssert( IsntNull( bufLength ) );
	pgpAssert( IsntNull( mustFree ) );

	*bufPtr = NULL;
	*bufLength = 0;
	*mustFree = FALSE;

	err = pgpSetupInput( context, optionList, NULL, NULL, FALSE, FALSE,
					&fileRef, &pfrin, &inFileDataType, bufPtr, bufLength );
	if( IsPGPError( err ) )
		goto error;

	if( IsntNull( pfrin ) ) {
		/* Read whole file into memory buffer */
		pgpVariableMemModCreate( context, &head, MAX_PGPSize );
		if( IsNull( head ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
		err = pgpFileReadPump( pfrin, head );
		if( IsPGPError( err ) )
			goto error;
		err = head->annotate( head, NULL, PGPANN_MEM_BUFSIZE,
							  (PGPByte *)bufLength, sizeof( *bufLength ) );
		if( IsPGPError( err ) )
			goto error;
		
		err = head->annotate( head, NULL, PGPANN_MEM_MEMBUF,
							  (PGPByte *)bufPtr, sizeof( *bufPtr ) );
		if( IsPGPError( err ) )
			return err;

		*mustFree = TRUE;
	}

 error:
	if( IsntNull( pfrin ) )
		pgpFileReadDestroy( pfrin );

	if( IsntNull( head ) )
		head->teardown( head );

	if( IsPGPError( err ) ) {
		*bufPtr = NULL;
		*bufLength = 0;
		mustFree = FALSE;
	}

	return err;
}


/*
 * Decode an input buffer which contains a PEM format base64 encoded
 * certificate.  Return in a dynamically allocated output buffer.
 */
	PGPError
pgpRemovePEMEncoding(
	PGPContextRef context,				/* Input params */
	PGPByte *inBuf,
	PGPSize inBufLength,
	PGPByte **pOutBuf,					/* Output params */
	PGPSize *pOutBufLength )
{
	void *outBuf = NULL;
	PGPSize outBufLength = 0;
	PGPSize inlen;
	PGPInt32 outlen;
	PGPError err = kPGPError_NoErr;
	PGPMemoryMgrRef mgr;
	static char sBegin[] = "-----BEGIN ";
	static char sEnd[] = "-----END ";

	pgpAssert( IsntNull( pOutBuf ) );
	pgpAssert( IsntNull( pOutBufLength ) );
	*pOutBuf = NULL;
	*pOutBufLength = 0;

	mgr = PGPGetContextMemoryMgr( context );
	outBuf = PGPNewData( mgr, 0, 0 );
	if( IsNull( outBuf ) ) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	pgpParseAscInit ();

	/* Skip leading spaces on each line */
	while (inBufLength && isspace(inBuf[0]))
		--inBufLength,++inBuf;

	if (inBufLength >= sizeof(sBegin)-1 &&
		pgpMemoryEqual (sBegin, inBuf, sizeof(sBegin)-1))
	{
		/* Must skip to end of line */
		while (inBufLength && (inBuf[0] != '\n') && (inBuf[0] != '\r'))
			--inBufLength,++inBuf;
	}
		
	for ( ; ; )
	{
		while (inBufLength && isspace(inBuf[0]))
			--inBufLength,++inBuf;
		if( inBufLength == 0 )
			break;
		if( inBufLength >= sizeof(sEnd)-1 &&
			pgpMemoryEqual (sEnd, inBuf, sizeof(sEnd)-1) )
			break;
		inlen = 0;
		while( inlen < inBufLength && (inBuf[inlen] != '\n' ) &&
			   inBuf[inlen] != '\r' )
			++inlen;
		if( inlen % 4 != 0 )
		{
			err = kPGPError_BadParams;
			goto error;
		}
		err = PGPReallocData( mgr, &outBuf, outBufLength + inlen*3/4, 0 );
		if( IsPGPError( err ) )
			goto error;
		outlen = pgpDearmorLine (inBuf, (PGPByte *)outBuf+outBufLength, inlen);
		if( outlen < 0 )
		{
			err = (PGPError) outlen;
			goto error;
		}
		pgpAssert( (PGPUInt32)outlen <= inlen*3/4 );
		outBufLength += outlen;
		inBuf += inlen;
		inBufLength -= inlen;
	}
	*pOutBuf = outBuf;
	*pOutBufLength = outBufLength;

 error:
	if( IsPGPError( err ) && IsntNull( outBuf ) )
		PGPFreeData( outBuf );

	return err;
}



/* Set up output file pipeline */

	PGPError
pgpSetupOutputPipeline(
	PGPContextRef	 	  context,		/* Input params */
	PGPOptionListRef	  optionList,
	PGPEnv				 *env,
	PGPBoolean			  fEncrypt,
	PGPBoolean			  fSign,
	PGPBoolean			  fDetachedSig,
	PGPBoolean			  fAppendOutput,
	PGPBoolean			  fExported509Keys,
	PGPPipeline		   ***tail,			/* Input/output param */
	PFLConstFileSpecRef	 *outFileRef,	/* Output params */
	PGPFile				 **pfout,
	PGPByte				**outBufPtr,
	PGPByte			   ***outBufPtrPtr,
	PGPSize				 *outBufLength,
	PGPSize				**outBufUsed,
	PGPPipeline			**outPipe
	)
{
	PGPOption			 op;			/* Selected option from list */
	PGPError			 err;			/* Error return code */
	PGPFileType			 filetype;		/* Type of output file we open */
	PGPUInt32			 fDiscard;		/* Discard output option flag */
	PGPBoolean			 fArmor;		/* Ascii armor */
	PGPMemoryMgrRef		memoryMgr	= NULL;

	/* Init return data to default states */
	pgpa( pgpaAddrValid( outFileRef, PFLConstFileSpecRef ) );
	pgpa( pgpaAddrValid( pfout, PGPFile * ) );
	pgpa( pgpaAddrValid( outBufPtr, PGPByte * ) );
	pgpa( pgpaAddrValid( outBufLength, PGPSize ) );
	pgpa( pgpaAddrValid( outBufUsed, PGPSize * ) );
	pgpa( pgpaAddrValid( outPipe, PGPPipeline * ) );
	*outFileRef = NULL;
	*pfout = NULL;
	*outBufPtr = NULL;
	*outBufLength = 0;
	*outBufUsed = NULL;
	*outPipe = NULL;
	
	memoryMgr	= PGPGetContextMemoryMgr( context );

	/* See if there is an output file specified */
	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
							 kPGPOptionType_OutputFileRef, &op ) ) )
		goto error;

	if( IsOp( op ) ) {
		/* Have an output file specified */
		PGPFileOpenFlags flags;

		if( IsPGPError( err = pgpOptionPtr( &op, (void **)outFileRef ) ) )
			goto error;

		/* See whether we are outputing an encrypted vs ascii armored file */
		fArmor = pgpenvGetInt( env, PGPENV_ARMOR, NULL, NULL );
		if( fArmor) {
			filetype = kPGPFileTypeArmorFile;
		} else if( fEncrypt ) {
			filetype = kPGPFileTypeEncryptedData;
		} else if( fSign ) {
			if( fDetachedSig ) {
				filetype = kPGPFileTypeDetachedSig;
			} else {
				filetype = kPGPFileTypeSignedData;
			}
		} else if( fExported509Keys ) {
			filetype = kPGPFileTypeExported509Keys;
		} else {
			pgpDebugMsg( "pgpSetupOutputPipeline(): Unknown file type" );
			filetype = kPGPFileTypeNone;
		}

		/* Open output file */
		flags = fAppendOutput ? kPGPFileOpenStdAppendFlags :
								kPGPFileOpenStdWriteFlags ;
		*pfout = pgpFileRefOpen( context, *outFileRef, flags, filetype, &err );
		if( IsNull( *pfout ) ) {
			goto error;
		}
		
		/* Set up output pipeline.  This reads and writes *tail. */
		if( IsNull( pgpFileWriteCreate( context,
					*tail, *pfout, 1 ) ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
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

		*outPipe = pgpMemModCreate( context, *tail, (char *) *outBufPtr,
									*outBufLength);
		if ( IsNull( *outPipe ) ) {
			err = kPGPError_OutOfMemory; /* What else could it be? */
			goto error;
		}
		if( fAppendOutput ) {
			/* Skip past existing buffer contents */
			if( IsPGPError( err = (*outPipe)->annotate( *outPipe, NULL,
					 PGPANN_MEM_PREPEND,
					 (unsigned char *)*outBufPtr, **outBufUsed ) ) )
				goto error;
		}
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

		*outPipe = pgpVariableMemModCreate( context,
						*tail, *outBufLength );
		if ( IsNull( *outPipe ) ) {
			err = kPGPError_OutOfMemory; /* What else could it be? */
			goto error;
		}
		if( fAppendOutput && **outBufUsed != 0 ) {
			/* Prepend existing buffer contents */
			if( IsPGPError( err = (*outPipe)->annotate( *outPipe, NULL,
					 PGPANN_MEM_PREPEND,
					 (unsigned char *)**outBufPtrPtr, **outBufUsed ) ) )
				goto error;
			/* Free buffer now that we have captured it */
			PGPFreeData( **outBufPtrPtr );
			**outBufPtrPtr = NULL;
		}
	}

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
		if ( IsNull( pgpDevNullCreate( context, *tail ) ) ) {
			err = kPGPError_OutOfMemory; /* What else could it be? */
			goto error;
		}
	} else if( IsNull( *outFileRef ) && IsNull( *outBufPtr )
			   						 && IsNull( *outBufPtrPtr ) ) {
		pgpDebugMsg( "Error: no output options" );
		err = kPGPError_BadParams;
		goto error;
	}
	return kPGPError_NoErr;

error:

	*outFileRef = NULL;
	if( IsntNull( *pfout ) ) {
		pgpFileClose( *pfout );
		*pfout = NULL;
	}
	*outBufPtr = NULL;
	*outBufPtrPtr = NULL;
	*outBufLength = 0;
	*outPipe = NULL;
	return err; 
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/