//////////////////////////////////////////////////////////////////////////////
// PGPdisk.h
//
// Declaration of class PGPdisk.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdisk.h,v 1.1.2.10 1998/07/06 08:58:25 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdisk_h	// [
#define Included_PGPdisk_h

#include "DualErr.h"
#include "GenericCallback.h"
#include "PGPdiskFileFormat.h"
#include "SecureMemory.h"

#include "CipherContext.h"
#include "VolFile.h"


////////////
// Constants
////////////

const PGPUInt32 kPGDBlocksPerOp = 8;	// cipher this many blocks at a time


////////
// Types
////////

enum PGPdiskOp {kPGDOP_Read, kPGDOP_Write};

// PGPdiskReadInfo holds information about read requests called down
// to a class PGPdisk object.

typedef struct PGPdiskReadInfo
{
	PGPUInt8	*bufHeader;			// output buffer for header data
	PGPUInt64	posHeader;			// transfer position for header
	PGPUInt32	nBytesHeader;		// size of header
	PGPBoolean	readHeader;			// done with header?

	PGPUInt8	*bufMiddle;			// output buffer for middle data
	PGPUInt64	posMiddle;			// transfer position for middle
	PGPUInt32	nBytesMiddle;		// size of middle left to read
	PGPBoolean	readMiddle;			// done with middle?

	PGPUInt8	*bufTail;			// output buffer for tail data
	PGPUInt64	posTail;			// transfer position for tail
	PGPUInt32	nBytesTail;			// size of tail
	PGPBoolean	readTail;			// done with tail?

} PGPdiskReadInfo;

// PGPdiskWriteInfo holds information about write requests called down
// to a class PGPdisk object.

typedef struct PGPdiskWriteInfo
{
	PGPUInt8	*bufHeader;			// output buffer for header data
	PGPUInt64	posHeader;			// transfer position for header
	PGPUInt32	nBytesHeader;		// size of header
	PGPBoolean	readHeader;			// done with header read?
	PGPBoolean	wroteHeader;		// done with header write?

	PGPUInt8	*bufMiddle;			// output buffer for middle data
	PGPUInt64	posMiddle;			// transfer position for middle
	PGPUInt32	nBytesMiddle;		// size of middle left to write
	PGPBoolean	wroteMiddle;		// done with middle?

	PGPUInt8	*bufTail;			// output buffer for tail data
	PGPUInt64	posTail;			// transfer position for tail
	PGPUInt32	nBytesTail;			// size of tail
	PGPBoolean	readTail;			// done with tail read?
	PGPBoolean	wroteTail;			// done with tail write?

} PGPdiskWriteInfo;

// PGPdiskRequestInfo holds either a read or a write request.

typedef struct PGPdiskRequestInfo
{
	PGPBoolean			isInUse;	// this object in use?
	PGPdiskOp			op;			// what kind of request?

	GenericCallbackInfo	downInfo;	// info for call down
	GenericCallbackInfo	*upInfo;	// info for call up

	PGPdiskReadInfo		read;		// info for read requests
	PGPdiskWriteInfo	write;		// info for write requests

} PGPdiskRequestInfo;


////////////////
// Class PGPdisk
////////////////

// Class PGPdisk represents a PGPdisk file. In the future it will handle
// encryption and decryption of the data area but for now it just handles
// parsing of the file header.

class PGPdisk : public VolFile
{
public:
	DualErr	mInitErr;

				PGPdisk();
				~PGPdisk();

	PGPBoolean	BestGuessComparePaths(LPCSTR path);
	PGPUInt64	GetUniqueSessionId();

	void		FlipContexts();
	DualErr		ValidateContexts();

	DualErr		Mount(LPCSTR path, CipherContext *context, PDCB pDcb = NULL, 
					PGPUInt8 drive = kInvalidDrive, 
					PGPBoolean mountReadOnly = FALSE);
	DualErr		Unmount(PGPBoolean isThisEmergency = FALSE);

	void		ReadAsync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo);
	void		WriteAsync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo);

	DualErr		ReadSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr		WriteSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo = NULL);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo = NULL);

private:
	CipherContext		*mContextA;				// cipher context A
	CipherContext		*mContextB;				// cipher context B

	PGPUInt64			mUniqueSessionId;		// unique session ID
	PGPdiskRequestInfo	mPGPdiskReq;			// request info
	PGPUInt8			*mDataBuf;				// data buffer

	SecureMemory		mSmContextA;			// secure mem for context A
	SecureMemory		mSmContextB;			// secure mem for context B

	void					SmartCipherBlocks(PGPUInt64 startBlockIndex, 
								PGPUInt32 nBlocks, const void *src, 
								void *dest,	CipherOp op);

	void					ExecuteRequest(PGPBoolean *doneWithRequest);

	static void				PGPdiskCallback(GenericCallbackInfo *downInfo);
	void					PGPdiskCallbackAux();

	void					ScheduleAsyncCallback(
								DualErr derr = DualErr::NoError);
	static VOID __stdcall	AsyncExecuteCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};

#endif	// ] Included_PGPdisk_h
