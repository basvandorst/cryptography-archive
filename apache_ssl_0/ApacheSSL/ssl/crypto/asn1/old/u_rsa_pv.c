#include <stdio.h>
#include "crypto.h"
#include "rsa.h"
#include "x509.h"

RSA *RSAPrivateKey_dup(x)
RSA *x;
	{
	return((RSA *)ASN1_dup(i2d_RSAPrivateKey,
		(char *(*)())d2i_RSAPrivateKey,(char *)(x)));
	}

RSA *d2i_RSAPrivateKey_fp(fp,x)
FILE *fp;
RSA **x;
	{
	return((RSA *)ASN1_d2i_fp((char *(*)())RSA_new,
		(char *(*)())d2i_RSAPrivateKey, (fp),(unsigned char **)(x)));
	}

int i2d_RSAPrivateKey_fp(fp,x)
FILE *fp;
RSA *x;
	{
	return(ASN1_i2d_fp(i2d_RSAPrivateKey,fp,(unsigned char *)x));
	}

