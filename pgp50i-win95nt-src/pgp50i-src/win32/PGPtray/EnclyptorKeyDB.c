/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"

#include "define.h"
#include "enclyptorkeydb.h"


BOOL WeNeedToWordWrap(unsigned short* WordWrapThreshold)
{
	long Value = 76;
	BOOL bWordWrap = FALSE;

	pgpLoadPrefs ();
	pgpGetPrefBoolean(kPGPPrefMailWordWrapEnable,  (Boolean*)&bWordWrap);

	if(bWordWrap)
	{
		pgpGetPrefNumber(kPGPPrefMailWordWrapWidth, &Value);

		*WordWrapThreshold = (unsigned short) Value;
	}

	return bWordWrap;
}
