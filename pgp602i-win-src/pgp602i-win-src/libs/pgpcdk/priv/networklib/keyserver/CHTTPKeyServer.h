/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: CHTTPKeyServer.h,v 1.3.10.1 1998/11/12 03:22:15 heller Exp $
____________________________________________________________________________*/

#ifndef Included_CHTTPKeyServer_h
#define Included_CHTTPKeyServer_h

#include "pgpSockets.h"

#include "CKeyServer.h"



class CHTTPKeyServer : public CKeyServer {
public:
					CHTTPKeyServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, PGPKeyServerType inType);
	virtual			~CHTTPKeyServer();
	
	virtual void	Cancel();
	
	virtual void	Query(PGPFilterRef inFilterRef,
						PGPKeySetRef * outFoundKeys);
	virtual void	Upload(PGPKeyUploadPreference inSendPrivateKeys,
						PGPKeySetRef inKeysToUpload,
						PGPKeySetRef * outKeysThatFailed);

protected:
	PGPSocketRef	mSocket;
	
	PGPUInt32		ReceiveResult(char ** outBuffer);
	PGPUInt32		URLEncode(char * inBuffer, PGPUInt32 inBufSize,
						char * inURLEncodedBuffer);
	PGPError		ProcessHTTPResult(char * inResult);
};



#endif