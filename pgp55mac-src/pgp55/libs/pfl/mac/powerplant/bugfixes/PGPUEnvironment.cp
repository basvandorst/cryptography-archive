/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
**
**	This is an identical copy of UEnvironment with extra checks added for weak
**	Drag Manager import failure. It is meant as a drop-in replacement for
**	UEnvironment.cp
*/

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include <UEnvironment.h>

#ifndef __GESTALT__
#include <Gestalt.h>
#endif

// === Static Member Variable ===

EnvironmentFeature	UEnvironment::sFeatures = 0;


// ---------------------------------------------------------------------------
//		* SetFeature
// ---------------------------------------------------------------------------
//	Set an Environment Feature on or off

void
UEnvironment::SetFeature(
	EnvironmentFeature	inFeature,
	Boolean				inSetting)
{
	if (inSetting) {
		sFeatures |= inFeature;
	} else {
		sFeatures &= ~inFeature;
	}
}


// ---------------------------------------------------------------------------
//		* HasFeature
// ---------------------------------------------------------------------------
//	Return whether an Environment Feature is on or off

Boolean
UEnvironment::HasFeature(
	EnvironmentFeature	inFeature)
{
	return ((sFeatures & inFeature) == inFeature);
}


// ---------------------------------------------------------------------------
//		* HasGestaltAttribute
// ---------------------------------------------------------------------------
//	Return whether an attribute bit of a Gestalt selector is set
//
//	inSelector should be a Gestalt selector with the "Attr" suffix
//	inAttribute is the bit number in the Atrribute to check

Boolean
UEnvironment::HasGestaltAttribute(
	OSType	inSelector,
	Uint32	inAttribute)
{
	Int32	response;
	Boolean	haveAttribute;
	
	haveAttribute = ( (::Gestalt(inSelector, &response) == noErr)  &&
			 ((response & (1L << inAttribute)) != 0) );
	if( haveAttribute &&
		GENERATINGCFM && 
		( inSelector == gestaltDragMgrAttr ) )
	{
		// If we're generating CFM code and the selector is for
		// the drag manager,
		// check for weak import failure.
		// This is not likely in the PPC case,
		// but we've seen it happen in the CFM68K case.
		haveAttribute = ((UInt32) TrackDrag != kUnresolvedCFragSymbolAddress);
	}
	
	return( haveAttribute );
}
