/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CHTTPXcertServer.h,v 1.8 1999/03/25 18:33:57 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_CHTTPXcertServer_h
#define Included_CHTTPXcertServer_h

#include "CHTTPKeyServer.h"

class CHTTPXcertServer : public CHTTPKeyServer {
public:
					CHTTPXcertServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, const char * inPath,
						PGPKeyServerProtocol inProtocol);
	virtual			~CHTTPXcertServer();
	
	virtual void	SendCertificateRequest(PGPKeyRef inCAKey, PGPKeyRef inRequestKey,
						const void * inBuffer, PGPSize inBufferSize, void ** outBuffer,
						PGPSize * outBufferSize);
	virtual void	RetrieveCertificate(PGPFilterRef inSearchFilter, PGPKeyRef inSearchKey,
						PGPKeyRef inSigningKey, PGPByte * inPassphrase,
						PGPSize inPassphraseLength, PGPBoolean inIsPassphrase,
						void ** outBuffer, PGPSize * outBufferSize);
	virtual void	RetrieveCRL(PGPKeyRef inCAKey, PGPKeySetRef inCAsKeySet, PGPKeyRef inSigningKey,
						PGPByte * inPassphrase, PGPSize inPassphraseLength,
						PGPBoolean inIsPassphrase, void ** outBuffer, PGPSize * outBufferSize);
	
protected:
	PGPUInt32		DigestKey(PGPKeyRef inKey, char * inOutputBuffer);
	PGPUInt32		GetMaxDigestedKeyBufferSize() { return GetMaxHexEncodedBufferSize(16); }

	void			ProcessSendCertResult(const char * inResult, void ** outBuffer,
						PGPSize * outBufferSize);
	void			ProcessRetrieveCertResult(const char * inResult, void ** outBuffer,
						PGPSize * outBufferSize);
	void			ProcessRetrieveCRLResult(const char * inResult, void ** outBuffer,
						PGPSize * outBufferSize);
};



#endif