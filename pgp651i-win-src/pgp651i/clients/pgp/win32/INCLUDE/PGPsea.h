/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: PGPsea.h,v 1.3 1999/03/10 02:55:49 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PGPSEA_h	/* [ */
#define Included_PGPSEA_h

typedef struct
{
	char szPGPSDA[6];
	DWORD offset;
	DWORD CompLength;
	DWORD NumFiles;

	// Preference file offsets and sizes
	DWORD AdminOffset;
	DWORD AdminSize;
	DWORD ClientOffset;
	DWORD ClientSize;
	DWORD NetOffset;
	DWORD NetSize;
	DWORD SetupOffset;
	DWORD SetupSize;
} SDAHEADER;

#endif /* ] Included_PGPSEA_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
