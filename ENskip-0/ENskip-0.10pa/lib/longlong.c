/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <sys/types.h>
#include "longlong.h"

int longlong_inc(u_char *a, int val)
{
  int i, ival;

  for (i = LONGLONG_SIZE - 1; i >= 0; i--)
  {
    ival = val & 0xff;
    val >>= 8;
    if ((ival == 0) && (val == 0))
      break;

    if ((a[i] += ival) < ival)  /* Overflow ? */
      val++;
  }

  return 0;
}

/*
 * Returns <0: a < b   0: a == b    >0: a > b
 */
int longlong_cmp(u_char *a, u_char *b)
{
  int i;


  for (i = 0; (i < LONGLONG_SIZE) && (*a == *b); i++, a++, b++)
    ;


  return *a - *b;
}
    
/*
 * Returns (a - b) if (a >= b) OR -1 if (a < b) or (a - b) too large
 */
int longlong_sub(u_char *a, u_char *b)
{
  int i, val;


  for (i = val = 0; i < LONGLONG_SIZE; i++)
  {
    if (val >= (LONGLONG_MAXSUB >> 8))
      return -1;
    val = (val << 8) + (*a++ - *b++);
  }

  return val;
}

/*
 * Returns a (has to fit in u_int, not checked)
 */
u_int longlong_val(u_char *a)
{
  int i;
  u_int val;

  for (i = val = 0; i < LONGLONG_SIZE; i++)
    val = (val << 8) + *a++;

  return val;
}
