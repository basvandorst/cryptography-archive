/*
 *   Program to factor big numbers using Williams (p+1) method.
 *   Works when for some prime divisor p of n, p+1 has only
 *   small factors.
 *   See "Speeding the Pollard and Elliptic Curve Methods"
 *   by Peter Montgomery, Math. Comp. Vol. 48. Jan. 1987 pp243-264
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <iomanip.h>
#include <number.h>

#define LIMIT1 4000    /* must be int, and > MULT/2 */
#define LIMIT2 200000L /* may be long */
#define MULT   210     /* must be int, product of small primes 2.3.. */
#define NTRYS  3       /* number of attempts */

miracl precision=50;   /* number of ints per ZZn */

static long p;
static int iv;
static ZZn b,q,fvw,fd,fp,fn,fu[1+MULT/2];
static bool cp[1+MULT/2];


void lucas(ZZn p,int r,ZZn& vp,ZZn& v)
{ /* calculate r-th elements of lucas sequence */
    ZZn t;
    int k,rr;
    vp=2;
    v=p;
    k=1;
    rr=r;
    while ((rr/=2)>1) k*=2;
    while (k>0)
    { /* use binary method */
        vp=v*vp-p;
        v=v*v-2;
        if ((r&k)>0)
        {
            t=p*v-vp;
            vp=v;
            v=t;
        }
        k/=2;
    }
}

void next_phase()
{ /* now change gear */
    ZZn t;
    fp=fu[1]=b;
    fd=b*b-2;
    fn=fd*b-b;
    for (int m=5;m<=MULT/2;m+=2)
    { /* store fu[m] = Vm(b) */
        t=fn*fd-fp;
        fp=fn;
        fn=t;
        if (!cp[m]) continue;
        fu[m]=t;
    }
    lucas(b,MULT,fp,fd);
    iv=p/MULT;
    if (p%MULT>MULT/2) iv++,p=2*(long)iv*MULT-p;
    lucas(fd,iv,fp,fvw);
    q=fvw-fu[p%MULT];
}

int giant_step()
{ /* increment giant step */
    ZZn t;
    iv++;
    p=(long)iv*MULT+1;
    t=fvw;
    fvw=fvw*fd-fp;
    fp=t;
    return 1;
}

void main()
{  /*  factoring program using Williams (p+1) method */
    int k,phase,m,nt,pos,btch;
    long i,pa;
    Big n,t;
    gprime(LIMIT1);
    for (m=1;m<=MULT/2;m+=2)
        if (igcd(MULT,m)==1) cp[m]=TRUE;
        else                 cp[m]=FALSE;
    cout << "input number to be factored\n";
    cin >> n;
    if (prime(n))
    {
        cout << "this number is prime!\n";
        exit(0);
    }
    modulo(n);                     /* do all arithmetic mod n */
    for (nt=0,k=3;k<10;k++)
    { /* try more than once for p+1 condition (may be p-1) */
        b=k;                       /* try b=3,4,5..        */
        nt++;
        phase=1;
        p=0;
        btch=50;
        i=0;
        cout << "phase 1 - trying all primes less than " << LIMIT1;
        cout << "\nprime= " << setw(8) << p;
        forever
        { /* main loop */
            if (phase==1)
            { /* looking for all factors of p+1 < LIMIT1 */
                p=PRIMES[i];
                if (PRIMES[i+1]==0)
                { /* now change gear */
                    phase=2;
                    cout << "\nphase 2 - trying last prime less than ";
                    cout << LIMIT2 << "\nprime= " << setw(8) << p;
                    next_phase();
                    btch*=10;
                    i++;
                    continue;
                }
                pa=p;
                while ((LIMIT1/p) > pa) pa*=p;
                lucas(b,(int)pa,fp,q);
                b=q;
                q-=2;
            }
            else
            { /* looking for last large prime factor of (p+1) */
                p+=2;
                pos=p%MULT;
                if (pos>MULT/2) pos=giant_step();
                if (!cp[pos]) continue;
                q*=(fvw-fu[pos]);  /* batching gcds */
            }
            if (i++%btch==0)
            { /* try for a solution */
                cout << "\b\b\b\b\b\b\b\b" << setw(8) << p << flush;
                t=gcd(q,n);
                if (t==1)
                {
                    if (p>LIMIT2) break;
                    else continue;
                }
                if (t==n)
                {
                    cout << "\ndegenerate case";
                    break;
                }
                if (prime(t)) cout << "\nprime factor     " << t;
                else          cout << "\ncomposite factor " << t;
                n/=t;
                if (prime(n)) cout << "\nprime factor     " << n;
                else          cout << "\ncomposite factor " << n;
                exit(0);
            }
        } 
        if (nt>=NTRYS) break;
        cout << "\ntrying again\n";
    }
    cout << "\nfailed to factor\n";
}

