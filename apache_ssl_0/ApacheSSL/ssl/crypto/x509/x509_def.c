/* lib/x509/x509_def.c */
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
#include <sys/types.h>
#include <sys/stat.h>
#include "crypto.h"
#include "x509.h"
#include "location.h"

char *X509_get_default_private_dir()
	{ return(SSL_PRIVATE_DIR); }
	
char *X509_get_default_cert_area()
	{ return(SSL_CERT_AREA); }

char *X509_get_default_cert_dir()
	{ return(SSL_CERT_DIR); }

char *X509_get_default_cert_file()
	{ return(SSL_CERT_FILE); }

char *X509_get_default_cert_dir_env()
	{ return(SSL_CERT_DIR_EVP); }

char *X509_get_default_cert_file_env()
	{ return(SSL_CERT_FILE_EVP); }

int X509_set_default_verify_paths(ctx)
CERTIFICATE_CTX *ctx;
	{
	char *file,*dir;

	if (!X509_load_verify_locations(ctx,SSL_CERT_FILE,SSL_CERT_DIR))
		{
		Fprintf(stderr,"error loading default cert locations, these are\n");
		Fprintf(stderr,"the directory %s\n",SSL_CERT_DIR);
		Fprintf(stderr,"and the file %s\n",SSL_CERT_FILE);
		return(0);
		}
	file=(char *)Getenv(SSL_CERT_FILE_EVP);
	dir =(char *)Getenv(SSL_CERT_DIR_EVP);
	if (!X509_load_verify_locations(ctx,file,dir))
		{
		Fprintf(stderr,"error loading cert locations, these are\n");
		if (file != NULL)
			{
			Fprintf(stderr,"the file %s\n",SSL_CERT_FILE);
			Fprintf(stderr,"which is specified by the %s environment variable\n",SSL_CERT_FILE_EVP);
			return(0);
			}

		if (dir != NULL)
			{
			Fprintf(stderr,"the directory %s\n",SSL_CERT_FILE);
			Fprintf(stderr,"which is specified by the %s environment variable\n",SSL_CERT_DIR_EVP);
			return(0);
			}
		}	
	return(1);
	}

int X509_load_verify_locations(ctx,file_env, dir_env)
CERTIFICATE_CTX *ctx;
char *file_env;
char *dir_env;
	{
	int i;
	struct stat st;
	char *str;

	str=file_env;
	if ((str != NULL) && (stat(str,&st) == 0))
		{
		i=X509_add_cert_file(ctx,str,X509_FILETYPE_PEM);
		if (!i) return(0);
		}	

	str=dir_env;
	if (str != NULL)
		{
		i=X509_add_cert_dir(ctx,str,X509_FILETYPE_PEM);
		if (!i) return(0);
		}
	return(1);
	}

