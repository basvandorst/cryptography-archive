/*
 * bninit64.c - Provide an init function that sets things up for 64-bit
 * operation.  This is a seaparate tiny file so you can compile two bn
 * packages into the library and write a custom init routine.
 *
 * Written in 1995 by Colin Plumb.
 *
 * $Id: bninit64.c,v 1.1 1997/12/14 11:30:29 wprice Exp $
 */

#include "bn.h"
#include "bn64.h"

void
bnInit(void)
{
	bnInit_64();
}
