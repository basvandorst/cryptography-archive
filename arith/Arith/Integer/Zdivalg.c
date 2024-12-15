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

/* The file @(#)Zdivalg.c 1.1, created on 3/29/90, contains
 * the functions which implement the division algorithm for the Arithmetic
 * of integers of arbitrary length.
 */

#include <stdio.h>
#include "integer.h"
#include "storage.h"

extern warehouse *Z_tmp_WH;

#define LEAD(a) (*((a) + *(a) - 1))
#define NEXT(a) (*((a) + *(a) - 2))
#define HALFLENGTH 8
#define HALFSHORT 256

/* forward definitions */
unsigned short *shortdiv();
unsigned short *longdiv();

/* HALFSHIFT() and HALFSHIFTBACK() either multiply or divide a very
 * long integer by HALFSHORT = 256 = 65536/2.
 */

unsigned short *
halfshift(a)
unsigned short *a;
{
  register int i;
  unsigned short *b;
  register unsigned short carry = 0;

  b = capture(Z_tmp_WH, *a + 1);
  *b = *a + 1;
  for (i = 1; i <= *a; ++i) {
    *(b + i) = carry + ( *(a + i) << HALFLENGTH);
    carry = *(a + i) >> HALFLENGTH;
  }
  while (LEAD(b) == 0)
    --(*b);
  return(b);
}

unsigned short *
halfshiftback(a)
unsigned short *a;
{
  register int i;
  unsigned short *b;
  register unsigned short carry = 0;

  b = capture(Z_tmp_WH, *a);
  *b = *a;
  for (i = *a; i > 0; --i) {
    *(b + i) = carry + ( *(a + i) >> HALFLENGTH);
    carry = *(a + i) << HALFLENGTH;
  }
  *b = Size(b);
  return(b);
}

/* ZDIVALG()
 * controls the division algorithm. Its main function is to make sure 
 * (i) that signs get handled coherently---the remainder has the same
 * sign as the dividend and the quotient then satisfies a = b q + r,
 * and (ii) sorting out the special case when the dividend has fewer
 * digits or when dividing by a single digit number. Divalg also 
 * pre-proceses the inputs so that the leading coefficient of the divisor
 * is bigger than half the base (in our case 65536). Notice that the
 * remainder can be ignored by calling divalg with ptr=NULL.
 */

unsigned short *
Zdivalg(a, b, ptr)
unsigned short *a, *b;
unsigned short *ptr;
{
  unsigned short *u, *v, *q;
  short asign = 1;
  short bsign = 1;
  bool halfflag = FALSE;

  if (*b == 0)
    bailout("Divalg: division by zero");
  else if (*a < *b) {                  /* fast special case */
    if (ptr != NULL) {
      Shcopy(a, ptr);
    }
    q = capture(Z_tmp_WH, 0);
    *q = 0;

  } else {
    v = b;
    if (SIGNUM(v) == NEGSIGNBIT) {
      bsign = -1;
      v = Zminus(v);
    }
    if (LEAD(v) < HALFSHORT) {
      halfflag = TRUE;
      v = halfshift(v);
    }

    if (SIGNUM(a) == NEGSIGNBIT) {
      asign = -1;
      u = Zminus(a);
      (void) capture(Z_tmp_WH, 0);       /* to allow an extra bit */
    } else {
      u = capture(Z_tmp_WH, *a + 1);
      Shcopy(a, u);
    }
    if (halfflag == TRUE)
      u = halfshift(u);

    if (*b == 2)
      q = shortdiv(u, LEAD(v), ptr);
    else
      q = longdiv(u, v, ptr);

    if (asign * bsign == -1)
      q = Zminus(q);
    if (ptr != NULL) {
      if (halfflag == TRUE)
	Shcopy(halfshiftback(ptr), ptr);
      if (asign == -1)
	Shcopy(Zminus(ptr), ptr);
    }
  }
  *q = Size(q);
  return(q);
}


/* SHORTDIV()
 * divides a very long integer a by a single digit integer b.
 * The quotient is returned, and the remainder is stored in ptr.
 * It assumes that both a and b are positive.
 */

unsigned short *
shortdiv(a, b, ptr)
unsigned short *a, b, *ptr;
{
  unsigned short *q;
  register int i;
  register unsigned long twofer;
  register unsigned long leftover = 0;

  if (SIGNUM(a) != 0)
    bailout("Shortdiv: non-positive dividend.");
  q = capture(Z_tmp_WH, *a);
  *q = *a;
  for (i = *a - 1; i > 0; --i) {
    twofer = (leftover << SHORTLENGTH) + *(a + i);
    *(q + i) = (twofer / b);
    leftover = twofer % b;
  }
  if (ptr != NULL)
    Shcopy(Constant(leftover), ptr);
  *q = Size(q);
  return(q);
}

/* These variables are used repeatedly when guessing trial digits
 * of the quotient.
 */

static unsigned long leadv;
static unsigned long nextv;

/* QTRIAL()
 * makes a reasonable guess at the next digit of the quotient. At worst,
 * it is only one larger than it should be.
 */

unsigned short
qtrial(u, k)
unsigned short *u;
unsigned k;
{
  unsigned long qhat;           /* trial digit */
  unsigned long twofer;         /* two digits of u */
  unsigned long left, right;    /* for quick test of trial digit */

  twofer = (*(u+k) << SHORTLENGTH) + *(u+k-1);
  if (*(u + k) == leadv)
    qhat = 1 << SHORTLENGTH;
  else
    qhat = (twofer/leadv) + 1;
  do {
    --qhat;
    left = nextv * qhat;
    if (twofer - qhat*leadv > NEGSIGNBIT)
      right = left + 1;
    else
      right =  ((twofer - qhat*leadv) << SHORTLENGTH) + *(u+k-2);
  } while (left > right);
  return((unsigned short)qhat);
}

/* MULTSUB()
 * replaces u with  u - ((v * s) shifted).
 */

bool
multsub(u, v, s, shift)
unsigned short *u, *v;
unsigned short s;
unsigned shift;
{
  register int i;
  register unsigned long remove;
  register unsigned long little;
  register unsigned long borrow;
  register unsigned long big = 0;

  for (i = 1; i <= *v; ++i) {
    remove = *(v + i) * s + big;
    little = remove & SHORTMASK;
    borrow = (little > *(u + i + shift));
    big = (remove >> SHORTLENGTH) + borrow;
    *(u + i + shift) = ((borrow << SHORTLENGTH) + *(u + i + shift)) - little;
  }
  return((bool)borrow);
}

/* LONGDIV()
 * is the main routine in actually doing long division of very long
 * integers. It assumes that the dividend u and the divisor v are
 * both strictly positive integers, and that the lead coefficient
 * of v is larger than half the base. It assumes further that it is 
 * safe to monkey around with u and change the data stored there.
 * It returns the quotient and leaves the remainder in u, and fixes
 * the pointer so it points to u.
 */

unsigned short *
longdiv(u, v, ptr)
unsigned short *u, *v, *ptr;
{
  unsigned short *q;              /* the quotient */
  register unsigned k;            /* the digit of q being computed */
  register unsigned short qhat;   /* trial digit of quotient */

  if (*u < 3 || SIGNUM(v) == NEGSIGNBIT || SIGNUM(u) == NEGSIGNBIT)
    bailout("Longdiv: faulty preprocessing of data.");
  leadv = LEAD(v);
  nextv = NEXT(v);
  q = capture(Z_tmp_WH, *u - *v + 2);
  *q = *u - *v + 2;
  SIGNUM(q) = 0;
  for (k = *q - 1; k > 0; --k) {
    qhat = qtrial(u, k + *v - 1);
    if (multsub(u, v, qhat, k - 1) == YES) {
      (void) fprintf(stderr, "Special Case\n");
      --qhat;
      Zshortmult(v, 1, u, k - 1);
    }
    *(q + k) = qhat;
  }
  if (ptr != NULL) {
    *u = Size(u);
    Shcopy(u, ptr);
  }
  return(q);
}
