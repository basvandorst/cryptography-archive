
/*  -- derived from original haval.c, changes are:
 *  -- mgream: added run time endian checks, instead of precompiled
 *  -- mgream: changed function args from ansi (for compat)
 *
 *  haval.c:  specifies the routines in the HAVAL (V.1) hashing library.
 *
 *      HAVAL is a one-way hashing algorithm with the following
 *      collision-resistant property:
 *             It is computationally infeasible to find two or more
 *             messages that are hashed into the same fingerprint.
 *      
 *  Reference:
 *       Y. Zheng, J. Pieprzyk and J. Seberry:
 *       ``HAVAL --- a one-way hashing algorithm with variable
 *       length of output'', Advances in Cryptology --- AUSCRYPT'92,
 *       Lecture Notes in Computer Science, Springer-Verlag, 1993.
 *
 *  Descriptions:
 *      -  haval_string:      hash a string
 *      -  haval_file:        hash a file
 *      -  haval_stdin:       filter -- hash input from the stdin device
 *      -  haval_hash:        hash a string of specified length
 *                            (Haval_hash is used in conjunction with
 *                             haval_start & haval_end.)
 *      -  haval_hash_block:  hash a 32-word block
 *      -  haval_start:       initialization
 *      -  haval_end:         finalization
 *
 *  Author:     Yuliang Zheng
 *              Department of Computer Science
 *              University of Wollongong
 *              Wollongong, NSW 2522, Australia
 *              Email: yuliang@cs.uow.edu.au
 *              Voice: +61 42 21 4331 (office)
 *
 *  Date:       June 1993
 *
 *      Copyright (C) 1993 by C^3SR. All rights reserved. 
 *      This program may not be sold or used as inducement to
 *      buy a product without the written permission of C^3SR.
 */

#include <stdio.h>
#include <memory.h>
#include "haval.h"

static   int        endian = 0;          /* 0  = little, 1 = big */

#define VERSION    1                         /* current version number */

void haval_start ();            /* initialization */
void haval_hash ();      /* updating routine */
void haval_end (); /* finalization */
void haval_hash_block ();       /* hash a 32-word block */
static void haval_tailor ();    /* folding the last output */

static unsigned char padding[128] = {        /* constants for padding */
0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define f_1(x6, x5, x4, x3, x2, x1, x0)          \
           ((x1) & ((x0) ^ (x4)) ^ (x2) & (x5) ^ \
            (x3) & (x6) ^ (x0))

#define f_2(x6, x5, x4, x3, x2, x1, x0)                         \
           ((x2) & ((x1) & ~(x3) ^ (x4) & (x5) ^ (x6) ^ (x0)) ^ \
            (x4) & ((x1) ^ (x5)) ^ (x3) & (x5) ^ (x0)) 

#define f_3(x6, x5, x4, x3, x2, x1, x0)          \
           ((x3) & ((x1) & (x2) ^ (x6) ^ (x0)) ^ \
            (x1) & (x4) ^ (x2) & (x5) ^ (x0))

#define f_4(x6, x5, x4, x3, x2, x1, x0)                                 \
           ((x4) & ((x5) & ~(x2) ^ (x3) & ~(x6) ^ (x1) ^ (x6) ^ (x0)) ^ \
            (x3) & ((x1) & (x2) ^ (x5) ^ (x6)) ^                        \
            (x2) & (x6) ^ (x0))

#define f_5(x6, x5, x4, x3, x2, x1, x0)             \
           ((x0) & ((x1) & (x2) & (x3) ^ ~(x5)) ^   \
            (x1) & (x4) ^ (x2) & (x5) ^ (x3) & (x6))

/*
 * Permutations phi_{i,j}, i=3,4,5, j=1,...,i.
 *
 * PASS = 3:
 *               6 5 4 3 2 1 0
 *               | | | | | | | (replaced by)
 *  phi_{3,1}:   1 0 3 5 6 2 4
 *  phi_{3,2}:   4 2 1 0 5 3 6
 *  phi_{3,3}:   6 1 2 3 4 5 0
 *
 * PASS = 4:
 *               6 5 4 3 2 1 0
 *               | | | | | | | (replaced by)
 *  phi_{4,1}:   2 6 1 4 5 3 0
 *  phi_{4,2}:   3 5 2 0 1 6 4
 *  phi_{4,3}:   1 4 3 6 0 2 5
 *  phi_{4,4}:   6 4 0 5 2 1 3
 *
 * PASS = 5:
 *               6 5 4 3 2 1 0
 *               | | | | | | | (replaced by)
 *  phi_{5,1}:   3 4 1 0 5 2 6
 *  phi_{5,2}:   6 2 1 0 3 4 5
 *  phi_{5,3}:   2 6 0 4 3 1 5
 *  phi_{5,4}:   1 5 3 2 0 4 6
 *  phi_{5,5}:   2 5 0 6 4 3 1
 */

#if   PASS == 3
#define Fphi_1(x6, x5, x4, x3, x2, x1, x0) \
           f_1(x1, x0, x3, x5, x6, x2, x4)
#elif PASS == 4
#define Fphi_1(x6, x5, x4, x3, x2, x1, x0) \
           f_1(x2, x6, x1, x4, x5, x3, x0)
#else 
#define Fphi_1(x6, x5, x4, x3, x2, x1, x0) \
           f_1(x3, x4, x1, x0, x5, x2, x6)
#endif

#if   PASS == 3
#define Fphi_2(x6, x5, x4, x3, x2, x1, x0) \
           f_2(x4, x2, x1, x0, x5, x3, x6)
#elif PASS == 4
#define Fphi_2(x6, x5, x4, x3, x2, x1, x0) \
           f_2(x3, x5, x2, x0, x1, x6, x4)
#else 
#define Fphi_2(x6, x5, x4, x3, x2, x1, x0) \
           f_2(x6, x2, x1, x0, x3, x4, x5)
#endif

#if   PASS == 3
#define Fphi_3(x6, x5, x4, x3, x2, x1, x0) \
           f_3(x6, x1, x2, x3, x4, x5, x0)
#elif PASS == 4
#define Fphi_3(x6, x5, x4, x3, x2, x1, x0) \
           f_3(x1, x4, x3, x6, x0, x2, x5)
#else 
#define Fphi_3(x6, x5, x4, x3, x2, x1, x0) \
           f_3(x2, x6, x0, x4, x3, x1, x5)
#endif

#if   PASS == 4
#define Fphi_4(x6, x5, x4, x3, x2, x1, x0) \
           f_4(x6, x4, x0, x5, x2, x1, x3)
#else 
#define Fphi_4(x6, x5, x4, x3, x2, x1, x0) \
            f_4(x1, x5, x3, x2, x0, x4, x6)
#endif

#define Fphi_5(x6, x5, x4, x3, x2, x1, x0) \
           f_5(x2, x5, x0, x6, x4, x3, x1)

#define rotate_right(x, n) (((x) >> (n)) | ((x) << (32-(n))))

#define FF_1(x7, x6, x5, x4, x3, x2, x1, x0, w) {                        \
      register haval_word temp = Fphi_1(x6, x5, x4, x3, x2, x1, x0);     \
      (x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w);       \
      }

#define FF_2(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {                      \
      register haval_word temp = Fphi_2(x6, x5, x4, x3, x2, x1, x0);      \
      (x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);  \
      }

#define FF_3(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {                      \
      register haval_word temp = Fphi_3(x6, x5, x4, x3, x2, x1, x0);      \
      (x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);  \
      }

#define FF_4(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {                      \
      register haval_word temp = Fphi_4(x6, x5, x4, x3, x2, x1, x0);      \
      (x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);  \
      }

#define FF_5(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {                      \
      register haval_word temp = Fphi_5(x6, x5, x4, x3, x2, x1, x0);      \
      (x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);  \
      }

/*
 * translate every four characters into a word.
 * assume the number of characters is a multiple of four.
 */
#define ch2uint(string, word, slen) {      \
  unsigned char *sp = string;              \
  haval_word    *wp = word;                \
  while (sp < (string) + (slen)) {         \
    *wp++ =  (haval_word)*sp            |  \
            ((haval_word)*(sp+1) <<  8) |  \
            ((haval_word)*(sp+2) << 16) |  \
            ((haval_word)*(sp+3) << 24);   \
    sp += 4;                               \
  }                                        \
}

/* translate each word into four characters */
#define uint2ch(word, string, wlen) {              \
  haval_word    *wp = word;                        \
  unsigned char *sp = string;                      \
  while (wp < (word) + (wlen)) {                   \
    *(sp++) = (unsigned char)( *wp        & 0xFF); \
    *(sp++) = (unsigned char)((*wp >>  8) & 0xFF); \
    *(sp++) = (unsigned char)((*wp >> 16) & 0xFF); \
    *(sp++) = (unsigned char)((*wp >> 24) & 0xFF); \
    wp++;                                          \
  }                                                \
}

/* initialization */
void haval_start (state)
    haval_state *state;
{
    state->count[0]       = state->count[1] = 0;   /* clear count */
    state->fingerprint[0] = 0x243F6A88;            /* initial fingerprint */
    state->fingerprint[1] = 0x85A308D3;
    state->fingerprint[2] = 0x13198A2E;
    state->fingerprint[3] = 0x03707344;
    state->fingerprint[4] = 0xA4093822;
    state->fingerprint[5] = 0x299F31D0;
    state->fingerprint[6] = 0x082EFA98;
    state->fingerprint[7] = 0xEC4E6C89;

    /* from shs.c
     *
	 * Find out what the byte order is on this machine.
	 * Big endian is for machines that place the most significant byte
	 * first (eg. Sun SPARC). Little endian is for machines that place
	 * the least significant byte first (eg. VAX).
	 *
	 * We figure out the byte order by stuffing a 2 byte string into a
	 * short and examining the left byte. '@' = 0x40  and  'P' = 0x50
	 * If the left byte is the 'high' byte, then it is 'big endian'.
	 * If the left byte is the 'low' byte, then the machine is 'little
	 * endian'.
	 *
	 *                          -- Shawn A. Clifford (sac@eng.ufl.edu)
	 */

	if ((*(unsigned short *) ("@P") >> 8) == '@')
        endian = 1;
    else
        endian = 0;

}

/*
 * hash a string of specified length.
 * to be used in conjunction with haval_start and haval_end.
 */
void haval_hash (state, str, str_len)
  haval_state* state;
  unsigned char *str;
  unsigned int str_len;
{
  unsigned int i,
           rmd_len,
           fill_len;

  /* calculate the number of bytes in the remainder */
  rmd_len  = (unsigned int)((state->count[0] >> 3) & 0x7F);
  fill_len = 128 - rmd_len;

  /* update the number of bits */
  if ((state->count[0] +=  (haval_word)str_len << 3)
                        < ((haval_word)str_len << 3)) {
     state->count[1]++;
  }
  state->count[1] += (haval_word)str_len >> 29;

/*#ifdef LITTLE_ENDIAN*/
  if (endian == 0) {

  /* hash as many blocks as possible */
  if (rmd_len + str_len >= 128) {
    memcpy (((unsigned char *)state->block)+rmd_len, str, fill_len);
    haval_hash_block (state);
    for (i = fill_len; i + 127 < str_len; i += 128){
      memcpy ((unsigned char *)state->block, str+i, 128);
      haval_hash_block (state);
    }
    rmd_len = 0;
  } else {
    i = 0;
  }
  memcpy (((unsigned char *)state->block)+rmd_len, str+i, str_len-i);

/*#else*/
  } else {

  /* hash as many blocks as possible */
  if (rmd_len + str_len >= 128) {
    memcpy (&state->remainder[rmd_len], str, fill_len);
    ch2uint(state->remainder, state->block, 128);
    haval_hash_block (state);
    for (i = fill_len; i + 127 < str_len; i += 128){
      memcpy (state->remainder, str+i, 128);
      ch2uint(state->remainder, state->block, 128);
      haval_hash_block (state);
    }
    rmd_len = 0;
  } else {
    i = 0;
  }
  /* save the remaining input chars */
  memcpy (&state->remainder[rmd_len], str+i, str_len-i);

/*#endif*/
  }

}

/* finalization */
void haval_end (state, final_fpt)
  haval_state * state;
  unsigned char final_fpt[FPTLEN >> 3];
{
  unsigned char tail[10];
  unsigned int  rmd_len, pad_len;

  /*
   * save the version number, the number of passes, the fingerprint 
   * length and the number of bits in the unpadded message.
   */
  tail[0] = (unsigned char)(((FPTLEN  & 0x3) << 6) |
                            ((PASS    & 0x7) << 3) |
                             (VERSION & 0x7));
  tail[1] = (unsigned char)((FPTLEN >> 2) & 0xFF);
  uint2ch (state->count, &tail[2], 2);

  /* pad out to 118 mod 128 */
  rmd_len = (unsigned int)((state->count[0] >> 3) & 0x7f);
  pad_len = (rmd_len < 118) ? (118 - rmd_len) : (246 - rmd_len);
  haval_hash (state, padding, pad_len);

  /*
   * append the version number, the number of passes,
   * the fingerprint length and the number of bits
   */
  haval_hash (state, tail, 10);

  /* tailor the last output */
  haval_tailor(state);

  /* translate and save the final fingerprint */
  uint2ch (state->fingerprint, final_fpt, FPTLEN >> 5);

  /* clear the state information */
  memset ((unsigned char *)state, 0, sizeof (*state));
}

/* hash a 32-word block */
void haval_hash_block (state)
  haval_state* state;
{
  register haval_word t0 = state->fingerprint[0],    /* make use of */
                      t1 = state->fingerprint[1],    /* internal registers */
                      t2 = state->fingerprint[2],
                      t3 = state->fingerprint[3],
                      t4 = state->fingerprint[4],
                      t5 = state->fingerprint[5],
                      t6 = state->fingerprint[6],
                      t7 = state->fingerprint[7],
                      *w = state->block;

  /* Pass 1 */
  FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w   ));
  FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 1));
  FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 2));
  FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 3));
  FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 4));
  FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 5));
  FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 6));
  FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 7));

  FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 8));
  FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 9));
  FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+10));
  FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+11));
  FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+12));
  FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+13));
  FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+14));
  FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+15));

  FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w+16));
  FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+17));
  FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+18));
  FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+19));
  FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+20));
  FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+21));
  FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+22));
  FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+23));

  FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w+24));
  FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+25));
  FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+26));
  FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+27));
  FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+28));
  FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+29));
  FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+30));
  FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+31));

  /* Pass 2 */
  FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 5), 0x452821E6);
  FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+14), 0x38D01377);
  FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+26), 0xBE5466CF);
  FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+18), 0x34E90C6C);
  FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+11), 0xC0AC29B7);
  FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+28), 0xC97C50DD);
  FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 7), 0x3F84D5B5);
  FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+16), 0xB5470917);

  FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w   ), 0x9216D5D9);
  FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+23), 0x8979FB1B);
  FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+20), 0xD1310BA6);
  FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+22), 0x98DFB5AC);
  FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 1), 0x2FFD72DB);
  FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+10), 0xD01ADFB7);
  FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 4), 0xB8E1AFED);
  FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 8), 0x6A267E96);

  FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w+30), 0xBA7C9045);
  FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 3), 0xF12C7F99);
  FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+21), 0x24A19947);
  FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 9), 0xB3916CF7);
  FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+17), 0x0801F2E2);
  FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+24), 0x858EFC16);
  FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+29), 0x636920D8);
  FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 6), 0x71574E69);

  FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w+19), 0xA458FEA3);
  FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+12), 0xF4933D7E);
  FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+15), 0x0D95748F);
  FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+13), 0x728EB658);
  FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 2), 0x718BCD58);
  FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+25), 0x82154AEE);
  FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+31), 0x7B54A41D);
  FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+27), 0xC25A59B5);

  /* Pass 3 */
  FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+19), 0x9C30D539);
  FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 9), 0x2AF26013);
  FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 4), 0xC5D1B023);
  FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+20), 0x286085F0);
  FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+28), 0xCA417918);
  FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w+17), 0xB8DB38EF);
  FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 8), 0x8E79DCB0);
  FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+22), 0x603A180E);

  FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+29), 0x6C9E0E8B);
  FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+14), 0xB01E8A3E);
  FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+25), 0xD71577C1);
  FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+12), 0xBD314B27);
  FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+24), 0x78AF2FDA);
  FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w+30), 0x55605C60);
  FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+16), 0xE65525F3);
  FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+26), 0xAA55AB94);

  FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+31), 0x57489862);
  FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+15), 0x63E81440);
  FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 7), 0x55CA396A);
  FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 3), 0x2AAB10B6);
  FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 1), 0xB4CC5C34);
  FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w   ), 0x1141E8CE);
  FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+18), 0xA15486AF);
  FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+27), 0x7C72E993);

  FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+13), 0xB3EE1411);
  FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 6), 0x636FBC2A);
  FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+21), 0x2BA9C55D);
  FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+10), 0x741831F6);
  FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+23), 0xCE5C3E16);
  FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w+11), 0x9B87931E);
  FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 5), 0xAFD6BA33);
  FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 2), 0x6C24CF5C);

#if PASS >= 4
  /* Pass 4. executed only when PASS =4 or 5 */
  FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+24), 0x7A325381);
  FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 4), 0x28958677);
  FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w   ), 0x3B8F4898);
  FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+14), 0x6B4BB9AF);
  FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 2), 0xC4BFE81B);
  FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 7), 0x66282193);
  FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+28), 0x61D809CC);
  FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+23), 0xFB21A991);

  FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+26), 0x487CAC60);
  FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 6), 0x5DEC8032);
  FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w+30), 0xEF845D5D);
  FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+20), 0xE98575B1);
  FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+18), 0xDC262302);
  FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+25), 0xEB651B88);
  FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+19), 0x23893E81);
  FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 3), 0xD396ACC5);

  FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+22), 0x0F6D6FF3);
  FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+11), 0x83F44239);
  FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w+31), 0x2E0B4482);
  FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+21), 0xA4842004);
  FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 8), 0x69C8F04A);
  FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+27), 0x9E1F9B5E);
  FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+12), 0x21C66842);
  FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 9), 0xF6E96C9A);

  FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 1), 0x670C9C61);
  FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+29), 0xABD388F0);
  FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 5), 0x6A51A0D2);
  FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+15), 0xD8542F68);
  FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+17), 0x960FA728);
  FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+10), 0xAB5133A3);
  FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+16), 0x6EEF0B6C);
  FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+13), 0x137A3BE4);
#endif

#if PASS == 5
  /* Pass 5. executed only when PASS = 5 */
  FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+27), 0xBA3BF050);
  FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 3), 0x7EFB2A98);
  FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+21), 0xA1F1651D);
  FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+26), 0x39AF0176);
  FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+17), 0x66CA593E);
  FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+11), 0x82430E88);
  FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+20), 0x8CEE8619);
  FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+29), 0x456F9FB4);

  FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+19), 0x7D84A5C3);
  FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w   ), 0x3B8B5EBE);
  FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+12), 0xE06F75D8);
  FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 7), 0x85C12073);
  FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+13), 0x401A449F);
  FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 8), 0x56C16AA6);
  FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+31), 0x4ED3AA62);
  FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+10), 0x363F7706);

  FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 5), 0x1BFEDF72);
  FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 9), 0x429B023D);
  FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+14), 0x37D0D724);
  FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+30), 0xD00A1248);
  FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+18), 0xDB0FEAD3);
  FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 6), 0x49F1C09B);
  FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+28), 0x075372C9);
  FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+24), 0x80991B7B);

  FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 2), 0x25D479D8);
  FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w+23), 0xF6E8DEF7);
  FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+16), 0xE3FE501A);
  FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+22), 0xB6794C3B);
  FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 4), 0x976CE0BD);
  FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 1), 0x04C006BA);
  FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+25), 0xC1A94FB6);
  FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+15), 0x409F60C4);
#endif

  state->fingerprint[0] += t0;
  state->fingerprint[1] += t1;
  state->fingerprint[2] += t2;
  state->fingerprint[3] += t3;
  state->fingerprint[4] += t4;
  state->fingerprint[5] += t5;
  state->fingerprint[6] += t6;
  state->fingerprint[7] += t7;
}

/* tailor the last output */
static void haval_tailor (state)
  haval_state * state;
{
  haval_word temp;

#if FPTLEN == 128
  temp = (state->fingerprint[7] & 0x000000FF) | 
         (state->fingerprint[6] & 0xFF000000) | 
         (state->fingerprint[5] & 0x00FF0000) | 
         (state->fingerprint[4] & 0x0000FF00);
  state->fingerprint[0] += rotate_right(temp,  8);

  temp = (state->fingerprint[7] & 0x0000FF00) | 
         (state->fingerprint[6] & 0x000000FF) | 
         (state->fingerprint[5] & 0xFF000000) | 
         (state->fingerprint[4] & 0x00FF0000);
  state->fingerprint[1] += rotate_right(temp, 16);

  temp  = (state->fingerprint[7] & 0x00FF0000) | 
          (state->fingerprint[6] & 0x0000FF00) | 
          (state->fingerprint[5] & 0x000000FF) | 
          (state->fingerprint[4] & 0xFF000000);
  state->fingerprint[2] += rotate_right(temp, 24);

  temp = (state->fingerprint[7] & 0xFF000000) | 
         (state->fingerprint[6] & 0x00FF0000) | 
         (state->fingerprint[5] & 0x0000FF00) | 
         (state->fingerprint[4] & 0x000000FF);
  state->fingerprint[3] += temp;

#elif FPTLEN == 160
  temp = (state->fingerprint[7] &  (haval_word)0x3F) | 
         (state->fingerprint[6] & ((haval_word)0x7F << 25)) |  
         (state->fingerprint[5] & ((haval_word)0x3F << 19));
  state->fingerprint[0] += rotate_right(temp, 19);

  temp = (state->fingerprint[7] & ((haval_word)0x3F <<  6)) | 
         (state->fingerprint[6] &  (haval_word)0x3F) |  
         (state->fingerprint[5] & ((haval_word)0x7F << 25));
  state->fingerprint[1] += rotate_right(temp, 25);

  temp = (state->fingerprint[7] & ((haval_word)0x7F << 12)) | 
         (state->fingerprint[6] & ((haval_word)0x3F <<  6)) |  
         (state->fingerprint[5] &  (haval_word)0x3F);
  state->fingerprint[2] += temp;

  temp = (state->fingerprint[7] & ((haval_word)0x3F << 19)) | 
         (state->fingerprint[6] & ((haval_word)0x7F << 12)) |  
         (state->fingerprint[5] & ((haval_word)0x3F <<  6));
  state->fingerprint[3] += temp >> 6; 

  temp = (state->fingerprint[7] & ((haval_word)0x7F << 25)) | 
         (state->fingerprint[6] & ((haval_word)0x3F << 19)) |  
         (state->fingerprint[5] & ((haval_word)0x7F << 12));
  state->fingerprint[4] += temp >> 12;

#elif FPTLEN == 192
  temp = (state->fingerprint[7] &  (haval_word)0x1F) | 
         (state->fingerprint[6] & ((haval_word)0x3F << 26));
  state->fingerprint[0] += rotate_right(temp, 26);

  temp = (state->fingerprint[7] & ((haval_word)0x1F <<  5)) | 
         (state->fingerprint[6] &  (haval_word)0x1F);
  state->fingerprint[1] += temp;

  temp = (state->fingerprint[7] & ((haval_word)0x3F << 10)) | 
         (state->fingerprint[6] & ((haval_word)0x1F <<  5));
  state->fingerprint[2] += temp >> 5;

  temp = (state->fingerprint[7] & ((haval_word)0x1F << 16)) | 
         (state->fingerprint[6] & ((haval_word)0x3F << 10));
  state->fingerprint[3] += temp >> 10;

  temp = (state->fingerprint[7] & ((haval_word)0x1F << 21)) | 
         (state->fingerprint[6] & ((haval_word)0x1F << 16));
  state->fingerprint[4] += temp >> 16;

  temp = (state->fingerprint[7] & ((haval_word)0x3F << 26)) | 
         (state->fingerprint[6] & ((haval_word)0x1F << 21));
  state->fingerprint[5] += temp >> 21;

#elif FPTLEN == 224
  state->fingerprint[0] += (state->fingerprint[7] >> 27) & 0x1F;
  state->fingerprint[1] += (state->fingerprint[7] >> 22) & 0x1F;
  state->fingerprint[2] += (state->fingerprint[7] >> 18) & 0x0F;
  state->fingerprint[3] += (state->fingerprint[7] >> 13) & 0x1F;
  state->fingerprint[4] += (state->fingerprint[7] >>  9) & 0x0F;
  state->fingerprint[5] += (state->fingerprint[7] >>  4) & 0x1F;
  state->fingerprint[6] +=  state->fingerprint[7]        & 0x0F;
#endif
}


