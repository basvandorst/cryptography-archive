/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc.	All rights reserved.	$Id: CString.cp,v 1.7 1999/04/11 14:09:52 jason Exp $____________________________________________________________________________*/#include <TextUtils.h>#include "pgpMem.h"#include "CPGPException.h"#include "CString.h"CString::CString(	const char *	inString,	PGPSize			inMinStorage)		: mMinStorage(inMinStorage){	Init();	Set(reinterpret_cast<const PGPByte *>(inString), strlen(inString));}CString::CString(	ConstStringPtr	inString,	PGPSize			inMinStorage)		: mMinStorage(inMinStorage){	Init();	Set(&inString[1], inString[0]);}CString::CString(	const CStringAdaptor &	inStringAdapter,	PGPSize					inMinStorage)		: mMinStorage(inMinStorage){	Init();	inStringAdapter.Set(this);}CString::CString(	PGPByte *	inChars,	PGPSize		inLength,	PGPSize		inMinStorage)		: mMinStorage(inMinStorage){	Init();	Set(inChars, inLength);}CString::CString(	const CString &	inRHS){	PGPSize	storageSize = inRHS.GetMaxStringLength() + 2;	*this = inRHS;	mStorage = static_cast<PGPByte *>(pgpAlloc(storageSize));	PGPThrowIfMemFail_(mStorage);	pgpCopyMemory(inRHS.mStorage, mStorage, storageSize);}CString::~CString(){	pgpFree(mStorage);}	voidCString::SetMinStorage(	PGPSize	inMinStorage){	mMinStorage = inMinStorage;	ReallocateStorage();}	voidCString::SetLength(	PGPSize	inStringLength){	mStringLength = inStringLength;	mStringLengthGood = true;	mIsCString = false;	mIsPascalString = false;	ReallocateStorage();}	PGPSizeCString::GetLength() const{	if (! mStringLengthGood) {		if (mIsPascalString) {			const_cast<CString *>(this)->mStringLength = mStorage[0];		} else if (mIsCString) {			const_cast<CString *>(this)->mStringLength =				strlen(reinterpret_cast<const char *>(&mStorage[1]));		}		const_cast<CString *>(this)->mStringLengthGood = true;		const_cast<CString *>(this)->ReallocateStorage();	}	return mStringLength;}	PGPByteCString::GetPascalLength() const{	PGPSize length = GetLength();		return (length < 0xFF) ? length : 0xFF;}CString::operator ConstStringPtr(){	if (! mIsPascalString) {		mStorage[0] = GetPascalLength();		mIsPascalString = true;	}		return mStorage;}CString::operator StringPtr(){	if (! mIsPascalString) {		mStorage[0] = GetPascalLength();		mIsPascalString = true;	}	mIsCString = false;	mStringLengthGood = false;		return mStorage;}CString::operator const char *(){	if (! mIsCString) {		mStorage[GetLength() + 1] = 0;		mIsCString = true;	}		return reinterpret_cast<const char *>(&mStorage[1]);}CString::operator char *(){	if (! mIsCString) {		mStorage[GetLength() + 1] = 0;		mIsCString = true;	}	mIsPascalString = false;	mStringLengthGood = false;		return reinterpret_cast<char *>(&mStorage[1]);}	voidCString::Assign(	const char *	inString){	Set(reinterpret_cast<const PGPByte *>(inString), strlen(inString));}	voidCString::Assign(	ConstStringPtr	inString){	Set(&inString[1], inString[0]);}	voidCString::Assign(	const CStringAdaptor &	inStringAdaptor){	inStringAdaptor.Set(this);}	CString &CString::operator=(	const CString &	inRHS){	PGPSize	storageSize = inRHS.GetMaxStringLength() + 2;	*this = inRHS;	mStorage = static_cast<PGPByte *>(pgpAlloc(storageSize));	PGPThrowIfMemFail_(mStorage);	pgpCopyMemory(inRHS.mStorage, mStorage, storageSize);	return *this;}	inline voidCString::Init(){	mStorage = static_cast<PGPByte *>(pgpAlloc(0));	PGPThrowIfMemFail_(mStorage);	mSecure = false;}	inline voidCString::Set(	const PGPByte *	inData,	PGPSize			inLength){	SetLength(inLength);	pgpCopyMemory(inData, &mStorage[1], inLength);}	inline PGPSizeCString::GetMaxStringLength() const{	PGPSize	minStorage = GetMinStorage();	PGPSize stringLength = GetLength();		return (minStorage > stringLength) ? minStorage : stringLength;}	inline voidCString::ReallocateStorage(){	PGPThrowIfPGPError_(pgpRealloc(&mStorage, GetMaxStringLength() + 2));}	voidCStringFromSTR::Set(	CString *	inCString) const{	StringHandle	theString = ::GetString(mResID);		PGPThrowIfResFail_(theString);	::HLock(reinterpret_cast<Handle>(theString));	inCString->SetLength((*theString)[0]);	pgpCopyMemory(*theString, inCString->mStorage, (*theString)[0] + 1);	inCString->mStorage[(*theString)[0] + 1] = 0;	::ReleaseResource(reinterpret_cast<Handle>(theString));	inCString->mIsCString = true;	inCString->mIsPascalString = true;}	voidCStringFromSTRx::Set(	CString *	inCString) const{	inCString->SetLength(UCHAR_MAX);	::GetIndString(inCString->mStorage, mResID, mIndex);	inCString->SetLength(inCString->mStorage[0]);	inCString->mStorage[inCString->mStorage[0] + 1] = 0;	inCString->mIsCString = true;	inCString->mIsPascalString = true;}	voidCStringFromInt::Set(	CString *	inCString) const{	const PGPSize	kMaxCharsInInt32	=	32; // Takes into account w_chars		inCString->SetLength(kMaxCharsInInt32);	inCString->SetLength(sprintf(reinterpret_cast<char *>(&inCString->mStorage[1]), "%ld",		mInt32));	inCString->mStorage[0] = inCString->GetPascalLength();	inCString->mIsCString = true;	inCString->mIsPascalString = true;}