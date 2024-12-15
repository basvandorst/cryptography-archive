//////////////////////////////////////////////////////////////////////////////
// DcbManager.h
//
// Declaration of class DcbManager.
//////////////////////////////////////////////////////////////////////////////

// $Id: DcbManager.h,v 1.4 1998/12/14 19:00:12 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_DcbManager_h	// [
#define Included_DcbManager_h

#include "DualErr.h"


///////////////////
// Class DcbManager
///////////////////

class DcbManager
{
public:
	DcbManager() { };

	DualErr	NewDcb(PDCB *ppDcb);
	void	DeleteDcb(PDCB pDcb);

private:
	void	InitDcb(PDCB pDcb);
};

#endif	// ] Included_DcbManager_h
