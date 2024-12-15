/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CRecessedCaption.h,v 1.2 1997/09/17 19:36:40 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include <LCaption.h>

class CRecessedCaption : public LCaption
{
private:
	typedef LCaption Inherited;
public:
	enum { class_ID = 'rCap' };
	static CRecessedCaption * CreateFromStream(LStream * inStream);

	// replicate all four of LCaption's constructors
	CRecessedCaption();
	CRecessedCaption(	const CRecessedCaption& inBorderCaption );
	CRecessedCaption(	LStream * inStream );
	CRecessedCaption(	const SPaneInfo &	inPaneInfo,
						ConstStringPtr		inString,
						ResIDT				inTextTraitsID );
	
protected:
	virtual void FinishCreateSelf();
	virtual void DrawSelf();
	virtual void DrawBackground(	const Rect & localFrame );
	virtual void DrawBox(			const Rect & localFrame );
	virtual void DrawGrayBox(		const Rect & localFrame );
	virtual void DrawText(			const Rect & localFrame );
};
