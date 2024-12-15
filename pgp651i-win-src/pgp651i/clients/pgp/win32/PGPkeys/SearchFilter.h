/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: SearchFilter.h,v 1.3 1998/08/11 15:20:19 pbj Exp $
____________________________________________________________________________*/
#ifndef _SEARCHFILTER_H
#define _SEARCHFILTER_H

#define ACTION_UNION		1
#define ACTION_INTERSECT	2
#define ACTION_NEGATE		3

PGPError CreateFilter(HWND hwnd, PGPFilterRef* filter, int* action);

#endif