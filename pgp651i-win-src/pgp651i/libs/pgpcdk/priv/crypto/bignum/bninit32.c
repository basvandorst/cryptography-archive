/*
 * bninit32.c - Provide an init function that sets things up for 32-bit
 * operation.  This is a seaparate tiny file so you can compile two bn
 * packages into the library and write a custom init routine.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bninit32.c,v 1.3 1998/05/14 19:07:24 cbertsch Exp $
 */

#include "bn.h"
#include "bn32.h"

void
bnInit(void)
{
	bnInit_32();
}
