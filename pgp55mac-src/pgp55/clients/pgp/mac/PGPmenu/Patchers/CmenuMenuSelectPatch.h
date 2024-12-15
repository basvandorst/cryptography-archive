/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuMenuSelectPatch.h,v 1.2.10.1 1997/12/05 22:14:19 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPmenuGlobals.h"
#include "CMenuSelectPatcher.h"
#include "CPGPEncoderDecoder.h"

class CmenuMenuSelectPatch : public CMenuSelectPatcher {
public:
						CmenuMenuSelectPatch(SGlobals & inGlobals);
	virtual				~CmenuMenuSelectPatch();
	
protected:
	SGlobals &			mGlobals;
	Boolean				mOptionDown;
	Boolean				mWorkingCount;
	
	virtual long		NewMenuSelect(Point startPt);
	
	SInt32				PGPmenuFilter(SInt32 inSelection);
	
	void				SendToolsEvent(SInt16 inSelection);
	void				Encode(EEncodeOptions inOptions);
	void				Decode();
			
//	SInt32				CountFinderSelection(DescType inObjectType);
	Boolean				CheckFinderSelection(Boolean * outTrashSelected);

	
	void				AdjustMenuItems();
		
};
