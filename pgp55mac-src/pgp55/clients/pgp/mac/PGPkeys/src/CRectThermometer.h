/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CRectThermometer.h,v 1.1 1997/06/30 10:33:12 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include <LControl.h>

class CRectThermometer : public LControl
{
private:
	typedef LControl Inherited;

public:
	enum { class_ID = 'rThm' };
	static CRectThermometer * CreateFromStream( LStream * inStreamP );
	CRectThermometer( LStream * inStreamP );
	~CRectThermometer();
	
protected:
	void DrawSelf( void );
};
