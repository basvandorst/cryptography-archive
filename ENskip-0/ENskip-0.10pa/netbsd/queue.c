/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <netinet/in.h>

#include "skip_defs.h"
#include "skipcache.h"
#include "queue.h"
#include "interface.h"

int queue_enqueuein(struct skipcache *c, struct mbuf *m)
{
  int result = 0;
  int old;

  old = splimp();
  /* limit reached? */
  if ((c->inq.ifq_maxlen > 0) && (c->inq.ifq_maxlen <= c->inq.ifq_len))
    result = -1;
  else
    IF_ENQUEUE(&c->inq, m);
  splx(old);

  return result;
}

int queue_enqueueout(struct skipcache *c, struct mbuf *m)
{
  int result = 0;
  int old;

  old = splimp();
  /* limit reached? */
  if ((c->outq.ifq_maxlen > 0) && (c->outq.ifq_maxlen <= c->outq.ifq_len))
    result = -1;
  else
    IF_ENQUEUE(&c->outq, m);
  splx(old);

  return result;
}

int queue_free(void *cptr)
{
  struct skipcache *c = (struct skipcache *)cptr;
  struct mbuf *m;
  int old;

  old = splimp();
  while (c->outq.ifq_head)
  {
    IF_DEQUEUE(&c->outq, m);
    m_freem(m);
  }

  while (c->inq.ifq_head)
  {
    IF_DEQUEUE(&c->inq, m);
    m_freem(m);
  }
  splx(old);

  return 0;
}

int queue_feed(void *cptr)
{
  struct skipcache *c = (struct skipcache *)cptr;
  struct mbuf *m;

  do
  {
    int old = splimp();
    IF_DEQUEUE(&c->outq, m);
    splx(old);
    if (m)
      interface_outfeedback(m);
  } while (m);

  do
  {
    int old = splimp();
    IF_DEQUEUE(&c->inq, m);
    splx(old);
    if (m)
      interface_infeedback(m);
  } while (m);

  return 0;
}

