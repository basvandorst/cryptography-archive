/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: SearchProc.h,v 1.2 1998/08/11 15:20:21 pbj Exp $
____________________________________________________________________________*/
#ifndef _SEARCHPROC_H
#define _SEARCHPROC_H


// the message handler for the search control
BOOL 
CALLBACK 
SearchProc(	HWND hwnd, 
			UINT msg, 
			WPARAM wParam, 
			LPARAM lParam );
#endif