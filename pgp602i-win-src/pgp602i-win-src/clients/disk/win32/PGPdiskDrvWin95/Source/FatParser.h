//////////////////////////////////////////////////////////////////////////////
// FatParser.h
//
// Declaration of class FatParser.
//////////////////////////////////////////////////////////////////////////////

// $Id: FatParser.h,v 1.1.2.7 1998/07/06 08:58:18 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_FatParser_h	// [
#define Included_FatParser_h

#include "DualErr.h"
#include "GenericCallback.h"

#include "FileSystemParser.h"


////////
// Types
////////

enum FatParserOp {kFPOP_Read, kFPOP_Write};

// A FileFrag describes a FAT file fragment on disk. 'fragBegin' and 'fragEnd'
// are expressed in bytes, absolute from beginning of file. 'diskBegin' and
// 'diskEnd' describe this same portion, also in bytes, but absolute from
// beginning of the volume itself.

typedef struct FileFrag
{
	PGPUInt64	fileBegin;
	PGPUInt64	fileEnd;
	PGPUInt64	diskBegin;
	PGPUInt64	diskEnd;

	struct FileFrag	*prev;
	struct FileFrag	*next;

} FileFrag, *PFileFrag;

// FPReadInfo holds information about read requests called down to a class
// FatParser object.

typedef struct FPReadInfo
{
	PGPUInt8	*bufHeader;			// output buffer for header data
	PGPUInt64	posHeader;			// transfer position for header
	PGPUInt32	nBytesHeader;		// size of header
	PGPBoolean	readHeader;			// done with header?

	PGPUInt8	*bufMiddle;			// output buffer for middle data
	PGPUInt64	posMiddle;			// transfer position for middle
	PGPUInt32	nBytesMiddle;		// size of middle left to read
	PGPUInt32	sizeCurrentRead;	// block size of chunk being read
	PGPBoolean	readMiddle;			// done with middle?

	PGPUInt8	*bufTail;			// output buffer for tail data
	PGPUInt64	posTail;			// transfer position for tail
	PGPUInt32	nBytesTail;			// size of tail
	PGPBoolean	readTail;			// done with tail?

} FPReadInfo;

// FPWriteInfo holds information about write requests called down to a class
// PGPdisk object.

typedef struct FPWriteInfo
{
	PGPUInt8	*bufHeader;			// output buffer for header data
	PGPUInt64	posHeader;			// transfer position for header
	PGPUInt32	nBytesHeader;		// size of header
	PGPBoolean	readHeader;			// done with header read?
	PGPBoolean	wroteHeader;		// done with header write?

	PGPUInt8	*bufMiddle;			// output buffer for middle data
	PGPUInt64	posMiddle;			// transfer position for middle
	PGPUInt32	nBytesMiddle;		// size of middle left to write
	PGPUInt32	sizeCurrentWrite;	// block size of chunk being written
	PGPBoolean	wroteMiddle;		// done with middle?

	PGPUInt8	*bufTail;			// output buffer for tail data
	PGPUInt64	posTail;			// transfer position for tail
	PGPUInt32	nBytesTail;			// size of tail
	PGPBoolean	readTail;			// done with tail read?
	PGPBoolean	wroteTail;			// done with tail write?

} FPWriteInfo;

// FPRequestInfo holds either a read or a write request.

typedef struct FPRequestInfo
{
	PGPBoolean			isInUse;	// this object in use?
	FatParserOp			op;			// what kind of request?

	GenericCallbackInfo	downInfo;	// info for call down
	GenericCallbackInfo	*upInfo;	// info for call up

	FPReadInfo			read;		// info for read requests
	FPWriteInfo			write;		// info for write requests

} FPRequestInfo;

// ParseNextClusterInfo is used to pass information between MakeFatFrags and
// ParseNextCluster.

typedef struct ParseNextClusterInfo
{
	PGPUInt16	blockSize;
	PGPUInt16	clustBias;
	PGPUInt16	spc;

	PGPUInt32	bitsClust;
	PGPUInt32	bitOffset;
	PGPUInt32	clustSec;
	PGPUInt32	curClust;
	PGPUInt32	firstSecData;
	PGPUInt32	firstSecFat;
	PGPUInt32	i;
	PGPUInt32	nextClust;
	PGPUInt32	overFlow;
	PGPUInt32	totalClusts;

} ParseNextClusterInfo;


//////////////////
// Class FatParser
//////////////////

// FatParser is a file-system parser for FAT drives. This class is Ring-0
// thread-safe.

class FatParser : public FileSystemParser
{
public:
	DualErr	mInitErr;

			FatParser(LPCSTR path,  PGPUInt8 hostDrive, PGPUInt32 bytesFile);
	virtual	~FatParser();

	void	ReadAsync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
				GenericCallbackInfo *upInfo);
	void	WriteAsync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
				GenericCallbackInfo *upInfo);

	DualErr	ReadSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr	WriteSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);

	DualErr	Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
				GenericCallbackInfo *upInfo = NULL);
	DualErr	Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
				GenericCallbackInfo *upInfo = NULL);

private:
	FatData			mFat;			// FAT-specific data regarding the file
	PGPUInt8		*mDataBuf;		// block buffer

	PGPUInt32		mNumFragsInUse;	// number of frags in the array in use
	PGPUInt32		mSizeFragArray;	// size of 'mFragArray'
	PFileFrag		mFragArray;		// array of file fragments on disk

	FPRequestInfo	mFatParserReq;	// request information

	PGPUInt64				CalcDiskSector(PGPUInt64 filePos, PFileFrag pFF, 
								PGPUInt16 blockSize);

	DualErr					ExpandFragArray();
	PGPUInt32				FindFragmentIndex(PGPUInt64 pos);
	void					InitFileFrag(PFileFrag pFF, PGPUInt64 fileByte, 
								PGPUInt64 diskByte);

#if PGP_DEBUG
	void					DumpFrags();
#endif	// PGP_DEBUG

	DualErr					GetFatData();

	DualErr					ParseNextCluster(ParseNextClusterInfo *pPNCI);
	DualErr					MakeFatFrags();

	void					ExecuteRequest(PGPBoolean *doneWithRequest);

	static void				FatParserCallback(GenericCallbackInfo *downInfo);
	void					FatParserCallbackAux();

	void					ScheduleAsyncCallback(
								DualErr derr = DualErr::NoError);
	static VOID __stdcall	AsyncExecuteCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};

#endif	// ] Included_FatParser_h
