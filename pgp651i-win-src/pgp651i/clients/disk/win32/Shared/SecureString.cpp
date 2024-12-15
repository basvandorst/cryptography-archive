//////////////////////////////////////////////////////////////////////////////
// SecureString.cpp
//
// Implementation of class SecureString.
//////////////////////////////////////////////////////////////////////////////

// $Id: SecureString.cpp,v 1.5 1999/03/31 23:51:09 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>

#elif defined(PGPDISK_NTDRIVER)

#define	__w64
#include <vdw.h>
#include <stdio.h>
#include <stdlib.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "SecureString.h"


////////////////////////////////////////////////////
// Class SecureString public inline member functions
////////////////////////////////////////////////////

// The SecureString creates our 8-bit key. Eat your heart out, NSA.

SecureString::SecureString(LPCSTR str)
{
	mLength = 0;

	do
	{
		mKey = (PGPUInt8) (PGPdiskGetTicks() % 0xFF);
	}
	while (mKey == 0);

	for (PGPUInt32 i=0; i<kMaxStringSize; i++)
		mString[i] = mKey^NULL;

	if (str)
		SetString(str);
}

// operator= copies one SecureString to another without changing the key of
// either string.

SecureString & 
SecureString::operator=(const SecureString &from)
{
	PGPUInt32 i = 0;

	if (this == &from)
		return (* this);

	ClearString();

	while (i < from.GetLength())
	{
		InsertAt(i, from.GetAt(i));
		i++;
	}

	return (* this);
}

// operator== compares one SecureString to another, returns TRUE if equal.

PGPBoolean 
SecureString::operator==(const SecureString &peer) const
{
	PGPBoolean areEqual = TRUE;

	if (mLength != peer.GetLength())
	{
		areEqual = FALSE;
	}
	else
	{
		for (PGPUInt32 i=0; i < mLength; i++)
		{
			if (GetAt(i) != peer.GetAt(i))
			{
				areEqual = FALSE;
				break;
			}
		}
	}

	return areEqual;
}

// operator!= compares one SecureString to another, returns FALSE if equal.

PGPBoolean 
SecureString::operator!=(const SecureString &peer) const
{
	return !operator==(peer);
}

// GetLength returns the length of the string, sans the terminating null.

PGPUInt32 
SecureString::GetLength() const
{
	return mLength;
}

// GetString copies the decrypted string into the output buffer.

void 
SecureString::GetString(LPSTR str, PGPUInt32 size) const
{
	PGPUInt32 i, numToCopy;

	i = 0;
	numToCopy = min(size, mLength);

	while (i < numToCopy)
	{
		str[i] = GetAt(i);
		i++;
	}

	str[i] = NULL;
}

// SetString assigns the input string to the SecureString object.

void 
SecureString::SetString(LPCSTR str)
{
	PGPUInt32 i = 0;

	pgpAssertStrValid(str);

	ClearString();

	while ((i < kMaxStringSize) && (str[i] != NULL))
	{
		InsertAt(i, str[i]);
		i++;
	}
}

// ClearString sets the length of the string to 0;

void 
SecureString::ClearString()
{
	mLength = 0;
}

// GetAt returns the unencrypted character at the specified index.

PGPUInt8 
SecureString::GetAt(PGPUInt32 i) const
{
	PGPUInt8 c;

	if (i > mLength)
		return NULL;

	c = mKey^mString[i];

	return c;
}

// SetAt sets the character at the specified index.

void 
SecureString::SetAt(PGPUInt32 i, PGPUInt8 c)
{
	if (i > mLength)
		return;

	mString[i] = mKey^c;
}

// RemoveAt removes and returns the character at the given index,
// and shrinks the string.

PGPUInt8 
SecureString::RemoveAt(PGPUInt32 i)
{
	PGPUInt8 c;

	if ((i > mLength) || (mLength == 0))
		return NULL;

	c = GetAt(i);

	for (PGPUInt32 j = i; j<mLength-1; j++)
		SetAt(j, GetAt(j+1));

	mLength--;

	return c;
}

// InsertAt inserts a character at the given location, and expands the
// string to accomodate.

void 
SecureString::InsertAt(PGPUInt32 i, PGPUInt8 c)
{
	if ((i > mLength) || (mLength == kMaxStringSize))
		return;

	mLength++;
	SetAt(i, c);

	for (PGPUInt32 j=mLength-1; j>i; j--)
		SetAt(j, GetAt(j-1));
}
