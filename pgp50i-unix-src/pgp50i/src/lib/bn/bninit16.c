/*
* bninit16.c - Provide an init function that sets things up for 16-bit
* operation. This is a seaparate tiny file so you can compile two bn
* packages into the library and write a custom init routine.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written in 1995 by Colin Plumb.
*
* $Id: bninit16.c,v 1.7.4.1 1997/06/07 09:49:37 mhw Exp $
*/

#include "bn.h"
#include "bn16.h"

void
bnInit(void)
{
	bnInit_16();
}
