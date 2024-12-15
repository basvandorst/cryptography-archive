/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CString.h,v 1.6.10.1 1997/12/05 22:15:08 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <string.h>

#include "UPGPException.h"

class CString {
public:
					CString();
					CString(const Str255 inString, UInt32 inBufSize = 0);
					CString(const char * inString, UInt32 inBufSize = 0);
					CString(const char * inBuffer, UInt32 inLength,
						UInt32 inBufSize)
						{ InitString(inBuffer, inLength, inBufSize); }
					CString(ResIDT inResID, SInt16 inIndex);
					CString(SInt32 inNumber);
					~CString();
					
	void			SetClearOnDelete(Boolean inClear)
						{ mClearOnDelete = inClear; }
				
					// Getting and setting the pascal string
	ConstStringPtr	GetConstPString();
	StringPtr		GetPString();
	
					// Getting and setting the c string
	const char *	GetConstCString();
	char *			GetCString();
				
					// Conversion operators
					operator ConstStringPtr() { return GetConstPString(); }
					operator StringPtr() { return GetPString(); }
					operator char *() { return GetCString(); }
					operator const char *() { return GetConstCString(); }
					
					// Index into string
	char &			operator[](UInt32 inPosition)
						{ PGPThrowIf_(inPosition > (GetLength() - 1));
							return mBuffer[inPosition + 1]; }
	const char &	operator[](UInt32 inPosition) const
						{ PGPThrowIf_(inPosition > (GetLength() - 1));
							return mBuffer[inPosition + 1]; }
				
					// Getting and setting length
	UInt8			GetPLength() const;
	UInt32			GetLength() const;
	void			SetLength(UInt32 inLength);
	
					// Getting and setting the buffer size
	UInt32			GetBufferSize() { return mBufSize; }
	void			SetBufferSize(UInt32 inBufSize)
						{ ReallocateBuffer(inBufSize); }
						
					// Assignment
	CString &		Assign(const char * inBuffer, UInt32 inLength);
	CString &		Assign(const char * inString)
						{ return Assign(inString, strlen(inString)); }
	CString &		Assign(ResIDT inResID, SInt16 inIndex);
	CString &		Assign(StringHandle	inStringH)
						{ StHandleLocker theLock((Handle) inStringH);
							return Assign(*inStringH); }
	CString &		Assign(ConstStringPtr inString)
						{ return Assign((const char *) &inString[1],
							inString[0]); }
	CString &		Assign(CString & inCString)
						{	return Assign(&inCString.mBuffer[1],
							inCString.mBufSize); }
						
	void			AssignTo(StringPtr inString);

protected:
	enum EUpdateString {
		updateString_None		=	0,
		updateString_PString	=	1,
		updateString_CString	=	2
	};
	EUpdateString	mUpdateString;
	UInt32			mLength;
	Boolean			mClearOnDelete;
	UInt32			mBufSize; // Does not include the pchar and cchar.
	char *			mBuffer;
	
	void			UpdatePString() const;
	void			UpdateCString() const;
	void			AdjustStringsToLength() const;
	void			AllocateBuffer(UInt32 inBufSize);
	void			ReallocateBuffer(UInt32 inBufSize);

private:
	void			InitString(const char * inBuffer, UInt32 inLength,
						UInt32 inBufSize);
	void			InitString();
};

