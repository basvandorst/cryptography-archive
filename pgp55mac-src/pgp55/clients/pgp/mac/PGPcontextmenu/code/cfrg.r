/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: cfrg.r,v 1.2.10.1 1997/12/05 22:13:49 mhw Exp $
____________________________________________________________________________*/



#define UseExtendedCFRGTemplate 1

#include "CodeFragmentTypes.r"


resource 'cfrg' (0) {
	{	/* array memberArray: 4 elements */
		extendedEntry {
			kPowerPC,
			kFullLib,
			kNoVersionNum,
			kNoVersionNum,
			kDefaultStackSize,
			kNoAppSubFolder,
			kIsLib,
			kOnDiskFlat,
			kZeroOffset,
			kSegIDZero,
			"PGPCMPlugin",
			kFragSOMClassLibrary,
			"AbstractCMPlugin",
			"",
			"",
			"A Contextual Menu Plugin which adds PGP support to the Finder."
		}
	}
};
