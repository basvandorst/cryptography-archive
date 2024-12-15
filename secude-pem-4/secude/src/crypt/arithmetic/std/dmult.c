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

#include "arithmetic.h"
#include "double.h"

char * dmult_string = "dmult: %x * %x = %x (HIGH), %x (LOW) \n";

/***************************************************************
 *
 * Procedure _dmult
 *
 ***************************************************************/
#ifdef ANSI

void _dmult(
	L_NUMBER	  a,
	L_NUMBER	  b,
	L_NUMBER	 *high,
	L_NUMBER	 *low
)

#else

void _dmult(
	a,
	b,
	high,
	low
)
L_NUMBER	  a;
L_NUMBER	  b;
L_NUMBER	 *high;
L_NUMBER	 *low;

#endif

{
	/* split words input parm */
	Word            A, B;

	/* split words of DMULTiplikation */
	Word            DMULT, DMULT_0, DMULT_16, DMULT_32;

	/* move parameter */
	W(A) = a;
	W(B) = b;

	/* 1. product */
	W(DMULT) = LSW(A) * LSW(B);
	W(DMULT_0) = LSW(DMULT);
	W(DMULT_16) = HSW(DMULT);
	W(DMULT_32) = 0;

	/* 2. product, shift 16 */
	HSW(DMULT_32) = _cadd(LSW(A) * HSW(B),
			     HSW(A) * LSW(B),
			     &W(DMULT), 0
		);
	HSW(DMULT_32) += _cadd(W(DMULT), W(DMULT_16), &W(DMULT), 0);

	HSW(DMULT_0) = LSW(DMULT);
	LSW(DMULT_32) = HSW(DMULT);

	/* 3. product, shift 32 */
	W(DMULT_32) += HSW(A) * HSW(B);

	/* result is catenation of DMULT_32|DMULT_0 */
	*high = W(DMULT_32);
	*low = W(DMULT_0);

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, dmult_string, a, b, *high, *low);
				}
#endif
	return;
}
