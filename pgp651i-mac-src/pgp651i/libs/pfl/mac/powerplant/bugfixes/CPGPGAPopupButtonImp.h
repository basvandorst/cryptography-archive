/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPGAPopupButtonImp.h,v 1.2 1999/03/10 02:36:48 heller Exp $____________________________________________________________________________*/#pragma once#include <LGAPopupButtonImp.h>class CPGPGAPopupButtonImp : public LGAPopupButtonImp{public:	enum { class_ID = 'GPOP' };						CPGPGAPopupButtonImp(LControlPane *inControlPane);						CPGPGAPopupButtonImp(LStream *inStream);			virtual				~CPGPGAPopupButtonImp(void);		virtual void		SetDescriptor(ConstStringPtr inDescriptor);};