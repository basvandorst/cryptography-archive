/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pflPrefs.h,v 1.13 1999/03/10 02:46:38 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pflPrefs_h	/* [ */
#define Included_pflPrefs_h

#include "pgpPFLConfig.h"
#include "pgpBase.h"
#include "pflTypes.h"

typedef struct PGPPref *	PGPPrefRef;
typedef PGPInt32			PGPPrefIndex;

typedef enum PGPPrefType
{
	kPGPPrefType_Boolean = 0,
	kPGPPrefType_Number = 1,
	kPGPPrefType_String = 2,
	kPGPPrefType_Byte = 3
} PGPPrefType;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

typedef struct PGPPrefDefault
{
	PGPPrefIndex	index;
	PGPPrefType		type;
	void *			data;
	PGPSize			size;
} PGPPrefDefault;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#define	kInvalidPGPPrefRef			((PGPPrefRef) NULL)
#define PGPPrefRefIsValid( ref )	( (ref) != kInvalidPGPPrefRef )

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


PGPError PGPOpenPrefFile(PFLFileSpecRef prefFileSpec, 
							const PGPPrefDefault *defaults,
							PGPUInt32 numDefaults,
							PGPPrefRef *prefRef);

PGPError PGPSavePrefFile(PGPPrefRef prefRef);

PGPError PGPClosePrefFile(PGPPrefRef prefRef);

PGPError PGPGetPrefFileSpec(PGPPrefRef prefRef,
								PFLFileSpecRef *prefFileSpec);


PGPError PGPGetPrefData(PGPPrefRef prefRef,
							 PGPPrefIndex prefIndex, 
							 PGPSize *dataLength, 
							 void **inBuffer);

PGPError PGPSetPrefData(PGPPrefRef prefRef, 
							 PGPPrefIndex prefIndex, 
							 PGPSize dataLength, 
							 const void *outBuffer);

PGPError PGPRemovePref(PGPPrefRef prefRef, 
							PGPPrefIndex prefIndex);

PGPError PGPDisposePrefData(PGPPrefRef prefRef, 
								 void *dataBuffer);

PGPError PGPExportPrefFileToBuffer(PGPPrefRef prefRef, PGPSize *bufferSize,
										void **buffer);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pflPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
