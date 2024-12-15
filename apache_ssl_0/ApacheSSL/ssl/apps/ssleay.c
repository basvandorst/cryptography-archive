/* apps/ssleay.c */
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
#include <string.h>
#include <stdlib.h>
#include "lhash.h"
#include "conf.h"
#include "x509.h"
#include "pem.h"
#include "ssl.h"
#include "s_apps.h"
#include "s_net.h"
#include "apps.h"
#include "err.h"

#include "progs.h"

#undef DEBUG

#ifndef NOPROTO
static int chopup(char *buf, int *argc, char **argv[]);
static unsigned long hash(FUNCTION *a);
static int cmp(FUNCTION *a,FUNCTION *b);
static LHASH *prog_init(void );
static int do_cmd(LHASH *prog,int argc,char *argv[]);
#else
static int chopup();
static unsigned long hash();
static int cmp();
static LHASH *prog_init();
static int do_cmd();
#endif

#ifdef _Windows
#include "conio.h"

int getkey(buf,num,verify)
char *buf;
int num;
int verify;
	{
	fputs("Enter PEM pass phrase:",stdout);
	fflush(stdout);
	gets(buf);
	printf("<%s>\n",buf);
	return(strlen(buf));
	}

int app_fwrite(buf,size,num,fp)
char *buf;
int size;
int num;
FILE *fp;
	{
	return(fwrite(buf,size,num,fp));
	}

int app_fread(buf,size,num,fp)
char *buf;
int size;
int num;
FILE *fp;
	{
	return(fread(buf,size,num,fp));
	}

char *app_fgets(s,n,fp)
char *s;
int n;
FILE *fp;
	{
	return(fgets(s,n,fp));
	}
#endif
	
LHASH *config=NULL;

int main(Argc,Argv)
int Argc;
char *Argv[];
	{
	FUNCTION f,*fp;
	char *prompt,buf[1024];
	int n,i,ret=0;
	int argc;
	char **argv,*p;
	LHASH *prog=NULL;
	long errline;
 
#ifdef _Windows
/*	PEM_set_getkey_callback(getkey); WINDOWS FIX */
	WIN_init(app_fread,app_fwrite,app_fgets);
#endif

	ERR_load_crypto_strings();

	/* Lets load up our environment a little */
	strcpy(buf,X509_get_default_cert_area());
	strcat(buf,"/lib/");
	strcat(buf,SSLEAY_CONF);

	config=CONF_load(config,buf,&errline);
	if (config == NULL) ERR_clear_error();

	prog=prog_init();

	/* first check the program name */
	p=strrchr(Argv[0],'/');
	if (p != NULL)
		p++;
	else
		p=Argv[0];

	f.name=p;
	fp=(FUNCTION *)lh_retrieve(prog,(char *)&f);
	if (fp != NULL)
		{
		Argv[0]=p;
		ret=fp->func(Argc,Argv);
		goto end;
		}

	/* ok, now check that there are not arguments, if there are,
	 * run with them, shifting the ssleay off the front */
	if (Argc != 1)
		{
		Argc--;
		Argv++;
		ret=do_cmd(prog,Argc,Argv);
		if (ret < 0) ret=0;
		goto end;
		}

	/* ok, lets enter the old 'SSLeay>' mode */
	
	for (;;)
		{
		ret=0;
		p=buf;
		n=1024;
		i=0;
		for (;;)
			{
			p[0]='\0';
			if (i++)
				prompt=">";
			else	prompt="SSLeay>";
			fputs(prompt,stdout);
			fflush(stdout);
			fgets(p,n,stdin);
			if (p[0] == '\0') goto end;
			i=strlen(p);
			if (i <= 1) break;
			if (p[i-2] != '\\') break;
			i-=2;
			p+=i;
			n-=i;
			}
		if (!chopup(buf,&argc,&argv)) break;

#ifdef DEBUG
		for (i=0; i<argc; i++)
			printf("%2d:<%s>\n",i,argv[i]);
#endif

		ret=do_cmd(prog,argc,argv);
		if (ret < 0)
			{
			ret=0;
			goto end;
			}
		if (ret != 0)
			fprintf(stderr,"error in %s\n",argv[0]);
		}
	fprintf(stderr,"bad exit\n");
	ret=1;
end:
	EXIT(ret);
	}

static int do_cmd(prog,argc,argv)
LHASH *prog;
int argc;
char *argv[];
	{
	FUNCTION f,*fp;
	int i,ret=1;

	f.name=argv[0];
	fp=(FUNCTION *)lh_retrieve(prog,(char *)&f);
	if (fp != NULL)
		{
		ret=fp->func(argc,argv);
		}
	else if ((strcmp(argv[0],"quit") == 0) ||
		(strcmp(argv[0],"exit") == 0) ||
		(strcmp(argv[0],"bye") == 0))
		{
		ret=-1;
		goto end;
		}
	else
		{
		fprintf(stderr,"bad command, valid commands are");
		i=0;
		for (fp=functions; fp->name != NULL; fp++)
			{
			if (((i++) % 5) == 0) fprintf(stderr,"\n");
			fprintf(stderr,"%-15s",fp->name);
			}
		fprintf(stderr,"\nquit\n");
		ret=0;
		}
end:
	return(ret);
	}

static int chopup(buf,argc,argv)
char *buf;
int *argc;
char **argv[];
	{
	int num,len,i;
	static char **arg=NULL;
	static int argcount=0;
	char *p;

	*argc=0;
	*argv=NULL;

	len=strlen(buf);
	i=0;
	if (argcount == 0)
		{
		argcount=20;
		arg=(char **)malloc(sizeof(char *)*argcount);
		}

	num=0;
	p=buf;
	for (;;)
		{
		/* first scan over white space */
		if (!*p) break;
		while (*p && ((*p == ' ') || (*p == '\t') || (*p == '\n')))
			p++;
		if (!*p) break;

		/* The start of something good :-) */
		if (num >= argcount)
			{
			argcount+=20;
			arg=(char **)realloc(arg,sizeof(char *)*argcount);
			if (argc == 0) return(0);
			}
		arg[num++]=p;

		/* now look for the end of this */
		if ((*p == '\'') || (*p == '\"')) /* scan for closing quote */
			{
			i= *(p++);
			arg[num-1]++; /* jump over quote */
			while (*p && (*p != i))
				p++;
			*p='\0';
			}
		else
			{
			while (*p && ((*p != ' ') &&
				(*p != '\t') && (*p != '\n')))
				p++;

			*p='\0';
			}
		p++;
		}
	*argc=num;
	*argv=arg;
	return(1);
	}

static LHASH *prog_init()
	{
	LHASH *ret;
	FUNCTION *f;

	if ((ret=lh_new(hash,cmp)) == NULL) return(NULL);

	for (f=functions; f->name != NULL; f++)
		lh_insert(ret,(char *)f);
	return(ret);
	}

static int cmp(a,b)
FUNCTION *a,*b;
	{
	return(strcmp(a->name,b->name));
	}

static unsigned long hash(a)
FUNCTION *a;
	{
	return(lh_strhash(a->name));
	}
