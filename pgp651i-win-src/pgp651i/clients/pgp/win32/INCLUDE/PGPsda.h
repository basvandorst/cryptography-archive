/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: PGPsda.h,v 1.2 1999/03/10 02:55:29 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PGPSDA_h	/* [ */
#define Included_PGPSDA_h

typedef union PassphraseSalt
	{
	PGPUInt8	saltBytes[ 8 ];
	PGPUInt32	saltLongs[ 8 / sizeof( PGPUInt32 ) ];
	} PassphraseSalt;

typedef struct
{
	char szPGPSDA[6];
	DWORD offset;
	DWORD CompLength;
	DWORD NumFiles;

	PassphraseSalt Salt;
	PGPUInt16 hashReps;
	char CheckBytes[8];
} SDAHEADER;

#endif /* ] Included_PGPSDA_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
