/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "memblk.h"
#include "crypt.h"

/*
 * Simple Crypt works with arbitrarily long keys.
 * SKIP uses fix key lens per algorithm hence we choose some key size
 */
#define KEYLEN 8

struct state
{
  int pos;
  u_char key[KEYLEN];
};

static int cryptsimple_init(void)
{
  return 0;
}

static int cryptsimple_exit(void)
{
  return 0;
}

static int cryptsimple_statelen(void)
{
  return sizeof(struct state);
}

static int cryptsimple_blocklen(void)
{
  return 1;
}

static int cryptsimple_keylen(void)
{
  return KEYLEN;
}

static int cryptsimple_MIlen(void)
{
  return 0;  /* Not used for stream ciphers */
}

static int cryptsimple_setkey(u_char *key, u_char *stateptr)
{
  struct state *state = (struct state *)stateptr;

  state->pos = 0;
  MEMCPY(state->key, key, KEYLEN);

  return 0;
}

static int cryptsimple_crypt(u_char *data, int len, u_char *stateptr)
{
  struct state *state = (struct state *)stateptr;
  int i, pos;

  pos = state->pos;

  for (i = 0; i < len; i++)
  {
    *data++ ^= state->key[pos];
    pos = (pos + 1) & (KEYLEN - 1);
  }

  state->pos = pos;

  return 0;
}

static int cryptsimple_fastforward(u_char *stateptr, u_int amount)
{
  struct state *state = (struct state *)stateptr;

  state->pos = (state->pos + amount) & (KEYLEN - 1);

  return 0;
}

/* Information structure containing pointers to algorithm functions */
struct crypt_algorithm cryptsimple =
{
  cryptsimple_init, cryptsimple_exit,
  cryptsimple_statelen, cryptsimple_statelen,
  cryptsimple_blocklen, cryptsimple_keylen, cryptsimple_MIlen,
  cryptsimple_setkey, cryptsimple_setkey,
  cryptsimple_crypt, cryptsimple_crypt,
  cryptsimple_fastforward
};
