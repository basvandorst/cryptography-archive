/*
 *   Program to generate RSA keys suitable for use with an encryption
 *   exponent of 3, and which are also 'Blum Integers'.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

static big pd,pl,ph;

long randise()
{ /* get a random number */
    long seed;
    printf("Enter 9 digit random number seed  = ");
    scanf("%ld",&seed);
    getchar();
    return seed;
}

void strongp(p,n,seed1,seed2)
big p;
int n;
long seed1,seed2;
{ /* generate strong prime number =11 mod 12 suitable for RSA encryption */
    int r,r1,r2;
    irand(seed1);
    bigdig(2*n/3,2,pd);
    nxprime(pd,pd);
    expb2(n,ph);
    divide(ph,pd,ph);
    expb2(n-1,pl);
    divide(pl,pd,pl);
    subtract(ph,pl,ph);
    irand(seed2);
    bigrand(ph,ph);
    add(ph,pl,ph);
    r1=subdiv(pd,12,pl);
    r2=subdiv(ph,12,pl);
    r=0;
    while ((r1*(r2+r))%12!=5) r++;
    incr(ph,r,ph);
    do 
    { /* find p=2*r*pd+1 = 11 mod 12 */
        multiply(ph,pd,p);
        premult(p,2,p);
        incr(p,1,p);
        incr(ph,12,ph);
    } while (!isprime(p));
}

main()
{  /*  calculate public and private keys  *
    *  for rsa encryption                 */
    int k,i,np;
    long seed[4];
    big p[2],ke;
    FILE *outfile;
    mirsys(100,0);
    np=2;      /* two primes only - could be more */
    gprime(15000);  /* speeds up large prime generation */
    for (i=0;i<np;i++) p[i]=mirvar(0);
    ke=mirvar(0);
    pd=mirvar(0);
    pl=mirvar(0);
    ph=mirvar(0);
    do
    {
        printf("size of each prime in bits= ");
        scanf("%d",&k);
        getchar();
    } while (k<128);
    for (i=0;i<2*np;i++)
        seed[i]=randise();
    printf("generating keys - please wait\n");
    convert(1,ke);
    for (i=0;i<np;i++) 
    {
        strongp(p[i],k,seed[2*i],seed[2*i+1]);
        multiply(ke,p[i],ke);
    }
    printf("public encryption key\n");
    cotnum(ke,stdout);
    printf("private decryption key\n");
    for (i=0;i<np;i++)
        cotnum(p[i],stdout);
    IOBASE=60;
    outfile=fopen("public.key","w");
    cotnum(ke,outfile);
    fclose(outfile);
    outfile=fopen("private.key","w");
    for (i=0;i<np;i++)
        cotnum(p[i],outfile);
    fclose(outfile);
}

