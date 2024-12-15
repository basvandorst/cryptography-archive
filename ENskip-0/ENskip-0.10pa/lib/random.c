/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "skip_defs.h"
#include "memblk.h"
#include "crypt.h"
#include "sign.h"
#include "skipcache.h"
#include "random.h"

/**************************************************************************
 * RC4 Implementation from Usenet source                                  *
 **************************************************************************/
 
typedef struct rc4_key
{
  unsigned char state[256];
  unsigned char x, y;
} rc4_key;

static SEMTYPE semaphore;
static rc4_key state;
 
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

static void rc4_crypt(unsigned char *src, unsigned char *dst, int len, rc4_key *
key)
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

int random_hash(u_char *buf, int buflen, u_char *val, int vallen)
{
  int digestlen = sign_icvlen(SIGNALG_MD5);
  int len;

  for (len = 0; buflen > 0; len += digestlen, buflen -= digestlen)
  {
    struct memblk m;
    struct memseg ms;
    u_char hash[64];

    m.len = len;
    m.offset = 0;
    m.ms = &ms;
    ms.len = len;
    ms.ptr = buf;
    sign_sign(SIGNALG_MD5, val, vallen, &m, hash);
    MEMCPY(buf + len, hash, (digestlen > buflen) ? buflen : digestlen);
  }

  return 0;
}

/*
 * Generate a random packet key Kp for given skipcache entry
 */
int random_randomKp(struct skipcache *c)
{
  struct memblk m;
  struct memseg ms;
  int keylen, padlen, blocklen;
  u_char MI[MAXBLOCKLEN];
  static u_char es[1024];  /* Encrypt state */

  if ((c->flags) & SKIP_VALIDKEY)
  {
    MEMZERO(MI, sizeof(MI));

    keylen = crypt_keylen(c->enskip_Kp_alg);
    blocklen = crypt_blocklen(c->enskip_Kij_alg);
    if ((padlen = keylen % blocklen))
      padlen = blocklen - padlen;

    c->enskip_Kp_ttl = c->enskip_Kp_maxttl;
    c->enskip_Kp_bytes = 0;
    random_random(c->data + c->enskip_Kp, keylen);
    MEMCPY(c->data + c->enskip_Kp_Kij, c->data + c->enskip_Kp, keylen);
    m.ms = &ms;
    m.len = keylen + padlen;
    m.offset = 0;
    ms.ptr = c->data + c->enskip_Kp_Kij;
    ms.len = m.len;
    crypt_encrypt(c->enskip_Kij_alg, es, c->data + c->Kij, MI, MI, &m);
  }

  return 0;
}

int random_init(void)
{
  SEEDTYPE seed;
  u_char rc4seed[256];

  SEMALLOC(semaphore);
  SEMINIT(semaphore);
  SEEDINIT(&seed);
  random_hash(rc4seed, sizeof(rc4seed), (u_char *)&seed, sizeof(seed));
  MEMZERO(&seed, sizeof(seed));
  rc4_setkey(rc4seed, sizeof(rc4seed), &state);
  MEMZERO(rc4seed, sizeof(rc4seed));
  return 0;
}

int random_exit(void)
{
  SEMFREE(semaphore);
  return 0;
}

int random_random(u_char *buf, int len)
{
  SEMLOCK(semaphore);
  rc4_crypt(buf, buf, len, &state);
  SEMUNLOCK(semaphore);
  return 0;
}
