#include <stdio.h>
#include "crypto.h"
#include "x509.h"

X509_CRL *X509_CRL_dup(x)
X509_CRL *x;
	{
	return((X509_CRL *)ASN1_dup(i2d_X509_CRL,
		(char *(*)())d2i_X509_CRL,(char *)x));
	}

X509_CRL *d2i_X509_CRL_fp(fp,x)
FILE *fp;
X509_CRL **x;
	{
	return((X509_CRL *)ASN1_d2i_fp((char *(*)())X509_CRL_new,
		(char *(*)())d2i_X509_CRL, (fp),(unsigned char **)(x)));
	}

int i2d_X509_CRL_fp(fp,x)
FILE *fp;
X509_CRL *x;
	{
	return(ASN1_i2d_fp(i2d_X509_CRL,fp,(unsigned char *)x));
	}

