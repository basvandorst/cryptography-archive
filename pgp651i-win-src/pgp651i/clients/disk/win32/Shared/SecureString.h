//////////////////////////////////////////////////////////////////////////////
// SecureString.h
//
// Declaration and implementation of class SecureString.
//////////////////////////////////////////////////////////////////////////////

// $Id: SecureString.h,v 1.5 1998/12/14 19:01:26 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SecureString_h	// [
#define Included_SecureString_h

#include "DualErr.h"
#include "UtilityFunctions.h"


/////////////////////
// Class SecureString
/////////////////////

// SecureString is a very simple string class that incorporates a randomly
// generated XOR encryption algorithm to foil techniques such as memory and
// swap-file keyword searching.

class SecureString
{
public:
					SecureString(LPCSTR str = NULL);

	SecureString &	operator=(const SecureString &from);
	PGPBoolean		operator==(const SecureString &peer) const;
	PGPBoolean		operator!=(const SecureString &peer) const;

	PGPUInt32		GetLength() const;

	void			GetString(LPSTR str, PGPUInt32 size) const;
	void			SetString(LPCSTR str);
	void			ClearString();

	PGPUInt8		GetAt(PGPUInt32 i) const;
	void			SetAt(PGPUInt32 i, PGPUInt8 c);

	PGPUInt8		RemoveAt(PGPUInt32 i);
	void			InsertAt(PGPUInt32 i, PGPUInt8 c);

private:
	PGPUInt8	mString[kMaxStringSize];	// encrypted string
	PGPUInt8	mKey;						// XOR key
	PGPUInt32	mLength;					// length of string
};

#endif	// ] Included_SecureString_h
