//////////////////////////////////////////////////////////////////////////////
// PGPdiskRandomPool.cpp
//
// Implementation of class PGPdiskRandomPool.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRandomPool.cpp,v 1.1.2.11.2.1 1998/10/28 01:42:33 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "UtilityFunctions.h"

#include "PGPdiskRandomPool.h"
#include "SHA.h"


//////////////////////////////////////////////////
// Class PGPdiskRandomPool public member functions
//////////////////////////////////////////////////

// The PGPdiskRandomPool default constructor.

PGPdiskRandomPool::PGPdiskRandomPool(PGPUInt32 poolBits)
{
	// The pool must be a multiple of the 16-byte (128-bit) SHA1 block size.
	pgpAssert(poolBits%128 == 0);

	mRandomPoolBits = poolBits;
	mRandomKeyAddPos = 0;

	mRandomPoolDWords = (poolBits+127 & ~127) >> 5;
	mRandomPoolGetPos = mRandomPoolDWords/4;

	pgpAssert(mRandomPoolDWords > kRandomKeyDWords);

	mInitErr = GetByteBuffer(mRandomPoolDWords*4, (PGPUInt8 **) &mRandomPool);
}

// The PGPdiskRandomPool destructor.

PGPdiskRandomPool::~PGPdiskRandomPool()
{
	if (mRandomPool)
		FreeByteBuffer((PGPUInt8 *) mRandomPool);
}

// AddMouseEntropy adds entropy to the pool. Data from the mouse position and
// the system clock is used. Pass in values for 'x' and 'y'.

void 
PGPdiskRandomPool::AddMouseEntropy(PGPUInt32 x, PGPUInt32 y)
{
	PGPUInt64 sysTime;

	AddBytes((PGPUInt8 *) &x, sizeof(PGPUInt32));
	AddBytes((PGPUInt8 *) &y, sizeof(PGPUInt32));
	
	// Add current ms since system startup.

	sysTime = PGPdiskGetTicks();
	
	AddBytes((PGPUInt8 *) &sysTime, sizeof(PGPUInt32));
}

// Stir "stirs in" any random seed material before removing any random bytes.
// This also ensures that no sensitive data remains in memory that can be
// recovered later.

void
PGPdiskRandomPool::Stir()
{
	PGPUInt32	i;
	PGPUInt32	iv[4];

	// Start IV from last block of the random pool.
	pgpCopyMemory(mRandomPool+mRandomPoolDWords - 4, iv, sizeof(iv));

	// CFB pass.
	for (i=0; i<mRandomPoolDWords; i+=4)
	{
		SHA			sha;
		SHA::Digest	digest;
		
		sha.Update((const PGPUInt8 *) iv, sizeof(iv));
		sha.Update((const PGPUInt8 *) &mRandomKey[0], sizeof(mRandomKey));
		sha.Final(&digest);
		pgpCopyMemory(&digest, iv, sizeof(iv));

		iv[0] = mRandomPool[i  ] ^= iv[0];
		iv[1] = mRandomPool[i+1] ^= iv[1];
		iv[2] = mRandomPool[i+2] ^= iv[2];
		iv[3] = mRandomPool[i+3] ^= iv[3];
	}

	// Wipe iv from memory.
	for(i=0; i<4; i++)
		iv[i] = 0;

	// Get new key.
	pgpCopyMemory(mRandomPool, mRandomKey, sizeof(mRandomKey));

	// Set up pointers for future addition or removal of random bytes.
	mRandomKeyAddPos = 0;
	mRandomPoolGetPos = sizeof(mRandomKey);
}

// GetByte gets a single byte.

PGPUInt8
PGPdiskRandomPool::GetByte()
{
	if (mRandomPoolGetPos == mRandomPoolDWords*4)
		Stir();

	return ((const PGPUInt8 *) mRandomPool)[mRandomPoolGetPos++];
}

// AddByte adds one byte to the entropy pool.

void
PGPdiskRandomPool::AddByte(PGPUInt8 byte)
{
	AddBytes(&byte, sizeof(PGPUInt8));
}

// GetBytes withdraws some bits from the pool. Regardless of the distribution
// of the input bits, the bits returned are uniformly distributed, although
// they cannot, of course, contain more Shannon entropy than the input bits.

void
PGPdiskRandomPool::GetBytes(PGPUInt8 *buf, PGPUInt32 len)
{
	PGPUInt32 t;

	while (len > (t = mRandomPoolDWords*4 - mRandomPoolGetPos))
	{
		pgpCopyMemory(((const PGPUInt8 *) mRandomPool) + mRandomPoolGetPos, 
			buf, t);

		buf	+= t;
		len	-= t;

		Stir();
	}

	pgpCopyMemory(((const PGPUInt8 *) mRandomPool)+mRandomPoolGetPos, buf, 
		len);
	mRandomPoolGetPos += len;
}

// AddBytes makes a deposit of information (entropy) into the pool. This is
// done by XORing the data into the key which used to encrypt the pool.
// Before any bytes are retrieved from the pool, the altered key will be used
// to encrypt the whole pool, causing all bits in the pool to depend on the
// new information.
//
// The bits deposited need not have any particular distribution; the stirring
// operation transforms them into uniformly-distributed bits.

void
PGPdiskRandomPool::AddBytes(const PGPUInt8 *buf, PGPUInt32 len)
{
	PGPUInt8	*p	= ((PGPUInt8 *) mRandomKey) + mRandomKeyAddPos;
	PGPUInt32	t	= sizeof(mRandomKey) - mRandomKeyAddPos;

	while (len > t)
	{
		len -= t;
		while (t--)
			(* p++) ^= (* buf++);

		Stir();		// sets mRandomKeyPos to 0

		p = (PGPUInt8 *) mRandomKey;
		t = sizeof(mRandomKey);
	}

	if (len)
	{
		mRandomKeyAddPos += len;
		do
			(* p++) ^= (* buf++);
		while (--len);

		mRandomPoolGetPos = mRandomPoolDWords*4;	// Force stir on get
	}
}
