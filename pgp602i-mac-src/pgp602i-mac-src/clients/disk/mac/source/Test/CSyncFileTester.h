/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CSyncFileTester.h,v 1.2.8.1 1998/11/12 03:06:24 heller Exp $____________________________________________________________________________*/#pragma onceclass CSyncFileTester	{public:		enum IODirection { kReadDirection, kWriteDirection };		protected:	long			mCurMark;	long			mEOF;		short			mTestRefNum;	FSSpec			mTestSpec;		public:				CSyncFileTester( short testFileRefNum );	virtual		~CSyncFileTester(  );		virtual void	Run( void );	};