/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	This class implements the PowerPlant LPicture class with one addition:
	when the bit depth is less than 8 bits, the picture drawn is the one
	referenced by mPICTid + 1, if present. It is assumed that the two
	pictures are the same size.

	$Id: CPicture.cp,v 1.4.10.1 1997/12/07 04:27:05 mhw Exp $
____________________________________________________________________________*/

/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */
#include "pgpMem.h"

#include <UDrawingUtils.h>

#include "CPicture.h"

// ---------------------------------------------------------------------------
//	* Constructors/Destructors
// ---------------------------------------------------------------------------

CPicture::CPicture(LStream *inStream)
		: LPicture(inStream)
{
	SPictureInfo	pictInfo;
	
	inStream->ReadData( &pictInfo, sizeof(pictInfo) );
	InitCPicture( &pictInfo );
}

CPicture::~CPicture(void)
{
}

// ---------------------------------------------------------------------------
//	* InitCPicture
// ---------------------------------------------------------------------------

	void
CPicture::InitCPicture(const SPictureInfo *pictInfo)
{
	Handle	thePicture;
	Boolean	saveResLoad;
	
	pgpAssertAddrValid( pictInfo, SPictureInfo );

	mBWResID	= pictInfo->bwResID;
	m4BitResID	= pictInfo->x4BitResID;
	// 8 bit Resource ID comes from LPicture
	m16BitResID	= pictInfo->x16BitResID;
	m24BitResID	= pictInfo->x24BitResID;
	
	pgpAssert( mBWResID != 0 );
	pgpAssert( m4BitResID != 0 );
	pgpAssert( m16BitResID != 0 );
	pgpAssert( m24BitResID != 0 );
	
	// Determine if we have pictures available. If not, we revert to the
	// next lowest bit depth
	saveResLoad = LMGetResLoad();
	SetResLoad( FALSE );
	
	thePicture = GetResource( 'PICT', mBWResID );
	if( IsNull( thePicture ) )
	{
		// Special case: BW Res ID becomes the 8-bit resource ID if not present.
		mBWResID = mPICTid;
	}
	
	thePicture = GetResource( 'PICT', m4BitResID );
	if( IsNull( thePicture ) )
	{
		m4BitResID = mBWResID;
	}

	thePicture = GetResource( 'PICT', m16BitResID );
	if( IsNull( thePicture ) )
	{
		m16BitResID = mPICTid;
	}

	thePicture = GetResource( 'PICT', m24BitResID );
	if( IsNull( thePicture ) )
	{
		m24BitResID = m16BitResID;
	}
	
	SetResLoad( TRUE );
}

// ---------------------------------------------------------------------------
//	* DrawSelf
// ---------------------------------------------------------------------------

	void
CPicture::DrawSelf(void)
{
	Rect		paneRect;
	ResIDT		savePictID;
	
	savePictID = mPICTid;
	
	CalcLocalFrameRect( paneRect );

	{
		StDeviceLoop	devLoop( paneRect );		
		SInt16			depth;
		RGBColor		saveBackColor;
		static RGBColor	sRGBWhiteColor = {0xFFFF, 0xFFFF, 0xFFFF};

		GetBackColor( &saveBackColor );
		RGBBackColor( &sRGBWhiteColor );
		
		while( devLoop.NextDepth( depth ) )
		{
			if( depth >= 24 )
			{
				mPICTid = m24BitResID;
			}
			else if( depth >= 16 )
			{
				mPICTid = m16BitResID;
			}
			else if( depth >= 8 )
			{
				// Nothing special to do
			}
			else if( depth >= 4 )
			{
				mPICTid = m4BitResID;
			}
			else
			{
				mPICTid = mBWResID;
			}

			LPicture::DrawSelf();
		}
	
		RGBBackColor( &saveBackColor );
	}
	
	mPICTid = savePictID;
}
