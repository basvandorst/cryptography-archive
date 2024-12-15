/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CTriangleControl.h,v 1.2 1999/03/10 02:41:38 heller Exp $____________________________________________________________________________*/#ifndef __CTriangleControl__#define __CTriangleControl__#include <LControl.h>class CTriangleControl : public LControl{public:	enum { class_ID = 'Tria' };	static CTriangleControl *CreateTriangleControlStream(LStream *inStream);						CTriangleControl(void);						CTriangleControl(const CTriangleControl &inOriginal);						CTriangleControl(const SPaneInfo &inPaneInfo, MessageT inValueMessage,										Int32 inValue, Int32 inMinValue, Int32 inMaxValue);						CTriangleControl(LStream *inStream);	virtual				~CTriangleControl(void);		virtual void		SetValue(Int32 inValue);	protected:	virtual Int16		FindHotSpot(Point inPoint);	virtual Boolean		PointInHotSpot(Point inPoint, Int16 inHotSpot);	virtual void		HotSpotAction(Int16 inHotSpot, Boolean inCurrInside, Boolean inPrevInside);	virtual void		HotSpotResult(Int16 inHotSpot);	virtual void		DrawSelf(void);	private:	PolyHandle			mTriangleToDraw;	Boolean				mHilight;	void 				InitTriangleControl(void);	void				DrawTriangle(Boolean hilite);};#endif