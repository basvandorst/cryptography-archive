/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	pgpTrayIPC.h - PGPtray inter-process communication header
	
	$Id: pgpTrayIPC.h,v 1.1 1999/02/25 21:21:42 pbj Exp $
____________________________________________________________________________*/

#ifndef _PGPTRAYIPC_H
#define _PGPTRAYIPC_H

// These are the names that the system tray app should use for its window
// class and title.  
#define PGPTRAY_WINDOWNAME		("PGPtray_Hidden_Window")

// applications can send the following messages to PGPtray

// sent by Options property sheet to disable hotkeys while the property
// sheet is displayed.  Otherwise the hotkey interpretation interferes with
// the setting of the hotkey combinations
#define PGPTRAY_M_DISABLEHOTKEYS			WM_APP+100

#endif //_PGPTRAYIPC_H