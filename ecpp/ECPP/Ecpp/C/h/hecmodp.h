
/* $Log:	hecmodp.h,v $
 * Revision 1.1  91/09/20  15:01:05  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:33:02  morain
 * Initial revision
 *  */

/**********************************************************************
                                 ECM
**********************************************************************/

struct curve{
    BigMod a, b;
};

typedef struct curve Ecm;

struct pt{
    BigMod x, y, z;
};

typedef struct pt ptt;
typedef ptt *Point;

#define PointAlloc() ((Point)malloc(sizeof(ptt)))

#define EcmIsZero(pt) (BnnIsZero((pt)->z, Modul))

#define EcmEqual(P, Q) ((!ModCompare(P->x, Q->x))\
			&& (!ModCompare(P->y, Q->y))\
			&& (P->z == Q->z))

#define ECMEXPLIMIT 100

extern Point PointCreate(), PointCopy();
extern PointFree(), EcmFree(), EcmComputeBound();
