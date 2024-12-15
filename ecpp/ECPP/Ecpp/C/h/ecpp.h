
/* $Log:	ecpp.h,v $
 * Revision 1.1  91/09/20  15:01:01  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:32:58  morain
 * Initial revision
 *  */

/**********************************************************************
                             Includes for ecpp
**********************************************************************/

#include "ifactor.h"
#include "pratt.h"

#ifdef DISTRIB
#include "distrib.h"
#endif

/********** Constantes **********/

#define ECPPSTRMAX      2000    /* maximal number of chars read for a BigNum */
#define PROFMAX 	100	/* maximum number of iterations */
#define RGDMIN	        0       /* minimum rank for discriminants */
#define RGDMAX 1685/* number of DfD in DFD */
#define MAXSTORED	836	/* number of equations in POLYJ */
#define HMAX            100
#define GMAX            128
#define SUPERPMAX	1999000	/* pmax in DIFNP */
#define SPMAX		1000	/* 32749 */
#define PMIN		0
#define NBNP		150000	/* number of p <= PMAX */
#define HACK_SIEVE_MIN  0       /* for fastsieve */
#define TFMAX           20      /* maximal number of factors */

/* Contains d_i=(p_(i+1) - p_i)/2 starting from d_1=(5-3)/2 */
/* Used in the sieve and in pollard */
#define DIFNP   "Data/difnp5e6.data"
#define NAMEDFD	"Data/ecpp.DfD"
#define JPATH	"Data/Weber"
#define FSPMAX  "Data/spmax.data"

/********** Structures **********/

struct listlong{
    long lg;
    struct listlong *cdr;
};

typedef struct listlong LL;
typedef LL *ListOfLong;
#define ListOfLongAlloc() ((ListOfLong)malloc(sizeof(LL)))

/* Lists of BigZ */
struct listz{
    BigZ z;
    struct listz *cdr;
};

typedef struct listz LLZ;
typedef LLZ *ListOfBigZ;
#define ListOfBigZAlloc() ((ListOfBigZ)malloc(sizeof(LLZ)))

/* 4 p = A^2 + D B^2 => 4 m = (A-2)^2 + D B^2 */
/* if size(p)=l, then A and B are of size [l/2]+2, m is of size l+1 */
struct list_of_decomp{
    BigZ A, B;
    BigNum m;
    ListOfFactors lfact;
    struct list_of_decomp *cdr;
};

typedef struct list_of_decomp LOD;
typedef LOD *ListOfABm;
#define ListOfABmAlloc() ((ListOfABm)malloc(sizeof(LOD)))

/* What is just needed for a certificate */
struct record_certif{
    BigNum p; int pl;
    long D;
    BigNum sqrtd;
    int h, g;
    BigZ A, B;
    BigNum m; int ml;
    ListOfFactors lfact, lfacto; /* factors of m (resp. order of P) */
    BigNum ea, eb, px, py;
    int eal, ebl, pxl, pyl;
};

typedef struct record_certif Certif;

typedef struct certifparam{
    int print;
    char label[50];
    char file[50];
    char format[20];
    char type[2];
} certif_parametres;

/* Contains a discriminant D, followed by a list of AB, m, lm*/
struct DlAB{
    long D;
    int h, g;
    ListOfABm ABm;
};

typedef struct DlAB ListOfDAB;
/*#define ListOfDABAlloc() ((ListOfDAB)malloc(sizeof(DlAB)))*/

/* right place for this ? */
extern ListOfLongFree();

/********** Global variables **********/

int rgd[PROFMAX];
int depth, rgd0;
BigNum pmin, Sievepmax;
BigNum tshanksz[PROFMAX]; /* the z needed for extracting roots */
LBig tfm[TFMAX], tfp[TFMAX];
int pminl;
BigNumDigit resp[NBNP];
char bobn[5], eobn[5], bol[5], eol[5], path[200];

extern int verbose, noproof, ibase, obase, cyclic, jumpj, nonminus1, nonplus1;
extern char difnp[200], dfd[200];
