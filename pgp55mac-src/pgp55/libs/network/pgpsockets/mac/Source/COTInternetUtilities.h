/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTInternetUtilities.h,v 1.1.10.1 1997/12/07 04:26:47 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CInternetUtilities.h"

class COTInternetUtilities : public CInternetUtilities {
public:
							COTInternetUtilities();
	virtual					~COTInternetUtilities();
	
	virtual PGPHostEntry *	GetHostByName(const char * inName);

protected:
	InetSvcRef				mInetSvcRef;

	static pascal void		NotifyProc(void* contextPtr, OTEventCode code,
									OTResult result, void* cookie);
};
