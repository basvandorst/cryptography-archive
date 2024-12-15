/* prv2pub.c - get public key from private */
/* private is stdin, public is stdout */
#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "prandom.h"
#include <stdio.h>
#include "misc.h"

main(int argc,char *argv[])
{
    MP_INT *e,*p,*q; MP_INT *n;
	unsigned char *ss;
	unsigned char k[16];
	unsigned char iv[8];
	int i;

	init(e); init(p); init(q); init(n);
    fprintf(stderr, "\nEnter password to private key: ");
    ss = getstring(1);
    mdstring(ss,strlen(ss),k);
    for (i=0;i<8;i++)
        iv[i] = 0;

    if (!read_numberx(stdin,p,k,iv) || !read_numberx(stdin,q,k,iv)) {
        fprintf(stderr, "wrong key to key entered or key corrupt\n");
        exit(1);
    }
    read_number(stdin,e);
	mpz_mul(n,p,q);
	write_number(stdout,n);
	write_number(stdout,e);

	zap(ss,strlen(ss));
	zap(k,strlen(k));
	clear(p); clear(q); clear(e);
	exit(0);
}
