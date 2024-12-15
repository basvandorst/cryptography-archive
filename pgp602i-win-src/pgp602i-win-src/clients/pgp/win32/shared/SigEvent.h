/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: SigEvent.h,v 1.4.8.1 1998/11/12 03:14:14 heller Exp $
____________________________________________________________________________*/
#ifndef Included_SIGEVENT_h	/* [ */
#define Included_SIGEVENT_h


#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

void SigEvent(HWND hwnd,PGPContextRef context,
			  PGPEventSignatureData *d,char *fileName);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif /* ] Included_SIGEVENT_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
