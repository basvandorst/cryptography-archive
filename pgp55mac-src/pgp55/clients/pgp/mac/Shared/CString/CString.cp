/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CString.cp,v 1.5.10.1 1997/12/05 22:15:07 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpMem.h"
#include "CString.h"



CString::CString()
{
	InitString("", 0, 0);
}



CString::CString(
	const Str255	inString,
	UInt32			inBufSize)
{
	InitString((const char *) &inString[1], inString[0], inBufSize);
}



CString::CString(
	const char *	inString,
	UInt32			inBufSize)
{
	InitString(inString, strlen(inString), inBufSize);
}



CString::CString(
	ResIDT	inResID,
	SInt16	inIndex)
{
	if (inIndex > 0) {
		Str255	theString;
	
		::GetIndString(theString, inResID, inIndex);
		InitString((const char *) &theString[1], theString[0], 0);
	} else {
		StringHandle	theStringH = ::GetString(inResID);
		
		if (theStringH != nil) {
			StHandleLocker	theLock((Handle) theStringH);
			
			InitString((const char *) &(*theStringH)[1], (*theStringH)[0], 0);
		}
	}
}



CString::CString(
	SInt32	inNumber)
{
	Str255	theString;
	
	::NumToString(inNumber, theString);
	InitString((const char *) &theString[1], theString[0], 0);
}



CString::~CString()
{
	if (mClearOnDelete) {
		pgpClearMemory(	mBuffer,
						mBufSize + 2);
	}
	delete [] mBuffer;
}



	void
CString::InitString()
{
	mUpdateString = updateString_None;
	mLength = 0;
	mBufSize = 0;
	mBuffer = nil;
	mClearOnDelete = false;
	AllocateBuffer(mBufSize);
}


	void
CString::InitString(
	const char *	inBuffer,
	UInt32			inLength,
	UInt32			inBufSize)
{
	mUpdateString = updateString_None;
	
	if (inBufSize < inLength) {
		mBufSize = inLength;
	} else {
		mBufSize = inBufSize;
	}
	
	mLength = inLength;
	
	mBuffer = nil;
	AllocateBuffer(mBufSize);
	
	::BlockMoveData(	inBuffer,
						&mBuffer[1],
						inLength);
	AdjustStringsToLength();
}



	void
CString::SetLength(
	UInt32	inLength)
{
	if (inLength > mBufSize) {
		ReallocateBuffer(inLength);
	}
	
	mLength = inLength;
	AdjustStringsToLength();
}



	UInt8
CString::GetPLength() const
{
	switch(mUpdateString) {
		case updateString_PString:
			UpdatePString();
			break;
	}
	return mBuffer[0];
}



	UInt32
CString::GetLength() const
{
	switch(mUpdateString) {
		case updateString_PString:
			UpdatePString();
			break;
			
		case updateString_CString:
			UpdateCString();
			break;
	}
	return mLength;
}



	void
CString::AllocateBuffer(
	UInt32	inBufSize)
{
	delete [] mBuffer;
	
	// The buffer should always be at least large enough for a pascal string
	if (inBufSize < 255) {
		inBufSize = 255;
	}
	
	mBuffer = new char[inBufSize + 2];
	PGPThrowIfNil_(mBuffer);
	mBufSize = inBufSize;
}



	void
CString::ReallocateBuffer(
	UInt32	inBufSize)
{
	char *	theBuffer = mBuffer;
	
	switch(mUpdateString) {
		case updateString_PString:
			UpdatePString();
			break;
			
		case updateString_CString:
			UpdateCString();
			break;
	}

	mBuffer = new char[inBufSize + 2];
	PGPThrowIfNil_(mBuffer);
	
	::BlockMoveData(	theBuffer,
						mBuffer,
						mLength + 2);
	delete [] theBuffer;
	
	mBufSize = inBufSize;
}



	ConstStringPtr
CString::GetConstPString()
{
	if (mUpdateString == updateString_PString) {
		UpdatePString();
	}
	return (ConstStringPtr) mBuffer;
}



	StringPtr
CString::GetPString()
{
	if (mUpdateString == updateString_PString) {
		UpdatePString();
	}
	mUpdateString = updateString_CString;
	return (StringPtr) mBuffer;
}



	const char *
CString::GetConstCString()
{
	if (mUpdateString == updateString_CString) {
		UpdateCString();
	}
	return static_cast<const char *>(&mBuffer[1]);
}



	char *
CString::GetCString()
{
	if (mUpdateString == updateString_CString) {
		UpdateCString();
	}
	mUpdateString = updateString_PString;
	return static_cast<char *>(&mBuffer[1]);
}



	void
CString::UpdatePString() const
{
	// We need to cast away const here so that we can do our lazy evaluation
	CString *	thisNoConst = const_cast<CString *>(this);
	
	thisNoConst->mLength = strlen(&mBuffer[1]);
	thisNoConst->mBuffer[0] = (mLength < 255) ? mLength : 255;
	thisNoConst->mUpdateString = updateString_None;
}



	void
CString::UpdateCString() const
{
	// We need to cast away const here so that we can do our lazy evaluation
	CString *	thisNoConst = const_cast<CString *>(this);
	
	thisNoConst->mLength = ((UInt8 *) mBuffer)[0];
	thisNoConst->mBuffer[mLength + 1] = 0;
	thisNoConst->mUpdateString = updateString_None;
}



	void
CString::AdjustStringsToLength() const
{
	mBuffer[0] = (mLength < 255) ? mLength : 255;
	mBuffer[mLength + 1] = 0;
}



	CString &
CString::Assign(
	ResIDT	inResID,
	SInt16	inIndex)
{
	if (inIndex > 0) {
		Str255	theString;
	
		::GetIndString(theString, inResID, inIndex);
		Assign(theString);
	} else {
		StringHandle	theStringH = ::GetString(inResID);
		
		if (theStringH != nil) {
			StHandleLocker	theLock((Handle) theStringH);
			
			Assign(theStringH);
		}
	}
	
	return *this;
}



	CString &
CString::Assign(
	const char *	inBuffer,
	UInt32			inLength)
{
	AllocateBuffer(inLength);
	
	::BlockMoveData(	inBuffer,
						&mBuffer[1],
						inLength);
	mLength = inLength;
	AdjustStringsToLength();
	
	return *this;
}



	void	
CString::AssignTo(StringPtr inString)
{
	::BlockMoveData(	GetConstPString(),
						inString,
						GetPLength());
}


