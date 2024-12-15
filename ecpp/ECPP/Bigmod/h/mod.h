#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "BigNum.h"
#include "BigZ.h"
#include "bntobnn.h"

/*************************************************************************
				TYPES
*************************************************************************/

#define BigMod BigNum
typedef struct lbg{
    BigNum n;
    int nl;
} LBig;

/*************************************************************************
  				MACROS
*************************************************************************/

#define BzToBn(z)               ((z)->Digits)

/* Creation of a BigMod */
#define BmCreate() (BigMod)BNC(Modul+1)
#define MdCreate() (BigMod)BNC(Modul+1)

/* Free a BigNum */
#define BnnFree(n) free((n))

/* Number of digits in an unknown BigNum */
#define NumDigits(n) BnNumDigits((n), 0, BnGetSize((n)))

/* Number of Digits of a BigMod */
#define ModNumDigits(n) BnNumDigits((n), 0, Modul)
/* Number of bits in c[cd] */
#define BnNumBitsInDigit(c, cd)	(BnIsDigitZero((c), (cd)) ? 1 : BN_DIGIT_SIZE-BnNumLeadingZeroBitsInDigit((c), (cd)))

/* Number of bits in n[nd..nd+nl-1] */
#define BnNumBits(n,nd,nl) BN_DIGIT_SIZE*((nl)-1)+BnNumBitsInDigit((n), (nd)+(nl)-1)

/* Is n > 1 */
#define BnnIsGtOne(n, nl) ((nl) == 1 && BnnDoesDigitFitInWord(*(n)) && BnnGetDigit((n)) <= (BigNumDigit)1) ? 0 : 1

/* Is n == 1 */
#define BnnIsOne(n, nl) (BnnNumDigits((n), (nl)) == 1 && BnnDoesDigitFitInWord(*(n)) && BnnGetDigit((n)) == (BigNumDigit)1) ? 1 : 0

/* a[0..Ml] <- b[0..Ml].*/
#define ModAssign(a, b) BnAssign((a), 0, (b), 0, Modul)

#define ModSetToZero(a) BnSetToZero((a), 0, Modul + 1)

#define ModCompare(a, b) BnCompare((a), 0, BnNumDigits((a), 0, Modul), (b), 0, BnNumDigits((b), 0, Modul))

#define ModIsZero(a) BnIsZero((a), 0, BnNumDigits((a), 0, Modul)

#if 0
#define ModGcd(a, b) BnGcd((a), 0, Modu, 0, Modul, (b), 0, BnNumDigits((b), 0, Modul)

#define ModHasInverse(ia, a, al) `(ModInv ,ia ,a))

#define ModInv(a, b) BnEea(Gbuf, 0, (a), 0, Modu, 0, Modul, (b), 0, BnNumDigits((b), 0, Modul))

#define ModInvDigit(a, b, bd) BnEea(Gbuf, 0, (a), 0, Modu, 0, Modul, (b), bd, 1)
#endif

#define ModJacobi(a) BnJacobi((a), 0, BnNumDigits((a), 0, Modul), Modu, 0, Modul)

#define From(n, nl) BzToString(BzFromBigNum((n), (nl)), 10)

#define To(s, p_nl) BzToBigNum(BzFromString((s), 10), (p_nl))

#define Z2STR(z) BzToString((z), 10)

/*************************************************************************
			      CONSTANTS
*************************************************************************/

#define STRMAX 1000 /* maximum length of string buffer */

/*************************************************************************
			     GLOBAL VARIABLES
*************************************************************************/

extern BigNum CONSTANTES, SCRATCH, Modu, Mbuf, Ebuf, Abuf;
extern BigNum Dbuf, Gbuf, Xbuf, Ybuf, Hbuf, Pbuf, Zbuf;

extern int Modul, Mtl, Etl;
extern int debuglevel;

/*************************************************************************
			EXPORTED FUNCTIONS
*************************************************************************/

extern BigNum BNC(), BnCopy(), BnConvert(), BufUpdate(), BufUpdateL();
extern ModSetUp(), Initampons(), MdInitialize(), ModInitialize(), MdAssign();
extern BigMod MdFix();
extern int MdMultiply(), MdSquare(), BnSquare();
extern BmMultiply(), BmSquare(), BmAdd(), BmFree(), BmAdd(), BmSubtract();
extern BigNumCmp FixCompareBn(), FixJacobiBn(), FixJacobiFix(), BnJacobiFix();

extern Print();
extern char * zBToString();

/* Old */
extern int MdAdd();
