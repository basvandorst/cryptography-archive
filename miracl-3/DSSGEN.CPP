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
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <fstream.h>
#include <big.h>

miracl precision=50;

void main()
{
    ofstream common="common.dss";    /* construct file I/O streams */
    ofstream public_key="public.dss";
    ofstream private_key="private.dss";
    Big p,q,h,g,x,y,n,s,t;
    long seed;
    cout << "Enter 9 digit random number seed  = ";
    cin >> seed;
    irand(seed);
/* generate q */
    forever
    {
        n=rand(159,2);
        q=2*n+1;
        while (!prime(q)) q+=2;
        if (bits(q)>=160) continue;
        break;
    }
    cout << "q= " << q << endl;
    common << q << endl;
/* generate p */
    t=(pow((Big)2,512)-1)/(2*q);
    s=(pow((Big)2,511)-1)/(2*q);
    forever 
    {
        n=rand(t);
        if (n<s) continue;
        p=2*n*q+1;
        if (prime(p)) break;
    } 
    cout << "p= " << p << endl;
    common << p << endl;
/* generate g */
    do {
        h=rand(p-1);
        g=pow(h,(p-1)/q,p);
    } while (g==1);    
    cout << "g= " << g << endl;
    common << g << endl;
/* generate keys */
    x=rand(q);
    y=pow(g,x,p);
    cout << "public key = " << y << endl;
    public_key << y << endl;
    private_key << x << endl;
}

