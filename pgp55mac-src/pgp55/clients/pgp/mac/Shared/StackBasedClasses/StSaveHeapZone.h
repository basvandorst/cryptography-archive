/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: StSaveHeapZone.h,v 1.2.10.1 1997/12/05 22:15:24 mhw Exp $
____________________________________________________________________________*/

#pragma once 

class StSaveHeapZone {
public:
				StSaveHeapZone()
					{ mSavedZone = ::GetZone(); }
				StSaveHeapZone(THz inSetZone)
					{ mSavedZone = ::GetZone(); ::SetZone(inSetZone); }
	virtual		~StSaveHeapZone()
					{ ::SetZone(mSavedZone); }

protected:
	THz			mSavedZone;
};
