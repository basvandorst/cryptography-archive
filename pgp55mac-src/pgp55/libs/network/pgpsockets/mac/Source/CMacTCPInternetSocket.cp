/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPInternetSocket.cp,v 1.2.8.1 1997/12/07 04:26:41 mhw Exp $
____________________________________________________________________________*/

#include <stdarg.h>

#include "CPGPSocketsArray.h"

#include "CMacTCPInternetSocket.h"



Boolean	CMacTCPInternetSocket::sRefInitialized = false;
SInt16	CMacTCPInternetSocket::sRefNum;


CMacTCPInternetSocket::CMacTCPInternetSocket()
	: mBoundPort(0), mBoundAddress(0), mInCallback(false), mClosing(false)
{
	OSStatus	err;
	
	if (! sRefInitialized) {
		ParamBlockRec theParamBlock;
		
		theParamBlock.ioParam.ioNamePtr = "\p.IPP";
		theParamBlock.ioParam.ioPermssn = fsCurPerm;
		
		err = ::PBOpenSync(&theParamBlock);
		if (err != noErr) {
			throw(err);
		}
		sRefNum = theParamBlock.ioParam.ioRefNum;
		sRefInitialized = true;
	}
}



CMacTCPInternetSocket::~CMacTCPInternetSocket()
{
}



	void
CMacTCPInternetSocket::Close()
{
	if (mInCallback) {
		mClosing = true;
	} else {
		Cleanup();
		delete this;
	}
}



	void
CMacTCPInternetSocket::Bind(
	const PGPSocketAddressInternet *	inAddress)
{
	mBoundPort = inAddress->sin_port;
	mBoundAddress = inAddress->sin_addr.s_addr;
}



	void
CMacTCPInternetSocket::Cleanup()
{
}



	void
CMacTCPInternetSocket::ProcessCommand(
	SInt16			inCSCode,
	IPParamBlock *	inParamBlock)
{
	OSStatus	err;
	
	inParamBlock->csCode = inCSCode;
	inParamBlock->ioCRefNum = sRefNum;
	
	err = ::PBControlAsync((ParmBlkPtr) inParamBlock);
	if (err != noErr) {
		SetSetInfo(triState_NoChange, triState_NoChange, triState_True);
		throw(err);
	}
	mInCallback = true;
	while ((inParamBlock->ioResult == inProgress) && (!mClosing)) {
		if (mCallback != nil) {
			mCallback(mCallbackData);
		}
	}
	mInCallback = false;
	if (mClosing) {
		Close();
		throw(kPGPSocketsErrorUserCanceled);
	}
	if (inParamBlock->ioResult != noErr) {
		SetSetInfo(triState_NoChange, triState_NoChange, triState_True);
		throw(inParamBlock->ioResult);
	}
}
