/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CCRSServer.cpp,v 1.10 1999/04/01 02:07:40 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpMem.h"
#include "pgpUtilities.h"

#include "CCRSServer.h"



CCRSServer::CCRSServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	const char *			inPath,
	PGPKeyServerProtocol	inProtocol)
		: CHTTPKeyServer(inContext, inHostName, inHostAddress, inHostPort, inPath, inProtocol)
{
}



CCRSServer::~CCRSServer()
{
}



	void
CCRSServer::CheckAndRemoveHTTPHeader(
	char *		inBuffer,
	PGPSize		inBufferSize,
	PGPByte **	outBuffer,
	PGPSize *	outBufferSize)
{
	char *	temp = inBuffer;
	
	while ((*temp != '\t') && (*temp != ' ') && ((PGPSize) (temp - inBuffer) < inBufferSize)) {
		temp++;
	}
	if (atoi(temp) != 200) {
		ThrowPGPError_(kPGPError_ServerRequestFailed);
	}
	temp = strstr(temp, "\r\n\r\n");
	temp += 4;
	*outBufferSize = inBufferSize - (temp - inBuffer);
	*outBuffer = reinterpret_cast<PGPByte *>(PGPNewData(	PGPGetContextMemoryMgr(mContext),
															*outBufferSize,
															kPGPMemoryMgrFlags_None));
	if (*outBuffer == 0) {
		ThrowPGPError_(kPGPError_OutOfMemory);
	}
	pgpCopyMemory(temp, *outBuffer, *outBufferSize);
}