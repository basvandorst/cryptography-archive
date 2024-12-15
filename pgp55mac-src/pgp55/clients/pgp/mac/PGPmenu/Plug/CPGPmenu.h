/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPmenu.h,v 1.5.10.1 1997/12/05 22:14:28 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPmenuGlobals.h"
#include "PGPSharedEncryptDecrypt.h"

class	CPGPmenuPlug {
public:
					CPGPmenuPlug(ProcessInfoRec * inProcInfoPtr,
						FSSpec * inFSSpecPtr);
	virtual			~CPGPmenuPlug();

	static Boolean	CaptureThisApp(OSType inCreator,
						Boolean * outPrivateScrap,
						Boolean * outUseOutputDialog);

	static void		ShowError(PGPError inErr);
					
protected:
	SGlobals &		mGlobals;
};



// UPP proc infos for patches
enum {
	kUPPExitToShellProcInfo	=	kPascalStackBased		
};

pascal	void	MyExitToShell();

