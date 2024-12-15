/*
	File:		cfrg.r

	Contains:	<contents>

	Written by:	Guy Fullerton

	Copyright:	<copyright>

	Change History (most recent first):

        <0+>     1/16/97    GBF     first checked in
*/


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
			"ContextTypeCMPlugin",
			kFragSOMClassLibrary,
			"AbstractCMPlugin",
			"",
			"",
			"A Contextual Menu Plugin which shows the type of the context descriptor"
		}
	}
};
