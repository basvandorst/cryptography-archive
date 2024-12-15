/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CCRSVerisignServer.h,v 1.9 1999/03/25 18:32:04 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_CCRSVerisignServer_h
#define Included_CCRSVerisignServer_h

#include "CCRSServer.h"

class CCRSVerisignServer : public CCRSServer {
public:
					CCRSVerisignServer(PGPContextRef inContext,
						const char * inHostName, PGPUInt32 inHostAddress,
						PGPUInt16 inHostPort, const char * inPath,
						PGPKeyServerProtocol inProtocol);
	virtual			~CCRSVerisignServer();
	
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
	PGPUInt32		PostCRSMessage(PGPKeyServerState inOperation, const char * inPath,
						PGPByte** outResult, PGPUInt32 inContentLength,
						const PGPByte * inData);

	void			PackageAndSendCRSMessage(PGPExportFormat inExportFormat,
						PGPOutputFormat inOutputFormat, PGPKeyRef inKey, PGPKeyRef inSigningKey,
						PGPByte * inPassphrase, PGPSize inPassphraseLength,
						PGPBoolean inIsPassphrase, void ** outBuffer, PGPSize * outBufferSize);
};



#endif