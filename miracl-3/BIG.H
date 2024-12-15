/*
 *    MIRACL  C++ Header file big.h
 *
 *    AUTHOR  :    N.Coghlan
 *                 Modified by M.Scott
 *             
 *    PURPOSE :    Definition of class Big
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#ifndef BIG_H
#define BIG_H

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

class Big 
{ 
    big fn;
public:
    Big()        {fn = mirvar(0); } 
    Big(int i)   {fn = mirvar(i); }
    Big(long lg) {fn = mirvar(0); lgconv(lg,fn);}
    Big(char* s) {fn = mirvar(0); cinstr(fn,s);}
    Big(const Big& b)  {fn = mirvar(0); copy(b.fn,fn);}

    Big& operator=(int i)  {convert(i,fn); return *this;}
    Big& operator=(long lg){lgconv(lg,fn); return *this;}
    Big& operator=(const Big& b) {copy(b.fn,fn); return *this;}
    Big& operator=(char* s){cinstr(fn,s);return *this;}

    Big& operator++()      {incr(fn,1,fn); return *this;}
    Big& operator--()      {decr(fn,1,fn); return *this;}
    Big& operator-()       {negate(fn,fn); return *this;}
    Big& operator+=(int i) {incr(fn,i,fn); return *this;}
    Big& operator+=(const Big& b){add(fn,b.fn,fn); return *this;}

    Big& operator-=(int i)  {decr(fn,i,fn); return *this;}
    Big& operator-=(const Big& b) {subtract(fn,b.fn,fn); return *this;}

    Big& operator*=(int i)  {premult(fn,i,fn); return *this;}
    Big& operator*=(const Big& b) {multiply(fn,b.fn,fn); return *this;}

    Big& operator/=(int i)  {subdiv(fn,i,fn);    return *this;}
    Big& operator/=(const Big& b) {divide(fn,b.fn,fn); return *this;}

    Big& operator%=(int i)  {convert(subdiv(fn,i,fn),fn); return *this;}
    Big& operator%=(const Big& b) {divide(fn,b.fn,b.fn); return *this;}

    Big& operator<<=(int i) {sftbit(fn,i,fn); return *this;}
    Big& operator>>=(int i) {sftbit(fn,-i,fn); return *this;}

    small& operator[](int i) {return fn[i];}

    int get(int index)          {int m; m=getdig(fn,index); return m; }
    void set(int index,int n)   { putdig(n,fn,index);}
    int len()                   { return numdig(fn); }

    friend Big operator-(const Big&);

    friend Big operator+(const Big&,int);
    friend Big operator+(int,const Big&);
    friend Big operator+(const Big&,const Big&);

    friend Big operator-(const Big&, int);
    friend Big operator-(int,const Big&);
    friend Big operator-(const Big&,const Big&);

    friend Big operator*(const Big&, int);
    friend Big operator*(int,const Big&);
    friend Big operator*(const Big&,const Big&);

    friend Big operator/(const Big&,int);
    friend Big operator/(const Big&,const Big&);

    friend int operator%(const Big&, int);
    friend Big operator%(const Big&, const Big&);

    friend Big operator<<(const Big&, int);
    friend Big operator>>(const Big&, int);

    friend bool operator<=(const Big& b1,const Big& b2)
             {if (compare(b1.fn,b2.fn)<=0) return TRUE; else return FALSE;}
    friend bool operator>=(const Big& b1,const Big& b2)
             {if (compare(b1.fn,b2.fn)>=0) return TRUE; else return FALSE;}
    friend bool operator==(const Big& b1,const Big& b2)
             {if (compare(b1.fn,b2.fn)==0) return TRUE; else return FALSE;}
    friend bool operator!=(const Big& b1,const Big& b2)
             {if (compare(b1.fn,b2.fn)!=0) return TRUE; else return FALSE;}
    friend bool operator<(const Big& b1,const Big& b2)
              {if (compare(b1.fn,b2.fn)<0) return TRUE; else return FALSE;}
    friend bool operator>(const Big& b1,const Big& b2)
              {if (compare(b1.fn,b2.fn)>0) return TRUE; else return FALSE;}

    friend Big modmult(const Big&,const Big&,const Big&);
    friend Big norm(const Big&);
    friend Big sqrt(const Big&);
    friend Big root(const Big&,int);
    friend Big gcd(const Big&,const Big&);
    friend Big pow(const Big&,int);
    friend Big pow(const Big&, int, const Big&);
    friend Big pow(int, const Big&, const Big&);
    friend Big pow(const Big&, const Big&, const Big&);
    friend Big inverse(const Big&, const Big&);
    friend Big rand(const Big&);
    friend Big rand(int,small);
    friend int bits(const Big& b) {return logb2(b.fn);}
    friend int jacobi(const Big& b1,const Big& b2) {return jack(b1.fn,b2.fn);}
    friend int toint(const Big& b)  {return size(b.fn);} 
    friend bool prime(const Big& b) {return isprime(b.fn);}  
    friend Big nextprime(const Big&);

/* Note that when inputting text as a number the CR is NOT   *
 * included in the text, unlike C I/O which does include CR. */

    friend istream& operator>>(istream& s, Big& x)
    { 
      if (IOBASE>60) {s.sync(); s.getline((char *)MR_IOBUFF,MR_IOBSIZ);}
      else s >> (char *)MR_IOBUFF;
      if (s.eof() || s.bad()) { zero(x.fn); return s; }
      cinstr(x.fn,(char *)MR_IOBUFF); return s;}

    friend ostream& operator<<(ostream& s, const Big& x)
    { cotstr(x.fn,(char *)MR_IOBUFF); s << (char *)MR_IOBUFF; return s;}

    ~Big() { free((char *)fn); }
};

#endif

