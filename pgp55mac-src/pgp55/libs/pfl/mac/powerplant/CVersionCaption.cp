/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	This class implements a caption field which displays the version
	number found in the 'vers' 1 resource.

	$Id: CVersionCaption.cp,v 1.4 1997/09/18 01:34:34 lloyd Exp $
____________________________________________________________________________*/
#include "pgpMem.h"

#include "MacStrings.h"

#include "CVersionCaption.h"

CVersionCaption::CVersionCaption(LStream *inStream)
		: LCaption(inStream)
{
}

CVersionCaption::~CVersionCaption(void)
{
}

	void
CVersionCaption::FinishCreateSelf(void)
{
	Str255		versionString;
	VersRec		**versionResource;
	
	LCaption::FinishCreateSelf();
	
	versionString[0] = 0;
	
	versionResource = (VersRec **) GetResource( 'vers', 1 );
	if( IsntNull( versionResource ) )
	{
		NumVersion numVersionCopy;
		
		// NumVersionToString will move memory.
		// Copy the numVersion field from
		// the resource to a local variable.
		numVersionCopy = (**versionResource).numericVersion;
		ReleaseResource( (Handle) versionResource );
		
		NumVersionToString( &numVersionCopy, versionString );
	}
	else
	{
		pgpDebugMsg( "CVersionCaption: No 'vers' resource found" );
	}

	SetDescriptor( versionString );
}