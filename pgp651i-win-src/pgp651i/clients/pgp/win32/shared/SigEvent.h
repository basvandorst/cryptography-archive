/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: SigEvent.h,v 1.5 1999/03/10 03:03:20 heller Exp $
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
