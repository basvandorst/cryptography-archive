/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc.	All rights reserved.	$Id: CPGPnetWindow.h,v 1.8 1999/05/08 05:49:27 wprice Exp $____________________________________________________________________________*/#pragma once#include <LWindow.h>#include <LMultiPanelView.h>#include <LTabsControl.h>class CPGPnetWindow	:	public LWindow,						public LListener{public:	enum { class_ID = 'wNet' };						CPGPnetWindow(LStream * inStream) : LWindow(inStream) { }	virtual				~CPGPnetWindow();	virtual void		ListenToMessage(MessageT inMessage, void * ioParam);	virtual void		AttemptClose();	virtual void		ShowPanel( UInt16	inIndex );protected:	virtual void		FinishCreateSelf();		LMultiPanelView *	mPanelView;	LTabsControl *		mTabsControl;};