
/*
 *  strandom.c:  specifies the routines for generating cryptographically
 *            strong pseudo-random numbers. The generator is based on
 *            the basic hashing function employed by HAVAL.
 *
 *            to be used in conjunction with
 *                haval.h
 *                haval.c
 *                havalapp.h (for defining PASS)
 *
 *  Descriptions:
 *      -  init_strandom:	initialization with a seed of specified length
 *                      	(in the number of 32-bit unsigned words)
 *      -  strandom:		generate random sequence of specified length
 *				(in the number of 32-bit unsigned words)
 *
 *  Author:     Yuliang Zheng
 *              Department of Computer Science
 *              University of Wollongong
 *              Wollongong, NSW 2522, Australia
 *              Email: yuliang@cs.uow.edu.au
 *              Voice: +61 42 21 4331 (office)
 *
 *  Date:       July 1993
 *
 *      Copyright (C) 1993 by C^3SR. All rights reserved. 
 *      This program may not be sold or used as inducement to
 *      buy a product without the written permission of C^3SR.
 */

#include <memory.h>
#include "haval.h"

#define BLOCKLEN      32                /* basic hashing unit of HAVAL (32 words) */

void init_strandom (unsigned int *, int); /* initialization */
void strandom (unsigned int *, int);    /* generate random numbers */

/*
 * The following 32 constant words are generated by
 * a Maple program described here.
 *
 *       printlevel := -1;
 *       Digits := 1000;
 *       goldenratio := evalf((sqrt(5) - 1) / 2);
 *       K := 2 ^ 32;
 *       for i from 1 by 1 while i <= 40 do
 *         nextword := trunc(goldenratio * K); 
 *         lprint(convert(nextword,hex));
 *         goldenratio := frac(goldenratio * K);
 *       od;
 *
 */

static unsigned int start_msg[]=                   /* initial block for HAVAL. */
  {0x9E3779B9, 0x7F4A7C15, 0xF39CC060, 0x5CEDC834, /* taken from the fraction */
   0x1082276B, 0xF3A27251, 0xF86C6A11, 0xD0C18E95, /* part of the Golden ratio */
   0x2767F0B1, 0x53D27B7F, 0x0347045B, 0x5BF1827F, /* [sqrt(5)-1]/2. */
   0x01886F09, 0x28403002, 0xC1D64BA4, 0x0F335E36,
   0xF06AD7AE, 0x9717877E, 0x85839D6E, 0xFFBD7DC6,
   0x64D325D1, 0xC5371682, 0xCADD0CCC, 0xFDFFBBE1,
   0x626E33B8, 0xD04B4331, 0xBBF73C79, 0x0D94F79D,
   0x471C4AB3, 0xED3D82A5, 0xFEC50770, 0x5E4AE6E5};


haval_state   state;                    /* state information for HAVAL */
unsigned int  fdbk_p;                   /* starting position for feedback */
unsigned int  init_fpt[8];              /* initial fpt for HAVAL */

/* set seed of specified length (in 32-bit words) */
void init_strandom (unsigned int *seed, int seed_len)
{
  int i;

  memcpy ((char *)state.block,          /* set initial 32-word block */ 
          (char *)start_msg, BLOCKLEN << 2);

  for (i=0; (i<seed_len) && (i<BLOCKLEN); i++) {
    state.block[i] ^= *(seed + i);      /* set seed */
  }

  fdbk_p = (0x03 << 3) & *seed;         /* position for feadback */
  haval_start (&state);                 /* initializing HAVAL */
  memcpy ((char *)init_fpt,             /* catch the initial fingerprint */ 
          (char *)state.fingerprint, 8 << 2);
}

/*
 * generate a cryptographically STRong pseudo-rANDOM sequence
 * of specified length (in the number of 32-bit words).
 * output is set in rand_num.
 */
void strandom (unsigned int *rand_num, int len)
{
  int i, j;
  
  for (i=0; i<(len/6); i++) {           /* repeat as many times as possible */
    memcpy ((char *)state.fingerprint,  /* set the initial fingerprint */ 
            (char *)init_fpt, 8 << 2);
    haval_hash_block(&state);           /* hash block */
    memcpy ((char *)rand_num,		/* output the first 6 words of fpt */
            (char *)state.fingerprint, 6 << 2);
    rand_num += 6;                      /* update output pointer */

    for (j=0; j<8; j++){                /* set feedback */
      state.block[fdbk_p + j] ^= state.fingerprint[j];
    }

    fdbk_p = (0x03 << 3) &              /* next point for feedback */
             (state.fingerprint[6] ^ state.fingerprint[7]);
  }

  if ((len % 6) != 0) {			/* call HAVAL one more time if necessary */
    memcpy ((char *)state.fingerprint,  /* set the initial fingerprint */ 
            (char *)init_fpt, 8 << 2);
    haval_hash_block(&state);
    memcpy ((char *)rand_num,
            (char *)state.fingerprint, (len % 6) << 2);

    for (j=0; j<8; j++){
      state.block[fdbk_p + j] ^= state.fingerprint[j];
    }

    fdbk_p = (0x03 << 3) &
             (state.fingerprint[6] ^ state.fingerprint[7]);
  }
}

