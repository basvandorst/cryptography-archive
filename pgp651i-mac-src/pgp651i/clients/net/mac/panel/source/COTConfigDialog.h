/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc.	All rights reserved.	$Id: COTConfigDialog.h,v 1.1 1999/02/19 11:26:47 wprice Exp $____________________________________________________________________________*/#pragma once#include <LGADialog.h>#include <LMultiPanelView.h>#include <LTabsControl.h>#include <LPushButton.h>#include "NetworkSetupHelpers.h"class COTConfigTable;class COTConfigDialog	:	public LGADialog{public:	enum { class_ID = 'CfgD' };						COTConfigDialog(LStream * inStream);	virtual				~COTConfigDialog();	virtual void		ListenToMessage(MessageT inMessage, void * ioParam);protected:	virtual void		FinishCreateSelf();		LPushButton *				mSecureButton;	COTConfigTable *			mConfigTable;	NSHConfigurationListHandle	mConfigList;	PGPBoolean					mChanged;};