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
 *	L_NUMBER subtraction

 *
 */

#include	"arithmetic.h"

char * sub_string1 = "sub:";
char * sub_string2 = "sub: - \nsub:";
char * sub_string3 = "sub: = \nsub:";

/*	A - B	*/
/***************************************************************
 *
 * Procedure _sub
 *
 ***************************************************************/
#ifdef ANSI

void _sub(
	register L_NUMBER	 *Ap,
	register L_NUMBER	 *Bp,
	register L_NUMBER	 *Diff
)

#else

void _sub(
	Ap,
	Bp,
	Diff
)
register L_NUMBER	 *Ap;
register L_NUMBER	 *Bp;
register L_NUMBER	 *Diff;

#endif

{
	register L_NUMBER *opA = Ap, *Dp = Diff;
	register int    cnt, carry = 1;
	int             residual;

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, sub_string1);
					arithmetic_trace_print(Ap); 
			 		fprintf(arithmetic_trace, sub_string2);
					arithmetic_trace_print(Bp);
				} 
#endif
	if (lngofln(Ap) < lngofln(Bp))
		ALU_exception(carry);

	cnt = lngofln(Bp);
	residual = lngofln(Ap) - lngofln(Bp);

	for (; cnt > 0; cnt--)
		carry = cadd(*++Ap, ~(*++Bp), ++Diff, carry);

	for (; !carry && (residual > 0); residual--)
		carry = cadd(*++Ap, ~(0), ++Diff, 0);

	if (Diff != Ap)
		for (; residual > 0; residual--)
			*++Diff = *++Ap;

	if (carry)		/* OK */
		;
	else
		ALU_exception(carry);

	cnt = lngofln(opA);
	Diff = Dp + cnt;
	for (; (Diff > Dp) && !*Diff; Diff--)
		cnt--;

	lngofln(Dp) = cnt;

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, sub_string3);
 					arithmetic_trace_print(Dp);
				}
#endif

	return;
}
