/*____________________________________________________________________________	Copyright (C) 1996-1999 Network Associates, Inc.	All rights reserved.	$Id: COneClikListBox.h,v 1.4 1999/03/10 02:35:18 heller Exp $____________________________________________________________________________*/#pragma once#include <LListBox.h>class COneClikListBox	:	public LListBox{public:	enum { class_ID = '1clb' };					COneClikListBox();					~COneClikListBox();					COneClikListBox(LStream *inStream);	static COneClikListBox*	CreateOneClikListBoxStream(LStream *inStream);	void			ClickSelf(const SMouseDownEvent &inMouseDown);};