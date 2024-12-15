/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/


/*
 *	division for LNUMBERs
 *
 *	div(A,B,Q,R)
 *
 *	exceptions for zero divide
 *	benoetigt dw division cdiv(dw,d,q,r)
 */

#include	"arithmetic.h"


char *div_string1 = "div:";
char *div_string2 = "div: / \ndiv:";
char *div_string3 = "div: = \ndiv:";
char *div_string4 = "div: Re \ndiv:";


/***************************************************************
 *
 * Procedure _div
 *
 ***************************************************************/
#ifdef ANSI

void _div(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  Q[],
	L_NUMBER	  R[]
)

#else

void _div(
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
	L_NUMBER        runA[MAXGENL];	/* acc for the R */
	L_NUMBER        local[MAXGENL];	/* prepare result */
	register L_NUMBER w, *Qp = Q;
	register int    i;


#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, div_string1);
					arithmetic_trace_print(A); 
					fprintf(arithmetic_trace, div_string2);
					arithmetic_trace_print(B);
				} 
#endif
	if (!*B)
		ALU_exception(*B);	/* zero divide */

	i = lngtouse(A) - lngtouse(B);
	if (i < 0) {
		trans(A, R);
		if (Q != R)
			inttoln(0, Q);
#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, div_string3);
 					arithmetic_trace_print(Q);
					fprintf(arithmetic_trace, div_string4);
 					arithmetic_trace_print(R);
				}
#endif
		return;
	}
	trans(A, runA);
	shift(B, i, local);
	lngofln(Qp) = i / WLNG + 1;
	w = 1 << (i % WLNG);
	Qp += lngofln(Qp);
	for (; Qp > Q; Qp--) {
		*Qp = 0;
		for (; w; w >>= 1) {
			if (comp(runA, local) >= 0) {
				*Qp |= w;
				sub(runA, local, runA);
			}
			shift(local, R1, local);
		}
		w = HSBIT;
	}

	normalize(Q);
	trans(runA, R);

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, div_string3);
 					arithmetic_trace_print(Q);
					fprintf(arithmetic_trace, div_string4);
 					arithmetic_trace_print(R);
				}
#endif
	return;
}
