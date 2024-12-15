/*
 * bn68000.c - bnInit() for Motorola 680x0 family, 16 or 32-bit.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bn68000.c,v 1.5.2.1 1997/06/07 09:49:27 mhw Exp $
 */

#include "bn.h"
#include "bni.h"
#include "bn16.h"
#include "bn32.h"

#ifndef BNINCLUDE
#error You must define BNINCLUDE to bni68000.h to use assembly primitives.
#endif

void
bnInit(void)
	{
			if (is68020())
				 bnInit_32();
			else
				 bnInit_16();
	}
