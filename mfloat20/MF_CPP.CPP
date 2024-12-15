#define mfloatb            //don't compile functions of mfloat.h
#include "mf_cpp.h"
#undef mfloatb
#include <stdlib.h>

#define maxdigits (int)((mfloatwords*24L)/5+8)

//----------------------------------------------------------------------//
//*********************** Functions of mfloat **************************//
//----------------------------------------------------------------------//

mfloat::mfloat(const char far *a) {
  // conversions of string constants to mfloat
  if (strtomf(mf,a) != 0) mfloatError(29);
}

//----------------------------------------------------------------------//

char far * mftoa(char far * str, const mfloat &a, int len) {

  mfloat t(0);
  mftostr_(str,a.mf,t.mf,&len,".32767F");
  str[len]=0;   //zero terminated string
  return str;
}

//----------------------------------------------------------------------//

char far * mftostr(char far * str, const mfloat &a, int len,
		   const char far * format) {

  mfloat t(0);
  mftostr_(str,a.mf,t.mf,&len,format);
  str[len]=0;   //zero terminated string
  return str;
}

//----------------------------------------------------------------------//

ostream& operator << (ostream& os,const mfloat &a) {

  char ch[maxdigits];
  char format[10];
  int len=maxdigits-1;
  char* f=format;
  int i,j;
  mfloat bound;

  bound.mf[0]=0;       //no error assumed
  // determine format
  if (os.flags() & os.showpos)   *(f++)='+';
  if (os.flags() & os.showpoint) *(f++)='.';
  f+=5;
  i=os.precision();
  if (i<=0) i=6;
  for (j=0; j<5; j++) {
    *(--f)=i%10+'0';
    i/=10;
  }
  f+=5;
  if (os.flags() & os.fixed) *f='f';
  else if (os.flags() & os.scientific) *f='e';
  else *f='g';
  if (os.flags() & os.uppercase) *f-='a'-'A';
  *(++f)=0;                                 // terminate format string
  mftostr_(ch,a.mf,bound.mf,&len,format);   // convert to a string
  ch[len]=0;                                // terminate string
  return os << ch;
}

//------------------------------------------------------------------------

istream& operator >> (istream& is, mfloat &a) {

  int c;
  char ch[maxdigits+1];
  char* chptr=ch;
  char* endptr=ch+maxdigits-1;
  mfloat t;

  c=is.get();
  while (isspace(c)) c=is.get();
  if ((c=='+') || (c=='-')) {
    *(chptr++)=c;
    c=is.get();
  }
  while (isdigit(c) && (chptr != endptr)) {
    *(chptr++)=c;
    c=is.get();
  }
  if ((c == '.') && (chptr != endptr)) {
    *(chptr++)=c;
    c=is.get();
  }
  while (isdigit(c) && (chptr != endptr)) {
    *(chptr++)=c;
    c=is.get();
  }
  if (((c == 'E') || (c == 'e')) && (chptr != endptr)) {
    *(chptr++)=c;
    c=is.get();
    if (((c=='+') || (c=='-')) && (chptr != endptr)) {
      *(chptr++)=c;
      c=is.get();
    }
    while (isdigit(c) && (chptr != endptr)) {
      *(chptr++)=c;
      c=is.get();
    }
  }
  is.putback(c);
  *chptr=0;      //null termination
  if (ch[0]==0) {
    // set error state
    if (c==EOF) is.clear(ios::eofbit | ios::failbit | is.rdstate());
    else is.clear(ios::failbit | is.rdstate());
  }
  else if (strtomf(t.mf,ch)==0) equm(a.mf,t.mf);
  else is.clear(ios::failbit | is.rdstate());
  return is;
}

//------------------------------------------------------------------------

#undef maxdigits

//------------------------------------------------------------------------

char MFLOATB_error() {
  // this function tests, if MFLOATB.C is compiled using 8086 instructions
  char testbyte;

  asm mov ax, seg ceilm;
  asm mov es, ax
  asm mov al, byte ptr es:[ceilm]     //get first byte of ceilm
  asm mov [testbyte], al
  return (testbyte != 0x55);          //push bp
}


//------------------------------------------------------------------------

mfloat initalize_and_get_pi() {

  mfloat t;

  if (MFLOATB_error()) {
    cout << "\n\nMFLOATB.OBJ don't use 8086 instructions only!\n";
    cout << "Please compile MFLOATB.C using 8086 instructions only!\n";
    cout << "Otherwise there is a problem at the stack check.\n\n\n";
    exit(1);
  }
  mfloatTest80386();
  setmantissawords(mfloatwords-1);
  equm(t.mf,pim);
  return t;
}

//------------------------------------------------------------------------

const mfloat pi=initalize_and_get_pi();

//----------------------------------------------------------------------//
//*********************** Functions of mcomplex ************************//
//----------------------------------------------------------------------//

inline void max_expof2(int &a, int b) {

  if ((unsigned int)b > (unsigned int)a) a=b;
}

//------------------------------------------------------------------------

mcomplex::mcomplex(const char far *a) {
  // conversions of string constants to mcomplex
  const char far *strptr=a;
  const char far *strptr1;
  mfloata bnd; // dummy

  //skip blanks
  while ((*strptr != 0) && ((*strptr == ' ') || (*strptr == '\t'))) strptr++;
  if (*strptr == '(') {
    // complex number
    strptr++;        //skip '('
    strptr1=strptr;
    while ((*strptr1 != 0) && (*strptr1 != ',')) strptr1++; //search ','
    if (strtomf_(re.mf,bnd,strptr,(int)(strptr1-strptr)) != 0)
      mfloatError(29);
    if (*strptr1 != 0) strptr1++; // skip ','
    strptr=strptr1;
    while ((*strptr1 != 0) && (*strptr1 != ')')) strptr1++; //search ')'
    if (strtomf_(im.mf,bnd,strptr,(int)(strptr1-strptr)) != 0)
      mfloatError(29);
    if (*strptr1 != ')') mfloatError(29); // ')' is necessary
  }
  // real number
  else if (strtomf(re.mf,a) != 0) mfloatError(29);
}

//------------------------------------------------------------------------

mcomplex& mcomplex::operator *= (const mcomplex &b) {

  mfloata t1,t2;
  subm(multm(equm(t1,b.re.mf),re.mf),multm(equm(t2,b.im.mf),im.mf));
  equm(t2,b.im.mf); // because this and &b could be the same variable
  addm(multm(im.mf,b.re.mf),multm(re.mf,t2));
  equm(re.mf,t1);
  return *this;
}

//------------------------------------------------------------------------

mcomplex& mcomplex::operator /= (mcomplex b) {

  int twoexp;
  mfloata norm,t1,t2;
  twoexp=b.re.mf[0];
  max_expof2(twoexp,b.im.mf[0]);
  if (twoexp != 0) {
    /* scaling */
    twoexp+=0x8000;
    ldexpm(re.mf,-twoexp);
    ldexpm(im.mf,-twoexp);
    ldexpm(b.re.mf,-twoexp);
    ldexpm(b.im.mf,-twoexp);
  }
  addm(multm(equm(norm,b.re.mf),b.re.mf),multm(equm(t2,b.im.mf),b.im.mf));
  divm(addm(multm(equm(t1,re.mf),b.re.mf),
	    multm(equm(t2,im.mf),b.im.mf)),norm);
  divm(subm(multm(im.mf,b.re.mf),multm(re.mf,b.im.mf)),norm);
  equm(re.mf,t1);
  return *this;
}

//------------------------------------------------------------------------

mcomplex polar(const mfloat &r, const mfloat &phi) {

  mcomplex t(phi);
  cossinm(t.re.mf,t.im.mf);
  multm(t.re.mf,r.mf);
  multm(t.im.mf,r.mf);
  return t;
}

//------------------------------------------------------------------------

void timesPi(mcomplex &a) {
// multiplication by +i
  mfloata t;
  equm(t,a.re.mf);
  equm(a.re.mf,negm(a.im.mf));
  equm(a.im.mf,t);
}

//------------------------------------------------------------------------

void timesMi(mcomplex &a) {
// multiplication by -i
  mfloata t;
  equm(t,a.re.mf);
  equm(a.re.mf,a.im.mf);
  equm(a.im.mf,negm(t));
}

//------------------------------------------------------------------------

mcomplex acos(const mcomplex &a) {

  mcomplex t(acosh(a));
  if (gtzero(a.im.mf) || (eqzero(a.im.mf) && gtm(onem,a.re.mf))) {
    timesMi(t);
    return t;
  }
  else {
    timesPi(t);
    return t;
  }
}

//------------------------------------------------------------------------

mcomplex asin(const mcomplex &a) {

  mcomplex t;
  negm(equm(t.re.mf,a.im.mf));
  equm(t.im.mf,a.re.mf);
  t=asinh(t);
  timesMi(t);
  return t;
}

//------------------------------------------------------------------------

mcomplex atan(const mcomplex &a) {

  mcomplex t;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=12; //atan SINULARITY error
  negm(equm(t.re.mf,a.im.mf));
  equm(t.im.mf,a.re.mf);
  t=atanh(t);
  timesMi(t);
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex acot(const mcomplex &a) {

  mcomplex t;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=13; //acot SINULARITY error
  negm(equm(t.re.mf,a.im.mf));
  equm(t.im.mf,a.re.mf);
  t=acoth(t);
  timesPi(t);
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex cos(const mcomplex &a) {

  mfloata sh;
  mcomplex t=a.re;
  sinhm(equm(sh,a.im.mf));
  cossinm(t.re.mf,t.im.mf);
  multm(t.im.mf,negm(sh));
  if ((unsigned int)sh[0] >= 0xC000)
    // there is no number for sqr(sinh(a.im))
    multm(t.re.mf,fabsm(sh));
  else multm(t.re.mf,sqrtm(addm(multm(sh,sh),onem)));
  return t;
}

//------------------------------------------------------------------------

mcomplex cosh(const mcomplex &a) {

  mfloata sh;
  mcomplex t(a.im);
  sinhm(equm(sh,a.re.mf));
  cossinm(t.re.mf,t.im.mf);
  multm(t.im.mf,sh);
  if ((unsigned int)sh[0] >= 0xC000)
    // there is no number for sqr(sinh(a.im))
    multm(t.re.mf,fabsm(sh));
  else multm(t.re.mf,sqrtm(addm(multm(sh,sh),onem)));
  return t;
}

//------------------------------------------------------------------------

mcomplex exp(const mcomplex &a) {

  mfloata ex;
  mcomplex t(a.im);
  expm(equm(ex,a.re.mf));
  cossinm(t.re.mf,t.im.mf);
  multm(t.re.mf,ex);
  multm(t.im.mf,ex);
  return t;
}

//------------------------------------------------------------------------

mcomplex log(const mcomplex &a) {

  mcomplex t;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=5; //log SINULARITY error
  atan2m(equm(t.im.mf,a.im.mf),a.re.mf);
  logm(hypotm(equm(t.re.mf,a.re.mf),a.im.mf));
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex log10(const mcomplex &a) {

  mfloata k;
  mcomplex t;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=7; //log10 SINULARITY error
  divm(atan2m(equm(t.im.mf,a.im.mf),a.re.mf),getln10m(k));
  divm(logm(hypotm(equm(t.re.mf,a.re.mf),a.im.mf)),k);
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex pow(mcomplex a,const signed int b) {
  /* repeated  multiplications */
  mcomplex result;
  unsigned int h;
  char oldint=useInterruptAtError;

  if (b > 0) h=b;
  else {
    h=-b;
    if (oldint == 1) useInterruptAtError=23; //pow: SINGULARITY error
    a=1/a;
    if (oldint == 1) useInterruptAtError=1;
  }
  result=1;
  while (h > 0) {
    if ((h & 1) == 1) result*=a;
    h/=2;
    if (h > 0) a*=a;
  }
  return result;
}

//------------------------------------------------------------------------

mcomplex pow(const mcomplex &a, const mfloat &b) {

  mfloata r;
  mcomplex t;
  char oldint=useInterruptAtError;
  if (eqzero(a.re.mf) && eqzero(a.im.mf) && gtzero(b.mf)) return a;
  if (oldint == 1) useInterruptAtError=23; //pow SINULARITY error
  cossinm(multm(atan2m(equm(t.re.mf,a.im.mf),a.re.mf),b.mf),t.im.mf);
  powm(hypotm(equm(r,a.re.mf),a.im.mf),b.mf);
  multm(t.re.mf,r);
  multm(t.im.mf,r);
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex pow(const mfloat &a, const mcomplex &b) {

  mcomplex t;
  char oldint=useInterruptAtError;
  if (eqzero(a.mf) && gtzero(b.re.mf)) return a;
  if (oldint == 1) useInterruptAtError=23; //pow SINULARITY error
  if (gtzero(a.mf)) t=exp(b*log(a));
  else t=exp(mcomplex(log(-mfloat(a)),pi)*b);
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex pow(const mcomplex &a,const mcomplex &b) {

  mcomplex t;
  if (eqzero(a.re.mf) && eqzero(a.im.mf) && gtzero(b.re.mf)) return a;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=23; //pow SINULARITY error
  t=exp(b*log(a));
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex sin(const mcomplex &a) {

  mfloata sh;
  mcomplex t;
  sinhm(equm(sh,a.im.mf));
  cossinm(equm(t.im.mf,a.re.mf),t.re.mf);
  multm(t.im.mf,sh);
  if ((unsigned int)sh[0] >= 0xC000)
    // there is no number for sqr(sinh(a.im))
    multm(t.re.mf,fabsm(sh));
  else multm(t.re.mf,sqrtm(addm(multm(sh,sh),onem)));
  return t;
}

//------------------------------------------------------------------------

mcomplex sinh(const mcomplex &a) {

  mfloata sh;
  mcomplex t(a.im);
  sinhm(equm(sh,a.re.mf));
  cossinm(t.re.mf,t.im.mf);
  multm(t.re.mf,sh);
  if ((unsigned int)sh[0] >= 0xC000) multm(t.im.mf,fabsm(sh));
			      // there is no number for sqr(sinh(a.im))
  else multm(t.im.mf,sqrtm(addm(multm(sh,sh),onem)));
  return t;
}

//------------------------------------------------------------------------

mcomplex sqrt(const mcomplex &a) {

 if (eqzero(a.re.mf) && eqzero(a.im.mf)) return a;
  mcomplex c;
  mfloata absa;
  hypotm(equm(absa,a.re.mf),a.im.mf);
  if (gtzero(a.re.mf)) {
    sqrtm(ldexpm(addm(equm(c.re.mf,absa),a.re.mf),-1));
    if (!eqzero(c.re.mf)) ldexpm(divm(equm(c.im.mf,a.im.mf),c.re.mf),-1);
  }
  else {
    sqrtm(ldexpm(subm(equm(c.im.mf,absa),a.re.mf),-1));
    if (!gezero(a.im.mf)) negm(c.im.mf);
    if (!eqzero(c.im.mf)) ldexpm(divm(equm(c.re.mf,a.im.mf),c.im.mf),-1);
  }
  return c;
}

//------------------------------------------------------------------------

mcomplex tan(const mcomplex &a) {

  mcomplex n(a.re);
  mcomplex z;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=8; //tan SINULARITY error
  cossinm(n.re.mf,n.im.mf);
  equm(z.re.mf,n.im.mf);
  negm(multm(n.im.mf,tanhm(equm(z.im.mf,a.im.mf))));
  multm(z.im.mf,n.re.mf);
  z/=n;
  if (oldint == 1) useInterruptAtError=1;
  return z;
}

//------------------------------------------------------------------------

mcomplex tanh(const mcomplex &a) {

  mcomplex z(a.im);
  mcomplex n;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=15; //tanh SINULARITY error
  cossinm(z.re.mf,z.im.mf);
  equm(n.re.mf,z.re.mf);
  multm(z.re.mf,tanhm(equm(n.im.mf,a.re.mf)));
  multm(n.im.mf,z.im.mf);
  z/=n;
  if (oldint == 1) useInterruptAtError=1;
  return z;
}

//------------------------------------------------------------------------

mcomplex acosh(const mcomplex &a) {

  mfloata t1,t2,t3;
  mcomplex t;
  if (((unsigned int)a.re.mf[0] >= 0xA000)||
      ((unsigned int)a.im.mf[0] >= 0xA000)) {
    // there is no number for sqr(norm(a))
    asinhm(hypotm(equm(t.re.mf,a.re.mf),a.im.mf));
    atan2m(equm(t.im.mf,a.im.mf),a.re.mf);
    return t;
  }
  subm(multm(equm(t1,a.re.mf),a.re.mf),onem);
  multm(equm(t2,a.im.mf),a.im.mf);
  sqrtm(addm(sqrm(ldexpm(addm(equm(t.re.mf,t1),t2),-1)),t2));
  ldexpm(subm(equm(t3,t2),t1),-1);
  if (gezero(t3)) {
    if (gezero(a.im.mf))
       atan2m(sqrtm(addm(equm(t.im.mf,t.re.mf),t3)),a.re.mf);
    else atan2m(negm(sqrtm(addm(equm(t.im.mf,t.re.mf),t3))),a.re.mf);
  }
  else {
    if (gezero(a.re.mf))
      atan2m(equm(t.im.mf,a.im.mf),sqrtm(negm(subm(t3,t.re.mf))));
    else atan2m(equm(t.im.mf,a.im.mf),negm(sqrtm(negm(subm(t3,t.re.mf)))));
  }
  ldexpm(addm(t1,t2),-1);
  if (gezero(t1)) asinhm(sqrtm(addm(t.re.mf,t1)));
  else {
    sqrtm(negm(subm(t1,t.re.mf)));
    asinhm(divm(fabsm(equm(t.re.mf,a.im.mf)),t1));
  }
  return t;
}

//------------------------------------------------------------------------

mcomplex acoth(const mcomplex &a) {

  mcomplex t;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=21; //acoth SINGULARITY error
  t=atanh(1/a);
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex asinh(const mcomplex &a) {

  mfloata t1,t2,t3;
  mcomplex t;
  if (((unsigned int)a.re.mf[0] >= 0xA000)||
      ((unsigned int)a.im.mf[0] >= 0xA000)) {
    // there is no number for sqr(norm(a))
    asinhm(hypotm(equm(t.re.mf,a.re.mf),a.im.mf));
    if (!gezero(a.re.mf) || (eqzero(a.re.mf) && !gezero(a.im.mf)))
      negm(t.re.mf);
    atan2m(equm(t.im.mf,a.im.mf),fabsm(equm(t1,a.re.mf)));
    return t;
  }
  multm(equm(t1,a.re.mf),a.re.mf);
  subm(multm(equm(t2,a.im.mf),a.im.mf),onem);
  sqrtm(addm(sqrm(ldexpm(addm(equm(t.re.mf,t1),t2),-1)),t1));
  ldexpm(subm(equm(t3,t1),t2),-1);
  if (gezero(t3)) atan2m(equm(t.im.mf,a.im.mf),sqrtm(addm(t3,t.re.mf)));
  else {
    sqrtm(subm(equm(t.im.mf,t.re.mf),t3));
    fabsm(equm(t3,a.re.mf));
    if (!gezero(a.im.mf))
      negm(t.im.mf);
    atan2m(t.im.mf,t3);
  }
  ldexpm(addm(t1,t2),-1);
  if (gezero(t1)) {
    asinhm(sqrtm(addm(t.re.mf,t1)));
    if (!gezero(a.re.mf) || (eqzero(a.re.mf) && !gezero(a.im.mf)))
      negm(t.re.mf);
  }
  else {
    sqrtm(negm(subm(t1,t.re.mf)));
    asinhm(divm(equm(t.re.mf,a.re.mf),t1));
  }
  return t;
}

//------------------------------------------------------------------------

mcomplex atanh(const mcomplex &a) {

  mfloata t1,t2,t3;
  mcomplex t;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=19; //atanh SINULARITY error
  if (((unsigned int)a.re.mf[0] >= 0xA000)||
      ((unsigned int)a.im.mf[0] >= 0xA000)) {
    hypotm(equm(t1,a.re.mf),a.im.mf);
    ldexpm(equm(t.im.mf,pim),-1);
    if (!gezero(a.im.mf) || (eqzero(a.im.mf) && gezero(a.re.mf)))
      negm(t.im.mf);
    hypotm(equm(t1,a.re.mf),a.im.mf);
    atanhm(divm(divm(equm(t.re.mf,a.re.mf),t1),t1));
  }
  else {
    multm(equm(t1,a.re.mf),a.re.mf);
    multm(equm(t2,a.im.mf),a.im.mf);
    ldexpm(subm(addm(equm(t3,t1),t2),onem),-1);
    sqrtm(addm(sqrm(equm(t.re.mf,t3)),t2));
    if (gezero(t3)) {
      addm(equm(t.im.mf,t.re.mf),t3);
      if (!gezero(a.im.mf) || (eqzero(a.im.mf) && gezero(a.re.mf)))
	negm(t.im.mf);
      atan2m(t.im.mf,fabsm(equm(t3,a.im.mf)));
    }
    else
      atan2m(equm(t.im.mf,a.im.mf),negm(subm(t3,t.re.mf)));
    addm(ldexpm(addm(addm(t1,t2),onem),-1),t.re.mf);
    atanhm(divm(equm(t.re.mf,a.re.mf),t1));
  }
  if (oldint == 1) useInterruptAtError=1;
  return t;
}

//------------------------------------------------------------------------

mcomplex cot(const mcomplex &a) {

  mcomplex z(a.re);
  mcomplex n;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=9; //cot SINULARITY error
  cossinm(z.re.mf,z.im.mf);
  equm(n.re.mf,z.im.mf);
  negm(multm(z.im.mf,tanhm(equm(n.im.mf,a.im.mf))));
  multm(n.im.mf,z.re.mf);
  z/=n;
  if (oldint == 1) useInterruptAtError=1;
  return z;
}

//------------------------------------------------------------------------

mcomplex coth(const mcomplex &a) {

  mcomplex n(a.im);
  mcomplex z;
  char oldint=useInterruptAtError;
  if (oldint == 1) useInterruptAtError=16; //coth SINULARITY error
  cossinm(n.re.mf,n.im.mf);
  equm(z.re.mf,n.re.mf);
  multm(n.re.mf,tanhm(equm(z.im.mf,a.re.mf)));
  multm(z.im.mf,n.im.mf);
  z/=n;
  if (oldint == 1) useInterruptAtError=1;
  return z;
}

//------------------------------------------------------------------------

mcomplex exp10(const mcomplex &a) {

  mfloata k;
  mcomplex t(a.im);
  cossinm(multm(t.re.mf,getln10m(k)),t.im.mf);
  multm(t.re.mf,expm(multm(k,a.re.mf)));
  multm(t.im.mf,k);
  return t;
}

//------------------------------------------------------------------------

mcomplex sqr(const mcomplex &a) {

  mcomplex t(a.re,a.im);
  subm(multm(t.re.mf,t.re.mf),multm(t.im.mf,t.im.mf));
  ldexpm(multm(equm(t.im.mf,a.re.mf),a.im.mf),1);
  return t;
}
//------------------------------------------------------------------------

ostream& operator << (ostream& os, const mcomplex &a) {

  int oldw=os.width(1);
  return os << '(' << setw(oldw) << a.re << setw(1) << ", " << setw(oldw)
	    << a.im << setw(1) << ')';
}

//------------------------------------------------------------------------

istream& operator >> (istream& is, mcomplex &a) {

  mcomplex t;
  char c = 0;
  is >> c;
  if (c == '(') {
    is >> t.re >> c;
    if (c == ',') is >> t.im >> c;
    if (c != ')') is.clear(ios::badbit | is.rdstate());
  }
  else {
    is.putback(c);
    is >> t.re;
  }
  if (is) a=t;
  return is;
}
