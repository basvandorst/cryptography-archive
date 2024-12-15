/*
 *   Program to generate RSA keys suitable for use with an encryption
 *   exponent of 3, and which are also 'Blum Integers'.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <big.h>   /* include MIRACL system */

miracl precision=100;

static Big pd,pl,ph;

long randise()
{ /* get a random number */
    long seed;
    cout << "Enter 9 digit random number seed  = ";
    cin >> seed;
    return seed;
}

Big strongp(int n,long seed1,long seed2)
{ /* generate strong prime number =11 mod 12 suitable for RSA encryption */
    Big p;
    int r,r1,r2;
    irand(seed1);
    pd=rand(2*n/3,2);
    pd=nextprime(pd);
    ph=pow((Big)2,n)/pd;  
    pl=pow((Big)2,n-1)/pd;
    ph-=pl;
    irand(seed2);
    ph=rand(ph);
    ph+=pl;
    r1=pd%12;
    r2=ph%12;
    r=0;
    while ((r1*(r2+r))%12!=5) r++;
    ph+=r;
    do 
    { /* find p=2*r*pd+1 = 11 mod 12 */
        p=2*ph*pd+1;
        ph+=12;
    } while (!prime(p));
    return p;
}

void main()
{  /*  calculate public and private keys  *
    *  for rsa encryption                 */
    int k,i,np;
    long seed[4];
    Big p[2],ke;
    ofstream public_key="public.key";
    ofstream private_key="private.key";
    np=2;      /* two primes only - could be more */
    gprime(15000);  /* speeds up large prime generation */
    do
    {
        cout << "size of each prime in bits= ";
        cin >> k;
    } while (k<128);
    for (i=0;i<2*np;i++)
        seed[i]=randise();
    cout << "generating keys - please wait\n";
    ke=1;
    for (i=0;i<np;i++) 
    {
        p[i]=strongp(k,seed[2*i],seed[2*i+1]);
        ke*=p[i];
    }
    cout << "public encryption key\n";
    cout << ke << endl;
    cout << "private decryption key\n";
    for (i=0;i<np;i++)
        cout << p[i] << endl;
    IOBASE=60;
    public_key << ke << endl;
    for (i=0;i<np;i++)
        private_key << p[i] << endl;
}

