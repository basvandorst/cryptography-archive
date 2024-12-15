//////////////////////////////////////////////////////////////////////////////
// PGPdiskRandomPool.h
//
// Contains declarations for class PGPdiskRandomPool.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRandomPool.h,v 1.1.2.5.2.1 1998/10/28 01:42:34 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskRandomPool_h	// [
#define Included_PGPdiskRandomPool_h


////////////
// Constants
////////////

const PGPUInt32	kDefaultRandomPoolBits	= 1024;		// default size of pool
const PGPUInt32 kRandomKeyDWords    	= 16;

//////////
// Classes
//////////

class PGPdiskRandomPool
{
public:
	DualErr mInitErr;

				PGPdiskRandomPool(
					PGPUInt32 poolBits = kDefaultRandomPoolBits);
				~PGPdiskRandomPool();

	void		AddMouseEntropy(PGPUInt32 x, PGPUInt32 y);
	void		Stir();

	PGPUInt8	GetByte();
	void		AddByte(PGPUInt8 byte);

	void		GetBytes(PGPUInt8 *buf, PGPUInt32 nBytes);
	void		AddBytes(const PGPUInt8 *buf, PGPUInt32 nBytes);

private:
	PGPUInt32	*mRandomPool;					// the pool itself
	PGPUInt32	mRandomKey[kRandomKeyDWords];	// next stirring key

	PGPUInt32	mRandomPoolGetPos;				// position to get from
	PGPUInt32	mRandomKeyAddPos;				// position to add from

	PGPUInt32	mRandomPoolBits;				// size of pool in bits
	PGPUInt32	mRandomPoolDWords;				// size of pool in dwords
};

#endif	// ] Included_PGPdiskRandomPool_h
