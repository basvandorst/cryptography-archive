/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "skip_defs.h"
#include "skipcache.h"
#include "longlong.h"

/* Sequence numbers more than MAXVAL from current are considered (-:C */
#define MAXVAL 1024

int sequence_inc(u_char *seq)
{
  return longlong_inc(seq, 1);
}

int sequence_check(struct sequence_history *h, u_char *hbuf, u_char *seq)
{
  int d;

  if (longlong_cmp(seq, h->last) <= 0)  /* seq <= last */
  {
    if (((d = longlong_sub(h->last, seq)) == -1) || (d >= h->len))
      return -1;  /* Difference too large */
      
    if (hbuf[(h->len + h->index - d) % h->len])
      return -1;  /* Already received this sequence number */

    hbuf[(h->len + h->index - d) % h->len] = 1;
  }
  else  /* seq > last */
  {
    if (((d = longlong_sub(seq, h->last)) == -1) || (d >= h->len))
    {
      /* Gap too large, clear whole history... */
      MEMZERO(hbuf, h->len);
    }
    else
    {
      /* Mark all from last to seq as not received */
      while (d--)
      {
        h->index = (h->index + 1) % h->len;
        hbuf[h->index] = 0;
      }
    }

    MEMCPY(h->last, seq, SKIP_SEQSIZE);
    hbuf[h->index] = 1;
  }

  return 0;  /* Sequence number is ok */
}

