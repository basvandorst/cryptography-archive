/*
 * bn8086.c - bnInit() for Intel x86 family in 16-bit mode.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bn8086.c,v 1.5.4.1 1997/06/07 09:49:27 mhw Exp $
 */

#include "bni.h"
#include "bn16.h"
#include "bn32.h"

#ifndef BNINCLUDE
#error You must define BNINCLUDE to bni8086.h to use assembly primitives.
#endif

void
bnInit(void)
	{
			if (not386())
				 bnInit_16();
			else
				 bnInit_32();
	}
