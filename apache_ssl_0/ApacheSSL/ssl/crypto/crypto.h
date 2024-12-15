/* lib/crypto.h */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error/err.h"

/* do we need to do this for getenv.
 * Just define getenv for use under windows */
#ifdef _Windows
#include <windows.h>
#endif

#if defined(MSDOS) || defined(_Windows)
#define LIST_SEPERATOR_CHAR ';'
#else
#define LIST_SEPERATOR_CHAR ':'
#endif

#ifdef _WINDLL
#define Getenv	GETENV
#define Fprintf	WIN_fprintf
#define Fputc	WIN_fputc
#define Fgets	WIN_fgets
#define Fwrite	WIN_fwrite
#define Fread	WIN_fread

#ifndef NOPROTO
int       WIN_fwrite(char *buf,int size,int count,FILE *fp);
char *    WIN_fgets(char *buf,int size,FILE *fp);
int   	  WIN_fputc(int ch,FILE *fp);
int cdecl WIN_fprintf(FILE *fp,char *format, ... );
int       WIN_fread(char *buf,int size,int count,FILE *fp);
#else
int       WIN_fwrite();
char *    WIN_fgets();
int   	  WIN_fputc();
int cdecl WIN_fprintf();
int       WIN_fread();
#endif

#else
#define Getenv getenv
#define Fprintf	fprintf
#define Fputc	fputc
#define Fgets	fgets
#define Fwrite	fwrite
#define Fread	fread
#endif

#define DG_GCC_BUG	/* gcc < 2.6.3 on DGUX */

#ifdef sgi
#define IRIX_CC_BUG	/* all version of IRIX I've tested (4.* 5.*) */
#endif

