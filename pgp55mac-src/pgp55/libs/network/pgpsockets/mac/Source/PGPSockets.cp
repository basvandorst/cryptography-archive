/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPSockets.cp,v 1.16.10.1 1997/12/07 04:26:53 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "CSocket.h"
#include "CInternetUtilities.h"

#define PGP_MACINTOSH 1

#include "PGPSockets.h"


PGPSocketAddressInternet	kPGPAddressAny = {	kPGPAddressFamilyInternet,
												0,
												{kPGPInternetAddressAny},
												{0,0,0,0,0,0,0,0}};

// Statics
static PGPSocketsError	sLastError = noErr;
static PGPSocketsError	sLastHostError = noErr;


PGPSocketsError	MapPGPSocketsError(PGPSocketsError inErr);



	short
PGPSocketsInit()
{
	short			result = 0;
	PGPSocketsError	err = noErr;

	try {
//		CSocket::Initialize();
		result = 1;
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;
	
	return result;
}



	void
PGPSocketsCleanup()
{
	try {
		CSocket::CleanupSockets();
	}

	catch (...) {
	}
}



	PGPSocketRef
PGPOpenSocket(
	short	inAddressFamily,
	short	inSocketType,
	short	inSocketProtocol)
{
	PGPSocketRef	theSocket = kPGPInvalidSocket;
	PGPSocketsError	err = noErr;
	
	try {
		CSocket::Initialize();

		// Switch on the family. Currently we only support the AF_INET family
		switch (inAddressFamily) {
			case kPGPAddressFamilyInternet:
			{
				// Switch on the type
				switch (inSocketType) {
					case kPGPSocketTypeStream:
					case kPGPSocketTypeDatagram:
					{
						// Switch on the protocol
						switch (inSocketProtocol) {
							case kPGPIPProtocol:
							case kPGPTCPProtocol:
							case kPGPUDPProtocol:
							{
								theSocket = CSocket::CreateSocket(
													inSocketType);
							}
							break;
							
							
							default:
							{
								throw((PGPSocketsError)
										kPGPSocketsErrorProtoNoSupport);
							}
							break;
						}
					}
					break;
					
					
					default:
					{
						throw((PGPSocketsError)
								kPGPSocketsErrorProtoNoSupport);
					}
					break;
				}
			}
			break;
			
			
			default:
			{
				throw((PGPSocketsError)
						kPGPSocketsErrorAFNoSupport);
			}
			break;
		}
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;
	
	return theSocket;
}




	short
PGPCloseSocket(
	PGPSocketRef	inSocketRef)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		// Everything is good, so close the object
		((CSocket *) inSocketRef)->Close();
		result = 0;
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	short
PGPBindSocket(
	PGPSocketRef				inSocketRef,
	const PGPSocketAddress *	inAddress,
	short						inAddressLength)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		if (inAddress == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		if ((inAddressLength != sizeof(PGPSocketAddressInternet))
		|| (inAddress->sa_family != kPGPAddressFamilyInternet)) {
			throw((PGPSocketsError) kPGPSocketsErrorAFNoSupport);
		}
		
		// Everything is good, so bind the object
		((CSocket *) inSocketRef)->
				Bind((const PGPSocketAddressInternet *) inAddress);
		result = 0;
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;
	
	return result;
}



	short
PGPConnect(
	PGPSocketRef				inSocketRef,
	const PGPSocketAddress *	inServerAddress,
	short						inAddressLength)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		if (inServerAddress == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		if ((inAddressLength != sizeof(PGPSocketAddressInternet))
		|| (inServerAddress->sa_family != kPGPAddressFamilyInternet)) {
			throw((PGPSocketsError) kPGPSocketsErrorAFNoSupport);
		}
		
		// Everything is good, so bind the object
		((CSocket *) inSocketRef)->
				Connect((const PGPSocketAddressInternet *) inServerAddress);
		result = 0;
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;
	
	return result;
}



	short
PGPSend(
	PGPSocketRef	inSocketRef,
	const void *	inBuffer,
	short			inBufferLength,
	ushort			inFlags)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		if (inBuffer == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		
		if ((inBufferLength < 0) ||
		(inFlags > (kPGPSendFlagDontRoute | kPGPSendFlagOOB))) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		// Everything is good, so send through the object
		result = ((CSocket *) inSocketRef)->Send(	inBuffer,
													inBufferLength,
													inFlags);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;
	
	return result;
}



	size_t
PGPWrite(
	PGPSocketRef	inSocketRef,
	const void *	inBuffer,
	size_t			inBufferLength)
{
	return PGPSend(inSocketRef, inBuffer, inBufferLength, kPGPSendFlagNone);
}



	short
PGPGatherWrite(
	PGPSocketRef		inSocketRef,
	const PGPIOVector *	inVectors,
	size_t				inCount)
{
	SInt16	total = 0;
	SInt16	result;
	
	for (size_t x = 0; x < inCount; x++) {
		result = PGPWrite(	inSocketRef,
							inVectors[x].iovbase,
							inVectors[x].iov_len);
		if (result > 0) {
			total += result;
		} else if (result == -1) {
			total = -1;
			break;
		}
		if (result != inVectors[x].iov_len) {
			break;
		}
	}
	
	return total;
}



	short
PGPSendTo(
	PGPSocketRef		inSocketRef,
	const void *		inBuffer,
	short				inBufferLength,
	ushort				inFlags,
	PGPSocketAddress *	inAddress,
	short				inAddressLength)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		if (inBuffer == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		
		if ((inBufferLength < 0) || (inFlags != kPGPSendFlagNone)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		if (inAddress == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		if ((inAddressLength != sizeof(PGPSocketAddressInternet))
		|| (inAddress->sa_family != kPGPAddressFamilyInternet)) {
			throw((PGPSocketsError) kPGPSocketsErrorAFNoSupport);
		}
		
		// Everything is good, so send through the object
		result = ((CSocket *) inSocketRef)->SendTo(inBuffer, inBufferLength,
					(const PGPSocketAddressInternet *) inAddress);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	short
PGPReceive(
	PGPSocketRef	inSocketRef,
	void *			outBuffer,
	short			inBufferSize,
	ushort			inFlags)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		if (outBuffer == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		
		if ((inBufferSize < 0) || (inFlags != kPGPReceiveFlagNone)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		// Everything is good, so send through the object
		result = ((CSocket *) inSocketRef)->
						Receive(outBuffer, inBufferSize, inFlags);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	short
PGPRead(
	PGPSocketRef	inSocketRef,
	void *			outBuffer,
	size_t			inBufferSize)
{
	return PGPReceive(	inSocketRef,
						outBuffer,
						inBufferSize,
						kPGPReceiveFlagNone);
}



	short
PGPScatterRead(
	PGPSocketRef		inSocketRef,
	const PGPIOVector *	inVectors,
	size_t				inCount)
{
	SInt16	total = 0;
	SInt16	result;
	
	for (size_t x = 0; x < inCount; x++) {
		result = PGPRead(	inSocketRef,
							inVectors[x].iovbase,
							inVectors[x].iov_len);
		if (result > 0) {
			total += result;
		} else if (result == -1) {
			total = -1;
			break;
		}
		if (result != inVectors[x].iov_len) {
			break;
		}
	}
	
	return total;
}



	short
PGPReceiveFrom(
	PGPSocketRef		inSocketRef,
	void *				outBuffer,
	short				inBufferSize,
	ushort				inFlags,
	PGPSocketAddress *	outAddress,
	short *				ioAddressLength)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		if (outBuffer == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		
		if ((inBufferSize < 0) || (inFlags != kPGPReceiveFlagNone)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		if ((outAddress != NULL)
		&& (*ioAddressLength < sizeof(PGPSocketAddressInternet))) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		
		// Everything is good, so send through the object
		result = ((CSocket *) inSocketRef)->
						ReceiveFrom(outBuffer,
									inBufferSize,
									(PGPSocketAddressInternet *) outAddress,
									ioAddressLength);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	short
PGPListen(
	PGPSocketRef	inSocketRef,
	short			inMaxBacklog)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		if (inMaxBacklog < 0) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		// Everything is good, so send through the object
		((CSocket *) inSocketRef)->Listen(inMaxBacklog);
		result = 0;
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	PGPSocketRef
PGPAccept(
	PGPSocketRef		inSocketRef,
	PGPSocketAddress *	outAddress,
	short *				ioAddressLength)
{
	PGPSocketRef	result = kPGPInvalidSocket;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		if (outAddress == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorFault);
		}
		if (*ioAddressLength < sizeof(PGPSocketAddressInternet)) {
			throw((PGPSocketsError) kPGPSocketsErrorAFNoSupport);
		}
		
		// Everything is good, so accept a connection
		result = ((CSocket *) inSocketRef)->
						Accept(	(PGPSocketAddressInternet *) outAddress,
								ioAddressLength);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	short
PGPSelect(
	short						inNumSetCount,
	PGPSocketSet *				ioReadSet,
	PGPSocketSet *				ioWriteSet,
	PGPSocketSet *				ioErrorSet,
	const PGPSocketsTimeValue *	inTimeout)
{
	short			result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		SInt16	i;
	
		// Make sure arguments are valid
		if ((ioReadSet == NULL) && (ioWriteSet == NULL)
		&& (ioErrorSet == NULL)) {
			throw((PGPSocketsError) kPGPSocketsErrorInval);
		}
		if (ioReadSet != NULL) {
			if (ioReadSet->fd_count > PGPSOCKETSET_SETSIZE) {
				throw((PGPSocketsError) kPGPSocketsErrorNotSock);
			}			
			for (i = 0; i < ioReadSet->fd_count; i++) {
				if (! CSocket::VerifyPGPSocketRef((CSocket *) ioReadSet->
				fd_array[i])) {
					throw((PGPSocketsError) kPGPSocketsErrorBadF);
				}
			}
		}
		if (ioWriteSet != NULL) {
			if (ioWriteSet->fd_count > PGPSOCKETSET_SETSIZE) {
				throw((PGPSocketsError) kPGPSocketsErrorNotSock);
			}
			for (i = 0; i < ioWriteSet->fd_count; i++) {
				if (! CSocket::VerifyPGPSocketRef((CSocket *) ioWriteSet->
				fd_array[i])) {
					throw((PGPSocketsError) kPGPSocketsErrorBadF);
				}
			}
		}
		if (ioErrorSet != NULL) {
			if (ioErrorSet->fd_count > PGPSOCKETSET_SETSIZE) {
				throw((PGPSocketsError) kPGPSocketsErrorNotSock);
			}
			for (i = 0; i < ioErrorSet->fd_count; i++) {
				if (! CSocket::VerifyPGPSocketRef((CSocket *) ioErrorSet->
				fd_array[i])) {
					throw((PGPSocketsError) kPGPSocketsErrorBadF);
				}
			}
		}
		if (inTimeout != NULL) {
			if ((inTimeout->tv_sec < 0) || (inTimeout->tv_usec < 0)) {
				throw((PGPSocketsError) kPGPSocketsErrorInval);
			}
		}
		
		result = CSocket::Select(	ioReadSet,
									ioWriteSet,
									ioErrorSet,
									inTimeout);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;

	return result;
}



	short
__PGPSocketsIsSet(
	PGPSocketRef	inSocketRef,
	PGPSocketSet *	inSocketSet)
{
	short 	result = 0;
	ushort i;
	
	for (i = 0; i < inSocketSet->fd_count; i++) {
		if (inSocketSet->fd_array[i] == inSocketRef) {
			result = 1;
			break;
		}
	}
	
	return result;
}



	PGPHostEntry *
PGPGetHostByName(
	const char * inName)
{
	PGPHostEntry *	result = NULL;
	PGPSocketsError	err = noErr;
	
	try {
		CSocket::Initialize();

		// Make sure that the parameters are correct
		if (inName == NULL) {
			throw((PGPSocketsError) kPGPSocketsErrorHostNotFound);
		}

		result = CSocket::sInternetUtilities->GetHostByName(inName);
	}
	
	catch (PGPSocketsError exception) {
		err = MapPGPSocketsError(exception);
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastHostError = err;

	return result;
}




	PGPSocketsError
PGPGetLastSocketsError(void)
{
	return sLastError;
}



	PGPSocketsError
PGPGetLastHostError(void)
{
	return sLastHostError;
}



	void
PGPSetDefaultCallback(
	PGPSocketsCallback	inCallback,
	void *				inData)
{
	CSocket::SetDefaultCallback(inCallback, inData);
}



	short
PGPSetCallback(
	PGPSocketRef		inSocketRef,
	PGPSocketsCallback	inCallback,
	void *				inData)
{
	int				result = -1;
	PGPSocketsError	err = noErr;
	
	try {
		// Make sure that the parameters are correct
		if (! CSocket::VerifyPGPSocketRef((CSocket *) inSocketRef)) {
			throw((PGPSocketsError) kPGPSocketsErrorBadF);
		}
		
		// Everything is good, so set the callback
		((CSocket *) inSocketRef)->SetCallback(inCallback, inData);
		result = 0;
	}
	
	catch (PGPSocketsError exception) {
		err = exception;
	}
	
	catch (...) {
		err = kPGPSocketsErrorUnknown;
	}
	
	sLastError = err;
	
	return result;
}



	PGPSocketsError
MapPGPSocketsError(
	PGPSocketsError	inErr)
{
	if (inErr >= kPGPSocketsErrorUnknown) {
		return inErr;
	}

	PGPSocketsError	result = kPGPSocketsErrorUnknown;
	
	switch (inErr) {
		case kOTBadAddressErr:
		{
			result = kPGPSocketsErrorAddrNotAvail;
		}
		break;
		
		
		case kOTBadOptionErr:
		{
			result = kPGPSocketsErrorOpNotSupp;
		}
		break;
		
		
		case kOTNoAddressErr:
		{
			result = kPGPSocketsErrorDestAddReq;
		}
		break;
		
		
		case kOTNotSupportedErr:
		{
			result = kPGPSocketsErrorOpNotSupp;
		}
		break;
		
		
		case kOTBadDataErr:
		{
			result = kPGPSocketsErrorMsgSize;
		}
		break;
		
		
		case kOTBadNameErr:
		{
			result = kPGPSocketsErrorHostNotFound;
		}
		break;
		
		
		case kOTBadQLenErr:
		{
			result = kPGPSocketsErrorBadQLen;
		}
		break;
		
		
		case kOTAddressBusyErr:
		{
			result = kPGPSocketsErrorAddrInUse;
		}
		break;
		
		
		case kOTOutOfMemoryErr:
		{
			result = kPGPSocketsErrorNoBufs;
		}
		break;
		
		
		case kOTCanceledErr:
		{
			result = kPGPSocketsErrorUserCanceled;
		}
		break;
	}
	
	return result;
}
