/*____________________________________________________________________________
	CResFileCloser.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CResFileCloser.h,v 1.3 1997/09/18 01:35:02 lloyd Exp $
____________________________________________________________________________*/
#pragma once

class CResFileCloser
{
protected:
	short	mFileRef;
	
public:
	CResFileCloser( short fileRef )	{ mFileRef	= fileRef; }
	CResFileCloser()
	{
		if ( mFileRef > 0 )	CloseResFile( mFileRef );
	}
};

