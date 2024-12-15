/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include "memblk.h"
#include "longlong.h"
#include "crypt.h"

#define STREAM_MILEN   8
/* Number of old states to be kept for decryption (MUST BE > 1) */
#define STREAM_DECRYPTSTATES 4
/* Limit amount of fast forwarding to avoid temporary lockups */
#define STREAM_MAXFASTFORWARD 100000
static u_char stream_maxfastforward[8] =
{
  0, 0, 0, 0,
  (STREAM_MAXFASTFORWARD >> 24) & 0xff, (STREAM_MAXFASTFORWARD >> 16) & 0xff,
  (STREAM_MAXFASTFORWARD >>  8) & 0xff, (STREAM_MAXFASTFORWARD      ) & 0xff
};

/*
 * Minimum state descriptions for cryptographic algorithms in SKIP
 */
struct state
{
  int alg;
  u_char state[1];  /* Actual layout will be: key, algorithm state */
};

struct streamdecryptstate
{
  int alg;
  int current;
  u_char state[1];  /* Actual layout will be: key, MI, algorithm state */
};

/* Precalculated values on init */
static int maxencryptstatelen;
static int maxdecryptstatelen;
static int maxblocklen;
static int maxkeylen;
static int maxMIlen;

static int maxblockencryptstatelen;
static int maxblockdecryptstatelen;
static int maxblockblocklen;
static int maxblockkeylen;
static int maxblockMIlen;

static int cryptalg_maxblockencryptstatelen(void)
{
  return maxblockencryptstatelen;
}

static int cryptalg_maxblockdecryptstatelen(void)
{
  return maxblockdecryptstatelen;
}

static int cryptalg_maxblockblocklen(void)
{
  return maxblockblocklen;
}

static int cryptalg_maxblockkeylen(void)
{
  return maxblockkeylen;
}

static int cryptalg_maxblockMIlen(void)
{
  return maxblockMIlen;
}

static struct crypt_algorithm cryptalg_maxblock =
{
  NULL, NULL,
  cryptalg_maxblockencryptstatelen, cryptalg_maxblockdecryptstatelen,
  cryptalg_maxblockblocklen, cryptalg_maxblockkeylen, cryptalg_maxblockMIlen, 
  NULL, NULL, NULL, NULL, NULL
};

static int cryptalg_maxencryptstatelen(void)
{
  return maxencryptstatelen;
}

static int cryptalg_maxdecryptstatelen(void)
{
  return maxdecryptstatelen;
}

static int cryptalg_maxblocklen(void)
{
  return maxblocklen;
}

static int cryptalg_maxkeylen(void)
{
  return maxkeylen;
}

static int cryptalg_maxMIlen(void)
{
  return maxMIlen;
}

static struct crypt_algorithm cryptalg_max =
{
  NULL, NULL,
  cryptalg_maxencryptstatelen, cryptalg_maxdecryptstatelen,
  cryptalg_maxblocklen, cryptalg_maxkeylen, cryptalg_maxMIlen, 
  NULL, NULL, NULL, NULL, NULL
};

/* Mapping from SKIP algorithm number to algorithm definition */
static struct crypt_algorithm *algorithm(int alg)
{
  /* Supported algorithms */
  extern struct crypt_algorithm cryptdes;
  extern struct crypt_algorithm cryptrc4_40;
  extern struct crypt_algorithm cryptrc4_128;
  extern struct crypt_algorithm crypt3des_2;
  extern struct crypt_algorithm crypt3des_3;
  extern struct crypt_algorithm cryptidea;
  extern struct crypt_algorithm cryptsimple;

  struct crypt_algorithm *a = (struct crypt_algorithm *)0;

  switch (alg)
  {
    case CRYPTALG_DES:      a = &cryptdes;     break; /* DES-CBC block */
    case CRYPTALG_RC4_40:   a = &cryptrc4_40;  break; /* RC4' stream */
    case CRYPTALG_RC4_128:  a = &cryptrc4_128; break; /* RC4' stream */
    case CRYPTALG_3DES_2:   a = &crypt3des_2;  break; /* DES-CBC block */
    case CRYPTALG_3DES_3:   a = &crypt3des_3;  break; /* DES-CBC block */
    case CRYPTALG_IDEA:     a = &cryptidea;    break; /* IDEA-CBC block */
    case CRYPTALG_SIMPLE:   a = &cryptsimple;  break; /* Simple stream */
    case CRYPTALG_MAX:      a = &cryptalg_max; break; /* Max */
    case CRYPTALG_MAXBLOCK: a = &cryptalg_maxblock; break; /* Max block */
  }

  return a;
}

/*
 * Initialize all crypt algorithms, return failure if one fails
 */
int crypt_init(void)
{
  int i, result = 0;

  for (i = 0; i < CRYPTALG_MAX; i++)
  {
    struct crypt_algorithm *a;

    if ((a = algorithm(i)))
    {
      if (a->init() == 0)
      {
        /* Calculate maximum size of misc algorithm values */
        int estatelen = a->encryptstatelen();
        int dstatelen = a->decryptstatelen();
        int blocklen = a->blocklen();
        int keylen = a->keylen();
        int MIlen = a->MIlen();

        if (blocklen == 1)  /* Stream Cipher, special states */
	{
          MIlen = STREAM_MILEN;

          /* state and streamdecryptstate are a bit too much, no problem... */
	  estatelen = sizeof(struct state) + keylen + estatelen;
	  dstatelen = sizeof(struct streamdecryptstate) +
                      STREAM_DECRYPTSTATES * (keylen + MIlen + dstatelen);
	}
        else  /* Store max values for block ciphers */
        {
          if (estatelen > maxblockencryptstatelen)
            maxblockencryptstatelen = estatelen;
          if (dstatelen > maxblockdecryptstatelen)
            maxblockdecryptstatelen = dstatelen;
          if (blocklen > maxblockblocklen)
            maxblockblocklen = blocklen;
          if (keylen > maxblockkeylen)
            maxblockkeylen = keylen;
          if (MIlen > maxblockMIlen)
            maxblockMIlen = MIlen;
        }

        if (estatelen > maxencryptstatelen)
          maxencryptstatelen = estatelen;
        if (dstatelen > maxdecryptstatelen)
          maxdecryptstatelen = dstatelen;
        if (blocklen > maxblocklen)
          maxblocklen = blocklen;
        if (keylen > maxkeylen)
          maxkeylen = keylen;
        if (MIlen > maxMIlen)
          maxMIlen = MIlen;
      }
      else
      {
        result = -1;
        break;  /* Error! Bail out */
      }
    }
  }

  return result;
}

/*
 * Cleanup all crypt algorithms, return failure if one fails
 */
int crypt_exit(void)
{
  int i, result = 0;

  for (i = 0; i < CRYPTALG_MAX; i++)
  {
    struct crypt_algorithm *a;

    if ((a = algorithm(i)))
      result |= a->exit();
  }

  return result;
}

int crypt_encryptstatelen(int alg)
{
  int len = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)))
  {
    len = a->encryptstatelen();

    if (a->blocklen() == 1)  /* Stream Cipher, special states */
      len = sizeof(struct state) + a->keylen() + len;
  }

  return len;
}

int crypt_decryptstatelen(int alg)
{
  int len = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)))
  {
    len = a->decryptstatelen();

    if (a->blocklen() == 1)  /* Stream Cipher, special states */
    {
      len = sizeof(struct streamdecryptstate) +
	    STREAM_DECRYPTSTATES * (a->keylen() + STREAM_MILEN + len);
    }
  }

  return len;
}

int crypt_blocklen(int alg)
{
  int len = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)))
    len = a->blocklen();

  return len;
}

int crypt_keylen(int alg)
{
  int len = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)))
    len = a->keylen();

  return len;
}

int crypt_MIlen(int alg)
{
  int len = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)))
    len = (a->blocklen() == 1) ? STREAM_MILEN : a->MIlen();

  return len;
}

/*
 * Encryption routine for block ciphers handling memblk structure
 * Length of data to process (m->len) has to be multiple of block size
 */
static int blockencrypt(int alg, struct crypt_algorithm *a, u_char *stateptr,
                        u_char *key, u_char *oldIV, u_char *newIV,
			struct memblk *m)
{
  struct state *state = (struct state *)stateptr;
  int blocklen = a->blocklen();
  int keylen = a->keylen();

  /* Key has changed? */
  if ((state->alg != alg) || (MEMCMP(state->state, key, keylen) != 0))
  {
    state->alg = alg;
    MEMCPY(state->state, key, keylen);
    a->setencryptkey(key, stateptr);
  }

  MEMCPY(oldIV, newIV, blocklen);

  while (m->len > 0)
  {
    u_char *data = BLKSTART(m);
    int len = m->ms->len - m->offset;


    if (len > m->len)  /* (1) More data in memblk than we need to process? */
      len = m->len;


    while (len >= blocklen)  /* Main encryption loop */
    {
      MEMEOR(data, newIV, blocklen);  /* CBC mode */
      a->encrypt(data, blocklen, stateptr);
      MEMCPY(newIV, data, blocklen);

      data += blocklen;
      m->offset += blocklen;
      m->len -= blocklen;
      len -= blocklen;
    }


    if (len > 0)  /* Partial data left? Always false if (1) above was true */
    {
      struct memseg *s;
      u_char *d;
      int i, l;

      /* Collect data up to block size (May span multiple segments) */
      for (i = 0, s = m->ms, l = len, d = data; i < blocklen; i++)
      {
        newIV[i] ^= *d++;

	if (--l <= 0)
	{
          s++;
	  d = s->ptr;
	  l = s->len;
	}
      }

      a->encrypt(newIV, blocklen, stateptr);

      /* Copy encrypted data back to memsegs */
      for (i = 0, s = m->ms, l = len; i < blocklen; i++)
      {
	*data++ = newIV[i];

	if (--l <= 0)
	{
          s++;
	  data = s->ptr;
	  l = s->len;
	}
      }

      /* Advance after processed data */
      m->ms = s;
      m->offset = data - s->ptr;
      m->len -= blocklen;
    }
    else if (m->offset == m->ms->len)  /* More data to be done? */
    {
      m->offset = 0;
      m->ms++;
    }
  }

  
  return 0;
}

/*
 * Decryption routine for block ciphers handling memblk structure
 * Length of data to process (m->len) has to be multiple of block size
 */
static int blockdecrypt(int alg, struct crypt_algorithm *a, u_char *stateptr,
                        u_char *key, u_char *IV, struct memblk *m)
{
  struct state *state = (struct state *)stateptr;
  int blocklen = a->blocklen();
  int keylen = a->keylen();
  u_char buf[MAXBLOCKLEN], buf2[MAXBLOCKLEN];

  /* Key has changed? */
  if ((state->alg != alg) || (MEMCMP(state->state, key, keylen) != 0))
  {
    state->alg = alg;
    MEMCPY(state->state, key, keylen);
    a->setdecryptkey(key, stateptr);
  }

  while (m->len > 0)
  {
    u_char *data = BLKSTART(m);
    int len = m->ms->len - m->offset;


    if (len > m->len)  /* (1) More data in memblk than we need to process? */
      len = m->len;


    while (len >= blocklen)  /* Main decryption loop */
    {
      MEMCPY(buf, data, blocklen);  /* CBC mode */
      a->decrypt(data, blocklen, stateptr);
      MEMEOR(data, IV, blocklen);
      MEMCPY(IV, buf, blocklen);

      data += blocklen;
      m->offset += blocklen;
      m->len -= blocklen;
      len -= blocklen;
    }


    if (len > 0)  /* Partial data left? Always false if (1) above was true */
    {
      struct memseg *s;
      int i, l;
      u_char *d;

      /* Collect data up to block size (May span multiple segments) */
      for (i = 0, s = m->ms, l = len, d = data; i < blocklen; i++)
      {
        buf[i] = buf2[i] = *d++;

	if (--l == 0)
	{
          s++;
	  d = s->ptr;
	  l = s->len;
	}
      }

      a->decrypt(buf, blocklen, stateptr);
      MEMEOR(buf, IV, blocklen);
      MEMCPY(IV, buf2, blocklen);

      /* Copy decrypted data back to memsegs */
      for (i = 0, s = m->ms, l = len; i < blocklen; i++)
      {
	*data++ = buf[i];

	if (--l == 0)
	{
          s++;
	  data = s->ptr;
	  len = s->len;
	}
      }

      /* Advance after processed data */
      m->ms = s;
      m->offset = data - s->ptr;
      m->len -= blocklen;
    }
    else if (m->offset == m->ms->len)  /* More data to be done? */
    {
      m->offset = 0;
      m->ms++;
    }
  }

  return 0;
}

static int streamencrypt(int alg, struct crypt_algorithm *a, u_char *stateptr,
                         u_char *key, u_char *oldIV, u_char *newIV,
			 struct memblk *m)
{
  struct state *state = (struct state *)stateptr;
  int keylen = a->keylen();

  /* Key has changed? */
  if ((state->alg != alg) || (MEMCMP(state->state, key, keylen) != 0))
  {
    state->alg = alg;
    MEMCPY(state->state, key, keylen);
    MEMZERO(newIV, STREAM_MILEN);
    a->setencryptkey(key, state->state + keylen);
  }

  MEMCPY(oldIV, newIV, STREAM_MILEN);
  longlong_inc(newIV, m->len);  /* Doesn't matter if before or after loop */

  while (m->len > 0)
  {
    u_char *data = BLKSTART(m);
    int len = m->ms->len - m->offset;

    if (len <= m->len)
    {
      m->ms++;
      m->offset = 0;
    }
    else
      m->offset += (len = m->len);

    a->encrypt(data, len, state->state + keylen);
    m->len -= len;
  }

  return 0;
}

static int streamdecrypt(int alg, struct crypt_algorithm *a, u_char *stateptr,
                        u_char *key, u_char *IV, struct memblk *m)
{
  int result = 0;
  struct streamdecryptstate *ds = (struct streamdecryptstate *)stateptr;
  int cur;
  int hole;
  int keylen = a->keylen();
  int decryptstatelen = a->decryptstatelen();
  int entrylen = keylen + STREAM_MILEN + decryptstatelen;
  u_char *s, *new;


  /* State previously used by same algorithm? No -> Initialize */
  if (ds->alg != alg)
  {

    ds->alg = alg;
    ds->current = 0;

    for (cur = 0, s = ds->state; cur < STREAM_DECRYPTSTATES; cur++)
    {
      MEMCPY(s, key, keylen);
      s += keylen;
      MEMZERO(s, STREAM_MILEN);
      s += STREAM_MILEN;
      a->setdecryptkey(key, s);
      s += decryptstatelen;
    }
  }

  cur = ds->current;
  result = -1;


  /* Search for same key */
  do
  {
    s = ds->state + cur * entrylen;

    if (MEMCMP(s, key, keylen) == 0)
    {

      /* Calculate hole size (Only accept 0 <= hole < LONGLONG_MAXSUB) */
      if ((hole = longlong_sub(IV, s + keylen)) >= 0)
      {
        /* Found the newest state with a hole >= 0 */

        if (hole == 0)  /* Ok, no hole */
          result = 0;
        else if (hole <= STREAM_MAXFASTFORWARD)  /* Valid fastforward range? */
        {
          if (cur == ds->current)  /* Use current? -> Update ds->current */
            ds->current = (cur + 1) % STREAM_DECRYPTSTATES;
          cur = (cur + 1) % STREAM_DECRYPTSTATES;
	  new = ds->state + cur * entrylen;


	  /* Copy old entry, fast forward and adjust MI to new position */
          MEMCPY(new, s, entrylen);
          a->streamfastforward(new + keylen + STREAM_MILEN, (u_int)hole);
          longlong_inc(new + keylen, hole);
	  s = new;
          result = 0;
        }

        break;  /* We either could create suitable state or we fail */
      }
    }
  } while ((cur = (cur + STREAM_DECRYPTSTATES - 1) % STREAM_DECRYPTSTATES) !=
	   ds->current);

  if (result != 0)  /* No suitable state yes, try to create new */
  {

    if (longlong_cmp(IV, stream_maxfastforward) <= 0)
    {
      /* New state */
      cur = ds->current = (ds->current + 1) % STREAM_DECRYPTSTATES;
      s = ds->state + cur * entrylen;
      MEMCPY(s, key, keylen);
      MEMCPY(s + keylen, IV, STREAM_MILEN);
      a->setdecryptkey(key, s + keylen + STREAM_MILEN);
      a->streamfastforward(s + keylen + STREAM_MILEN, longlong_val(IV));
      result = 0;
    }
  }

  if (result == 0)  /* We have a suitable state, decrypt */
  {

    stateptr = s + keylen + STREAM_MILEN;
    longlong_inc(s + keylen, (u_int32)m->len);

    while (m->len > 0)
    {
      u_char *data = BLKSTART(m);
      int len = m->ms->len - m->offset;

      if (len <= m->len)
      {
        m->ms++;
        m->offset = 0;
      }
      else
        m->offset += (len = m->len);

      a->decrypt(data, len, stateptr);
      m->len -= len;
    }
  }


  return result;
}

int crypt_encrypt(int alg, u_char *stateptr, u_char *key,
		  u_char *oldMI, u_char *newMI, struct memblk *m)
{
  int result = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)) && (a->encrypt))
  {
    if (a->blocklen() > 1)
      result = blockencrypt(alg, a, stateptr, key, oldMI, newMI, m);
    else
      result = streamencrypt(alg, a, stateptr, key, oldMI, newMI, m);
  }

  return result;
}

int crypt_decrypt(int alg, u_char *priv, u_char *key, u_char *MI,
                  struct memblk *m)
{
  int result = -1;
  struct crypt_algorithm *a;

  if ((a = algorithm(alg)) && (a->decrypt))
  {
    if (a->blocklen() > 1)
      result = blockdecrypt(alg, a, priv, key, MI, m);
    else
      result = streamdecrypt(alg, a, priv, key, MI, m);
  }

  return result;
}
