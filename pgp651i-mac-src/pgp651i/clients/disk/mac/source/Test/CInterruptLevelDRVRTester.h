/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CInterruptLevelDRVRTester.h,v 1.4 1999/03/10 02:34:07 heller Exp $____________________________________________________________________________*/#pragma once#include "CInterruptLevelTester.h"class CInterruptLevelDRVRTester : public CInterruptLevelTester	{	protected:		typedef CInterruptLevelTester	inherited;				short			mDriveNumber;		short			mDriverRefNum;				ulong			mFirstBlockToTest;		ulong			mNumBlocksToTest;		ulong			mCurBlock;						virtual OSErr	GetTestBlockRange( short fileRefNum,							ulong * firstBlockToTest,							ulong * numBlocksToTest);				virtual void	GetNextIOInfo( ulong * startBlockPtr,							ulong * numBlocksPtr);		virtual void	HandleIOCompletion( MyPB *myPB );		virtual void	DoIOFromTimeMgrTask( void );				virtual OSErr	Setup( short fileRefNum );	public:								CInterruptLevelDRVRTester( void );		virtual			~CInterruptLevelDRVRTester( void );	};