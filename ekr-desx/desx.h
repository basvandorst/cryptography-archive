/* desx.h -- include for desx.c
   Copyright (C) 1994 ekr@rtfm.com

   This software may be used, redistributed, copied, or sold
   for any purpose and without fee provided that this notice
   is maintained.
*/   

typedef unsigned long UINT4;      /*Some unsigned 32 bit int*/

typedef unsigned char UCHAR;
typedef struct {
  char ks[16][8]; /*DES key schedule*/
  UCHAR prewhite[8],postwhite[8];
} desx_context;

