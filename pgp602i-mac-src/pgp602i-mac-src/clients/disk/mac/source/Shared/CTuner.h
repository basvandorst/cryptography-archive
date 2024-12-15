/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CTuner.h,v 1.2.8.1 1998/11/12 03:06:03 heller Exp $____________________________________________________________________________*/#pragma once#include "TunerStruct.h"class CTuner	{public:	enum IODirection { kReadDirection, kWriteDirection };	typedef OSErr		(*TunerCallback)( long callbackData,								ushort percentDone );	protected:	short			mVRefNum;	IODirection		mDirection;	void *			mBuffer;	short			mFileRefNum;	ulong			mTestFileSize;	TunerCallback	mCallback;	long			mCallbackData;		ushort			mNumTests;	ushort			mCurTestIndex;		virtual OSStatus	TestKPerSecond( short fileRefNum, void * buffer,							ulong ioSize, ulong testFileSize,							ulong * kPerSecond);						virtual OSStatus	FindOptimalBlocks( ulong ioSize, ulong bestBlocks,							ulong bestKPerSec, ulong * optimalBlocks);						virtual OSErr		DetermineTestFileSize( ulong ioSize, ulong *testSize);		virtual OSStatus	OpenTestFile( FSSpec *spec, short *	fileRefNum );		virtual OSStatus	TuneSize( ulong ioBlocks, ulong minBlocks,							ulong * optimalChunks );		virtual OSStatus	Tune( short vRefNum, IODirection direction,							TunerStruct *optimalTune,							TunerCallback callback = nil);		virtual OSStatus	CallCallback( void );public:						CTuner( void );	virtual				~CTuner( void );			virtual OSStatus	Tune( short vRefNum, TunerCallback callback = nil,							long callbackData = 0);	};