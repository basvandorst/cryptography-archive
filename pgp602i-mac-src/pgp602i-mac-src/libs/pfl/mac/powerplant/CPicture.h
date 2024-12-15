/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates, Inc. and its affiliates.	All rights reserved.	$Id: CPicture.h,v 1.5.16.1 1998/11/12 03:19:02 heller Exp $____________________________________________________________________________*/#pragma once#include <LPicture.h>#pragma options align=mac68ktypedef struct SPictureInfo{	ResIDT		bwResID;	ResIDT		color4BitResID;	ResIDT		color16BitResID;	ResIDT		color24BitResID;	UInt32		reserved[3];	} SPictureInfo;#pragma options align=resetclass CPicture : public LPicture{public:	enum { class_ID = 'Pict' };						CPicture(LStream *inStream);	virtual				~CPicture(void);	protected:	ResIDT				mBWResID;	ResIDT				m4BitResID;	ResIDT				m16BitResID;	ResIDT				m24BitResID;		virtual void		DrawSelf(void);	private:	void				InitCPicture(const SPictureInfo *pictInfo);};