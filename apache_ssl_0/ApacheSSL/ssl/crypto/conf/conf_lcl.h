/* lib/conf/conf_lcl.h */
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

#define CONF_NUMBER		1
#define CONF_UPPER		2
#define CONF_LOWER		4
#define CONF_UNDER		256
#define CONF_WS			16
#define CONF_ESC		32
#define CONF_QUOTE		64
#define CONF_COMMENT		128
#define CONF_EOF		8
#define CONF_ALPHA		(CONF_UPPER|CONF_LOWER)
#define CONF_ALPHA_NUMERIC	(CONF_ALPHA|CONF_NUMBER|CONF_UNDER)

#define IS_COMMENT(a)		(CONF_COMMENT&(CONF_type[(a)&0x7f]))
#define IS_EOF(a)		((a) == '\0')
#define IS_ESC(a)		((a) == '\\')
#define IS_NUMER(a)		(CONF_type[(a)&0x7f]&CONF_NUMBER)
#define IS_WS(a)		(CONF_type[(a)&0x7f]&CONF_WS)
#define IS_ALPHA_NUMERIC(a)	(CONF_type[(a)&0x7f]&CONF_ALPHA_NUMERIC)
#define IS_QUOTE(a)		(CONF_type[(a)&0x7f]&CONF_QUOTE)

unsigned short CONF_type[128]={
	0x008,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x010,0x010,0x000,0x000,0x010,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x010,0x000,0x040,0x080,0x000,0x000,0x000,0x040,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x001,0x001,0x001,0x001,0x001,0x001,0x001,0x001,
	0x001,0x001,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x002,0x002,0x002,0x002,0x002,0x002,0x002,
	0x002,0x002,0x002,0x002,0x002,0x002,0x002,0x002,
	0x002,0x002,0x002,0x002,0x002,0x002,0x002,0x002,
	0x002,0x002,0x002,0x000,0x020,0x000,0x000,0x100,
	0x040,0x004,0x004,0x004,0x004,0x004,0x004,0x004,
	0x004,0x004,0x004,0x004,0x004,0x004,0x004,0x004,
	0x004,0x004,0x004,0x004,0x004,0x004,0x004,0x004,
	0x004,0x004,0x004,0x000,0x000,0x000,0x000,0x000,
	};
