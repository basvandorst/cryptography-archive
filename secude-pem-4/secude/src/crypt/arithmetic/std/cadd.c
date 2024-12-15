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

/*	addition/subtraktion mit carry
 *
 *	cadd(A,B,A+B,carry) RETURNS carry
 */

#include	"arithmetic.h"

char *cadd_string = "cadd: %x + %x + %x(CARRY)= %x (CARRY), %x (LOW) \n";

/***************************************************************
 *
 * Procedure _cadd
 *
 ***************************************************************/
#ifdef ANSI

int _cadd(
	L_NUMBER		  opa,
	L_NUMBER		  opb,
	register L_NUMBER	 *opa_b,
	register L_NUMBER	  carry
)

#else

int _cadd(
	opa,
	opb,
	opa_b,
	carry
)
L_NUMBER		  opa;
L_NUMBER		  opb;
register L_NUMBER	 *opa_b;
register L_NUMBER	  carry;

#endif

{
	register L_NUMBER zulu = opb + carry;
	register L_NUMBER sum;

	/*
	 * NOTE: optimized to achieve best result for subtraktion of small
	 * int
	 */
	if (zulu) {		/* means no carry at all */
		sum = opa + zulu;
		zulu |= opa;
		*opa_b = sum;
#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace,  cadd_string, opa, opb, carry, zulu > sum, *opa_b);
				}
#endif
		return (zulu > sum);
	} else {		/* propagate carry */
		*opa_b = opa;
#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, cadd_string, opa, opb, carry, carry, *opa_b);
				}
#endif
		return carry;
	}
}
