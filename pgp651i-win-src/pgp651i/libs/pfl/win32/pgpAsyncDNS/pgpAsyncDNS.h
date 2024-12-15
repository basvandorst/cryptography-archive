/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	pgpAsyncDNS.h	-	Async calls for dns
	

	$Id: pgpAsyncDNS.h,v 1.3 1999/03/10 02:47:09 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpAsyncDNS_h	/* [ */
#define Included_pgpAsyncDNS_h


#if ! PGP_WIN32
#error this file should only be used for PGP_WIN32
#endif

#include "pgpSockets.h"

PGP_BEGIN_C_DECLARATIONS

typedef struct PGPAsyncHostEntry *		PGPAsyncHostEntryRef;

#define kPGPInvalidAsyncHostEntryRef	((PGPAsyncHostEntryRef) NULL)
#define PGPAsyncHostEntryRefIsValid(x)	((x) != kPGPInvalidAsyncHostEntryRef)



PGPError	PGPStartAsyncGetHostByName(const char * inName,
				PGPAsyncHostEntryRef * outAsyncHostEntry);
				
PGPError	PGPStartAsyncGetHostByAddress(PGPUInt32 inAddress,
				PGPAsyncHostEntryRef * outAsyncHostEntry);
				
PGPError	PGPWaitForGetHostByName(PGPAsyncHostEntryRef inAsyncHostEntry,
				PGPUInt32 * outAddress);
				
PGPError	PGPWaitForGetHostByAddress(PGPAsyncHostEntryRef inAsyncHostEntry,
				char * outName, PGPInt32 inNameLength); 
				
PGPError	PGPCancelAsyncHostEntryRef(PGPAsyncHostEntryRef inAsyncHostEntry);

PGP_END_C_DECLARATIONS


#endif /* ] */