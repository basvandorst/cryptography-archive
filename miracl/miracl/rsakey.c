/*
 *   Program to generate RSA keys.
 */

#include <stdio.h>
#include "miracl.h"

main()
{  /*  calculate public and private keys  *
    *  for rsa encryption                 */
    int n1,n2,k;
    long seed;
    big p,q,ke,kd;
    FILE *outfile;
    mirsys(100,MAXBASE);
    printf("random number seed = ");
    scanf("%ld",&seed);
    getchar();
    irand(seed);
    p=mirvar(0);
    q=mirvar(0);
    ke=mirvar(0);
    kd=mirvar(0);
    k=0;
    do
    {
        printf("minimum size of keys in decimal digits= ");
        scanf("%d",&k);
        getchar();
    } while (k<30);
    printf("generating keys - please wait\n");
    n1=k/2+4;
    n2=k/2-4;
    do
    { /* ensure p>>q */
        strongp(p,n1);
        strongp(q,n2);
        premult(q,1000,ke);
    } while (compare(p,ke)<=0);
/*  ke=p*q                  */
    multiply(p,q,ke);
/*  kd=(2*(p-1)*(q-1)+1)/3  */
    decr(p,1,p);
    decr(q,1,q);
    multiply(p,q,kd);
    premult(kd,2,kd);
    incr(kd,1,kd);
    subdiv(kd,3,kd);
    incr(p,1,p);
    incr(q,1,q);
    printf("public encryption key\n");
    cotnum(ke,stdout);
    printf("private decryption key\n");
    cotnum(kd,stdout);
    WRAP=OFF;
    IOBASE=60;
    outfile=fopen("public.key","w");
    cotnum(ke,outfile);
    fclose(outfile);
    outfile=fopen("private.key","w");
    cotnum(kd,outfile);
    cotnum(p,outfile);
    cotnum(q,outfile);
    fclose(outfile);
}
