/* lib/pem/pem_err.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SS
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
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIA
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

#include <stdio.h>
#include "crypto.h"
#include "pem.h"

/* BEGIN ERROR CODES */
static ERR_STRING_DATA PEM_str_functs[]=
	{
{ERR_PACK(0,PEM_F_DEF_CALLBACK,0),	"DEF_CALLBACK"},
{ERR_PACK(0,PEM_F_LOAD_IV,0),	"LOAD_IV"},
{ERR_PACK(0,PEM_F_PEM_ASN1_READ,0),	"PEM_ASN1_read"},
{ERR_PACK(0,PEM_F_PEM_DO_HEADER,0),	"PEM_do_header"},
{ERR_PACK(0,PEM_F_PEM_EVP_CIPHER_INFO,0),	"PEM_EVP_CIPHER_INFO"},
{ERR_PACK(0,PEM_F_PEM_READ,0),	"PEM_read"},
{ERR_PACK(0,PEM_F_PEM_SEALFINAL,0),	"PEM_SealFinal"},
{ERR_PACK(0,PEM_F_PEM_SEALINIT,0),	"PEM_SealInit"},
{ERR_PACK(0,PEM_F_PEM_SIGNFINAL,0),	"PEM_SignFinal"},
{ERR_PACK(0,PEM_F_PEM_WRITE,0),	"PEM_write"},
{ERR_PACK(0,PEM_F_PEM_WRITE_RSA,0),	"PEM_WRITE_RSA"},
{ERR_PACK(0,PEM_F_PEM_X509_INFO_READ,0),	"PEM_X509_INFO_read"},
{0,NULL},
	};

static ERR_STRING_DATA PEM_str_reasons[]=
	{
{PEM_R_BAD_BASE64_DECODE                 ,"bad base64 decode"},
{PEM_R_BAD_DECRYPT                       ,"bad decrypt"},
{PEM_R_BAD_END_LINE                      ,"bad end line"},
{PEM_R_BAD_IV_CHARS                      ,"bad iv chars"},
{PEM_R_BAD_PASSWORD_READ                 ,"bad password read"},
{PEM_R_NOT_DEK_INFO                      ,"not dek info"},
{PEM_R_NOT_ENCRYPTED                     ,"not encrypted"},
{PEM_R_NOT_PROC_TYPE                     ,"not proc type"},
{PEM_R_NO_START_LINE                     ,"no start line"},
{PEM_R_READ_KEY                          ,"read key"},
{PEM_R_SHORT_HEADER                      ,"short header"},
{PEM_R_UNSUPPORTED_CIPHER                ,"unsupported cipher"},
{PEM_R_UNSUPPORTED_ENCRYPTION            ,"unsupported encryption"},
{0,NULL},
	};

void ERR_load_PEM_strings()
	{
	static int init=1;

	if (init)
		{
		init=0;
		ERR_load_strings(ERR_LIB_PEM,PEM_str_functs);
		ERR_load_strings(ERR_LIB_PEM,PEM_str_reasons);
		}
	}
