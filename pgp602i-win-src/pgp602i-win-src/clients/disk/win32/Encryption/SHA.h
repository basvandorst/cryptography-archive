//////////////////////////////////////////////////////////////////////////////
// SHA.h
//
// Contains declarations for class SHA.
//////////////////////////////////////////////////////////////////////////////

// $Id: SHA.h,v 1.1.2.4 1998/07/06 08:56:40 nryan Exp $

// Portions Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SHA_h	// [
#define Included_SHA_h

/* NIST proposed Secure Hash Standard.

   Written 2 September 1992, Peter C. Gutmann.
   This implementation placed in the public domain.

   Modified 1 June 1993, Colin Plumb.
   These modifications placed in the public domain.

   Modified 22 October 1998, Network Associates, Inc.
   Modifications copyright Network Associates, Inc.

   Comments to pgut1@cs.aukuni.ac.nz */


////////////
// Class SHA
////////////

class SHA
{
public:
	enum	{kSHS_BLOCKSIZE = 64};
	enum	{kSHS_DIGESTSIZE = 20};
	
	typedef union Digest
	{
		PGPUInt8	bytes[kSHS_DIGESTSIZE];
		PGPUInt32	dwords[kSHS_DIGESTSIZE / sizeof(PGPUInt32)];
	} Digest;
	
					SHA();
	virtual			~SHA()	{ };

	virtual void	Init();
	virtual void	Update(const PGPUInt8 *buf, PGPUInt16 len);
	virtual void	Final(Digest *digest);

	virtual SHA *	Clone();

protected:
	Digest		mDigest;
	PGPUInt32	mCountHi;			// 64-bit byte count
	PGPUInt32	mCountLo;			// 64-bit byte count
	PGPUInt32	mData[16];			// SHS data buffer

	void	ByteReverse(PGPUInt32 *buf, PGPUInt32 byteCount);
	void	Transform();
};

#endif	// ] Included_SHA_h
