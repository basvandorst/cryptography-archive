/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpMacBinary.h,v 1.3 1997/09/18 01:36:01 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpMacBinary_h	/* [ */
#define Included_pgpMacBinary_h


/*____________________________________________________________________________
	'MacBinaryHeader' is defined once for Macintosh and once for other
	platforms.  Because compilers may pad structs differently, only the Mac
	version is an accurate on-disk description.
____________________________________________________________________________*/


#if PGP_MACINTOSH /* [ */

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

typedef struct MacBinaryHeader
{
	uchar	NOT_PART_OF_STRUCT;	/* NOT part of the struct; used to pad */
	uchar	oldVersion;			/* 000: Must be zero for compatibility */
	uchar	name[64];			/* 001: Pascal string */
	FInfo	info1;				/* 065: Original Finder info: */
	uchar	protectedBit;		/* 081: Low order bit */
	uchar	zero1;				/* 082: Must be zero for compatibility */
	long	dLength;			/* 083: Data fork length (in bytes) */
	long	rLength;			/* 087: Resource fork length (in bytes) */
	ulong	creationDate;		/* 091: Creation date */
	ulong	modificationDate;	/* 095: Modification date */
	short	getInfoLength;		/* 099:	Get info length */
	uchar	info2;				/* 101: Finder flags, low byte */
	uchar	dummy1;
	uchar	dummy2[18];			/* 103: Not used */
	uchar	dummy3;
	uchar	newVersion;			/* 122: Uploading program version number */
	uchar	minimumVersion;		/* 123: Minimum version number needed */
	uchar	crc1;
	uchar	crc2;
	uchar	endFiller[2];		/* To pad out to 128 bytes */
} MacBinaryHeader;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


#else	/* ][ */

/* definitions for non-Mac */
typedef struct FInfo
{
	PGPUInt32		fdType;
	PGPUInt32		fdCreator;
	PGPUInt16		fdFlags;
	struct
	{
		PGPInt16	v;
		PGPInt16	h;
	} fdLocation;
	PGPInt16		fdFldr;
} FInfo;

typedef struct MacBinaryHeader
{
	/* NOT part of the struct; used to pad */
	PGPUInt8	NOT_PART_OF_STRUCT;		
	PGPUInt8	oldVersion;	
	PGPUInt8	name[64];
	FInfo		info1;
	PGPUInt8	protectedBit;	
	PGPUInt8	zero1;
	PGPInt32	dLength;
	PGPInt32	rLength;
	PGPUInt32	creationDate;
	PGPUInt32	modificationDate;
	PGPInt16	getInfoLength;
	PGPUInt8	info2;
	PGPUInt8	dummy1;
	PGPUInt8	dummy2[18];
	PGPUInt8	dummy3;
	PGPUInt8	newVersion;
	PGPUInt8	minimumVersion;
	PGPUInt8	crc1;
	PGPUInt8	crc2;
	PGPUInt8	endFiller[2];
} MacBinaryHeader;


#endif	/* ] */



#define kMacBinaryOffset_Version				0
#define kMacBinaryOffset_NameLength				1
#define kMacBinaryOffset_Name					2
#define kMacBinaryOffset_FInfo					65
#define kMacBinaryOffset_FileType				kMacBinaryOffset_FInfo
#define kMacBinaryOffset_FileCreator			69
#define kMacBinaryOffset_FinderFlags			73
#define kMacBinaryOffset_VPos					75
#define kMacBinaryOffset_HPos					77
#define kMacBinaryOffset_Folder					79
#define kMacBinaryOffset_ProtectedBit			81
#define kMacBinaryOffset_Zero1					82
#define kMacBinaryOffset_DataForkLength			83
#define kMacBinaryOffset_ResForkLength			87
#define kMacBinaryOffset_CreationDate			91
#define kMacBinaryOffset_ModificationDate		95
#define kMacBinaryOffset_Zero2					99
#define kMacBinaryOffset_crc1					124
#define kMacBinaryOffset_crc2					125
#define kMacBinaryOffset_EndFiller				126

#define kPGPMacBinaryHeaderSize		128




PGPError	pgpMacBinaryToLocal( PFLFileSpecRef inPGPSpec,
				PFLFileSpecRef * outPGPSpec,
				PGPUInt32 * macCreator, PGPUInt32 * macTypeCode );
	
	
#endif /* ] Included_pgpMacBinary_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

