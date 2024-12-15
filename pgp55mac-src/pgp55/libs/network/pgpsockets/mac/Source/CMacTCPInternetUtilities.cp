/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPInternetUtilities.cp,v 1.1.10.1 1997/12/07 04:26:42 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "CMacTCPInternetUtilities.h"



UInt32	CMacTCPInternetUtilities::sOutstandingCalls	=	0;



CMacTCPInternetUtilities::CMacTCPInternetUtilities()
{
	OSStatus	err;
	
	err = ::OpenResolver(nil);
	if (err != noErr) {
		throw(err);
	}
}



CMacTCPInternetUtilities::~CMacTCPInternetUtilities()
{
	// Delay until all outstanding DNR requests are complete
	while (sOutstandingCalls > 0) {
		CSocket::CallDefaultCallback();
	}
	::CloseResolver();
}



	PGPHostEntry *
CMacTCPInternetUtilities::GetHostByName(
	const char *	inName)
{
	PGPHostEntry *	result = nil;
	OSStatus		err;
	Boolean			done = false;
	struct hostInfo	hInfo;
	
	sOutstandingCalls++;
	err = ::StrToAddr(	(char *) inName,
						&hInfo,
						NewResultProc(ResultProc),
						(char *) &done);
	
	// Delay if we have to query the name server
	if (err == cacheFault) {
		while (! done) {
			CSocket::CallDefaultCallback();
		}
		err = hInfo.rtnCode;
	}
	sOutstandingCalls--;
	if (err != noErr) {
		throw(err);
	}

	// Insert the name
	strncpy(sHostEntry.h_name, hInfo.cname, kMaxHostNameLen);
	sHostEntry.h_name[kMaxHostNameLen] = 0;
	::BlockMoveData(	hInfo.addr,
						sHAddressesBuffer,
						sizeof(hInfo.addr));
	memset(sHAddressesListBuffer, 0, sizeof(sHAddressesListBuffer));
	for (UInt8 i = 0; (i < NUM_ALT_ADDRS) && (sHAddressesBuffer[i] != 0);
	 i++) {
		sHAddressesListBuffer[i] = &sHAddressesBuffer[i];
	}
	result = &sHostEntry;

	return result;
}



	pascal void
CMacTCPInternetUtilities::ResultProc(
	struct hostInfo *	hostInfoPtr,
	char *				userDataPtr)
{
	(*(Boolean *) userDataPtr) = true;
}
