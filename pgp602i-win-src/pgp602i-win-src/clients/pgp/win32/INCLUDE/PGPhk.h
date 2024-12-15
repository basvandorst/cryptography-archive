/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPsc.h - include file for PGPsc DLL
	

	$Id: PGPhk.h,v 1.1.2.1 1998/10/06 18:17:11 wjb Exp $
____________________________________________________________________________*/
#ifndef _PGPHKH
#define _PGPHKH

#ifdef __cplusplus
extern "C" {
#endif

// Use Current Window routines for PGPtray's CBThook
HWND ReadCurrentFocus(void);
void SetHook(void);
void DeleteHook(void);

#ifdef __cplusplus
}
#endif

#endif // PGPHKH
