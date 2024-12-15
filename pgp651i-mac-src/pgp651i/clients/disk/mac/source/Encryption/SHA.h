/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: SHA.h,v 1.5 1999/03/10 03:03:18 heller Exp $____________________________________________________________________________*/#ifndef SHS_H#define SHS_H/* NIST proposed Secure Hash Standard.   Written 2 September 1992, Peter C. Gutmann.   This implementation placed in the public domain.   Modified 1 June 1993, Colin Plumb.   These modifications placed in the public domain.   Comments to pgut1@cs.aukuni.ac.nz */// The SHS block size and message digest sizes, in bytes#define kSHS_BLOCKSIZE	64#define kSHS_DIGESTSIZE	20class SHA{public:	typedef union Digest		{		uchar	bytes[ kSHS_DIGESTSIZE ];		ulong	longs[ kSHS_DIGESTSIZE / sizeof( ulong ) ];		} Digest;					SHA(void);	virtual		~SHA(void)	{ }	virtual void	Init(void);	virtual void	Update(const uchar *buf, ulong len);	virtual void	Final(Digest *	digest);	virtual SHA		*Clone();protected:	void	Transform(void);	ulong	mData[16];				// SHS data buffer	Digest	mDigest;	ulong	mCountHi;		// 64-bit byte count	ulong	mCountLo;		// 64-bit byte count};#endif	// SHS_H