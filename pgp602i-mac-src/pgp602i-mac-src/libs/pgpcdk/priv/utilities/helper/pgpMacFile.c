/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: pgpMacFile.c,v 1.56.4.1.2.1 1998/11/12 03:23:22 heller Exp $____________________________________________________________________________*/#include "pgpConfig.h"#include <Devices.h>#include <Files.h>#include <Script.h>#include <errno.h>#include <stdio.h>#if PGP_MACINTOSH /* [ */#include <Errors.h>#include "MacStrings.h"#include "MacFiles.h"#include "pgpTypes.h"#include "pgpFile.h"#include "pgpMacFile.h"#include "pgpMacUtils.h"#include "pgpFileRef.h"#include "pgpFileSpec.h"#include "pgpFileMod.h"#include "pgpCFB.h"#include "pgpMem.h"#include "pgpErrors.h"#include "pgpUsuals.h"#include "pgpContext.h"#include "pgpMacFileMapping.h"/* XXX Is this stuff still necessary? */#define CHANGED		0x0200	/* ---- --1- ---- ---- */#define kFinderFlagsIgnore	(kIsOnDesk | kHasBeenInited | CHANGED )#if PRAGMA_ALIGN_SUPPORTED#pragma options align=mac68k#endiftypedef struct MacBinaryHeader{	uchar	filler;	uchar	oldVersion;			/* 000: Must be zero for compatibility */	uchar	name[64];			/* 001: Pascal string */	FInfo	info1;				/* 065: Original Finder info: */								/*		File type (long) */								/*		File creator (long) */								/*		Flags (word, low byte must be zero) */								/*		File's location (Point) */								/*		File's window (short) */	uchar	protectedBit;		/* 081: Low order bit */	uchar	zero1;				/* 082: Must be zero for compatibility */	long	dLength;			/* 083: Data fork length (in bytes) */	long	rLength;			/* 087: Resource fork length (in bytes) */	ulong	creationDate;		/* 091: Creation date */	ulong	modificationDate;	/* 095: Modification date */	short	getInfoLength;		/* 099:	Get info length */	uchar	info2;				/* 101: Finder flags, low byte */	uchar	dummy1;	uchar	dummy2[18];			/* 103: Not used */	uchar	dummy3;	uchar	newVersion;			/* 122: Uploading program version number */	uchar	minimumVersion;		/* 123: Minimum version number needed */	uchar	crc1;	uchar	crc2;	uchar	endFiller[2];		/* To pad out to 128 bytes */	DEBUG_STRUCT_CONSTRUCTOR( MacBinaryHeader )} MacBinaryHeader;#if PRAGMA_ALIGN_SUPPORTED#pragma options align=reset#endif/*____________________________________________________________________________	A MacFileTypeEntry maps a pgp file type to a mac creator/type pair.____________________________________________________________________________*/typedef struct MacFileTypeEntry{	PGPFileType		pgpFileType;	OSType			creator;	OSType			type;} MacFileTypeEntry;static MacFileTypeEntry	sMacFileTypeTable[kPGPFileTypeNumTypes] ={	{ kPGPFileTypePrefs,		kPGPMacFileCreator_Keys, kPGPMacFileType_Preferences },			{ kPGPFileTypePubRing,		kPGPMacFileCreator_Keys, kPGPMacFileType_PubRing },			{ kPGPFileTypePrivRing,		kPGPMacFileCreator_Keys, kPGPMacFileType_PrivRing },			{ kPGPFileTypeDetachedSig,		kPGPMacFileCreator_Tools, kPGPMacFileType_DetachedSig },			{ kPGPFileTypeRandomSeed,		kPGPMacFileCreator_Keys, kPGPMacFileType_RandomSeed },			{ kPGPFileTypeExportedKeys,		kPGPMacFileCreator_Keys, kPGPMacFileType_ExportedKeys },			{ kPGPFileTypeArmorFile,		kPGPMacFileCreator_Tools, kPGPMacFileType_ArmorFile },			{ kPGPFileTypeEncryptedData,		kPGPMacFileCreator_Tools, kPGPMacFileType_EncryptedData },	{ kPGPFileTypeGroups,		kPGPMacFileCreator_Keys, kPGPMacFileType_Groups },	/* XXX: The below should be user preferences */	/* but these are the defaults */	{ kPGPFileTypeDecryptedText,		kPGPMacFileCreator_DecryptedText, kPGPMacFileType_DecryptedText },			{ kPGPFileTypeDecryptedBin,		kPGPMacFileCreator_DecryptedBinary, kPGPMacFileType_DecryptedBinary },			{ kPGPFileTypeSignedData,		kPGPMacFileCreator_Tools, kPGPMacFileType_SignedData }};#define kNumMacFileTypeEntries	\	( sizeof( sMacFileTypeTable ) / sizeof( sMacFileTypeTable[ 0 ] ) )#define kPGPMacRead			0x01L#define kPGPMacWrite		0x02L#define kPGPMacBinMode		0x04L/* For write mode only: whether we should check */#define kPGPCheckMacBin		0x08L/* Strip out volatile information for detached sig */#define kPGPMacBinHashOnly	0x10L/* CRC 0 in MacBin header is okay */#define kPGPNoMacBinCRCOkay	0x20L/* This is the private data for macfiles */typedef struct MacFile{	ulong			flags;	short			dataRef, resRef;	long			dataOffset, resOffset, totalSize;	long			filePos;	PGPFileError	err;	PGPError		error;	PFLFileSpecRef		fileRef;	PGPFileType		fileType;	MacBinaryHeader	macBinHeader;	DEBUG_STRUCT_CONSTRUCTOR( MacFile )} MacFile;static PGPError PrepareToWrite(PGPFile *file, PGPBoolean autoMapMacTypes );static PGPError SetFileInfo( PGPFile *file);	static voidmacSetError(	PGPFile *		file,	PGPError		code){	MacFile *	mf = (MacFile *)file->priv;	pgpAssert( pgpContextIsValid( file->context ) );	mf->err.f = file;	mf->err.fpos = mf->filePos;	mf->error = mf->err.error = code;	mf->err.syserrno = 0;}	static OSErrmacFileReadFork(	PGPFile *		file,	short			refNum,	long			forkOffset,	long			forkEnd,	long *			sizeLeftPtr,	void **			ptrPtr){	MacFile *	mf = (MacFile *)file->priv;	long		chunkSize;	OSErr		err = noErr;	pgpAssert( pgpContextIsValid( file->context ) );	if (mf->filePos >= forkOffset &&		(chunkSize = pgpMin(*sizeLeftPtr, forkEnd - mf->filePos)) > 0)	{		UInt32	actualCount;		err	= FSReadAtOffset( refNum, mf->filePos - forkOffset,					chunkSize, *ptrPtr, &actualCount, FALSE );		mf->filePos		+= actualCount;		*(Ptr *)ptrPtr	+= actualCount;		*sizeLeftPtr	-= actualCount;		if ((mf->flags & kPGPMacBinMode) && err == eofErr)		{			/*			 * Clear any memory which was beyond the EOF.  This is expected			 * in MacBinary because each fork is padded to a multiple of			 * 128-bytes.			 */			chunkSize	-= actualCount;			pgpClearMemory((uchar *)*ptrPtr, chunkSize);			mf->filePos += chunkSize;			*(Ptr *)ptrPtr += chunkSize;			*sizeLeftPtr -= chunkSize;			err = noErr;		}		else if (err != noErr)			macSetError(file, kPGPError_FileOpFailed);	}	return err;}	static size_tmacFileRead(	void *			ptr,	size_t			size,	PGPFile *		file){	MacFile *	mf = (MacFile *)file->priv;	long		sizeLeft = (long)size;	long		chunkSize;	OSErr		result = noErr;	pgpAssert(mf->filePos >= 0);	if (!(mf->flags & kPGPMacRead))	{		macSetError(file, kPGPError_IllegalFileOp);		return 0;	}	/* Handle portion in the MacBinary header, if any */	chunkSize	= pgpMin(sizeLeft, mf->dataOffset - mf->filePos);	if ( chunkSize > 0 )	{		pgpCopyMemory( ((uchar *)&mf->macBinHeader) + 1 + mf->filePos,						ptr, chunkSize);		mf->filePos += chunkSize;		ptr = (void *)((Ptr)ptr + chunkSize);		sizeLeft -= chunkSize;	}	/* Handle portion in the data fork, if any */	if (result == noErr && sizeLeft > 0)		result = macFileReadFork(file, mf->dataRef, mf->dataOffset,								mf->resOffset, &sizeLeft, &ptr);	/*	 * Handle zeroing the first 128 bytes of the resource fork,	 * but only if kPGPMacBinHashOnly is set.	 */	chunkSize = pgpMin(sizeLeft,			pgpMin(mf->resOffset + 128, mf->totalSize) - mf->filePos);	if (result == noErr			&& (mf->flags & kPGPMacBinMode) && (mf->flags & kPGPMacBinHashOnly)			&& ( chunkSize > 0 ))	{		pgpClearMemory(ptr, chunkSize);		mf->filePos += chunkSize;		ptr = (void *)((Ptr)ptr + chunkSize);		sizeLeft -= chunkSize;	}	/* Handle portion in the resource fork, if any */	if (result == noErr && sizeLeft > 0 && (mf->flags & kPGPMacBinMode))		result = macFileReadFork(file, mf->resRef, mf->resOffset,								mf->totalSize, &sizeLeft, &ptr);	return (size_t)(size - sizeLeft);}	static OSErrmacFileWriteFork(	PGPFile *		file,	short			refNum,	long			forkOffset,	long			forkEnd,	/* Ignored unless kPGPMacBinMode flag set */	long			forkLen,	/* Ignored unless kPGPMacBinMode flag set */	long *			sizeLeftPtr,	void const **	ptrPtr){	MacFile *	mf = (MacFile *)file->priv;	long		chunkSize;	OSErr		result = noErr;	chunkSize = *sizeLeftPtr;	if (mf->flags & kPGPMacBinMode)		chunkSize = pgpMin(chunkSize, forkEnd - mf->filePos);	if (mf->filePos >= forkOffset && chunkSize > 0)	{		IOParam		pb;		pb.ioRefNum		= refNum;		pb.ioBuffer		= (Ptr)*ptrPtr;		pb.ioPosMode	= fsFromStart;		pb.ioPosOffset	= mf->filePos - forkOffset;		pb.ioReqCount	= chunkSize;		if (mf->flags & kPGPMacBinMode)			pb.ioReqCount = pgpMin(pb.ioReqCount, forkLen - pb.ioPosOffset);		result	= PBWriteSync((ParmBlkPtr)&pb);		mf->filePos		+= chunkSize;		*(Ptr *)ptrPtr	+= chunkSize;		*sizeLeftPtr	-= chunkSize;		if (result != noErr)			macSetError(file, kPGPError_FileOpFailed);	}	if (mf->totalSize < mf->filePos)		mf->totalSize = mf->filePos;	return result;}	static size_tmacFileWrite(	void const *	ptr,	size_t			size,	PGPFile *		file){	MacFile *	mf = (MacFile *)file->priv;	long		sizeLeft = (long)size;	long		chunkSize;	OSErr		macErr = noErr;	pgpAssert(mf->filePos >= 0);	pgpAssert( ( mf->flags & kPGPMacWrite) != 0 );	if (!(mf->flags & kPGPMacWrite))	{		macSetError(file, kPGPError_FileLocked);		return 0;	}	/* Handle portion in the MacBinary header, if any */	chunkSize	= pgpMin(sizeLeft, mf->dataOffset - mf->filePos);	if ( chunkSize > 0)	{		pgpAssert(mf->filePos + chunkSize <= 128);		pgpCopyMemory(ptr, ((uchar *)&mf->macBinHeader) + 1 + mf->filePos,						chunkSize);		mf->filePos += chunkSize;		ptr = (void *)((Ptr)ptr + chunkSize);		sizeLeft -= chunkSize;		if (mf->totalSize < mf->filePos)			mf->totalSize = mf->filePos;	}	/* Handle portion in the data fork, if any */	if (macErr == noErr && sizeLeft > 0)	{		if ( IsPGPError( PrepareToWrite(file, TRUE ) ) )			return( 0 );					macErr = macFileWriteFork(file, mf->dataRef, mf->dataOffset,								mf->resOffset, mf->macBinHeader.dLength,								&sizeLeft, &ptr);	}	/* Handle portion in the resource fork, if any */	if (macErr == noErr &&		sizeLeft > 0 &&		(mf->flags & kPGPMacBinMode))	{		macErr = macFileWriteFork(file, mf->resRef, mf->resOffset,								mf->totalSize, mf->macBinHeader.rLength,								&sizeLeft, &ptr);	}	return (size_t)(size - sizeLeft);}	static PGPErrormacFileFlush(	PGPFile *		file){	MacFile *	mf = (MacFile *)file->priv;	IOParam		pb;	pb.ioRefNum = mf->dataRef;	PBFlushFileSync((ParmBlkPtr)&pb);	pb.ioRefNum = mf->resRef;	PBFlushFileSync((ParmBlkPtr)&pb);	return kPGPError_NoErr;}	static PGPErrormacFileClose(	PGPFile *		file){	MacFile *	mf = (MacFile *)file->priv;	PGPContextRef	cdkContext;	pgpAssertAddrValid( file, PGPFile );	cdkContext	= file->context;	pgpAssert( pgpContextIsValid( cdkContext ) );	if ( ( mf->flags & kPGPMacWrite) != 0 )	{		PrepareToWrite(file, TRUE );	}		if (mf->dataRef > 0)		FSClose(mf->dataRef);	if (mf->resRef > 0)		FSClose(mf->resRef);	SetFileInfo(file);	PFLFreeFileSpec(mf->fileRef);	pgpClearMemory((void *)mf, sizeof(*mf));	pgpContextMemFree( cdkContext, mf);	pgpContextMemFree( cdkContext, file);	return kPGPError_NoErr;}	static longmacFileTell(	PGPFile *		file){	MacFile *	mf = (MacFile *)file->priv;	return mf->filePos;}	static PGPErrormacFileSeek(	PGPFile *		file,	long			offset,	int				whence){	MacFile *	mf = (MacFile *)file->priv;	switch (whence)	{		case SEEK_SET:			/* offset is correct, as is */			break;		case SEEK_CUR:			offset += mf->filePos;			break;		case SEEK_END:			offset += mf->totalSize;			break;	}	if (offset < 0 || offset > mf->totalSize)	{		macSetError(file, kPGPError_FileOpFailed);		return kPGPError_FileOpFailed;	}	mf->filePos = offset;	return kPGPError_NoErr;}	static PGPBooleanmacFileEof(	PGPFile const *	file){	MacFile *	mf = (MacFile *)file->priv;	/*	 * XXX This will always return TRUE when writing files.	 *     Is that semantically correct?	 */	return mf->filePos >= mf->totalSize;}	static longmacFileSizeAdvise(	PGPFile const *	file){	MacFile *	mf = (MacFile *)file->priv;	return mf->totalSize;}	static PGPFileError const *macFileError(	PGPFile const *	file){	MacFile *	mf = (MacFile *)file->priv;	if (mf->error != kPGPError_NoErr)		return &mf->err;	else		return NULL;}	static voidmacFileClearError(	PGPFile *		file){	MacFile *	mf = (MacFile *)file->priv;	mf->error = kPGPError_NoErr;}	static PGPErrormacFileWrite2Read(	PGPFile *		file){	macSetError(file, kPGPError_IllegalFileOp);	return kPGPError_IllegalFileOp;}	static PGPCFBContext *macFileCfb(	PGPFile const *	file){	(void)file;	/* Quiet compiler warning */	/* XXX: I'm not really sure what should be done here */	return NULL;}#ifndef __powercstatic ushort asm CalcCRC16Contin(ushort start, const void *data, long len){	fralloc +	move.l	D3, -(SP)	move.w	#0x1021, D3	clr.l	D0	move.w	start, D0	move.l	data, A0	move.l	len, D2	bra		Loop1Loop0:	move.b	(A0)+, D1	lsl.w	#8, D1	eor.w	D1, D0	moveq	#7, D1ShiftLoop0:	lsl.w	#1, D0	bcc		ShiftLoop1	eor.w	D3, D0ShiftLoop1:	dbra	D1, ShiftLoop0Loop1:	dbra	D2, Loop0	move.l	(SP)+, D3	frfree#if GENERATINGCFM || defined(__CFM68K__)	rtd		#12#else    rts#endif}static uchar asm CalcChecksum8Contin(uchar start, const void *data, long len){	FRALLOC	+	CLR.L	D0	MOVE.B	start, D0	MOVE.L	data, A0	MOVE.L	len, D1	BRA		Loop1Loop0:	ADD.B	(A0)+, D0Loop1:	DBRA	D1, Loop0	FRFREE#if GENERATINGCFM || defined(__CFM68K__)	rtd		#12#else    rts#endif}static ulong asm CalcCRC32(const void *data, long len){	fralloc	+	move.l	data, A0	clr.l	D0	move.l	len, D1	lsr.l	#2, D1	bra		Loop1Loop0:	rol.l	#1, D0	add.l	(A0)+, D0Loop1:	dbra	D1, Loop0	moveq	#3, D2	move.l	len, D1	and.l	D2, D1	lsl.w	D2, D1	clr.l	D2	not.l	D2	lsr.l	D1, D2	not.l	D2	and.l	(A0), D2	rol.l	#1, D0	add.l	D2, D0	frfree#if GENERATINGCFM || defined(__CFM68K__)	rtd		#8#else    rts#endif}#elsestatic ushort CalcCRC16Contin(ushort crc, const void *data, long len){	const char *dp=(const char *)data;	short i;	while(len--)	{		crc ^= (ushort)(*dp++) << 8;		for (i = 0; i < 8; ++i)		{			if (crc & 0x8000)				crc = (crc << 1) ^ 0x1021;			else				crc <<= 1;		}	}	return(crc);}static uchar CalcChecksum8Contin(uchar start, const void *data, long len){	const char *dp=(char *)data;	while(len--)		start+=*dp++;	return start;}static ulong CalcCRC32(const void *data, register long len){	const ulong *dp=(const ulong *)data;	ulong crc=0, x;	long len2;	ulong highSet;	len2=len;	len>>=2;	while(--len>=0)	{		highSet=(crc & 0x80000000);		crc<<=1;		if(highSet)			crc|=1;		crc+=*dp++;	}	x=3;	len2 &=x;	len2<<=x;	x=0;	x=~x;	x>>=len2;	x=~x;	x &= *dp;	highSet=(crc & 0x80000000);	crc<<=1;	if(highSet)		crc|=1;	crc+=x;	return crc;}#endifstatic uchar CalcChecksum8(const void *data, long len){	return CalcChecksum8Contin(0, data, len);}static ushort CalcCRC16(const void *data, long len){	return CalcCRC16Contin(0, data, len);}/* * This routine is called each time before writing to an actual fork.  If * the kPGPCheckMacBin flag is set, it determines whether or not it's a * valid MacBinary header.  If so, it creates the file and opens both the * data and resource forks.  Otherwise it creates and opens the data fork, * and writes false header information to the data fork, using whatever * hints it can find to create the file with the correct type and creator. */	static PGPErrorPrepareToWrite(	PGPFile *		file,	PGPBoolean		autoMapMacTypes ){	MacFile *		mf = (MacFile *)file->priv;	FSSpec			spec;	PGPError		err = kPGPError_NoErr;	OSErr			macResult;	long			length;	pgpAssert( (mf->flags & kPGPMacWrite) != 0 );	if (!(mf->flags & kPGPMacWrite))		return kPGPError_NoErr;	if (mf->flags & kPGPCheckMacBin)	{		mf->flags &= ~(kPGPMacBinMode | kPGPCheckMacBin);		if (mf->totalSize >= 126 && mf->macBinHeader.oldVersion == 0 &&					mf->macBinHeader.minimumVersion <= 129 &&					(mf->macBinHeader.info1.fdFlags & 0xFF) == 0 &&					mf->macBinHeader.zero1 == 0 &&					mf->macBinHeader.name[0] < 64)		{			ushort		crc;			pgpAssert(sizeof(crc) == 2);			pgpCopyMemory(&mf->macBinHeader.crc1, &crc, sizeof(crc));			if (crc == CalcCRC16((uchar *)&mf->macBinHeader + 1, 124)					|| ((mf->flags & kPGPNoMacBinCRCOkay) && crc == 0))			{				mf->flags 		|= kPGPMacBinMode;				file->dataType 	= kPGPFileDataType_Binary;			}		}	}	if (mf->dataRef != 0)		return kPGPError_NoErr;	if ((err = PFLGetFSSpecFromFileSpec(mf->fileRef, &spec)) != kPGPError_NoErr)		goto error;	if (mf->flags & kPGPMacBinMode)	{		/* XXX Reconsider using a different file type for this */		macResult = FSpCreate( &spec,			pgpGetMacFileCreatorFromPGPFileType( kPGPFileTypeDecryptedBin ),			pgpGetMacFileTypeFromPGPFileType( kPGPFileTypeDecryptedBin ),			smSystemScript);		if ( IsPGPError( macResult ) )			goto macError;		if ((macResult = FSpOpenDF(&spec, fsWrPerm, &mf->dataRef)) != noErr)			goto macError;		if ((macResult = FSpOpenRF(&spec, fsWrPerm, &mf->resRef)) != noErr)			goto macError;		mf->dataOffset = 128;		mf->resOffset = mf->dataOffset +						(mf->macBinHeader.dLength + 127) & ~127L;		mf->totalSize = mf->resOffset +						(mf->macBinHeader.rLength + 127) & ~127L;	}	else	{		OSType			creator;		OSType			type;		if ( autoMapMacTypes )		{			err	= pgpMapFileNameToMacCreatorType( spec.name,					&creator, &type );									if ( IsPGPError( err ) )			{				err	= pgpMapFileDataToMacCreatorType(							((uchar *)&mf->macBinHeader) + 1,							&creator, &type );			}		}		if ( IsPGPError( err ) || ! autoMapMacTypes )		{			/* if no mapping, use default types */			creator	= pgpGetMacFileCreatorFromPGPFileType( mf->fileType );			type	= pgpGetMacFileTypeFromPGPFileType( mf->fileType );			err		= noErr;		}		macResult = FSpCreate( &spec, creator,					type, smSystemScript);		if ( IsPGPError( macResult ) )			goto macError;		if ((macResult = FSpOpenDF(&spec, fsWrPerm, &mf->dataRef)) != noErr)			goto macError;		mf->dataOffset = mf->resOffset = 0;		length = mf->totalSize;		if ((macResult = FSWrite(mf->dataRef, &length,								(Ptr)&mf->macBinHeader + 1)) != noErr)			goto macError;	}	return kPGPError_NoErr;macError:	err = pgpErrorFromMacError(macResult, kPGPError_FileOpFailed);error:	macSetError(file, err);	return err;}/* * This routine is called after closing a MacBinary file for writing.  It * sets the finder information including type and creator. */	static PGPErrorSetFileInfo(	PGPFile *		file){	const MacFile *	mf = (MacFile *)file->priv;	FSSpec			spec;	CInfoPBRec		cpb;	FInfo			fInfo;	if (!(mf->flags & kPGPMacWrite) || !(mf->flags & kPGPMacBinMode))		return kPGPError_NoErr;	if (PFLGetFSSpecFromFileSpec(mf->fileRef, &spec) != kPGPError_NoErr)		goto error;	cpbCreationDate( &cpb )		= mf->macBinHeader.creationDate;	cpbModificationDate( &cpb )	= mf->macBinHeader.modificationDate;	fInfo				= mf->macBinHeader.info1;	fInfo.fdFlags 		&= 0xFF00;	fInfo.fdFlags 		|= mf->macBinHeader.info2;	fInfo.fdFlags		&= ~kFinderFlagsIgnore;	fInfo.fdLocation.h	= 0;	fInfo.fdLocation.v	= 0;	fInfo.fdFldr 		= 0;	cpbFInfo( &cpb )	= fInfo;	if ( FSpSetCatInfo( &spec, &cpb ) != noErr)		goto error;	return kPGPError_NoErr;error:	/* XXX Improve error reporting */	macSetError(file, kPGPError_FileOpFailed);	return kPGPError_FileOpFailed;}/* * It should be noted that writing to MacBinary files won't work if seeking * is used in certain ways.  The MacBinary header only interpreted the first * time data is written past the first 128 bytes, and then the fork lengths * and offsets are fixed and immutable.  This could be fixed at some point, * but there's no need for it now. * * The filename used to create the file is always the <fileRef> passed in, * not the name in the MacBinary header. */	PGPFile *pgpFileRefMacWriteOpen(	PGPContextRef			cdkContext,	PFLConstFileSpecRef		fileRef,	PGPFileType			fileType,	PGPFileOpenFlags	flags,	PGPError *			errorCode){	PGPFile *		file = NULL;	MacFile *		mf = NULL;	FSSpec			spec;	PGPError		err = kPGPError_NoErr;	PGPMemoryMgrRef	memoryMgr	= pgpGetFileRefMemoryMgr( fileRef );	if ((file = (PGPFile *)PGPNewData( memoryMgr,		sizeof(*file), kPGPMemoryMgrFlags_Clear)) == NULL)	{		err = kPGPError_OutOfMemory;		goto error;	}	file->context	= cdkContext;	file->dataType 	= kPGPFileDataType_Unknown;	if ((mf = (MacFile *)PGPNewData( memoryMgr,		sizeof(*mf), kPGPMemoryMgrFlags_Clear)) == NULL)	{		PGPFreeData( file );		err = kPGPError_OutOfMemory;		goto error;	}	err = PFLCopyFileSpec(fileRef, &mf->fileRef );	if ((err = PFLGetFSSpecFromFileSpec(mf->fileRef, &spec)) != kPGPError_NoErr)		goto error;	mf->fileType = fileType;	/* Try to delete the file first, in case it already exists */	/* XXX Maybe we should check the error, and/or truncate the file instead */	FSpDelete( &spec );	pgpAssert(sizeof(mf->macBinHeader) >= 129);	pgpAssert((flags & kPGPFileOpenLocalEncodeHashOnly) == 0);	mf->flags = kPGPMacWrite;	if (flags & (kPGPFileOpenMaybeLocalEncode | kPGPFileOpenForceLocalEncode))	{		mf->flags |= kPGPCheckMacBin;		if (flags & kPGPFileOpenNoMacBinCRCOkay)			mf->flags |= kPGPNoMacBinCRCOkay;		mf->dataOffset = mf->resOffset = 128;	}	else		mf->dataOffset = mf->resOffset = 0;	mf->dataRef = mf->resRef = 0;	mf->totalSize = 0;	mf->filePos = 0;	file->priv = mf;	file->read			= macFileRead;	file->write			= macFileWrite;	file->flush			= macFileFlush;	file->close			= macFileClose;	file->tell			= macFileTell;	file->seek			= macFileSeek;	file->eof			= macFileEof;	file->sizeAdvise	= macFileSizeAdvise;	file->error			= macFileError;	file->clearError	= macFileClearError;	file->write2read	= macFileWrite2Read;	file->cfb			= macFileCfb;	if ( !( mf->flags & kPGPCheckMacBin) )	{		if ((err = PrepareToWrite(file, FALSE )) != kPGPError_NoErr)			goto error;	}	return file;error:	if (mf != NULL)	{		if (mf->fileRef != NULL)			PFLFreeFileSpec(mf->fileRef);		pgpContextMemFree( cdkContext, mf);	}	if (file != NULL)		pgpContextMemFree( cdkContext, file);	if (errorCode != NULL)	{		pgpAssertAddrValid(errorCode, PGPError);		*errorCode = err;	}	return NULL;}	PGPFile *pgpFileRefMacReadOpen(	PGPContextRef	cdkContext,	PFLConstFileSpecRef		fileRef,	PGPFileOpenFlags	flags,	PGPError *			errorCode){	PGPFile *		file = NULL;	MacFile *		mf = NULL;	FSSpec			spec;	CInfoPBRec		cpb;	ushort			crc;	OSErr			macResult;	PGPError		result = kPGPError_NoErr;	pgpAssert( pgpContextIsValid( cdkContext ) );	if ((file = (PGPFile *)pgpContextMemAlloc( cdkContext,		sizeof(*file), kPGPMemoryMgrFlags_Clear)) == NULL)	{		result = kPGPError_OutOfMemory;		goto error;	}	file->context	= cdkContext;	file->dataType 	= kPGPFileDataType_Unknown;		if ((mf = (MacFile *)pgpContextMemAlloc( cdkContext,		sizeof(*mf), kPGPMemoryMgrFlags_Clear)) == NULL)	{		pgpContextMemFree( cdkContext, file );		result = kPGPError_OutOfMemory;		goto error;	}	result = PFLCopyFileSpec(fileRef, &mf->fileRef);	if ( result != noErr )		goto error;	result = PFLGetFSSpecFromFileSpec(mf->fileRef, &spec);	if ( IsPGPError( result ) )		goto error;	mf->fileType = kPGPFileTypeNone;	macResult	= FSpGetCatInfo( &spec, &cpb );	if ( macResult != noErr )		goto macError;	pgpAssert(sizeof(mf->macBinHeader) >= 129);	pgpClearMemory(&mf->macBinHeader, sizeof(mf->macBinHeader));	CopyPString(spec.name, mf->macBinHeader.name);	mf->macBinHeader.info2 = cpbFInfo( &cpb ).fdFlags & 0x00FF;	cpbFInfo( &cpb ).fdFlags &= 0xFF00;	cpbFInfo( &cpb ).fdLocation.h = 0;	cpbFInfo( &cpb ).fdLocation.v = 0;	cpbFInfo( &cpb ).fdFldr = 0;	mf->macBinHeader.info1				= cpbFInfo( &cpb );	mf->macBinHeader.dLength			= cpbDataForkSize( &cpb );	mf->macBinHeader.rLength			= cpbResForkSize( &cpb );	mf->macBinHeader.creationDate		= cpbCreationDate( &cpb );	mf->macBinHeader.modificationDate	= cpbModificationDate( &cpb );	mf->macBinHeader.newVersion			= 129;	mf->macBinHeader.minimumVersion		= 129;	crc = CalcCRC16((const uchar *)&mf->macBinHeader + 1, 124);	pgpAssert(sizeof(crc) == 2);	pgpCopyMemory(&crc, &mf->macBinHeader.crc1, sizeof(crc));	mf->flags = kPGPMacRead;	if ( flags & kPGPFileOpenForceLocalEncode)	{		mf->flags 		|= kPGPMacBinMode;		file->dataType 	= kPGPFileDataType_Binary;	}	else if (flags & kPGPFileOpenMaybeLocalEncode)	{		if ( pgpOKToEncodeFSSpecWithoutMacBinary( &spec ) )		{			mf->flags	&= ~ kPGPMacBinMode;		}		else		{			mf->flags		|= kPGPMacBinMode;			file->dataType 	= kPGPFileDataType_Binary;		}	}		if ((macResult = FSpOpenDF(&spec, fsRdPerm, &mf->dataRef)) != noErr)		goto macError;	if (mf->flags & kPGPMacBinMode)	{		if ((macResult = FSpOpenRF(&spec, fsRdPerm, &mf->resRef)) != noErr)			goto macError;		mf->dataOffset = 128;		mf->resOffset =			mf->dataOffset + (cpbDataForkSize( &cpb ) + 127) & ~127L;		mf->totalSize = mf->resOffset + (cpbResForkSize( &cpb ) + 127) & ~127L;		if (flags & kPGPFileOpenLocalEncodeHashOnly)		{			mf->flags |= kPGPMacBinHashOnly;			/* Clear the finder flags */			mf->macBinHeader.info1.fdFlags = 0;			mf->macBinHeader.info2 = 0;			/* Clear the creation and modification dates */			mf->macBinHeader.creationDate = 0;			mf->macBinHeader.modificationDate = 0;			/* Clear the filename */			pgpClearMemory(mf->macBinHeader.name,				sizeof(mf->macBinHeader.name));			/*			 * Clear the CRC so that if this output gets used to			 * recreate a file (which it shouldn't), it won't work			 * properly and someone will notice the problem.			 */			pgpClearMemory(&mf->macBinHeader.crc1, sizeof(crc));		}	}	else	{		mf->resRef = 0;		mf->dataOffset = 0;		mf->totalSize = mf->resOffset = cpbDataForkSize( &cpb );	}	mf->filePos = 0;	file->priv = mf;	file->read = macFileRead;	file->write = macFileWrite;	file->flush = macFileFlush;	file->close = macFileClose;	file->tell = macFileTell;	file->seek = macFileSeek;	file->eof = macFileEof;	file->sizeAdvise = macFileSizeAdvise;	file->error = macFileError;	file->clearError = macFileClearError;	file->write2read = macFileWrite2Read;	file->cfb = macFileCfb;	return file;macError:	result = pgpErrorFromMacError(macResult, kPGPError_CantOpenFile);error:	if (mf != NULL)	{		if (mf->fileRef != NULL)			PFLFreeFileSpec(mf->fileRef);		pgpContextMemFree( cdkContext, mf);	}	if (file != NULL)		pgpContextMemFree( cdkContext, file);	if (errorCode != NULL)	{		pgpAssertAddrValid(errorCode, PGPError);		*errorCode = result;	}	return NULL;}	PGPErrorpgpMacCalcFileSize(	PFLConstFileSpecRef		fileRef,	PGPFileOpenFlags	flags,	size_t *			fileSize){	FSSpec			spec;	CInfoPBRec		cpb;	PGPBoolean		macBinMode;	PGPError		result = kPGPError_NoErr;	*fileSize = 0;	/* In case there's an error */	if ((result = PFLGetFSSpecFromFileSpec(fileRef, &spec)) != kPGPError_NoErr)		return result;	if ( (result = FSpGetCatInfo( &spec, &cpb )) != noErr)		return pgpErrorFromMacError( result, kPGPError_CantOpenFile);	macBinMode = FALSE;	if ( flags & kPGPFileOpenForceLocalEncode )	{		macBinMode = TRUE;	}	else if (flags & kPGPFileOpenMaybeLocalEncode)	{		macBinMode	= ! pgpOKToEncodeFSSpecWithoutMacBinary( &spec );	}	if (macBinMode)	{		*fileSize = 128 +								/* MacBinary header */					((cpbDataForkSize( &cpb ) + 127) & ~127L) +	/* Data fork */					((cpbResForkSize( &cpb ) + 127) & ~127L);	/* Res fork */	}	else	{		*fileSize = cpbDataForkSize( &cpb );	}	return kPGPError_NoErr;}	static MacFileTypeEntry const *sFindMacFileTypeEntry( PGPFileType type ){	PGPUInt16					index;	MacFileTypeEntry const *	entry	= NULL;		entry	= &sMacFileTypeTable[ 0 ];	for( index = 0; index < kNumMacFileTypeEntries; ++index, ++entry )	{		if ( entry->pgpFileType == type )		{			return( entry );		}	}		pgpDebugMsg( "can't find entry for PGPFileType" );	return( NULL );}	OSTypepgpGetMacFileTypeFromPGPFileType( PGPFileType pgpType ){	MacFileTypeEntry const *	entry	= NULL;		entry	= sFindMacFileTypeEntry( pgpType );	if ( IsntNull( entry ) )		return( entry->type );	return( 0 );}	OSTypepgpGetMacFileCreatorFromPGPFileType( PGPFileType pgpType ){	MacFileTypeEntry const *	entry	= NULL;		entry	= sFindMacFileTypeEntry( pgpType );	if ( IsntNull( entry ) )		return( entry->creator );	return( 0 );}#endif	/* ] PGP_MACINTOSH *//* * Local Variables: * tab-width: 4 * End: * vi: ts=4 sw=4 * vim: si */