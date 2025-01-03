/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CCRSEntrustServer.h,v 1.8 1999/03/25 18:31:02 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_CCRSEntrustServer_h
#define Included_CCRSEntrustServer_h

#include "CCRSServer.h"

class CCRSEntrustServer : public CCRSServer {
public:
					CCRSEntrustServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, const char * inPath,
						PGPKeyServerProtocol inProtocol);
	virtual			~CCRSEntrustServer();
	
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
	PGPUInt32		PostMessage(PGPKeyServerState inOperation, const PGPByte * inMessage,
						PGPSize inMessageLength, PGPByte ** outResult);

	void			PackageAndSendMessage(PGPExportFormat inExportFormat,
						PGPOutputFormat inOutputFormat, PGPKeyRef inKey, PGPKeyRef inSigningKey,
						PGPByte * inPassphrase, PGPSize inPassphraseLength,
						PGPBoolean inIsPassphrase, PGPByte * inDistributionPoint,
						PGPSize inDistributionPointSize, void ** outBuffer,
						PGPSize * outBufferSize);
};



#endif
