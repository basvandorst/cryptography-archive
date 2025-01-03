/* crypto/pem/pem_lib.c */
/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
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
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
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
#include "cryptlib.h"
#include "buffer.h"
#include "objects.h"
#include "evp.h"
#include "rand.h"
#include "x509.h"
#include "pem.h"
#ifndef NO_DES
#include "des.h"
#endif

char *PEM_version="PEM part of SSLeay 0.8.1b 29-Jun-1998";

#define MIN_LENGTH	4

/* PEMerr(PEM_F_PEM_WRITE_BIO,ERR_R_MALLOC_FAILURE);
 * PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE);
 */

#ifndef NOPROTO
static int def_callback(char *buf, int num, int w);
static int load_iv(unsigned char **fromp,unsigned char *to, int num);
#else
static int def_callback();
static int load_iv();
#endif

static int def_callback(buf, num, w)
char *buf;
int num;
int w;
	{
#ifdef WIN16
	/* We should not ever call the default callback routine from
	 * windows. */
	PEMerr(PEM_F_DEF_CALLBACK,ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
	return(-1);
#else
	int i,j;
	char *prompt;

	prompt=EVP_get_pw_prompt();
	if (prompt == NULL)
		prompt="Enter PEM pass phrase:";

	for (;;)
		{
		i=EVP_read_pw_string(buf,num,prompt,w);
		if (i != 0)
			{
			PEMerr(PEM_F_DEF_CALLBACK,PEM_R_PROBLEMS_GETTING_PASSWORD);
			memset(buf,0,(unsigned int)num);
			return(-1);
			}
		j=strlen(buf);
		if (j < MIN_LENGTH)
			{
			fprintf(stderr,"phrase is too short, needs to be at least %d chars\n",MIN_LENGTH);
			}
		else
			break;
		}
	return(j);
#endif
	}

void PEM_proc_type(buf, type)
char *buf;
int type;
	{
	char *str;

	if (type == PEM_TYPE_ENCRYPTED)
		str="ENCRYPTED";
	else if (type == PEM_TYPE_MIC_CLEAR)
		str="MIC-CLEAR";
	else if (type == PEM_TYPE_MIC_ONLY)
		str="MIC-ONLY";
	else
		str="BAD-TYPE";
		
	strcat(buf,"Proc-Type: 4,");
	strcat(buf,str);
	strcat(buf,"\n");
	}

void PEM_dek_info(buf, type, len, str)
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

#ifndef WIN16
char *PEM_ASN1_read(d2i,name,fp, x, cb)
char *(*d2i)();
char *name;
FILE *fp;
char **x;
int (*cb)();
	{
        BIO *b;
        char *ret;

        if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		PEMerr(PEM_F_PEM_ASN1_READ,ERR_R_BUF_LIB);
                return(0);
		}
        BIO_set_fp(b,fp,BIO_NOCLOSE);
        ret=PEM_ASN1_read_bio(d2i,name,b,x,cb);
        BIO_free(b);
        return(ret);
	}
#endif

char *PEM_ASN1_read_bio(d2i,name,bp, x, cb)
char *(*d2i)();
char *name;
BIO *bp;
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
		if (!PEM_read_bio(bp,&nm,&header,&data,&len)) return(NULL);
		if (	(strcmp(nm,name) == 0) ||
			((strcmp(nm,PEM_STRING_RSA) == 0) &&
			 (strcmp(name,PEM_STRING_EVP_PKEY) == 0)) ||
			((strcmp(nm,PEM_STRING_DSA) == 0) &&
			 (strcmp(name,PEM_STRING_EVP_PKEY) == 0)) ||
			((strcmp(nm,PEM_STRING_X509_OLD) == 0) &&
			 (strcmp(name,PEM_STRING_X509) == 0)) ||
			((strcmp(nm,PEM_STRING_X509_REQ_OLD) == 0) &&
			 (strcmp(name,PEM_STRING_X509_REQ) == 0))
			)
			break;
		Free(nm);
		Free(header);
		Free(data);
		}
	if (!PEM_get_EVP_CIPHER_INFO(header,&cipher)) goto err;
	if (!PEM_do_header(&cipher,data,&len,cb)) goto err;
	p=data;
	if (strcmp(name,PEM_STRING_EVP_PKEY) == 0)
		{
		if (strcmp(nm,PEM_STRING_RSA) == 0)
			ret=d2i(EVP_PKEY_RSA,x,&p,len);
		else if (strcmp(nm,PEM_STRING_DSA) == 0)
			ret=d2i(EVP_PKEY_DSA,x,&p,len);
		}
	else	
		ret=d2i(x,&p,len);
	if (ret == NULL)
		PEMerr(PEM_F_PEM_ASN1_READ_BIO,ERR_R_ASN1_LIB);
err:
	Free(nm);
	Free(header);
	Free(data);
	return(ret);
	}

#ifndef WIN16
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
        BIO *b;
        int ret;

        if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		PEMerr(PEM_F_PEM_ASN1_WRITE,ERR_R_BUF_LIB);
                return(0);
		}
        BIO_set_fp(b,fp,BIO_NOCLOSE);
        ret=PEM_ASN1_write_bio(i2d,name,b,x,enc,kstr,klen,callback);
        BIO_free(b);
        return(ret);
        }
#endif

int PEM_ASN1_write_bio(i2d,name,bp, x, enc, kstr, klen, callback)
int (*i2d)();
char *name;
BIO *bp;
char *x;
EVP_CIPHER *enc;
unsigned char *kstr;
int klen;
int (*callback)();
	{
	EVP_CIPHER_CTX ctx;
	int dsize=0,i,j,ret=0;
	unsigned char *p,*data=NULL;
	char *objstr=NULL;
#define PEM_BUFSIZE	1024
	char buf[PEM_BUFSIZE];
	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH];
	
	if (enc != NULL)
		{
		objstr=OBJ_nid2sn(EVP_CIPHER_nid(enc));
		if (objstr == NULL)
			{
			PEMerr(PEM_F_PEM_ASN1_WRITE_BIO,PEM_R_UNSUPPORTED_CIPHER);
			goto err;
			}
		}

	dsize=i2d(x,NULL);
	/* dzise + 8 bytes are needed */
	data=(unsigned char *)Malloc((unsigned int)dsize+20);
	if (data == NULL)
		{
		PEMerr(PEM_F_PEM_ASN1_WRITE_BIO,ERR_R_MALLOC_FAILURE);
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
				PEMerr(PEM_F_PEM_ASN1_WRITE_BIO,PEM_R_READ_KEY);
				goto err;
				}
			kstr=(unsigned char *)buf;
			}
		RAND_seed(data,i);/* put in the RSA key. */
		RAND_bytes(iv,8);	/* Generate a salt */
		/* The 'iv' is used as the iv and as a salt.  It is
		 * NOT taken from the BytesToKey function */
		EVP_BytesToKey(enc,EVP_md5(),iv,kstr,klen,1,key,NULL);

		if (kstr == (unsigned char *)buf) memset(buf,0,PEM_BUFSIZE);

		buf[0]='\0';
		PEM_proc_type(buf,PEM_TYPE_ENCRYPTED);
		PEM_dek_info(buf,objstr,8,(char *)iv);
		/* k=strlen(buf); */
	
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
	i=PEM_write_bio(bp,name,buf,data,i);
	if (i <= 0) ret=0;
err:
	memset(key,0,sizeof(key));
	memset(iv,0,sizeof(iv));
	memset((char *)&ctx,0,sizeof(ctx));
	memset(buf,0,PEM_BUFSIZE);
	memset(data,0,(unsigned int)dsize);
	Free(data);
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
	EVP_BytesToKey(cipher->cipher,EVP_md5(),&(cipher->iv[0]),
		(unsigned char *)buf,klen,1,key,NULL);

	j=(int)len;
	EVP_DecryptInit(&ctx,cipher->cipher,key,&(cipher->iv[0]));
	EVP_DecryptUpdate(&ctx,data,&i,data,j);
	o=EVP_DecryptFinal(&ctx,&(data[i]),&j);
	EVP_CIPHER_CTX_cleanup(&ctx);
	memset((char *)buf,0,sizeof(buf));
	memset((char *)key,0,sizeof(key));
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
		{ PEMerr(PEM_F_PEM_GET_EVP_CIPHER_INFO,PEM_R_NOT_PROC_TYPE); return(0); }
	header+=11;
	if (*header != '4') return(0); header++;
	if (*header != ',') return(0); header++;
	if (strncmp(header,"ENCRYPTED",9) != 0)
		{ PEMerr(PEM_F_PEM_GET_EVP_CIPHER_INFO,PEM_R_NOT_ENCRYPTED); return(0); }
	for (; (*header != '\n') && (*header != '\0'); header++)
		;
	if (*header == '\0')
		{ PEMerr(PEM_F_PEM_GET_EVP_CIPHER_INFO,PEM_R_SHORT_HEADER); return(0); }
	header++;
	if (strncmp(header,"DEK-Info: ",10) != 0)
		{ PEMerr(PEM_F_PEM_GET_EVP_CIPHER_INFO,PEM_R_NOT_DEK_INFO); return(0); }
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
	cipher->cipher=enc=EVP_get_cipherbyname(p);
	*header=c;
	header++;

	if (enc == NULL)
		{
		PEMerr(PEM_F_PEM_GET_EVP_CIPHER_INFO,PEM_R_UNSUPPORTED_ENCRYPTION);
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

#ifndef WIN16
int PEM_write(fp, name, header, data,len)
FILE *fp;
char *name;
char *header;
unsigned char *data;
long len;
        {
        BIO *b;
        int ret;

        if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		PEMerr(PEM_F_PEM_WRITE,ERR_R_BUF_LIB);
                return(0);
		}
        BIO_set_fp(b,fp,BIO_NOCLOSE);
        ret=PEM_write_bio(b, name, header, data,len);
        BIO_free(b);
        return(ret);
        }
#endif

int PEM_write_bio(bp, name, header, data,len)
BIO *bp;
char *name;
char *header;
unsigned char *data;
long len;
	{
	int nlen,n,i,j,outl;
	unsigned char *buf;
	EVP_ENCODE_CTX ctx;
	int reason=ERR_R_BUF_LIB;
	
	EVP_EncodeInit(&ctx);
	nlen=strlen(name);

	if (	(BIO_write(bp,"-----BEGIN ",11) != 11) ||
		(BIO_write(bp,name,nlen) != nlen) ||
		(BIO_write(bp,"-----\n",6) != 6))
		goto err;
		
	i=strlen(header);
	if (i > 0)
		{
		if (	(BIO_write(bp,header,i) != i) ||
			(BIO_write(bp,"\n",1) != 1))
			goto err;
		}

	buf=(unsigned char *)Malloc(PEM_BUFSIZE*8);
	if (buf == NULL)
		{
		reason=ERR_R_MALLOC_FAILURE;
		goto err;
		}

	i=j=0;
	while (len > 0)
		{
		n=(int)((len>(PEM_BUFSIZE*5))?(PEM_BUFSIZE*5):len);
		EVP_EncodeUpdate(&ctx,buf,&outl,&(data[j]),n);
		if ((outl) && (BIO_write(bp,(char *)buf,outl) != outl))
			goto err;
		i+=outl;
		len-=n;
		j+=n;
		}
	EVP_EncodeFinal(&ctx,buf,&outl);
	if ((outl > 0) && (BIO_write(bp,(char *)buf,outl) != outl)) goto err;
	Free(buf);
	if (	(BIO_write(bp,"-----END ",9) != 9) ||
		(BIO_write(bp,name,nlen) != nlen) ||
		(BIO_write(bp,"-----\n",6) != 6))
		goto err;
	return(i+outl);
err:
	PEMerr(PEM_F_PEM_WRITE_BIO,reason);
	return(0);
	}

#ifndef WIN16
int PEM_read(fp, name, header, data,len)
FILE *fp;
char **name;
char **header;
unsigned char **data;
long *len;
        {
        BIO *b;
        int ret;

        if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		PEMerr(PEM_F_PEM_READ,ERR_R_BUF_LIB);
                return(0);
		}
        BIO_set_fp(b,fp,BIO_NOCLOSE);
        ret=PEM_read_bio(b, name, header, data,len);
        BIO_free(b);
        return(ret);
        }
#endif

int PEM_read_bio(bp, name, header, data, len)
BIO *bp;
char **name;
char **header;
unsigned char **data;
long *len;
	{
	EVP_ENCODE_CTX ctx;
	int end=0,i,k,bl=0,hl=0,nohead=0;
	char buf[256];
	BUF_MEM *nameB;
	BUF_MEM *headerB;
	BUF_MEM *dataB,*tmpB;
	
	nameB=BUF_MEM_new();
	headerB=BUF_MEM_new();
	dataB=BUF_MEM_new();
	if ((nameB == NULL) || (headerB == NULL) || (dataB == NULL))
		{
		PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE);
		return(0);
		}

	buf[254]='\0';
	for (;;)
		{
		i=BIO_gets(bp,buf,254);

		if (i <= 0)
			{
			PEMerr(PEM_F_PEM_READ_BIO,PEM_R_NO_START_LINE);
			goto err;
			}

		while ((i >= 0) && (buf[i] <= ' ')) i--;
		buf[++i]='\n'; buf[++i]='\0';

		if (strncmp(buf,"-----BEGIN ",11) == 0)
			{
			i=strlen(&(buf[11]));

			if (strncmp(&(buf[11+i-6]),"-----\n",6) != 0)
				continue;
			if (!BUF_MEM_grow(nameB,i+9))
				{
				PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE);
				goto err;
				}
			strncpy(nameB->data,&(buf[11]),(unsigned int)i-6);
			nameB->data[i-6]='\0';
			break;
			}
		}
	hl=0;
	if (!BUF_MEM_grow(headerB,256))
		{ PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE); goto err; }
	headerB->data[0]='\0';
	for (;;)
		{
		i=BIO_gets(bp,buf,254);
		if (i <= 0) break;

		while ((i >= 0) && (buf[i] <= ' ')) i--;
		buf[++i]='\n'; buf[++i]='\0';

		if (buf[0] == '\n') break;
		if (!BUF_MEM_grow(headerB,hl+i+9))
			{ PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE); goto err; }
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
	if (!BUF_MEM_grow(dataB,1024))
		{ PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE); goto err; }
	dataB->data[0]='\0';
	if (!nohead)
		{
		for (;;)
			{
			i=BIO_gets(bp,buf,254);
			if (i <= 0) break;

			while ((i >= 0) && (buf[i] <= ' ')) i--;
			buf[++i]='\n'; buf[++i]='\0';

			if (i != 65) end=1;
			if (strncmp(buf,"-----END ",9) == 0)
				break;
			if (i > 65) break;
			if (!BUF_MEM_grow(dataB,i+bl+9))
				{
				PEMerr(PEM_F_PEM_READ_BIO,ERR_R_MALLOC_FAILURE);
				goto err;
				}
			strncpy(&(dataB->data[bl]),buf,(unsigned int)i);
			dataB->data[bl+i]='\0';
			bl+=i;
			if (end)
				{
				buf[0]='\0';
				i=BIO_gets(bp,buf,254);
				if (i <= 0) break;

				while ((i >= 0) && (buf[i] <= ' ')) i--;
				buf[++i]='\n'; buf[++i]='\0';

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
		PEMerr(PEM_F_PEM_READ_BIO,PEM_R_BAD_END_LINE);
		goto err;
		}

	EVP_DecodeInit(&ctx);
	i=EVP_DecodeUpdate(&ctx,
		(unsigned char *)dataB->data,&bl,
		(unsigned char *)dataB->data,bl);
	if (i < 0)
		{
		PEMerr(PEM_F_PEM_READ_BIO,PEM_R_BAD_BASE64_DECODE);
		goto err;
		}
	i=EVP_DecodeFinal(&ctx,(unsigned char *)&(dataB->data[bl]),&k);
	if (i < 0)
		{
		PEMerr(PEM_F_PEM_READ_BIO,PEM_R_BAD_BASE64_DECODE);
		goto err;
		}
	bl+=k;

	if (bl == 0) goto err;
	*name=nameB->data;
	*header=headerB->data;
	*data=(unsigned char *)dataB->data;
	*len=bl;
	Free(nameB);
	Free(headerB);
	Free(dataB);
	return(1);
err:
	BUF_MEM_free(nameB);
	BUF_MEM_free(headerB);
	BUF_MEM_free(dataB);
	return(0);
	}
