/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "memblk.h"
#include "crypt.h"

/**************************************************************************
 * RC4 Implementation from Usenet source                                  *
 **************************************************************************/

typedef struct rc4_key
{
  unsigned char state[256];
  unsigned char x, y;
} rc4_key;

static void rc4_setkey(unsigned char *keydata, int len, rc4_key *key)
{
  unsigned char i2, is, i2s, *state, *k;
  int i, i1;

  state = key->state;
  for (i = 0; i < 256; i++)
    *state++ = i;

  state = key->state;
  for (i = i1 = i2 = 0, k = keydata; i < 256; i++)
  {
    is = state[i];
    i2 += *k++ + is;
    i2s = state[i2];
    state[i] = i2s;
    state[i2] = is;
    
    if (++i1 >= len)
    {
      k = keydata;
      i1 = 0;
    }
  }

  key->x = key->y = 0;
}

static void rc4_fastforward(rc4_key *key, u_int amount)
{
  int i;
  unsigned char x, y, xs, ys, *state;

  x = key->x;
  y = key->y;
  state = key->state;

  for (i = 0; i < amount; i++)
  {
    xs = state[++x];
    ys = state[y += xs];
    state[x] = ys;
    state[y] = xs;
  }

  key->x = x;
  key->y = y;
}

static void rc4_crypt(unsigned char *src, unsigned char *dst, int len, rc4_key *key)
{
  int i;
  unsigned char x, y, xs, ys, *state;

  x = key->x;
  y = key->y;
  state = key->state;

  for (i = 0; i < len; i++)
  {
    xs = state[++x];
    ys = state[y += xs];
    state[x] = ys;
    state[y] = xs;
    *dst++ = *src++ ^ state[(unsigned char)(xs + ys)];
  }

  key->x = x;
  key->y = y;
}


/**************************************************************************
 * RC4 interface for SKIP                                                 *
 **************************************************************************/

/*
 * RC4' works with arbitrarily long keys.
 * SKIP uses fix key lens per algorithm hence we choose some key size for RC4'
 */
#define KEYLEN_40   5
#define KEYLEN_128 16

static int cryptrc4_init(void)
{
  return 0;
}

static int cryptrc4_exit(void)
{
  return 0;
}

static int cryptrc4_statelen(void)
{
  return sizeof(rc4_key);
}

static int cryptrc4_blocklen(void)
{
  return 1;
}

static int cryptrc4_40_keylen(void)
{
  return KEYLEN_40;
}

static int cryptrc4_128_keylen(void)
{
  return KEYLEN_128;
}

static int cryptrc4_MIlen(void)
{
  return 0;  /* Not used for stream ciphers */
}

static int cryptrc4_40_setkey(u_char *key, u_char *stateptr)
{
  rc4_setkey(key, KEYLEN_40, (rc4_key *)stateptr);

  return 0;
}

static int cryptrc4_128_setkey(u_char *key, u_char *stateptr)
{
  rc4_setkey(key, KEYLEN_128, (rc4_key *)stateptr);

  return 0;
}

static int cryptrc4_crypt(u_char *data, int len, u_char *stateptr)
{
  rc4_crypt(data, data, len, (rc4_key *)stateptr);

  return 0;
}

static int cryptrc4_fastforward(u_char *stateptr, u_int amount)
{
  rc4_fastforward((rc4_key *)stateptr, amount);

  return 0;
}

/* Information structure containing pointers to algorithm functions */
struct crypt_algorithm cryptrc4_40 =
{
  cryptrc4_init, cryptrc4_exit,
  cryptrc4_statelen, cryptrc4_statelen,
  cryptrc4_blocklen, cryptrc4_40_keylen, cryptrc4_MIlen,
  cryptrc4_40_setkey, cryptrc4_40_setkey,
  cryptrc4_crypt, cryptrc4_crypt,
  cryptrc4_fastforward
};

struct crypt_algorithm cryptrc4_128 =
{
  cryptrc4_init, cryptrc4_exit,
  cryptrc4_statelen, cryptrc4_statelen,
  cryptrc4_blocklen, cryptrc4_128_keylen, cryptrc4_MIlen,
  cryptrc4_128_setkey, cryptrc4_128_setkey,
  cryptrc4_crypt, cryptrc4_crypt,
  cryptrc4_fastforward
};
