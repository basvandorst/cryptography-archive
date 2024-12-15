/*
 *   Program to factor big numbers using Pomerance-Silverman-Montgomery  
 *   multiple polynomial quadratic sieve.
 *   See "The Multiple Polynomial Quadratic Sieve", R.D. Silverman,
 *   Math. Comp. Vol. 48, 177, Jan. 1987, pp329-339
 *
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <iomanip.h>
#include <big.h>

#define MEM 225        /* Maximum size of factor base   */
#define MLF  50        /* Max. number of large factors  */
#define PAK 1+MEM/(MR_IBITS)
#define SSIZE 5000     /* Maximum sieve size            */

miracl precision=(-30);          /* number of bytes per big */

static int pk[]={0,1,2,3,5,6,7,10,11,13,14,15,17,0};
static Big NN,DD,DG,TT,RR,VV,PP,IG,AA,BB;
static Big x[MEM+1],y[MEM+1],z[MLF+1],w[MLF+1];
static int epr[MEM+1],r1[MEM+1],r2[MEM+1],rp[MEM+1],pr[MEM+1];
static int b[MEM+1],e[MEM+1],hash[2*MLF+1];
static unsigned int EE[MEM+1][PAK],G[MLF+1][PAK];
static unsigned char logp[MEM+1],sieve[SSIZE+1];
static int mm,jj,nbts,nlp,lp,hmod,hmod2;
static bool partial;

int knuth(int *ep,Big& N,Big& R,Big& D)
{ /* Input number to be factored N, find best multiplier k  *
   * set D=k.N and calculate factor base ep[] and R=sqrt(D) */
    Big T;
    double fks,dp,top;
    bool found;
    int i,j,mm,bk,nk,m,digits,kk,rem,p,maxp;
    cout << "input number to be factored N= \n";
    cin >> N;
    if (prime(N))
    {
        cout << "this number is prime!\n";
        exit(0);
    }
    T=N;
    digits=1;                   /* digits in N */
    while ((T/=10)>0) digits++;

    if (digits<10) m=digits;               
    else m=25;
    if (digits>20) m=30*(digits-20);
    if (m>MEM) m=MEM;
    dp=(double)2*(m+5);          /* number of primes to generate */
    if (dp<20.0) dp=20.0;
    maxp=(int)(dp*(log(dp*log(dp))-0.5)); /* Rossers upper bound */
    gprime(maxp);
    top=(-10.0e0);
    found=FALSE;
    nk=0;
    bk=0;
    ep[0]=1;
    ep[1]=2;
    do
    { /* search for best Knuth-Schroepel multiplier */
        mm=m;
        kk=pk[++nk];
        if (kk==0)
        { /* finished */
            kk=pk[bk];
            found=TRUE;
        }
        D=kk*N;
        R=sqrt(D);
        fks=log(2.0e0)/(2.0e0);
        rem=D%8;
        if (rem==1) fks*=(4.0e0);
        if (rem==5) fks*=(2.0e0);
        fks-=log((double)kk)/(2.0e0);
        i=0;
        j=1;
        while (j<m)
        { /* select small primes */
            p=PRIMES[++i];
            if (p==0)
            {
                mm=j;
                break;
            }
            rem=D%p;
            if (spmd(rem,(p-1)/2,p)<=1) /* x = spmd(a,b,c) = a^b mod c */
            { /* use only if Jacobi symbol = 0 or 1 */
                ep[++j]=p;
                dp=(double)p;
                if (kk%p==0) fks+=log(dp)/dp;
                else         fks+=2*log(dp)/(dp-1.0e0);
            }
        }
        if (fks>top)
        { /* find biggest fks */
            top=fks;
            bk=nk;
        }
    } while (!found);
    cout << "using multiplier k= " << kk;
    cout << "\nand " << mm << " small primes as factor base\n";
    gprime(0);   /* reclaim PRIMES space */
    return mm;
}

void initv()
{ /* initialize */
    Big TT;
    int i,j;
    nbts=8*sizeof(int);

    for (i=0;i<=mm;i++)
    { 
        b[i]=(-1);
        for (j=0;j<=mm/nbts;j++)
            EE[i][j]=0;
    }
    for (i=0;i<=MLF;i++)
        for (j=0;j<=mm/nbts;j++)
            G[i][j]=0;

    hmod=2*MLF+1;               /* set up hash table */
    TT=hmod;
    while (!prime(TT)) TT-=2;
    hmod=toint(TT);
    hmod2=hmod-2;
    for (i=0;i<hmod;i++) hash[i]=(-1);
    jj=0;
    nlp=0;
}

bool gotcha(Big& NN,Big& P)
{ /* use new factorisation */
    Big XX,YY,T;
    int r,j,i,k,n,rb,had,hp;
    bool found;
    found=TRUE;
    if (partial)
    { /* check partial factorisation for usefulness */
        had=lp%hmod;
        forever
        { /* hash search for matching large prime */
            hp=hash[had];
            if (hp<0)
            { /* failed to find match */
                found=FALSE;
                break;
            }
            if (pr[hp]==lp) break; /* hash hit! */
            had=(had+(hmod2-lp%hmod2))%hmod;
        }
        if (!found && nlp>=MLF) return FALSE;
    }
    XX=P;
    YY=1;
    for (k=1;k<=mm;k++)
    { /* build up square part in YY  *
       * reducing e[k] to 0s and 1s */
        if (e[k]<2) continue;
        r=e[k]/2;
        e[k]%=2;
        T=epr[k];
        YY*=pow(T,r);
    }
    if (partial)
    { /* factored with large prime */
        if (!found)
        { /* store new partial factorization   */
            hash[had]=nlp;
            pr[nlp]=lp;
            z[nlp]=XX;
            w[nlp]=YY;
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                G[nlp][n]|=((e[j]&1)<<rb);
                if (++rb==nbts) n++,rb=0;
            }
            nlp++;         
        }
        if (found)
        { /* match found so use as factorization  */
            cout << "\b\b\b\b\b*";
            XX=(XX*z[hp])%NN;
            YY=(YY*w[hp])%NN;
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                e[j]+=((G[hp][n]>>rb)&1);
                if (e[j]==2)
                {
                    YY=(YY*epr[j])%NN;
                    e[j]=0;
                }
                if (++rb==nbts) n++,rb=0;
            }
            YY=(YY*lp)%NN; 
        }
    }
    else cout << "\b\b\b\b\b " << flush;
    if (found)
    {
        for (k=mm;k>=0;k--)
        { /* use new factorization in search for solution */
            if (e[k]%2==0) continue;
            if (b[k]<0)
            { /* no solution this time */
                found=FALSE;
                break;
            }
            i=b[k];
            XX=(XX*x[i])%NN;    /* This is very inefficient -  */
            YY=(YY*y[i])%NN;    /* There must be a better way! */
            for (n=0,rb=0,j=0;j<=mm;j++)
            { /* Gaussian elimination */
                e[j]+=((EE[i][n]>>rb)&1);
                if (++rb==nbts) n++,rb=0;
            }
        }
        for (j=0;j<=mm;j++)
        { /* update YY */
            if (e[j]<2) continue;
            T=epr[j];
            YY=(YY*pow(T,e[j]/2,NN))%NN;  /* x = pow(a,b,c) = a^b mod c */
        }
        if (!found)
        { /* store details in EE, x and y for later  */
            b[k]=jj;
            x[jj]=XX;
            y[jj]=YY;
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                EE[jj][n]|=((e[j]&1)<<rb);
                if (++rb==nbts) n++,rb=0;
            }
            jj++;
            cout << setw(4) << jj; 
        }
    }
    if (found)
    { /* check for false alarm */
        P=XX+YY;
        cout << "\ntrying...\n";
        if (XX==YY || P==NN) found=FALSE;
        if (!found) cout << "working... " << setw(4) << jj << flush;
    }
    return found;
}

bool factored(long lptr,Big& T)
{ /* factor quadratic residue T */
    bool facted;
    int i,j,r,st;  
    partial=FALSE;
    facted=FALSE;
    for (j=1;j<=mm;j++)
    { /* now attempt complete factorisation of T */
        r=lptr%epr[j];
        if (r<0) r+=epr[j];
        if (r!=r1[j] && r!=r2[j]) continue;
        while (T%epr[j]==0)
        { /* cast out epr[j] */
            e[j]++;
            T/=epr[j];
        }
        st=toint(T);      /*  st = T as an int; st=TOOBIG if too big */
        if (st==1)
        {
           facted=TRUE;
           break;
        }
        if ((T/epr[j])<=epr[j])
        { /* st is prime < epr[mm]^2 */
            if (st==TOOBIG || (st/epr[mm])>(1+MLF/50)) break;
            if (st<=epr[mm])
                for (i=j;i<=mm;i++)
                if (st==epr[i])
                {
                    e[i]++;
                    facted=TRUE;
                    break;
                }
            if (facted) break;
            lp=st;  /* factored with large prime */
            partial=TRUE;
            facted=TRUE;
            break;
        }
    }
    return facted;
} 

void new_poly()
{ /* form the next polynomial */
    int i,r,s,s1,s2;
    r=MR_NTRY;        /* MR_NTRY is global - number of trys at proving */
    MR_NTRY=1;        /* a probable prime  */
    do
    { /* looking for suitable prime DG = 3 mod 4 */
        do DG+=4; while(!prime(DG));
        TT=(DG-1)/2;
        TT=pow(DD,TT,DG);  /*  check DD is quad residue */
    } while (TT!=1);
    MR_NTRY=r;
    TT=(DG+1)/4;
    BB=pow(DD,TT,DG);
    TT=(DD-BB*BB)/DG;
    AA=inverse(2*BB,DG);
    AA=(AA*TT)%DG;
    BB=AA*DG+BB;       /* BB^2 = DD mod DG^2  */
    AA=DG*DG;
    IG=inverse(DG,DD); /*  IG = 1/DG mod DD  */ 
    r1[0]=r2[0]=0;
    for (i=1;i<=mm;i++) 
    { /* find roots of quadratic mod each prime */
        s=BB%epr[i];
        r=AA%epr[i];
        r=invers(r,epr[i]);      /* r = 1/AA mod p */
        s1=(-s+rp[i]);
        s2=(-s-rp[i]);
        r1[i]=smul(s1,r,epr[i]);  /* s1 = s1*r mod epr[i] */
        r2[i]=smul(s2,r,epr[i]);  
    }
}


void main()
{ /* factoring via quadratic sieve */
    unsigned int i,j,a,isz;
    int S,r,s1,s2,NS,logm,ptr,threshold;
    unsigned int  *isp;
    long M,la,lptr;
    mm=knuth(epr,NN,RR,DD); 
    initv();
    isz=SSIZE/sizeof(int);
    isp=(unsigned int *)sieve;  /* equivalence sieve to int array */

    M=50*(long)mm;
    NS=M/SSIZE;
    if (M%SSIZE!=0) NS++;
    M=SSIZE*(long)NS;
    logm=0;
    la=M;
    while ((la/=2)>0) logm++;   /* logm = log(M) */

    rp[0]=logp[0]=0;
    for (i=1;i<=mm;i++)
    { /* find root mod each prime, and approx log of each prime */
        r=DD%epr[i];
        rp[i]=sqrmp(r,epr[i]);     /* = square root of r mod epr[i] */
        logp[i]=0;
        r=epr[i];
        while((r/=2)>0) logp[i]++;
    }
    r=DD%8;             /* take special care of 2  */
    if (r==5) logp[1]++;
    if (r==1) logp[1]+=2;
    threshold=logm-2*logp[mm]; 
    TT=RR;
    while ((TT/=2)>0) threshold++;   /*  add in number of bits in RR */
    DG=sqrt(DD*2);
    DG=sqrt(DG/M);
    if (DG%2==0) ++DG;
    if (DG%4==1) DG+=2;
    cout << "working...    0" << flush;
    forever
    { /* try a new polynomial  */

        new_poly();

        for (ptr=(-NS);ptr<NS;ptr++)
        { /* sieve over next period */
            la=(long)ptr*SSIZE;
            for (i=0;i<isz;i++) isp[i]=0;
            for (i=1;i<=mm;i++)
            { /* sieving with each prime */
                r=la%epr[i];
                s1=(r1[i]-r)%epr[i];
                if (s1<0) s1+=epr[i];
                s2=(r2[i]-r)%epr[i];
                if (s2<0) s2+=epr[i];
                for (j=s1;j<SSIZE;j+=epr[i]) sieve[j]+=logp[i];
                if (s1==s2) continue;
                for (j=s2;j<SSIZE;j+=epr[i]) sieve[j]+=logp[i];
            }

            for (a=0;a<SSIZE;a++)
            {  /* main loop - look for fully factored residues */
                if (sieve[a]<threshold) continue;
                lptr=la+a;
                S=0;
                TT=AA*lptr+BB;
                PP=(TT*IG)%DD;             /*  PP = (AAx + BB)/G */
                if (PP<0) PP+=DD;
                VV=(PP*PP)%DD;             /*  VV = PP^2 mod kN  */
                if (TT<0) TT=(-TT);
                if (TT<RR) S=1;            /*  check for -ve VV  */
                if (S==1) VV=DD-VV;
                e[0]=S;
                for (i=1;i<=mm;i++) e[i]=0;
                if (!factored(lptr,VV)) continue;
                if (gotcha(NN,PP))
                { /* factors found! */
                    PP=gcd(PP,NN);
                    cout << "factors are";
                    if (prime(PP)) cout << "\nprime factor     " << PP;
                    else           cout << "\ncomposite factor " << PP;
                    NN/=PP;
                    if (prime(NN)) cout << "\nprime factor     " << NN;
                    else           cout << "\ncomposite factor " << NN;
                    exit(0);
                }
            }
        }
    }
}

