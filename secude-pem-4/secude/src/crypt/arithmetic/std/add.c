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
 *	L_NUMBER addition
 *
 */

#include	"arithmetic.h"

char *add_string1 = "add:";
char *add_string2 = "add: + \nadd:";
char *add_string3 = "add: = \nadd:";

/*	A + B	*/
/***************************************************************
 *
 * Procedure _add
 *
 ***************************************************************/
#ifdef ANSI

void _add(
	register L_NUMBER	 *Ap,
	register L_NUMBER	 *Bp,
	register L_NUMBER	 *Sum
)

#else

void _add(
	Ap,
	Bp,
	Sum
)
register L_NUMBER	 *Ap;
register L_NUMBER	 *Bp;
register L_NUMBER	 *Sum;

#endif

{
	register L_NUMBER *Sp;
	register int    cnt, carry = 0;
	int             residual;


#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, add_string1);
					arithmetic_trace_print(Ap); 
			 		fprintf(arithmetic_trace, add_string2);
					arithmetic_trace_print(Bp);
				} 
#endif
	if (lngofln(Ap) < lngofln(Bp))
		Sp = Bp, Bp = Ap, Ap = Sp;	/* swap operands */

	cnt = lngofln(Bp);
	residual = lngofln(Ap) - lngofln(Bp);
	Sp = Sum;
	lngofln(Sum) = lngofln(Ap);

	for (; cnt > 0; cnt--)
		carry = cadd(*++Ap, *++Bp, ++Sp, carry);

	for (; carry && (residual > 0); residual--)
		carry = cadd(*++Ap, 0, ++Sp, 1);

	if (Sp != Ap)
		for (; residual > 0; residual--)
			*++Sp = *++Ap;

	if (carry)
		(lngofln(Sum))++, Sum[lngofln(Sum)] = 1;

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, add_string3);
 					arithmetic_trace_print(Sum);
				}
#endif
	return;
}
