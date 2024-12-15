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
#include "shs.h"

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

old_pwcrunch(b,k)
     char *b;
     cfs_admkey *k;
{
	int l;
	u_char k1[8];
	u_char k2[8];

	if ((l=strlen(b))<3)
		return -1;
	/* note that we always use the DES key crunch. */
	/* that's not a significant weakness, since here DES */
	/* is just being used as a non-cryptographic hash. */
	/* note also that k1 and k2 are created from disjoint parts */
	/* of the passphrase. */
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
	    case CFS_TRUE_THREE_DES: /* this is really 2des */
		bcopy(k1,k->cfs_admkey_u.tdes3key.primary1,8);
		bcopy(k1,k->cfs_admkey_u.tdes3key.secondary1,8);
		bcopy(k2,k->cfs_admkey_u.tdes3key.primary2,8);
		bcopy(k2,k->cfs_admkey_u.tdes3key.secondary2,8);
		bcopy(k1,k->cfs_admkey_u.tdes3key.primary3,8);
		bcopy(k1,k->cfs_admkey_u.tdes3key.secondary3,8);
		break;
	    case CFS_MACGUFFIN:
		bcopy(k1,k->cfs_admkey_u.mcgkey.primary,8);
		bcopy(k2,&(k->cfs_admkey_u.mcgkey.primary[8]),8);
		bcopy(k1,k->cfs_admkey_u.mcgkey.secondary,8);
		bcopy(k2,&(k->cfs_admkey_u.mcgkey.secondary[8]),8);
		break;
	    case CFS_BLOWFISH:
		bcopy(k1,k->cfs_admkey_u.blowkey.primary,8);
		bcopy(k2,&(k->cfs_admkey_u.blowkey.primary[8]),8);
		bcopy(k1,k->cfs_admkey_u.blowkey.secondary,8);
		bcopy(k2,&(k->cfs_admkey_u.blowkey.secondary[8]),8);
		break;
	    case CFS_SAFER_SK128:
		bcopy(k1,k->cfs_admkey_u.saferkey.primary,8);
		bcopy(k2,&(k->cfs_admkey_u.saferkey.primary[8]),8);
		bcopy(k1,k->cfs_admkey_u.saferkey.secondary,8);
		bcopy(k2,&(k->cfs_admkey_u.saferkey.secondary[8]),8);
	    default:
		break;
	}
	return 0;
}
new_pwcrunch(b,k)
     char *b;
     cfs_admkey *k;
{
	int l;
	u_char *k1;
	u_char *k2;
	u_char *k3;
	u_char *hash;
	u_char h1[20];
	u_char h2[20];
	
	if ((l=strlen(b))<3)
		return -1;
	hash = qshs(b,l);
	bcopy(hash,h1,20);
	k1 = h1;
	k2 = &(h1[8]);
	/* for true threedes, we do one more hash to get the third key */
	hash = qshs(h1,20);
	bcopy(hash,h2,20);
	k3 = h2;
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
	    case CFS_TRUE_THREE_DES:
		bcopy(k1,k->cfs_admkey_u.tdes3key.primary1,8);
		bcopy(k1,k->cfs_admkey_u.tdes3key.secondary1,8);
		bcopy(k2,k->cfs_admkey_u.tdes3key.primary2,8);
		bcopy(k2,k->cfs_admkey_u.tdes3key.secondary2,8);
		bcopy(k3,k->cfs_admkey_u.tdes3key.primary3,8);
		bcopy(k3,k->cfs_admkey_u.tdes3key.secondary3,8);
		break;
	    case CFS_MACGUFFIN:
		bcopy(k1,k->cfs_admkey_u.mcgkey.primary,16);
		bcopy(k1,k->cfs_admkey_u.mcgkey.secondary,16);
		break;
	    case CFS_BLOWFISH:
		bcopy(k1,k->cfs_admkey_u.blowkey.primary,16);
		bcopy(k1,k->cfs_admkey_u.blowkey.secondary,16);
		break;
	    case CFS_SAFER_SK128:
		bcopy(k1,k->cfs_admkey_u.saferkey.primary,16);
		bcopy(k1,k->cfs_admkey_u.saferkey.secondary,16);
		break;
	    default:
		break;
	}
	return 0;
}

decrypt_key(k,ek)
     cfs_admkey *k;
     u_char *ek;
{
	mcg_key mk;
	BF_KEY bk;
	safer_key_t sk;
	
	switch (k->cipher) {
	    case CFS_STD_DES:
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.deskey.secondary,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.deskey.secondary,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[8]),1);
		bcopy(&(ek[0]),k->cfs_admkey_u.deskey.primary,8);
		bcopy(&(ek[8]),k->cfs_admkey_u.deskey.secondary,8);
		break;
	    case CFS_THREE_DES:
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.des3key.primary2,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.des3key.primary2,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[8]),1);
		bcopy(&(ek[0]),k->cfs_admkey_u.des3key.primary1,8);
		bcopy(&(ek[0]),k->cfs_admkey_u.des3key.secondary1,8);
		bcopy(&(ek[8]),k->cfs_admkey_u.des3key.primary2,8);
		bcopy(&(ek[8]),k->cfs_admkey_u.des3key.secondary2,8);
		break;
	    case CFS_TRUE_THREE_DES:
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary3,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary2,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary1,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary3,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary2,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary1,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary3,&(ek[16]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary2,&(ek[16]),1);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary1,&(ek[16]),1);
		bcopy(&(ek[0]),k->cfs_admkey_u.tdes3key.primary1,8);
		bcopy(&(ek[0]),k->cfs_admkey_u.tdes3key.secondary1,8);
		bcopy(&(ek[8]),k->cfs_admkey_u.tdes3key.primary2,8);
		bcopy(&(ek[8]),k->cfs_admkey_u.tdes3key.secondary2,8);
		bcopy(&(ek[16]),k->cfs_admkey_u.tdes3key.primary3,8);
		bcopy(&(ek[16]),k->cfs_admkey_u.tdes3key.secondary3,8);
		break;
	    case CFS_MACGUFFIN:
		mcg_keyset(k->cfs_admkey_u.mcgkey.primary,&mk);
		mcg_block_decrypt(&mk,&(ek[0]));
		mcg_block_decrypt(&mk,&(ek[8]));
		bcopy(ek,k->cfs_admkey_u.mcgkey.primary,16);
		bcopy(ek,k->cfs_admkey_u.mcgkey.secondary,16);
		break;
	    case CFS_BLOWFISH:
		BF_set_key(&bk,16,k->cfs_admkey_u.blowkey.primary);
		BF_encrypt(&(ek[0]),&bk,BF_DECRYPT);
		BF_encrypt(&(ek[8]),&bk,BF_DECRYPT);
		bcopy(ek,k->cfs_admkey_u.blowkey.primary,16);
		bcopy(ek,k->cfs_admkey_u.blowkey.secondary,16);
		break;
	    case CFS_SAFER_SK128:
		Safer_Init_Module();
		Safer_Expand_Userkey(k->cfs_admkey_u.saferkey.primary,
				     &(k->cfs_admkey_u.saferkey.primary[8]),
				     SAFER_SK128_DEFAULT_NOF_ROUNDS, 1,
				     sk);
		Safer_Decrypt_Block(&(ek[0]),sk,&(ek[0]));
		Safer_Decrypt_Block(&(ek[8]),sk,&(ek[8]));
		bcopy(ek,k->cfs_admkey_u.saferkey.primary,16);
		bcopy(ek,k->cfs_admkey_u.saferkey.secondary,16);
		break;
	    default:
		break;
	}
}

encrypt_key(k,ek)
     cfs_admkey *k;
     u_char *ek;
{
	mcg_key mk;
	BF_KEY bk;
	safer_key_t sk;

	switch (k->cipher) {
	    case CFS_STD_DES:
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.deskey.secondary,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.deskey.secondary,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.deskey.primary,&(ek[8]),0);
		break;
	    case CFS_THREE_DES:
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.des3key.primary2,&(ek[0]),1);
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.des3key.primary2,&(ek[8]),1);
		q_block_cipher(k->cfs_admkey_u.des3key.primary1,&(ek[8]),0);
		break;
	    case CFS_TRUE_THREE_DES:
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary1,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary2,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary3,&(ek[0]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary1,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary2,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary3,&(ek[8]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary1,&(ek[16]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary2,&(ek[16]),0);
		q_block_cipher(k->cfs_admkey_u.tdes3key.primary3,&(ek[16]),0);
		break;
	    case CFS_BLOWFISH:
		BF_set_key(&bk,16,k->cfs_admkey_u.blowkey.primary);
		BF_encrypt(&(ek[0]),&bk,BF_ENCRYPT);
		BF_encrypt(&(ek[8]),&bk,BF_ENCRYPT);
		break;
	    case CFS_MACGUFFIN:
		mcg_keyset(k->cfs_admkey_u.mcgkey.primary,&mk);
		mcg_block_encrypt(&mk,&(ek[0]));
		mcg_block_encrypt(&mk,&(ek[8]));
		break;
	    case CFS_SAFER_SK128:
		Safer_Init_Module();
		Safer_Expand_Userkey(k->cfs_admkey_u.saferkey.primary,
				     &(k->cfs_admkey_u.saferkey.primary[8]),
				     SAFER_SK128_DEFAULT_NOF_ROUNDS, 1,
				     sk);
		Safer_Encrypt_Block(&(ek[0]),sk,&(ek[0]));
		Safer_Encrypt_Block(&(ek[8]),sk,&(ek[8]));
		break;
	    default:
		break;
	}
}
