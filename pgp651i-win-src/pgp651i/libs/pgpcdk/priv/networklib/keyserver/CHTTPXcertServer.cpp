/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CHTTPXcertServer.cpp,v 1.19 1999/03/13 03:15:30 jason Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpEventPriv.h"
#include "pgpHash.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpStrings.h"
#include "pgpUtilities.h"
#include "StPGPRefs.h"

#include "CHTTPXcertServer.h"

//namespace {
static const char *		kCA							=	"CA=";
static const char *		kPKCS10_input				=	"&pkcs10_input=";
static const char *		kCNK						=	"&CNK=";
static const char *		kContentType				=	"application/x-www-form-urlencoded";
static const char *		kDefaultRequestPath			=	"/add-pkcs10-request.xuda";
static const char *		kDefaultRetrievalPath		=	"/getcert2.xuda?";
static const char *		kDefaultCRLRetrievalPath	=	"/get-crl.xuda?md5=";
static const char *		kRequestAccepted			=	"Request Accepted";
static const char *		kBeginCertificate			=	"CERTIFICATE-----<BR>";
static const char *		kEndCertificate				=	"-----END";
static const char *		kBeginCRL					=	"BEGIN CRL-----<BR>";
static const char *		kEndCRL						=	"-----END";
static const char *		kHTMLBreak					=	"<BR>";

//}


CHTTPXcertServer::CHTTPXcertServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	const char *			inPath,
	PGPKeyServerProtocol	inProtocol)
	: CHTTPKeyServer(inContext, inHostName, inHostAddress, inHostPort, inPath, inProtocol)
{
}



CHTTPXcertServer::~CHTTPXcertServer()
{
}



	void
CHTTPXcertServer::SendCertificateRequest(
	PGPKeyRef		inCAKey,
	PGPKeyRef		inRequestKey,
	const void *	inBuffer,
	PGPSize			inBufferSize,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	try {
		PGPError			pgpErr;
		StPGPDataRef		result;
		StPGPDataRef		digest;
		PGPSize				digestSize;
		StPGPDataRef		cnk;
		PGPSize				cnkSize;
		StPGPDataRef		encodedCNK;
		PGPSize				bufSize;
		StPGPDataRef		post;
		PGPSize				postSize;
		StPGPDataRef		encodedBuffer;
		
		InitOperation();
		if (! PGPKeyRefIsValid(inCAKey)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		if (! PGPKeyRefIsValid(inRequestKey)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		
		digest = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
													GetMaxDigestedKeyBufferSize(),
													kPGPMemoryMgrFlags_None));
		if (digest == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		};
		digestSize = DigestKey(inCAKey, digest);
		
		encodedBuffer = static_cast<PGPByte *>(
			PGPNewData(	PGPGetContextMemoryMgr(mContext),
						GetMaxBase64EncodedBufferSize(inBufferSize),
						kPGPMemoryMgrFlags_None));
		if (encodedBuffer == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		};
		bufSize = Base64Encode(	static_cast<const PGPByte *>(inBuffer),
									inBufferSize,
									encodedBuffer);
		pgpErr = PGPGetKeyPropertyBuffer(inRequestKey, kPGPKeyPropX509MD5Hash, 0, 0, &cnkSize);
		ThrowIfPGPError_(pgpErr);
		cnk = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
													cnkSize,
													kPGPMemoryMgrFlags_None));
		if (cnk == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		pgpErr = PGPGetKeyPropertyBuffer(	inRequestKey,
											kPGPKeyPropX509MD5Hash,
											cnkSize,
											static_cast<PGPByte *>(cnk),
											&cnkSize);
		ThrowIfPGPError_(pgpErr);
		encodedCNK = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
														GetMaxHexEncodedBufferSize(cnkSize),
														kPGPMemoryMgrFlags_None));
		if (encodedCNK == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		cnkSize = HexEncode(cnk, cnkSize, encodedCNK);
		cnk.Free();
		post = static_cast<PGPByte *>(
			PGPNewData(	PGPGetContextMemoryMgr(mContext),
						strlen(kCA) + digestSize + strlen(kCNK) + cnkSize
							+ strlen(kPKCS10_input) + GetMaxURLEncodedBufferSize(bufSize),
						kPGPMemoryMgrFlags_None));
		if (post == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		};
		postSize = sprintf(post, "%s%s%s%s%s", kCA, static_cast<char *>(digest), kCNK,
						static_cast<char *>(encodedCNK), kPKCS10_input);
		digest.Free();
		encodedCNK.Free();
		postSize += URLEncode(encodedBuffer, bufSize, static_cast<char *>(post) + postSize);
		encodedBuffer.Free();
		GetPost(	kPGPKeyServerState_Uploading,
					(mPath == 0) ? kDefaultRequestPath : mPath,
					&result,
					kContentType,
					postSize,
					post);
		post.Free();
		ProcessSendCertResult(result, outBuffer, outBufferSize);
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
CHTTPXcertServer::RetrieveCertificate(
	PGPFilterRef	inSearchFilter,
	PGPKeyRef		inSearchKey,
	PGPKeyRef		inSigningKey,
	PGPByte *		inPassphrase,
	PGPSize			inPassphraseLength,
	PGPBoolean		inIsPassphrase,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	(void) inSearchKey;
	(void) inSigningKey;
	(void) inPassphrase;
	(void) inPassphraseLength;
	(void) inIsPassphrase;
	
	try {
		StPGPDataRef		query;
		StPGPDataRef		path;
		StPGPDataRef		result;
		PGPError			pgpErr;
		const char *		prefix = (mPath == 0) ? kDefaultRetrievalPath : mPath;
		
		InitOperation();
		if (! PGPFilterRefIsValid(inSearchFilter)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		
		pgpErr = PGPNetToolsCAHTTPQueryFromFilter(	inSearchFilter,
												static_cast<char **>(static_cast<void *>(&query)));
		ThrowIfPGPError_(pgpErr);
		path = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
													strlen(prefix) + strlen(query) + 1,
													kPGPMemoryMgrFlags_None));
		if (path == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		sprintf(path, "%s%s", prefix, static_cast<char *>(query));
		query.Free();

		GetPost(kPGPKeyServerState_Querying, path, &result);
		path.Free();
		
		pgpErr = pgpEventKeyServer(	mContext,
									mEventHandler,
									mEventHandlerData,
									this,
									kPGPKeyServerState_ProcessingResults);
		ThrowIfPGPError_(pgpErr);
		ProcessRetrieveCertResult(result, outBuffer, outBufferSize);
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
CHTTPXcertServer::RetrieveCRL(
	PGPKeyRef		inCAKey,
	PGPKeySetRef	inCAsKeySet,
	PGPKeyRef		inSigningKey,
	PGPByte *		inPassphrase,
	PGPSize			inPassphraseLength,
	PGPBoolean		inIsPassphrase,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	(void) inCAsKeySet;
	(void) inSigningKey;
	(void) inPassphrase;
	(void) inPassphraseLength;
	(void) inIsPassphrase;

	try {
		StPGPDataRef		digest;
		PGPUInt32			digestSize;
		StPGPDataRef		path;
		StPGPDataRef		result;
		PGPError			pgpErr;
		const char *		prefix = (mPath == 0) ? kDefaultCRLRetrievalPath : mPath;
		
		InitOperation();
		if (! PGPKeyRefIsValid(inCAKey)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		
		digest = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
													GetMaxDigestedKeyBufferSize(),
													kPGPMemoryMgrFlags_None));
		if (digest == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		};
		digestSize = DigestKey(inCAKey, digest);
		path = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
													strlen(prefix) + digestSize + 1,
													kPGPMemoryMgrFlags_None));
		if (path == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		sprintf(path, "%s%s", prefix, static_cast<char *>(digest));
		digest.Free();
		
		GetPost(kPGPKeyServerState_Querying, path, &result);
		path.Free();
		
		pgpErr = pgpEventKeyServer(	mContext,
									mEventHandler,
									mEventHandlerData,
									this,
									kPGPKeyServerState_ProcessingResults);
		ThrowIfPGPError_(pgpErr);
		
		ProcessRetrieveCRLResult(result, outBuffer, outBufferSize);
	}
	
	catch (...) {
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			throw;
		}
	}
}



	PGPUInt32
CHTTPXcertServer::DigestKey(
	PGPKeyRef	inKey,
	char *		inOutputBuffer)
{
	StPGPKeySetRef		singleKeySet;
	StPGPHashContextRef	hashContext;
	StPGPDataRef		buffer;
	PGPSize				bufSize;
	StPGPDataRef		encodedBuffer;
	PGPError			pgpErr;

	pgpErr = PGPNewSingletonKeySet(inKey, &singleKeySet);
	ThrowIfPGPError_(pgpErr);

	pgpErr = PGPExportKeySet(	singleKeySet,
								PGPOExportFormat(mContext, kPGPExportFormat_X509Cert),
								PGPOAllocatedOutputBuffer(	mContext,
															(void **) &buffer,
															MAX_PGPSize,
															&bufSize),
								PGPOLastOption(mContext));
	ThrowIfPGPError_(pgpErr);
	singleKeySet.Free();
	encodedBuffer = static_cast<PGPByte *>(
		PGPNewData(	PGPGetContextMemoryMgr(mContext),
					GetMaxBase64EncodedBufferSize(bufSize),
					kPGPMemoryMgrFlags_None));
	if (encodedBuffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	};
	bufSize = Base64Encode(	static_cast<const PGPByte *>(buffer),
							bufSize,
							encodedBuffer);
	buffer.Free();
	pgpErr = PGPNewHashContext(	PGPGetContextMemoryMgr(mContext),
								kPGPHashAlgorithm_MD5,
								&hashContext);
	ThrowIfPGPError_(pgpErr);
	
	// We have to skip the CRs in order to match XCert's hash
	char *	cur = encodedBuffer;
	char *	next;
	
	while ((next = strchr(cur, '\r')) != 0) {
		pgpErr = PGPContinueHash(hashContext, cur, next - cur);
		ThrowIfPGPError_(pgpErr);
		cur = next + 1;
	}
	pgpErr = PGPContinueHash(hashContext, cur, strlen(cur));
	ThrowIfPGPError_(pgpErr);
	encodedBuffer.Free();
	pgpErr = PGPGetHashSize(hashContext, &bufSize);
	buffer = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
												bufSize,
												kPGPMemoryMgrFlags_None));
	if (buffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	};
	pgpErr = PGPFinalizeHash(hashContext, static_cast<char *>(buffer));
	ThrowIfPGPError_(pgpErr);
	return HexEncode(buffer, bufSize, inOutputBuffer);
}



	void
CHTTPXcertServer::ProcessSendCertResult(
	const char *	inResult,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	*outBufferSize = strlen(inResult);
	*outBuffer = PGPNewData(	PGPGetContextMemoryMgr(mContext),
								*outBufferSize + 1,
								kPGPMemoryMgrFlags_None);
	if (*outBuffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	};
	pgpCopyMemory(inResult, *outBuffer, *outBufferSize + 1);
	
	// Look for the result
	char *	found;
	
	found = FindSubStringNoCase(inResult, kRequestAccepted);
	if (found == 0) {
		ThrowPGPError_(kPGPError_UnknownError);
	}
}



	void
CHTTPXcertServer::ProcessRetrieveCertResult(
	const char *	inResult,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	StPGPDataRef	result;
	StPGPDataRef	decodedResult;
	PGPSize			resultSize = strlen(inResult);
	
	result = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
												resultSize + 1,
												kPGPMemoryMgrFlags_None));
	if (result == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	};
	pgpCopyMemory(inResult, static_cast<char *>(result), resultSize + 1);
	
	char *	found;
	char *	end;
	
	found = FindSubStringNoCase(result, kBeginCertificate);
	if (found == 0) {
		ThrowPGPError_(kPGPError_ServerCertNotFound);
	}
	found += strlen(kBeginCertificate);
	end = FindSubStringNoCase(found, kEndCertificate);
	if (end == 0) {
		ThrowPGPError_(kPGPError_ServerCertNotFound);
	}
	
	if( end - found < 20 ) {
		/* Arbitrary check for "empty" certificate block. */
		ThrowPGPError_(kPGPError_ServerCertNotFound);
	}
	
	decodedResult = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
														GetMaxBase64DecodedBufferSize(end - found),
														kPGPMemoryMgrFlags_None));
	if (decodedResult == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	};
	
	// Remove any <BR>s
	char *		p = found;
	PGPUInt16	breakSize = strlen(kHTMLBreak);
	
	*end = 0;
	while ((p = FindSubStringNoCase(p, kHTMLBreak)) != 0) {
		for (PGPUInt16 i = 0; i < breakSize; i++, p++) {
			*p = ' ';
		}
	}
	
	*outBufferSize = Base64Decode(found, end - found, decodedResult);
	*outBuffer = static_cast<PGPByte *>(decodedResult);
	decodedResult = 0;
}



	void
CHTTPXcertServer::ProcessRetrieveCRLResult(
	const char *	inResult,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	StPGPDataRef	result;
	char *			found;
	char *			end;
	
	found = FindSubStringNoCase(inResult, kBeginCRL);
	if (found == 0) {
		ThrowPGPError_(kPGPError_ServerCertNotFound);
	}
	found += strlen(kBeginCRL);
	end = FindSubStringNoCase(found, kEndCRL);
	if (end == 0) {
		ThrowPGPError_(kPGPError_ServerCertNotFound);
	}
	result = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
												GetMaxBase64DecodedBufferSize(end - found),
												kPGPMemoryMgrFlags_None));
	if (result == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	};
	
	*outBufferSize = Base64Decode(found, end - found, result);
	*outBuffer = static_cast<PGPByte *>(result);
	result = 0;
}
