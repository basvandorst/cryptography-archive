/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CCRSVerisignServer.cpp,v 1.13 1999/03/13 03:15:21 jason Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpMem.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"

#include "StPGPRefs.h"
#include "CCRSVerisignServer.h"


// namespace {
		static const char *	kContentType	=	"application/octet-stream";
		static const char *	kDefaultPath	=	"/crs.exe";
// }


CCRSVerisignServer::CCRSVerisignServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	const char *			inPath,
	PGPKeyServerProtocol	inProtocol)
		: CCRSServer(inContext, inHostName, inHostAddress, inHostPort, inPath, inProtocol)
{
}



CCRSVerisignServer::~CCRSVerisignServer()
{
}



	void
CCRSVerisignServer::SendCertificateRequest(
	PGPKeyRef		inCAKey,
	PGPKeyRef		inRequestKey,
	const void *	inBuffer,
	PGPSize			inBufferSize,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	(void) inRequestKey;
	(void) inCAKey;
	
	try {
		InitOperation();
		if (IsNull(inBuffer)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}

		*outBufferSize = PostCRSMessage(kPGPKeyServerState_Uploading,
										(mPath == 0) ? kDefaultPath : mPath,
										reinterpret_cast<PGPByte **>(outBuffer),
										inBufferSize,
										static_cast<const PGPByte *>(inBuffer));
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
CCRSVerisignServer::RetrieveCertificate(
	PGPFilterRef	inSearchFilter,
	PGPKeyRef		inSearchKey,
	PGPKeyRef		inSigningKey,
	PGPByte *		inPassphrase,
	PGPSize			inPassphraseLength,
	PGPBoolean		inIsPassphrase,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	(void) inSearchFilter;

	try {
		PackageAndSendCRSMessage(	kPGPExportFormat_VerisignV1_GetCertInitial,
									kPGPOutputFormat_VerisignV1_GetCertInitialInPKCS7,
									inSearchKey,
									inSigningKey,
									inPassphrase,
									inPassphraseLength,
									inIsPassphrase,
									outBuffer,
									outBufferSize);
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
CCRSVerisignServer::RetrieveCRL(
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
	
	try {
		PackageAndSendCRSMessage(	kPGPExportFormat_VerisignV1_GetCRL,
									kPGPOutputFormat_VerisignV1_GetCRLInPKCS7,
									inCAKey,
									inSigningKey,
									inPassphrase,
									inPassphraseLength,
									inIsPassphrase,
									outBuffer,
									outBufferSize);
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
CCRSVerisignServer::PostCRSMessage(
	PGPKeyServerState	inOperation,
	const char *		inPath,
	PGPByte**			outResult,
	PGPUInt32			inContentLength,
	const PGPByte *		inData)
{
	StPGPDataRef	result;
	PGPSize			resultSize;

	resultSize = GetPost(inOperation, inPath, &result, kContentType, inContentLength, inData);
	CheckAndRemoveHTTPHeader(result, resultSize, outResult, &resultSize);

	return resultSize;
}



	void
CCRSVerisignServer::PackageAndSendCRSMessage(
	PGPExportFormat	inExportFormat,
	PGPOutputFormat	inOutputFormat,
	PGPKeyRef		inKey,
	PGPKeyRef		inSigningKey,
	PGPByte *		inPassphrase,
	PGPSize			inPassphraseLength,
	PGPBoolean		inIsPassphrase,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	PGPError		pgpErr;
	StPGPKeySetRef	singleKeySet;
	StPGPDataRef	buffer;
	PGPSize			bufSize;
	StPGPDataRef	crsMessage;
	PGPSize			crsMessageSize;
	
	InitOperation();
	if (! PGPKeyRefIsValid(inKey)) {
		ThrowPGPError_(kPGPError_OptionNotFound);
	}
	if (! PGPKeyRefIsValid(inSigningKey)) {
		ThrowPGPError_(kPGPError_OptionNotFound);
	}
	if (IsNull(inPassphrase)) {
		ThrowPGPError_(kPGPError_OptionNotFound);
	}
	
	pgpErr = PGPNewSingletonKeySet(inKey, &singleKeySet);
	ThrowIfPGPError_(pgpErr);
	pgpErr = PGPExportKeySet(	singleKeySet,
								PGPOExportFormat(	mContext,
													inExportFormat),
								PGPOAllocatedOutputBuffer(	mContext,
															(void **) &buffer,
															MAX_PGPSize,
															&bufSize),
								PGPOLastOption(mContext));
	ThrowIfPGPError_(pgpErr);
	singleKeySet.Free();
	pgpErr = PGPEncode(	mContext,
						PGPOSignWithKey(	mContext,
											inSigningKey,
											(inIsPassphrase) ?
												PGPOPassphraseBuffer(mContext,
													inPassphrase, inPassphraseLength) :
												PGPOPasskeyBuffer(mContext,
													inPassphrase, inPassphraseLength),
											PGPOLastOption(mContext)),
						PGPOInputBuffer(mContext, static_cast<PGPByte *>(buffer), bufSize),
						PGPOAllocatedOutputBuffer(	mContext,
													(void **) &crsMessage,
													MAX_PGPSize,
													&crsMessageSize),
						PGPOOutputFormat(	mContext,
											inOutputFormat),
						PGPOLastOption(mContext));
	ThrowIfPGPError_(pgpErr);
	buffer.Free();
	*outBufferSize = PostCRSMessage(kPGPKeyServerState_Querying,
									(mPath == 0) ? kDefaultPath : mPath,
									reinterpret_cast<PGPByte **>(outBuffer),
									crsMessageSize,
									static_cast<const PGPByte *>(crsMessage));
}