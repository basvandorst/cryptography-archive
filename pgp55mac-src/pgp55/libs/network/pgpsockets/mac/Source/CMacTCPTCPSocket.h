/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPTCPSocket.h,v 1.1.10.1 1997/12/07 04:26:44 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CMacTCPInternetSocket.h"




class CMacTCPTCPSocket : public CMacTCPInternetSocket {
public:
							CMacTCPTCPSocket(CMacTCPTCPSocket * inParent = nil);
	virtual					~CMacTCPTCPSocket();
	
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
								
protected:
//	Boolean					mPassiveOpen;
//	PGPSocketsError			mPassiveOpenError;
//	SInt32					mPassiveOpenHost;
//	SInt16					mPassiveOpenPort;
	StreamPtr				mStream;
	CMacTCPTCPSocket *		mParent;
//	CPGPSocketsArray		mChildArray;
	char *					mReceiveBuffer;

	virtual Boolean			IsReadable();
};

