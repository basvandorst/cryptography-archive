/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CInterruptLevelFileTester.h,v 1.3.8.1 1998/11/12 03:06:20 heller Exp $____________________________________________________________________________*/#pragma once#include "CInterruptLevelTester.h"class CInterruptLevelFileTester : public CInterruptLevelTester	{public:		enum IODirection { kReadDirection, kWriteDirection };		protected:		typedef CInterruptLevelTester	inherited;				long			mCurMark;		long			mEOF;						virtual void		HandleIOCompletion( MyPB *myPB );		virtual void		DoIOFromTimeMgrTask( void );		virtual OSErr		Setup( short	fileRefNum );public:					CInterruptLevelFileTester( void );		virtual		~CInterruptLevelFileTester(  );	};