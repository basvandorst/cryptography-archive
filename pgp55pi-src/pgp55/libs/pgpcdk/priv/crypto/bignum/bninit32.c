/*
 * bninit32.c - Provide an init function that sets things up for 32-bit
 * operation.  This is a seaparate tiny file so you can compile two bn
 * packages into the library and write a custom init routine.
 *
 * Written in 1995 by Colin Plumb.
 *
 * WARNING: This file was automatically generated from bninit16.c
 *
 * $Id: bninit32.c,v 1.2 1997/05/13 01:18:47 mhw Exp $
 */

#include "bn.h"
#include "bn32.h"

void
bnInit(void)
{
	bnInit_32();
}
