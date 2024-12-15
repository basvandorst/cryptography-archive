/*
 *   MIRACL fast fourier multiplication routine, using 3 prime method.
 *   mrfast.c  - only faster for very high precision multiplication
 *               of numbers > about 500 decimal digits  (see below)
 *   See "The Fast Fourier Transform in a Finite Field"  by J.M. Pollard,
 *   Mathematics of Computation, Vol. 25, No. 114, April 1971, pp365-374
 *   Also Knuth Vol. 2, Chapt. 4.3.3
 *
 *   Takes time preportional to 9+15N+9N.lg(N) to multiply two different 
 *   N digit numbers. This reduces to 6+18N+6N.lg(N) when squaring.
 *   The classic method takes N.N and N(N+1)/2 respectively
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <miracl.h>

extern small MR_MSBIT;   /* msb of small    */
extern small MR_OBITS;   /* other bits      */
extern small mr_base;    /* number base     */
extern int  mr_nib;      /* length of bigs  */
extern int  mr_depth;    /* error tracing ..*/
extern int  mr_trace[];  /* .. mechanism    */
extern bool mr_check;    /* error checking  */

static int logN,N=0;
static utype prime[3];
static utype inverse[3],*roots[3];
static utype const1,const2,const3;
static small msw,lsw;

static void init_it(newn)
int newn;
{ /* find primes, table of roots, inverses etc for new N */
    small kk;
    int r,pr,j;
    utype p,proot;
    logN=0;
    r=newn;
    while ((r>>=1)!=0) logN++;
    kk=((small)1<<(MR_SBITS-2-logN));
    if (mr_base!=0) while (kk*newn>mr_base) kk>>=1;
    for (pr=0;pr<3;pr++)
    { /* find three suitable primes */
        if (N!=0) free(roots[pr]);
        roots[pr]=(utype *)calloc((newn+1),sizeof(utype));
        if (roots[pr]==NULL)
        {
            berror(8);
            mr_depth--;
            return;
        }
        do
        { /* look for prime of required form */
            kk--;
            p=kk*newn+1;
        } while(spmd((small)2,p-1,p)!=1);
        proot=p-1;
        roots[pr][0]=1; /* find primitive Nth root - and set up table */
        for (j=1;j<logN;j++) proot=sqrmp(proot,p); 
        for (j=1;j<=newn;j++) roots[pr][j]=smul(roots[pr][j-1],proot,p);
        inverse[pr]=invers((small)newn,p);
        prime[pr]=p;
    }
    const1=invers(prime[0],prime[1]);
    const2=invers(prime[0],prime[2]);
    const3=invers(prime[1],prime[2]);
    if (mr_base==0) msw=muldvd(prime[0],prime[1],(small)0,&lsw);
    else            msw=muldiv(prime[0],prime[1],(small)0,mr_base,&lsw);
    N=newn;
}

static void dif_fft(data,prime,roots)
utype data[],prime,roots[];
{ /* decimate-in-frequency fourier transform */
    int mmax,m,j,k,istep,i,ii,jj;
    utype w,temp;
    mmax=N;
    for (k=1;k<=logN;k++) {
        istep=mmax;
        mmax>>=1; 
        ii=N;
        jj=N/istep;
        ii-=jj;
        for (i=1;i<=N;i+=istep)
        { /* special case root=1 */ 
            j=i+mmax;
            temp=data[i]-data[j];
            if (temp<0) temp+=prime;
            data[i]+=data[j];
            if (data[i]>prime) data[i]-=prime;
            data[j]=temp;
        }
        for (m=2;m<=mmax;m++) {
            w=roots[ii];
            ii-=jj;
            for (i=m;i<=N;i+=istep) {
                j=i+mmax;
                temp=data[i]-data[j];
                if (temp<0) temp+=prime;
                data[i]+=data[j];
                if (data[i]>prime) data[i]-=prime;
                muldiv(w,temp,(small)0,prime,(small *)&data[j]);
            }
        }
    }
}

static void dit_fft(data,prime,roots)
utype data[],prime,roots[];
{ /* decimate-in-time inverse fourier transform */
    int mmax,m,j,k,i,istep,ii,jj;
    utype w,temp;
    mmax=1;
    for (k=1;k<=logN;k++) {
        istep=(mmax<<1); 
        ii=0;
        jj=N/istep;
        ii+=jj;
        for (i=1;i<=N;i+=istep)
        { /* special case root=1 */
            j=i+mmax;
            temp=data[j];
            data[j]=data[i]-temp;
            if (data[j]<0) data[j]+=prime;
            data[i]+=temp;
            if (data[i]>prime) data[i]-=prime;
        }
        for (m=2;m<=mmax;m++) {
            w=roots[ii];
            ii+=jj;
            for (i=m;i<=N;i+=istep) {
                j=i+mmax;
                muldiv(w,data[j],(small)0,prime,(small *)&temp);
                data[j]=data[i]-temp;
                if (data[j]<0) data[j]+=prime;
                data[i] += temp;
                if (data[i]>prime) data[i]-=prime;
            }
        }
        mmax=istep;
    }
}

void fastmult(x,y,z)
big x,y,z;
{ /* "fast" O(n.log n) multiplication */
    int i,pr,xl,yl,zl,newn;
    small v1,v2,v3,ic,c1,c2,sz,p;
    utype *work,*d[3],*dptr,*d0,*d1,*d2,t;
    if (MR_ERNUM) return;
    if (y[0]==0 || x[0]==0) 
    {
        zero(z);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=72;
    if (TRACER) track();
    if (notint(x) || notint(y))
    {
        berror(12);
        mr_depth--;
        return;
    }  
    sz=((x[0]&MR_MSBIT)^(y[0]&MR_MSBIT));
    xl=(int)(x[0]&MR_OBITS);
    yl=(int)(y[0]&MR_OBITS);
    zl=xl+yl;
    if (xl<128 || yl<128)
    { /* not worth it! */
        multiply(x,y,z);
        mr_depth--;
        return;
    }
    if (zl>mr_nib && mr_check)
    {
        berror(3);
        mr_depth--;
        return;
    }
    newn=1;
    while (zl>newn) newn<<=1;
    if (N!=newn) init_it(newn);
    if (x!=y) 
    { /* get work space */
        work=(utype *)calloc((N+1),sizeof(utype));
        if (work==NULL)
        {
            berror(8);
            mr_depth--;
            return;
        }
    }
    for (pr=0;pr<3;pr++)
    { 
        d[pr]=(utype *)calloc((N+1),sizeof(utype));
        if (d[pr]==NULL)
        {
            berror(8);
            mr_depth--;
            return;
        }
    }
    d0=d[0];
    d1=d[1];
    d2=d[2];
    for (pr=0;pr<3;pr++)
    { /* multiply mod each prime */
        p=prime[pr];
        dptr=d[pr];
        for (i=1;i<=xl;i++) dptr[i]=x[i]%p;
        dif_fft(dptr,p,roots[pr]);
        if (x!=y)
        {
            for (i=1;i<=yl;i++) work[i]=y[i]%p;
            for (i=yl+1;i<=N;i++) work[i]=0;
            dif_fft(work,p,roots[pr]);
        }
        else work=dptr;
        for (i=1;i<=N;i++)
        {  /* "multiply" Fourier transforms */
            muldiv(dptr[i],work[i],(small)0,p,(small *)&dptr[i]);
        }
        dit_fft(dptr,p,roots[pr]);

        for (i=1;i<=N;i++)
        {  /* convert to mixed radix form     *
            * using chinese remainder thereom */
            muldiv(dptr[i],inverse[pr],(small)0,p,(small *)&dptr[i]); 
            if (pr==1)
            {
                t=d1[i]-d0[i];
                while (t<0) t+=prime[1];
                muldiv(t,const1,(small)0,prime[1],(small *)&d1[i]);
            } 
            if (pr==2)
            {
                t=d2[i]-d0[i];
                while (t<0) t+=prime[2];
                muldiv(t,const2,(small)0,prime[2],(small *)&t);
                t-=d1[i];
                while (t<0) t+=prime[2];
                muldiv(t,const3,(small)0,prime[2],(small *)&d2[i]);
            }
        }    
    }
    zero(z);
    ic=c1=c2=0;
    if (mr_base==0) for (i=1;i<=zl;i++)
    { /* propagate carries */
        v1=d0[i];
        v2=d1[i];
        v3=d2[i];
        v2=muldvd(v2,prime[0],v1,&v1);
        c1+=v1;
        if (c1<v1) v2++;
        ic=c2+muldvd(lsw,v3,c1,&z[i]);
        c2=muldvd(msw,v3,ic,&c1);
        c1+=v2;
        if (c1<v2) c2++;
    }
    else for (i=1;i<=zl;i++)
    { /* propagate carries */
        v1=d0[i];
        v2=d1[i];
        v3=d2[i];
        v2=muldiv(v2,prime[0],v1+c1,mr_base,&v1);
        ic=c2+muldiv(lsw,v3,v1,mr_base,&z[i]);
        c2=muldiv(msw,v3,v2+ic,mr_base,&c1);
    }
    for (pr=0;pr<3;pr++) free(d[pr]);
    if (x!=y) free(work);
    z[0]=(sz|zl); /* set length and sign of result */
    lzero(z);
    mr_depth--;
}

