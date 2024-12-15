/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: COTInternetUtilities.h,v 1.3.10.1 1998/11/12 03:22:32 heller Exp $____________________________________________________________________________*/#pragma once #include "CInternetUtilities.h"class COTInternetUtilities : public CInternetUtilities {public:							COTInternetUtilities();	virtual					~COTInternetUtilities();		virtual PGPHostEntry *	GetHostByName(const char * inName);	virtual	PGPHostEntry *	GetHostByAddress(PGPInternetAddress inAddress);	virtual void			GetHostName(char * outName,									SInt32 inNameLength);protected:	InetSvcRef				mInetSvcRef;	static pascal void		NotifyProc(void* contextPtr, OTEventCode code,									OTResult result, void* cookie);};