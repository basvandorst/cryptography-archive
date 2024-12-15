/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPemailer.h,v 1.2.10.1 1997/12/05 22:13:59 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPemailerGlobals.h"

class	CPGPemailerPlug {
public:
					CPGPemailerPlug(ProcessInfoRec * inProcInfoPtr,
						FSSpec * inFSSpecPtr);
	virtual			~CPGPemailerPlug();

protected:
	SGlobals		mGlobals;
};



// UPP proc infos for patches
enum {
	kUPPExitToShellProcInfo	=	kPascalStackBased		
};

pascal	void	MyExitToShell();


