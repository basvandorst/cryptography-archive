/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPreferencesDialog.h,v 1.5.8.1 1998/11/12 03:05:16 heller Exp $____________________________________________________________________________*/#pragma once#include "CPGPDiskDialog.h"class CPreferencesDialog : public CPGPDiskDialog{public:	enum { class_ID = 'Pref' };						CPreferencesDialog();					CPreferencesDialog(LStream *inStream);						virtual			~CPreferencesDialog();	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);protected:	virtual void	FinishCreateSelf(void);};OSStatus	DoPreferencesDialog(void);