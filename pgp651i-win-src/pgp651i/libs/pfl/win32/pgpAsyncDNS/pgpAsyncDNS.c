/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	pgpAsyncDNS.c	-	Async calls for dns
	

	$Id: pgpAsyncDNS.c,v 1.4 1999/03/10 02:47:07 heller Exp $
____________________________________________________________________________*/

#include <process.h>

#include "pgpRMWOLock.h"
#include "pgpAsyncDNS.h"


#define kMaxNameSize	256


struct PGPAsyncHostEntry {
	HANDLE			threadHandle;
	HANDLE			cancelSemaphore;
	PGPRMWOLock		resultLock;
	PGPError		result;
	PGPUInt32		address;
	char			name[kMaxNameSize];
};



	static void
sGetHostByNameThread(
	void *	inAsyncHostEntry)
{
	PGPHostEntry *				hostEntry;
	PGPError					result = kPGPError_NoErr;
	PGPSocketsThreadStorageRef	storage;
	
	PGPSocketsInit();
	PGPSocketsCreateThreadStorage(&storage);
	hostEntry = PGPGetHostByName(((PGPAsyncHostEntryRef) inAsyncHostEntry)->name);
	if (hostEntry == NULL) {
		result = PGPGetLastSocketsError();
		if (result == kPGPError_NoErr) {
			result = kPGPError_UnknownError;
		}
	}
	PGPSocketsDisposeThreadStorage(storage);
	PGPSocketsCleanup();
	PGPRMWOLockStartWriting(&((PGPAsyncHostEntryRef) inAsyncHostEntry)->resultLock);
	if (((PGPAsyncHostEntryRef) inAsyncHostEntry)->result != kPGPError_UserAbort) {
		((PGPAsyncHostEntryRef) inAsyncHostEntry)->result = result;
		if (hostEntry != NULL) {
			((PGPAsyncHostEntryRef) inAsyncHostEntry)->address = 
				*((PGPUInt32 *)hostEntry->h_addr);
		}
	}
	PGPRMWOLockStopWriting(&((PGPAsyncHostEntryRef) inAsyncHostEntry)->resultLock);
	
	if (result == kPGPError_UserAbort) {
		CloseHandle(((PGPAsyncHostEntryRef) inAsyncHostEntry)->cancelSemaphore);
		DeletePGPRMWOLock(&((PGPAsyncHostEntryRef) inAsyncHostEntry)->resultLock);
		free(inAsyncHostEntry);
	}
}



	static void
sGetHostByAddressThread(
	void *	inAsyncHostEntry)
{
	PGPHostEntry *				hostEntry;
	PGPInternetAddress			theAddress;
	PGPError					result = kPGPError_NoErr;
	PGPSocketsThreadStorageRef	storage;
	
	theAddress.s_addr = PGPHostToNetLong(
							((PGPAsyncHostEntryRef) inAsyncHostEntry)->address);
	PGPSocketsInit();
	PGPSocketsCreateThreadStorage(&storage);
	hostEntry = PGPGetHostByAddress(	(const char *) &theAddress,
										sizeof(PGPInternetAddress),
										kPGPProtocolFamilyInternet);
	if (hostEntry == NULL) {
		result = PGPGetLastSocketsError();
		if (result == kPGPError_NoErr) {
			result = kPGPError_UnknownError;
		}
	}
	PGPSocketsDisposeThreadStorage(storage);
	PGPSocketsCleanup();
	PGPRMWOLockStartWriting(&((PGPAsyncHostEntryRef) inAsyncHostEntry)->resultLock);
	if (((PGPAsyncHostEntryRef) inAsyncHostEntry)->result != kPGPError_UserAbort) {
		((PGPAsyncHostEntryRef) inAsyncHostEntry)->result = result;
		if (hostEntry != NULL) {
			strcpy(((PGPAsyncHostEntryRef) inAsyncHostEntry)->name, hostEntry->h_name);
		}
	}
	PGPRMWOLockStopWriting(&((PGPAsyncHostEntryRef) inAsyncHostEntry)->resultLock);
	
	if (result == kPGPError_UserAbort) {
		CloseHandle(((PGPAsyncHostEntryRef) inAsyncHostEntry)->cancelSemaphore);
		DeletePGPRMWOLock(&((PGPAsyncHostEntryRef) inAsyncHostEntry)->resultLock);
		free(inAsyncHostEntry);
	}
}


	PGPError
PGPStartAsyncGetHostByName(
	const char *			inName,
	PGPAsyncHostEntryRef *	outAsyncHostEntry)
{
	PGPError				result = kPGPError_NoErr;
	PGPAsyncHostEntryRef	ref = kPGPInvalidAsyncHostEntryRef;
	
	PGPValidatePtr(outAsyncHostEntry);
	*outAsyncHostEntry = kPGPInvalidAsyncHostEntryRef;
	PGPValidatePtr(inName);
	PGPValidateParam(strlen(inName) < kMaxNameSize);
	
	ref = (PGPAsyncHostEntryRef) calloc(1, sizeof(struct PGPAsyncHostEntry));
	if (ref == NULL) {
		result = kPGPError_OutOfMemory;
	}
	
	if (IsntPGPError(result)) {
		ref->result = kPGPError_SocketsInProgress;
		InitializePGPRMWOLock(&ref->resultLock);
	}
	
	if (IsntPGPError(result)) {
		ref->cancelSemaphore = CreateSemaphore(NULL, 0, 3, NULL);
		if (ref->cancelSemaphore == NULL) {
			result = kPGPError_UnknownError;
		}
	}
	
	if (IsntPGPError(result)) {
		strcpy(ref->name, inName);
		ref->threadHandle = (HANDLE) _beginthread(sGetHostByNameThread, 0, ref);
		if (ref->threadHandle == (HANDLE) -1) {
			result = kPGPError_UnknownError;
		}
	}
	
	if (IsntPGPError(result)) {
		*outAsyncHostEntry = ref;
	} else {
		if (ref != NULL) {
			if (ref->cancelSemaphore != NULL) {
				CloseHandle(ref->cancelSemaphore);
			}
			DeletePGPRMWOLock(&ref->resultLock);
			free(ref);
		}
	}
	
	return result;
}



	PGPError
PGPStartAsyncGetHostByAddress(
	PGPUInt32				inAddress,
	PGPAsyncHostEntryRef *	outAsyncHostEntry)
{
	PGPError				result = kPGPError_NoErr;
	PGPAsyncHostEntryRef	ref = kPGPInvalidAsyncHostEntryRef;
	
	PGPValidatePtr(outAsyncHostEntry);
	*outAsyncHostEntry = kPGPInvalidAsyncHostEntryRef;
	
	ref = (PGPAsyncHostEntryRef) calloc(1, sizeof(struct PGPAsyncHostEntry));
	if (ref == NULL) {
		result = kPGPError_OutOfMemory;
	}
	
	if (IsntPGPError(result)) {
		ref->result = kPGPError_SocketsInProgress;
		InitializePGPRMWOLock(&ref->resultLock);
	}
	
	if (IsntPGPError(result)) {
		ref->cancelSemaphore = CreateSemaphore(NULL, 0, 3, NULL);
		if (ref->cancelSemaphore == NULL) {
			result = kPGPError_UnknownError;
		}
	}
	
	if (IsntPGPError(result)) {
		ref->address = inAddress;
		ref->threadHandle = (HANDLE) _beginthread(sGetHostByAddressThread, 0, ref);
		if (ref->threadHandle == (HANDLE) -1) {
			result = kPGPError_UnknownError;
		}
	}
	
	if (IsntPGPError(result)) {
		*outAsyncHostEntry = ref;
	} else {
		if (ref != NULL) {
			if (ref->cancelSemaphore != NULL) {
				CloseHandle(ref->cancelSemaphore);
			}
			DeletePGPRMWOLock(&ref->resultLock);
			free(ref);
		}
	}
	
	return result;
}



	PGPError
PGPWaitForGetHostByName(
	PGPAsyncHostEntryRef	inAsyncHostEntry,
	PGPUInt32 *				outAddress)
{
	PGPError	result;
	
	PGPValidateParam(PGPAsyncHostEntryRefIsValid(inAsyncHostEntry));
	PGPValidatePtr(outAddress);
	
	WaitForMultipleObjects(2, &inAsyncHostEntry->threadHandle, FALSE, INFINITE);
	PGPRMWOLockStartReading(&inAsyncHostEntry->resultLock);
	result = inAsyncHostEntry->result;
	PGPRMWOLockStopReading(&inAsyncHostEntry->resultLock);
	if (result != kPGPError_UserAbort) {
		*outAddress = inAsyncHostEntry->address;
		CloseHandle(inAsyncHostEntry->cancelSemaphore);
		DeletePGPRMWOLock(&inAsyncHostEntry->resultLock);
		free(inAsyncHostEntry);
	}
	
	return result;
}



	PGPError
PGPWaitForGetHostByAddress(
	PGPAsyncHostEntryRef	inAsyncHostEntry,
	char *					outName,
	PGPInt32				inNameLength)
{
	PGPError	result;
	
	PGPValidateParam(PGPAsyncHostEntryRefIsValid(inAsyncHostEntry));
	PGPValidatePtr(outName);
	PGPValidateParam(inNameLength > 0);
	
	WaitForMultipleObjects(2, &inAsyncHostEntry->threadHandle, FALSE, INFINITE);
	PGPRMWOLockStartReading(&inAsyncHostEntry->resultLock);
	result = inAsyncHostEntry->result;
	PGPRMWOLockStopReading(&inAsyncHostEntry->resultLock);
	if (result != kPGPError_UserAbort) {
		strncpy(outName, inAsyncHostEntry->name, inNameLength);
		outName[inNameLength - 1] = 0;
		CloseHandle(inAsyncHostEntry->cancelSemaphore);
		DeletePGPRMWOLock(&inAsyncHostEntry->resultLock);
		free(inAsyncHostEntry);
	}
	
	return result;
}



	PGPError
PGPCancelAsyncHostEntryRef(
	PGPAsyncHostEntryRef	inAsyncHostEntry)
{
	PGPError	result = kPGPError_NoErr;

	PGPValidateParam(PGPAsyncHostEntryRefIsValid(inAsyncHostEntry));
	
	PGPRMWOLockStartWriting(&inAsyncHostEntry->resultLock);
	if (inAsyncHostEntry->result == kPGPError_SocketsInProgress) {
		inAsyncHostEntry->result = kPGPError_UserAbort;
		ReleaseSemaphore(inAsyncHostEntry->cancelSemaphore, 1, NULL);
	}
	PGPRMWOLockStopWriting(&inAsyncHostEntry->resultLock);
	
	return result;
}
