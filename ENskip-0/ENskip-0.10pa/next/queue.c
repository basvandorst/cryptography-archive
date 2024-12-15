/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <sys/socket.h>
#include <net/if.h>

#include "config.h"
#include "skip_defs.h"
#include "skipcache.h"
#include "queue.h"
#include "interface.h"

int queue_enqueuein(struct skipcache *c, struct mbuf *header, struct mbuf *m)
{
  int result = -1;
  int old;

  old = spl6();
  /* limit reached? */
  if ((c->inq.ifq_maxlen <= 0) || (c->inq.ifq_maxlen > c->inq.ifq_len))
  {
    if (header)
    {
      header->m_next = m;
      m = header;
    }
    IF_ENQUEUE(&c->inq, m);
    result = 0;
  }
  splx(old);

  return result;
}

int queue_enqueueout(struct skipcache *c, struct mbuf *header, struct mbuf *m)
{
  int result = -1;
  int old;

  old = spl6();
  /* limit reached? */
  if ((c->outq.ifq_maxlen <= 0) || (c->outq.ifq_maxlen > c->outq.ifq_len))
  {
    if (header)
    {
      header->m_next = m;
      m = header;
    }

    IF_ENQUEUE(&c->outq, m);
    result = 0;
  }
  splx(old);

  return result;
}

int queue_free(void *x)
{
  struct skipcache *c = (struct skipcache *)x;
  struct mbuf *m;
  int old;

  old = spl6();
  while (c->outq.ifq_head)
  {
    IF_DEQUEUE(&c->outq, m);
    SKIP_MFREE(m);
  }

  while (c->inq.ifq_head)
  {
    IF_DEQUEUE(&c->inq, m);
    SKIP_MFREE(m->m_next);
    SKIP_MFREE(m);
  }
  splx(old);

  return 0;
}

int queue_feed(void *x)
{
  struct skipcache *c = (struct skipcache *)x;
  struct mbuf *m;

  do
  {
    int old = spl6();
    IF_DEQUEUE(&c->outq, m);
    splx(old);
    if (m)
      interface_outfeedback(m);
  } while (m);

  do
  {
    int old = spl6();
    IF_DEQUEUE(&c->inq, m);
    splx(old);
    if (m)
      interface_infeedback(m);
  } while (m);

  return 0;
}

