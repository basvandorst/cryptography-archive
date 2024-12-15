/*
 *    MIRACL  C++  Flash functions flash.cpp 
 *
 *    AUTHOR  :    N.Coghlan
 *                 Modified by M.Scott
 *             
 *    PURPOSE :    Implementation of Class Flash functions
 *
 *    Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <flash.h>

Flash operator-(const Flash& f)  {Flash nf; negate(f.fn,nf.fn); return nf;}

Flash operator+(const Flash& f1, const Flash& f2)
{Flash aff; fadd(f1.fn,f2.fn,aff.fn); return aff;}
Flash operator+(const Flash& f,int i)
{Flash afi;    fincr(f.fn,i,1,afi.fn);return afi;}
Flash operator+(int i,const Flash& f)
{Flash aif;    fincr(f.fn,i,1,aif.fn);return aif;}

Flash operator-(const Flash& f1, const Flash& f2)
{Flash mff; fsub(f1.fn,f2.fn,mff.fn); return mff;}
Flash operator-(const Flash& f, int i)
{Flash mfi; fincr(f.fn,(-i),1,mfi.fn);return mfi;}
Flash operator-(int i, const Flash& f)
{Flash mif;fincr(f.fn,(-i),1,mif.fn);negate(mif.fn,mif.fn);return mif;}

Flash operator*(const Flash& f1, const Flash& f2)
{Flash xff; fmul(f1.fn,f2.fn,xff.fn); return xff;}
Flash operator*(const Flash& f, int i)
{Flash xfi;  fpmul(f.fn,i,1,xfi.fn);  return xfi;}
Flash operator*(int i, const Flash& f)
{Flash xif;  fpmul(f.fn,i,1,xif.fn);  return xif;}

Flash operator/(const Flash& f1, const Flash& f2)
{Flash dff; fdiv(f1.fn,f2.fn,dff.fn); return dff;}
Flash operator/(const Flash& f, int i)
{Flash dfi;  fpmul(f.fn,1,i,dfi.fn);  return dfi;}
Flash operator/(int i, const Flash& f)
{Flash dif;  fpmul(f.fn,1,i,dif.fn);frecip(dif.fn,dif.fn); return dif;}

Flash pi() {Flash z; fpi(z.fn); return z;}

Flash cos(const Flash& f) {Flash z; fcos(f.fn, z.fn); return z;}
Flash sin(const Flash& f) {Flash z; fsin(f.fn, z.fn); return z;}
Flash tan(const Flash& f) {Flash z; ftan(f.fn, z.fn); return z;}

Flash acos(const Flash& f){Flash z; facos(f.fn, z.fn);return z;}
Flash asin(const Flash& f){Flash z; fasin(f.fn, z.fn);return z;}
Flash atan(const Flash& f){Flash z; fatan(f.fn, z.fn);return z;}

Flash cosh(const Flash& f){Flash z; fcosh(f.fn, z.fn);return z;}
Flash sinh(const Flash& f){Flash z; fsinh(f.fn, z.fn);return z;}
Flash tanh(const Flash& f){Flash z; ftanh(f.fn, z.fn);return z;}

Flash acosh(const Flash& f){Flash z; facosh(f.fn, z.fn);return z;}
Flash asinh(const Flash& f){Flash z; fasinh(f.fn, z.fn);return z;}
Flash atanh(const Flash& f){Flash z; fatanh(f.fn, z.fn);return z;}

Flash log(const Flash& f) {Flash z; flog(f.fn, z.fn); return z;}
Flash exp(const Flash& f) {Flash z; fexp(f.fn, z.fn); return z;}
Flash pow(const Flash& f1,const Flash& f2)            
{Flash z;fpowf(f1.fn,f2.fn,z.fn); return z;}
Flash pow(const Flash& f,int i)
{Flash z; fpower(f.fn,i,z.fn); return z;}

Flash sqrt(const Flash& f) {Flash z; froot(f.fn, 2, z.fn); return z;}
Flash fabs(const Flash& f) {Flash z; absol(f.fn,z.fn);     return z;}

