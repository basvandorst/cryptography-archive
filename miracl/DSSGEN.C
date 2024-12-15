/*
 *   Proposed Digital Signature Standard
 *
 *   See Communications ACM July 1992, Vol. 35 No. 7
 *   This new standard for digital signatures has been proposed by 
 *   the American National Institute of Standards and Technology (NIST)
 *   under advisement from the National Security Agency (NSA). 
 *
 *   This program generates the common values p, q and g to a file
 *   common.dss, and one set of public and private keys in files public.dss
 *   and private.dss respectively
 *   
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

main()
{
    FILE *fp;
    big p,q,h,g,x,y,n,s,t;
    long seed;
    mirsys(50,0);
    p=mirvar(0);
    q=mirvar(0);
    h=mirvar(0);
    g=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);
    n=mirvar(0);
    s=mirvar(0);
    t=mirvar(0);
/* randomise */
    printf("Enter 9 digit random number seed  = ");
    scanf("%ld",&seed);
    getchar();
    irand(seed);
    fp=fopen("common.dss","w");
/* generate q */
    forever 
    {
        bigdig(159,2,n);
        premult(n,2,q);
        nxprime(q,q);
        if (logb2(q)>=160) continue;
        break;
    }
    printf("q= ");
    cotnum(q,stdout);
    cotnum(q,fp);
/* generate p */
    expb2(512,t);
    decr(t,1,t);
    premult(q,2,n);
    divide(t,n,t);
    expb2(511,s);
    decr(s,1,s);
    divide(s,n,s);
    forever 
    {
        bigrand(t,p);
        if (compare(p,s)<=0) continue;
        premult(p,2,p);
        multiply(p,q,p);
        incr(p,1,p);
        copy(p,n);
        if (isprime(p)) break;
    } 
    printf("p= ");
    cotnum(p,stdout);
    cotnum(p,fp);
/* generate g */
    do {
        decr(p,1,t);
        bigrand(t,h);
        divide(t,q,t);
        powmod(h,t,p,g);
    } while (size(g)==1);    
    printf("g= ");
    cotnum(g,stdout);
    cotnum(g,fp);
    fclose(fp);
/* generate keys */
    bigrand(q,x);
    powmod(g,x,p,y);
    printf("public key = ");
    cotnum(y,stdout);
    fp=fopen("public.dss","w");
    cotnum(y,fp);
    fclose(fp);
    fp=fopen("private.dss","w");
    cotnum(x,fp);
    fclose(fp);
}

