/*
 * Arithmetic in Global Fields
 * Copyright (C) 1990 Kevin R. Coombes and David R. Grant
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*  @(#)collect.c	1.1  3/29/90  */

#include <stdio.h>
#include "normal.h"
#include "Imacro.h"
#include "Pmacro.h"
#include "myinput.h"

extern void bailout();

void flush_input();

#define MAXDIGIT 1024

/* COLLECT()
 * actually collects the very long integer or polynomial to be stored
 * in an array of unsigned shorts located at the pointer a.
 *     Integers are entered  with an optional sign followed by a string
 * of digits in big-endian order. They are stored in two's complement
 * notation in the smallest amount of necessary space. The length of
 * the integer is stored at *a, the units digit at *(a+1), and the
 * signbit at *(a + *a).  All digits are collected; all nondigits except
 * signs and the controls ^A or ^R are ignored. Note that numbers are
 * written in base 65536, the size of an unsigned short.
 *     Polynomials are entered in big-endian and stored in little-endian
 * order, with the size (=degree+1) stored at *a and the zeroth coefficient 
 * at *(a+1). Notice that any entry other than a digit or ^A or ^R
 * is treated as whitespace, and that coefficients are separated by
 *  whitespace.
 */

bool
collect(s, t, a, step, with_sign)
char *s, *t;             /* two-part description of input */
unsigned short *a;       /* where to store the input - not my problem */
short step;              /* backward communication with calling routine */
bool with_sign;          /* are we expecting a signed integer ? */
{
  register int c;        /* next character */
  register short i;      /* loop variable */
  short numdigit;        /* number of digits read in so far */
  bool indigit;          /* are we currently in the middle of a digit ? */
  register unsigned long digit;    /* value of the digit being read */
  unsigned short hold[MAXDIGIT];   /* temporary storage of a very long */
  bool cont;             /* is this number continued on the next line ? */
  bool sign;             /* what sign is the number we're reading ? */

  if (printf("%s %s: ", s, t) == EOF)
    bailout("Collect: unable to print messages.");

  digit = 0;
  numdigit = 0;
  cont = indigit = NO;
  sign = POS;
  for (i = 0; i < MAXDIGIT; ++i)
    hold[i] = 0;

  for (;;) {
    c = getchar();
    switch (c) {
    case EOF:
      bailout("Collect: End of file encountered.");
    case '&':
      cont = YES;
      break;
    case '-':
      if (with_sign == TRUE) {
	if (sign == NEG) {
	  if (printf("Illegal sign entry. Reenter:\n") == EOF)
	    bailout("Collect: cannot print.");
	  flush_input();
	  return step;
	}
	sign = NEG;
      }
      break;
    case '+':
      if (with_sign == TRUE && sign == NEG) {
	if (printf("Illegal sign entry. Reenter:\n") == EOF)
	  bailout("Collect: cannot print.");
	flush_input();
	return step;
      }
      break;
    case '\n':
      if (cont == YES)
	cont = NO;
      else {                         /* finish up */
	if (indigit == YES) {
	  if (with_sign == TRUE)
	    hold[numdigit] = digit;
	  else
	    hold[numdigit] = MOD(digit);
	}

	if (numdigit < 1)
	  return step;

	for (i = 1; i <= numdigit && hold[i] == 0; ++i)
	  ;                        /* ignore high order 0's */
	if (i > numdigit) {
	  *a = 0;
	  return step + 1;
	}
	if (with_sign == TRUE) {    /* get size and sign correct */
	  *a = numdigit - i + 2;
	  if (sign == NEG)
	    SIGNUM(a) = NEGSIGNBIT;
	  else
	    SIGNUM(a) = 0;
	} else {
	  *a = numdigit - i + 1;
	}
	if (with_sign == TRUE && sign == NEG) {  /* store the number */
	  for ( ; i <= numdigit; ++i)
	    *(a + (numdigit - i + 1)) = ~hold[i];
	  for (i = 1; *(a + i) == NEGSIGNBIT; ++i)
	    *(a+i) = 0;
	  *(a+i) += 1;
	} else {
	  for ( ; i <= numdigit; ++i)
	    *(a + (numdigit - i + 1)) = hold[i];
	}
	return step + 1;
      }
      break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      if (indigit == YES) {
	digit = digit * 10 + c - '0';
	if (digit > NEGSIGNBIT)
	  bailout("Collect: digit exceeds size of short.");
      } else {
	indigit = YES;
	digit = c - '0';
	++numdigit;
	if (numdigit >= MAXDIGIT) {     /* flush extraneous input */
	  flush_input();
	  bailout("Collect: Degree exceeds MAXDIGIT.");
	}
      }
      break;
    case '\022':
      flush_input();
      return REENTER;
    case '\001':
      flush_input();
      return CANCEL;
    default:
      if (indigit == YES) {
	if (with_sign == TRUE)
	  hold[numdigit] = digit;
	else
	  hold[numdigit] = MOD(digit);
	indigit = NO;
      }
      break;
    }
  }
}

void
flush_input()
{
  bool cont = NO;

  for (;;)
    switch (getchar()) {
    case EOF:
      bailout("FlushInput: end of file encountered.");
    case '&':
      cont = YES;
      break;
    case '\n':
      if (cont == NO) return;
      cont = NO;
      break;
    default:
      break;
    }
}
