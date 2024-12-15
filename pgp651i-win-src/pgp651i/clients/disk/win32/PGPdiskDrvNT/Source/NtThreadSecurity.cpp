//////////////////////////////////////////////////////////////////////////////
// NtThreadSecurity.cpp
//
// Thread security functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: NtThreadSecurity.cpp,v 1.4 1999/03/31 23:51:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>

#include "Required.h"
#include "NtThreadSecurity.h"


//////////
// Globals
//////////

HANDLE		mImpersonationToken;
PGPUInt32	mAppropriateProcId;


///////////////////////////////
// Token manipulation functions
///////////////////////////////

// DuplicateProcessToken returns a copy of the token of the specified
// process.

DualErr	
DuplicateProcessToken(PGPUInt32 procId, PHANDLE pDupedToken)
{
	CLIENT_ID			clientId;
	DualErr				derr;
	HANDLE				procHandle, token;
	NTSTATUS			status;
	OBJECT_ATTRIBUTES	objAttribs;
	PGPBoolean			openedProcess, openedToken;

	openedProcess = openedToken = FALSE;

	pgpAssertAddrValid(pDupedToken, HANDLE);

	InitializeObjectAttributes(&objAttribs, NULL, 0, NULL, NULL);

	clientId.UniqueThread = NULL;
	clientId.UniqueProcess = (PVOID) procId;

	// Open a handle to the process.
	status = ZwOpenProcess(&procHandle, PROCESS_ALL_ACCESS, &objAttribs, 
		&clientId);

	if (!NT_SUCCESS(status))
	{
		derr = DualErr(kPGDMinorError_ZwOpenProcessFailed, status);
	}

	openedProcess = derr.IsntError();

	// Open a handle to the process token.
	if (derr.IsntError())
	{
		status = ZwOpenProcessToken(procHandle, TOKEN_ALL_ACCESS, &token);

		if (!NT_SUCCESS(status))
		{
			derr = DualErr(kPGDMinorError_ZwOpenProcessTokenFailed, status);
		}

		openedToken = derr.IsntError();
	}

	// Duplicate the token.
	if (derr.IsntError())
	{
		SECURITY_QUALITY_OF_SERVICE SQOS;

		SQOS.Length = sizeof(SQOS);
		SQOS.ImpersonationLevel = SecurityImpersonation;
		SQOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
		SQOS.EffectiveOnly = FALSE;

		objAttribs.SecurityQualityOfService = (PVOID) &SQOS;

		status = ZwDuplicateToken(token, TOKEN_QUERY | TOKEN_IMPERSONATE, 
			&objAttribs, SecurityAnonymous, TokenImpersonation, pDupedToken);

		if (!NT_SUCCESS(status))
		{
			derr = DualErr(kPGDMinorError_ZwDuplicateTokenFailed, status);
		}
	}

	if (openedToken)
		ZwClose(token);

	if (openedProcess)
		ZwClose(procHandle);

	return derr;
}

// IsImpersonationTokenSet returns TRUE if an impersonation token is set, 
// FALSE otherwise.

PGPBoolean 
IsImpersonationTokenSet()
{
	return (IsntNull(mImpersonationToken));
}

// ClearImpersonationToken frees the current impersonation token.

void 
ClearImpersonationToken(HANDLE token)
{
	pgpAssert(IsImpersonationTokenSet());

	ZwClose(mImpersonationToken);
	mImpersonationToken = NULL;
}

// SetImpersonationToken creates a copy of the token of the specified process
// to be used for future impersonations.

DualErr 
SetImpersonationToken(PGPUInt32 procId)
{
	DualErr derr;

	pgpAssert(!IsImpersonationTokenSet());

	derr = DuplicateProcessToken(procId, &mImpersonationToken);

	if (derr.IsntError())
	{
		mAppropriateProcId = (PGPUInt32) PsGetCurrentProcessId();
	}

	return derr;
}

// ImpersonateToken impersonates the specified token.

void 
ImpersonateToken()
{
	pgpAssert(IsImpersonationTokenSet());

	if (mAppropriateProcId != (PGPUInt32) PsGetCurrentProcessId())
		return;

	ZwSetInformationThread(NtCurrentThread(), ThreadImpersonationToken, 
		&mImpersonationToken, sizeof(mImpersonationToken));
}

// RevertToSelf stops impersonation.

void 
RevertToSelf()
{
	HANDLE nullToken = NULL;

	if (mAppropriateProcId != (PGPUInt32) PsGetCurrentProcessId())
		return;

	ZwSetInformationThread(NtCurrentThread(), ThreadImpersonationToken, 
		&nullToken, sizeof(nullToken));
}
