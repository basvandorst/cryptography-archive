/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <sys/types.h>

#include "memblk.h"
#include "sign.h"

/* Precalculated values on init */
static int maxicvlen;

/* Mapping from SKIP algorithm number to algorithm definition */
static struct sign_algorithm *algorithm(int alg)
{
  /* Supported algorithms */
  extern struct sign_algorithm signmd5;

  struct sign_algorithm *a = (void *)0;

  switch (alg)
  {
    case SIGNALG_MD5:	a = &signmd5; break;	/* md5 message digest */
  }

  return a;
}

/*
 * Initialize all sign algorithms, return failure if one fails
 */
int sign_init(void)
{
  int i, result = 0;

  for (maxicvlen = i = 0; i < SIGNALG_MAX; i++)
  {
    struct sign_algorithm *a;

    if ((a = algorithm(i)))
    {
      if (a->init() == 0)
      {
        /* Calculate maximum size of misc algorithm values */
        int icvlen = a->icvlen();

        if (icvlen > maxicvlen)
          maxicvlen = icvlen;
      }
      else  /* An error occured, bail out */
      {
        result = -1;
        break;
      }
    }
  }

  return result;
}

/*
 * Cleanup all sign algorithms, return failure if one fails
 */
int sign_exit(void)
{
  int i, result = 0;

  for (i = 0; i < SIGNALG_MAX; i++)
  {
    struct sign_algorithm *a;

    if ((a = algorithm(i)))
      result |= a->exit();
  }

  return result;
}

int sign_icvlen(int alg)
{
  int len = -1;
  struct sign_algorithm *a;

  if (alg == SIGNALG_MAX)
    len = maxicvlen;
  else if ((a = algorithm(alg)))
    len = a->icvlen();

  return len;
}

int sign_sign(int alg, u_char *Kp, int Kplen, struct memblk *m, u_char *icv)
{
  struct sign_algorithm *a = algorithm(alg);

  return a ? a->sign(Kp, Kplen, m, icv) : -1;
}
