/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */

#pragma once

#include <LPicture.h>

#pragma options align=mac68k

typedef struct SPictureInfo
{
	ResIDT		bwResID;
	ResIDT		x4BitResID;
	ResIDT		x16BitResID;
	ResIDT		x24BitResID;
	UInt32		reserved[3];
	
} SPictureInfo;

#pragma options align=reset

class CPicture : public LPicture
{
public:
	enum { class_ID = 'Pict' };

						CPicture(LStream *inStream);
	virtual				~CPicture(void);
	
protected:

	ResIDT				mBWResID;
	ResIDT				m4BitResID;
	ResIDT				m16BitResID;
	ResIDT				m24BitResID;
	
	virtual void		DrawSelf(void);
	
private:

	void				InitCPicture(const SPictureInfo *pictInfo);
};