/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <netinet/in_systm.h>
#include <netinet/in.h>

#include "skip_defs.h"
#include "skipcache.h"
#include "queue.h"
#include "interface.h"

int queue_enqueuein(struct skipcache *c, struct mbuf *m)
{
  int result = 0;

  IFQ_LOCK(&c->inq);
  /* limit reached? */
  if ((c->inq.ifq_maxlen > 0) && (c->inq.ifq_maxlen <= c->inq.ifq_len))
    result = -1;
  else
    IF_ENQUEUE_NOLOCK(&c->inq, m);
  IFQ_UNLOCK(&c->inq);

  return result;
}

int queue_enqueueout(struct skipcache *c, struct mbuf *m)
{
  int result = 0;

  IFQ_LOCK(&c->outq);
  /* limit reached? */
  if ((c->outq.ifq_maxlen > 0) && (c->outq.ifq_maxlen <= c->outq.ifq_len))
    result = -1;
  else
    IF_ENQUEUE_NOLOCK(&c->outq, m);
  IFQ_UNLOCK(&c->outq);

  return result;
}

int queue_free(void *cptr)
{
  struct skipcache *c = (struct skipcache *)cptr;
  struct mbuf *m;

  IFQ_LOCK(&c->outq);
  while (c->outq.ifq_head)
  {
    IF_DEQUEUE_NOLOCK(&c->outq, m);
    m_freem(m);
  }
  IFQ_UNLOCK(&c->outq);

  IFQ_LOCK(&c->inq);
  while (c->inq.ifq_head)
  {
    IF_DEQUEUE_NOLOCK(&c->inq, m);
    m_freem(m);
  }
  IFQ_UNLOCK(&c->inq);

  return 0;
}

int queue_feed(void *cptr)
{
  struct skipcache *c = (struct skipcache *)cptr;
  struct mbuf *m;

  do
  {
    IF_DEQUEUE(&c->outq, m);
    if (m)
      interface_outfeedback(m);
  } while (m);

  do
  {
    IF_DEQUEUE(&c->inq, m);
    if (m)
      interface_infeedback(m);
  } while (m);

  return 0;
}

int queue_init(struct skipcache *c)
{
  IFQ_INITLOCK(&c->inq);
  IFQ_INITLOCK(&c->outq);
  c->inq.ifq_maxlen = QUEUESIZE;
  c->outq.ifq_maxlen = QUEUESIZE;
  return 0;
}
