/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CMacTCPUDPSocket.h,v 1.6.10.1 1998/11/12 03:22:29 heller Exp $____________________________________________________________________________*/#pragma once #include "CMacTCPInternetSocket.h"class CMacTCPUDPSocket : public CMacTCPInternetSocket {public:								CMacTCPUDPSocket();	virtual						~CMacTCPUDPSocket();		virtual void				Connect(const PGPSocketAddressInternet *										inAddress);	virtual SInt32				Send(const void * inBuffer, SInt32 inLength,									SInt32 inFlags);	virtual SInt32				SendTo(const void * inBuffer, SInt32 inLength,									const PGPSocketAddressInternet *										inAddress);	virtual SInt32				Receive(void * outBuffer, SInt32 inLength,									SInt32 inFlags);	virtual SInt32				ReceiveFrom(void * outBuffer, SInt32 inSize,									PGPSocketAddressInternet * outAddress,									SInt32 * ioAddressLength);	virtual void				Listen(SInt32 inMaxBacklog);	virtual CSocket *			Accept(PGPSocketAddressInternet * outAddress,									SInt32 * ioAddressLength);		virtual void				IOControlSocket(SInt32 inCommand,									UInt32 * ioParam)									{ 	(void) inCommand; (void) ioParam;										ThrowPGPError_(									kPGPError_SocketsOperationNotSupported); }	virtual void				GetPeerName(PGPSocketAddressInternet * outName);protected:	typedef CMacTCPInternetSocket		inherited;		PGPSocketAddressInternet	mConnectAddress;	Boolean						mCreated;	SInt32						mStream;	char *						mReceiveBuffer;	Boolean						mSending;	Handle						mExcessReceiveHandle;	Boolean						mDataAvailable;	UDPNotifyUPP				mNotifyProcUPP;			virtual Boolean				IsReadable();	virtual Boolean				IsWriteable() { return mBound; }	virtual Boolean				IsError() { return false; }	virtual void				CreateUDPStream(SInt16	inPort);		static pascal void			UDPNotifyProc(StreamPtr inUDPStream,									UInt16 inEventCode, Ptr inUserData,									struct ICMPReport * inICMPMsg);};