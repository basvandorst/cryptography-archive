/*
 *   Program to generate OKAMOTO keys.
 */

#include <stdio.h>
#include "miracl.h"

main()
{  /*  calculate public and private keys  *
    *  for oka encryption                 */
    int n1,n2,k;
    long seed;
    big p,q,pq,u,u1,u2,a1,a2,b1,b2,n;
    FILE *outfile;
    mirsys(100,MAXBASE);
    printf("random number seed = ");
    scanf("%ld",&seed);
    getchar();
    irand(seed);
    p=mirvar(0);
    q=mirvar(0);
    pq=mirvar(0);
    u=mirvar(0);
    u1=mirvar(0);
    u2=mirvar(0);
    a1=mirvar(0);
    a2=mirvar(0);
    b1=mirvar(0);
    b2=mirvar(0);
    n=mirvar(0);
    k=0;
    do
    {
        printf("minimum size of keys in decimal digits= ");
        scanf("%d",&k);
        getchar();
    } while (k<30);
    printf("generating keys - please wait\n");
    n1=k/3+4;
    n2=k/3-2;
    do
    { /* ensure p<<q */
        strongp(q,n1);
        strongp(p,n2);
        premult(p,1000,pq);
    } while (compare(q,pq)<=0);
    multiply(p,p,u);
    root(u,3,u);
    subdiv(u,2,u);            /* p^(2/3) /2 */
    bigrand(u,a1);
    bigrand(u,a2);
    bigrand(p,b1);
    bigrand(p,b2);
    multiply(p,q,pq);
    multiply(pq,b1,u1);       
    add(u1,a1,u1);            /* u1 = a1 + b1.pq */
    multiply(pq,b2,u2);
    add(u2,a2,u2);            /* u2 = a2 + b2.pq */
    multiply(pq,p,n);
    xgcd(u1,n,u,u,u);         /* u =  1/u1 mod n */
    mad(u,u2,u2,n,n,u);       /* u = u2/u1 mod n */
    printf("public encryption key\n");
    cotnum(n,stdout);
    cotnum(u,stdout);
    printf("private decryption key\n");
    cotnum(p,stdout);
    cotnum(q,stdout);
    cotnum(u1,stdout);
    cotnum(u2,stdout);
    WRAP=OFF;
    IOBASE=60;
    outfile=fopen("public.key","w");
    cotnum(n,outfile);
    cotnum(u,outfile);
    fclose(outfile);
    outfile=fopen("private.key","w");
    cotnum(p,outfile);
    cotnum(q,outfile);
    cotnum(u1,outfile);
    cotnum(u2,outfile);
    fclose(outfile);
}


