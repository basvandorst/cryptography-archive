/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CHTTPKeyServer.h,v 1.13.6.1 1999/06/04 00:28:49 heller Exp $
____________________________________________________________________________*/

#ifndef Included_CHTTPKeyServer_h
#define Included_CHTTPKeyServer_h

#include "pgpSockets.h"

#include "CKeyServer.h"



class CHTTPKeyServer : public CKeyServer {
public:
					CHTTPKeyServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, const char * inPath,
						PGPKeyServerProtocol inType);
	virtual			~CHTTPKeyServer();
	
	virtual void	Open(PGPtlsSessionRef inTLSSession);
	virtual void	Cancel();
	
protected:
	PGPSocketRef	mSocket;
	char *			mProxyName;
	PGPUInt32		mProxyAddress;
	PGPUInt16		mProxyPort;
	
	PGPUInt32		GetPost(PGPKeyServerState inOperation, const char * inPath,
						PGPByte** outResult, const char * inContentType = 0,
						PGPUInt32 inContentLength = 0, const PGPByte * inData = 0,
						const char * inExtraHeaders = 0);
	PGPUInt32		URLEncode(const char * inBuffer, PGPUInt32 inBufSize,
						char * inOutputBuffer);
	PGPUInt32		GetMaxURLEncodedBufferSize(PGPUInt32 inInputBufSize)
						{ return (inInputBufSize * 6) + 1; }
	PGPUInt32		Base64Encode(const PGPByte * inBuffer, PGPUInt32 inBufSize,
						char * inOutputBuffer, PGPBoolean inLineBreak = true);
	PGPUInt32		GetMaxBase64EncodedBufferSize(PGPUInt32 inInputBufSize)
						{ PGPUInt32 result = ((inInputBufSize / 3) * 4) + 5;
						  result += ((result / 64) * 2) + 2; return result; }
	PGPUInt32		Base64Decode(const char * inBuffer, PGPUInt32 inBufSize,
						PGPByte * inOuputBuffer);
	PGPUInt32		GetMaxBase64DecodedBufferSize(PGPUInt32 inInputBufSize)
						{ return ((inInputBufSize / 4) * 3) + 1; }
	PGPUInt32		HexEncode(const PGPByte * inBuffer, PGPUInt32 inBufSize,
						char * inOutputBuffer);
	PGPUInt32		GetMaxHexEncodedBufferSize(PGPUInt32 inInputBufSize)
						{ return (inInputBufSize * 2) + 1; }
	void			Send(const void * inBuffer, PGPInt32 inBufferLength);
	PGPUInt32		ReceiveResult(PGPByte ** outBuffer);
	
private:
	PGPUInt32		DecodeBase64Morsel(const char * inBuffer, PGPUInt32 inBufSize,
						PGPByte outByte[3], PGPUInt16 * outNumGood);
};



#endif