/*
 *    MIRACL  C++ Header file number.h
 *
 *    AUTHOR  : M. Scott
 *  
 *    PURPOSE : Definition of class ZZn  (Arithmetic mod n)
 *    NOTE    : Must be used in conjunction with big.cpp and number.cpp
 *              The modulus n is always set dynamically (via the modulo() 
 *              routine) - so beware the pitfalls implicit in declaring
 *              static or global ZZn's (which are initialised before n is 
 *              set!). Uninitialised data is OK 
 *                
 *    Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#ifndef NUMBER_H
#define NUMBER_H

#include <big.h>

extern void modulo(const Big&);
extern void reduce(Big &);
extern void restore(Big &);
extern Big *modn,*modnn;    /* modulus & normalised modulus */

class ZZn 
{ 
    Big fn;
public:
    ZZn()       {  } 
    ZZn(int i)  { fn = i; if (i<0 || i>1) reduce (fn); }
    ZZn(long lg){ fn = lg; if (lg<0L || lg>1L) reduce(fn); }
    ZZn(Big& b) { fn=b; reduce(fn); }   /* Big -> ZZn */
    ZZn(ZZn& b) { fn=b.fn; }
    ZZn(char* s){ fn=s; reduce(fn); }

    ZZn& operator=(int i) {fn=i; if (i<0 || i>1) reduce(fn); return *this;}
    ZZn& operator=(long lg){fn=lg; if (lg<0L || lg>1L) reduce(fn); 
                                                             return *this;}
    ZZn& operator=(const ZZn& b){fn=b.fn; return *this;}
    ZZn& operator=(char* s){fn=s;reduce(fn);return *this;}

    ZZn& operator++() {++fn;if (fn==*modnn) fn=0;return *this;}
    ZZn& operator--() {if (fn==0) fn=*modnn;--fn;return *this;}
    ZZN& operator-()  {fn=*modnn-fn; return *this; }
    ZZn& operator+=(int i) {fn+=i;reduce(fn);return *this;}
    ZZn& operator+=(const ZZn& b){fn+=b.fn;fn%=*modnn;return *this;}
    ZZn& operator-=(int i) {fn-=i;reduce(fn); return *this;}
    ZZn& operator-=(const ZZn& b){fn-=b.fn;reduce(fn);return *this;}
    ZZn& operator*=(int i) {fn*=i; reduce(fn); return *this;}
    ZZn& operator*=(const ZZn& b) {fn=modmult(fn,b.fn,*modnn);return *this;}

    operator Big() {return fn;}   /* ZZn -> Big */

    ZZn& operator/=(const ZZn&);

    friend ZZn operator-(const ZZn&);

    friend ZZn operator+(const ZZn&,int);
    friend ZZn operator+(int, const ZZn&);
    friend ZZn operator+(const ZZn&, const ZZn&);

    friend ZZn operator-(const ZZn&, int);
    friend ZZn operator-(int, const ZZn&);
    friend ZZn operator-(const ZZn&, const ZZn&);

    friend ZZn operator*(const ZZn&, int);
    friend ZZn operator*(int, const ZZn&);
    friend ZZn operator*(const ZZn&, const ZZn&);

    friend ZZn operator/(const ZZn&, const ZZn&);

    friend bool operator==(ZZn& b1,ZZn& b2)
    {restore(b1.fn);restore(b2.fn); if (b1.fn==b2.fn) 
                                      return TRUE; else return FALSE;}
    friend bool operator!=(ZZn& b1,ZZn& b2)
    {restore(b1.fn);restore(b2.fn); if (b1.fn!=b2.fn) 
                                      return TRUE; else return FALSE;}

    friend ZZn  pow( const ZZn&,int);
    friend ZZn  pow( int,const ZZn&);
    friend ZZn  pow( const ZZn&, const ZZn&);


    ~ZZn() { }
};

#endif

