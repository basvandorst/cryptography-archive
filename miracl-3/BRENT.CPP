/*
 *   Program to factor big numbers using Brent-Pollard method.
 *   See "An Improved Monte Carlo Factorization Algorithm"
 *   by Richard Brent in BIT Vol. 20 1980 pp 176-184
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <iomanip.h>
#include <number.h>

#define min(a,b) ((a) < (b)? (a) : (b))

miracl precision=50;

void main()
{  /*  factoring program using Brents method */
    long k,r,i,m,iter;
    Big n,z;
    ZZn x,y,q,ys;
    cout << "input number to be factored\n";
    cin >> n;
    if (prime(n))
    {
        cout << "this number is prime!\n";
        exit(0);
    }
    m=10L;
    r=1L;
    iter=0L;
    y=0;
    do
    {
        modulo(n);    /* ZZn arithmetic done mod n */
        cout << "iterations=" << setw(5) << iter;
        q=1;
        do
        {
            x=y;
            for (i=1L;i<=r;i++) y=(y*y+3);
            k=0;
            do
            {
                iter++;
                if (iter%10==0) cout << "\b\b\b\b\b" << setw(5) << iter << flush;
                ys=y;
                for (i=1L;i<=min(m,r-k);i++)
                {   
                    y=(y*y+3);
                    q=((y-x)*q);
                }
                z=gcd(q,n);
                k+=m;
            } while (k<r && z==1);
            r*=2;
        } while (z==1);
        if (z==n) do 
        { /* back-track */
            ys=(ys*ys+3);
            z=gcd(ys-x,n);
        } while (z==1);
        if (!prime(z))
             cout << "\ncomposite factor ";
        else cout << "\nprime factor     ";
        cout << z << "\n";
        if (z==n) exit(0);
        n/=z;
    } while (!prime(n));      
    cout << "prime factor     ";
    cout << n << "\n";
}

