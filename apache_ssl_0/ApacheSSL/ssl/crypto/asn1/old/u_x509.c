#include <stdio.h>
#include "crypto.h"
#include "x509.h"

X509 *X509_dup(x)
X509 *x;
	{
	return((X509 *)ASN1_dup(i2d_X509,(char *(*)())d2i_X509,(char *)(x)));
	}

X509 *d2i_X509_fp(fp,x)
FILE *fp;
X509 **x;
	{
	return((X509 *)ASN1_d2i_fp((char *(*)())X509_new,
		(char *(*)())d2i_X509, (fp),(unsigned char **)(x)));
	}

int i2d_X509_fp(fp,x)
FILE *fp;
X509 *x;
	{
	return(ASN1_i2d_fp(i2d_X509,fp,(unsigned char *)x));
	}

