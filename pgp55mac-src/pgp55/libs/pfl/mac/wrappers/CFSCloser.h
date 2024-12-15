/*____________________________________________________________________________
	CFSCloser.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CFSCloser.h,v 1.2 1997/05/01 20:28:46 lloyd Exp $
____________________________________________________________________________*/
#pragma once

class CFSCloser
{
protected:
	short	mFileRef;
	
public:
	CFSCloser( short fileRef )
			{ mFileRef	= fileRef; }
			
	~CFSCloser()
			{ if ( mFileRef > 0 )	FSClose( mFileRef ); }
};

