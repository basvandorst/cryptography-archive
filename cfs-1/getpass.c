/* adapted from d. mitchell's code */
/* includes code derived from the ucb getpass */
/*
 * Copyright (c) 1988 The Regents of the University of California.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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

#include <stdio.h>
#include <signal.h>
#ifndef linux
#include <sgtty.h>
#endif
#include <sys/types.h>
#include <rpc/rpc.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

#if defined(irix) || defined(linux)
/* hacks to use POSIX style termios instead of old BSD style sgttyb */
#include <termios.h>
#define sgttyb termios
#define gtty(a,b) tcgetattr((a), (b))
#define stty(a,b) tcsetattr((a), TCSAFLUSH, (b))
#define sg_flags c_lflag
#endif

	
char *
getpassword(prompt)
char *prompt;
{
	struct sgttyb ttyb;
	int flags;
	register char *p;
	register c;
	FILE *fi;
	static char pbuf[128];
#ifdef MACH
	int (*signal())();
	int (*sig)();
#else
	void (*sig)();
#endif

	if ((fi = fdopen(open("/dev/tty", 2), "r")) == NULL)
		fi = stdin;
	else
		setbuf(fi, (char *)NULL);
	sig = signal(SIGINT, SIG_IGN);
	gtty(fileno(fi), &ttyb);
	flags = ttyb.sg_flags;
	ttyb.sg_flags &= ~ECHO;
	stty(fileno(fi), &ttyb);
	fprintf(stderr, "%s", prompt); fflush(stderr);
	for (p=pbuf; (c = getc(fi))!='\n' && c!=EOF;) {
		if (p < &pbuf[127])
			*p++ = c;
	}
	*p = '\0';
	fprintf(stderr, "\n"); fflush(stderr);
	ttyb.sg_flags = flags;
	stty(fileno(fi), &ttyb);
	signal(SIGINT, sig);
	if (fi != stdin)
		fclose(fi);
	return(pbuf);
}

pwcrunch(b,k)
     char *b;
     cfs_admkey *k;
{
	int l;
	u_char k1[8];
	u_char k2[8];

	if ((l=strlen(b))<3)
		return -1;
	key_crunch(&b[l/2],l-l/2,k1);
	key_crunch(b,l/2,k2);
	switch (k->cipher) {
	    case CFS_STD_DES:
		bcopy(k1,k->cfs_admkey_u.deskey.primary,8);
		bcopy(k2,k->cfs_admkey_u.deskey.secondary,8);
		break;
	    case CFS_THREE_DES:
		bcopy(k1,k->cfs_admkey_u.des3key.primary1,8);
		bcopy(k1,k->cfs_admkey_u.des3key.secondary1,8);
		bcopy(k2,k->cfs_admkey_u.des3key.primary2,8);
		bcopy(k2,k->cfs_admkey_u.des3key.secondary2,8);
		break;
	    default:
		break;
	}
	return 0;
}
