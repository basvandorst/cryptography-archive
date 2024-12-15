/* lib/pem/pem_lib.c */
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
#include "crypto.h"
#include "objects.h"
#include "rsa.h"
#include "envelope.h"
#include "rand.h"
#include "x509.h"
#include "pem.h"
#include "buffer.h"
#include "des.h"

char *PEM_version="\0PEM part of SSLeay v 0.5.1 21/12/95";

#define MIN_LENGTH	4

#ifndef NOPROTO
static void PEM_proc_type(char *buf, int type);
static void PEM_dek_info(char *buf, char *type, int len, char *str);
static int def_callback(char *buf, int num, int w);
static int load_iv(unsigned char **fromp,unsigned char *to, int num);
#else
static void PEM_proc_type();
static void PEM_dek_info();
static int def_callback();
static int load_iv();
#endif

static int def_callback(buf, num, w)
char *buf;
int num;
int w;
	{
	int i,j;

#ifdef _Windows
	/* We should not ever call the default callback routine from
	 * windows. */
	PEMerr(PEM_F_DEF_CALLBACK,ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
	return(-1);
#else

	for (;;)
		{
		i=EVP_read_pw_string(buf,num,"Enter PEM pass phrase:",w);
		if (i != 0)
			{
			Fprintf(stderr,"problems getting password\n");
			memset(buf,0,(unsigned int)num);
			return(-1);
			}
		j=strlen(buf);
		if (j < MIN_LENGTH)
			{
			Fprintf(stderr,"phrase is too short, needs to be at least %d chars\n",MIN_LENGTH);
			}
		else
			break;
		}
	return(j);
#endif
	}

static void PEM_proc_type(buf, type)
char *buf;
int type;
	{
	char *str;

	if (type == PEM_TYPE_ENCRYPTED)
		str="ENCRYPTED";
	else if (type == PEM_TYPE_ENCRYPTED)
		str="MIC-CLEAR";
	else if (type == PEM_TYPE_MIC_ONLY)
		str="MIC-ONLY";
	else
		str="BAD-TYPE";
		
	strcat(buf,"Proc-Type: 4,");
	strcat(buf,str);
	strcat(buf,"\n");
	}

static void PEM_dek_info(buf, type, len, str)
char *buf;
char *type;
int len;
char *str;
	{
	static unsigned char map[17]="0123456789ABCDEF";
	long i;
	int j;

	strcat(buf,"DEK-Info: ");
	strcat(buf,type);
	strcat(buf,",");
	j=strlen(buf);
	for (i=0; i<len; i++)
		{
		buf[j+i*2]  =map[(str[i]>>4)&0x0f];
		buf[j+i*2+1]=map[(str[i]   )&0x0f];
		}
	buf[j+i*2]='\n';
	buf[j+i*2+1]='\0';
	}

char *PEM_ASN1_read(d2i,name,fp, x, cb)
char *(*d2i)();
char *name;
FILE *fp;
char **x;
int (*cb)();
	{
	EVP_CIPHER_INFO cipher;
	char *nm=NULL,*header=NULL;
	unsigned char *p=NULL,*data=NULL;
	long len;
	char *ret=NULL;

	for (;;)
		{
		if (!PEM_read(fp,&nm,&header,&data,&len)) return(0);
		if (strcmp(nm,name) == 0) break;
		free(nm);
		free(header);
		free(data);
		}
	if (!PEM_get_EVP_CIPHER_INFO(header,&cipher)) goto err;
	if (!PEM_do_header(&cipher,data,&len,cb)) goto err;
	p=data;
	ret=d2i(x,&p,len);
	if (ret == NULL)
		PEMerr(PEM_F_PEM_ASN1_READ,ERR_R_ASN1_LIB);
err:
	free(nm);
	free(header);
	free(data);
	return(ret);
	}

int PEM_ASN1_write(i2d,name,fp, x, enc, kstr, klen, callback)
int (*i2d)();
char *name;
FILE *fp;
char *x;
EVP_CIPHER *enc;
unsigned char *kstr;
int klen;
int (*callback)();
	{
	EVP_CIPHER_CTX ctx;
	int dsize=0,i,j,k,ret=0;
	unsigned char *p,*data=NULL;
	char *objstr=NULL;
#define PEM_BUFSIZE	1024
	char buf[PEM_BUFSIZE];
	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH];
	
	if (enc != NULL)
		{
		objstr=OBJ_nid2sn(enc->type);
		if (objstr == NULL)
			{
			PEMerr(PEM_F_PEM_WRITE_RSA,PEM_R_UNSUPPORTED_CIPHER);
			goto err;
			}
		}

	dsize=i2d(x,NULL);
	/* dzise + 8 bytes are needed */
	data=(unsigned char *)malloc((unsigned int)dsize+20);
	if (data == NULL)
		{
		PEMerr(PEM_F_PEM_WRITE_RSA,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	p=data;
	i=i2d(x,&p);

	if (enc != NULL)
		{
		if (kstr == NULL)
			{
			if (callback == NULL)
				klen=def_callback(buf,PEM_BUFSIZE,1);
			else
				klen=(*callback)(buf,PEM_BUFSIZE,1);
			if (klen <= 0)
				{
				PEMerr(PEM_F_PEM_WRITE_RSA,PEM_R_READ_KEY);
				goto err;
				}
			kstr=(unsigned char *)buf;
			}
		RAND_seed(data,i);/* put in the RSA key. */
		RAND_bytes(iv,8);	/* Generate a salt */
		/* The 'iv' is used as the iv and as a salt.  It is
		 * NOT taken from the BytesToKey function */
		EVP_BytesToKey(enc,EVP_md5,iv,kstr,klen,1,key,NULL);

		if (kstr == (unsigned char *)buf) memset(buf,0,PEM_BUFSIZE);

		buf[0]='\0';
		PEM_proc_type(buf,PEM_TYPE_ENCRYPTED);
		PEM_dek_info(buf,objstr,8,(char *)iv);
		k=strlen(buf);
	
		EVP_EncryptInit(&ctx,enc,key,iv);
		EVP_EncryptUpdate(&ctx,data,&j,data,i);
		EVP_EncryptFinal(&ctx,&(data[j]),&i);
		i+=j;
		ret=1;
		}
	else
		{
		ret=1;
		buf[0]='\0';
		}
	i=PEM_write(fp,name,buf,data,i);
	if (i <= 0) ret=0;
err:
	memset(key,0,sizeof(key));
	memset(iv,0,sizeof(iv));
	memset((char *)&ctx,0,sizeof(ctx));
	memset(buf,0,PEM_BUFSIZE);
	memset(data,0,(unsigned int)dsize);
	free(data);
	return(ret);
	}

int PEM_do_header(cipher, data, plen, callback)
EVP_CIPHER_INFO *cipher;
unsigned char *data;
long *plen;
int (*callback)();
	{
	int i,j,o,klen;
	long len;
	EVP_CIPHER_CTX ctx;
	unsigned char key[EVP_MAX_KEY_LENGTH];
	char buf[PEM_BUFSIZE];

	len= *plen;

	if (cipher->cipher == NULL) return(1);
	if (callback == NULL)
		klen=def_callback(buf,PEM_BUFSIZE,0);
	else
		klen=callback(buf,PEM_BUFSIZE,0);
	if (klen <= 0)
		{
		PEMerr(PEM_F_PEM_DO_HEADER,PEM_R_BAD_PASSWORD_READ);
		return(0);
		}
	EVP_BytesToKey(cipher->cipher,EVP_md5,&(cipher->iv[0]),
		(unsigned char *)buf,klen,1,key,NULL);

	j=len;
	EVP_DecryptInit(&ctx,cipher->cipher,key,&(cipher->iv[0]));
	EVP_DecryptUpdate(&ctx,data,&i,data,j);
	o=EVP_DecryptFinal(&ctx,&(data[i]),&j);
	j+=i;
	if (!o)
		{
		PEMerr(PEM_F_PEM_DO_HEADER,PEM_R_BAD_DECRYPT);
		return(0);
		}
	*plen=j;
	return(1);
	}

int PEM_get_EVP_CIPHER_INFO(header,cipher)
char *header;
EVP_CIPHER_INFO *cipher;
	{
	int o;
	EVP_CIPHER *enc=NULL;
	char *p,c;

	cipher->cipher=NULL;
	if ((header == NULL) || (*header == '\0') || (*header == '\n'))
		return(1);
	if (strncmp(header,"Proc-Type: ",11) != 0)
		{ PEMerr(PEM_F_PEM_EVP_CIPHER_INFO,PEM_R_NOT_PROC_TYPE); return(0); }
	header+=11;
	if (*header != '4') return(0); header++;
	if (*header != ',') return(0); header++;
	if (strncmp(header,"ENCRYPTED",9) != 0)
		{ PEMerr(PEM_F_PEM_EVP_CIPHER_INFO,PEM_R_NOT_ENCRYPTED); return(0); }
	for (; (*header != '\n') && (*header != '\0'); header++)
		;
	if (*header == '\0')
		{ PEMerr(PEM_F_PEM_EVP_CIPHER_INFO,PEM_R_SHORT_HEADER); return(0); }
	header++;
	if (strncmp(header,"DEK-Info: ",10) != 0)
		{ PEMerr(PEM_F_PEM_EVP_CIPHER_INFO,PEM_R_NOT_DEK_INFO); return(0); }
	header+=10;

	p=header;
	for (;;)
		{
		c= *header;
		if (!(	((c >= 'A') && (c <= 'Z')) || (c == '-') ||
			((c >= '0') && (c <= '9'))))
			break;
		header++;
		}
	*header='\0';
	o=OBJ_sn2nid(p);
	*header=c;
	header++;

	switch (o)
		{
	case NID_des_cbc:
		cipher->cipher=enc=EVP_des_cbc;
		break;
	case NID_des_ede3_cbc:
		cipher->cipher=EVP_des_ede3_cbc;
		break;
#ifndef NOIDEA
	case NID_idea_cbc:
		cipher->cipher=EVP_idea_cbc;
		break;
#endif
	default:
		PEMerr(PEM_F_PEM_EVP_CIPHER_INFO,PEM_R_UNSUPPORTED_ENCRYPTION);
		return(0);
		}
	if (!load_iv((unsigned char **)&header,&(cipher->iv[0]),8)) return(0);

	return(1);
	}

static int load_iv(fromp,to,num)
unsigned char **fromp,*to;
int num;
	{
	int v,i;
	unsigned char *from;

	from= *fromp;
	for (i=0; i<num; i++) to[i]=0;
	num*=2;
	for (i=0; i<num; i++)
		{
		if ((*from >= '0') && (*from <= '9'))
			v= *from-'0';
		else if ((*from >= 'A') && (*from <= 'F'))
			v= *from-'A'+10;
		else if ((*from >= 'a') && (*from <= 'f'))
			v= *from-'a'+10;
		else
			{
			PEMerr(PEM_F_LOAD_IV,PEM_R_BAD_IV_CHARS);
			return(0);
			}
		from++;
		to[i/2]|=v<<(long)((!(i&1))*4);
		}

	*fromp=from;
	return(1);
	}

int PEM_write(fp, name, header, data,len)
FILE *fp;
char *name;
char *header;
unsigned char *data;
long len;
	{
	int n,i,outl;
	unsigned char *buf;
	EVP_ENCODE_CTX ctx;
	
	EVP_EncodeInit(&ctx);

	Fprintf(fp,"-----BEGIN %s-----\n",name);
	Fwrite(header,strlen(header),1,fp);
	if (header[0] != '\0')
		Fprintf(fp,"\n");

	buf=(unsigned char *)malloc(PEM_BUFSIZE*8);
	if (buf == NULL)
		{
		PEMerr(PEM_F_PEM_WRITE,ERR_R_MALLOC_FAILURE);
		return(0);
		}

	i=0;
	while (len > 0)
		{
		n=((len>(PEM_BUFSIZE*5))?(PEM_BUFSIZE*5):len);
		EVP_EncodeUpdate(&ctx,buf,&outl,&(data[i]),n);
		if (outl)
			Fwrite(buf,(unsigned int)outl,1,fp);
		i+=outl;
		len-=n;
		}
	EVP_EncodeFinal(&ctx,buf,&outl);
	if (outl > 0)
		Fwrite(buf,(unsigned int)outl,1,fp);
	free(buf);
	Fprintf(fp,"-----END %s-----\n",name);
	return(i+outl);
	}

int PEM_read(fp, name, header, data, len)
FILE *fp;
char **name;
char **header;
unsigned char **data;
long *len;
	{
	EVP_ENCODE_CTX ctx;
	int end=0,i,k,bl=0,hl=0,nohead=0;
	char buf[256];
	BUFFER *nameB;
	BUFFER *headerB;
	BUFFER *dataB,*tmpB;
	
	nameB=BUF_new();
	headerB=BUF_new();
	dataB=BUF_new();
	if ((nameB == NULL) || (headerB == NULL) || (dataB == NULL))
		{
		PEMerr(PEM_F_PEM_READ,ERR_R_MALLOC_FAILURE);
		return(0);
		}

	for (;;)
		{
		buf[0]='\0';
		Fgets(buf,256,fp);

		if (buf[0] == '\0')
			{
			PEMerr(PEM_F_PEM_READ,PEM_R_NO_START_LINE);
			goto err;
			}
		if (strncmp(buf,"-----BEGIN ",11) == 0)
			{
			i=strlen(&(buf[11]));
			if (strncmp(&(buf[11+i-6]),"-----\n",6) != 0)
				continue;
			if (!BUF_grow(nameB,i+9))
				{
				PEMerr(PEM_F_PEM_READ,ERR_R_MALLOC_FAILURE);
				goto err;
				}
			strncpy(nameB->data,&(buf[11]),(unsigned int)i-6);
			nameB->data[i-6]='\0';
			break;
			}
		}
	hl=0;
	if (!BUF_grow(headerB,256))
		{ PEMerr(PEM_F_PEM_READ,ERR_R_MALLOC_FAILURE); goto err; }
	headerB->data[0]='\0';
	for (;;)
		{
		buf[0]='\0';
		Fgets(buf,256,fp);
		if (buf[0] == '\0') break;
		if (buf[0] == '\n') break;
		i=strlen(buf);
		if (!BUF_grow(headerB,hl+i+9))
			{ PEMerr(PEM_F_PEM_READ,ERR_R_MALLOC_FAILURE); goto err; }
		if (strncmp(buf,"-----END ",9) == 0)
			{
			nohead=1;
			break;
			}
		strncpy(&(headerB->data[hl]),buf,(unsigned int)i);
		headerB->data[hl+i]='\0';
		hl+=i;
		}

	bl=0;
	if (!BUF_grow(dataB,1024))
		{ PEMerr(PEM_F_PEM_READ,ERR_R_MALLOC_FAILURE); goto err; }
	dataB->data[0]='\0';
	if (!nohead)
		{
		for (;;)
			{
			buf[0]='\0';
			Fgets(buf,256,fp);
			if (buf[0] == '\0') break;
			i=strlen(buf);
			if (i != 65) end=1;
			if (strncmp(buf,"-----END ",9) == 0)
				break;
			if (i > 65) break;
			if (!BUF_grow(dataB,i+bl+9))
				{
				PEMerr(PEM_F_PEM_READ,ERR_R_MALLOC_FAILURE);
				goto err;
				}
			strncpy(&(dataB->data[bl]),buf,(unsigned int)i);
			dataB->data[bl+i]='\0';
			bl+=i;
			if (end)
				{
				buf[0]='\0';
				Fgets(buf,256,fp);
				break;
				}
			}
		}
	else
		{
		tmpB=headerB;
		headerB=dataB;
		dataB=tmpB;
		bl=hl;
		}
	i=strlen(nameB->data);
	if (	(strncmp(buf,"-----END ",9) != 0) ||
		(strncmp(nameB->data,&(buf[9]),(unsigned int)i) != 0) ||
		(strncmp(&(buf[9+i]),"-----\n",6) != 0))
		{
		PEMerr(PEM_F_PEM_READ,PEM_R_BAD_END_LINE);
		goto err;
		}
	
	EVP_DecodeInit(&ctx);
	i=EVP_DecodeUpdate(&ctx,
		(unsigned char *)dataB->data,&bl,
		(unsigned char *)dataB->data,bl);
	if (i < 0) { PEMerr(PEM_F_PEM_READ,PEM_R_BAD_BASE64_DECODE); goto err; }
	i=EVP_DecodeFinal(&ctx,(unsigned char *)&(dataB->data[bl]),&k);
	if (i < 0) { PEMerr(PEM_F_PEM_READ,PEM_R_BAD_BASE64_DECODE); goto err; }
	bl+=k;

	if (bl == 0) goto err;
	*name=nameB->data;
	*header=headerB->data;
	*data=(unsigned char *)dataB->data;
	*len=bl;
	free(nameB);
	free(headerB);
	free(dataB);
	return(1);
err:
	BUF_free(nameB);
	BUF_free(headerB);
	BUF_free(dataB);
	return(0);
	}

