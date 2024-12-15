/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGAFix.h,v 1.1 1997/08/20 12:33:36 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include <LGAPopup.h>
#include <LGACheckbox.h>

class CGAPopupFix : public LGAPopup
{
public:
	enum { class_ID = 'GAPf' };
							CGAPopupFix(LStream *inStream);
	virtual					~CGAPopupFix();

	void					DrawSelf();
	
private:
};

class CGACheckboxFix : public LGACheckbox
{
public:
	enum { class_ID = 'GACf' };
							CGACheckboxFix(LStream *inStream);
	virtual					~CGACheckboxFix();

	void					DrawSelf();
	
private:
};

