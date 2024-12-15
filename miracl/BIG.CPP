/*
 *    MIRACL  C++ functions big.cpp
 *
 *    AUTHOR  :    N.Coghlan
 *                 Modified by M.Scott
 *             
 *    PURPOSE :    Implementation of class Big functions
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <big.h>

Big operator-(const Big& b)
{Big nb; negate(b.fn,nb.fn); return nb;}
Big operator+(const Big& b,int i)
{Big abi; incr(b.fn, i, abi.fn); return abi;}
Big operator+(int i,const Big& b)
{Big aib; incr(b.fn, i, aib.fn); return aib;}
Big operator+(const Big& b1, const Big& b2)
{Big abb;add(b1.fn,b2.fn,abb.fn);return abb;}

Big operator-(const Big& b, int i)
{Big mbi; decr(b.fn, i, mbi.fn); return mbi;}
Big operator-(int i, const Big& b)
{Big mib;decr(b.fn, i, mib.fn);negate(mib.fn,mib.fn);return mib;}
Big operator-(const Big& b1, const Big& b2)
{Big mbb; subtract(b1.fn,b2.fn,mbb.fn); return mbb;}

Big operator*(const Big& b, int i)
{Big xbi; premult(b.fn, i, xbi.fn); return xbi;}
Big operator*(int i, const Big& b)
{Big xib; premult(b.fn, i, xib.fn); return xib;}
Big operator*(const Big& b1, const Big& b2)
{Big xbb; multiply(b1.fn,b2.fn,xbb.fn); return xbb;}

Big operator/(const Big& b, int i)
{Big dbi; subdiv(b.fn, i, dbi.fn); return dbi;}
Big operator/(const Big& b1, const Big& b2)
{Big dbb; divide(b1.fn,b2.fn,dbb.fn); return dbb;}

int operator%(const Big& b, int i)
{Big mdbi; return(subdiv(b.fn,i, mdbi.fn));}
Big operator%(const Big& b1, const Big& b2)
{Big mdbb;copy(b1.fn,mdbb.fn);divide(mdbb.fn,b2.fn,b2.fn);return mdbb;}

Big operator<<(const Big& b, int i)
{Big ms; sftbit(b.fn,i,ms.fn); return ms;}

Big operator>>(const Big& b, int i)
{Big ms; sftbit(b.fn,-i,ms.fn); return ms;}

Big modmult(const Big& b1,const Big& b2,const Big& m)
{Big z; mad(b1.fn,b2.fn,b2.fn,m.fn,m.fn,z.fn); return z;}
Big norm(const Big& b) {Big z; normalise(b.fn,z.fn); return z;}
Big sqrt(const Big& b) {Big z; nroot(b.fn, 2, z.fn); return z;}
Big root(const Big &b,int n) {Big z; nroot(b.fn, n, z.fn); return z;}
Big gcd(const Big& b1, const Big& b2){Big z;egcd(b1.fn,b2.fn,z.fn);return z;}
Big pow(const Big& b,int n)
{Big z;if (size(b.fn)==2) expb2(n,z.fn);else power(b.fn,n,z.fn,z.fn);return z;}
Big pow(const Big& b1,int n, const Big& b3)
{Big z; power(b1.fn,n,b3.fn,z.fn); return z;}
Big pow(int x, const Big& b2, const Big& b3)
{Big z; powltr(x,b2.fn,b3.fn,z.fn); return z;}
Big pow(const Big& b1, const Big& b2, const Big& b3)
{Big z; powmod(b1.fn,b2.fn,b3.fn,z.fn); return z;}
Big inverse(const Big& b1, const Big& b2)
{Big z; xgcd(b1.fn,b2.fn,z.fn,z.fn,z.fn);return z;}
Big rand(const Big& b) {Big z; bigrand(b.fn,z.fn); return z;}
Big rand(int n,small b) {Big z; bigdig(n,b,z.fn);  return z;}
Big nextprime(const Big& b) {Big z; nxprime(b.fn,z.fn); return z;}

