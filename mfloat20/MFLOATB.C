/* Compile using 8086 instruction only, otherwise there are problems at   */
/* the implemented stack check.                                           */
/* Standard start code for functions is necessary:                        */
/*   push bp                                                              */
/*   mov bp,sp                                                            */
/*   sub sp, localarea                                                    */
/* No enter instructions should be used, since the carry from instruction */
/* "sub sp, localarea" is need for stack-check.                           */

#pragma option -ml     /* model large    */
#pragma option -N-     /* no stack check */
#ifdef __STDC__
#error don't use ANSI compatibility mode to compile 'mfloatb'
#endif

#define mfloatb
#include "mfloat.h"
#undef mfloatb

#define isdigit(v) 	(((v) >= '0') && ((v) <= '9'))
#define TAB 		('\t')

/* ----------------------------------------------------------------------- */

extern void pascal sinbasis(mfloat_ a);
/* sin basic series for |a| < 1  */

extern void pascal sinhbasis(mfloat_ a);
/* sinh basic series for |a| < 1  */

extern void pascal atanbasis(mfloat_ a);
/* arctan basic series for |a| < 1  */

extern void pascal artanhbasis(mfloat_ a);
/* Artan basic series for |a| < tanh(2)=0.964  */

extern void pascal modulom(mfloat_ a, const mfloat_ modul,
			   int* round, char* overflow);
/* "modulom" is a special function for the sine function and exp-function.*/
/* a <-- a - r*|modul|   |a-r*|modul|| <= 0.5*|modul|, "r" is an integer  */
/* round <-- lowest word of "r"                                           */
/* if ("r" == round) overflow <-- 1  else  overflow <-- 0                 */

extern  int pascal mftoint(const mfloat_ a);
/* "mftoint" is a special function: it returns zero, if "a" isn't an      */
/* integer. If the mfloat number is an integer, but not representable     */
/* by the type "int" the values 0x7FFE or 0x7FFF are returned, depending  */
/* on the argument.                                                       */
/* The return value is odd, if the argument is odd, and even, if the      */
/* argument is even.                                                      */

extern void pascal mfloatStackOverflow(void);
extern void pascal mfloatError(int);

/* ----------------------------------------------------------------------- */

void pascal setmantissawords_(int number) {

  mantissawords=number;
  if (mantissawords > mfloatwords-1) mantissawords=mfloatwords-1;
  if (mantissawords < 1) mantissawords=1;
}

/* ----------------------------------------------------------------------- */

int pascal getmantissawords(void) {

  return mantissawords;
}

/* ----------------------------------------------------------------------- */

char pascal eqzero(const mfloat_ a) {

  return a[0] == 0;
}

/* ----------------------------------------------------------------------- */

char pascal gtzero(const mfloat_ a) {

  return (a[0] != 0) && (a[1] >= 0);
}

/* ----------------------------------------------------------------------- */

char pascal gezero(const mfloat_ a) {

  return (a[1] >= 0) || (a[0] == 0);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal floorm(mfloat_ a) {

  mfloat b;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  equm(b,a);
  truncm(a);
  if ((!gezero(b)) && (!eqm(a,b))) subm(a,onem);
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal ceilm(mfloat_ a) {

  mfloat b;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  equm(b,a);
  truncm(a);
  if (gtzero(b) && (!eqm(a,b))) addm(a,onem);
  return a;
}

/* ----------------------------------------------------------------------- */

void pascal tenexpcalc(const mfloat_ absb, mfloat_ pow10, int * tenexp) {
  /* calculation of exponent of 10 : binary search for  absb >= 0           */
  /* if absb >= 1 : pow10 <= absb < 10*pow10, tenexp=log10(pow10)           */
  /* if absb <  1 : pow10 < 10 / absb <= 10*absb, tenexp= log10(pow10)      */
  /* if biased 2's exponent of absb < 10 then the number is multipl. by 100 */
  /* 1 <= pow10 <= 1E+9863         -9863 <= tenexp <= 9863                  */
  /* if absb = 0  : pow10 = 1   tenexp = 0                                  */

  mfloata h,absb_l;
  unsigned t1;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  equm(pow10,onem);
  *tenexp=0;
  if (absb[0] != 0) {
    if (gtm(onem,absb)) {
      /* exponent of ten is negative */
      equm(h,absb);
      if ((unsigned)absb[0] < 10) multi(h,100);   /* avoids an overflow */
      divm(inttomf(absb_l,10),h);
    }
    else equm(absb_l,absb);
    /* estimate of tenexp */
    if ((unsigned)absb_l[0] > 0x8001) {
      t1=(unsigned)absb_l[0]-0x8001;
      /* multiply t1 by 0.301025391 < log10(2) using no extra word (chop) */
      *tenexp=(t1>>2)+(t1>>5)+(t1>>6)+(t1>>8)+(t1>>12);
    }
    pow10m(pow10,*tenexp);
    while ((*tenexp < 9863) && !gtm(multi(equm(h,pow10),10),absb_l)) {
      (*tenexp)++;
      equm(pow10,h);
    }
    if (gtm(onem,absb)) *tenexp=- *tenexp;
    if ((unsigned)absb[0] < 10)
      /* correct exponent for special case */
      *tenexp-=2;
  }
}

//-----------------------------------------------------------------------

void pascal calcstring(char * a, const mfloat_ b, const mfloat_ bound,
		       int * len, int prec) {
  /*  converts number b (b >= 0) to a string                           */
  /*  bound = bound for accuracy (for range arithmetic)                */
  /*  All seen digits but last digit are correct.                      */
  /*  prec = number of digits behind the point                         */
  /*  len = max. number of characters ( len >= 2)                      */
  /*  len return : actual number of characters                         */
  /*  If there is definded a range all seen digits but last digit are  */
  /*  correct. The last can differ by one.                             */

  mfloata trb,h1,h2,frac,bnd;
  char *strptr=a,*ptr1,*ptr2;
  char temp,roundup=0;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  ldexpm(equm(bnd,bound),1);              /* multiply by two */
  subm(equm(frac,b),truncm(equm(trb,b)));
  /* calculate digits in front of the point */
  if (trb[0] == 0) {
    /* the absolute value of the number is less than one */
    *(strptr++)='0';
    *(strptr++)='.';
  }
  else {
    /* calculate digits in reversed order */
    while ((trb[0] != 0) && ((strptr-a) < *len)) {
      equm(h1,trb);
      equm(h2,divi(trb,10));
      truncm(trb);
      if (eqm(trb,h2)) temp='0';
      else temp=(char)('0'+mftoint(subm(h1,multi(equm(h2,trb),10))));
      if (gtm(bnd,onem)) {
	*(strptr++)='*';     /* digit is not accurate            */
	roundup=temp;        /* store last, not displayed digit  */
      }
      else *(strptr++)=temp;
      divi(bnd,10);
    }
    if (trb[0] != 0) *a='!';
      /* there was an error, the space of "a" is too little */
    else {
      /* change order of the digits in front of the point */
      ptr1=a;
      ptr2=strptr;
      ptr2--;
      while (ptr2 > ptr1) {
	/* change order */
	temp= *ptr2;
	*ptr2= *ptr1;
	*ptr1=temp;
	ptr2--;
	ptr1++;
      }
    }
    /* add point */
    if ((strptr-a) < *len) *(strptr++)='.';
    else *a='!';                  /* there was an error */
  }
  /* calculate digits behind the point */
  ldexpm(equm(bnd,bound),1);            /* multiply by two */
  while ((prec > 0) && ((strptr-a) < *len)) {
    prec--;
    multi(frac,10);
    multi(bnd,10);
    temp=(char)('0'+mftoint(truncm(equm(h1,frac))));
    if (gtm(bnd,onem)) {
      *(strptr++)='*';                /* digit is not accurate           */
      if (roundup == 0) roundup=temp; /* store last, not displayed digit */
    }
    else *(strptr++)=temp;
    subm(frac,h1);
  }
  if (roundup == 0) roundup=(char)('0'+mftoint(truncm(multi(frac,10))));
  if (roundup >= '5') {
    /* round up: ASCII addition of one */
    ptr1=strptr;
    do {
      ptr1--;
      if ((*ptr1 >= '0') && (*ptr1 < '9')) (*ptr1)++;
      else if (*ptr1 == '9') *ptr1='0';
    } while ((ptr1 > a) && (*ptr1 == '0' || *ptr1 == '.' || *ptr1 == '*'));
    if (*ptr1 == '0') {
      /* there was an overflow by rounding */
      if (((strptr-a) >= *len) && (*(strptr-1) == '.')){
	/* the point would be lost -> error: shown by "!" */
	*a='!';
      }
      else {
	if ((strptr-a) < *len) strptr++;
	ptr1=strptr-1;
	/* shift of digits */
	while (ptr1 >= a) {
	  *(ptr1+1)= *ptr1;
	  ptr1--;
	}
	*a='1';
      }
    }
  }
  /* return actual number of digits */
  *len=(int)(strptr-a);
}

/* ----------------------------------------------------------------------- */

char * pascal mftostr_(char* a, const mfloat_ b, const mfloat_ bound,
		       int* len, const char* format) {
  /* converts a mfloat number to a string                                  */
  /* b: mfloat number to convert                                           */
  /* bound: bound for accuracy (for range arithmetic), only 1 mantissaword */
  /* a: string for result                                                  */
  /* format: format string: see help of C-library                          */
  /* len: max. number of characters ( len >= 2)                            */
  /* len return : actual number of characters                              */
  /* return value: pointer to string                                       */

  char showpos=0,showpoint=0,expchar='E';
  char type='G',fixed,neg;
  char* strptr=a;
  const char* fp;
  int prec=6,tenexp,est_word;
  int oldmantissawords,oldlen,i;
  mfloata absb,pow10,bnd;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  /* decode format string */
  for (fp=format; *fp != NULL; fp++) {
    switch (*fp) {
      case ' ': showpos=' ';  break;
      case '+': showpos=(b[0] == 0) ? ' ' : '+';  break;
      case '.': showpoint=1;  break;
      case 'E':
      case 'F':
      case 'G': type= *fp; expchar='E'; break;
      case 'e':
      case 'f':
      case 'g': type= *fp-'a'+'A'; expchar='e'; break;
      default:  if ((*fp >= '0') && (*fp <= '9')) {
	prec=0;
	for (; (*fp >= '0') && (*fp <= '9'); fp++)
	  prec=prec*10+ *fp-'0';
	fp--;
      } /* default */
    } /* switch */
  } /* for */
  bnd[0]=bound[0];          /* exponent of bound */
  bnd[1]=bound[1] & 0x7FFF; /* first mantissa-word: delete sign bit */
  for (i=2; i < mfloatwords; i++) bnd[i]=0;
  neg= !gezero(b);
  fabsm(equm(absb,b));
  oldmantissawords=mantissawords;
  /* estimate neccesary precision */
  if (prec > 6552) est_word=mfloatwords-1; /* avoid overflow */
  else est_word=(prec*5)/24+3;             /* each word: 4.8 digits */
  /* the accuracy is about 9-10 decimals higher than least sign. bit */
  if (((type == 'F') || (type == 'G')) && ((unsigned)b[0] > 0x8000))
    /* further precision for fix-point format each 16 bits=1 mantissa-word */
    est_word+=(((unsigned)b[0]-0x8000)>>4)+1;
  setmantissawords(est_word); /* increase accuracy of the calculation */
  /* delete new mantissa-words */
  for (i=oldmantissawords+1; i <= mantissawords; i++) absb[i]=0;
  tenexpcalc(absb,pow10,&tenexp);
  /* check format type */
  fixed=(type == 'F') || ((type == 'G') && ((b[0]==0) || (tenexp >= -4)));
  oldlen= *len;
  /* handle sign */
  (*len)--;
  if (neg) *(strptr++)='-';
  else {
    if (showpos != 0) *(strptr++)=showpos;
    else  (*len)++;
  }
  /* check, if there is a chance for fixpoint format */
  if ((fixed) && (((*len)-prec-1) <= tenexp)) fixed=0;  /* place for point */
  if (fixed) {
    /* fixpoint conversion */
    if (*len < 2) {
      /* there isn't enough space */
      for (strptr=a; (strptr-a) < oldlen; *(strptr++)='*');
    }
    else {
      i= *len;                                   /* store len */
      calcstring(strptr,absb,bnd,len,prec);
      if ((*strptr == '!') || (*(strptr+ *len-prec-1) != '.')) {
	/* the conversion is incorrect, or there are not enough  */
	/* digits behind the point                               */
	fixed=0;
	*len=i;                                 /* restore len */
      }
      else {
	strptr+= *len;       /* place pointer to free character */
	/* handle showpoint */
	if (!showpoint) {
	  while (*(--strptr) == '0'); /* remove not important zeros */
	  if (*strptr != '.') strptr++;
	}
      }
    }
  }
  if (!fixed) {
    /* scientific representation of the number (with exponent of ten) */
    if ((tenexp < 100) && (tenexp > -100)) (*len)-=4;
    else if ((tenexp < 1000) && (tenexp > -1000)) (*len)-=5;
    else (*len)-=6;
    if ((unsigned)absb[0] < 10) {
      multi(absb,100); /* special case */
      if (absb[0] != 0) multi(bnd,100);
    }
    if (tenexp >= 0) {
      divm(absb,pow10);
      divm(bnd,pow10);
    }
    else {
      multm(absb,pow10);
      multm(bnd,pow10);
    }
    if (*len < 2) {
      /* there isn't enough space */
      for (strptr=a; (strptr-a) < oldlen; *(strptr++)='*');
      *len=oldlen;
      mantissawords=oldmantissawords;
      return a;
    }
    else {
      i= *len;    /* store len */
      calcstring(strptr,absb,bnd,len,prec);
      if (*(strptr+1) != '.') {
	/* there was a rounding overflow */
	*len=i;   /* restore len */
	tenexp++;
	if ((tenexp == -999) || (tenexp == -99)) (*len)++;
	if ((tenexp == 1000) || (tenexp == 100)) (*len)--;
	divi(absb,10);
	divi(bnd,10);
	calcstring(strptr,absb,bnd,len,prec);
      }
    }
    strptr+= *len; /* place pointer to next free character */
    /* handle showpoint */
    if (!showpoint) {
      while (*(--strptr) == '0'); /* remove not important zeros */
      if (*strptr != '.') strptr++;
    }
    /* add exponent */
    *(strptr++)=expchar;
    if (tenexp >= 0) *(strptr++)='+';
    else {
      *(strptr++)='-';
      tenexp=-tenexp;
    }
    if (tenexp >= 100) {
      if (tenexp >= 1000) {
	*(strptr++)='0'+(tenexp/1000);
	tenexp%=1000;
      }
      *(strptr++)='0'+(tenexp/100);
      tenexp%=100;
    }
    *(strptr++)='0'+(tenexp/10);
    *(strptr++)='0'+(tenexp%10);
  }
  *len=(int)(strptr-a);
  mantissawords=oldmantissawords;
  return a;
}

/* ----------------------------------------------------------------------- */

int pascal strtomf_(mfloat_ a, mfloat_ bound, const char * b, int len) {
  /*  converts a string to a mfloat number                             */
  /*  bound = maximal error of the conversion : only one mantissa-word */
  /*  return value: element of the string at first error               */
  /*  if the conversion is correct, return value = 0                   */

  int BehindPoint=0,InfrontofPoint=0,Exponent=0,Errsum=0;
  unsigned expbnd;
  char NegExp=0,NegMant=0,PointFound=0,DigitFound=0;
  mfloat temp,aloc;
  const char * ptr;
  const char * pend;
  int oldmantissawords;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  ptr=b;
  pend=b+len;
  aloc[0]=0;
  /* ignore leading blanks and tabs */
  while ((ptr != pend) && ((*ptr == ' ') || (*ptr == TAB))) ptr++;
  /* handle sign */
  if ((ptr != pend) && !isdigit(*ptr) && (*ptr != '.'))
    if (*ptr == '+') ptr++;
    else if (*ptr == '-') { ptr++; NegMant=1; }
    else return((int) (ptr-b+1));
  oldmantissawords=mantissawords;
  /* increase accuracy */
  /* 9-10 decimals more accuracy than specified */
  setmantissawords(mantissawords+2);
  /* read mantissa */
  if (mantissawords >= 0x0800) expbnd=0xFFFF; /* avoid an overflow */
  else expbnd=0x8000+16*mantissawords;
  while ((ptr != pend) && (isdigit(*ptr) || (*ptr == '.'))) {
    if (*ptr == '.')
      if (PointFound) {
	/* error: there are two or more points */
	mantissawords=oldmantissawords;
	return((int)(ptr-b+1));
      }
      else PointFound=1;
    else {
      DigitFound=1;
      multi(aloc,10);
      /* truncation errors occure, if the integer isn't representable  */
      /* by a mfloat number exactly                                    */
      if ((unsigned)aloc[0] > expbnd) Errsum++;
      addm(aloc,inttomf(temp,*ptr-'0'));
      /* truncation errors occure, if the integer isn't representable  */
      /* by a mfloat number exactly                                    */
      if ((unsigned)aloc[0] > expbnd) Errsum++;
      if (PointFound) BehindPoint++;
      else if (aloc[0] != 0) InfrontofPoint++;
    }
    ptr++;
  }
  if (DigitFound == 0) {
    /* There was no digit in front of the exponent character */
    mantissawords=oldmantissawords;
    return((int)(ptr-b+1));
  }
  /* read exponent */
  if ((ptr != pend) && ((*ptr == 'E') || (*ptr == 'e'))) {
    ptr++;    /* skip character E or e */
    if ((ptr != pend) && !isdigit(*ptr))
      if      (*ptr == '+')   ptr++;
      else if (*ptr == '-') { ptr++; NegExp=1; }
      else {
	/* There is no digit or + or - right to e or E */
	mantissawords=oldmantissawords;
	return((int)(ptr-b+1));
      }
    while ((ptr != pend) && isdigit(*ptr) && (Exponent < 1000)) {
      /* read digits of the exponent */
      Exponent=Exponent*10+(*ptr)-'0';
      ptr++;
    }
    if (NegExp) Exponent=-Exponent;
  }
  /* read rest of the string */
  while (ptr != pend) {
    if ((*ptr != ' ') && (*ptr != TAB)) {
      /* There are other characters than blancs and tabs */
      mantissawords=oldmantissawords;
      return((int)(ptr-b+1));
    }
    ptr++;
  }
  if ((Exponent+InfrontofPoint > 9863) ||
      (Exponent+InfrontofPoint < -9863)) {
    mantissawords=oldmantissawords;
    return((int)(ptr-b));
  }
  if (Exponent < -0x1000) {
    divm(aloc,pow10m(temp,0x1000));
    /* The error of pow10m(4096) is maximaly 255 LSB */
    Errsum+=256;
    Exponent+=0x1000;
  }
  if (Exponent-BehindPoint >= 0)
    multm(aloc,pow10m(temp,Exponent-BehindPoint));
  else divm(aloc,pow10m(temp,BehindPoint-Exponent));
  Errsum++; /* 1 LSB error due to the multm or divm operation */
  /* estimate errors of pow10m */
  Exponent-=BehindPoint;
  if (Exponent < 0) Exponent=-Exponent;
  Errsum+=(Exponent>>4)+1;            /* due to "pow10m" : |exponent|/16 */
  if (NegMant) negm(aloc);
  bound[0]=aloc[0];
  bound[1]=aloc[1] & 0x7FFF;             /* delete sign bit            */
  if (mantissawords == oldmantissawords) {
    mantissawords=1;
    multi(bound,Errsum);  /* only one mantissa-word is manipulated     */
  }
  else {
    /* else the error due to conversion is neglectable, the error due  */
    /* to the truncation, which results at reduction of the accuracy,  */
    /* is dominant. The conversion error is only some percent of the   */
    /* rest. For 5000 decimal digits there is a maximal error of 16.3% */
    /* due to conversion, the rest is due to the truncation.           */
    mantissawords=1;
    multi(divi(bound,6),7);                  /*  16.6% further error  */
  }
  mantissawords=oldmantissawords;
  ldexpm(bound,1-16*mantissawords);   /* only exponent is manipulated  */
  if ((bound[0] == 0) && (aloc[0] != 0)) {
    /* there was an underflow: bound <- least positive number          */
    bound[0]=1;
    bound[1]=0;
  }
  equm(a,aloc);   /* since everything is OK: copy "aloc" to "a"        */
  return 0;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal sqrm(mfloat_ a) {
  /* sqare:  a <- a*a */
  return multm(a,a);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal inversm(mfloat_ a) {
  /* inverse number: a <- 1/a */
  mfloat temp;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  return equm(a,divm(equm(temp,onem),a));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal atanm(mfloat_ a) {
  /* arctan function: a <- arctan(a) */
  int reduction;
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] >= 0xC000) {
      /*   there is no mfloat number for sqr(a)   */
    if (gtzero(a)) equm(a,pim);
    else negm(equm(a,pim));
    a[0]--;   /*   a <- a / 2   */
  }
  else {
    reduction=0;
    while ((unsigned)a[0] > 0x8000-4) {
	/*   |a| >= 0.0625 = 2**(-4)  */
      reduction++;
      divm(a,addm(sqrtm(addm(sqrm(equm(help,a)),onem)),onem));
    }
    atanbasis(a);
    if (a[0] != 0) a[0]+=reduction;   /*   a <- a * (2 ** reduction)   */
  }
  return a;
}

/* ----------------------------------------------------------------------- */

static void pascal artanhbasis1(mfloat_ a) {
  /* faster Artanh function, than "artanhbasis" for larger arguments */
  /* a <- Artanh(a)                                                  */
  int reduction,n;
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] > 0x8000-50) reduction=5;
      /*   |a| >= 8.881784e-16 = 2**(-50)   */
  else reduction=0;
  for (n=1; n <= reduction; n++)
    divm(a,addm(sqrtm(negm(subm(sqrm(equm(help,a)),onem))),onem));
  artanhbasis(a);
  if (a[0] != 0) a[0]+=reduction;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal sinm(mfloat_ a) {
  /*   a <- sin(a)   */
  int quot;
  char overflow;   /*   here "overflow" is only dummy   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  equm(help,pim);
  help[0]--;   /*   help <- pi / 2   */
  modulom(a,help,&quot,&overflow);
  if ((unsigned)a[0] <= 1) {
      /*   "a" is near an underflow   */
    if ((quot & 1) != 0) equm(a,onem); /* "quot" is odd */
      /*   for even "quot": "a" is not changed   */
  }
  else {
    a[0]--;                   /*  a <- a / 2  */
    sinbasis(a);
    if ((quot & 1) != 0) {
      /* "quot" is odd */
      sqrm(a);
      if (a[0] != 0) a[0]++;  /*  2 * sqr(a), (a[0] = 0 <-> a = 0) */
      negm(subm(a,onem));
    }
    else {
      sqrtm(negm(subm(sqrm(equm(help,a)),onem)));
      help[0]++;
      multm(a,help);
    }
  }
  if ((quot & 2) != 0) negm(a);
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal cosm(mfloat_ a) {
  /*   a <- cos(a)   */
  int quot;
  char overflow;   /*   here "overflow" is only dummy   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  equm(help,pim);
  help[0]--;          /*   help <- pi / 2   */
  modulom(a,help,&quot,&overflow);
  if ((unsigned)a[0] <= 1) {
      /*   "a" is near an underflow   */
    if ((quot & 1) == 0) equm(a,onem); /* "quot" is even */
      /*   for odd "quot": a is not changed   */
  }
  else {
    a[0]--;          /*   a <- a / 2         */
    sinbasis(a);
    if ((quot & 1) != 0) {
      /* "quot" is odd */
      sqrtm(negm(subm(sqrm(equm(help,a)),onem)));
      help[0]++;
      multm(a,help);
    }
    else {
      sqrm(a);
      if (a[0] != 0)   /*   a[1] = 0 <-> a = 0   */
	a[0]++;        /*   2 * sqr(a)           */
      negm(subm(a,onem));
    }
  }
  if (((quot + 1) & 2) != 0) negm(a);
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal cossinm(mfloat_ a, mfloat_ b) {
  /*   a <- cos(a)     b <- sin(a)     */
  /*   This calculation is faster than two separate calculations.     */
  /*   stack check isn't necessary                                    */
  int quot;
  char overflow;   /*   here "overflow" is only dummy   */

  equm(b,pim);
  b[0]--;          /*   b <- pi / 2   */
  modulom(a,b,&quot,&overflow);
  if ((unsigned)a[0] <= 1) {
      /*   "a" is near an underflow   */
    if ((quot & 1) != 0) equm(b,onem); /* "quot" is odd */
    else {
      /* "quot" is even */
      equm(b,a);
      equm(a,onem);
    }
  }
  else {
    a[0]--;
    sinbasis(a);
    if ((quot & 1) != 0) {
      /* "quot" is odd */
      equm(b,a);
      sqrtm(negm(subm(sqrm(a),onem)));
      a[0]++;
      multm(a,b);
      sqrm(b);
      if (b[0] != 0) b[0]++;
      negm(subm(b,onem));
    }
    else {
      /* "quot" is even */
      sqrtm(negm(subm(sqrm(equm(b,a)),onem)));
      b[0]++;
      multm(b,a);
      sqrm(a);
      if (a[0] != 0) a[0]++;
      negm(subm(a,onem));
    }
  }
  if ((quot & 2) != 0) negm(b);
  if (((quot + 1) & 2) != 0) negm(a);
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal tanm(mfloat_ a) {
  /*   a <- tan(a)   */
  mfloat si;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  cossinm(a,si);
  if (a[0] == 0) mfloatError(8); /* tan: SINGULARITY error */
  return equm(a,divm(si,a));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal cotm(mfloat_ a) {
  /*   a <- cot(a)   */
  mfloat si;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  cossinm(a,si);
  if (si[0] == 0) mfloatError(9); /* cot: SINGULARITY error */
  return divm(a,si);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal asinm(mfloat_ a) {
  /*   a <- arcsin(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  negm(subm(sqrm(equm(help,a)),onem));
  if (!gezero(help)) mfloatError(10); /* asin: DOMAIN error */
  if (help[0] == 0) {
    /*   a = +- 1   */
    if (gtzero(a)) {
      equm(a,pim);
      a[0]--;   /*   a <- pi / 2   */
    }
    else {
      negm(equm(a,pim));
      a[0]--;   /*   a <- -pi / 2   */
    }
    return a;
  }
  else return atanm(divm(a,sqrtm(help)));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal acosm(mfloat_ a) {
  /*   a <- arccos(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] <= 0x4000) {
    equm(a,pim);
    a[0]--;        /*   a <- pi / 2   */
  }
  else {
    equm(help,a);
    negm(subm(sqrm(a),onem));
    if (!gezero(a)) mfloatError(11); /* acos: DOMAIN error */
    atanm(divm(sqrtm(a),help));
    if (!gtzero(help)) addm(a,pim); /* a < 0  */
  }
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal acotm(mfloat_ a) {
  /*   a <- arccot(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if (gtm(a,onem)) {
    return atanm(inversm(a));
  }
  else {
    equm(help,pim);
    help[0]--;      /*   help <- pi / 2   */
    return addm(negm(atanm(a)),help);
  }
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal expm(mfloat_ a) {
  /*   a <- exp(a)   */
  int quot;
  char overflow,positive;
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  positive=gtzero(a);
  modulom(a,ln2m,&quot,&overflow);
  if (overflow || ((quot == 0x7FFF) && gezero(a))) {
    if (positive) mfloatError(2); /* overflow error */
    else a[0]=0; /* underflow */
  }
  else {
    sinhbasis(a);
    addm(a,sqrtm(addm(sqrm(equm(help,a)),onem)));
    a[0]+=quot;
  }
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal sinhm(mfloat_ a) {
  /*   a <- sinh(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] <= 0x8000-2) sinhbasis(a);  /* |a| < 0.25 */
  else {
    expm(a);
    subm(a,divm(equm(help,onem),a));
    a[0]--;
  }
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal coshm(mfloat_ a) {
  /*   a <- cosh(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  expm(a);
  addm(a,divm(equm(help,onem),a));
  a[0]--;
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal tanhm(mfloat_ a) {
  /*   a <- tanh(a)   */
  mfloat help,maxz;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  fabsm(equm(help,a));
  inttomf(maxz,11350);
  if (gtm(help,maxz)) {
    if (gtzero(a)) equm(a,onem);
    else negm(equm(a,onem));
  }
  else {
    sinhm(a);
    divm(a,sqrtm(addm(sqrm(equm(help,a)),onem)));
  }
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal cothm(mfloat_ a) {
  /*   a <- coth(a)   */
  if (eqzero(tanhm(a))) mfloatError(16); /* coth: SINGULARITY error */
  return inversm(a);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal logm(mfloat_ a) {
  /*   a <- ln(a)   */
  int exponent;
  mfloat help,limitz;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if (a[0] == 0) mfloatError(5);  /* log: SINGULARITY error  */
  if (!gtzero(a)) mfloatError(4); /* log: DOMAIN error       */
  exponent = a[0]+0x8000;
  a[0]=0x8000;
  ldexpm(inttomf(limitz,23170),-15); /* limit = 0.7070922 */
  if (gtm(limitz,a)) {
    a[0]++;
    exponent--;
  }
  addm(equm(help,onem),a);
  artanhbasis1(divm(subm(a,onem),help));
  if (a[0] != 0) a[0]++;
  return addm(a,multi(equm(help,ln2m),exponent));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal asinhm(mfloat_ a) {
  /*   a <- Arsinh(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] <= 0x8000 - 2) {
      /*   for little arguments: |a| <= 0.25 = 2**(-2)   */
    artanhbasis1(divm(a,sqrtm(addm(sqrm(equm(help,a)),onem))));
    return a;
  }
  else {
    if ((unsigned)a[0] >= 0xC000) {
	/*   There is no mfloat number for sqr(help)   */
      if (gtzero(a)) return addm(logm(a),ln2m);
      else return negm(addm(logm(negm(a)),ln2m));
    }
    else {
      if (gtzero(a))
	return logm(addm(a,sqrtm(addm(sqrm(equm(help,a)),onem))));
      else
	return negm(logm(addm(negm(a),sqrtm(addm(sqrm(equm(help,a)),onem)))));
    }
  }
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal acoshm(mfloat_ a) {
  /*   a <- Arcosh(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if (gtm(onem,a)) mfloatError(17); /* acosh: DOMAIN error */
  if ((unsigned)a[0] >= 0xC000) return addm(logm(a),ln2m);
      /*   There is no mfloat number for sqr(help)   */
  else return logm(addm(a,sqrtm(subm(sqrm(equm(help,a)),onem))));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal atanhm(mfloat_ a) {
  /*   a <- Artanh(a)   */
  mfloat help;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] <= 0x8000 - 2)
      /*   for little arguments: |a| < 0.25 = 2**(-2)   */
    artanhbasis1(a);
  else {
    subm(equm(help,onem),a);
    addm(a,onem);
    if (gtzero(help) && gtzero(a)) logm(divm(a,help));
    else {
      if ((help[0] == 0) || (a[0] == 0))
	mfloatError(19);    /* atanh: SING.error */
      else mfloatError(18); /* atanh: DOMAIN error */
    }
    a[0]--;
  }
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal acothm(mfloat_ a) {
  /*   a <- Arcoth(a)   */

  if ((unsigned)a[0] <= 0x8000)
    mfloatError(20); /* acoth: DOMAIN error */
  if (eqm(a,onem) || eqm(negm(a),onem))
    mfloatError(21); /*acoth: SING error */
  return atanhm(inversm(negm(a)));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal pow10m(mfloat_ a, int b) {
  /* power of ten: a <- 10^b */
  int negative=0,LastDiv=0;
  mfloat h,pow10;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if (b > 9863) mfloatError(2); /* overflow error */
  equm(a,onem);
  if (b < 0) {
    if (b < -9864) {
      /* underflow */
      b=0;
      a[0]=0;
    }
    else {
      negative=1;
      b=-b;
      if (b == 9864) {
	LastDiv=1;
	b--;
      }
    }
  }
  inttomf(pow10,10);
  while (b > 0) {
    if ((b & 1) == 1) multm(a,pow10);
    b>>=1;
    if (b > 0) multm(pow10,pow10);
  }
  if (negative) {
    equm(a,divm(equm(h,onem),a));
    if (LastDiv) return divi(a,10);
    else return a;
  }
  else return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal exp10m(mfloat_ a) {
  /*   a <- 10^a   */
  int aint;

  aint=mftoint(a);
  if ((a[0] == 0) || ((aint != 0) && (aint < 10000) && (aint > -10000)))
    return pow10m(a,aint);
  else return expm(multm(a,ln10m));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal log10m(mfloat_ a) {
  /*   a <- log(a)    */
  if (a[0] == 0) mfloatError(7);  /* log10 SINGULARITY error */
  if (!gtzero(a)) mfloatError(6); /* log10 DOMAIN error      */
  return divm(logm(a),ln10m);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal atan2m(      mfloat_ a,
		      const mfloat_ b) {
  /* a <- atan2(a,b)  (arctan(a/b)) */
  mfloat h;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((a[0] == 0) && (b[0] == 0)) mfloatError(14); /* atan2 DOMAIN error */
  if ((unsigned)b[0] >= (unsigned)a[0]) {
    /* 2 * |b| >= |a|  */
    if (!gtzero(b)) {
      equm(h,pim);
      if (!gezero(a)) negm(h);
    }
    else h[0]=0;
    return addm(atanm(divm(a,b)),h);
  }
  else {
    /* 2 * |a| >= |b| */
    negm(atanm(divm(equm(h,b),a)));
    if (gtzero(a)) equm(a,pim);
    else negm(equm(a,pim));
    a[0]--;
    return addm(a,h);
  }
}

/* ----------------------------------------------------------------------- */

mfloat_  pascal hypotm(mfloat_ a, const mfloat_ b) {
  /*   a <- sqrt(a*a + b*b)    */
  mfloat h;
  int twoexp;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((unsigned)a[0] > (unsigned)b[0]) twoexp=a[0];
  else twoexp=b[0];
  equm(h,b);
  if (twoexp != 0) {
    /* a != 0  or  b != 0 */
    twoexp+=0x8000;
    ldexpm(a,-twoexp);
    ldexpm(h,-twoexp);
  }
  return ldexpm(sqrtm(addm(sqrm(a),sqrm(h))),twoexp);
}

/* ----------------------------------------------------------------------- */

mfloat_  pascal fmodm(mfloat_ a, const mfloat_ b) {
  /*  a <-  a - b*int(a/b) */
  /*  if the number a/b can not be represented by a mfloat exactly, */
  /*  zero is returned                                              */
  mfloat h;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  divm(equm(h,a),b);
  if ((unsigned)h[0] >= (unsigned)(0x8000+16*mantissawords)) {
    a[0]=0;
    return a;
  }
  else return subm(a,multm(floorm(h),b));
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal frexpm(mfloat_ a, int * b) {
 /*  a <- a * 2**(-b)  if a=0:b=0 else 0.5 <= abs(a) < 1 */
 if (a[0] == 0) *b=0;
 else {
   *b=a[0]-0x8000;
   a[0]=0x8000;
 }
 return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal modfm(mfloat_ a, mfloat_ b) {
  /* the number a is divided in the integer part and the fractal part  */
  /* the integer part is returned by "b", the fractal part is returned */
  /* by the number "a" and the return value of the function            */
  truncm(equm(b,a));
  return subm(a,b);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal powi(mfloat_ a, int b) {
  /* a <- a^b */
  mfloat h;
  unsigned bu;

  asm {
   jb sterror      /* check of the stack */
   cmp sp, 0x250   /* minimum rest stack */
   ja stok
  }
  sterror:
  asm mov sp, bp  /* remove local variables */
  mfloatStackOverflow();
  stok:
  if ((a[0] == 0) && (b <= 0)) mfloatError(23); /* pow SINULARITY error */
  equm(h,a);
  equm(a,onem);
  if (b < 0) {
    bu=-b;
    inversm(h);
  }
  else bu=b;
  while (bu > 0) {
    if ((bu & 1) == 1) multm(a,h);    /* "bu" is odd */
    bu/=2;
    if (bu > 0) multm(h,h);
  }
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal powm(mfloat_ a, const mfloat_ b) {
  /* a <- a^b */
  int bint;
  char neg;

  if ((a[0] == 0) && (!gtzero(b))) mfloatError(23); /* pow SINULARITY error */
    /* 0^b -> not defined for b <= 0 */
  bint=mftoint(b);
  if ((b[0] == 0) || ((bint != 0) && (bint < 0x7FFE) && (bint > -0x7FFE)))
    /* b is an integer: repeated multiplications */
    return powi(a,bint);
  else {
    if (a[0] == 0) return a;         /* a=0,   b > 0 */
    else {
      if ((!gezero(a)) && (bint == 0)) mfloatError(22); /* pow DOMAIN error */
	/*  a < 0  and "b" isn't an integer */
      neg=!gezero(a);
      expm(multm(logm(fabsm(a)),b));
      if ((neg) && ((bint & 1) == 1)) negm(a); /* a < 0  and "b" is odd */
      return a;
    }
  }
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal getzerom(mfloat_ a) {
  /*   a <- 0  */

  a[0]=0;
  return a;
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal getonem(mfloat_ a) {
  /*   a <- 1: this constant is stored in "MFLOATC.ASM" */
  return equm(a,onem);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal getpim(mfloat_ a) {
  /*   a <- Pi=3.14159..   this constant is stored in "MFLOATC.ASM" */
  return equm(a,pim);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal getln2m(mfloat_ a) {
  /*   a <- ln(2)=0.69314..  this constant is stored in "MFLOATC.ASM" */
  return equm(a,ln2m);
}

/* ----------------------------------------------------------------------- */

mfloat_ pascal getln10m(mfloat_ a) {
  /*   a <- ln(10)=2.302585..  this constant is stored in "MFLOATC.ASM" */
  return equm(a,ln10m);
}

/* ----------------------------------------------------------------------- */
