/*
 *	des - fast & portable DES encryption & decryption.
 *	Copyright (C) 1992  Dana L. How
 *	Please see the file `README' for the complete copyright notice.
 */

#ifndef	lint
static char desKerb_cRcs[] = "$Id: desKerb.c,v 1.3 1992/05/19 23:59:24 how E $";
#endif

#include	"desCore.h"


/* permit the default style of des functions to be changed */

void (*DesCryptFuncs[2])() = { DesSmallFipsDecrypt, DesSmallFipsEncrypt };

/* kerberos-compatible key schedule function */

int
des_key_sched(k, s)
byte * k;
word * s;
{
	return DesMethod(s, k);
}

/* kerberos-compatible des coding function */

int
des_ecb_encrypt(s, d, r, e)
byte * s, * d;
word * r;
int e;
{
	(*DesCryptFuncs[e])(d, r, s);
	return 0;
}

/* also needs to be defined,  as pointed out by eay@psych.psy.uq.oz.au */

int
des_encrypt(s, d, r, e)
byte * s, * d;
word * r;
int e;
{
        (*DesCryptFuncs[e])(d, r, s);
        return 0;
}
