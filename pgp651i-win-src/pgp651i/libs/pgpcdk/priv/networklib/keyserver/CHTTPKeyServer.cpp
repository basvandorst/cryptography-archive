/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CHTTPKeyServer.cpp,v 1.39.6.1 1999/06/04 19:00:53 heller Exp $
____________________________________________________________________________*/

#include <ctype.h>
#include <string.h>

#include "pgpEventPriv.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpMemoryMgr.h"
#include "pgpProxyServer.h"
#include "pgpUtilities.h"

#include "StPGPRefs.h"

#include "CHTTPKeyServer.h"


//namespace {
static const PGPUInt16	kDefaultProxyPort	=	80;
static const PGPSize	kTempStorageSize	=	4028;
static const char *	kHexString			=	"0123456789ABCDEF";
static const char *	kBase64Table		=	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef"
											"ghijklmnopqrstuvwxyz0123456789+/";
static const char *	kGet				=	"GET ";
static const char *	kPost				=	"POST ";
static const char *	kHTTPVersion		=	" HTTP/1.0";
static const char *	kContentLength		=	"\r\nContent-length: ";
static const char *	kContentType		=	"\r\nContent-type: ";
#if PGP_MACINTOSH
static const char *	kUserAgent 			= "\r\nUser-Agent: PGP (Macintosh)\r\n\r\n";
#elif PGP_WIN32
static const char *	kUserAgent 			= "\r\nUser-Agent: PGP (Windows)\r\n\r\n";
#elif PGP_UNIX
static const char *	kUserAgent 			= "\r\nUser-Agent: PGP (UNIX)\r\n\r\n";
#endif

	class StCloseAndInvalidateSocket {
	public:
						StCloseAndInvalidateSocket(PGPSocketRef * inSocket)
							{ mSocket = inSocket; }
		virtual			~StCloseAndInvalidateSocket()
							{ PGPCloseSocket(*mSocket); *mSocket = kInvalidPGPSocketRef; }
		PGPSocketRef *	mSocket;
	};
//}


CHTTPKeyServer::CHTTPKeyServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	const char *			inPath,
	PGPKeyServerProtocol	inType)
		: CKeyServer(inContext, inHostName, inHostAddress, inHostPort, inPath, inType),
		  mSocket(kInvalidPGPSocketRef), mProxyName(0), mProxyAddress(0), mProxyPort(kDefaultProxyPort)
{
}



CHTTPKeyServer::~CHTTPKeyServer()
{
	delete mProxyName;
}



	void
CHTTPKeyServer::Open(
	PGPtlsSessionRef	inTLSSession)
{
	PGPError	pgpErr;
	char *		temp;
	
	CKeyServer::Open(inTLSSession);

	if (mType == kPGPKeyServerProtocol_HTTP) {
		pgpErr = PGPGetProxyServer(mContext, kPGPProxyServerType_HTTP, &temp, &mProxyPort);
		ThrowIfPGPError_(pgpErr);
		if (temp != NULL) {
			mProxyName = new char[strlen(temp) + 1];
			if (mProxyName != NULL) {
				strcpy(mProxyName, temp);
				PGPFreeData(temp);
				InitializeHostNameAndAddress(&mProxyName, &mProxyAddress);
			} else {
				PGPFreeData(temp);
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
		}
	}
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



	PGPUInt32
CHTTPKeyServer::URLEncode(
	const char *	inBuffer,
	PGPUInt32		inBufSize,
	char *			inOutputBuffer)
{
	char *	p = inOutputBuffer;
	
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
	
	return (p - inOutputBuffer);
}



	PGPUInt32
CHTTPKeyServer::Base64Encode(
	const PGPByte *	inBuffer,
	PGPUInt32 		inBufSize,
	char *			inOutputBuffer,
	PGPBoolean		inLineBreak)
{
	char *		p = inOutputBuffer;
	PGPUInt32	bufferIndex = 0;
	PGPUInt32	offset = 0;
	
	if (inBufSize > 0) {
		for (;;) {
			*p++ = kBase64Table[(inBuffer[bufferIndex] >> 2) & 0x3F];
			if (++bufferIndex == inBufSize) {
				*p++ = kBase64Table[(inBuffer[bufferIndex - 1] << 4) & 0x30];
				break;
			}

			*p++ = kBase64Table[((inBuffer[bufferIndex - 1] << 4) & 0x30) + ((inBuffer[bufferIndex] >> 4) & 0x0F)];
			if (++bufferIndex == inBufSize) {
				*p++ = kBase64Table[(inBuffer[bufferIndex - 1] << 2) & 0x3C];
				break;
			}
			*p++ = kBase64Table[((inBuffer[bufferIndex - 1] << 2) & 0x3C) + ((inBuffer[bufferIndex] >> 6) & 0x03)];
			*p++ = kBase64Table[inBuffer[bufferIndex] & 0x3F];
			
			// Linebreak every 64 characters
			if (((((p - offset) - inOutputBuffer) % 64) == 0) && inLineBreak) {
				*p++ = '\r';
				*p++ = '\n';
				offset += 2;
			}
			if (++bufferIndex == inBufSize) {
				break;
			}
		}

		for (PGPInt32 x = ((inBufSize % 3) == 0) ? 0 : 3 - (inBufSize % 3); x > 0; x--) {
			*p++ = '=';
		}
		if (inLineBreak) {
			*p++ = '\r';
			*p++ = '\n';
		}

		*p = 0;
	}
	
	return p - inOutputBuffer;
}


	PGPUInt32
CHTTPKeyServer::DecodeBase64Morsel(
	const char *	inBuffer,
	PGPUInt32		inBufSize,
	PGPByte			outByte[3],
	PGPUInt16 *		outNumGood)
{
	const char *	curBuffer = inBuffer;
	const char *	end = inBuffer + inBufSize;
	PGPByte			byte[4];
	char *			p;
	PGPUInt16		i;
	
	*outNumGood = 0;
	for (i = 0; i < 4; i++) {
		while ((curBuffer != end) && isspace(*curBuffer)) {
			curBuffer++;
		}
		if (curBuffer == end) {
			break;
		}
		p = strchr(kBase64Table, *curBuffer++);
		if (p == 0) {
			// Do some sanity checking
			if ((i == 1)
			|| ((i == 2) && ((*(curBuffer - 1) != '=') || (*curBuffer != '=')))
			|| ((i == 3) && (*(curBuffer - 1) != '='))) {
				*outNumGood = 5; // Used as error condition
			}
			break;
		} else {
			byte[i] = p - kBase64Table;
		}	
	}
	
	if (*outNumGood != 5) {
		if (i > 1) {
			outByte[0] = (byte[0] << 2) + (byte[1] >> 4);
			*outNumGood = 1;
			if (i > 2) {
				outByte[1] = ((byte[1] & 0x0F) << 4) + (byte[2] >> 2);
				*outNumGood = 2;
				if (i == 4) {
					outByte[2] = ((byte[2] & 0x03) << 6) + byte[3];
					*outNumGood = 3;
				}
			}
		}
	}
	
	return curBuffer - inBuffer;
}


	PGPUInt32
CHTTPKeyServer::Base64Decode(
	const char *	inBuffer,
	PGPUInt32 		inBufSize,
	PGPByte *		inOutputBuffer)
{
	PGPByte *	p = inOutputBuffer;
	PGPUInt16	numGood;
	PGPUInt32	offset = 0;
	
	for(;;) {
		offset += DecodeBase64Morsel(	inBuffer + offset,
										inBufSize - offset,
										p,
										&numGood);
		p += numGood;
		if (numGood != 3) {
			break;
		}
	}
	
	if (numGood == 5) {
		ThrowPGPError_(kPGPError_CorruptData);
	} else {
		*p = 0;
	}
	
	return p - inOutputBuffer;
}



	PGPUInt32
CHTTPKeyServer::HexEncode(
	const PGPByte *	inBuffer,
	PGPUInt32		inBufSize,
	char *			inOutputBuffer)
{
	char *	p = inOutputBuffer;
	
	for (PGPUInt32 i = 0; i < inBufSize; i++) {
		*p++ = kHexString[inBuffer[i] >> 4];;
		*p++ = kHexString[inBuffer[i] & 0x0F];
	}
	
	*p = '\0';
	
	return (p - inOutputBuffer);
}



	void
CHTTPKeyServer::Send(
	const void *	inBuffer,
	PGPInt32		inBufferLength)
{
/*
FILE * temp = fopen("HTTPSend", "ab");
fwrite(inBuffer, 1, inBufferLength, temp);
fclose(temp);
*/

	if (PGPSend(mSocket, inBuffer, inBufferLength, kPGPSendFlagNone) != inBufferLength) {
		ThrowPGPError_(PGPGetLastSocketsError());
	}
}




	PGPUInt32
CHTTPKeyServer::ReceiveResult(
	PGPByte **	outBuffer)
{
	PGPUInt32		result = 0;
	StPGPDataRef	tempStorage;
	StPGPDataRef	buffer;
	PGPInt32		socketResult;
	PGPError		pgpErr;
	
	tempStorage = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
														kTempStorageSize,
														kPGPMemoryMgrFlags_None));
	if (tempStorage == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	buffer = static_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
												result + 1,
												kPGPMemoryMgrFlags_None));
	if (buffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	socketResult = PGPReceive(	mSocket,
								static_cast<PGPByte *>(tempStorage),
								kTempStorageSize,
								kPGPReceiveFlagNone);
	while (socketResult > 0) {
		pgpErr = PGPReallocData(	PGPGetContextMemoryMgr(mContext),
									(void **) &buffer,
									result + socketResult + 1,
									kPGPMemoryMgrFlags_None);
		ThrowIfPGPError_(pgpErr);
		pgpCopyMemory(	static_cast<PGPByte *>(tempStorage),
						static_cast<PGPByte *>(buffer) + result,
						socketResult);
		result += socketResult;
		socketResult = PGPReceive(	mSocket,
									static_cast<PGPByte *>(tempStorage),
									kTempStorageSize,
									kPGPReceiveFlagNone);
	}
	
	if (socketResult != 0) {
		ThrowPGPError_(PGPGetLastSocketsError());
	}
	
	*outBuffer = buffer;
	buffer = 0;
	(*outBuffer)[result] = 0;
	
	return result;
}



	PGPUInt32
CHTTPKeyServer::GetPost(
	PGPKeyServerState	inOperation,
	const char *		inPath,
	PGPByte** 			outResult,
	const char *		inContentType,
	PGPUInt32			inContentLength,
	const PGPByte *		inData,
	const char *		inExtraHeaders)
{
	StPreserveSocketsEventHandler	preserve(this);
	PGPUInt32						result;
	const char *					type = (inData == 0) ? kGet : kPost;
	PGPSocketAddressInternet		address;
	PGPInt32						socketResult;
	PGPError						pgpErr;
	
	pgpErr = pgpEventKeyServer(	mContext,
								mEventHandler,
								mEventHandlerData,
								this,
								kPGPKeyServerState_Opening);
	ThrowIfPGPError_(pgpErr);
	mSocket = PGPOpenSocket(	kPGPAddressFamilyInternet,
								kPGPSocketTypeStream,
								kPGPTCPProtocol);
	if (mSocket == kInvalidPGPSocketRef) {
		ThrowPGPError_(PGPGetLastSocketsError());
	}
	if (mCanceled) {
		ThrowPGPError_(kPGPError_UserAbort);
	}
	
	StCloseAndInvalidateSocket	theCloser(&mSocket);
	
	address.sin_family = kPGPAddressFamilyInternet;
	if (mProxyAddress == 0) {
		address.sin_port = PGPHostToNetShort(mHostPort);
		address.sin_addr.s_addr = PGPHostToNetLong(mHostAddress);
	} else {
		address.sin_port = PGPHostToNetShort(mProxyPort);
		address.sin_addr.s_addr = PGPHostToNetLong(mProxyAddress);
	}
	socketResult = PGPConnect(	mSocket,
								(PGPSocketAddress *) &address,
								sizeof(address));
	if (socketResult == kPGPSockets_Error) {
		ThrowPGPError_(PGPGetLastSocketsError());
	}
		
	// Secure connection for HTTPS
	if (mType == kPGPKeyServerProtocol_HTTPS) {
		if ((mTLSSession == kInvalidPGPtlsSessionRef)
		|| (PGPSocketsEstablishTLSSession(mSocket, mTLSSession) !=
		kPGPError_NoErr)) {
			pgpErr = pgpEventKeyServerTLS(	mContext,
											mEventHandler,
											mEventHandlerData,
											this,
											kPGPKeyServerState_TLSUnableToSecureConnection,
											mTLSSession);
		} else {
			pgpErr = pgpEventKeyServerTLS(	mContext,
											mEventHandler,
											mEventHandlerData,
											this,
											kPGPKeyServerState_TLSConnectionSecured,
											mTLSSession);
			mSecured = true;
		}
		ThrowIfPGPError_(pgpErr);
	}

	// Send data
	pgpErr = pgpEventKeyServer(	mContext,
								mEventHandler,
								mEventHandlerData,
								this,
								inOperation);
	ThrowIfPGPError_(pgpErr);
	
	Send(type, strlen(type));
	if (mProxyAddress != 0) {
		char	temp[256];
		
		Send(temp, sprintf(temp, "http://%s:%hu", mHostName, mHostPort));
	}
	Send(inPath, strlen(inPath));
	Send(kHTTPVersion, strlen(kHTTPVersion));
	if (inData != 0) {
		char		contentLength[20];
		
		Send(kContentLength, strlen(kContentLength));
		Send(contentLength, sprintf(contentLength, "%lu",
			static_cast<unsigned long>(inContentLength)));
		Send(kContentType, strlen(kContentType));
		Send(inContentType, strlen(inContentType));
	}
	if (inExtraHeaders != 0) {
		Send(inExtraHeaders, strlen(inExtraHeaders));
	}
	Send(kUserAgent, strlen(kUserAgent));
	if (inData != 0) {
		Send(inData, inContentLength);
	}

	// Receive results
	pgpErr = pgpEventKeyServer(	mContext,
								mEventHandler,
								mEventHandlerData,
								this,
								kPGPKeyServerState_ReceivingResults);
	ThrowIfPGPError_(pgpErr);
	result = ReceiveResult(outResult);
	
	// Close connection
	pgpErr = pgpEventKeyServer(	mContext,
								mEventHandler,
								mEventHandlerData,
								this,
								kPGPKeyServerState_Closing);
	ThrowIfPGPError_(pgpErr);
	if (mSecured) {
		PGPtlsClose(mTLSSession, false);
		mSecured = false;
	}
	// Socket gets closed using the closer above
	
	return result;
}
