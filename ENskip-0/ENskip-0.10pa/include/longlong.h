/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#define LONGLONG_SIZE          8 /* Bytes per longlong */
#define LONGLONG_MAXSUB 0x7fffff /* Maximum value calculated by longlong_sub */

extern int longlong_inc(u_char *a, int val);
extern int longlong_cmp(u_char *a, u_char *b);
extern int longlong_sub(u_char *a, u_char *b);
extern u_int longlong_val(u_char *a);
