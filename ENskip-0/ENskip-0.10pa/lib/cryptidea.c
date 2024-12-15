/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "memblk.h"
#include "crypt.h"

/*************************************************************************
 * IDEA implementation by Richard De Moliner                             *
 *************************************************************************/

/****************************************************************************/
/*                                                                          */
/*             C R Y P T O G R A P H I C - A L G O R I T H M S              */
/*                                                                          */
/****************************************************************************/
/* Author:       Richard De Moliner (demoliner@isi.ethz.ch)                 */
/*               Signal and Information Processing Laboratory               */
/*               Swiss Federal Institute of Technology                      */
/*               CH-8092 Zuerich, Switzerland                               */
/* Last Edition: 23 April 1992                                              */
/* System:       SUN SPARCstation, SUN cc C-Compiler, SUN-OS 4.1.1          */
/****************************************************************************/
/* Change this type definitions to the representations in your computer.      */

/* Do not change the lines below.                                             */

#define dataSize       8 /* bytes = 64 bits */
#define dataLen        4
#define keySize      104 /* bytes = 832 bits */
#define keyLen        52
#define userKeySize   16 /* bytes = 128 bits */
#define userKeyLen     8

#define DataT(v) u_int16 v [dataLen]
#define KeyT(v) u_int16 v [keyLen]
#define UserKeyT(v) u_int16 v [userKeyLen]


#define mulMod        0x10001 /* 2**16 + 1                                 */
#define addMod        0x10000 /* 2**16                                     */
#define ones           0xFFFF /* 2**16 - 1                                 */

#define nofKeyPerRound      6 /* number of used keys per round             */
#define nofRound            8 /* number of rounds                          */

#define GETWORD(x)  ((((u_char *)x)[0] << 8) | (((u_char *)x)[1])); x++
#define PUTWORD(val,x)  ((u_char *)x)[0] = val >> 8; \
                        ((u_char *)x)[1] = val & 0xff ; x++

/***************************************************************************/
/*                          A L G O R I T H M                              */
/***************************************************************************/
/* multiplication                                                          */


#ifndef NO_MUL_MACRO

#define Mul(a,b) (((q = (t = (a)) * (p = (b))) == 0) ? (1 - t - p) : \
		  (((p = (u_int16)q) <= (q >>= 16)) ? (p - q + 1) : (p - q)))


#else /* NO_MUL_MACRO */

static u_int16 Mul(u_int16 a, u_int16 b)
{
  u_int16 p;
  u_int32 q;
  
  if ((q = a * b) == 0)
    p = 1 - a - b;
  else
  {
    p = (u_int16)q;
    q >>= 16;
    if (p <= (u_int16)q)
      q--;
    p -= q;
  }

  return p;
} /* Mul */

#endif /* NO_MUL_MACRO */

/***************************************************************************/
/* compute inverse of 'x' by Euclidean gcd algorithm                       */

static u_int16 MulInv(u_int16 x)
{
  int32 n1, n2, q, r, b1, b2, t;

  if (x == 0) return 0;
  n1 = mulMod; n2 = (int32)x; b2 = 1; b1 = 0;
  do {
    r = (n1 % n2); q = (n1 - r) / n2;
    if (r == 0) { if (b2 < 0) b2 = mulMod + b2; }
    else { n1 = n2; n2 = r; t = b2; b2 = b1 - q * b2; b1 = t; }
  } while (r != 0);
  return (u_int16)b2;
} /* MulInv */

/***************************************************************************/
/* encryption and decryption algorithm IDEA                                */

void  Idea(u_int16 *dataIn, u_int16 *dataOut, u_int16 *key)
{
#ifndef NO_MUL_MACRO
  register u_int16 p;
  register u_int32 q;
#endif /* NO_MUL_MACRO */
  register u_int16 round, x0, x1, x2, x3, t0, t1, t;

  x0 = GETWORD(dataIn); x1 = GETWORD(dataIn);
  x2 = GETWORD(dataIn); x3 = GETWORD(dataIn);

  for (round = nofRound; round > 0; round--)
  {
    x0 = Mul(x0, *key++);
    x1 += *key++;
    x2 += *key++;
    x3 = Mul(x3, *key++);
    t0 = Mul(*key++, x0 ^ x2);      
    t1 = Mul(*key++, t0 + (x1 ^ x3));
    t0 += t1;
    x0 ^= t1; x3 ^= t0;
    t0 ^= x1; x1 = x2 ^ t1; x2 = t0;
  }

  x0 = Mul(x0, *key++); x2 += *key++; x1 += *key++; x3 = Mul(x3, *key);

  PUTWORD(x0, dataOut); PUTWORD(x2, dataOut);
  PUTWORD(x1, dataOut); PUTWORD(x3, dataOut);
} /* Idea */
 
/***************************************************************************/
/* invert decryption / encrytion key for IDEA                              */

void InvertIdeaKey(u_int16 *key, u_int16 *invKey)
{
  register int  i;
  KeyT(dk);

  dk[nofKeyPerRound * nofRound + 0] = MulInv(*(key++));
  dk[nofKeyPerRound * nofRound + 1] = (addMod - *(key++)) & ones;
  dk[nofKeyPerRound * nofRound + 2] = (addMod - *(key++)) & ones;
  dk[nofKeyPerRound * nofRound + 3] = MulInv(*(key++));
  for (i = nofKeyPerRound * (nofRound - 1); i >= 0; i -= nofKeyPerRound) {
    dk[i + 4] = *(key++);
    dk[i + 5] = *(key++);
    dk[i + 0] = MulInv(*(key++));
    if (i > 0) {
      dk[i + 2] = (addMod - *(key++)) & ones;
      dk[i + 1] = (addMod - *(key++)) & ones;
    }
    else {
      dk[i + 1] = (addMod - *(key++)) & ones;
      dk[i + 2] = (addMod - *(key++)) & ones;
    }
    dk[i + 3] = MulInv(*(key++));
  }
  for (i = 0; i < keyLen; i++) invKey[i] = dk[i]; 
} /* InvertIdeaKey */


/***************************************************************************/
/* expand user key of 128 bits to full key of 832 bits                     */

void ExpandUserKey(u_int16 *userKey, u_int16 *key)
{
  register int i;

  for (i = 0; i < userKeyLen; i++)
    key[i] = (((u_char *)userKey)[0] << 8) | ((u_char *)userKey)[1];

  /* shifts */
  for (i = userKeyLen; i < keyLen; i++) {
    if ((i + 2) % 8 == 0)                    /* for key[14],key[22],..  */
      key[i] = ((key[i - 7] & 127) << 9) ^ (key[i - 14] >> 7); 
    else if ((i + 1) % 8 == 0)               /* for key[15],key[23],..  */
      key[i] = ((key[i - 15] & 127) << 9) ^ (key[i - 14] >> 7); 
    else
      key[i] = ((key[i - 7] & 127) << 9 ) ^ (key[i - 6] >> 7);
   }
} /* ExpandUserKey */


/*************************************************************************
 * IDEA interface for SKIP                                               *
 *************************************************************************/

struct state
{
  int alg;
  UserKeyT(key);
  KeyT(schedule);
};

static int cryptidea_init(void)
{
  return 0;
}

static int cryptidea_exit(void)
{
  return 0;
}

static int cryptidea_encryptstatelen(void)
{
  return sizeof(struct state);
}

static int cryptidea_decryptstatelen(void)
{
  return sizeof(struct state);
}

static int cryptidea_blocklen(void)
{
  return dataSize;
}

static int cryptidea_keylen(void)
{
  return userKeySize;
}

static int cryptidea_MIlen(void)
{
  return dataSize;
}

static int cryptidea_setencryptkey(u_char *key, u_char *stateptr)
{
  struct state *state = (struct state *)stateptr;

  ExpandUserKey(state->key, state->schedule);
  return 0;
}

static int cryptidea_setdecryptkey(u_char *key, u_char *stateptr)
{
  struct state *state = (struct state *)stateptr;

  ExpandUserKey(state->key, state->schedule);
  InvertIdeaKey(state->schedule, state->schedule);
  return 0;
}

static int cryptidea_crypt(u_char *data, int len, u_char *stateptr)
{
  struct state *state = (struct state *)stateptr;

  Idea((void *)data, (void *)data, state->schedule);
  return 0;
}

struct crypt_algorithm cryptidea =
{
  cryptidea_init, cryptidea_exit,
  cryptidea_encryptstatelen, cryptidea_decryptstatelen, 
  cryptidea_blocklen, cryptidea_keylen, cryptidea_MIlen,
  cryptidea_setencryptkey, cryptidea_setdecryptkey,
  cryptidea_crypt, cryptidea_crypt,
  NULL
};
