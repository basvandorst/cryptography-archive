/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.  You may
 * not export it, in whole or in part, or cause or allow such export,
 * through act or omission, without prior authorization from the United
 * States government and written permission from AT&T.  In particular,
 * you may not make any part of this software available for general or
 * unrestricted distribution to others, nor may you disclose this software
 * to persons other than citizens and permanent residents of the United
 * States and Canada. 
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * support for client adm functions
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

#define MAXADMERR 7

char *admerrs[] = {
	"no error",			/* 0 */
	"permission denied",		/* 1 */
	"too many attached directories",/* 2 */
	"no such attached name",	/* 3 */
	"name already attached",	/* 4 */
	"no such encrypted directory",	/* 5 */
	"incorrect passphrase",		/* 6 */
	"badly formed name"		/* 7 */
	};

char *admmsg(e)
     int e;
{
	if ((e>MAXADMERR) || (e<0))
		return "unknown error";
	return admerrs[e];
}
