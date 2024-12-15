/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COTTCPSocket.h,v 1.5.10.1 1997/12/07 04:26:49 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "COTInternetSocket.h"




class COTTCPSocket : public COTInternetSocket {
public:
							COTTCPSocket();
	virtual					~COTTCPSocket();
	
	virtual void			Connect(const PGPSocketAddressInternet *
									inAddress);
	virtual SInt16			Send(const void * inBuffer, SInt16 inLength,
								UInt16 inFlags);
	virtual SInt16			SendTo(const void * inBuffer, SInt16 inLength,
								const PGPSocketAddressInternet * inAddress);
	virtual SInt16			Receive(void * outBuffer, SInt16 inLength,
								UInt16 inFlags);
	virtual SInt16			ReceiveFrom(void * outBuffer, SInt16 inSize,
								PGPSocketAddressInternet * outAddress,
								SInt16 * ioAddressLength);
	virtual void			Listen(SInt16 inMaxBacklog);
	virtual CSocket *		Accept(PGPSocketAddressInternet * outAddress,
								SInt16 * ioAddressLength);
};

