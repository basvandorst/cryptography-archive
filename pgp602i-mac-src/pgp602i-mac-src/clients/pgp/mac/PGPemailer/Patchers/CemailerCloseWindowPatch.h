/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CemailerCloseWindowPatch.h,v 1.2.20.1 1998/11/12 03:07:39 heller Exp $____________________________________________________________________________*/#pragma once #include "CCloseWindowPatcher.h"#include "PGPemailerGlobals.h"class CemailerCloseWindowPatch	:	public	CCloseWindowPatcher {public:									CemailerCloseWindowPatch(											SGlobals & inGlobals);	virtual							~CemailerCloseWindowPatch();protected:	SGlobals &					mGlobals;		virtual void				NewCloseWindow(WindowRef theWindow);};