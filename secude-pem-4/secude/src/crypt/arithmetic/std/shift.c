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
 *	shift(OPa,exp2b,result)
 */


#include "arithmetic.h"

char * shift_string1 = "shift:";
char * shift_string2 = "shift: >> %d\n";
char * shift_string3 = "shift: = \nshift:";
char * shift_string5 = "shift: << %d\n";

/***************************************************************
 *
 * Procedure _shift
 *
 ***************************************************************/
#ifdef ANSI

void _shift(
	L_NUMBER	  A[],
	int		  exp2b,
	L_NUMBER	  S[]
)

#else

void _shift(
	A,
	exp2b,
	S
)
L_NUMBER	  A[];
int		  exp2b;
L_NUMBER	  S[];

#endif

{
	register int    wordshift;
	register int    bitshift;
	register int    left = exp2b > 0;
	register int    right = exp2b < 0;
	register L_NUMBER cnt;
	register L_NUMBER *Ap;
	register L_NUMBER *Sp;
	register int    WLmBITS;
	register L_NUMBER dH;
	register L_NUMBER dL;

	if (right) {
#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, shift_string1);
					arithmetic_trace_print(A);
					fprintf(arithmetic_trace, shift_string2, -(exp2b));
				}
#endif
		bitshift = (-exp2b) & (WLNG - 1);
		WLmBITS = WLNG - bitshift;
		wordshift = (-exp2b) >> SWBITS;

		cnt = lngofln(A) - wordshift;
		if (cnt <= 0) {
			lngofln(S) = 0;
#ifdef ARITHMETIC_TEST
			 	if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, shift_string3);
			 		arithmetic_trace_print(S);
				}
#endif
			return;
		}
		Sp = S;
		Ap = A + wordshift;
		lngofln(S) = cnt;
		dH = dL = *++Ap;
		for (; cnt > 1; cnt--) {
			dH = *++Ap;
			if (bitshift) {
				*++Sp = (dH << WLmBITS) | (dL >> bitshift);
				dL = dH;
			} else
				*++Sp = dH;
		}

		dH >>= bitshift;
		if (dH)
			*++Sp = dH;
		else
			lngofln(S)--;

#ifdef ARITHMETIC_TEST
			 	if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, shift_string3);
			 		arithmetic_trace_print(S);
				}
#endif
		return;
	}
	if (left) {
#ifdef ARITHMETIC_TEST
				if(arithmetic_trace_counter) {
					fprintf(arithmetic_trace, shift_string1);
					arithmetic_trace_print(A);
					fprintf(arithmetic_trace, shift_string5, exp2b);
				}
#endif
		cnt = lngofln(A);
		bitshift = (exp2b) & (WLNG - 1);
		WLmBITS = WLNG - bitshift;
		wordshift = (exp2b) >> SWBITS;

		lngofln(S) = wordshift + cnt;
		Sp = S + lngofln(S);
		Ap = A + cnt;
		dH = dL = *Ap;
/* sparc arch can't handle shifs with word length !!! */
		if (bitshift) {
			if (dL >>= WLmBITS) {
				lngofln(S)++;
				*(Sp + 1) = dL;
			}
		} else
			dL = 0;

		for (; cnt > 1; cnt--) {
			dL = *--Ap;
			if (bitshift)
				*Sp-- = (dH << bitshift) | (dL >> WLmBITS);
			else
				*Sp-- = dH;
			dH = dL;
		}
		*Sp-- = dH << bitshift;

		for (; Sp > S; Sp--)
			*Sp = 0;

#ifdef ARITHMETIC_TEST
			 	if(arithmetic_trace_counter) {
 					arithmetic_trace_counter--;
					fprintf(arithmetic_trace, shift_string3);
			 		arithmetic_trace_print(S);
				}
#endif
		return;
	}
	trans(A, S);
	return;
}
