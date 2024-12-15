/*
 *   Program to factor numbers using brute force.
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <big.h>

#define LIMIT 10000

miracl precision=50;

void main()
{
    int n,p;
    Big x;
    gprime(LIMIT); /* generate all primes < LIMIT */
    cout << "input number to be factored\n";
    cin >> x;
    n=0;
    p=PRIMES[0];
    cout << "factors are ";
    forever
    { /* try division by each prime */
        if (x%p==0)
        { /* factor found */
            x/=p;
            cout << "\nprime factor     " << p << flush;
            if (x==1) exit(0);
            continue;
        }
        if ((x/p)<=p)
        { /* must be prime */
            cout << "\nprime factor     " << x << "\n"; 
            exit(0);
        }
        p=PRIMES[++n];
        if (p==0) break;
    }
    if (prime(x)) cout << "\nprime factor     " << x << "\n";
    else          cout << "\ncomposite factor " << x << "\n";
}

