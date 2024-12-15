/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPUDPSocket.h,v 1.1.10.1 1997/12/07 04:26:45 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CMacTCPInternetSocket.h"

class CMacTCPUDPSocket : public CMacTCPInternetSocket {
public:
								CMacTCPUDPSocket();
	virtual						~CMacTCPUDPSocket();
	
	virtual void				Bind(const PGPSocketAddressInternet *
										inAddress);
	virtual void				Connect(const PGPSocketAddressInternet *
										inAddress);
	virtual SInt16				Send(const void * inBuffer, SInt16 inLength,
									UInt16 inFlags);
	virtual SInt16				SendTo(const void * inBuffer, SInt16 inLength,
									const PGPSocketAddressInternet *
										inAddress);
	virtual SInt16				Receive(void * outBuffer, SInt16 inLength,
									UInt16 inFlags);
	virtual SInt16				ReceiveFrom(void * outBuffer, SInt16 inSize,
									PGPSocketAddressInternet * outAddress,
								SInt16 * ioAddressLength);
	virtual void				Listen(SInt16 inMaxBacklog);
	virtual CSocket *			Accept(PGPSocketAddressInternet * outAddress,
									SInt16 * ioAddressLength);
	
protected:
	Boolean						mConnected;
	PGPSocketAddressInternet	mConnectAddress;
	Boolean						mBound;
	Boolean						mCreated;
	SInt32						mStream;
	char *						mReceiveBuffer;
	Boolean						mSending;
	Handle						mExcessReceiveHandle;
	
	
	virtual Boolean				IsReadable();

	virtual void				CreateUDPStream(SInt16	inPort);
};
