/*____________________________________________________________________________
	pgpUtilities.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains miscellaneous utility functions needed for the PGPsdk.

	$Id: pgpUtilities.h,v 1.54.4.1 1997/11/08 00:32:48 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUtilities_h	/* [ */
#define Included_pgpUtilities_h

#if PGP_MACINTOSH	/* [ */
#include <Files.h>
#endif	/* ] PGP_MACINTOSH */

#include "pgpPubTypes.h"

#include <time.h>


#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k	/* [ */
#endif


#if PGP_MACINTOSH	/* [ */

/* these types are used as strings in pgpMacFileMapping.h */
#define kPGPMacFileCreator_Keys				'pgpK'
#define kPGPMacFileCreator_DecryptedBinary	'????'
#define kPGPMacFileCreator_DecryptedText	'ttxt'
#define kPGPMacFileCreator_Tools			'pgpM'

#define kPGPMacFileTypeArmorFile		'TEXT'
#define kPGPMacFileTypeEncryptedData	'pgEF'
#define kPGPMacFileTypeSignedData		'pgSF'
#define kPGPMacFileTypeDetachedSig		'pgDS'
#define kPGPMacFileTypeRandomSeed		'pgRS'
#define kPGPMacFileTypePrivRing			'pgRR'
#define kPGPMacFileTypePubRing			'pgPR'
#define kPGPMacFileTypePref				'pref'
#define kPGPMacFileTypeExportedKeys		'TEXT'
#define kPGPMacFileTypeDecryptedText	'TEXT'
#define kPGPMacFileTypeDecryptedBinary	'BINA'

#endif	/* ] PGP_MACINTOSH */

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*____________________________________________________________________________
	Context routines
____________________________________________________________________________*/

enum 
{
	kPGPMemoryFlags_Clear = 1
};

typedef PGPFlags	PGPMemoryFlags;


typedef	void	*(*PGPMemoryAllocationProc)( PGPContextRef context,
						PGPSize allocationSize, PGPMemoryFlags flags,
						PGPUserValue userValue );

typedef	PGPError (*PGPMemoryReallocationProc)( PGPContextRef context,
						void **allocation, PGPSize newAllocationSize,
						PGPMemoryFlags flags, PGPUserValue userValue );

typedef	PGPError (*PGPMemoryDeallocationProc)( PGPContextRef context,
						void *allocation, PGPUserValue userValue );


/*____________________________________________________________________________
	API version:
	Top byte is major, next 3 nibbles minor, next 2 bug fix,
	last nibble reserved:  0xMMmmmrrR
	
	example: 1.0.0 = 0x01000000
	
	0x01000000		SDK 1.0.0
	0x02000000		SDK 1.1.0
	0x02000010		SDK 1.1.1
____________________________________________________________________________*/
#define kPGPsdkAPIVersion		( (PGPUInt32)0x02000010 )
#define PGPMajorVersion( v )	( ( ((PGPUInt32)(v)) & 0xFF000000 ) >> 24 )
#define PGPMinorVersion( v )	( ( ((PGPUInt32)(v)) & 0x00FFF000 ) >> 16 )
#define PGPRevVersion( v )		( ( ((PGPUInt32)(v)) & 0x00000FF0 ) >> 4 )

typedef struct PGPNewContextStruct
{
	/* sizeofStruct must be inited to sizeof( PGPNewContextStruct ) */
	PGPUInt32						sizeofStruct;
	PGPMemoryAllocationProc			allocProc;
	PGPMemoryReallocationProc		reallocProc;
	PGPMemoryDeallocationProc		deallocProc;
	PGPUserValue					allocUserValue;
} PGPNewContextStruct;



/* always pass kPGPsdkVersion */
PGPError 	PGPNewContext( PGPUInt32 clientAPIVersion,
				PGPContextRef *newContext );
PGPError 	PGPNewContextCustom( PGPUInt32 clientAPIVersion,
				PGPNewContextStruct	const *custom,
				PGPContextRef *newContext );

PGPError 	PGPFreeContext( PGPContextRef context );

PGPError 	PGPGetContextUserValue( PGPContextRef context,
						PGPUserValue *userValue );
PGPError 	PGPSetContextUserValue( PGPContextRef context,
						PGPUserValue userValue );

PGPError	PGPContextGetRandomBytes(PGPContextRef context,
						void *buf, PGPSize len );


/*____________________________________________________________________________
	Memory allocation
	
	The PGPsdk uses PGPNewData() for allocated return parameters.
	Clients should use PGPFreeData() to deallocate memory allocated with
	PGPNewData().
	
	PGPNewData should generally not be used by clients (there are a few
	special cases).
	
	PGPNewSecureData is intended to store sensitive data such as passphrases.
____________________________________________________________________________*/

void  * 	PGPNewData( PGPContextRef context, PGPSize allocationSize );

/* 'didLock' is TRUE if the memory is guaranteed to not be paged */
/* Some OSes don't support this feature */
void  * 	PGPNewSecureData( PGPContextRef context,
				PGPSize allocationSize, PGPBoolean *didLock);
				
/* can free allocation from PGPNewData or PGPNewSecureData */
void 		PGPFreeData( void * allocation );


/*____________________________________________________________________________
	FileRefs
____________________________________________________________________________*/

PGPError 		PGPCopyFileSpec(PGPConstFileSpecRef fileRef,
					PGPFileSpecRef *ref);
PGPError 		PGPFreeFileSpec(PGPFileSpecRef fileRef);

#if PGP_MACINTOSH	/* [ */

PGPError 		PGPNewFileSpecFromFSSpec(PGPContextRef context,
						const FSSpec *spec, PGPFileSpecRef *ref);
PGPError 		PGPGetFSSpecFromFileSpec(PGPConstFileSpecRef fileRef,
						FSSpec *spec);
#else

PGPError 		PGPNewFileSpecFromFullPath( PGPContextRef context,
						char const *path, PGPFileSpecRef *ref);

/* *fullPathPtr ***must*** be freed with PGPFreeData */
PGPError 		PGPGetFullPathFromFileSpec( PGPConstFileSpecRef	fileRef,
						char ** fullPathPtr);
						
#endif	/* ] PGP_MACINTOSH */


						
/*____________________________________________________________________________
	Time
____________________________________________________________________________*/
PGPTime 			PGPGetTime(void);

/* these use time_t type as returned by time() in Std C libraries */
time_t 			PGPGetStdTimeFromPGPTime(PGPTime theTime);
PGPTime 		PGPGetPGPTimeFromStdTime(time_t theTime);


/* year, month, day may be NULL if desired */
void 			PGPGetYMDFromPGPTime( PGPTime theTime,
						PGPUInt16 *year, PGPUInt16 *month, PGPUInt16 *day );


#if PGP_MACINTOSH	/* [ */
PGPUInt32 		PGPTimeToMacTime(PGPTime theTime);
PGPTime 		PGPTimeFromMacTime(PGPUInt32 theTime);
#endif	/* ] PGP_MACINTOSH */



/*____________________________________________________________________________
	MacBinary support
____________________________________________________________________________*/


/*____________________________________________________________________________
	Examine the input file to see if it's a MacBinary file.  If it is
	not a MacBinary file, then the original file is unaltered.
	Otherwise, the file is converted and the original file is deleted.
	
	The resulting file is designated by 'outPGPSpec' and may have a different
	name than the original.
	
	If the file is a TEXT file, appropriate line-end conversion is done.
	
	creator and type code pointers may be
	null but otherwise contain the mac creator and type.
	
	This routine can be called on a Mac, but generally doesn't need to be.
____________________________________________________________________________*/
PGPError	PGPMacBinaryToLocal( PGPFileSpecRef inSpec,
				PGPFileSpecRef * outSpec,
				PGPUInt32 * macCreator, PGPUInt32 * macTypeCode );




/*____________________________________________________________________________
	Call PGPsdkInit() before using the SDK.  Call PGPsdkCleanup() when you are
	done (but after disposing of any PGPContexts).
	
	You may call multiple times (with no effect), but be sure to call
	PGPsdkCleanup() for each call to PGPsdkInit().
____________________________________________________________________________*/
PGPError	PGPsdkInit( void );
PGPError	PGPsdkCleanup( void );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset			/* ] */
#endif

#endif /* ] Included_pgpUtilities_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
