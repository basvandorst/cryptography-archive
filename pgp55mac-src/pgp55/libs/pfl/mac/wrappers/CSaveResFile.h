/*____________________________________________________________________________
	CSaveResFile.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSaveResFile.h,v 1.2 1997/05/01 20:28:48 lloyd Exp $
____________________________________________________________________________*/

#pragma once


class CSaveResFile
{
protected:
	short	mSaveResFile;
	
public:
			CSaveResFile( void )		{ mSaveResFile	= CurResFile(); }
	virtual ~CSaveResFile()				{ UseResFile( mSaveResFile ); }
};

