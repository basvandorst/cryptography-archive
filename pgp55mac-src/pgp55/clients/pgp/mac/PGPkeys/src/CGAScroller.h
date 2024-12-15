/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGAScroller.h,v 1.1 1997/06/30 10:33:04 wprice Exp $
____________________________________________________________________________*/
#pragma once

class	CGAScroller	:	public LActiveScroller
{
public:
	enum { class_ID = 'gAsc' };

						CGAScroller(LStream *inStream);
	virtual				~CGAScroller();
	void				DrawSelf();
};

