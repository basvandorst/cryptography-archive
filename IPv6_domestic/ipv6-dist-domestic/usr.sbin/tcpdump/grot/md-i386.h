/*
 * Copyright (c) 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @(#) /master/usr.sbin/tcpdump/tcpdump/md-i386.h,v 2.1 1995/02/03 18:13:59 polk Exp (LBL)
 */

#define TCPDUMP_ALIGN

#include <machine/endian.h>

/* 32-bit data types */
/* N.B.: this doesn't address printf()'s %d vs. %ld formats */
#define	int32 long /* signed 32-bit integer */
#ifndef	AUTH_UNIX
#define	u_int32 unsigned long /* unsigned 32-bit integer */
#endif
