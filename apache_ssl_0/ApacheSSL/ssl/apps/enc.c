/* apps/enc.c */
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
#include <unistd.h>
#include "envelope.h"
#include "objects.h"
#include "x509.h"
#include "md5.h"
#include "pem.h"

#define SIZE	(20*1024)
#define BSIZE	(32*1024)

#define	PROG	enc_main

#if defined(MSDOS) || defined(_Windows)
#define EXIT(n)         return(n);
#define LIST_SEPERATOR_CHAR ';'
#include <io.h>
#else
#ifndef MONOLITH
#define EXIT(n)         exit(n); return(n)
#else
#define EXIT(n)         return(n)
#endif
#endif

#ifndef MONOLITH
#define MAIN(a,v)       main(a,v)
#else
#define MAIN(a,v)       PROG(a,v)
#endif

int MAIN(argc,argv)
int argc;
char **argv;
	{
	EVP_ENCODE_CTX basein,baseout;
	EVP_CIPHER_CTX p;
	unsigned char b1[BSIZE],b2[BSIZE];
	unsigned char *buff=b1,*buff2=b2,*btmp;
	int ret=1,inl,outl,donesome=0;
	unsigned char key[24],*str=NULL,iv[MD5_DIGEST_LENGTH];
	int enc=1,printkey=0,i,base64=0,done=0;
	EVP_CIPHER *cipher=NULL,*c;
	char *inf=NULL,*outf=NULL;
	FILE *in=NULL,*out=NULL;

	cipher=EVP_get_cipherbyname(argv[0]);

	argc--;
	argv++;
	while (argc >= 1)
		{
		if	(strcmp(*argv,"-e") == 0)
			enc=1;
		else if (strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			inf= *(++argv);
			}
		else if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outf= *(++argv);
			}
		else if	(strcmp(*argv,"-d") == 0)
			enc=0;
		else if	(strcmp(*argv,"-p") == 0)
			printkey=1;
		else if	(strcmp(*argv,"-P") == 0)
			printkey=2;
		else if	(strcmp(*argv,"-a") == 0)
			base64=1;
		else if (strcmp(*argv,"-k") == 0)
			{
			if (--argc < 1) goto bad;
			str=(unsigned char *)*(++argv);
			}
		else if	((argv[0][0] == '-') &&
			((c=EVP_get_cipherbyname(&(argv[0][1]))) != NULL))
			{
			cipher=c;
			}
		else
			{
			fprintf(stderr,"unknown option '%s'\n",*argv);
bad:
			fprintf(stderr,"options are\n");
			fprintf(stderr,"%-14s input file\n","-in <file>");
			fprintf(stderr,"%-14s output fileencrypt\n","-out <file>");
			fprintf(stderr,"%-14s encrypt\n","-e");
			fprintf(stderr,"%-14s decrypt\n","-d");
			fprintf(stderr,"%-14s base64 encode/decode, depending on encryption flag\n","-a");
			fprintf(stderr,"%-14s key is next argument\n","-k");
			fprintf(stderr,"%-14s print the iv/key (then exit if -P)\n","-[pP]");

			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_ecb, LN_des_ecb);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_cfb, LN_des_cfb);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_ofb, LN_des_ofb);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_cbc, LN_des_cbc);
			fprintf(stderr,"-%-13s use %s encryption\n",
				"des", LN_des_cbc);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_ede, LN_des_ede);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_ede3, LN_des_ede3);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_ede_cbc, LN_des_ede_cbc);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_des_ede3_cbc, LN_des_ede3_cbc);
			fprintf(stderr,"-%-13s use %s encryption\n",
				"des3", LN_des_ede3_cbc);
#ifndef NORC4
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_rc4, LN_rc4);
#endif
#ifndef NOIDEA
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_idea_ecb, LN_idea_ecb);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_idea_cfb, LN_idea_cfb);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_idea_ofb, LN_idea_ofb);
			fprintf(stderr,"-%-13s use %s encryption\n",
				LN_idea_cbc, LN_idea_cbc);
			fprintf(stderr,"-%-13s use %s encryption\n",
				"idea", LN_idea_cbc);
#endif
			goto end;
			}
		argc--;
		argv++;
		}

	if (inf == NULL)
		in=stdin;
	else
		{
		in=fopen(inf,"r");
		if (in == NULL) { perror(inf); goto end; }
		}
	if (outf == NULL)
		out=stdout;
	else
		{
		out=fopen(outf,"w");
		if (out == NULL) { perror(outf); goto end; }
		}

	if ((str == NULL) && (cipher != NULL))
		{
		for (;;)
			{
			char buf[200];

			sprintf(buf,"enter %s %s password:",
				OBJ_nid2ln(cipher->type),
				(enc)?"encryption":"decryption");
			i=EVP_read_pw_string((char *)buff,SIZE,buf,enc);
			if (i == 0)
				{
				if (buff[0] == '\0') exit(0);
				str=buff;
				break;
				}
			if (i < 0)
				{
				fprintf(stderr,"bad password read\n");
				goto end;
				}
			}
		}

	if (cipher != NULL)
		{
		/* generate a salt */
		EVP_BytesToKey(cipher,EVP_md5,NULL,str,strlen((char *)str),
			1,key,iv);
		EVP_CipherInit(&p,cipher,key,iv,enc);

		if (printkey)
			{
			if (cipher->key_len > 0)
				{
				printf("key=");
				for (i=0; i<cipher->key_len; i++)
					printf("%02X",key[i]);
				printf("\n");
				}
			if (cipher->iv_len > 0)
				{
				printf("iv =");
				for (i=0; i<cipher->iv_len; i++)
					printf("%02X",iv[i]);
				printf("\n");
				}
			if (printkey == 2)
				{
				ret=0;
				goto end;
				}
			}
		}

	if (base64)
		{
		if (enc)
			EVP_EncodeInit(&baseout);
		else
			EVP_DecodeInit(&basein);
		}

	for (;;)
		{
		inl=read(fileno(in),buff,SIZE);
		if (inl == 0) break;

		if (base64 && !enc)
			{
			i=EVP_DecodeUpdate(&basein,buff2,&inl,buff,inl);
			if (i < 0)
				{
				if (donesome)
					break;
				else
					{
					EVP_DecodeInit(&baseout);
					continue;
					}
				}
			donesome=1;
			if (i == 0) done=1;
			btmp=buff2; buff2=buff; buff=btmp;
			}

		outl=inl;
		if (cipher != NULL)
			{
			EVP_CipherUpdate(&p,buff2,&outl,buff,inl);
			btmp=buff2; buff2=buff; buff=btmp;
			}

		if (base64 && enc)
			{
			EVP_EncodeUpdate(&baseout,buff2,&outl,buff,outl);
			btmp=buff2; buff2=buff; buff=btmp;
			}

		if (outl != 0)
			write(fileno(out),buff,outl);
		if (done) break;
		}

	outl=0;
	if (cipher != NULL)
		{
		if (!EVP_CipherFinal(&p,buff,&outl))
			{
			fprintf(stderr,"error in EVP_CipherFinal\n");
			goto end;
			}
		if (base64 && enc)
			{
			EVP_EncodeUpdate(&baseout,buff2,&outl,buff,outl);
			btmp=buff2; buff2=buff; buff=btmp;
			if (outl != 0)
				write(fileno(out),buff,outl);
			outl=0;
			}
		}

	if (base64 && enc)
		{
		EVP_EncodeFinal(&baseout,buff2,&outl);
		btmp=buff2; buff2=buff; buff=btmp;
		}

	if (outl != 0)
		write(fileno(out),buff,outl);

	ret=0;
end:
	if ((in != NULL) && (in != stdin)) fclose(in);
	if ((out != NULL) && (out != stdout)) fclose(out);
	EXIT(ret);
	}
