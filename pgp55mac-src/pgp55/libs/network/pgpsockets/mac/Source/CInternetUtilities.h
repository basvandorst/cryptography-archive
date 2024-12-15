/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CInternetUtilities.h,v 1.2.10.1 1997/12/07 04:26:40 mhw Exp $
____________________________________________________________________________*/

#pragma once

#include "CSocket.h"

class CInternetUtilities {
public:
							CInternetUtilities() { }
	virtual					~CInternetUtilities() { }
	virtual PGPHostEntry *	GetHostByName(const char * inName) = 0;

protected:
	static char				sHNameBuffer[kMaxHostNameLen + 1];
	static char **			sHAliasesBuffer;
	static UInt32 *			sHAddressesListBuffer[kMaxHostAddrs];
	static UInt32			sHAddressesBuffer[kMaxHostAddrs];
	static PGPHostEntry		sHostEntry;
};
