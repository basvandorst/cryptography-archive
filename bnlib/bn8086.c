/*
 * bn8086.c - bnInit() for Intel x86 family in 16-bit mode.
 *
 * Written in 1995 by Colin Plumb.  I'm not going to embarass myself
 * by claiming copyright on something this trivial.
 */

#include "lbn.h"
#include "bn16.h"
#include "bn32.h"

#ifndef BNINCLUDE
#error You must define BNINCLUDE to lbn8086.h to use assembly primitives.
#endif

void
bnInit(void)
{
	if (not386())
		bnInit_16();
	else
		bnInit_32();
}
