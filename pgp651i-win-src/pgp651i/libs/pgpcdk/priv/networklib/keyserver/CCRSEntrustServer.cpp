/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CCRSEntrustServer.cpp,v 1.12 1999/03/25 22:35:07 melkins Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

#include "StPGPRefs.h"

#include "CCRSEntrustServer.h"


// namespace {
	static const char *	kMessagePrefix	=	"operation=PKIOperation&message=";
	static const char *	kDefaultPath	=	"/cgi-bin/pkiclient.exe";
	static const char *	kContentType	=	"application/x-www-form-urlencoded";
// }


CCRSEntrustServer::CCRSEntrustServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	const char *			inPath,
	PGPKeyServerProtocol	inProtocol)
		: CCRSServer(inContext, inHostName, inHostAddress, inHostPort, inPath, inProtocol)
{
}



CCRSEntrustServer::~CCRSEntrustServer()
{
}



	void
CCRSEntrustServer::SendCertificateRequest(
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
		if (IsNull(inBuffer)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		*outBufferSize = PostMessage(	kPGPKeyServerState_Uploading,
										static_cast<const PGPByte *>(inBuffer),
										inBufferSize,
										reinterpret_cast<PGPByte **>(outBuffer));
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
CCRSEntrustServer::RetrieveCertificate(
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
		PackageAndSendMessage(	kPGPExportFormat_EntrustV1_GetCertInitial,
								kPGPOutputFormat_EntrustV1_GetCertInitialInPKCS7,
								inSearchKey,
								inSigningKey,
								inPassphrase,
								inPassphraseLength,
								inIsPassphrase,
								0,
								0,
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
CCRSEntrustServer::RetrieveCRL(
	PGPKeyRef		inCAKey,
	PGPKeySetRef	inCAsKeySet,
	PGPKeyRef		inSigningKey,
	PGPByte *		inPassphrase,
	PGPSize			inPassphraseLength,
	PGPBoolean		inIsPassphrase,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	try {
		PGPError		pgpErr;
		PGPUInt32		numPoints;
		StPGPDataRef	points;
		StPGPDataRef	pointSizes;
		PGPByte *		currPoint;
		PGPSize *		currPointSize;
		StPGPDataRef	crl;
		PGPSize			crlSize;
		StPGPDataRef	accumulatedCRLs = static_cast<PGPByte *>(PGPNewData(
											PGPGetContextMemoryMgr(mContext), 0,
											kPGPMemoryMgrFlags_None));
		PGPSize			accumulatedSize = 0;

		
		if (! PGPKeyRefIsValid(inCAKey)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		if (! PGPKeySetRefIsValid(inCAsKeySet)) {
			ThrowPGPError_(kPGPError_OptionNotFound);
		}
		if (accumulatedCRLs == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		pgpErr = PGPGetCRLDistributionPoints(inCAKey, inCAsKeySet, &numPoints, &points,
					&currPointSize);
		pointSizes.Set(reinterpret_cast<PGPByte *>(currPointSize));
		ThrowIfPGPError_(pgpErr);
		currPoint = points;
		for (PGPUInt32 i = 0; i < numPoints; i++) {
			PackageAndSendMessage(	kPGPExportFormat_EntrustV1_GetCRL,
									kPGPOutputFormat_EntrustV1_GetCRLInPKCS7,
									inCAKey,
									inSigningKey,
									inPassphrase,
									inPassphraseLength,
									inIsPassphrase,
									currPoint,
									*currPointSize,
									(void **) &crl,
									&crlSize);
			pgpErr = PGPReallocData(PGPGetContextMemoryMgr(mContext), (void **) &accumulatedCRLs,
						accumulatedSize + crlSize, kPGPMemoryMgrFlags_None);
			ThrowIfPGPError_(pgpErr);
#if PGP_COMPILER_SUN
			pgpCopyMemory(static_cast<PGPByte *>(crl), static_cast<PGPByte *>(accumulatedCRLs) + accumulatedSize, crlSize);
#else
			pgpCopyMemory(crl, static_cast<PGPByte *>(accumulatedCRLs) + accumulatedSize, crlSize);
#endif
			accumulatedSize += crlSize;
			currPoint += *currPointSize;
			currPointSize++;
		}
		*outBuffer = accumulatedCRLs;
		accumulatedCRLs.Set(0);
		*outBufferSize = accumulatedSize;
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
CCRSEntrustServer::PostMessage(
	PGPKeyServerState	inOperation,
	const PGPByte *		inMessage,
	PGPSize				inMessageLength,
	PGPByte**			outResult)
{
	StPGPDataRef	result;
	PGPSize			resultSize;
	StPGPDataRef	encodedBuffer;
	PGPSize			encodedSize;
	StPGPDataRef	urlEncodedBuffer;
	PGPSize			bufSize;
		
	InitOperation();
	bufSize = strlen(kMessagePrefix);
	encodedBuffer = static_cast<PGPByte *>(
						PGPNewData(	PGPGetContextMemoryMgr(mContext),
									GetMaxBase64EncodedBufferSize(inMessageLength),
									kPGPMemoryMgrFlags_None));
	if (encodedBuffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	encodedSize = Base64Encode(	static_cast<const PGPByte *>(inMessage),
								inMessageLength,
								static_cast<char *>(encodedBuffer),
								false);
	urlEncodedBuffer = static_cast<PGPByte *>(
						PGPNewData(	PGPGetContextMemoryMgr(mContext),
									GetMaxURLEncodedBufferSize(encodedSize) + bufSize,
									kPGPMemoryMgrFlags_None));
	if (encodedBuffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	strcpy(urlEncodedBuffer, kMessagePrefix);
	bufSize += URLEncode( static_cast<const char *>(encodedBuffer),
							encodedSize,
							static_cast<char *>(urlEncodedBuffer) + bufSize);
	encodedBuffer.Free();
	resultSize = GetPost(	inOperation,
							(mPath == 0) ? kDefaultPath : mPath,
							&result,
							kContentType,
							bufSize,
							urlEncodedBuffer);
	CheckAndRemoveHTTPHeader(result, resultSize, outResult, &resultSize);

	return resultSize;
}



	void
CCRSEntrustServer::PackageAndSendMessage(
	PGPExportFormat	inExportFormat,
	PGPOutputFormat	inOutputFormat,
	PGPKeyRef		inKey,
	PGPKeyRef		inSigningKey,
	PGPByte *		inPassphrase,
	PGPSize			inPassphraseLength,
	PGPBoolean		inIsPassphrase,
	PGPByte *		inDistributionPoint,
	PGPSize			inDistributionPointSize,
	void **			outBuffer,
	PGPSize *		outBufferSize)
{
	PGPError		pgpErr;
	StPGPKeySetRef	singleKeySet;
	StPGPDataRef	buffer;
	PGPSize			bufSize;
	StPGPDataRef	crsMessage;
	
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
								PGPOExportFormat(mContext, inExportFormat),
								(inDistributionPoint != 0) ?
									PGPOInputBuffer(mContext, inDistributionPoint,
										inDistributionPointSize) :
									PGPONullOption(mContext),
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
						PGPOAllocatedOutputBuffer(mContext, (void **) &crsMessage, MAX_PGPSize, &bufSize),
						PGPOOutputFormat(	mContext,
											inOutputFormat),
						PGPOLastOption(mContext));
	ThrowIfPGPError_(pgpErr);
	buffer.Free();
								
	*outBufferSize = PostMessage(	kPGPKeyServerState_Querying,
									crsMessage,
									bufSize,
									reinterpret_cast<PGPByte **>(outBuffer));
}
