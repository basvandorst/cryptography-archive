/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPGPDiskWindow.h,v 1.3.8.1 1998/11/12 03:05:14 heller Exp $____________________________________________________________________________*/#pragma once#include <LWindow.h>class CPGPDiskWindow : public LWindow{public:	enum { class_ID = 'DWnd' };							CPGPDiskWindow();						CPGPDiskWindow(LStream *inStream);							virtual				~CPGPDiskWindow();	virtual void		ApplyForeAndBackColors(void) const;protected:	virtual void		FinishCreateSelf(void);};