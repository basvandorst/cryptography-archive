/* apps/x509.c */
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "apps.h"
#include "err.h"
#include "bn.h"
#include "rsa.h"
#include "envelope.h"
#include "x509.h"
#include "objects.h"
#include "pem.h"

#define PROG x509_main

#define	POSTFIX	".srl"
#define DEF_DAYS	30

#define FORMAT_UNDEF	0
#define FORMAT_ASN1	1
#define FORMAT_TEXT	2
#define FORMAT_PEM	3

char *usage[]={
"usage: x509 args\n",
"\n",
" -inform arg     - input format - default PEM (one of DER, TXT or PEM)\n",
" -outform arg    - output format - default PEM\n",
" -keyform arg    - rsa key format - default PEM\n",
" -CAform arg     - CA format - default PEM\n",
" -CAkeyform arg  - CA key format - default PEM\n",
" -days arg       - How long till expiry of a signed certificate - def 30 days\n",
" -in arg         - input file - default stdin\n",
" -out arg        - output file - default stdout\n",
" -serial         - print serial number value\n",
" -hash           - print hash value\n",
" -subject        - print subject DN\n",
" -issuer         - print issuer DN\n",
" -startdate      - notBefore field\n",
" -enddate        - notAfter field\n",
" -noout          - no certificate output\n",
" -signkey arg    - set issuer to subject and sign with private key and\n",
"                   put the public in cert.\n",
" -RSAreq arg     - output a certification request object\n",
" -req            - input is a certificate request.\n",
" -CA arg         - set the CA certificate, must be PEM format.\n",
" -CAkey arg      - set the CA RSA key, must be PEM format, if this arg\n",
"                   missing, it is asssumed to be in the CA file.\n",
" -CAcreateserial - create serial number file if it does not exist\n",
" -CAserial       - serial file\n",
" -text           - print the certitificate in text form\n",
NULL
};

#ifndef NOPROTO
static int callb(int ok, X509 *xs, X509 *xi, int depth, int error);
static RSA *load_key(char *file, int format);
static X509 *load_cert(char *file, int format);
static int sign (X509 *x, RSA *rsa);
static int certify (CERTIFICATE_CTX *ctx,char *CAfile, X509 *x,
	X509 *xca, RSA *rsa,char *serial, int create, int days);
#else
static int callb();
static RSA *load_key();
static X509 *load_cert();
static int sign ();
static int certify ();
#endif

int MAIN(argc, argv)
int argc;
char **argv;
	{
	int ret=1;
	X509_REQ *req=NULL;
	X509 *x=NULL,*xca=NULL;
	RSA *rsa=NULL,*CArsa=NULL;
	int i,num,badops=0;
	FILE *out=NULL;
	int informat,outformat,keyformat,CAformat,CAkeyformat;
	char *infile=NULL,*outfile=NULL,*keyfile=NULL,*CAfile=NULL;
	char *CAkeyfile=NULL,*str=NULL,*CAserial=NULL;
	int text=0,serial=0,hash=0,subject=0,issuer=0,startdate=0,enddate=0;
	int noout=0,sign_flag=0,CA_flag=0,CA_createserial=0;
	int RSAreq=0,days=DEF_DAYS;
	char *reqfile=NULL,*reqfile2=NULL;
	char **pp;
	CERTIFICATE_CTX *ctx=NULL;
	X509_REQ *rq=NULL;
	RSA *r=NULL;

	informat=FORMAT_PEM;
	outformat=FORMAT_PEM;
	keyformat=FORMAT_PEM;
	CAformat=FORMAT_PEM;
	CAkeyformat=FORMAT_PEM;

	ctx=CERTIFICATE_CTX_new();
	if (ctx == NULL) goto end;
	argc--;
	argv++;
	num=0;
	while (argc >= 1)
		{
		if 	(strcmp(*argv,"-inform") == 0)
			{
			if (--argc < 1) goto bad;
			informat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-outform") == 0)
			{
			if (--argc < 1) goto bad;
			outformat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-keyform") == 0)
			{
			if (--argc < 1) goto bad;
			keyformat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-req") == 0)
			{
			if (--argc < 1) goto bad;
			reqfile2= *(++argv);
			}
		else if (strcmp(*argv,"-CAform") == 0)
			{
			if (--argc < 1) goto bad;
			CAformat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-CAkeyform") == 0)
			{
			if (--argc < 1) goto bad;
			CAformat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-days") == 0)
			{
			if (--argc < 1) goto bad;
			days=atoi(*(++argv));
			if (days == 0)
				{
				fprintf(stderr,"bad number of days\n");
				goto bad;
				}
			}
		else if (strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			infile= *(++argv);
			}
		else if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outfile= *(++argv);
			}
		else if (strcmp(*argv,"-signkey") == 0)
			{
			if (--argc < 1) goto bad;
			keyfile= *(++argv);
			sign_flag= ++num;
			}
		else if (strcmp(*argv,"-CA") == 0)
			{
			if (--argc < 1) goto bad;
			CAfile= *(++argv);
			CA_flag= ++num;
			}
		else if (strcmp(*argv,"-CAkey") == 0)
			{
			if (--argc < 1) goto bad;
			CAkeyfile= *(++argv);
			}
		else if (strcmp(*argv,"-CAserial") == 0)
			{
			if (--argc < 1) goto bad;
			CAserial= *(++argv);
			}
		else if (strcmp(*argv,"-serial") == 0)
			serial= ++num;
		else if (strcmp(*argv,"-RSAreq") == 0)
			{
			if (--argc < 1) goto bad;
			reqfile= *(++argv);
			RSAreq= ++num;
			}
		else if (strcmp(*argv,"-text") == 0)
			text= ++num;
		else if (strcmp(*argv,"-hash") == 0)
			hash= ++num;
		else if (strcmp(*argv,"-subject") == 0)
			subject= ++num;
		else if (strcmp(*argv,"-issuer") == 0)
			issuer= ++num;
		else if (strcmp(*argv,"-startdate") == 0)
			startdate= ++num;
		else if (strcmp(*argv,"-enddate") == 0)
			enddate= ++num;
		else if (strcmp(*argv,"-noout") == 0)
			noout= ++num;
		else if (strcmp(*argv,"-CAcreateserial") == 0)
			CA_createserial= ++num;
		else
			{
			fprintf(stderr,"unknown option %s\n",*argv);
			badops=1;
			break;
			}
		argc--;
		argv++;
		}

	if (badops)
		{
bad:
		for (pp=usage; (*pp != NULL); pp++)
			fprintf(stderr,*pp);
		goto end;
		}

	ERR_load_crypto_strings();

	if (!X509_set_default_verify_paths(ctx))
		{
		ERR_print_errors(stderr);
		goto end;
		}

	if ((CAkeyfile == NULL) && (CA_flag) && (CAformat == FORMAT_PEM))
		{ CAkeyfile=CAfile; }
	else if ((CA_flag) && (CAkeyfile == NULL))
		{
		fprintf(stderr,"need to specify a CAkey if using the CA command\n");
		goto end;
		}

	if (reqfile2 != NULL)
		{
		req=X509_REQ_new();
		if (req == NULL) goto end;
		}
	x=load_cert(infile,informat);
	if (x == NULL) goto end;
	if (CA_flag)
		{
		xca=load_cert(CAfile,CAformat);
		if (xca == NULL) goto end;
		}

	if (num)
		{
		for (i=1; i<=num; i++)
			{
			if (issuer == i)
				{
				str=X509_NAME_oneline(X509_get_issuer_name(x));
				if (str == NULL)
					{
					fprintf(stderr,"unable to get issuer Name from certificate\n");
					ERR_print_errors(stderr);
					goto end;
					}
				fprintf(stdout,"issuer= %s\n",str);
				free(str);
				}

			if (subject == i) 
				{
				str=X509_NAME_oneline(X509_get_subject_name(x));
				if (str == NULL)
					{
					fprintf(stderr,"unable to get subject Name from certificate\n");
					ERR_print_errors(stderr);
					goto end;
					}
				fprintf(stdout,"subject=%s\n",str);
				free(str);
				}
			if (serial == i)
				{
				fprintf(stdout,"serial ");
				i2f_ASN1_INTEGER(stdout,x->cert_info->serialNumber);
				}
			if (hash == i)
				{
				fprintf(stdout,"%08lx\n",
					X509_subject_name_hash(x));
				}
			if (text == i)
				{
				X509_print(stdout,x);
				}
			if (startdate == i)
				{
				fprintf(stdout,"notBefore=%s\n",
					x->cert_info->validity->notBefore);
				}
			if (enddate == i)
				{
				fprintf(stdout,"notAfter =%s\n",
					x->cert_info->validity->notAfter);
				}

			/* should be in the library */
			if (sign_flag == i)
				{
				fprintf(stderr,"Getting Private key\n");
				if (rsa == NULL)
					{
					rsa=load_key(keyfile,keyformat);
					if (rsa == NULL) goto end;
					}
				if (!sign(x,rsa)) goto end;
				}
			if (CA_flag == i)
				{
				fprintf(stderr,"Getting CA Private Key\n");
				if (CAkeyfile != NULL)
					{
					CArsa=load_key(CAkeyfile,CAkeyformat);
					if (CArsa == NULL) goto end;
					}
				if (!certify(ctx,CAfile,x,xca,CArsa,CAserial,
					CA_createserial,days))
					goto end;
				}
			if (RSAreq == i)
				{
				fprintf(stderr,"Getting request Private Key\n");
				if (reqfile == NULL)
					{
					fprintf(stderr,"no request key file specified\n");
					goto end;
					}
				else
					{
					r=load_key(reqfile,FORMAT_PEM);
					if (r == NULL) goto end;
					}

				fprintf(stderr,"Generating request to send to RSA\n");

				rq=(X509_REQ *)X509_X509_TO_req(x,r);
				if (rq == NULL)
					{
					ERR_print_errors(stderr);
					goto end;
					}
				if (!noout)
					{
					if (outfile == NULL)
						out=stdout;
					else
						{
						out=fopen(outfile,"w");
						if (out == NULL)
							{
							perror(outfile);
							goto end;
							}
						}
					X509_REQ_print(out,rq);
					PEM_write_X509_REQ(out,rq);
					}
				noout=1;
				}
			}
		}

	if (noout)
		{
		ret=0;
		goto end;
		}

	if (outfile == NULL)
		out=stdout;
	else
		{
		out=fopen(outfile,"w");
		if (out == NULL) { perror(outfile); goto end; }
		}

	if 	(outformat == FORMAT_ASN1)
		i=i2d_X509_fp(out,x);
#ifdef F2I
	else if (outformat == FORMAT_TEXT)
		{ i2f_X509(out,x); i=1; }
#endif
	else if (outformat == FORMAT_PEM)
		i=PEM_write_X509(out,x);
	else	{
		fprintf(stderr,"bad output format specified for outfile\n");
		goto end;
		}
	if (!i) {
		fprintf(stderr,"unable to write certificate\n");
		ERR_print_errors(stderr);
		goto end;
		}
	ret=0;
end:
	if ((out != NULL) && (out != stdout)) fclose(out);
	if (ctx != NULL) CERTIFICATE_CTX_free(ctx);
	if (req != NULL) X509_REQ_free(req);
	if (x != NULL) X509_free(x);
	if (xca != NULL) X509_free(xca);
	if (rsa != NULL) RSA_free(rsa);
	if (CArsa != NULL) RSA_free(CArsa);
	if (rq != NULL) X509_REQ_free(rq);
	if (r != NULL) RSA_free(r);
	EXIT(ret);
	}

static int certify(ctx,CAfile, x, xca, rsa, serialfile, create,days)
CERTIFICATE_CTX *ctx;
char *CAfile;
X509 *x;
X509 *xca;
RSA *rsa;
char *serialfile;
int create;
int days;
	{
	FILE *io;
	char *buf,buf2[1024];
	BIGNUM *serial;
	ASN1_INTEGER *bs,bs2;

	buf=(char *)malloc(RSA_size(rsa)*2+
		((serialfile == NULL)
			?(strlen(CAfile)+strlen(POSTFIX)+1)
			:(strlen(serialfile)))+1);
	if (buf == NULL) { fprintf(stderr,"out of mem\n"); return(0); }
	if (serialfile == NULL)
		{
		strcpy(buf,CAfile);
		strcat(buf,POSTFIX);
		}
	else
		strcpy(buf,serialfile);
	serial=BN_new();
	bs=ASN1_INTEGER_new();
	if ((serial == NULL) || (bs == NULL))
		{
		ERR_print_errors(stderr);
		return(0);
		}

	io=fopen(buf,"r");
	if (io == NULL)
		{
		if (!create)
			{
			perror(buf);
			return(0);
			}
		else
			BN_zero(serial);
		}
	else 
		{
		if (!f2i_ASN1_INTEGER(io,bs,buf2,1024))
			{
			fprintf(stderr,"unable to load serial number from %s\n",buf);
			ERR_print_errors(stderr);
			fclose(io);
			return(0);
			}
		else
			{
			serial=BN_bin2bn(bs->data,bs->length,serial);
			if (serial == NULL)
				{
				fprintf(stderr,"error converting bin 2 bn");
				fclose(io);
				return(0);
				}
			}
		fclose(io);
		}

	if (!BN_add_word(serial,1))
		{ fprintf(stderr,"add_word failure\n"); return(0); }
	bs2.data=(unsigned char *)buf2;
	bs2.length=BN_bn2bin(serial,bs2.data);

	io=fopen(buf,"w");
	if (io == NULL)
		{
		fprintf(stderr,"error attempting to write serial number file\n");
		perror(buf);
		return(0);
		}
	i2f_ASN1_INTEGER(io,&bs2);
	fclose(io);
	
	if (!X509_add_cert(ctx,x)) return(0);
	/* NOTE: this certificate can/should be self signed */
	if (!X509_cert_verify(ctx,x,callb)) return(0);

	/* don't free this X509 struct or bad things will happen
	 * unless you put issuer first :-) */
	x->cert_info->issuer=xca->cert_info->subject;
	if (x->cert_info->validity->notBefore == NULL)
		free(x->cert_info->validity->notBefore);
	x->cert_info->validity->notBefore=(char *)malloc(100);
	x->cert_info->serialNumber=bs;

	if (x->cert_info->validity->notBefore == NULL)
		{
		fprintf(stderr,"out of mem\n");
		return(0);
		}

	X509_gmtime(x->cert_info->validity->notBefore,0);
	/* hardwired expired */
	X509_gmtime(x->cert_info->validity->notAfter,60*60*24*days);
	if (!X509_sign(x,rsa,EVP_md5))
		{
		ERR_print_errors(stderr);
		return(0);
		}
	return(1);
	}

static int callb(ok, xs, xi, depth, error)
int ok;
X509 *xs;
X509 *xi;
int depth;
int error;
	{
	/* it is ok to use a self signed certificate */
	if ((!ok) && (error == VERIFY_ERR_DEPTH_ZERO_SELF_SIGNED_CERT))
		return(1);

	/* BAD we should have gotten an error :-) */
	if (ok)
		printf("error with certificate to be certified - should be self signed\n");
	else
		{
		char *s;

		s=X509_NAME_oneline(X509_get_subject_name(xs));
		printf("%s\n",s);
		free(s);
		printf("error with certificate - error %d at depth %d\n%s\n",
			error,depth,X509_cert_verify_error_string(error));
		}
#ifdef LINT
	xi=xs; xs=xi;
#endif
	return(0);
	}

static RSA *load_key(file, format)
char *file;
int format;
	{
	FILE *key;
	RSA *rsa;

	if (file == NULL)
		{
		fprintf(stderr,"no keyfile specified\n");
		return(NULL);
		}
	key=fopen(file,"r");
	if (key == NULL) { perror(file); return(NULL); }
	if	(format == FORMAT_ASN1)
		rsa=d2i_RSAPrivateKey_fp(key,NULL);
#ifdef F2I
	else if (format == FORMAT_TEXT)
		i=f2i_RSAPrivateKey(key,rsa);
#endif
	else if (format == FORMAT_PEM)
		rsa=PEM_read_RSAPrivateKey(key,NULL,NULL);
	else
		{
		fprintf(stderr,"bad input format specified for key\n");
		return(NULL);
		}
	fclose(key);
	if (rsa == NULL)
		{
		fprintf(stderr,"unable to load Private Key\n");
		return(NULL);
		}
	return(rsa);
	}

static X509 *load_cert(file, format)
char *file;
int format;
	{
	X509 *x;
	FILE *cert;

	if (file == NULL)
		cert=stdin;
	else
		{
		cert=fopen(file,"r");
		if (cert == NULL) { perror(file); return(NULL); }
		}
	if 	(format == FORMAT_ASN1)
		x=d2i_X509_fp(cert,NULL);
#ifdef F2I
	else if (format == FORMAT_TEXT)
		i=f2i_X509(cert,x);
#endif
	else if (format == FORMAT_PEM)
		x=PEM_read_X509(cert,NULL,NULL);
	else	{
		fprintf(stderr,"bad input format specified for input cert\n");
		return(NULL);
		}
	if (x == NULL)
		{
		fprintf(stderr,"unable to load certificate\n");
		ERR_print_errors(stderr);
		return(NULL);
		}
	if (cert != stdin) fclose(cert);
	return(x);
	}

static int sign(x, rsa)
X509 *x;
RSA *rsa;
	{
	int j;
	unsigned char *p;
	/* X509_NAME *n; */

	/* n=x->cert_info->subject; */
	/* don't free this X509 struct or bad
	 * things will happen unless you put
	 * n back first :-) */
	x->cert_info->issuer=x->cert_info->subject;
	X509_gmtime(x->cert_info->validity->notBefore,0);
	/* Lets just make it 12:00am GMT, Jan 1 1970 */
	memcpy(x->cert_info->validity->notBefore,"700101120000Z",13);
	/* 28 days to be certified */
	X509_gmtime(x->cert_info->validity->notAfter,60*60*24*28);
	j=i2d_RSAPublicKey(rsa,NULL);
	p=x->cert_info->key->public_key->data=(unsigned char *)malloc(
		(unsigned int)j+10);
	if (p == NULL) { fprintf(stderr,"out of memory\n"); return(0); }

	x->cert_info->key->public_key->length=j;
	if (!i2d_RSAPublicKey(rsa,&p))
		{
		ERR_print_errors(stderr);
		return(0);
		}
	if (!X509_sign(x,rsa,EVP_md5))
		{
		ERR_print_errors(stderr);
		return(0);
		}
	return(1);
	}
