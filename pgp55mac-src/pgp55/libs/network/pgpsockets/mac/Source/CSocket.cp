/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSocket.cp,v 1.12.8.1 1997/12/07 04:26:52 mhw Exp $
____________________________________________________________________________*/

#include "CPGPSocketsArray.h"

#include "COTInternetUtilities.h"
#include "COTUDPSocket.h"
#include "COTTCPSocket.h"
#include "CMacTCPInternetUtilities.h"
#include "CMacTCPUDPSocket.h"
#include "CMacTCPTCPSocket.h"
#include "CSocket.h"

Boolean					CSocket::sInitialized = false;
Boolean					CSocket::sClosed = false;
CPGPSocketsArray		CSocket::sSocketsArray(sizeof(CSocket *));
PGPSocketsCallback		CSocket::sDefaultCallback = nil;
void *					CSocket::sDefaultCallbackData = nil;
CInternetUtilities *	CSocket::sInternetUtilities = nil;

	void
CSocket::Initialize()
{
	SInt32		theGestaltResult;

	if (! sInitialized) {
#if __MC68K__
		if (::Gestalt('mtcp', &theGestaltResult) != noErr) {
			throw((PGPSocketsError) kPGPSocketsErrorNetDown);
		}
		sInternetUtilities = new CMacTCPInternetUtilities;
		if (sInternetUtilities == nil) {
			throw(kPGPSocketsErrorFault);
		}
#else
		OSStatus	err;

		if (::Gestalt(gestaltOpenTpt, &theGestaltResult) != noErr) {
			throw((PGPSocketsError) kPGPSocketsErrorNetDown);
		}
		err = ::InitOpenTransport();
		
		if (err != kOTNoError) {
			throw(err);
		}
//		sInternetUtilities = new COTInternetUtilities;
		sInternetUtilities = new CMacTCPInternetUtilities;
		if (sInternetUtilities == nil) {
			throw(kPGPSocketsErrorFault);
		}
#endif
		sInitialized = true;
	}
}

	

	void
CSocket::CleanupSockets()
{
	if (! sClosed) {
		sClosed = true;
		
		delete sInternetUtilities;
	
#if __MC68K__
#else
		::CloseOpenTransport();
#endif
	}
}



	CSocket *
CSocket::CreateSocket(
	SInt32	inType)
{
	CSocket *	theSocket = (CSocket *) kPGPInvalidSocket;
	
	// Create a socket of the correct type
	switch (inType) {
		case kPGPSocketTypeStream:
		{
#if __MC68K__
			theSocket = new CMacTCPTCPSocket;
#else
			theSocket = new COTTCPSocket;
#endif
			if (theSocket == nil) {
				throw((PGPSocketsError) kPGPSocketsErrorMFile);
			}
			sSocketsArray.InsertItemsAt(	1,
											CPGPSocketsArray::index_Last,
											theSocket);
		}
		break;
		
		
		case kPGPSocketTypeDatagram:
		{
#if __MC68K__
			theSocket = new CMacTCPUDPSocket;
#else
			theSocket = new COTUDPSocket;
#endif
			if (theSocket == nil) {
				throw((PGPSocketsError) kPGPSocketsErrorMFile);
			}
			sSocketsArray.InsertItemsAt(	1,
											CPGPSocketsArray::index_Last,
											theSocket);
		}
		break;
		
		
		default:
		{
			throw((PGPSocketsError) kPGPSocketsErrorProtoNoSupport);
		}
		break;
	}
	
	return theSocket;
}



CSocket::CSocket()
	: mCallback(sDefaultCallback), mCallbackData(sDefaultCallbackData),
	  mIsReadable(false), mIsWriteable(false), mIsError(false)
{
}



CSocket::~CSocket()
{
	sSocketsArray.Remove(this);
}



	SInt16
CSocket::Select(
	PGPSocketSet *				ioReadSet,
	PGPSocketSet *				ioWriteSet,
	PGPSocketSet *				ioErrorSet,
	const PGPSocketsTimeValue *	inTimeout)
{
	UInt32				ticks;
	SInt16				result = 0;
	UInt16				i;
	PGPSocketSet *		readSet =
							(PGPSocketSet *) ::NewPtr(sizeof(PGPSocketSet));
	PGPSocketSet *		writeSet =
							(PGPSocketSet *) ::NewPtr(sizeof(PGPSocketSet));
	PGPSocketSet *		errorSet =
							(PGPSocketSet *) ::NewPtr(sizeof(PGPSocketSet));
	
	PGPSOCKETSET_ZERO(readSet);
	PGPSOCKETSET_ZERO(writeSet);
	PGPSOCKETSET_ZERO(errorSet);

	// Set the timeout
	if (inTimeout == NULL) {
		ticks = 0xFFFFFFFF;
	} else {
		ticks = ::TickCount() + (60 * inTimeout->tv_sec)
					+ ((1000 * inTimeout->tv_usec) / 60);
	}
	
	// Check the sets
	do {
		if (ioReadSet != NULL) {
			for (i = 0; i < ioReadSet->fd_count; i++) {
				if (((CSocket *) ioReadSet->fd_array[i])->IsReadable()) {
					PGPSOCKETSET_SET(ioReadSet->fd_array[i], readSet);
					result++;
				}
			}
		}
		if (ioWriteSet != NULL) {
			for (i = 0; i < ioWriteSet->fd_count; i++) {
				if (((CSocket *) ioWriteSet->fd_array[i])->IsWriteable()) {
					PGPSOCKETSET_SET(ioWriteSet->fd_array[i], writeSet);
					result++;
				}
			}
		}
		if (ioErrorSet != NULL) {
			for (i = 0; i < ioErrorSet->fd_count; i++) {
				if (((CSocket *) ioErrorSet->fd_array[i])->IsError()) {
					PGPSOCKETSET_SET(ioErrorSet->fd_array[i], errorSet);
					result++;
				}
			}
		}
		
		if (result > 0) {
			break;
		}
		
		// Give time to callback
		if (sDefaultCallback != nil) {
			sDefaultCallback(sDefaultCallbackData);
		}
	} while (::TickCount() < ticks);
	
	if (ioReadSet != NULL) {
		*ioReadSet = *readSet;
	}
	if (ioWriteSet != NULL) {
		*ioWriteSet = *writeSet;
	}
	if (ioErrorSet != NULL) {
		*ioErrorSet = *errorSet;
	}
	
	::DisposePtr((Ptr) readSet);
	::DisposePtr((Ptr) writeSet);
	::DisposePtr((Ptr) errorSet);
	
	return result;
}
	

	void
CSocket::SetSetInfo(
	TriStateBoolean	inIsReadable,
	TriStateBoolean	inIsWriteable,
	TriStateBoolean	inIsError)
{
	if (inIsReadable != triState_NoChange) {
		mIsReadable = inIsReadable;
	}
	if (inIsWriteable != triState_NoChange) {
		mIsWriteable = inIsWriteable;
	}
	if (inIsError != triState_NoChange) {
		mIsError = inIsError;
	}
}
