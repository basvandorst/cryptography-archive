/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: CHTTPKeyServer.cpp,v 1.24.10.1 1998/11/12 03:22:14 heller Exp $
____________________________________________________________________________*/

#include <string.h>
#include <ctype.h>

#include "pgpMem.h"
#include "pgpOptionList.h"
#include "pgpEventPriv.h"
#include "pgpKeys.h"
#include "pgpStrings.h"

#include "CHTTPKeyServer.h"



static const PGPUInt32	kDefaultBufferSize = 4096;
static const char *		kQueryPrefix = "GET /pks/lookup?op=get&";
static const char *		kQuerySuffix = " HTTP/1.0\x0D\x0A";
#if PGP_MACINTOSH
static const char *		kUserAgent = 
							"User-Agent: PGP (Macintosh)\x0D\x0A";
#elif PGP_WIN32
static const char *		kUserAgent = 
							"User-Agent: PGP (Windows)\x0D\x0A";
#elif PGP_UNIX
static const char *		kUserAgent = 
							"User-Agent: PGP (UNIX)\x0D\x0A";
#endif
static const char *		kUploadPrefix = "POST /pks/add HTTP/1.0\x0D\x0A";
static const char *		kContentLength = "Content-length: ";
static const char *		kContentType = 
							"Content-type: application/x-www-form-urlencoded\x0D\x0A\x0D\x0A";
static const char *		kPostPrefix = "keytext=";
static const char *		kHexString = "0123456789ABCDEF";
static const char *		kHeadBeginTag = "<head";
static const char *		kHeadEndTag = "</head>";
static const char *		kTitleBeginTag = "<title";
static const char *		kTitleEndTag = "</title>";
static const char *		kH1BeginTag = "<h1";
static const char *		kH1EndTag = "</h1>";
static const char *		kPreBeginTag = "<pre>";

static const char *		kKeyServerMessages[] =
	{ "Key block corrupt: pubkey decode failed",
	  "Key block added to key server database.\x0A"\
	  	"  New public keys added: ",
	  "Key block in add request contained no new\x0A"\
	  	"keys, userid's, or signatures.",
	  "Timed out waiting for LDAP response\x0A",
	  "No matches\x0A",
	  "No matching keys in database",
	  "-----BEGIN",
	  NULL };
static const PGPError	kKeyServerErrors[] =
	{ kPGPError_ServerCorruptKeyBlock,
	  kPGPError_NoErr,
	  kPGPError_ServerKeyAlreadyExists,
	  kPGPError_ServerTimedOut,
	  kPGPError_NoErr,
	  kPGPError_NoErr,
	  kPGPError_NoErr };


CHTTPKeyServer::CHTTPKeyServer(
	PGPContextRef		inContext,
	const char *		inHostName,
	PGPUInt32			inHostAddress,
	PGPUInt16			inHostPort,
	PGPKeyServerType	inType)
	: CKeyServer(inContext, inHostName, inHostAddress, inHostPort, inType),
	  mSocket(kInvalidPGPSocketRef)
{
}



CHTTPKeyServer::~CHTTPKeyServer()
{
}



	void
CHTTPKeyServer::Cancel()
{
	CKeyServer::Cancel();
	if (mIsBusy && (mSocket != kInvalidPGPSocketRef)) {
		PGPCloseSocket(mSocket);
		mSocket = kInvalidPGPSocketRef;
	}
}



	void
CHTTPKeyServer::Query(
	PGPFilterRef	inFilterRef,
	PGPKeySetRef *	outFoundKeys)
{
	StPreserveSocketsEventHandler	preserve(this);
	char *							query = NULL;
	PGPError						pgpError = kPGPError_NoErr;
	char *							buffer = NULL;
	PGPSize							bufferSize = 0;
		
	SetErrorString(NULL);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}

	try {
		// Establish connection
		PGPSocketAddressInternet	address;
		PGPInt32					socketResult;
		
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Opening);
		ThrowIfPGPError_(pgpError);
		
		mSocket = PGPOpenSocket(	kPGPAddressFamilyInternet,
									kPGPSocketTypeStream,
									kPGPTCPProtocol);
		if (mSocket == kInvalidPGPSocketRef) {
			ThrowPGPError_(PGPGetLastSocketsError());
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		}
		address.sin_family = kPGPAddressFamilyInternet;
		address.sin_port = PGPHostToNetShort(mHostPort);
		address.sin_addr.s_addr = PGPHostToNetLong(mHostAddress);
		socketResult = PGPConnect(	mSocket,
									(PGPSocketAddress *) &address,
									sizeof(address));
		if (socketResult == kPGPSockets_Error) {
			ThrowPGPError_(PGPGetLastSocketsError());
		}
		
		// Secure connection for HTTPS
		if (mType == kPGPKeyServerType_HTTPS) {
			if ((mTLSSession == kInvalidPGPtlsSessionRef)
			|| (PGPSocketsEstablishTLSSession(mSocket, mTLSSession) !=
			kPGPError_NoErr)) {
				pgpError = pgpEventKeyServerTLS(
								mContext,
								mEventHandler,
								mEventHandlerData,
								(PGPKeyServerRef) this,
								kPGPKeyServerState_TLSUnableToSecureConnection,
								mTLSSession);
			} else {
				pgpError = pgpEventKeyServerTLS(
								mContext,
								mEventHandler,
								mEventHandlerData,
								(PGPKeyServerRef) this,
								kPGPKeyServerState_TLSConnectionSecured,
								mTLSSession);
				mSecured = true;
			}
			ThrowIfPGPError_(pgpError);
		}
		
		// Build and send query
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Querying);
		ThrowIfPGPError_(pgpError);
		pgpError = PGPHKSQueryFromFilter(inFilterRef, &query);
		ThrowIfPGPError_(pgpError);
		bufferSize = strlen(kQueryPrefix) + strlen(query) +
						strlen(kQuerySuffix) + strlen(kUserAgent) + 3;
		buffer = new char[bufferSize];
		if (buffer == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		sprintf(	buffer,
					"%s%s%s%s%s",
					kQueryPrefix,
					query,
					kQuerySuffix,
					kUserAgent,
					"\x0D\x0A");
		PGPFreeData(query);
		query = NULL;
		socketResult = PGPSend(	mSocket,
								buffer,
								bufferSize - 1,
								kPGPSendFlagNone);
		if (socketResult != (PGPInt32) (bufferSize - 1)) {
			ThrowPGPError_(PGPGetLastSocketsError());
		}
		delete[] buffer;
		buffer = NULL;
		
		// Receive results
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_ProcessingResults);
		ThrowIfPGPError_(pgpError);
		bufferSize = ReceiveResult(&buffer);
		
		// Close connection
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Closing);
		ThrowIfPGPError_(pgpError);
		if (mSecured) {
			PGPtlsClose(mTLSSession, false);
			mSecured = false;
		}
		PGPCloseSocket(mSocket);
		mSocket = kInvalidPGPSocketRef;
		
		// Import the keyset
		pgpError = ProcessHTTPResult(buffer);
		ThrowIfPGPError_(pgpError);
		pgpError = PGPImportKeySet(	mContext,
									outFoundKeys,
									PGPOInputBuffer(	mContext,
														buffer,
														bufferSize),
									PGPOLastOption(mContext));
		ThrowIfPGPError_(pgpError);
		delete[] buffer;
	}
	
	catch (...) {
		if (buffer != NULL) {
			delete[] buffer;
		}
		if (query != NULL) {
			PGPFreeData(query);
		}
		if (mSocket != kInvalidPGPSocketRef) {
			PGPCloseSocket(mSocket);
			mSocket = kInvalidPGPSocketRef;
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			throw;
		}
	}
}
	
	
	
	void
CHTTPKeyServer::Upload(
	PGPKeyUploadPreference inSendPrivateKeys,
	PGPKeySetRef	inKeysToUpload,
	PGPKeySetRef *	outKeysThatFailed)
{
	StPreserveSocketsEventHandler	preserve(this);
	PGPError						pgpError = kPGPError_NoErr;
	PGPKeySetRef					errorKeySet = kInvalidPGPKeySetRef;
	PGPKeyListRef					keyList = kInvalidPGPKeyListRef;
	PGPKeyIterRef					keyIter = kInvalidPGPKeyIterRef;
	PGPKeySetRef					singleKeySet = kInvalidPGPKeySetRef;
	PGPByte *						buffer = NULL;
	char *							urlEncodedBuffer = NULL;
	char *							sendBuffer = NULL;
	char *							receiveBuffer = NULL;
	PGPBoolean						partialFailure = false;
	PGPBoolean						keyAlreadyExists = false;
	PGPBoolean						exportPrivateKeys = false;
	
	SetErrorString(NULL);
	if (! mIsOpen) {
		ThrowIfPGPError_(kPGPError_ServerNotOpen);
	}
	
	if (inSendPrivateKeys == kPGPPrivateKeyAllowed) {

		if (mType != kPGPKeyServerType_HTTPS) {
			ThrowPGPError_(kPGPError_ServerOperationRequiresTLS);
		}

		exportPrivateKeys = true;
	}

	try {
		PGPSocketAddressInternet	address;
		PGPSize						bufSize;
		PGPKeyRef					key;
		PGPInt32					socketResult;

		// Create error set
		pgpError = PGPNewKeySet(mContext, &errorKeySet);
		ThrowIfPGPError_(pgpError);
		
		// Iterate through keys uploading them one at a time
		pgpError = PGPOrderKeySet(	inKeysToUpload,
									kPGPAnyOrdering,
									&keyList);
		ThrowIfPGPError_(pgpError);
		pgpError = PGPNewKeyIter(keyList, &keyIter);
		ThrowIfPGPError_(pgpError);
		address.sin_family = kPGPAddressFamilyInternet;
		address.sin_port = PGPHostToNetShort(mHostPort);
		address.sin_addr.s_addr = PGPHostToNetLong(mHostAddress);
		while ((pgpError = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr) {
			// Export the key
			pgpError = PGPNewSingletonKeySet(key, &singleKeySet);
			ThrowIfPGPError_(pgpError);
			pgpError = PGPExportKeySet(
							singleKeySet,
							PGPOExportPrivateKeys(mContext,
												exportPrivateKeys),
							PGPOAllocatedOutputBuffer(
												mContext,
												(void **) &buffer,
												MAX_PGPSize,
												&bufSize),
							PGPOExportFormat(mContext,
											kPGPExportFormat_Basic),
							PGPOLastOption(mContext));

			ThrowIfPGPError_(pgpError);
			
			// URL encode the buffer
			urlEncodedBuffer = new char[(bufSize * 6) + 1];
			if (urlEncodedBuffer == NULL) {
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
			bufSize = URLEncode((char *) buffer, bufSize, urlEncodedBuffer);
			PGPFreeData(buffer);
			buffer = NULL;
			
			// Connect to server
			pgpError = pgpEventKeyServer(	mContext,
											mEventHandler,
											mEventHandlerData,
											(PGPKeyServerRef) this,
											kPGPKeyServerState_Opening);
			ThrowIfPGPError_(pgpError);
			mSocket = PGPOpenSocket(	kPGPAddressFamilyInternet,
										kPGPSocketTypeStream,
										kPGPTCPProtocol);
			if (mSocket == kInvalidPGPSocketRef) {
				ThrowPGPError_(PGPGetLastSocketsError());
			}
			if (mCanceled) {
				ThrowPGPError_(kPGPError_UserAbort);
			}
			socketResult = PGPConnect(	mSocket,
										(PGPSocketAddress *) &address,
										sizeof(address));
			if (socketResult == kPGPSockets_Error) {
				ThrowPGPError_(PGPGetLastSocketsError());
			}
			
			// Secure connection for HTTPS
			if (mType == kPGPKeyServerType_HTTPS) {
				if ((mTLSSession == kInvalidPGPtlsSessionRef)
				|| (PGPSocketsEstablishTLSSession(mSocket, mTLSSession) !=
				kPGPError_NoErr)) {
					pgpError = pgpEventKeyServerTLS(
									mContext,
									mEventHandler,
									mEventHandlerData,
									(PGPKeyServerRef) this,
								kPGPKeyServerState_TLSUnableToSecureConnection,
									mTLSSession);
				} else {
					pgpError = pgpEventKeyServerTLS(
									mContext,
									mEventHandler,
									mEventHandlerData,
									(PGPKeyServerRef) this,
									kPGPKeyServerState_TLSConnectionSecured,
									mTLSSession);
					mSecured = true;
				}
				ThrowIfPGPError_(pgpError);
			}
			
			// Send the buffer
			pgpError = pgpEventKeyServer(	mContext,
											mEventHandler,
											mEventHandlerData,
											(PGPKeyServerRef) this,
											kPGPKeyServerState_Uploading);
			ThrowIfPGPError_(pgpError);
			sendBuffer = new char[bufSize + 256];
			if (sendBuffer == NULL) {
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
			bufSize = sprintf(	sendBuffer,
								"%s%s%s%lu%s%s%s%s",
								kUploadPrefix,
								kUserAgent,
								kContentLength,
								(unsigned long) bufSize + strlen(kPostPrefix),
								"\x0D\x0A",
								kContentType,
								kPostPrefix,
								urlEncodedBuffer);
			delete[] urlEncodedBuffer;
			urlEncodedBuffer = NULL;
			socketResult = PGPSend(	mSocket,
									sendBuffer,
									bufSize,
									kPGPSendFlagNone);
			if (socketResult != (PGPInt16) bufSize) {
				ThrowPGPError_(PGPGetLastSocketsError());
			}
			delete[] sendBuffer;
			sendBuffer = NULL;
			
			// Receive result
			pgpError = pgpEventKeyServer(
									mContext,
									mEventHandler,
									mEventHandlerData,
									(PGPKeyServerRef) this,
									kPGPKeyServerState_ProcessingResults);
			ThrowIfPGPError_(pgpError);
			bufSize = ReceiveResult(&receiveBuffer);
			pgpError = ProcessHTTPResult(receiveBuffer);
			delete[] receiveBuffer;
			receiveBuffer = NULL;
			if (IsPGPError(pgpError)) {
				PGPAddKeys(singleKeySet, errorKeySet);
				partialFailure = true;
				if (pgpError == kPGPError_ServerKeyAlreadyExists) {
					keyAlreadyExists = true;
				}
			}
			PGPFreeKeySet(singleKeySet);
			singleKeySet = kInvalidPGPKeySetRef;
			
			// Close connection
			pgpError = pgpEventKeyServer(	mContext,
											mEventHandler,
											mEventHandlerData,
											(PGPKeyServerRef) this,
											kPGPKeyServerState_Closing);
			ThrowIfPGPError_(pgpError);
			if (mSecured) {
				PGPtlsClose(mTLSSession, FALSE);
				mSecured = FALSE;
			}
			PGPCloseSocket(mSocket);
			mSocket = kInvalidPGPSocketRef;
		}
		
		if (IsPGPError(pgpError) && (pgpError != kPGPError_EndOfIteration)) {
			ThrowPGPError_(pgpError);
		}
		PGPFreeKeyIter(keyIter);
		keyIter = kInvalidPGPKeyIterRef;
		PGPFreeKeyList(keyList);
		keyList = kInvalidPGPKeyListRef;
		if (partialFailure) {
			*outKeysThatFailed = errorKeySet;
			errorKeySet = kInvalidPGPKeySetRef;
			if (keyAlreadyExists) {
				ThrowPGPError_(kPGPError_ServerKeyAlreadyExists);
			} else {
				ThrowPGPError_(kPGPError_ServerPartialAddFailure);
			}
		} else {
			PGPFreeKeySet(errorKeySet);
		}
	}
	
	catch (...) {
		if (singleKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(singleKeySet);
		}
		if (buffer != NULL) {
			PGPFreeData(buffer);
		}
		delete[] urlEncodedBuffer;
		delete[] sendBuffer;
		delete[] receiveBuffer;
		if (keyIter != kInvalidPGPKeyIterRef) {
			PGPFreeKeyIter(keyIter);
		}
		if (keyList != kInvalidPGPKeyListRef) {
			PGPFreeKeyList(keyList);
		}
		if (errorKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(errorKeySet);
		}
		if (mSocket != kInvalidPGPSocketRef) {
			PGPCloseSocket(mSocket);
			mSocket = kInvalidPGPSocketRef;
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			throw;
		}
	}
}

	PGPUInt32
CHTTPKeyServer::ReceiveResult(
	char **	outBuffer)
{
	PGPSize				result = 0;
	PGPSize				bufferSize = kDefaultBufferSize;
	char *				buffer = NULL;
	char *				tempBuffer = NULL;
	PGPInt32			socketResult = 0;
	char *				temp = NULL;
	
	try {
		// Receive the data
		tempBuffer = new char[kDefaultBufferSize];
		if (tempBuffer == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		buffer = new char[bufferSize + 1];
		if (buffer == NULL) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		socketResult = PGPReceive(	mSocket,
									tempBuffer,
									kDefaultBufferSize,
									kPGPReceiveFlagNone);
		while (socketResult > 0) {
			if ((result + socketResult) > bufferSize) {
				temp = NULL;
				
				bufferSize += kDefaultBufferSize;
				temp = new char[bufferSize + 1];
				if (temp == NULL) {
					ThrowPGPError_(kPGPError_OutOfMemory);
				}
				pgpCopyMemory(buffer, temp, result);
				delete[] buffer;
				buffer = temp;
			}
			pgpCopyMemory(tempBuffer, buffer + result, socketResult);
			result += socketResult;
			socketResult = PGPReceive(	mSocket,
										tempBuffer,
										kDefaultBufferSize,
										kPGPReceiveFlagNone);
		}
		delete[] tempBuffer;
		tempBuffer = NULL;
		
		// Check for errors
		if (socketResult != 0) {
			ThrowIfPGPError_(PGPGetLastSocketsError());
		}
		if (buffer == NULL) {
			ThrowPGPError_(kPGPError_ServerRequestFailed);
		}
		
		// Everything good
		buffer[result] = '\0';
		*outBuffer = buffer;
	}
	
	catch (PGPError) {
		delete[] tempBuffer;
		delete[] buffer;
		throw;
	}
	
	return result;
}



	PGPUInt32
CHTTPKeyServer::URLEncode(
	char *		inBuffer,
	PGPUInt32	inBufSize,
	char *		inURLEncodedBuffer)
{
	char *	p = inURLEncodedBuffer;
	
	for (PGPUInt32 i = 0; i < inBufSize; i++) {
		if ((! isalnum(inBuffer[i])) && (inBuffer[i] != '-')) {
			if (inBuffer[i] == ' ') {
				*p++ = '+';
			} else if (((inBuffer[i] == '\x0D') && (inBuffer[i+1] != '\x0A'))
			|| ((inBuffer[i+1] == '\x0A') && (inBuffer[i] != '\x0D'))) {
				*p++ = '%';
				*p++ = '0';
				*p++ = 'D';
				*p++ = '%';
				*p++ = '0';
				*p++ = 'A';
			} else {
				*p++ = '%';
				*p++ = kHexString[inBuffer[i] >> 4];;
				*p++ = kHexString[inBuffer[i] & 0x0F];
			}
		} else {
			*p++ = inBuffer[i];
		}		
	}
	
	*p = '\0';
	
	return (p - inURLEncodedBuffer);
}


/*
	PGPError
CHTTPKeyServer::ProcessHTTPResult(
	char *	inResult)
{
	PGPError	result = kPGPError_ServerUnknownResponse;
	char *		currentItem;
	char		endChar;
	char *		endOfItem;
	
	// Skip over HTTP headers
	currentItem = strstr(inResult, "\x0D\x0A\x0D\x0A");
	if (currentItem != NULL) {
		while (isspace(*currentItem)) {
			currentItem++;
		}
		
		// Skip over title
		currentItem = strchr(currentItem, '\x0A');
		if (currentItem != NULL) {
			while (isspace(*currentItem)) {
				currentItem++;
			}
			
			// Skip over header
			currentItem = strchr(currentItem, '\x0A');
			if (currentItem != NULL) {
				while (isspace(*currentItem)) {
					currentItem++;
				}
				
				// Apparently some of the messages begin with <pre>
				if (strncmp(currentItem, "<pre>", 5) == 0) {
					currentItem += 5;
					while (isspace(*currentItem)) {
						currentItem++;
					}
				}
				
				// To quote Mark McArdle's original code, "God willing and the
				// creek don't rise" we now point to the actual error message
				for (PGPUInt16 i = 0; kKeyServerMessages[i] != NULL; i++) {
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
		}
	}
	
	return result;
}
*/



	PGPError
CHTTPKeyServer::ProcessHTTPResult(
	char *	inResult)
{
	PGPError	result = kPGPError_ServerUnknownResponse;
	char *		currentItem;
	char		endChar;
	char *		endOfItem;
	
	// Skip over HTTP headers
	currentItem = strstr(inResult, "\x0D\x0A\x0D\x0A");
	if (currentItem != NULL) {
		// Skip head
		endOfItem = FindSubStringNoCase(currentItem, kHeadBeginTag);
		if (endOfItem != NULL) {
			endOfItem = FindSubStringNoCase(endOfItem, kHeadEndTag);
			if (endOfItem != NULL) {
				currentItem = endOfItem + strlen(kHeadEndTag);
			}
		}
			
		// Skip title
		endOfItem = FindSubStringNoCase(currentItem, kTitleBeginTag);
		if (endOfItem != NULL) {
			endOfItem = FindSubStringNoCase(endOfItem, kTitleEndTag);
			if (endOfItem != NULL) {
				currentItem = endOfItem + strlen(kTitleEndTag);
			}
		}


		// Skip H1
		endOfItem = FindSubStringNoCase(currentItem, kH1BeginTag);
		if (endOfItem != NULL) {
			endOfItem = FindSubStringNoCase(endOfItem, kH1EndTag);
			if (endOfItem != NULL) {
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
		for (PGPUInt16 i = 0; kKeyServerMessages[i] != NULL; i++) {
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
