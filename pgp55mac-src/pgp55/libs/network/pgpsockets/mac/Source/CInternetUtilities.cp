/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CInternetUtilities.cp,v 1.1.10.1 1997/12/07 04:26:39 mhw Exp $
____________________________________________________________________________*/

#include "CInternetUtilities.h"


char			CInternetUtilities::sHNameBuffer[kMaxHostNameLen + 1];
char **			CInternetUtilities::sHAliasesBuffer = NULL;
UInt32 *		CInternetUtilities::sHAddressesListBuffer[kMaxHostAddrs];
UInt32			CInternetUtilities::sHAddressesBuffer[kMaxHostAddrs];
PGPHostEntry	CInternetUtilities::sHostEntry = {sHNameBuffer,
										sHAliasesBuffer,
										kPGPAddressFamilyInternet,
										sizeof(UInt32),
										(char**) sHAddressesListBuffer};
