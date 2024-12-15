/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/* SecuDE extensions for MS-DOS */

#include "arithmetic.h"

/***************************************************************
 *
 * Procedure add
 *
 ***************************************************************/
#ifdef __STDC__

void add(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  R[]
)

#else

void add(
	A,
	B,
	R
)
L_NUMBER	  A[];
L_NUMBER	  B[];
L_NUMBER	  R[];

#endif

{
        A[0] <<= 1;
        if (B != A) B[0] <<= 1;

        ADD(A,B,R);

        A[0] >>= 1;
        if (B != A) B[0] >>= 1;
        if ((R != A) && (R != B)) R[0] >>= 1;
}


/***************************************************************
 *
 * Procedure sub
 *
 ***************************************************************/
#ifdef __STDC__

void sub(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  R[]
)

#else

void sub(
	A,
	B,
	R
)
L_NUMBER	  A[];
L_NUMBER	  B[];
L_NUMBER	  R[];

#endif

{
        A[0] <<= 1;
        if (B != A) B[0] <<= 1;

        SUB(A,B,R);

        A[0] >>= 1;
        if (B != A) B[0] >>= 1;
        if ((R != A) && (R != B)) R[0] >>= 1;

}

/***************************************************************
 *
 * Procedure mult
 *
 ***************************************************************/
#ifdef __STDC__

void mult(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  R[]
)

#else

void mult(
	A,
	B,
	R
)
L_NUMBER	  A[];
L_NUMBER	  B[];
L_NUMBER	  R[];

#endif

{
        A[0] <<= 1;
        if (B != A) B[0] <<= 1;

        MULT(A,B,R);

        A[0] >>= 1;
        if (B != A) B[0] >>= 1;
        if ((R != A) && (R != B)) R[0] >>= 1;
}


/***************************************************************
 *
 * Procedure div
 *
 ***************************************************************/
#ifdef __STDC__

void div(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  Q[],
	L_NUMBER	  R[]
)

#else

void div(
	A,
	B,
	Q,
	R
)
L_NUMBER	  A[];
L_NUMBER	  B[];
L_NUMBER	  Q[];
L_NUMBER	  R[];

#endif

{
        A[0] <<= 1;
        if (B != A) B[0] <<= 1;

        DIV(A,B,Q,R);

        A[0] >>= 1;
        if (B != A) B[0] >>= 1;
        if ((Q != A) && (Q != B)) Q[0] >>= 1;
        if ((R != A) && (R != B) && (R != Q)) R[0] >>= 1;
}

/*
void    shift( A, B, R )
L_NUMBER        A[], R[];
int             B;
{
        A[0] <<= 1;

        SHIFT(A,B,R);

        A[0] >>= 1;
        if (R != A) R[0] >>= 1;
}

*/
