/*
 *   UNIXCRPT.C
 *
 *   UNIX Crypt() Implementation
 *   Programmed by Mark Saarinen
 *
 *   If anyone is interested in understanding how the UNIX Crypt() really
works, *   I'm posting a 'minium', stripped down version of the Crypt()
algorithm.
 *
 *   Personally, I think that the UNIX operating systems should have the
Crypt() *   implemented like this, with no optimizations (and very little
memory
 *   consumption). Fast Crypt()'s are only needed to crack passwords. Do you
 *   want _your_ users cracking passwords? :)
 *
 *   Note that the only change to the DES algorithm is the the entries in the E
 *   (expansion?) table are swapped according to the salt.
 *
 *   This source is available in Pascal too.
 *
 *   --------------------------------------------------------------------------
 *   Soon:
 *
 *   ZenKrack (5.5 megabyte buffers and assembly-language optimizations)
 *   in a FTP-site near you..
 *   --------------------------------------------------------------------------
 *
 *   Mark Saarinen
 *   email: freenet.hut.fi
 *
 */

#include <string.h>

/*
 *  asc_bin_conv() macro translates a ascii character to
 *  a 6-bit binary number. Used to translate salts.
 */

#define asc_bin_conv(x) x >= 'a' ? x - 59 : (x >= 'A' ? x - 53 : x - 46 )

/*  output_conv[] table is used for the reverse translation */

const char output_conv[] =
  "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/*
 *  These tables are a part of the original DES specification
 */

/* Key permutation PC1 */

const unsigned char PC1[56] =
{ 57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
  10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
  63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
  14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4};

/* Key schedule of left shifts LS */

const unsigned char LS[16] =
{  1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

/* Key permutation PC2 */

const unsigned char PC2[56] =
{ 14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
  23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
  41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
  44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32};

/* Bit selection table E */

const unsigned char SE[48] =
{ 32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
   8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
  16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
  24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1};

/* Permutation P */

const unsigned char P[32] =
{ 16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
   2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25};

/* Selection functions ('S-boxes') */

const unsigned char S[8][4][16] =
{ { { 14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7 },
    {  0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8 },
    {  4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0 },
    { 15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13 } },

  { { 15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10 },
    {  3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5 },
    {  0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15 },
    { 13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9 } },

  { { 10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8 },
    { 13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1 },
    { 13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7 },
    {  1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12 } },

  { {  7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15 },
    { 13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9 },
    { 10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4 },
    {  3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14 } },

  { {  2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9 },
    { 14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6 },
    {  4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14 },
    { 11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 } },

  { { 12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11 },
    { 10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8 },
    {  9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6 },
    {  4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 } },

  { {  4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1 },
    { 13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6 },
    {  1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2 },
    {  6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 } },

  { { 13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7 },
    {  1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2 },
    {  7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8 },
    {  2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 } }
};

/* Final permutation IP^-1 */

const unsigned char FP[64] =
{ 40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
  38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
  36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
  34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25};

/* 64-bit bitvector type */

typedef unsigned char vec64[64];

/* return buffer */

static char crypt_return[16];

/* interfaced as the UNIX crypt() */

char *crypt (char *pw, char *salt)
{
  int i,j,s,t,cs,l,sb,p1,p2;
  char *ret;
  vec64 B,K,T,T2;
  unsigned char KP[16][48];
  unsigned char E[48];

  /* Convert pw into a bitvector */

  memset (K, 0, 64);

  for( i = 0; (i < 64) && (*pw != '\0'); pw++ )
  {
    K[i++] = (*pw >> 6) & 1;
    K[i++] = (*pw >> 5) & 1;
    K[i++] = (*pw >> 4) & 1;
    K[i++] = (*pw >> 3) & 1;
    K[i++] = (*pw >> 2) & 1;
    K[i++] = (*pw >> 1) & 1;
    K[i++] = *pw & 1;
    i++;
  }

 /*
  * combine PC1, LS and PC2 - tables into one for key generation
  * KP[i][j] is used to generate the iteration key K_i
  */

  cs = 0;

  for (i = 0; i < 16; i++)
  {
    cs += LS[i];
    for (j = 0; j < 48; j++)
    {
      l = PC2[j] - 1;
      l = (l < 28) ? (l+cs) % 28 : (l+cs) % 28 + 28;
      KP[i][j] = PC1[l]-1;
    }
  }

  /* copy salt to the beginning of the return buffer */

  ret = crypt_return;
  *ret++ = salt[0];
  *ret++ = salt[1];

  /*
   * permutate the E table according to the saltvalue
   * this is the only actual change in the DES algorithm!
   */

  s = ((asc_bin_conv(salt[1])) << 6) | (asc_bin_conv(salt[0]));

  memcpy(E, SE, 48);

  for( i = 0; i < 12; i++)
    if( (s >> i) & 1 != 0)
    {
      t = E[i];
      E[i] = E[i+24];
      E[i+24] = t;
    }

  memset (T, 0, 64);            /* all zero : no need for IP */

  for (cs = 0; cs < 25; cs++)   /* DES is called 25 times */
    for (i = 0; i < 16; i++)    /* 16 DES iterations */
    {
       /* do PC1-LS-PC2 key-shuffle and xor with the E-expanded left side */

       for (j = 0; j < 48; j++)
         B[j] = T[E[j]+31] ^ K[KP[i][j]];

       /* do sbox - substitution*/

       p1 = 0;
       p2 = 3;

       for (j = 0; j < 8; j++)
       {
         sb = S[j][(B[p1] << 1) | B[p1+5]]
            [(B[1+p1] << 3) | (B[2+p1] << 2) | (B[3+p1] << 1) | B[4+p1]];

          for (l = 0; l < 4; l++)
            T2[p2-l] = (sb >> l) & 1;

          p1 += 6;
          p2 += 4;
       }

       /* permutation P and xor with the right side */

       for (j = 0; j < 32; j++)
         T[j] ^= T2[P[j]-1];

       /* swap the halves (not in the last iteration!) */

       if (i != 15)
         for (j = 0; j < 32; j++)
         {
           l = T[j];
           T[j] = T[j+32];
           T[j+32] = l;
         }
    }

  /* final permutation IP^-1 */

  memcpy(T2, T, 64);

  for(i = 0; i < 64; i++)
    T[i] = T2[FP[i]-1];

  /* Output conversion */

  for (i = 0; i < 60; i += 6 )
    *ret++ = output_conv[ T[i] << 5 | T[i+1] << 4 | T[i+2] << 3 |
      T[i+3] << 2 | T[i+4] << 1 | T[i+5] ];

  /* the last character has only 4 bits */

  *ret++ = output_conv[T[60] << 5 | T[61] << 4 | T[62] << 3 | T[63] << 2];

  /* return */

  *ret++ = '\0';
  return crypt_return;
}
