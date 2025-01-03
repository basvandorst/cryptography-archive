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

/*	L_NUMBER multiplikation
 *
 *	mult(A,B,Produkt)
 *
 *	Achtung: Produkt muss gross genug sein > len(A) + len(B)
 *	Es wird benoetigt eine Funktion cmult(i,j,i_j), die
 *	zwei integer multipliziert und ein doppel integer liefert.
 */

#include	"arithmetic.h"
#include	"optimize.h"

#define	Max(a,b)	( a>b? a:b )
#define	Min(a,b)	( a>b? b:a )


char * mult_string1 = "mult:";
char * mult_string2 = "mult: * \nmult:";
char * mult_string3 = "mult: = \nmult:";

/***************************************************************
 *
 * Procedure _mult
 *
 ***************************************************************/
#ifdef ANSI

void _mult(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  erg[]
)

#else

void _mult(
	A,
	B,
	erg
)
L_NUMBER	  A[];
L_NUMBER	  B[];
L_NUMBER	  erg[];

#endif

{
	register int    i, j;
	L_NUMBER        Prod[2 * MAXGENL];
	register L_NUMBER *Ap, *Bp, *Cp = Prod;
	register int    lenA = lngofln(A);
	register int    lenB = lngofln(B);
	register int    lenC = lenA + lenB;
	register int    carry;

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, mult_string1);
					arithmetic_trace_print(A); 
			 		fprintf(arithmetic_trace, mult_string2);
					arithmetic_trace_print(B);
				} 
#endif
	if (!(lenA && lenB)) {
		*erg = 0;
		return;
	}
	*Cp++ = 0;		/* init 0 */
	*Cp = carry = 0;	/* clear first words */

	for (i = 1; i < lenC; i++, Cp++) {	/* run through result */
		*(Cp + 1) = carry;
		j = Max(0, i - lenA);
		Bp = B + j;
		Ap = A + i - j;
		carry = 0;	/* reset carry */
		for (; (j < i) && (j < lenB); j++) {
			REGISTER L_NUMBER dlow;
			REGISTER L_NUMBER dhigh;

			/* multiply A[i-j] * B[j] */
			dmult(*Ap--, *++Bp, ADDR dhigh, ADDR dlow);
			/* ... and add to current sum */
			carry += cadd(*(Cp + 1), dhigh, Cp + 1,
				      cadd(*Cp, dlow, Cp, 0)
				);
		}
	}

	if (!*Cp)
		Cp--;
	lngofln(Prod) = Cp - Prod;
	trans(Prod, erg);

#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, mult_string3);
 					arithmetic_trace_print(erg);
				}
#endif
	return;
}
