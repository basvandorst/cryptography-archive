/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CHTTPPGPKeyServer.h,v 1.3 1999/03/25 18:33:17 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_CHTTPPGPKeyServer_h
#define Included_CHTTPPGPKeyServer_h

#include "CHTTPKeyServer.h"



class CHTTPPGPKeyServer : public CHTTPKeyServer {
public:
					CHTTPPGPKeyServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, PGPKeyServerProtocol inType);
	virtual			~CHTTPPGPKeyServer();
	
	virtual void	Query(PGPFilterRef inFilterRef,
						PGPKeySetRef * outFoundKeys);
	virtual void	Upload(PGPKeyUploadPreference inSendPrivateKeys,
						PGPKeySetRef inKeysToUpload,
						PGPKeySetRef * outKeysThatFailed);

protected:
	PGPError		ProcessHTTPResult(const char * inResult);
};



#endif
