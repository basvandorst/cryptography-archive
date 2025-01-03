/*
 * bninit16.c - Provide an init function that sets things up for 16-bit
 * operation.  This is a seaparate tiny file so you can compile two bn
 * packages into the library and write a custom init routine.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bninit16.c,v 1.1 1997/04/30 08:36:16 mhw Exp $
 */

#include "bn.h"
#include "bn16.h"

void
bnInit(void)
{
	bnInit_16();
}
