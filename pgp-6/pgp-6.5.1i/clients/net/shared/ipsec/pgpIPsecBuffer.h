/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecBuffer.h,v 1.2 1999/03/19 22:38:51 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecBuffer_h	/* [ */
#define Included_pgpIPsecBuffer_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"

typedef struct _PGPIPsecBuffer PGPIPsecBuffer;

struct _PGPIPsecBuffer
{
	PGPByte *		data;
	PGPUInt32		dataSize;
	PGPUInt32		allocatedSize;
	PGPIPsecBuffer *next;
};

#endif /* ] Included_pgpIPsecBuffer_h */


PGPError PGPCopyIPsecBuffer(PGPMemoryMgrRef memoryMgr,
			PGPIPsecBuffer *inBuffer, PGPUInt32 inStart,
			PGPIPsecBuffer *outBuffer, PGPUInt32 outStart,
			PGPUInt32 numBytes, PGPUInt32 maxBufferSize);

PGPError PGPExpandIPsecBuffer(PGPMemoryMgrRef memoryMgr, 
			PGPIPsecBuffer *buffer, PGPUInt32 maxBufferSize, 
			PGPUInt32 expandSize);

PGPBoolean PGPIPsecBufferDataIsEqual(PGPIPsecBuffer *inBuffer, 
			PGPUInt32 inStart, PGPIPsecBuffer *outBuffer, PGPUInt32 outStart,
			PGPUInt32 numBytes);

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
