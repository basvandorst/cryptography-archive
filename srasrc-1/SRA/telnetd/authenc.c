/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)authenc.c	5.1 (Berkeley) 3/1/91";
#endif /* not lint */

#if	defined(ENCRYPT) || defined(AUTHENTICATE)
#include "telnetd.h"
#include <libtelnet/misc.h>

	int
net_write(str, len)
	unsigned char *str;
	int len;
{
	if (nfrontp + len < netobuf + BUFSIZ) {
		memcpy((void *)nfrontp, (void *)str, len);
		nfrontp += len;
		return(len);
	}
	return(0);
}

	void
net_encrypt()
{
#if	defined(ENCRYPT)
	char *s = (nclearto > nbackp) ? nclearto : nbackp;
	if (s < nfrontp && encrypt_output) {
		(*encrypt_output)((unsigned char *)s, nfrontp - s);
	}
	nclearto = nfrontp;
#endif
}

	int
telnet_spin()
{
	ttloop();
	return(0);
}

	char *
telnet_getenv(val)
	char *val;
{
	extern char *getenv();
	return(getenv(val));
}

	char *
telnet_gets(prompt, result, length, echo)
	char *prompt;
	char *result;
	int length;
	int echo;
{
	return((char *)0);
}
#endif
