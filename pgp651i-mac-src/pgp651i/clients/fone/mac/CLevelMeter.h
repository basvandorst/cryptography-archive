/*____________________________________________________________________________	Copyright (C) 1996-1999 Network Associates, Inc.	All rights reserved.	$Id: CLevelMeter.h,v 1.4 1999/03/10 02:34:34 heller Exp $____________________________________________________________________________*/#include <LPane.h>#include <LPeriodical.h>class CLevelMeter	:	public LPane,						public LPeriodical{public:	enum { class_ID = 'Metr' };	static CLevelMeter *CreateLevelMeterStream(LStream *inStream);						CLevelMeter();						CLevelMeter(LStream *inStream);						~CLevelMeter();	void				DrawSelf();	void				SpendTime(const EventRecord &inMacEvent);	void				SetLevel(long level);	void				SetStatus(Boolean status);private:	void				DoDraw(short full);		Boolean				mOn;	long				mLevel;	PixPatHandle		mBackPat, mForePat, mDisabledPat;};