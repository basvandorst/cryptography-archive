/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CFocusBorder.h,v 1.2 1997/10/30 20:53:00 heller Exp $
____________________________________________________________________________*/

#include "LGAFocusBorder.h"

class CFocusBorder : public LGAFocusBorder
{
public:
	
	enum { class_ID = 'FBDR' };

						CFocusBorder(LStream *inStream);
	virtual				~CFocusBorder(void);

	virtual	void		DrawFocusBorder(Int16 inBitDepth);
	virtual	RgnHandle	GetFocusRegion(void);
};