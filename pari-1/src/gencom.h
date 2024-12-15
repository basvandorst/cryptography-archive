/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                        Fichier Include PARI                     */
/*                                                                 */
/*                    commun a toutes les versions                 */
/*                                                                 */
/*                        copyright  Babecool                      */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef LONG_IS_32BIT
#define TWOPOTBYTES_IN_LONG  2
#define TWOPOTBITS_IN_LONG   5
#define BYTES_IN_LONG        4
#define BITS_IN_LONG        32
#define MAXULONG    0xffffffffL
#define MAXHALFULONG    0xffffL
#define MAXUBYTE          0xffL
#define HIGHBIT     0x80000000L
#define HIGHMASK    0xffff0000L
#define LOWMASK         0xffffL
#define MAXVAR 150
#define MAXVARN 255
#define MAXSHIFTVAR 8
#endif

#ifdef LONG_IS_64BIT
#define TWOPOTBYTES_IN_LONG          3
#define TWOPOTBITS_IN_LONG           6
#define BYTES_IN_LONG                8
#define BITS_IN_LONG                64
#define MAXULONG    0xffffffffffffffffL
#define MAXHALFULONG        0xffffffffL
#define MAXUBYTE                0xffffL
#define HIGHBIT     0x8000000000000000L
#define HIGHMASK    0xffffffff00000000L
#define LOWMASK             0xffffffffL
#define MAXVAR 8100
#define MAXVARN 8191
#define MAXSHIFTVAR 13
#endif

#ifdef LONG_IS_32BIT
#define SIGNBITS 0xff000000L
#define SIGNSHIFT 24
#define TYPBITS 0xff000000L
#define TYPSHIFT 24
#define PEREBITS 0xff0000L
#define PERESHIFT 16
#define LGBITS 0xffffL
#define LGEFBITS 0xffffL
#define EXPOBITS 0xffffffL
#define HIGHEXPOBIT 0x800000L
#define VALPBITS 0xffffL
#define HIGHVALPBIT 0x8000L
#define PRECPBITS 0xffff0000L
#define PRECPSHIFT 16
#define VARNBITS 0xff0000L
#define VARNSHIFT 16
#endif

#ifdef LONG_IS_64BIT
#define SIGNBITS  0xffff000000000000L
#define SIGNSHIFT 48
#define TYPBITS   0xffff000000000000L
#define TYPSHIFT  48
#define PEREBITS      0xffff00000000L
#define PERESHIFT 32
#define LGBITS            0xffffffffL
#define LGEFBITS          0xffffffffL
#define EXPOBITS      0xffffffffffffL
#define HIGHEXPOBIT   0x800000000000L
#define VALPBITS          0xffffffffL
#define HIGHVALPBIT       0x80000000L
#define PRECPBITS 0xffffffff00000000L
#define PRECPSHIFT 32
#define VARNBITS      0xffff00000000L
#define VARNSHIFT  32
#endif

typedef long    *GEN;

#ifdef ULONG_NOT_DEFINED
typedef unsigned long ulong;
#endif

typedef struct entree {
  char *name;
  long valence;
  void *value;
  long menu;
  struct entree *next;
} entree;

typedef unsigned char *byteptr;

/*      Variables statiques communes :
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  */

extern  unsigned long overflow,hiremainder;

extern  long    prec,precdl,defaultpadicprecision;
extern  GEN     bernzone,gpi,geuler;
extern  long    tglobal,paribuffsize,pariecho;
extern  long    *ordvar,varchanged;
extern  GEN     polvar;
extern  GEN     RAVYZARC;

extern  long    NUMFUNC;
extern  entree  fonctions[],**hashtable;
extern  long    lontyp[],lontyp2[];

extern  jmp_buf environnement;
extern  FILE    *outfile, *logfile, *infile, *errfile;

extern  unsigned long    avma,bot,top;
extern  GEN     gnil,gun,gdeux,ghalf,gi,gzero;

extern  GEN     *polun,*polx;
extern  byteptr diffptr;

extern  GEN     *g;
extern  entree  **varentries; /* noms des inconnues actives */
extern  GEN     *blocliste;   /* tableau des pointeur de blocs */
extern  long    nvar;         /* numero de la prochaine inconnue */
extern  long    glbfmt[];
extern  long    **rectgraph;
extern  long    pari_randseed;
extern  long    DEBUGLEVEL;

extern const int STACKSIZE;  /* nombre de gn possibles */
extern const int TBLSZ;  /* taille de la table de hashcodes */
extern const int MAXBLOC;  /* nombre de blocs autorises dans le tas */

extern const double K;             /* 32*log(2)/log(10)    */
extern const double K1;             /* log(10)/(32*log(2))  */
extern const double K2;               /* 1/(1-(log(2)/(2*pi)))*/
extern const double K4;            /* e*pi/16              */
extern const double LOG2;       /* log(2)               */
extern const double L2SL10;     /* log(2)/log(10)       */
#ifndef  PI
extern const double PI;          /* pi                   */
#endif
extern const double rac5;           /* racine de 5          */
extern const double C1;            /* log(2*pi)/2          */
extern const double C2;             /* 32*log(2)            */
extern const double C3;     /* log((1+sqrt(5))/2)/(32*log(2)) */
extern const double C31;           /* 2^31                 */
extern const long BIGINT;                  /* 2^15-1               */
extern const long EXP220;            /* 2^20                 */
extern const long VERYBIGINT;        /* 2^31-1               */

extern  char    *helpmessage[]; /* un message pour chaque fonction predefinie */
extern  char    *errmessage[];  /* un par numero d'erreur */

#ifndef exp2
#ifdef __cplusplus
inline double exp2(double x) {return exp(x*LOG2);}
inline double log2(double x) {return log(x)/LOG2;}
#else
#define exp2(x) exp((double)(x)*log(2.))
#define log2(x) log((double)(x))/log(2.)
#endif
#endif

#ifdef LONG_IS_32BIT
#define labs(x) abs(x)
#endif

#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((a)>(b)?(a):(b))

#define separe(c)     ((c==';')||(c==':')||(c=='\n'))

#define output(x)     {brute(x,'g',-1);pariputc('\n');fflush(stdout);}
#define outmat(x)     {matbrute(x,'g',-1);pariputc('\n');fflush(stdout);}
#define outbeaut(x)   {sor(x,'g',-1,0);pariputc('\n');fflush(stdout);}

#define addis(x,s)  addsi(s,x)
#define addrs(x,s)  addsr(s,x)
#define mulis(x,s)  mulsi(s,x)
#define mulri(x,s)  mulir(s,x)
#define mulrs(x,s)  mulsr(s,x)

#define gval(x,v) ggval(x,polx[v])
#define gvar9(x) ((typ(x)==9)?gvar2(x):gvar(x))

#define lgeti   (long)cgeti
#define lgetr   (long)cgetr
#define lpile   (long)gerepile
#define lstoi   (long)stoi
#define lnegi   (long)negi
#define lnegr   (long)negr
#define lmpneg  (long)mpneg
#define labsi   (long)absi
#define labsr   (long)absr
#define lmpabs  (long)mpabs
#define lmptrunc (long)mptrunc
#define lmpent  (long)mpent
#define lshifts (long)shifts
#define lshifti (long)shifti
#define lshiftr (long)shiftr
#define lmpshift (long)mpshift
#define laddsi  (long)addsi
#define laddsr  (long)addsr
#define laddis  (long)addis
#define laddrs  (long)addrs
#define laddii  (long)addii
#define laddir  (long)addir
#define laddrr  (long)addrr
#define lmpadd  (long)mpadd
#define lsubsi  (long)subsi
#define lsubis  (long)subis
#define lsubsr  (long)subsr
#define lsubrs  (long)subrs
#define lsubii  (long)subii
#define lsubir  (long)subir
#define lsubri  (long)subri
#define lsubrr  (long)subrr
#define lmpsub  (long)mpsub
#define lmulss  (long)mulss
#define lmulsi  (long)mulsi
#define lmulsr  (long)mulsr
#define lmulis  (long)mulis
#define lmulrs  (long)mulrs
#define lmulii  (long)mulii
#define lmulir  (long)mulir
#define lmulri  (long)mulri
#define lmulrr  (long)mulrr
#define lmpmul  (long)mpmul
#define ldivsi  (long)divsi
#define ldivis  (long)divis
#define ldivsr  (long)divsr
#define ldivrs  (long)divrs
#define ldivii  (long)divii
#define ldivir  (long)divir
#define ldivri  (long)divri
#define ldivrr  (long)divrr
#define lmpdiv  (long)mpdiv
#define lmodii  (long)modii
#define lresii  (long)resii
#define ldvmdii (long)dvmdii
#define ldvmdsi (long)dvmdsi
#define ldvmdis (long)dvmdis
  
#define ltree   (long)gettree
#define lgen    (long)getgen
#define lcopy   (long)gcopy
#define lclone  (long)gclone
#define lgetg   (long)cgetg
#define lgetp   (long)cgetp
#define laddpex (long)gaddpex
#define lgreffe (long)greffe
#define lopsg2  (long)gopsg2
#define lopgs2  (long)gopgs2
#define lco8    (long)co8
#define lneg    (long)gneg
#define lmax    (long)gmax
#define lmin    (long)gmin
#define ladd    (long)gadd
#define lsub    (long)gsub
#define lmul    (long)gmul
#define ldiv    (long)gdiv
#define linv    (long)ginv
#define lmod    (long)gmod
#define ldivmod (long)gdivmod
#define lshift  (long)gshift
#define lmul2n  (long)gmul2n
#define lpuigs  (long)gpuigs
#define lpui    (long)gpui
#define lsubst  (long)gsubst
#define lderiv  (long)deriv
#define linteg  (long)integ
#define lrecip  (long)recip
#define lceil   (long)gceil
#define lfloor  (long)gfloor
#define lround  (long)ground
#define lcvtoi  (long)gcvtoi
#define lrndtoi (long)grndtoi
#define lfrac   (long)gfrac
#define ltrunc  (long)gtrunc
#define lmodulcp  (long)gmodulcp
#define lmodulo  (long)gmodulo
  
#define lconcat (long)concat
#define lnorm   (long)gnorm
#define lnorml2 (long)gnorml2
#define lconj   (long)gconj
#define lreal   (long)greal
#define limag   (long)gimag
#define lmppi   (long)mppi
#define lmpeuler (long)mpeuler
#define lmpsqrt (long)mpsqrt
#define lsqrt   (long)gsqrt
#define lmpexp1 (long)mpexp1
#define lmpexp  (long)mpexp
#define lexp    (long)gexp
#define lmplog  (long)mplog
#define llog    (long)glog
#define lmpsc1  (long)mpsc1
#define lmpcos  (long)mpcos
#define lcos    (long)gcos
#define lmpsin  (long)mpsin
#define lsin    (long)gsin
#define lmpaut  (long)mpaut
#define lmptan  (long)mptan
#define ltan    (long)gtan
#define lmpatan (long)mpatan
#define latan   (long)gatan
#define lmpasin (long)mpasin
#define lasin   (long)gasin
#define lmpacos (long)mpacos
#define lacos   (long)gacos
#define lmpch   (long)mpch
#define lch     (long)gch
#define lmpsh   (long)mpsh
#define lsh     (long)gsh
#define lmpth   (long)mpth
#define lth     (long)gth
#define lmpath  (long)mpath
#define lath    (long)gath
#define lmpash  (long)mpash
#define lash    (long)gash
#define lmpach  (long)mpach
#define lach    (long)gach
#define lmpgamma (long)mpgamma
#define lgamma  (long)ggamma
#define lmplngamma (long)mplngamma
#define llngamma  (long)glngamma
#define lgamd   (long)ggamd
#define lmppsi  (long)mppsi
#define lpsi    (long)gpsi
#define lmpgamd (long)mpgamd
#define larg    (long)garg
#define lsqr    (long)gsqr
  
#define ltrans  (long)gtrans
#define lscalmat (long)gscalmat
#define lscalsmat (long)gscalsmat
#define laddmat (long)gaddmat
#define laddsmat (long)gaddsmat
#define lgauss  (long)gauss
#define linvmat (long)invmat
#define linvmulmat (long)invmulmat
#define ldet    (long)det
#define ldet2   (long)det2
#define lcaract (long)caract
#define lcaradj (long)caradj
#define ladj    (long)adj
#define ltrace  (long)trace
#define lassmat (long)assmat
#define lscal   (long)gscal
  
#define linvmod (long)ginvmod
#define lred    (long)gred
#define ldeuc   (long)gdeuc
#define lres    (long)gres
#define ldivres (long)poldivres
#define lpoleval (long)poleval
#define lroots  (long)roots
#define lgcd    (long)ggcd
#define lpolgcd (long)polgcd
#define lcontent (long)content
#define lprimpart (long)primpart
#define lpsres  (long)psres
#define lsubres (long)subres
#define ldiscsr (long)discsr
#define lquadpoly (long)quadpoly
#define lquadgen (long)quadgen
  
#define llegendre (long)legendre
#define ltchebi (long)tchebi
#define lhilb   (long)hilb
#define lpasc   (long)pasc
#define lprec   (long)gprec
#define lbinome (long)binome
  
#define lracine (long)racine
#define lmppgcd (long)mppgcd
#define lmpfact (long)mpfact
#define lsfcont (long)sfcont
#define lbezout (long)bezout
#define lmpinvmod (long)mpinvmod
#define lpuissmodulo (long)puissmodulo
#define lfibo   (long)fibo
#define lchangevar (long)changevar
  
#define zero    (long)gzero
#define un      (long)gun
#define deux    (long)gdeux
#define lhalf   (long)ghalf
  
#define lpolx   (long)polx
#define lpolun   (long)polun

#define mpmodz(x,y,z)     (modiiz(x,y,z))
#define mpresz(x,y,z)     (resiiz(x,y,z))
#define mpmod(x,y)        (modii(x,y))
#define mpres(x,y)        (resii(x,y))

#define laddsg(s,y)         (lopsg2(gadd,s,y))
#define laddgs(y,s)         (lopsg2(gadd,s,y))
#define lsubsg(s,y)         (lopsg2(gsub,s,y))
#define lsubgs(y,s)         (lopgs2(gsub,y,s))
#define lmulsg(s,y)         ((long)gmulsg(s,y))
#define lmulgs(y,s)         ((long)gmulsg(s,y))
#define ldivsg(s,y)         (lopsg2(gdiv,s,y))
#define ldivgs(x,s)         ((long)gdivgs(x,s))
#define lmodsg(s,y)         (lopsg2(gmod,s,y))
#define lmodgs(y,s)         (lopgs2(gmod,y,s))
#define ldiventsg(s,y)      (lopsg2(gdivent,s,y))
#define ldiventgs(y,s)      (lopgs2(gdivent,y,s))
#define lminsg(s,y)         (lopsg2(gmin,s,y))
#define lmings(y,s)         (lopgs2(gmin,y,s))
#define lmaxsg(s,y)         (lopsg2(gmax,s,y))
#define lmaxgs(y,s)         (lopgs2(gmax,y,s))


#define mppiz(x)              (gop0z(mppi,x))
#define mpeulerz(x)           (gop0z(mpeuler,x))
#define mpsqrtz(x,y)        (gop1z(mpsqrt,x,y))
#define mpexpz(x,y)         (gop1z(mpexp,x,y))
#define mpexp1z(x,y)        (gop1z(mpexp1,x,y))
#define mplogz(x,y)         (gop1z(mplog,x,y))
#define mpcosz(x,y)         (gop1z(mpcos,x,y))
#define mpsinz(x,y)         (gop1z(mpsin,x,y))
#define autz(x,y)           (gop1z(mpaut,x,y))
#define mptanz(x,y)         (gop1z(mptan,x,y))
#define mpatanz(x,y)        (gop1z(mpatan,x,y))
#define mpasinz(x,y)        (gop1z(mpasin,x,y))
#define mpacosz(x,y)        (gop1z(mpacos,x,y))
#define mpchz(x,y)          (gop1z(mpch,x,y))
#define mpshz(x,y)          (gop1z(mpsh,x,y))
#define mpthz(x,y)          (gop1z(mpth,x,y))
#define mpathz(x,y)         (gop1z(mpath,x,y))
#define mpashz(x,y)         (gop1z(mpash,x,y))
#define mpachz(x,y)         (gop1z(mpach,x,y))
#define mpgammaz(x,y)       (gop1z(mpgamma,x,y))
#define mpargz(x,y,z)      (gop2z(mparg,x,y,z))
#define mpfactz(s,y)        (gops1z(mpfact,s,y))

#define gredz(x,y)          (gop1z(gred,x,y))
#define gnegz(x,y)          (gop1z(gneg,x,y))
#define gabsz(x,y)          (gop1z(gabs,x,y))
#define gmaxz(x,y,z)       (gop2z(gmax,x,y,z))
#define gminz(x,y,z)       (gop2z(gmin,x,y,z))
#define gaddz(x,y,z)       (gop2z(gadd,x,y,z))
#define gsubz(x,y,z)       (gop2z(gsub,x,y,z))
#define gmulz(x,y,z)       (gop2z(gmul,x,y,z))
#define gdivz(x,y,z)       (gop2z(gdiv,x,y,z))
#define gdeucz(x,y,z)      (gop2z(gdeuc,x,y,z))
#define gdiventz(x,y,z)    (gop2z(gdivent,x,y,z))
#define gmodz(x,y,z)       (gop2z(gmod,x,y,z))
#define gshiftz(x,s,z)      (gops2gsz(gshift,x,s,z))
#define gmul2nz(x,s,z)      (gops2gsz(gmul2n,x,s,z))
#define gaddsg(s,y)         (gopsg2(gadd,s,y))
#define gaddgs(y,s)         (gopsg2(gadd,s,y))
#define gsubsg(s,y)         (gopsg2(gsub,s,y))
#define gsubgs(y,s)         (gopgs2(gsub,y,s))
#define gcmpsg(s,y)         (-opgs2(gcmp,y,s))
#define gcmpgs(y,s)         (opgs2(gcmp,y,s))
#define gegalsg(s,y)        (opgs2(gegal,y,s))
#define gegalgs(y,s)        (opgs2(gegal,y,s))
#define gmulgs(y,s)         (gmulsg(s,y))
#define gdivsg(s,y)         (gopsg2(gdiv,s,y))
#define gdiventsg(s,y)      (gopsg2(gdivent,s,y))
#define gdiventgs(y,s)      (gopgs2(gdivent,y,s))
#define gmodsg(s,y)         (gopsg2(gmod,s,y))
#define gmodgs(y,s)         (gopgs2(gmod,y,s))
#define gminsg(s,y)         (gopsg2(gmin,s,y))
#define gmings(y,s)         (gopgs2(gmin,y,s))
#define gmaxsg(s,y)         (gopsg2(gmax,s,y))
#define gmaxgs(y,s)         (gopgs2(gmax,y,s))


#define gaddsgz(s,y,z)    (gopsg2z(gadd,s,y,z))
#define gaddgsz(y,s,z)    (gopsg2z(gadd,s,y,z))
#define gsubsgz(s,y,z)    (gopsg2z(gsub,s,y,z))
#define gsubgsz(y,s,z)    (gopgs2z(gsub,y,s,z))
#define gmulsgz(s,y,z)    (gops2sgz(gmulsg,s,y,z))
#define gmulgsz(y,s,z)    (gops2sgz(gmulsg,s,y,z))
#define gdivsgz(s,y,z)    (gopsg2z(gdiv,s,y,z))
#define gdivgsz(y,s,z)    (gops2gsz(gdivgs,y,s,z))
#define gdiventsgz(s,y,z) (gopsg2z(gdivent,s,y,z))
#define gdiventgsz(y,s,z) (gopgs2z(gdivent,y,s,z))
#define gmodsgz(s,y,z)    (gopsg2z(gmod,s,y,z))
#define gmodgsz(y,s,z)    (gopgs2z(gmod,y,s,z))
#define gminsgz(s,y,z)    (gopsg2z(gmin,s,y,z))
#define gmingsz(y,s,z)    (gopgs2z(gmin,y,s,z))
#define gmaxsgz(s,y,z)    (gopsg2z(gmax,s,y,z))
#define gmaxgsz(y,s,z)    (gopgs2z(gmax,y,s,z))

#define coeff(a,i,j)      (*((long*)(*(a+(j)))+(i)))
#define gcoeff(a,i,j)     (GEN)coeff(a,i,j)
#define bern(i)           (GEN)(bernzone + (i)*(*(bernzone + 2)) + 3)

#define copyifstack(x) (RAVYZARC=(GEN)(x),((RAVYZARC>=(GEN)bot)&&(RAVYZARC<(GEN)top))?lcopy(RAVYZARC):(long)RAVYZARC)

#ifdef __cplusplus
inline int isonstack(GEN x) {GEN RAVYZARC=x; return ((RAVYZARC>=(GEN)bot)&&(RAVYZARC<(GEN)top));}
inline int adecaler(GEN x, long tetpil, long anavma) {GEN RAVYZARC=x;return ((RAVYZARC>=(GEN)anavma)&&(RAVYZARC<(GEN)tetpil));}
#else
#define isonstack(x)   (RAVYZARC=(GEN)(x),((RAVYZARC>=(GEN)bot)&&(RAVYZARC<(GEN)top)))
#define adecaler(x,tetpil,anavma) (RAVYZARC=(GEN)(x),((RAVYZARC>=(GEN)anavma)&&(RAVYZARC<(GEN)tetpil)))
#endif

#define isscalar(x)   ((typ(x)<10)||((typ(x)==10)&&(lgef(x)<=3)))
#define isnonscalar(x)  ((typ(x)==10)&&(lgef(x)>3))
#define leadingterm(x)  ((typ(x)<10)?x:((GEN)(x[lgef(x)-1])))

#ifdef __cplusplus
inline int odd(long x) {return x&1;}
#else
#define odd(x)                ((x) & 1)
#endif

#define mpodd(x) (signe(x) && odd(mant(x,lgef(x) - 2)))

#define evalsigne(x) (((long)(x))<<SIGNSHIFT)
#define evaltyp(x) (((ulong)(x))<<TYPSHIFT)
#define evalpere(x) (((ulong)(x))<<PERESHIFT)
#define evallg(x) (x)
#define evallgef(x) (x)
#define evalvarn(x) (((ulong)(x))<<VARNSHIFT)
#define evalprecp(x) (((long)(x))<<PRECPSHIFT)
#define evalexpo(x) (HIGHEXPOBIT+(x))
#define evalvalp(x) (HIGHVALPBIT+(x))

/* alglin.c */
     
GEN     gtrans(GEN x),gscalmat(GEN x, long n),gscalsmat(long x, long n),gaddmat(GEN x, GEN y),gaddsmat(long s, GEN y),inverseimage(GEN mat, GEN y);
GEN     ker(GEN x),keri(GEN x),kerreel(GEN x, long prec);
GEN     image(GEN x),imagereel(GEN x, long prec),imagecompl(GEN x),image2(GEN x),suppl(GEN x),eigen(GEN x, long prec),hess(GEN x);
GEN     carhess(GEN x, long v);
GEN     gauss(GEN a, GEN b),invmat(GEN a),det(GEN a),detreel(GEN a),det2(GEN a);
GEN     caract(GEN x, int v),caradj(GEN x, long v, GEN *py),adj(GEN x),caradj0(GEN x, long v),trace(GEN x),trace9(GEN x,GEN p1);
GEN     assmat(GEN x),gnorm(GEN x),gnorml2(GEN x),gconj(GEN x),concat(GEN x, GEN y),idmat(long n),conjvec(GEN x,long prec);
GEN     extract(GEN x, GEN l),matextract(GEN x, GEN l1, GEN l2),gtomat(GEN x),invmulmat(GEN a, GEN b),invmulmatreel(GEN a, GEN b),invmatreel(GEN a);
GEN     sqred(GEN a),sqred1(GEN a),sqred2(GEN a, long flg),sqred3(GEN a),signat(GEN a),jacobi(GEN a, long prec),matrixqz(GEN x, GEN pp),matrixqz2(GEN x),matrixqz3(GEN x);
GEN     indexrank(GEN x),kerint(GEN x),kerint1(GEN x),kerint2(GEN x),intersect(GEN x, GEN y),deplin(GEN x),detint(GEN x);
GEN     hnfspec(long** mat,GEN* ptdep,GEN* ptmatc,long* vperm,GEN* ptmatalpha,long co,long li,long k0,long* ptnlze,long* ptcol);
GEN     hnffinal(GEN matgen,GEN* ptpdep,GEN* ptmatc,long* vperm,GEN* ptmatalpha,long lnz,long co,long li,long col,long lig,long nlze,long* ptcol);
GEN     hnfadd(GEN mit,GEN* ptpdep,GEN* ptmatc,long* vperm,GEN* ptmatalpha,long co,long li,long col,long* ptnlze,GEN extramat,GEN extramatc);
long    rank(GEN x),perf(GEN a);

/* anal.c */
     
GEN     lisexpr(char *t),readexpr(char **c),lisseq(char *t),readseq(char **c);
void    switchin(char *name), switchout(char *name), fliplog(void);
     
/* arith.c */
     
GEN     racine(GEN a),mppgcd(GEN a, GEN b),mpfact(long n),mpfactr(long n, long prec);
GEN     sfcont(GEN x, GEN x1, long k),sfcont2(GEN b, GEN x),gcf(GEN x),gcf2(GEN b, GEN x),pnqn(GEN x),gboundcf(GEN x, long k);
GEN     bestappr(GEN x, GEN k);
GEN     bezout(GEN a, GEN b, GEN *u, GEN *v),chinois(GEN x, GEN y),mpinvmod(GEN a, GEN m),puissmodulo(GEN a, GEN n, GEN m),fibo(long n),bigprem(GEN n),prime(long n);
GEN     primes(long n),phi(GEN n),decomp(GEN n),auxdecomp(GEN n, long all),smallfact(GEN n),boundfact(GEN n, long lim);
GEN     sumdiv(GEN n),sumdivk(long k, GEN n),numbdiv(GEN n),binaire(GEN x),order(GEN x),gener(GEN m),divisors(GEN n);
GEN     ellfacteur(GEN n1),classno(GEN x),classno2(GEN x),classno3(GEN x),fundunit(GEN x),regula(GEN x, long prec);
GEN     compimag(GEN x, GEN y),sqcomp(GEN x),qfi(GEN x, GEN y, GEN z),qfr(GEN x, GEN y, GEN z, GEN d),compreal(GEN x, GEN y),redreal(GEN x),sqcompreal(GEN x);
GEN     rhoreal(GEN x),rhorealnod(GEN x, GEN isqrtD),redrealnod(GEN x, GEN isqrtD),redimag(GEN x);
#ifdef __cplusplus
GEN     primeform(GEN x, GEN p, long prec = 0);
#else
GEN     primeform(GEN x, GEN p, ...);
#endif
GEN     nucomp(GEN x, GEN y, GEN l),nudupl(GEN x, GEN l),nupow(GEN x, GEN n);
GEN     comprealraw(GEN x, GEN y),sqcomprealraw(GEN x),powrealraw(GEN x, long n, long prec);

GEN     gkronecker(GEN x, GEN y),gkrogs(GEN x, long y),gcarreparfait(GEN x),gcarrecomplet(GEN x, GEN *pt);
GEN     gisprime(GEN x),gispsp(GEN x),gissquarefree(GEN x),gisfundamental(GEN x),gbittest(GEN x, GEN n);
GEN     gpseudopremier(GEN n, GEN a),gmillerrabin(GEN n, long k),gmu(GEN n),gomega(GEN n),gbigomega(GEN n);

long    kronecker(GEN x, GEN y),krosg(long s, GEN x),krogs(GEN x, long y),kross(long x, long y),kro8(GEN x, GEN y);
long    mu(GEN n),omega(GEN n),bigomega(GEN n),hil(GEN x, GEN y, GEN p);
int     carreparfait(GEN x),carrecomplet(GEN x, GEN *pt),bittest(GEN x, long n);
int     isprime(GEN x),ispsp(GEN x),issquarefree(GEN x),isfundamental(GEN x),mpsqrtmod(GEN a, GEN p, GEN *pr);
int     millerrabin(GEN n, long k),pseudopremier(GEN n, GEN a),inversemodulo(GEN a, GEN b, GEN *res);
byteptr initprimes(long maxnum);
void    lucas(long n, GEN *ln, GEN *ln1);

/* base.c */

GEN     base(GEN x, GEN *y),smallbase(GEN x, GEN *y),discf(GEN x),smalldiscf(GEN x),discf2(GEN x);
GEN     hnf(GEN x),hnfhavas(GEN x),cleanmod(GEN x,GEN detmat,GEN detmatsur2);
GEN     hnfmod(GEN x, GEN detmat),hnfmodid(GEN x,GEN p),smith(GEN x),smith2(GEN x);
GEN     factoredbase(GEN x, GEN p, GEN *y),factoreddiscf(GEN x, GEN p),allbase(GEN x, long code, GEN *y),galois(GEN x, long prec),initalg(GEN x, long prec),initalg0(GEN x, long prec);
GEN     tschirnhaus(GEN x),galoisconj(GEN x, long prec),initalg2(GEN x, long prec);
GEN     primedec(GEN nf,GEN p),idealmul(GEN nf,GEN ix,GEN iy),idealmulred(GEN nf, GEN ix, GEN iy, long prec);
GEN     idealmulh(GEN nf, GEN ix, GEN iy),element_mulh(GEN nf, long limi, long limj, GEN x, GEN y);
GEN     idealmulprime(GEN nf,GEN ix,GEN vp),minideal(GEN nf,GEN ix,GEN vdir,long prec);
GEN     ideallllredall(GEN nf, GEN ix, GEN vdir, long prec, long precint);
GEN     ideallllred(GEN nf,GEN ix,GEN vdir,long prec);
GEN     ideallllredpart1(GEN nf,GEN x,GEN vdir,long prec);
GEN     ideallllredpart2(GEN nf,GEN arch,GEN z,long prec);
GEN     element_mul(GEN nf,GEN x,GEN y),element_pow(GEN nf,GEN x,GEN k);
GEN     rootsof1(GEN x),hnfmod(GEN x,GEN detmat),idealinv(GEN nf, GEN ix);
GEN     idealpow(GEN nf, GEN ix, GEN n),idealpowred(GEN nf, GEN ix, GEN n, long prec);
GEN     idealpowprime(GEN nf, GEN vp, GEN n,long prec),idealfactor(GEN nf, GEN x);
GEN     idealadd(GEN nf, GEN x, GEN y), idealdiv(GEN nf, GEN x, GEN y);
GEN     idealintersect(GEN nf, GEN x, GEN y), principalideal(GEN nf, GEN a);
GEN     principalidele(GEN nf, GEN a);
long    idealval(GEN nf,GEN ix,GEN vp), isideal(GEN nf,GEN x);
long    elementval2(GEN nf, GEN x, GEN d, GEN vp);

GEN     allbase4(GEN f,long code, GEN *y ),base2(GEN x, GEN *y);
GEN     bsrch(GEN p, GEN fa, long Ka, GEN eta, long Ma),setup(GEN p,GEN f,GEN theta,GEN nut),eleval(GEN f,GEN h,GEN a),vstar(GEN p,GEN h),factcp(GEN p,GEN f,GEN beta),bestnu(GEN w),gcdpm(GEN f1,GEN f2,GEN pm);
GEN     compositum(GEN pol1, GEN pol2);

/* bibli1.c */
     
GEN     tayl(GEN x, long v, long precdl),legendre(long n),tchebi(long n),hilb(long n),pasc(long n),laplace(GEN x);
GEN     gprec(GEN x, long l),convol(GEN x, GEN y),ggrando(GEN x, long n),gconvsp(GEN x),gconvpe(GEN x);
GEN     lll(GEN x, long prec),lll1(GEN x, long prec),lllrat(GEN x),lllgram(GEN x, long prec),lllgram1(GEN x, long prec),lllgramint(GEN x),lllint(GEN x);
GEN     lllgramkerim(GEN x),lllkerim(GEN x),lllgramall(GEN x, long all),lllall0(GEN x, long all);
GEN     lllgen(GEN x),lllkerimgen(GEN x),lllgramgen(GEN x),lllgramkerimgen(GEN x),lllgramallgen(GEN x, long all);
GEN     binome(GEN x, long k),gscal(GEN x, GEN y),cyclo(long n),vecsort(GEN x, GEN k);
GEN     lindep(GEN x, long prec),lindep2(GEN x, long bit, long prec),algdep(GEN x, long n, long prec),algdep2(GEN x, long n, long bit, long prec),changevar(GEN x, GEN y),ordred(GEN x, long prec);
GEN     polrecip(GEN x),reorder(GEN x),sort(GEN x),lexsort(GEN x),indexsort(GEN x),polsym(GEN x, long n);
GEN     minim(GEN a, long borne, long stockmax),minimprim(GEN a, long borne, long stockmax);
GEN     polred(GEN x, long prec),factoredpolred(GEN x, GEN p, long prec),smallpolred(GEN x, long prec),polred2(GEN x, long prec),factoredpolred2(GEN x, GEN p, long prec);
GEN     smallpolred2(GEN x, long prec),allpolred(GEN x, GEN *pta, long code, long prec),polymodrecip(GEN x),genrand(void),permute(long n, GEN x);
int     mymyrand();
long    setprecr(long n),setserieslength(long n);
GEN     setrand(int seed),getrand(void),getstack(void),gettime(void);
     
/* bibli2.c */

GEN     somme(entree *ep, GEN x, GEN a, GEN b, char *ch),produit(entree *ep, GEN x, GEN a, GEN b, char *ch),suminf(entree *ep, GEN a, char *ch, long prec),prodinf(entree *ep, GEN a, char *ch, long prec),prodinf1(entree *ep, GEN a, char *ch, long prec),prodeuler(entree *ep, GEN a, GEN b, char *ch, long prec);
GEN     vecteur(entree *ep, GEN nmax, char *ch),vvecteur(entree *ep, GEN nmax, char *ch),matrice(entree *ep1, entree *ep2, GEN nlig, GEN ncol, char *ch),divsomme(entree *ep, GEN num, char *ch);
GEN     qromb(entree *ep, GEN a, GEN b, char *ch, long prec),qromo(entree *ep, GEN a, GEN b, char *ch, long prec),qromi(entree *ep, GEN a, GEN b, char *ch, long prec),rombint(entree *ep, GEN a, GEN b, char *ch, long prec);
GEN     polint(GEN xa, GEN ya, GEN x, GEN *dy),plot(entree *ep, GEN a, GEN b, char *ch),ploth(entree *ep, GEN a, GEN b, char *ch, long prec),ploth2(entree *ep, GEN a, GEN b, char *ch, long prec),plothraw(GEN listx, GEN listy),zbrent(entree *ep, GEN a, GEN b, char *ch, long prec),sumalt(entree *ep, GEN a, char *ch, long prec),sumpos(entree *ep, GEN a, char *ch, long prec);
GEN     forpari(entree *ep, GEN a, GEN b, char *ch),forstep(entree *ep, GEN a, GEN b, GEN s, char *ch),fordiv(entree *ep, GEN a, char *ch),forprime(entree *ep, GEN a, GEN b, char *ch);
GEN     initrect(long ne, long x, long y),killrect(long ne),rectcursor(long ne),rectmove(long ne, GEN x, GEN y),rectrmove(long ne, GEN x, GEN y),rectpoint(long ne, GEN x, GEN y);
GEN     rectrpoint(long ne, GEN x, GEN y),rectbox(long ne, GEN gx2, GEN gy2),rectrbox(long ne, GEN gx2, GEN gy2),rectline(long ne, GEN gx2, GEN gy2),rectrline(long ne, GEN gx2, GEN gy2),rectdraw(GEN list);
GEN     rectpoints(long ne, GEN listx, GEN listy),rectlines(long ne, GEN listx, GEN listy),rectstring(long ne, GEN x),rectscale(long ne, GEN x1, GEN x2, GEN y1, GEN y2);
GEN     postdraw(GEN list),postploth(entree *ep, GEN a, GEN b, char *ch),postploth2(entree *ep, GEN a, GEN b, char *ch),postplothraw(GEN listx, GEN listy);

/* buch1.c et buch2.c */

GEN     buchimag(GEN D, GEN gcbach, GEN gcbach2);
GEN     buchreal(GEN D, GEN gsens, GEN gcbach, GEN gcbach2, long prec);
GEN     buchgen(GEN P, GEN gcbach, GEN gcbach2, long prec);
GEN     buchgenfu(GEN P, GEN gcbach, GEN gcbach2, long prec);
GEN     buchinit(GEN P, GEN gcbach, GEN gcbach2, long prec);
GEN     buchinitfu(GEN P, GEN gcbach, GEN gcbach2, long prec);
GEN     isprincipal(GEN bignf, GEN x);
GEN     isunit(GEN bignf, GEN x), signunit(GEN bignf);
int     compte(long **mat, long row, long longueur, long *firstnonzero);
int     compte2(long **mat, long row, long longueur, long *firstnonzero);

/* elliptic.c */

GEN     ghell(GEN e, GEN a, long prec),ghell2(GEN e, GEN a, long prec),ghell3(GEN e, GEN a, long prec);
GEN     initell(GEN x, long prec),initell2(GEN x, long prec),smallinitell(GEN x),zell(GEN e, GEN z, long prec),coordch(GEN e, GEN ch),pointch(GEN x, GEN ch);
GEN     addell(GEN e, GEN z1, GEN z2),subell(GEN e, GEN z1, GEN z2),powell(GEN e, GEN z, GEN n);
GEN     mathell(GEN e, GEN x, long prec),bilhell(GEN e, GEN z1, GEN z2, long prec);
GEN     ordell(GEN e, GEN x, long prec),apell(GEN e, GEN pl),apell1(GEN e, GEN p),apell2(GEN e, GEN p);
GEN     anell(GEN e, long n),akell(GEN e, GEN n);
GEN     localreduction(GEN e, GEN p1), globalreduction(GEN e1);
GEN     lseriesell(GEN e, GEN s, GEN N, GEN A, long prec);
GEN     pointell(GEN e, GEN z, long prec),taniyama(GEN e);
GEN     orderell(GEN e, GEN p),torsell(GEN e);
int     oncurve(GEN e, GEN z);
void    eulsum(GEN *sum, GEN term, long jterm, GEN *tab, long *dsum, long prec);

/* es.c */

void    filtre(char *s),  pariputc(char c), pariputs(char *s), ecrire(GEN x, char format, long dec, long chmp), voir(GEN x, long nb), sor(GEN g, char fo, long dd, long chmp);
void    brute(GEN g, char format, long dec), matbrute(GEN g, char format, long dec), texe(GEN g, char format, long dec), etatpile(unsigned int n);

void printvargp(long);
extern void (*printvariable)(long);

long timer(void),timer2(void);

/* gen1.c */

GEN     gadd(GEN x, GEN y),gsub(GEN x, GEN y),gmul(GEN x, GEN y),gdiv(GEN x, GEN y);

/* gen2.c gen3.c */

GEN     gcopy(GEN x),forcecopy(GEN x),gclone(GEN x),cgetp(GEN x),gaddpex(GEN x, GEN y);
GEN     greffe(GEN x, long l),gopsg2(GEN (*f) (GEN, GEN), long s, GEN y),gopgs2(GEN (*f) (GEN, GEN), GEN y, long s),co8(GEN x, long l),cvtop(GEN x, GEN p, long l),compo(GEN x, long n),gsqr(GEN x);
#ifdef  __cplusplus
GEN     gneg(GEN x),gabs(GEN x, long prec = 0);
GEN     gpuigs(GEN x, long n, long prec = 0);
GEN     gpui(GEN x, GEN n, long prec = 0);
#else
GEN     gneg(GEN x),gabs(GEN x, ...);
GEN     gpuigs(GEN x, long n, ...);
GEN     gpui(GEN x, GEN n, ...);
#endif
GEN     gmax(GEN x, GEN y),gmin(GEN x, GEN y),ginv(GEN x),denom(GEN x),numer(GEN x),lift(GEN x),centerlift(GEN x),vecmax(GEN x),vecmin(GEN x);
GEN     gmulsg(long s, GEN y),gdivgs(GEN x, long s),gmodulo(GEN x, GEN y),gmodulcp(GEN x, GEN y),simplify(GEN x);
GEN     gmod(GEN x, GEN y),gshift(GEN x, long n),gmul2n(GEN x, long n);
GEN     gsubst(GEN x, long v, GEN y),deriv(GEN x, long v),integ(GEN x, long v),recip(GEN x),ground(GEN x),gcvtoi(GEN x, long *e),grndtoi(GEN x, long *e);
GEN     gceil(GEN x),gfloor(GEN x),gfrac(GEN x),gtrunc(GEN x),gdivent(GEN x, GEN y),gdiventres(GEN x, GEN y);
GEN     gdivmod(GEN x, GEN y, GEN *pr),geval(GEN x),glt(GEN x, GEN y),gle(GEN x, GEN y),ggt(GEN x, GEN y),gge(GEN x, GEN y),geq(GEN x, GEN y),gne(GEN x, GEN y);
GEN     gand(GEN x, GEN y),gor(GEN x, GEN y),glength(GEN x),matsize(GEN x),truecoeff(GEN x, long n),gtype(GEN x),gsettype(GEN x,long t);
GEN     gtopoly(GEN x, long v),gtopolyrev(GEN x, long v),gtoser(GEN x, long v),gtovec(GEN x),dbltor(double x);
GEN     karamul(GEN x, GEN y, long k);

void    gop0z(GEN (*f) (void), GEN x),gop1z(GEN (*f) (GEN), GEN x, GEN y),gop2z(GEN (*f) (GEN, GEN), GEN x, GEN y, GEN z),gops2gsz(GEN (*f) (GEN, long), GEN x, long s, GEN z),gops2sgz(GEN (*f) (long, GEN), long s, GEN y, GEN z),gops2ssz(GEN (*f) (long, long), long s, long y, GEN z);
void    gop3z(GEN (*f) (GEN, GEN, GEN), GEN x, GEN y, GEN z, GEN t),gops1z(GEN (*f) (long), long s, GEN y),gopsg2z(GEN (*f) (GEN, GEN), long s, GEN y, GEN z),gopgs2z(GEN (*f) (GEN, GEN), GEN y, long s, GEN z),gaffsg(long s, GEN x),gaffect(GEN x, GEN y);
void    normalize(GEN *px),normalizepol(GEN *px);

int     gcmp0(GEN x),gcmp1(GEN x),gcmp_1(GEN x),gcmp(GEN x, GEN y),lexcmp(GEN x, GEN y),gegal(GEN x, GEN y),polegal(GEN x, GEN y),gsigne(GEN x);
int     gvar(GEN x),gvar2(GEN x),tdeg(GEN x),precision(GEN x),ismonome(GEN x),iscomplex(GEN x),isexactzero(GEN g);
long opgs2(int (*f) (GEN, GEN), GEN y, long s);
long taille(GEN x),taille2(GEN x),gexpo(GEN x),gtolong(GEN x),ggval(GEN x, GEN p),rounderror(GEN x),gsize(GEN x),pvaluation(GEN x, GEN p, GEN *py);
double  rtodbl(GEN x), gtodouble(GEN x);

/* init.c */

GEN     newbloc(long n),geni(void);
GEN     allocatemem(unsigned long newsize);
long    marklist(void);
#ifdef __cplusplus
void    init(long parisize, long maxprime, void (*printvar)(long)=printvargp);
#else
void    init(long parisize, long maxprime);
#endif

void    killbloc(GEN x),newvalue(entree *ep, GEN val),killvalue(entree *ep);
#ifdef __cplusplus
extern "C" void    err(long numerr, ...);
#else
void    err(long numerr, ...);
#endif
void    recover(long listloc),changevalue(entree *ep, GEN val),allocatemoremem(unsigned long newsize);

/* polarit.c */
     
GEN     ginvmod(GEN x, GEN y),gred(GEN x),gdeuc(GEN x, GEN y),gres(GEN x, GEN y),poldivres(GEN x, GEN y, GEN *pr);
GEN     poleval(GEN x, GEN y),roots(GEN x, long l),rootslong(GEN x, long l),ggcd(GEN x, GEN y),gbezout(GEN x, GEN y, GEN *u, GEN *v),vecbezout(GEN x, GEN y),glcm(GEN x, GEN y);
GEN     polgcd(GEN x, GEN y),srgcd(GEN x, GEN y),polgcdnun(GEN x, GEN y),content(GEN x),primpart(GEN x),psres(GEN x, GEN y),factmod9(GEN f, GEN p, GEN a);
GEN     factmod(GEN f, GEN p),factmod2(GEN f, GEN p),factmod_gen(GEN f, GEN p),rootmod(GEN f, GEN p),rootmod2(GEN f, GEN p),decpol(GEN x, long klim),factor(GEN x);
GEN     factpol(GEN x, long klim),factpol2(GEN x, long klim),simplefactmod(GEN f, GEN p);
GEN     subres(GEN x, GEN y),discsr(GEN x),quadpoly(GEN x),quadgen(GEN x),quaddisc(GEN x),bezoutpol(GEN a, GEN b, GEN *u, GEN *v),polinvmod(GEN x, GEN y);
GEN     resultant2(GEN x, GEN y),polfnf(GEN a, GEN t),nfiso(GEN a, GEN b),nfincl(GEN a, GEN b);
GEN     newtonpoly(GEN x, GEN p),apprgen(GEN f, GEN a),apprgen9(GEN f, GEN a),rootpadic(GEN f, GEN p, long r),gcvtop(GEN x, GEN p, long r),factorpadic2(GEN x, GEN p, long r);
GEN     factorpadic4(GEN x, GEN p, long r),nilordpadic(GEN p,long r,GEN fx,long mf,GEN gx),Decomppadic(GEN p,long r,GEN f,long mf,GEN theta,GEN chi,GEN nu),squarefree(GEN f);
long    sturm(GEN x),sturmpart(GEN x, GEN a, GEN b);
int     poldivis(GEN x, GEN y, GEN *z),gdivise(GEN x, GEN y);
void    gredsp(GEN *px),split(long m, GEN *t, long d, long p, GEN q),split9(long m, GEN *t, long d, long p, GEN q, GEN unfq, long qqs, GEN a),splitgen(GEN m, GEN *t,long d,GEN p, GEN q);
     
/* trans.c */
     
GEN     greal(GEN x),gimag(GEN x),teich(GEN x),agm(GEN x, GEN y, long prec),palog(GEN x);
#ifdef __cplusplus
GEN     mpsqrt(GEN x),gsqrt(GEN x, long prec);
GEN     gexp(GEN x, long prec);
GEN     mplog(GEN x),glog(GEN x, long prec);
#else
GEN     mpsqrt(GEN x),gsqrt(GEN x, long prec);
GEN     gexp(GEN x, long prec);
GEN     mplog(GEN x),glog(GEN x, long prec);
#endif
GEN     mpexp1(GEN x),mpexp(GEN x);
GEN     logagm(GEN q),glogagm(GEN x, long prec);
GEN     mpsc1(GEN x, long *ptmod8),mpcos(GEN x),gcos(GEN x, long prec),mpsin(GEN x),gsin(GEN x, long prec);
GEN     mpaut(GEN x),mptan(GEN x),gtan(GEN x, long prec),mpatan(GEN x),gatan(GEN x, long prec),mpasin(GEN x),gasin(GEN x, long prec);
GEN     mpacos(GEN x),gacos(GEN x, long prec),mparg(GEN x, GEN y),mpch(GEN x),gch(GEN x, long prec),mpsh(GEN x),gsh(GEN x, long prec);
GEN     mpth(GEN x),gth(GEN x, long prec),mpath(GEN x),gath(GEN x, long prec),mpash(GEN x),gash(GEN x, long prec);
GEN     garg(GEN x, long prec),sarg(GEN x, GEN y, long prec),mppsi(GEN z),gpsi(GEN x, long prec),transc(GEN (*f) (GEN, long), GEN x, long prec),kbessel(GEN nu, GEN gx, long prec),hyperu(GEN a, GEN b, GEN gx, long prec);
GEN     cxpsi(GEN z, long prec),jbesselh(GEN n, GEN z, long prec),gzeta(GEN x, long prec);
GEN     kbessel2(GEN nu, GEN x, long prec),eint1(GEN x, long prec),gerfc(GEN x, long prec),eta(GEN x, long prec),jell(GEN x, long prec),wf2(GEN x, long prec),wf(GEN x, long prec);
GEN     incgam(GEN a, GEN x, long prec),incgam1(GEN a, GEN x, long prec),incgam2(GEN a, GEN x, long prec),incgam3(GEN a, GEN x, long prec),incgam4(GEN a, GEN x, GEN z, long prec),bernreal(long n, long prec),bernvec(long nomb);
GEN     mpach(GEN x),gach(GEN x, long prec),mpgamma(GEN x),cxgamma(GEN x, long prec),ggamma(GEN x, long prec),mpgamd(long x, long prec),ggamd(GEN x, long prec),mppi(long prec);
GEN     mpeuler(long prec),polylog(long m, GEN x, long prec),dilog(GEN x, long prec),polylogd(long m, GEN x, long prec),polylogdold(long m, GEN x, long prec),polylogp(long m, GEN x, long prec),gpolylog(long m, GEN x, long prec);
GEN     theta(GEN q, GEN z, long prec),thetanullk(GEN q, long k, long prec),mplngamma(GEN x),cxlngamma(GEN x, long prec),glngamma(GEN x, long prec),izeta(GEN x, long prec);

void    constpi(long prec),consteuler(long prec),mpbern(long nomb, long prec),gsincos(GEN x, GEN *s, GEN *c, long prec);
void    gsqrtz(GEN x, GEN y),gexpz(GEN x, GEN y),glogz(GEN x, GEN y),gcosz(GEN x, GEN y),gsinz(GEN x, GEN y),mpsincos(GEN x, GEN *s, GEN *c),gtanz(GEN x, GEN y);
void    gatanz(GEN x, GEN y),gasinz(GEN x, GEN y),gacosz(GEN x, GEN y),gchz(GEN x, GEN y),gshz(GEN x, GEN y),gthz(GEN x, GEN y),gashz(GEN x, GEN y),gachz(GEN x, GEN y);
void    gathz(GEN x, GEN y),ggammaz(GEN x, GEN y),glngammaz(GEN x, GEN y),mpgamdz(long s, GEN y),ggamdz(GEN x, GEN y),gpsiz(GEN x, GEN y),gzetaz(GEN x, GEN y);
void    gpolylogz(long m, GEN x, GEN y);

/* version.c */

GEN     gerepilc(GEN l, GEN p, GEN q);
void    printversion(void), printversionno(void);

#ifdef __cplusplus
extern "C" {
#endif
long    mulmodll(ulong a, ulong b, ulong c);
long    addll(long, long), subll(long, long), addllx(long, long), subllx(long, long);
long    shiftl(unsigned long, unsigned long), shiftlr(unsigned long, unsigned long);
long    mulll(unsigned long, unsigned long),addmul(unsigned long, unsigned long), divll(unsigned long, unsigned long);
int     bfffo(unsigned long);
#ifdef __cplusplus
}
#endif

