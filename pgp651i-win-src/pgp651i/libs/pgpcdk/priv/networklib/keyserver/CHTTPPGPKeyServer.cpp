/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CHTTPPGPKeyServer.cpp,v 1.6.8.1 1999/06/04 00:28:53 heller Exp $
____________________________________________________________________________*/

#include <ctype.h>
#include <string.h>

#include "pgpEventPriv.h"
#include "pgpKeys.h"
#include "pgpStrings.h"
#include "pgpUtilities.h"

#include "StPGPRefs.h"

#include "CHTTPPGPKeyServer.h"


//namespace {
static const char *	kQueryPrefix	=	"/pks/lookup?op=get&";
static const char *	kUploadPath		=	"/pks/add";
static const char *	kContentType	=	"application/x-www-form-urlencoded";
static const char *	kPostPrefix		=	"keytext=";
static const char *	kHeadBeginTag	=	"<head";
static const char *	kHeadEndTag		=	"</head>";
static const char *	kTitleBeginTag	=	"<title";
static const char *	kTitleEndTag	=	"</title>";
static const char *	kH1BeginTag		=	"<h1";
static const char *	kH1EndTag		=	"</h1>";

static const char *	kKeyServerMessages[] =
		{ "Key block corrupt: pubkey decode failed",
		  "Key block added to key server database.\x0A  New public keys added: ",
		  "Key block in add request contained no new\x0Akeys, userid's, or signatures.",
		  "Timed out waiting for LDAP response\x0A",
		  "No matches\x0A",
		  "No matching keys in database",
		  "-----BEGIN",
		  0 };
static const PGPError	kKeyServerErrors[] =
		{ kPGPError_ServerCorruptKeyBlock,
		  kPGPError_NoErr,
		  kPGPError_ServerKeyAlreadyExists,
		  kPGPError_ServerTimedOut,
		  kPGPError_NoErr,
		  kPGPError_NoErr,
		  kPGPError_NoErr };
//}


CHTTPPGPKeyServer::CHTTPPGPKeyServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	PGPKeyServerProtocol	inType)
		: CHTTPKeyServer(inContext, inHostName, inHostAddress, inHostPort, 0, inType)
{
}



CHTTPPGPKeyServer::~CHTTPPGPKeyServer()
{
}





	void
CHTTPPGPKeyServer::Query(
	PGPFilterRef	inFilterRef,
	PGPKeySetRef *	outFoundKeys)
{
	try {
		StPGPDataRef		query;
		StPGPDataRef		path;
		StPGPDataRef		result;
		PGPUInt32			resultSize;
		PGPError			pgpErr;
		
		InitOperation();
		
		pgpErr = PGPHKSQueryFromFilter(	inFilterRef,
										static_cast<char **>(static_cast<void *>(&query)));
		ThrowIfPGPError_(pgpErr);
		path = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
													strlen(kQueryPrefix) + strlen(query) + 1,
													kPGPMemoryMgrFlags_None));
		if (path == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		sprintf(path, "%s%s", kQueryPrefix, static_cast<char *>(query));
		query.Free();
		
		resultSize = GetPost(kPGPKeyServerState_Querying, path, &result);
		path.Free();
		
		pgpErr = pgpEventKeyServer(	mContext,
									mEventHandler,
									mEventHandlerData,
									this,
									kPGPKeyServerState_ProcessingResults);
		ThrowIfPGPError_(pgpErr);
		
		pgpErr = ProcessHTTPResult(result);
		ThrowIfPGPError_(pgpErr);
		pgpErr = PGPImportKeySet(	mContext,
									outFoundKeys,
									PGPOInputBuffer(	mContext,
														static_cast<PGPByte *>(result),
														resultSize),
									PGPOLastOption(mContext));
		ThrowIfPGPError_(pgpErr);
	}
	
	catch (...) {
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			throw;
		}
	}
}



	void
CHTTPPGPKeyServer::Upload(
	PGPKeyUploadPreference	inSendPrivateKeys,
	PGPKeySetRef			inKeysToUpload,
	PGPKeySetRef *			outKeysThatFailed)
{
	try {
		StPGPKeySetRef	singleKeySet;
		StPGPKeySetRef	keysThatFailed;
		StPGPKeyListRef	keyList;
		StPGPKeyIterRef	keyIter;
		StPGPDataRef	buffer;
		PGPSize			bufSize;
		StPGPDataRef	urlEncodedBuffer;
		PGPKeyRef		key;
		PGPSize			postPrefixSize = strlen(kPostPrefix);
		PGPBoolean		partialFailure = false;
		PGPBoolean		keyAlreadyExists = false;
		PGPError		pgpErr;
		
		InitOperation();
		if (inSendPrivateKeys == kPGPPrivateKeyAllowed) {

			if (mType != kPGPKeyServerProtocol_HTTPS) {
				ThrowPGPError_(kPGPError_ServerOperationRequiresTLS);
			}
		}

		pgpErr = PGPNewKeySet(mContext, &keysThatFailed);
		ThrowIfPGPError_(pgpErr);

		// Iterate through keys uploading them one at a time
		pgpErr = PGPOrderKeySet(	inKeysToUpload,
									kPGPAnyOrdering,
									&keyList);
		ThrowIfPGPError_(pgpErr);
		pgpErr = PGPNewKeyIter(keyList, &keyIter);
		ThrowIfPGPError_(pgpErr);
		while ((pgpErr = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr) {
			pgpErr = PGPNewSingletonKeySet(key, &singleKeySet);
			ThrowIfPGPError_(pgpErr);
			pgpErr = PGPExportKeySet(	singleKeySet,
										PGPOExportPrivateKeys(	mContext,
																inSendPrivateKeys == 
																	kPGPPrivateKeyAllowed),
										PGPOAllocatedOutputBuffer(	mContext,
																	(void **) &buffer,
																	MAX_PGPSize,
																	&bufSize),
										PGPOExportFormat(	mContext,
															kPGPExportFormat_Basic),
										PGPOLastOption(mContext));

			ThrowIfPGPError_(pgpErr);
			urlEncodedBuffer = static_cast<PGPByte *>(
				PGPNewData(	PGPGetContextMemoryMgr(mContext),
							GetMaxURLEncodedBufferSize(bufSize) + postPrefixSize,
							kPGPMemoryMgrFlags_None));
			if (urlEncodedBuffer == 0) {
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
			strcpy(urlEncodedBuffer, kPostPrefix);
			bufSize = URLEncode(buffer, bufSize, static_cast<char *>(urlEncodedBuffer)
				+ postPrefixSize) + postPrefixSize;
			buffer.Free();
			bufSize = GetPost(	kPGPKeyServerState_Uploading,
								kUploadPath,
								&buffer,
								kContentType,
								bufSize,
								urlEncodedBuffer);
			urlEncodedBuffer.Free();
			pgpErr = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										this,
										kPGPKeyServerState_ProcessingResults);
			ThrowIfPGPError_(pgpErr);
			pgpErr = ProcessHTTPResult(buffer);
			buffer.Free();
			if (IsPGPError(pgpErr)) {
				PGPAddKeys(singleKeySet, keysThatFailed);
				partialFailure = true;
				if (pgpErr == kPGPError_ServerKeyAlreadyExists) {
					keyAlreadyExists = true;
				}
			}
			singleKeySet.Free();
		}
		if (IsPGPError(pgpErr) && (pgpErr != kPGPError_EndOfIteration)) {
			ThrowPGPError_(pgpErr);
		}
		if (partialFailure) {
			*outKeysThatFailed = keysThatFailed;
			keysThatFailed = kInvalidPGPKeySetRef;
			if (keyAlreadyExists) {
				ThrowPGPError_(kPGPError_ServerKeyAlreadyExists);
			} else {
				ThrowPGPError_(kPGPError_ServerPartialAddFailure);
			}
		}
	}
	
	catch (...) {
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			throw;
		}
	}
}



	PGPError
CHTTPPGPKeyServer::ProcessHTTPResult(
	const char *	inResult)
{
	PGPError	result = kPGPError_ServerUnknownResponse;
	char *		currentItem;
	char		endChar;
	char *		endOfItem;
	
	// Skip over HTTP headers
	currentItem = strstr(inResult, "\r\n\r\n");
	if (currentItem != 0) {
		// Skip head
		endOfItem = FindSubStringNoCase(currentItem, kHeadBeginTag);
		if (endOfItem != 0) {
			endOfItem = FindSubStringNoCase(endOfItem, kHeadEndTag);
			if (endOfItem != 0) {
				currentItem = endOfItem + strlen(kHeadEndTag);
			}
		}
			
		// Skip title
		endOfItem = FindSubStringNoCase(currentItem, kTitleBeginTag);
		if (endOfItem != 0) {
			endOfItem = FindSubStringNoCase(endOfItem, kTitleEndTag);
			if (endOfItem != 0) {
				currentItem = endOfItem + strlen(kTitleEndTag);
			}
		}


		// Skip H1
		endOfItem = FindSubStringNoCase(currentItem, kH1BeginTag);
		if (endOfItem != 0) {
			endOfItem = FindSubStringNoCase(endOfItem, kH1EndTag);
			if (endOfItem != 0) {
				currentItem = endOfItem + strlen(kH1EndTag);
			}
		}
		
		// Skip all prefix tags and whitespace
		PGPBoolean	skip;
		PGPBoolean	inTag = false;
		
		do {
			skip = false;
			if (isspace(*currentItem)) {
				skip = true;
			} else {
				if (inTag) {
					skip = true;
					if (*currentItem == '>') {
						inTag = false;
					}
				} else {
					if (*currentItem == '<') {
						skip = true;
						inTag = true;
					}
				}
			}
			if (skip) {
				currentItem++;
			}
		} while (skip);

		// To quote Mark McArdle's original code, "God willing and the
		// creek don't rise" we now point to the actual error message
		for (PGPUInt16 i = 0; kKeyServerMessages[i] != 0; i++) {
			if ( strncmp(kKeyServerMessages[i], currentItem,
			strlen(kKeyServerMessages[i])) == 0) {
				result = kKeyServerErrors[i];
				break;
			}
		}
		
		// Set the error string if necessary
		if (result != kPGPError_NoErr) {
			endOfItem = currentItem;
			while ((*endOfItem != '\0') && (*endOfItem != '\x0D')
			&& (*endOfItem != '\x0A') && (*endOfItem != '<')) {
				endOfItem++;
			}
			
			endChar = *endOfItem;
			*endOfItem = '\0';
			SetErrorString(currentItem);
			*endOfItem = endChar;
		}
	}
	
	return result;
}
