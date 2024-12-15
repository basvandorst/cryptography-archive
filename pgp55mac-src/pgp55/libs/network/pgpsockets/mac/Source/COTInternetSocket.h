/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTInternetSocket.h,v 1.2.10.1 1997/12/07 04:26:46 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CInternetSocket.h"


class COTInternetSocket : public CInternetSocket {
public:
							COTInternetSocket();
	virtual 				~COTInternetSocket();

	virtual void			Bind(const PGPSocketAddressInternet *
									inAddress);
	virtual void			Close();
	
protected:
	EndpointRef				mEndpointRef;
	InetAddress				mBoundAddress;
	Boolean					mInCallback;
	Boolean					mAsyncCancel;
	
	virtual void			Cleanup();
	virtual void			DoBind(OTQLen inQLen);
	
	virtual Boolean			IsReadable();

	virtual void			ThrowIfOTError(OSStatus inErr);
	
	static pascal void		NotifyProc(void* contextPtr, OTEventCode code, 
									   OTResult result, void* cookie);
};
