/*____________________________________________________________________________	Copyright (C) 1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPAMEditTextImp.h,v 1.2 1999/03/10 02:36:22 heller Exp $____________________________________________________________________________*/#pragma once#include <LAMEditTextImp.h>class CPGPAMEditTextImp : public LAMEditTextImp{public:	enum { class_ID = 'IETX' };						CPGPAMEditTextImp(LControlPane *inControlPane);						CPGPAMEditTextImp(LStream *inStream);			virtual				~CPGPAMEditTextImp(void);protected:		virtual void		DrawSelf(void);};