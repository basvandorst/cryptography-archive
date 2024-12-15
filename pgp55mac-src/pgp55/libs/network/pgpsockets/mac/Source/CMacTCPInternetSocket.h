/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPInternetSocket.h,v 1.1.10.1 1997/12/07 04:26:41 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <MacTCP.h>

#include "CInternetSocket.h"


class CMacTCPInternetSocket : public CInternetSocket {
public:
							CMacTCPInternetSocket();
	virtual 				~CMacTCPInternetSocket();

	virtual void			Bind(const PGPSocketAddressInternet *
									inAddress);
	virtual void			Close();
	
protected:
	static Boolean			sRefInitialized;
	static SInt16			sRefNum;
	
	SInt16					mBoundPort;
	SInt32					mBoundAddress;
	Boolean					mInCallback;
	Boolean					mClosing;
	
	
	virtual void			Cleanup();
	
	virtual void			ProcessCommand(SInt16 inCSCode,
								IPParamBlock * inParamBlock);
};
