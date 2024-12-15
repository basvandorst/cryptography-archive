/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CCRSServer.h,v 1.6 1999/03/25 18:31:30 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_CCRSServer_h
#define Included_CCRSServer_h

#include "CHTTPKeyServer.h"

class CCRSServer : public CHTTPKeyServer {
public:
					CCRSServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, const char * inPath,
						PGPKeyServerProtocol inProtocol);
	virtual			~CCRSServer();
	
protected:
	void			CheckAndRemoveHTTPHeader(char * inBuffer, PGPSize inBufferSize,
						PGPByte ** outBuffer, PGPSize * outBufferSize);
};



#endif