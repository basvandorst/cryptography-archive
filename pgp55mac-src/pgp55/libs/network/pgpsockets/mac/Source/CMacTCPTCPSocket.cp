/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPTCPSocket.cp,v 1.3.8.1 1997/12/07 04:26:43 mhw Exp $
____________________________________________________________________________*/


#include <string.h>

#include "pgpMem.h"

#include "CMacTCPTCPSocket.h"



const SInt32	kReceiveBufferSize	=	16*1024;



CMacTCPTCPSocket::CMacTCPTCPSocket(
	CMacTCPTCPSocket *	inParent)
		: mStream(nil), mReceiveBuffer(nil), mParent(inParent)/*,
		  mChildArray(sizeof(CMacTCPTCPSocket *)), mPassiveOpen(false),
		  mPassiveOpenError(noErr), mPassiveOpenHost(0), mPassiveOpenPort(0)*/
{
	TCPiopb	paramBlock;
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.csParam.create.rcvBuff =
		mReceiveBuffer = new char[kReceiveBufferSize];
	if (mReceiveBuffer == nil) {
		SetSetInfo(triState_NoChange, triState_NoChange, triState_True);
		throw(kPGPSocketsErrorNoBufs);
	}
	paramBlock.csParam.create.rcvBuffLen = kReceiveBufferSize;
	ProcessCommand(TCPCreate, (IPParamBlock *) &paramBlock);
	mStream = paramBlock.tcpStream;
}



CMacTCPTCPSocket::~CMacTCPTCPSocket()
{
	if (mStream != nil) {
		TCPiopb	paramBlock;
		
		if (! IsReadable()) {
			pgpClearMemory(&paramBlock, sizeof(paramBlock));
			paramBlock.ioCRefNum = sRefNum;
			paramBlock.tcpStream = mStream;
			paramBlock.csCode = TCPClose;
			::PBControlSync((ParmBlkPtr) &paramBlock);
		}
		pgpClearMemory(&paramBlock, sizeof(paramBlock));
		paramBlock.ioCRefNum = sRefNum;
		paramBlock.tcpStream = mStream;
		paramBlock.csCode = TCPRelease;
		::PBControlSync((ParmBlkPtr) &paramBlock);
	}
/*	
	LArrayIterator		iter(mChildArray);
	CMacTCPTCPSocket *	currenChild;
	
	while (iter.Next(&currentChild)) {
		currentChild->Close();
	}
*/	
	delete [] mReceiveBuffer;
}



	void
CMacTCPTCPSocket::Connect(
	const PGPSocketAddressInternet *	inAddress)
{
	TCPiopb	paramBlock;
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.tcpStream = mStream;
	paramBlock.csParam.open.remoteHost = inAddress->sin_addr.s_addr;
	paramBlock.csParam.open.remotePort = inAddress->sin_port;
	paramBlock.csParam.open.localPort = mBoundPort;
	ProcessCommand(TCPActiveOpen, (IPParamBlock *) &paramBlock);
	
	// Connection was accepted
	SetSetInfo(triState_NoChange, triState_True, triState_False);
}


	SInt16
CMacTCPTCPSocket::Send(
	const void *	inBuffer,
	SInt16			inLength,
	UInt16			inFlags)
{
	TCPiopb	paramBlock;
	wdsEntry		wds[2] = {	{inLength, (char *) inBuffer},
								{0, 0}};
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.tcpStream = mStream;
	paramBlock.csParam.send.wdsPtr = (Ptr) wds;
	if (inFlags & kPGPSendFlagOOB) {
		paramBlock.csParam.send.urgentFlag = 1;
	}
	ProcessCommand(TCPSend, (IPParamBlock *) &paramBlock);
	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);

	return inLength;
}



	SInt16
CMacTCPTCPSocket::SendTo(
	const void *						inBuffer,
	SInt16								inLength,
	const PGPSocketAddressInternet *	inAddress)
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
	
	return -1;
}



	SInt16
CMacTCPTCPSocket::Receive(
	void *	outBuffer,
	SInt16	inLength,
	UInt16	inFlags)
{
	SInt16	result = 0;
	TCPiopb	paramBlock;
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.tcpStream = mStream;
	paramBlock.csParam.receive.rcvBuff = (Ptr) outBuffer;
	paramBlock.csParam.receive.rcvBuffLen = inLength;
	try {
		ProcessCommand(TCPRcv, (IPParamBlock *) &paramBlock);
		result = paramBlock.csParam.receive.rcvBuffLen;
	}
	
	catch (PGPSocketsError exception) {
		if ((exception != connectionClosing)
		&& (exception != connectionTerminated)) {
			throw;
		}
	}
	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);

	return result;
}



	SInt16
CMacTCPTCPSocket::ReceiveFrom(
	void *						outBuffer,
	SInt16						inSize,
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	if (ioAddressLength != NULL) {
		*ioAddressLength = 0;
	}

	return Receive(outBuffer, inSize, kPGPReceiveFlagNone);	
}



	Boolean
CMacTCPTCPSocket::IsReadable()
{
	Boolean	result = false;
	TCPiopb	paramBlock;
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.ioCRefNum = sRefNum;
	paramBlock.tcpStream = mStream;
	paramBlock.csCode = TCPStatus;
	::PBControlSync((ParmBlkPtr) &paramBlock);

	if (paramBlock.csParam.status.amtUnreadData > 0) {
		SetSetInfo(triState_True, triState_NoChange, triState_False);
		result = true;
	}
	
	return result;
}



	void
CMacTCPTCPSocket::Listen(
	SInt16	inMaxBacklog)
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
}

	CSocket *
CMacTCPTCPSocket::Accept(
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	throw((PGPSocketsError) kPGPSocketsErrorOpNotSupp);
	return nil;
}
/*	This is a real pain in MacTCP. It is almost implemented, but returning
	control from currentChild->PassiveOpen() is a problem. It probably
	requires using a NotificationProc

	void
CMacTCPTCPSocket::Listen(
	SInt16	inMaxBacklog)
{
	CMacTCPTCPSocket *	currentChild;
	if (inMacBacklog < 1) {
		SetSetInfo(triState_NoChange, triState_NoChange, triState_True);
		throw(kPGPSocketsErrorBadQLen);
	}
	
	while (inMaxBacklog-- > 0) {
		currentChild = new MacTCPTCPSocket(this);
		if (currentChild == nil) [
			SetSetInfo(triState_NoChange, triState_NoChange, triState_True);
			throw(kPGPSocketsErrorNoBufs);
		}
		mChildArray.InsertItemsAt(	1,
									CPGPSocketsArray::index_Last,
									currentChild);
		currentChild->mBoundPort = mBoundPort;
		currentChild->mBoundAddress = mBoundAddress;
	}
	currentChild->PassiveOpen();
	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);
}



	CSocket *
CMacTCPTCPSocket::Accept(
	PGPSocketAddressInternet *	outAddress,
	SInt16 *					ioAddressLength)
{
	CMacTCPTCPSocket *	theNewSocket = nil;
	CMacTCPTCPSocket *	currentChild;
	
	while (theNewSocket == nil) {
		LArrayIterator	iter(mChildArray);
		
		while (iter.Next(&currentChild)) {
			if (currentChild->mPassiveOpen) {
				if (currentChild->mPassiveOpenError != noErr) {
					SetSetInfo(triState_NoChange, triState_NoChange,
						triState_True);
					throw(currentChild->mPassiveOpenError);
				}
				theNewSocket = currentChild;
				break;
			}
		}
		
		if ((theNewSocket == nil) && (mCallback != nil)) {
			mCallback(mCallbackData);
		}
	}
	
	// Fill in the parms
	*ioAddressLength = sizeof(PGPSocketAddressInternet);
	outAddress->sin_family = kPGPAddressFamilyInternet;
	outAddress->sin_port = theNewSocket->mPassiveOpenPort;
	outAddress->sin_addr.s_addr = theNewSocket->mPassiveOpenHost;
	
	theNewSocket->SetSetInfo(	triState_NoChange,
								triState_True,
								triState_NoChange);
		
	SetSetInfo(triState_NoChange, triState_NoChange, triState_False);
	return theNewSocket;
}
*/
