/****************************************************************
 *                                                              *
 *  This program is part of an implementation of the block      *
 *  cipher RC6(TM) that is being submitted to NIST as a         *
 *  candidate for the AES.                                      *
 *                                                              *
 *  Ronald L. Rivest and RSA Laboratories, the submitters of    *
 *  RC6, retain all rights to RC6 and to this code, except for  *
 *  those which NIST requires to be waived for submissions.     *
 *                                                              *
 *  Copyright (C) 1998 RSA Data Security, Inc.                  *
 *                                                              *
 ****************************************************************/

/*
 * This file contains a version of RC6 in C which is
 * specifically written to make available the internal values
 * of RC6 computations.
 */




#include <stdlib.h>
#include <string.h>


#include "aes_iv.h"


#define Sn (2+2*ROUNDS+2)


#define ROTL(W,c) (((W) << (c)) | ((W) >> (32-(c))))
#define ROTR(W,c) (((W) >> (c)) | ((W) << (32-(c))))



void* ComputeKeySchedule(unsigned char* key, unsigned long keyLength)
{
  unsigned long L[256/4];
  unsigned long Ln;
  unsigned long A, B, i, j;
  long count;
  unsigned long* S = (unsigned long*) malloc(Sn*sizeof(unsigned long));


  /* Get original key into array of dwords */
  if (keyLength == 0) {
    Ln = 1;
    L[0] = 0;
  } else {
    Ln = (keyLength+3)/4;
    L[Ln-1] = 0;
    memcpy(L, key, (size_t) keyLength);
  }


  /* Initialize S[] */
  S[0] = 0xb7e15163;
  for (count = 1; count < Sn; ++count)
    S[count] = S[count-1]+0x9e3779b9;


  /* Perform key schedule computation*/
  A = B = i = j = 0;

  for (count = 3*((Sn > Ln)?Sn:Ln); --count >= 0;) {
    unsigned long t;

    A += S[i]+B;
    S[i] = A = ROTL(A, 3);

    B = L[j]+(t = A+B);
    L[j] = B = ROTL(B, t);

    if (++i == Sn)
      i = 0;

    if (++j == Ln)
      j = 0;
  }


  return (void*) S;
}



void Encrypt(void* keySchedule, Block* input, CryptData* cryptData)
{
  DwordsBlock regs = input->dwordsBlock;
  unsigned long* S = (unsigned long*) keySchedule;
  long i;

  /* Round 0 */
  regs.B += S[0];
  regs.D += S[1];
  cryptData->roundData[0].dwordsBlock = regs;


  /* Now all the "real" rounds */
  for (i = 0; ++i <= ROUNDS;) {
    unsigned long t = regs.B*(2*regs.B+1);
    unsigned long u = regs.D*(2*regs.D+1);
    unsigned long temp;

    t = ROTL(t, 5);
    u = ROTL(u, 5);

    regs.A ^= t;
    regs.A = ROTL(regs.A, u);
    regs.A += S[2*i];

    regs.C ^= u;
    regs.C = ROTL(regs.C, t);
    regs.C += S[2*i+1];

    temp = regs.A;
    regs.A = regs.B;
    regs.B = regs.C;
    regs.C = regs.D;
    regs.D = temp;
    cryptData->roundData[i].dwordsBlock = regs;
  }


  /* Round ROUNDS+1 */
  regs.A += S[2*ROUNDS+2];
  regs.C += S[2*ROUNDS+3];
  cryptData->roundData[ROUNDS+1].dwordsBlock = regs;
}


void Decrypt(void* keySchedule, Block* input, CryptData* cryptData)
{
  DwordsBlock regs = input->dwordsBlock;
  unsigned long* S = (unsigned long*) keySchedule;
  long i;

  /* Round ROUNDS+1 */
  regs.C -= S[2*ROUNDS+3];
  regs.A -= S[2*ROUNDS+2];
  cryptData->roundData[0].dwordsBlock = regs;


  /* Now all the "real" rounds */
  for (i = ROUNDS+1; --i >= 1;) {
    unsigned long t;
    unsigned long u;
    unsigned long temp;

    temp = regs.D;
    regs.D = regs.C;
    regs.C = regs.B;
    regs.B = regs.A;
    regs.A = temp;

    t = regs.B*(2*regs.B+1);
    u = regs.D*(2*regs.D+1);
    t = ROTL(t, 5);
    u = ROTL(u, 5);

    regs.C -= S[2*i+1];
    regs.C = ROTR(regs.C, t);
    regs.C ^= u;

    regs.A -= S[2*i];
    regs.A = ROTR(regs.A, u);
    regs.A ^= t;

    cryptData->roundData[ROUNDS+1-i].dwordsBlock = regs;
  }


  /* Round 0 */
  regs.D -= S[1];
  regs.B -= S[0];
  cryptData->roundData[ROUNDS+1].dwordsBlock = regs;
}


void DeleteKeySchedule(void* keySchedule)
{
  free(keySchedule);
}

