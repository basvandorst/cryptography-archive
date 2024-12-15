/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CWindowRefArray.cp,v 1.2.10.1 1997/12/05 22:15:11 mhw Exp $
____________________________________________________________________________*/

#include "CWindowRefArray.h"



CWindowRefArray::CWindowRefArray()
	: LArray(sizeof(SWindowRef))
{
}



CWindowRefArray::~CWindowRefArray()
{
}



	ArrayIndexT
CWindowRefArray::FetchIndexOfWindowRef(
	const WindowRef	inWindowRef)
{
	SWindowRef	theItem = {inWindowRef, nil};
	
	SetComparator(new CWindowRefComparator);
	return FetchIndexOf(&theItem, sizeof(SWindowRef));
}



	ArrayIndexT
CWindowRefArray::FetchIndexOfGrafPortView(
	const LGrafPortView *	inGrafPortView)
{
	SWindowRef	theItem = {nil, const_cast<LGrafPortView *>(inGrafPortView)};
	
	SetComparator(new CGrafPortViewComparator);
	return FetchIndexOf(&theItem, sizeof(SWindowRef));
}
