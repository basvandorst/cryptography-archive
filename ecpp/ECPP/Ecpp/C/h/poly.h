
/* $Log:	poly.h,v $
 * Revision 1.1  91/09/20  15:01:11  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:33:10  morain
 * Initial revision
 *  */

/**********************************************************************
                      Data structures for polynomials
**********************************************************************/

#define DEGMAX 40

struct pol{
    int deg, length;
    BigMod coeff[DEGMAX];
};

typedef struct pol polynom;
typedef polynom *Poly;

#define PolyAlloc() ((Poly)malloc(sizeof(polynom)))

extern Poly TP, PolyCreate(), PolyCopy();
extern PolyFree(), PolyInit(), PolyClose(), PolyPrint();
