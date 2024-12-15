/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTInternetUtilities.cp,v 1.1.10.1 1997/12/07 04:26:47 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "COTInternetUtilities.h"



COTInternetUtilities::COTInternetUtilities()
{
	OSStatus	err;

	mInetSvcRef = ::OTOpenInternetServices(	kDefaultInternetServicesPath,
											0,
											&err);
	if (err != kOTNoError) {
		throw(err);
	}
	err = ::OTInstallNotifier(	mInetSvcRef,
								NotifyProc,
								nil);
	if (err != kOTNoError) {
		throw(err);
	}
	err = ::OTUseSyncIdleEvents(	mInetSvcRef,
									true);
	if (err != kOTNoError) {
		throw(err);
	}
}



COTInternetUtilities::~COTInternetUtilities()
{
	::OTCloseProvider(mInetSvcRef);
}


	PGPHostEntry *
COTInternetUtilities::GetHostByName(
	const char * inName)
{
	InetHostInfo	hInfo;
	PGPHostEntry *	result = NULL;
	OSStatus		err;
	
	// Get the info
	err = ::OTInetStringToAddress(	mInetSvcRef,
									(char *) inName,
									&hInfo);
	if (err != kOTNoError) {
		throw(err);
	}
										
	// Insert the name
	strncpy(sHostEntry.h_name, hInfo.name, kMaxHostNameLen);
	sHostEntry.h_name[kMaxHostNameLen] = 0;
	::BlockMoveData(	hInfo.addrs,
						sHAddressesBuffer,
						sizeof(hInfo.addrs));
	memset(sHAddressesListBuffer, 0, sizeof(sHAddressesListBuffer));
	for (UInt8 i = 0; sHAddressesBuffer[i] != 0; i++) {
		sHAddressesListBuffer[i] = &sHAddressesBuffer[i];
	}
	result = &sHostEntry;

	return result;
}



	pascal void
COTInternetUtilities::NotifyProc(
	void *		contextPtr,
	OTEventCode	code, 
	OTResult	result,
	void *		cookie)
{
	CSocket::CallDefaultCallback();
}
