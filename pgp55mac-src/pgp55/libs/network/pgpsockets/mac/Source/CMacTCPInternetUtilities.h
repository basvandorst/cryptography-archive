/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMacTCPInternetUtilities.h,v 1.1.10.1 1997/12/07 04:26:42 mhw Exp $
____________________________________________________________________________*/

#pragma once 


#include <AddressXlation.h>
#include "CInternetUtilities.h"


class CMacTCPInternetUtilities : public CInternetUtilities {
public:
							CMacTCPInternetUtilities();
	virtual					~CMacTCPInternetUtilities();
	
	virtual PGPHostEntry *	GetHostByName(const char * inName);

protected:
	static UInt32			sOutstandingCalls;
	Boolean					mDone;
	
	static pascal void		ResultProc(struct hostInfo * hostInfoPtr,
								char * userDataPtr);
};
