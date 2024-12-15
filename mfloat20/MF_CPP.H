#ifndef mf_cpp_h
#define mf_cpp_h

#include "mfloat.h"
#include <iomanip.h>
#include <ctype.h>
#include <math.h>
#pragma intrinsic -fabs   //avoids inline instr. for fabs: see optimisation
#undef mfloat             //deletes macro "mfloat": see "MFLOAT.H"

class mfloat {
#define fmo friend mfloat operator
#define mfo mfloat operator
#define mro mfloat& operator
#define fbo friend char operator
#define fmf friend mfloat
   public:
     // constructors
     mfloat()                                 { mf[0]=0; }
     mfloat(const char far *a);       // conversions of a string constant
     mfloat(const unsigned int a)             { ldtomf(mf,a); }
     mfloat(const signed int a)               { ldtomf(mf,a); }
     mfloat(const unsigned long a)            { ldtomf(mf,a); }
     mfloat(const signed long a)              { ldtomf(mf,a); }
     mfloat(const mfloat &a)                  { equm(mf,a.mf); }
     // operators and functions
     mro = (const mfloat &a)                  { equm(mf,a.mf); return *this; }
     fmo + (const mfloat &a)                  { return a; }
     mro ++ ()     /*prefix*/                 { addm(mf,onem); return *this; }
     mfo ++ (int)  /*suffix*/                 { mfloat t; equm(t.mf,mf);
						addm(mf,onem); return t; }
     mro -- ()     /*prefix*/                 { subm(mf,onem); return *this; }
     mfo -- (int)  /*suffix*/                 { mfloat t; equm(t.mf,mf);
						subm(mf,onem); return t; }
     fmo - (mfloat a)                         { negm(a.mf); return a; }
     fmo + (mfloat a, const mfloat &b)        { return a+=b; }
     mro += (const mfloat &b)                 { addm(mf,b.mf); return *this; }
     fmo - (mfloat a, const mfloat &b)        { return a-=b; }
     mro -= (const mfloat &b)                 { subm(mf,b.mf); return *this; }
     fmo * (mfloat a, const char far *b)      { return a*=b; }
     fmo * (mfloat a, const unsigned int b)   { return a*=b; }
     fmo * (mfloat a, const signed int b)     { return a*=b; }
     fmo * (mfloat a, const unsigned long b)  { return a*=b; }
     fmo * (mfloat a, const signed long b)    { return a*=b; }
     fmo * (const char far *b, mfloat a)      { return a*=b; }
     fmo * (const unsigned int b, mfloat a)   { return a*=b; }
     fmo * (const signed int b, mfloat a)     { return a*=b; }
     fmo * (const unsigned long b, mfloat a)  { return a*=b; }
     fmo * (const signed long b, mfloat a)    { return a*=b; }
     fmo * (mfloat a, const mfloat &b)        { return a*=b; }
     mro *= (const char far *b)               { return *this*=mfloat(b); }
     mro *= (const unsigned int b)            { if (b<0x8000)
						// fast "multi" can be used
						  return *this*=(int)b;
						else
						  return
						   *this*=mfloat(b); }
     mro *= (const signed int b)              { multi(mf,b); return *this; }
     mro *= (const unsigned long b)           { return
						  *this*=mfloat(b); }
     mro *= (const signed long b)             { return
						  *this*=mfloat(b); }
     mro *= (const mfloat &b)                 { multm(mf,b.mf); return *this;}
     fmo / (mfloat a, const char far *b)      { return a/=b; }
     fmo / (mfloat a, const unsigned int b)   { return a/=b; }
     fmo / (mfloat a, const signed int b)     { return a/=b; }
     fmo / (mfloat a, const unsigned long b)  { return a/=b; }
     fmo / (mfloat a, const signed long b)    { return a/=b; }
     fmo / (const char far *a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (const unsigned int a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (const signed int a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (const unsigned long a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (const signed long a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (mfloat a, const mfloat &b)        { return a/=b; }
     mro /= (const char far *b)               { return *this/=mfloat(b); }
     mro /= (const unsigned int b)            { if (b < 0x8000)
						  // fast "divi" can be used
						  return *this/=(int)b;
						else
						  return
						    *this/=mfloat(b); }
     mro /= (const signed int b)              { divi(mf,b); return *this; }
     mro /= (const unsigned long b)           { return
						 *this/=mfloat(b); }
     mro /= (const signed long b)             { return
						 *this/=mfloat(b); }
     mro /= (const mfloat &b)                 { divm(mf,b.mf); return *this; }
     fbo <  (const mfloat &a, const mfloat &b){ return gtm(b.mf,a.mf); }
     fbo >  (const mfloat &a, const mfloat &b){ return gtm(a.mf,b.mf); }
     fbo <= (const mfloat &a, const mfloat &b){ return !gtm(a.mf,b.mf); }
     fbo >= (const mfloat &a, const mfloat &b){ return !gtm(b.mf,a.mf); }
     fbo == (const mfloat &a, const mfloat &b){ return eqm(a.mf,b.mf); }
     fbo != (const mfloat &a, const mfloat &b){ return !eqm(a.mf,b.mf); }
     friend ostream& operator << (ostream& os, const mfloat &a);
     friend istream& operator >> (istream& is, mfloat &a);
     /* basic functions */
     friend int strtomf(mfloat &a,const char *s) { mfloata bnd;
						   return strtomf_(a.mf,
						    bnd,s,strlen(s)); }
     friend char far * mftoa(char far * str, const mfloat &a, int len);
     friend char far * mftostr(char far * str, const mfloat &a, int len,
			       const char far * format);
     friend double mftod(const mfloat &a)     { return mftod(a.mf); }
     friend long double mftold(const mfloat &a)
					      { return mftold(a.mf); }
     // standard functions (Borland C: MATH.H)
     fmf acos(mfloat a)     	              { acosm(a.mf); return a; }
     fmf asin(mfloat a)   	              { asinm(a.mf); return a; }
     fmf atan(mfloat a)	                      { atanm(a.mf); return a; }
     fmf atan2(mfloat a,const mfloat &b)      { atan2m(a.mf,b.mf);
						return a; }
     fmf atofm(const char *s)	              { mfloat t,bnd;
						strtomf_(t.mf,bnd.mf,
							 s,strlen(s));
						return t; }
     fmf ceil(mfloat a)	                      { ceilm(a.mf); return a; }
     fmf cos(mfloat a)	                      { cosm(a.mf); return a; }
     fmf cosh(mfloat a)	                      { coshm(a.mf); return a; }
     fmf exp(mfloat a)	                      { expm(a.mf); return a; }
     fmf fabs(mfloat a)	                      { fabsm(a.mf); return a; }
     fmf floor(mfloat a)	              { floorm(a.mf); return a; }
     fmf fmod(mfloat a,const mfloat &b)       { fmodm(a.mf,b.mf); return a; }
     fmf frexp(mfloat a,signed int *b)        { frexpm(a.mf,b); return a; }
     fmf hypot(mfloat a,const mfloat &b)      { hypotm(a.mf,b.mf); return a; }
     fmf ldexp(mfloat a,signed int b)         { ldexpm(a.mf,b); return a; }
     fmf log(mfloat a)	                      { logm(a.mf); return a; }
     fmf log10(mfloat a)	              { log10m(a.mf); return a; }
     fmf modf(mfloat a,mfloat *b)             { modfm(a.mf,(*b).mf);
						return a; }
     fmf pow(mfloat a,const char far *b)      { return pow(a,mfloat(b)); }
     fmf pow(mfloat a,const unsigned int b)   { return pow(a,mfloat(b)); }
     fmf pow(mfloat a,const signed int b)     { powi(a.mf,b); return a;  }
     fmf pow(mfloat a,const unsigned long b)  { return pow(a,mfloat(b)); }
     fmf pow(mfloat a,const signed long b)    { return pow(a,mfloat(b)); }
     fmf pow(mfloat a,const mfloat &b)        { powm(a.mf,b.mf); return a; }
     fmf pow(const char far *a,
	     const mfloat &b)                 { return pow(mfloat(a),b); }
     fmf pow(unsigned int a, const mfloat &b) { return pow(mfloat(a),b); }
     fmf pow(signed int a, const mfloat &b)   { return pow(mfloat(a),b); }
     fmf pow(unsigned long a, const mfloat &b){ return pow(mfloat(a),b); }
     fmf pow(signed long a, const mfloat &b)  { return pow(mfloat(a),b); }
     fmf pow10m(signed int b)                 { mfloat a; pow10m(a.mf,b);
						return a; }
     fmf sin(mfloat a)	                      { sinm(a.mf); return a; }
     fmf sinh(mfloat a)	                      { sinhm(a.mf); return a; }
     fmf sqrt(mfloat a)	                      { sqrtm(a.mf); return a; }
     fmf tan(mfloat a)	                      { tanm(a.mf); return a; }
     fmf tanh(mfloat a)	                      { tanhm(a.mf); return a; }
     // extended standard functions
     fmf acosh(mfloat a)	              { acoshm(a.mf); return a; }
     fmf asinh(mfloat a)	              { asinhm(a.mf); return a; }
     fmf atanh(mfloat a)	              { atanhm(a.mf); return a; }
     fmf acot(mfloat a)	                      { acotm(a.mf); return a; }
     fmf acoth(mfloat a)	              { acothm(a.mf); return a; }
     fmf cot(mfloat a)	                      { cotm(a.mf); return a; }
     fmf coth(mfloat a)	                      { cothm(a.mf); return a; }
     fmf exp10(mfloat a)	              { exp10m(a.mf); return a; }
     fmf sqr(mfloat a)	                      { sqrm(a.mf); return a; }
     fmf trunc(mfloat a)	              { truncm(a.mf); return a; }

#ifdef convert_IEEE_float
     // constructors
     mfloat(const float a)                    { ldtomf(mf,a); }
     mfloat(const double a)                   { dtomf(mf,a); }
     mfloat(const long double a)              { ldtomf(mf,a); }
     // operators and functions
     fmo * (mfloat a, const float b)          { return a*=b; }
     fmo * (mfloat a, const double b)         { return a*=b; }
     fmo * (mfloat a, const long double b)    { return a*=b; }
     fmo * (const float b, mfloat a)          { return a*=b; }
     fmo * (const double b, mfloat a)         { return a*=b; }
     fmo * (const long double b, mfloat a)    { return a*=b; }
     mro *= (const float b)                   { return
						  *this*=(long double)b; }
     mro *= (const double b)                  { mfloata t;
						multm(mf,dtomf(t,b));
						return *this; }
     mro *= (const long double b)             { mfloata t;
						multm(mf,ldtomf(t,b));
						return *this; }
     fmo / (mfloat a, const float b)          { return a/=b; }
     fmo / (mfloat a, const double b)         { return a/=b; }
     fmo / (mfloat a, const long double b)    { return a/=b; }
     fmo / (const float a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (const double a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     fmo / (const long double a,
	    const mfloat &b)                  { return mfloat(a)/=b; }
     mro /= (const float b)                   { return
						 *this/=(long double)b; }
     mro /= (const double b)                  { mfloata t;
						divm(mf,dtomf(t,b));
						return *this; }
     mro /= (const long double b)             { mfloata t;
						divm(mf,ldtomf(t,b));
						return *this; }
     fmf pow(mfloat a,const float b)          { return pow(a,mfloat(b)); }
     fmf pow(mfloat a,const double b)         { return pow(a,mfloat(b)); }
     fmf pow(mfloat a,const long double b)    { return pow(a,mfloat(b)); }
     fmf pow(const float &a,
	     const mfloat &b)                 { return pow(mfloat(a),b); }
     fmf pow(const double &a,
	     const mfloat &b)                 { return pow(mfloat(a),b); }
     fmf pow(const long double &a,
	     const mfloat &b)                 { return pow(mfloat(a),b); }
#endif // ifdef convert_IEEE_float
     // data of a mfloat number
     mfloata mf;

#undef fmo
#undef mro
#undef mfo
#undef fbo
#undef fmf
};

extern const mfloat pi; //see MF_CPP.CPP

//------------------------------------------------------------------------
//************************************************************************
//------------------------------------------------------------------------

class mcomplex {
#define fmo friend mcomplex operator
#define fmc friend mcomplex
#define fmf friend mfloat
#define fbo friend char operator
#define mco mcomplex operator
#define mro mcomplex& operator
   public:
     // constructors
     mcomplex()                               { /* do nothing */ }
     mcomplex(const char far *a);      // conversions of a string constant
     mcomplex(const unsigned int a)           { ldtomf(re.mf,a); }
     mcomplex(const signed int a)             { ldtomf(re.mf,a); }
     mcomplex(const unsigned long a)          { ldtomf(re.mf,a); }
     mcomplex(const signed long a)            { ldtomf(re.mf,a); }
     mcomplex(const mfloat &a)                { re=a; }
     mcomplex(const mfloat &a,const mfloat &b){ re=a; im=b; }
     mcomplex(const mcomplex &a)              { re=a.re; im=a.im; }
     // operators and functions
     mro = (const mcomplex &a)                { re=a.re; im=a.im;
						return *this; }
     fmo + (const mcomplex &a)                { return a; }
     mro + ()                                 { return *this; }
     mro ++ ()      /*prefix*/                { ++re; return *this; }
     mco ++ (int)   /*suffix*/                { mcomplex t=*this; ++re;
						return t; }
     mro -- ()      /*prefix*/                { --re; return *this; }
     mco -- (int)   /*suffix*/                { mcomplex t=*this; --re;
						return t; }
     fmo - (mcomplex a)        		      { negm(a.re.mf);
						negm(a.im.mf);
						return a; }
     fmo + (mcomplex a, const mcomplex &b)    { return a+=b; }
     mro += (const mcomplex &b)               { re+=b.re; im+=b.im;
						return *this; }
     fmo - (mcomplex a, const mcomplex &b)    { return a-=b; }
     mro -= (const mcomplex &b)               { re-=b.re; im-=b.im;
						return *this; }
     fmo * (mcomplex a, const char far *b)    { return a*=b; }
     fmo * (mcomplex a, const unsigned int b) { return a*=b; }
     fmo * (mcomplex a, const signed int b)   { return a*=b; }
     fmo * (mcomplex a, const unsigned long b){ return a*=b; }
     fmo * (mcomplex a, const long b)         { return a*=b; }
     fmo * (mcomplex a, const mfloat &b)      { return a*=b; }
     fmo * (const char far *b, mcomplex a)    { return a*=b; }
     fmo * (const unsigned int b, mcomplex a) { return a*=b; }
     fmo * (const signed int b, mcomplex a)   { return a*=b; }
     fmo * (const unsigned long b, mcomplex a){ return a*=b; }
     fmo * (const long b, mcomplex a)         { return a*=b; }
     fmo * (const mfloat &b, mcomplex a)      { return a*=b; }
     fmo * (mcomplex a, const mcomplex &b)    { return a*=b; }
     mro *= (const char far *b)               { return *this*=mcomplex(b); }
     mro *= (const unsigned int b)            { if (b < 0x8000)
						  // fast "multi" can be used
						  return *this*=(int)b;
						else
						  return
						   *this*=mfloat(b); }
     mro *= (const signed int b)              { re*=b;im*=b; return *this; }
     mro *= (const unsigned long b)           { return
						   *this*=mfloat(b); }
     mro *= (const signed long b)             { return
						   *this*=mfloat(b); }
     mro *= (const mfloat &b)                 { re*=b;im*=b; return *this; }
     mro *= (const mcomplex &b);
     fmo / (mcomplex a, const char far *b)    { return a/=b; }
     fmo / (mcomplex a, const unsigned int b) { return a/=b; }
     fmo / (mcomplex a, const signed int b)   { return a/=b; }
     fmo / (mcomplex a, const unsigned long b){ return a/=b; }
     fmo / (mcomplex a, const signed long b)  { return a/=b; }
     fmo / (mcomplex a, const mfloat &b)      { return a/=b; }
     fmo / (const char far *a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const unsigned int a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const signed int a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const unsigned long a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const signed long a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const mfloat &a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (mcomplex a, const mcomplex &b)    { return a/=b; }
     mro /= (const char far *b)               { return *this/=mcomplex(b); }
     mro /= (const unsigned int b)            { if (b < 0x8000)
						  // fast "divi" can be used
						  return *this/=(int)b;
						else
						  return
						    *this/=mfloat(b); }
     mro /= (const signed int b)              { re/=b; im/=b; return *this; }
     mro /= (const unsigned long b)           { return
						 *this/=mfloat(b); }
     mro /= (const signed long b)             { return
						 *this/=mfloat(b); }
     mro /= (const mfloat &b)                 { re/=b;im/=b; return *this; }
     mro /= (mcomplex b);
     fbo == (const mcomplex &a, const mcomplex &b)
					      { return (a.re==b.re) &&
						       (a.im==b.im); }
     fbo != (const mcomplex &a, const mcomplex &b)
					      { return (a.re!=b.re) ||
						       (a.im!=b.im); }
     friend ostream& operator << (ostream& os, const mcomplex &a);
     friend istream& operator >> (istream& is, mcomplex &a);
     // complex manipulations equivalent to COMPLEX.H
     fmf real(const mcomplex &a)              { return a.re; }
     fmf imag(const mcomplex &a)              { return a.im; }
     fmc conj(mcomplex a)                     { negm(a.im.mf); return a; }
     fmf norm(mcomplex a)                     { return
						(a.re*=a.re)+=(a.im*=a.im); }
     fmf arg(const mcomplex &a)               { return atan2(a.im,a.re); }
     // Create a complex object given polar coordinates
     fmc polar(const mfloat &r, const mfloat &phi);
     // Overloaded ANSI C math functions
     fmf abs(const mcomplex &a)               { return hypot(a.re,a.im); }
     fmc acos(const mcomplex &a);
     fmc asin(const mcomplex &a);
     fmc atan(const mcomplex &a);
     fmc cos(const mcomplex &a);
     fmc cosh(const mcomplex &a);
     fmc exp(const mcomplex &a);
     fmc log(const mcomplex &a);
     fmc log10(const mcomplex &a);
     fmc pow(mcomplex a,const char far *b)      { return pow(a,mfloat(b)); }
     fmc pow(mcomplex a,const unsigned int b)   { return pow(a,mfloat(b)); }
     fmc pow(mcomplex a,const signed int b);
     fmc pow(mcomplex a,const unsigned long b)  { return pow(a,mfloat(b)); }
     fmc pow(mcomplex a,const signed long b)    { return pow(a,mfloat(b)); }
     fmc pow(const mcomplex &a, const mfloat &b);
     fmc pow(const mcomplex &a, const mcomplex &b);
     fmc pow(const char far * a,
	     const mcomplex &b)                 { return pow(mcomplex(a),b); }
     fmc pow(unsigned int a, const mcomplex &b) { return pow(mcomplex(a),b); }
     fmc pow(signed int a, const mcomplex &b)   { return pow(mcomplex(a),b); }
     fmc pow(unsigned long a, const mcomplex &b){ return pow(mcomplex(a),b); }
     fmc pow(signed long a, const mcomplex &b)  { return pow(mcomplex(a),b); }
     fmc pow(const mfloat &a, const mcomplex &b);
     fmc sin(const mcomplex &a);
     fmc sinh(const mcomplex &a);
     fmc sqrt(const mcomplex &a);
     fmc tan(const mcomplex &a);
     fmc tanh(const mcomplex &a);
     //extended standard functions
     fmc acosh(const mcomplex &a);
     fmc asinh(const mcomplex &a);
     fmc atanh(const mcomplex &a);
     fmc acot(const mcomplex &a);
     fmc acoth(const mcomplex &a);
     fmc cot(const mcomplex &a);
     fmc coth(const mcomplex &a);
     fmc exp10(const mcomplex &a);
     fmc sqr(const mcomplex &a);

#ifdef convert_IEEE_float
     // constructors
     mcomplex(const float a)                  { ldtomf(re.mf,a); }
     mcomplex(const double a)                 { dtomf(re.mf,a); }
     mcomplex(const long double a)            { ldtomf(re.mf,a); }
     // operators and functions
     fmo * (mcomplex a, const float b)        { return a*=b; }
     fmo * (mcomplex a, const double b)       { return a*=b; }
     fmo * (mcomplex a, const long double b)  { return a*=b; }
     fmo * (const float b, mcomplex a)        { return a*=b; }
     fmo * (const double b, mcomplex a)       { return a*=b; }
     fmo * (const long double b, mcomplex a)  { return a*=b; }
     mro *= (const float b)                   { return
						   *this*=(long double)b; }
     mro *= (const double b)                  { mfloata t;
						multm(re.mf,dtomf(t,b));
						multm(im.mf,t);
						return *this; }
     mro *= (const long double b)             { mfloata t;
						multm(re.mf,ldtomf(t,b));
						multm(im.mf,t);
						return *this; }
     fmo / (mcomplex a, const float b)        { return a/=b; }
     fmo / (mcomplex a, const double b)       { return a/=b; }
     fmo / (mcomplex a, const long double b)  { return a/=b; }
     fmo / (const float a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const double a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     fmo / (const long double a,
	    const mcomplex &b)                { return mcomplex(a)/=b; }
     mro /= (const float b)                   { return
						 *this/=(long double)b; }
     mro /= (const double b)                  { mfloata t;
						divm(re.mf,dtomf(t,b));
						divm(im.mf,t);
						return *this; };
     mro /= (const long double b)             { mfloata t;
						divm(re.mf,ldtomf(t,b));
						divm(im.mf,t);
						return *this; }
     fmc pow(mcomplex a,const float b)          { return pow(a,mfloat(b)); }
     fmc pow(mcomplex a,const double b)         { return pow(a,mfloat(b)); }
     fmc pow(mcomplex a,const long double b)    { return pow(a,mfloat(b)); }
     fmc pow(const float &a, const mcomplex &b) { return pow(mcomplex(a),b); }
     fmc pow(const double &a, const mcomplex &b){ return pow(mcomplex(a),b); }
     fmc pow(const long double &a,
	     const mcomplex &b)                 { return pow(mcomplex(a),b); }
#endif // ifdef convert_IEEE_float
     // data of a mcomplex number
     mfloat re,im;

#undef fmo
#undef fmc
#undef fmf
#undef fbo
#undef mco
#undef mro
};

#endif /* mf_cpp_h */
