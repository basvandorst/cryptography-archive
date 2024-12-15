/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSocket.h,v 1.12.10.1 1997/12/07 04:26:53 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <OpenTransport.h>
#include <OpenTptInternet.h>

#define PGP_MACINTOSH 1

#include "PGPSockets.h"
//#include "CInternetUtilities.h"
#include "CPGPSocketsArray.h"

class CInternetUtilities;

enum TriStateBoolean {
	triState_False		=	0,
	triState_True		=	1,
	triState_NoChange	=	2
};

class CSocket {
public:
	static void					Initialize();
	static void					SetDefaultCallback(
									PGPSocketsCallback inCallback,
									void * inData)
									{ sDefaultCallback = inCallback;
										sDefaultCallbackData = inData; }
	static void					CallDefaultCallback()
									{ if (sDefaultCallback != nil) 
									sDefaultCallback(sDefaultCallbackData); }
	static CSocket *			CreateSocket(SInt32 inType);
	static Boolean				VerifyPGPSocketRef(CSocket * inSocketRef)
									{ return (sSocketsArray.FetchIndexOf(
									inSocketRef)
									!= CPGPSocketsArray::index_Bad); }
	static void					CleanupSockets();

	static SInt16				Select(PGPSocketSet * ioReadSet,
									PGPSocketSet * ioWriteSet,
									PGPSocketSet * ioErrorSet,
									const PGPSocketsTimeValue * inTimeout);
										
								CSocket();
	virtual						~CSocket();
	
	virtual void				SetCallback(PGPSocketsCallback inCallback,
									void * inData)
									{ mCallback = inCallback;
									mCallbackData = inData; }
	
	virtual void				Bind(const PGPSocketAddressInternet *
										inAddress) = 0;
	virtual void				Connect(const PGPSocketAddressInternet *
										inAddress) = 0;
	virtual SInt16				Send(const void * inBuffer, SInt16 inLength,
									UInt16 inFlags) = 0;
	virtual SInt16				SendTo(const void * inBuffer, SInt16 inLength,
									const PGPSocketAddressInternet *
											inAddress) = 0;
	virtual SInt16				Receive(void * outBuffer, SInt16 inLength,
									UInt16 inFlags) = 0;
	virtual SInt16				ReceiveFrom(void * outBuffer, SInt16 inSize,
									PGPSocketAddressInternet * outAddress,
									SInt16 * ioAddressLength) = 0;
	virtual void				Listen(SInt16 inMaxBacklog) = 0;
	virtual CSocket *			Accept(PGPSocketAddressInternet * outAddress,
									SInt16 * ioAddressLength) = 0;
	virtual void				Close() = 0;
	
	static CInternetUtilities *	sInternetUtilities;

protected:
	static Boolean				sInitialized;
	static Boolean				sClosed;
	static CPGPSocketsArray		sSocketsArray;
	static PGPSocketsCallback	sDefaultCallback;
	static void *				sDefaultCallbackData;
	
	PGPSocketsCallback			mCallback;
	void *						mCallbackData;
	Boolean						mIsReadable;
	Boolean						mIsWriteable;
	Boolean						mIsError;
	
	virtual Boolean				IsReadable() { return mIsReadable; }
	virtual Boolean				IsWriteable() { return mIsWriteable; }
	virtual Boolean				IsError() { return mIsError; }
	
	void						SetSetInfo(TriStateBoolean inIsReadable,
									TriStateBoolean inIsWriteable,
									TriStateBoolean inIsError);
};
