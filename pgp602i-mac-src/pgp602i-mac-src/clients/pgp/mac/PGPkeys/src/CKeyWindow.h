/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: CKeyWindow.h,v 1.4.8.1 1998/11/12 03:08:07 heller Exp $____________________________________________________________________________*/#pragma once#include <LWindow.h>#include "pgpKeys.h"class CKeyView;class LWindowHeader;class CKeyWindow		:		public LWindow{public:	enum { class_ID = 'keyW' };								CKeyWindow();								CKeyWindow(LStream *inStream);								~CKeyWindow();	void						FinishCreateSelf();	virtual void				FindCommandStatus(CommandT inCommand,									Boolean &outEnabled, Boolean &outUsesMark,									Char16 &outMark, Str255 outName);	Boolean						ObeyCommand(									CommandT	inCommand,									void		*ioParam);	CKeyView					*GetKeyView()									{	return mKeyView;	}	void						DoToolbar();	private:	CKeyView					*mKeyView;	LWindowHeader				*mToolbar;	PGPBoolean					mShowToolbar;};