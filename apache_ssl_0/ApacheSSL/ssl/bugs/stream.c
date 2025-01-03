/* bugs/stream.c */
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

#include <stdio.h>
#include "rc4.h"
#include "des.h"

/* show how stream ciphers are not very good.  The mac has no affect
 * on RC4 while it does for cfb DES
 */

main()
	{
	fprintf(stderr,"rc4\n");
	rc4();
	fprintf(stderr,"cfb des\n");
	des();
	}

int des()
	{
	des_key_schedule ks;
	des_cblock iv,key;
	int num;
	static char *keystr="01234567";
	static char *in1="0123456789ABCEDFdata 12345";
	static char *in2="9876543210abcdefdata 12345";
	unsigned char out[100];
	int i;

	des_set_key((des_cblock *)keystr,ks);

	num=0;
	memset(iv,0,8);
	des_cfb64_encrypt(in1,out,26,ks,(des_cblock *)iv,&num,1);
	for (i=0; i<26; i++)
		fprintf(stderr,"%02X ",out[i]);
	fprintf(stderr,"\n");

	num=0;
	memset(iv,0,8);
	des_cfb64_encrypt(in2,out,26,ks,(des_cblock *)iv,&num,1);
	for (i=0; i<26; i++)
		fprintf(stderr,"%02X ",out[i]);
	fprintf(stderr,"\n");
	}

int rc4()
	{
	static char *keystr="0123456789abcdef";
	RC4_KEY key;
	unsigned char in[100],out[100];
	int i;

	RC4_set_key(&key,16,keystr);
	in[0]='\0';
	strcpy(in,"0123456789ABCEDFdata 12345");
	RC4(key,26,in,out);

	for (i=0; i<26; i++)
		fprintf(stderr,"%02X ",out[i]);
	fprintf(stderr,"\n");

	RC4_set_key(&key,16,keystr);
	in[0]='\0';
	strcpy(in,"9876543210abcdefdata 12345");
	RC4(key,26,in,out);

	for (i=0; i<26; i++)
		fprintf(stderr,"%02X ",out[i]);
	fprintf(stderr,"\n");
	}
