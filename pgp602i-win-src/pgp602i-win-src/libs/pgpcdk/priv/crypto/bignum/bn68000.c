/*
 * bn68000.c - bnInit() for Motorola 680x0 family, 16 or 32-bit.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bn68000.c,v 1.1 1997/04/30 08:36:06 mhw Exp $
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
