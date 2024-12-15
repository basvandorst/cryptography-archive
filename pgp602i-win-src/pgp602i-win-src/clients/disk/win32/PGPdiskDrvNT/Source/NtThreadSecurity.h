//////////////////////////////////////////////////////////////////////////////
// NtThreadSecurity.h
//
// Kernel-mode thread security functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: NtThreadSecurity.h,v 1.1.2.3 1998/07/06 08:57:52 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_NtThreadSecurity_h	// [
#define Included_NtThreadSecurity_h

#include "DualErr.h"


////////////
// Constants
////////////

// From Winnt.h.

#define TOKEN_ASSIGN_PRIMARY    (0x0001)
#define TOKEN_DUPLICATE         (0x0002)
#define TOKEN_IMPERSONATE       (0x0004)
#define TOKEN_QUERY             (0x0008)
#define TOKEN_QUERY_SOURCE      (0x0010)
#define TOKEN_ADJUST_PRIVILEGES (0x0020)
#define TOKEN_ADJUST_GROUPS     (0x0040)
#define TOKEN_ADJUST_DEFAULT    (0x0080)

#define TOKEN_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED  |\
                          TOKEN_ASSIGN_PRIMARY      |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)


#define TOKEN_READ       (STANDARD_RIGHTS_READ      |\
                          TOKEN_QUERY)


#define TOKEN_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

#define TOKEN_EXECUTE    (STANDARD_RIGHTS_EXECUTE)


////////
// Types
////////

// From Winnt.h.

typedef enum _TOKEN_TYPE 
{
    TokenPrimary = 1,
    TokenImpersonation

} TOKEN_TYPE, *PTOKEN_TYPE;


/////////////////////
// Imported Functions
/////////////////////

#define EZNTAPI extern "C" NTSYSAPI NTSTATUS NTAPI

EZNTAPI	ZwOpenProcess(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, 
			POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);

EZNTAPI	ZwOpenProcessToken(HANDLE ProcessHandle, PGPUInt32 DesiredAccess, 
			PHANDLE TokenHandle);

EZNTAPI	ZwDuplicateToken(HANDLE TokenHandle, PGPUInt32 DesiredAccess, 
			POBJECT_ATTRIBUTES ObjectAttributes, 
			SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, 
			TOKEN_TYPE TokenType, PHANDLE phNewToken);


/////////////////////
// Exported functions
/////////////////////

PGPBoolean	IsImpersonationTokenSet();
DualErr		SetImpersonationToken(PGPUInt32 procId);
void		ClearImpersonationToken();

void		ImpersonateToken();
void		RevertToSelf();


#endif	// ] Included_NtThreadSecurity_h
