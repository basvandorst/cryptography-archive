/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecContext.c,v 1.5 1999/03/10 02:51:57 heller Exp $
____________________________________________________________________________*/

#include "pgpIPsecContextPriv.h"
#include "pgpMem.h"


PGPError PGPNewIPsecContext(PGPIPsecContextRef *ipsec)
{
	PGPError err = kPGPError_NoErr;
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPIPsecContextRef newIPsec = NULL;

	if (IsNull(ipsec))
		return kPGPError_BadParams;

	*ipsec = NULL;

	err = PGPNewMemoryMgr(0, &memoryMgr);
	if (IsPGPError(err))
		return err;

	newIPsec = (PGPIPsecContextRef) PGPNewData(memoryMgr, 
										sizeof(PGPIPsecContext),
										kPGPMemoryMgrFlags_Clear);

	if (IsNull(newIPsec))
	{
		PGPFreeMemoryMgr(memoryMgr);
		return kPGPError_OutOfMemory;
	}

	newIPsec->memory = memoryMgr;

	*ipsec = newIPsec;
	return kPGPError_NoErr;;
}


PGPError PGPFreeIPsecContext(PGPIPsecContextRef ipsec)
{
	PGPMemoryMgrRef memoryMgr;

	if (IsNull(ipsec))
		return kPGPError_BadParams;

	memoryMgr = ipsec->memory;
	
	PGPFreeData(ipsec);
	PGPFreeMemoryMgr(memoryMgr);

	return kPGPError_NoErr;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/