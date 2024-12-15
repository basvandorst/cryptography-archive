/*
 *    MIRACL  C++ Header file flash.h
 *
 *    AUTHOR  :    N.Coghlan
 *                 Modified by M.Scott
 *             
 *    PURPOSE :    Definition of class Flash
 *
 *    Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#ifndef FLASH_H
#define FLASH_H

#include <stdlib.h>
#include <math.h>
#include <stdiostream.h>

extern "C"    
{
    #include <miracl.h>
}

#ifndef MIRACL_CLASS
#define MIRACL_CLASS

class miracl
{ /* dummy class to initialise MIRACL - MUST be called before any Bigs    *
   * are created. This could be a problem for static/global data declared *
   * in modules other than the main module */
public:
    miracl(int nd,small nb=0) {mirsys(nd,nb);POINT=TRUE;}
    ~miracl()                 {mirexit();}
};

#endif

class Flash
{ /* Flash Class Definitions */
    flash fn;      /* pointer to actual data */
public:
    Flash()         {fn=mirvar(0);}
    Flash(int i)    {fn=mirvar(i);}
    Flash(long lg)  {fn=mirvar(0);   lgconv(lg,fn);}
    Flash(double d) {fn=mirvar(0);    dconv(d,fn);}
    Flash(Flash& f) {fn=mirvar(0); copy(f.fn, fn);}
    Flash(char* s)  {fn=mirvar(0); cinstr(fn,s);}

    Flash& operator=(int i) {convert(i,fn); return *this;}
    Flash& operator=(long lg){lgconv(lg,fn); return *this;}
    Flash& operator=(double& d)  {dconv(d,fn);    return *this;}
    Flash& operator=(const Flash& f)   {copy(f.fn, fn); return *this;}
    Flash& operator=(char* s)    {cinstr(fn,s);return *this;}

    Flash& operator++()      {fincr(fn,1,1,fn);  return *this;}
    Flash& operator--()      {fincr(fn,-1,1,fn); return *this;}
    Flash& operator+=(int i) {fincr(fn,i,1,fn);  return *this;}
    Flash& operator+=(const Flash& f) {fadd(fn,f.fn,fn);  return *this;}
    Flash& operator-=(int i) {fincr(fn,-i,1,fn); return *this;}
    Flash& operator-=(const Flash& f) {fsub(fn,f.fn,fn);  return *this;}
    Flash& operator*=(int i) {fpmul(fn,i,1,fn);  return *this;}
    Flash& operator*=(const Flash& f) {fmul(fn,f.fn,fn);  return *this;}
    Flash& operator/=(int i) {fpmul(fn,1,i,fn);  return *this;}
    Flash& operator/=(const Flash& f) {fdiv(fn,f.fn,fn);  return *this;}

    friend Flash operator-(const Flash&);   /* unary - */

    /* binary ops */

    friend Flash operator+(const Flash&, const Flash&);
    friend Flash operator+(const Flash&, int);
    friend Flash operator+(int, const Flash&);

    friend Flash operator-(const Flash&, const Flash&);
    friend Flash operator-(const Flash&, int);
    friend Flash operator-(int, const Flash&);

    friend Flash operator*(const Flash&, const Flash&);
    friend Flash operator*(const Flash&, int);
    friend Flash operator*(int, const Flash&);

    friend Flash operator/(const Flash&, const Flash&);
    friend Flash operator/(const Flash&, int);
    friend Flash operator/(int, const Flash&);

    /* relational ops */

    friend bool operator<=(const Flash& f1, const Flash& f2)
    {if (fcomp(f1.fn,f2.fn) <= 0) return TRUE; else return FALSE;}
    friend bool operator>=(const Flash& f1, const Flash& f2) 
    {if (fcomp(f1.fn,f2.fn) >= 0) return TRUE; else return FALSE;}
    friend bool operator==(const Flash& f1, const Flash& f2)
    {if (fcomp(f1.fn,f2.fn) == 0) return TRUE; else return FALSE;}
    friend bool operator!=(const Flash& f1, const Flash& f2)
    {if (fcomp(f1.fn,f2.fn) != 0) return TRUE; else return FALSE;}
    friend bool operator<(const Flash& f1, const Flash& f2)
    {if (fcomp(f1.fn,f2.fn) < 0)  return TRUE; else return FALSE;}
    friend bool operator>(const Flash& f1, const Flash& f2) 
    {if (fcomp(f1.fn,f2.fn) > 0)  return TRUE; else return FALSE;}

    friend Flash pi(); 
    friend Flash cos(const Flash&);
    friend Flash sin(const Flash&);
    friend Flash tan(const Flash&);

    friend Flash acos(const Flash&);
    friend Flash asin(const Flash&);
    friend Flash atan(const Flash&);

    friend Flash cosh(const Flash&);
    friend Flash sinh(const Flash&);
    friend Flash tanh(const Flash&);

    friend Flash acosh(const Flash&);
    friend Flash asinh(const Flash&);
    friend Flash atanh(const Flash&);

    friend Flash log(const Flash&);
    friend Flash exp(const Flash&);
    friend Flash pow(const Flash&,const Flash&);
    friend Flash pow(const Flash&,int);
    friend Flash sqrt(const Flash&);
    friend Flash fabs(const Flash&);

    friend double todouble(const Flash& f) { return fdsize(f.fn);}

    friend istream& operator>>(istream& s, Flash& x)
    { s >> (char *)MR_IOBUFF; if (s.eof() || s.bad()) {zero(x.fn); return s;} 
      cinstr(x.fn,(char *)MR_IOBUFF) ; return s; }
    friend ostream& operator<<(ostream& s, const Flash& x)
    { cotstr(x.fn,(char *)MR_IOBUFF); s << (char *)MR_IOBUFF;   return s;}

    ~Flash()   {free((char *)fn);}
};

#endif

