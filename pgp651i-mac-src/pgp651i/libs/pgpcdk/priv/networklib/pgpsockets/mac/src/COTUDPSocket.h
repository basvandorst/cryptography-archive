/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: COTUDPSocket.h,v 1.5 1999/03/10 02:35:27 heller Exp $____________________________________________________________________________*/#pragma once #include "COTInternetSocket.h"class COTUDPSocket : public COTInternetSocket {protected:	typedef COTInternetSocket	inherited;	public:								COTUDPSocket();	virtual						~COTUDPSocket();		virtual void				Bind(const PGPSocketAddressInternet *										inAddress);	virtual void				Connect(const PGPSocketAddressInternet *										inAddress);	virtual SInt32				Send(const void * inBuffer, SInt32 inLength,									SInt32 inFlags);	virtual SInt32				SendTo(const void * inBuffer, SInt32 inLength,									const PGPSocketAddressInternet *										inAddress);	virtual SInt32				Receive(void * outBuffer, SInt32 inLength,									SInt32 inFlags);	virtual SInt32				ReceiveFrom(void * outBuffer, SInt32 inSize,									PGPSocketAddressInternet * outAddress,								SInt32 * ioAddressLength);	virtual void				Listen(SInt32 inMaxBacklog);	virtual CSocket *			Accept(PGPSocketAddressInternet * outAddress,									SInt32 * ioAddressLength);		virtual void				GetPeerName(PGPSocketAddressInternet * outName);protected:	Boolean						mConnected;	PGPSocketAddressInternet	mConnectAddress;	Boolean						mBound;};