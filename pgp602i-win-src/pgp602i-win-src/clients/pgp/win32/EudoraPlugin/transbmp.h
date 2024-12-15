/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	transbmp.h,v 1.3 1997/10/22 23:06:03 elrod Exp
____________________________________________________________________________*/

#ifndef Included_TransBMP_h	/* [ */
#define Included_TransBMP_h

#ifdef __cplusplus
extern "C" {
#endif

BOOL TransparentBitmap(HBITMAP hBitmap, 
					   int iXsize, 
					   int iYsize, 
                       DWORD rgbTransparent, 
					   HDC hDC, 
					   int iXoffset,
					   int iYoffset);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_TransBMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
