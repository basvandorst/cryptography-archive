/*
 *   Program to factor big numbers using Pomerance-Silverman-Montgomery  
 *   multiple polynomial quadratic sieve.
 *   See "The Multiple Polynomial Quadratic Sieve", R.D. Silverman,
 *   Math. Comp. Vol. 48, 177, Jan. 1987, pp329-339
 *
 */

#include <stdio.h>
#include <math.h> 
#include "miracl.h"

#define MEM 240        /* Maximum size of factor base   */
#define MLF 100        /* Max. number of large factors  */
#define PAK 1+MEM/(2*BTS)
#define SSIZE 10000    /* Maximum sieve size            */
#define LOG log        /* if log not available use any other log */

big NN,TT,DD,RR,VV,PP,XX,YY,DG,IG,AA,BB;
big x[MEM+1],y[MEM+1],z[MLF+1],w[MLF+1];
int ep[MEM+1],r1[MEM+1],r2[MEM+1],rp[MEM+1],pr[MLF+1];
int b[MEM+1],e[MEM+1],hash[2*MLF+1];
unsigned int EE[MEM+1][PAK],G[MLF+1][PAK];
unsigned char logp[MEM+1],sieve[SSIZE+1];
int mm,jj,nbts,nlp,lp,hmod,hmod2;
bool partial;

extern char *malloc();

int knuth(ep,N,R,D,T)
int ep[];
big N,R,D,T;
{ /* initialization */
    double fks,dp,top;
    bool found;
    int *primes;
    int i,j,mm,bk,nk,m,d,kk,r,p;
    static int K[]={0,1,2,3,5,6,7,10,11,13,14,15,17,0};
    printf("input number to be factored N= \n");
    d=cinnum(N,stdin);
    if (prime(N))
    {
        printf("this number is prime!\n");
        exit(0);
    }
    if (d<10) m=d;
    else m=25;
    if (d>20) m=30*(d-20);
    if (m>MEM) m=MEM;
    primes=gprime((-2)*(m+5));
    top=(-10.0e0);
    found=FALSE;
    nk=0;
    bk=0;
    ep[0]=1;
    ep[1]=2;
    do
    { /* search for best Knuth-Schroepel multiplier */
        mm=m;
        kk=K[++nk];
        if (kk==0)
        { /* finished */
            kk=K[bk];
            found=TRUE;
        }
        premult(N,kk,D);
        if (root(D,2,R))
        {
            printf("%dN is a perfect square!\n",kk);
            continue;
        }
        fks=LOG(2.0e0)/(2.0e0);
        r=subdiv(D,8,T);
        if (r==1) fks*=(4.0e0);
        if (r==5) fks*=(2.0e0);
        fks-=LOG((double)kk)/(2.0e0);
        i=0;
        j=1;
        while (j<m)
        { /* select small primes */
            p=primes[++i];
            if (p==0)
            {
                mm=j;
                break;
            }
            r=subdiv(D,p,T);
            if (spmd(r,(p-1)/2,p)<=1)
            { /* use only if Jacobi symbol = 0 or 1 */
                ep[++j]=p;
                dp=(double)p;
                if (kk%p==0) fks+=LOG(dp)/dp;
                else         fks+=2*LOG(dp)/(dp-1.0e0);
            }
        }
        if (fks>top)
        { /* find biggest fks */
            top=fks;
            bk=nk;
        }
    } while (!found);
    printf("using multiplier k= %d\n",kk);
    printf("and %d small primes as factor base\n",mm);
    free(primes);
    return mm;
}

bool factored(lptr,T)
big T;
long lptr;
{ /* factor quadratic residue */
    bool facted;
    int i,j,r,st;  
    partial=FALSE;
    facted=FALSE;
    for (j=1;j<=mm;j++)
    { /* now attempt complete factorisation of T */
        r=lptr%ep[j];
        if (r<0) r+=ep[j];
        if (r!=r1[j] && r!=r2[j]) continue;
        while (subdiv(T,ep[j],XX)==0)
        { /* cast out ep[j] */
            e[j]++;
            copy(XX,T);
        }
        st=size(T);
        if (st==1)
        {
           facted=TRUE;
           break;
        }
        if (size(XX)<=ep[j])
        { /* st is prime < ep[mm]^2 */
            if (st>=TOOBIG || (st/ep[mm])>=(1+MLF/50)) break;
            if (st<=ep[mm])
                for (i=j;i<=mm;i++)
                if (st==ep[i])
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

bool gotcha()
{ /* use new factorisation */
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
    copy(PP,XX);
    convert(1,YY);
    for (k=1;k<=mm;k++)
    { /* build up square part in YY  *
       * reducing e[k] to 0s and 1s */
        if (e[k]<2) continue;
        r=e[k]/2;
        e[k]%=2;
        convert(ep[k],TT);
        power(TT,r,TT,TT);
        multiply(TT,YY,YY);
    }
/* debug only - print out details of each factorization 
    cotnum(XX,stdout);
    cotnum(YY,stdout);
    if (e[0]==1) printf("-1");
    else printf("1");
    for (k=1;k<=mm;k++)
    {
        if (e[k]==0) continue;
        printf(".%d",ep[k]);
    }
    if (partial) printf(".%d\n",lp);
    else printf("\n");
*/
    if (partial)
    { /* factored with large prime */
        if (!found)
        { /* store new partial factorization */
            hash[had]=nlp;
            pr[nlp]=lp;
            copy(XX,z[nlp]);
            copy(YY,w[nlp]);
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                G[nlp][n]|=((e[j]&1)<<rb);
                if (++rb==nbts) n++,rb=0;
            }
            nlp++;
        }
        if (found)
        { /* match found so use as factorization */
            printf("\b\b\b\b\b*");
            mad(XX,z[hp],XX,NN,NN,XX);
            mad(YY,w[hp],YY,NN,NN,YY);
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                e[j]+=((G[hp][n]>>rb)&1);
                if (e[j]==2)
                {
                    premult(YY,ep[j],YY);
                    divide(YY,NN,NN);
                    e[j]=0;
                }
                if (++rb==nbts) n++,rb=0;
            }
            premult(YY,lp,YY);
            divide(YY,NN,NN);
        }
    }
    else printf("\b\b\b\b\b ");
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
            mad(XX,x[i],XX,NN,NN,XX);    /* This is very inefficient -  */
            mad(YY,y[i],YY,NN,NN,YY);    /* There must be a better way! */
            for (n=0,rb=0,j=0;j<=mm;j++)
            { /* Gaussian elimination */
                e[j]+=((EE[i][n]>>rb)&1);
                if (++rb==nbts) n++,rb=0;
            }
        }
        for (j=0;j<=mm;j++)
        { /* update YY */
            if (e[j]<2) continue;
            convert(ep[j],TT);
            power(TT,e[j]/2,NN,TT);
            mad(YY,TT,YY,NN,NN,YY);
        }
        if (!found)
        { /* store details in EE, x and y for later */
            b[k]=jj;
            copy(XX,x[jj]);
            copy(YY,y[jj]);
            for (n=0,rb=0,j=0;j<=mm;j++)
            {
                EE[jj][n]|=((e[j]&1)<<rb);
                if (++rb==nbts) n++,rb=0;
            }
            jj++;
            printf("%4d",jj);
        }
    }
    if (found)
    { /* check for false alarm */
        printf("\ntrying...\n");
        add(XX,YY,TT);
        if (compare(XX,YY)==0 || compare(TT,NN)==0) found=FALSE;
        if (!found) printf("working... %4d",jj);
    }
    return found;
}

void initv()
{ /* initialize big numbers and arrays */
    int i,j;
    NN=mirvar(0); 
    TT=mirvar(0);
    DD=mirvar(0);
    RR=mirvar(0);
    VV=mirvar(0);
    PP=mirvar(0);
    XX=mirvar(0);
    YY=mirvar(0);
    DG=mirvar(0);
    IG=mirvar(0);
    AA=mirvar(0);
    BB=mirvar(0);
    for (i=0;i<=MEM;i++)
    {
        x[i]=mirvar(0);
        y[i]=mirvar(0);
    }
    for (i=0;i<=MLF;i++)
    {
        z[i]=mirvar(0);
        w[i]=mirvar(0);
    }

    nbts=8*sizeof(int);
    mm=knuth(ep,NN,RR,DD,TT);

    for (i=0;i<=mm;i++)
    { 
        b[i]=(-1);
        for (j=0;j<=mm/nbts;j++)
            EE[i][j]=0;
    }
    for (i=0;i<=MLF;i++)
        for (j=0;j<=mm/nbts;j++)
            G[i][j]=0;
}
 
main()
{ /* factoring via quadratic sieve */
    int i,j,S,r,s1,s2,s,NS,logm,a,ptr,threshold,isz;
    unsigned int  *isp;
    long M,la,lptr;
    mirsys(-30,MAXBASE);
    initv();
    isz=SSIZE/sizeof(int);
    isp=(unsigned int *)sieve;  /* equivalence sieve to int array */

    hmod=2*MLF+1;               /* set up hash table */
    convert(hmod,TT);
    while (!prime(TT)) decr(TT,2,TT);
    hmod=size(TT);
    hmod2=hmod-2;
    for (i=0;i<hmod;i++) hash[i]=(-1);

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
        r=subdiv(DD,ep[i],TT);
        rp[i]=sqrmp(r,ep[i]);
        logp[i]=0;
        r=ep[i];
        while((r/=2)>0) logp[i]++;
    }
    r=subdiv(DD,8,TT);    /* take special care of 2 */
    if (r==5) logp[1]++;
    if (r==1) logp[1]+=2;

    threshold=logm+logb2(RR)-2*logp[mm];

    jj=0;
    nlp=0;
    premult(DD,2,DG);
    root(DG,2,DG);
    lconv(M,TT);
    divide(DG,TT,DG);
    root(DG,2,DG);
    if (subdiv(DG,2,TT)==0) incr(DG,1,DG);
    if (subdiv(DG,4,TT)==1) incr(DG,2,DG);
    printf("working...    0");
    forever
    { /* try a new polynomial */
        r=NTRY;
        NTRY=1;         /* speed up search for prime */
        do
        { /* looking for suitable prime DG = 3 mod 4 */
            do incr(DG,4,DG); while(!prime(DG));
            decr(DG,1,TT);
            subdiv(TT,2,TT);
            powmod(DD,TT,DG,TT);  /* check D is quad residue */
        } while (size(TT)!=1);
        NTRY=r;
        incr(DG,1,TT);
        subdiv(TT,4,TT);
        powmod(DD,TT,DG,BB);
        negate(DD,TT);
        mad(BB,BB,TT,DG,TT,TT);
        negate(TT,TT);
        premult(BB,2,AA);
        xgcd(AA,DG,AA,AA,AA);
        mad(AA,TT,TT,DG,DG,AA);
        multiply(AA,DG,TT);
        add(BB,TT,BB);        /* BB^2 = DD mod DG^2 */
        multiply(DG,DG,AA);   /* AA = DG*DG         */
        xgcd(DG,DD,IG,IG,IG); /* IG = 1/DG mod DD  */

        r1[0]=r2[0]=0;
        for (i=1;i<=mm;i++) 
        { /* find roots of quadratic mod each prime */
            s=subdiv(BB,ep[i],TT);
            r=subdiv(AA,ep[i],TT);
            r=inverse(r,ep[i]);     /* r = 1/AA mod p */
            s1=(-s+rp[i]);
            s2=(-s-rp[i]);
            r1[i]=smul(s1,r,ep[i]);
            r2[i]=smul(s2,r,ep[i]);
        }

        for (ptr=(-NS);ptr<NS;ptr++)
        { /* sieve over next period */
            la=(long)ptr*SSIZE;
            for (i=0;i<isz;i++) isp[i]=0;
            for (i=1;i<=mm;i++)
            { /* sieving with each prime */
                r=la%ep[i];
                s1=(r1[i]-r)%ep[i];
                if (s1<0) s1+=ep[i];
                s2=(r2[i]-r)%ep[i];
                if (s2<0) s2+=ep[i];
                for (j=s1;j<SSIZE;j+=ep[i]) sieve[j]+=logp[i];
                if (s1==s2) continue;
                for (j=s2;j<SSIZE;j+=ep[i]) sieve[j]+=logp[i];
            }

            for (a=0;a<SSIZE;a++)
            { /* main loop - look for fully factored residues */
                if (sieve[a]<threshold) continue;
                lptr=la+a;
                lconv(lptr,TT);
                S=0;
                multiply(AA,TT,TT);           /* TT = AAx + BB      */
                add(TT,BB,TT);
                mad(TT,IG,TT,DD,DD,PP);       /* PP = (AAx + BB)/G  */
                if (size(PP)<0) add(PP,DD,PP);
                mad(PP,PP,PP,DD,DD,VV);       /* VV = PP^2 mod kN  */
                absol(TT,TT);
                if (compare(TT,RR)<0) S=1;    /* check for -ve VV */
                if (S==1) subtract(DD,VV,VV);
                copy(VV,TT);
                e[0]=S;
                for (i=1;i<=mm;i++) e[i]=0;
                if (!factored(lptr,TT)) continue;
                if (gotcha())
                { /* factors found! */
                    gcd(TT,NN,PP);
                    printf("factors are\n");
                    if (prime(PP)) printf("prime factor     ");
                    else          printf("composite factor ");
                    cotnum(PP,stdout);
                    divide(NN,PP,NN);
                    if (prime(NN)) printf("prime factor     ");
                    else          printf("composite factor ");
                    cotnum(NN,stdout);
                    exit(0);
                }
            }
        }
    }
}

