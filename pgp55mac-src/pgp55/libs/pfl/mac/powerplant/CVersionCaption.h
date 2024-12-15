/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

#pragma once

#include <LCaption.h>

class CVersionCaption : public LCaption
{
public:

	enum { class_ID = 'VCpt' };

						CVersionCaption(LStream *inStream);
	virtual				~CVersionCaption(void);
	
protected:

	virtual void		FinishCreateSelf(void);
};