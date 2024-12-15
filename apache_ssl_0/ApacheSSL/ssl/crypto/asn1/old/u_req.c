#include <stdio.h>
#include "crypto.h"
#include "x509.h"

X509_REQ *X509_REQ_dup(x)
X509_REQ *x;
	{
	return((X509_REQ *)ASN1_dup(i2d_X509_REQ,
		(char *(*)())d2i_X509_REQ,(char *)(x)));
	}

X509_REQ *d2i_X509_REQ_fp(fp,x)
FILE *fp;
X509_REQ **x;
	{
	return((X509_REQ *)ASN1_d2i_fp((char *(*)())X509_REQ_new,
		(char *(*)())d2i_X509_REQ, (fp),(unsigned char **)(x)));
	}

int i2d_X509_REQ_fp(fp,x)
FILE *fp;
X509_REQ *x;
	{
	return(ASN1_i2d_fp(i2d_X509_REQ,fp,(unsigned char *)x));
	}

