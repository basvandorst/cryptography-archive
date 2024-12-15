/*
 *    MIRACL  C++  functions number.cpp
 *
 *    AUTHOR  :    M. Scott
 *             
 *    PURPOSE :    Implementation of class ZZn functions
 *    NOTE    :    Must be used in conjunction with big.h and big.cpp
 *
 *    Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <number.h>

Big *modn=NULL,*modnn=NULL;   /* not initialised yet! */

void restore(Big &b)
{ /* reduce modn */
    b%=*modn;
    if (b<0) b+=*modn;
}

void reduce(Big &b)
{/* reduce modnn */
    if (modnn==NULL)
    {
        cout << "MIRACL C++ Error - Modulus is not initialised\n";
        exit(0);
    }
    b%=*modnn;
    if (b<0) b+=*modnn;
}

void modulo(const Big& b)  
{ /* set modulus, which must be > 1 */
    if (b<2) 
    {
        cout << "MIRACL C++ Error - Modulus must be > 1\n";
        exit(0);
    }
    if (modn!=NULL) delete modn;
    if (modnn!=NULL) delete modnn; 
    modn=new Big;
    *modn=b;
    modnn=new Big;
    *modnn=norm(b);
}

ZZn& ZZn::operator/=( const ZZn& b)   
{ZZn z; z.fn=inverse(b.fn,*modn);fn=modmult(z.fn,fn,*modnn);return *this;}

ZZn operator-( const ZZn& b)
{ZZn nb; nb.fn=*modnn-b.fn; return nb;}
ZZn operator+( const ZZn& b,int i)
{ZZn abi; abi.fn=b.fn+i;reduce(abi.fn);return abi;}
ZZn operator+(int i, const ZZn& b)
{ZZn aib; aib.fn=b.fn+i;reduce(aib.fn);return aib;}
ZZn operator+( const ZZn& b1,  const ZZn& b2)
{ZZn abb;abb.fn=b1.fn+b2.fn;if (abb.fn>=*modnn) abb.fn-=*modnn; return abb;}

ZZn operator-(const ZZn& b, int i)
{ZZn mbi; mbi.fn=b.fn-i;reduce(mbi.fn); return mbi;}
ZZn operator-(int i, const ZZn& b)   
{ZZn mib; mib.fn=b.fn-i;mib.fn=(-mib.fn); reduce(mib.fn);
                                                     return mib;}
ZZn operator-(const ZZn& b1,  const ZZn& b2)
{ZZn mbb;mbb.fn=b1.fn-b2.fn; if (mbb.fn<0) mbb.fn+=*modnn; return mbb;}

ZZn operator*(const ZZn& b, int i)
{ZZn xbi; xbi.fn=b.fn*i;reduce(xbi.fn); return xbi;}
ZZn operator*(int i, const ZZn& b)
{ZZn xib; xib.fn=b.fn*i;reduce(xib.fn); return xib;}
ZZn operator*( const ZZn& b1, const ZZn& b2)
{ZZn xbb; xbb.fn=modmult(b1.fn,b2.fn,*modnn); return xbb;}

ZZn operator/(const ZZn& b1, const ZZn& b2)
{ZZn z; z.fn=inverse(b2.fn,*modn);z.fn=modmult(b1.fn,z.fn,*modnn);return z;}

ZZn pow( const ZZn& b,int n)  {ZZn z;z.fn=pow(b.fn,n,*modnn);return z;}
ZZn pow( int x, const ZZn& b) {ZZn z;z.fn=pow(x,b.fn,*modnn);return z;}
ZZn pow( const ZZn& b1, const ZZn& b2){ZZn z;z.fn=pow(b1.fn,b2.fn,*modnn);
                                                            return z;}

