/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <sys/stream.h>

#include "skip_defs.h"
#include "skipcache.h"
#include "queue.h"
#include "interface.h"

static SEMTYPE semaphore;

int queue_enqueuein(struct skipcache *c, mblk_t *m)
{
  int result = -1;

  SEMLOCK(semaphore);
  /* limit reached? */
  if ((c->inq.ifq_maxlen <= 0) || (c->inq.ifq_maxlen > c->inq.ifq_len))
  {
    /* enqueue */
    m->b_next = NULL;
    if (c->inq.ifq_tail == NULL)
    {
      m->b_prev = NULL;
      c->inq.ifq_head = c->inq.ifq_tail = m;
    }
    else
    {
      m->b_prev = c->inq.ifq_tail;
      c->inq.ifq_tail->b_next = m;
      c->inq.ifq_tail = m;
    }

    c->inq.ifq_len++;
    result = 0;
  }
  SEMUNLOCK(semaphore);

  return result;
}

int queue_enqueueout(struct skipcache *c, mblk_t *header, mblk_t *m)
{
  int result = -1;

  SEMLOCK(semaphore);
  /* limit reached? */
  if ((c->outq.ifq_maxlen <= 0) || (c->outq.ifq_maxlen > c->outq.ifq_len))
  {
    /* Queue both header and m */
    if (header)
    {
      linkb(header, m);
      m = header;
    }

    /* enqueue */
    m->b_next = NULL;
    if (c->outq.ifq_tail == NULL)
    {
      m->b_prev = NULL;
      c->outq.ifq_head = c->outq.ifq_tail = m;
    }
    else
    {
      m->b_prev = c->outq.ifq_tail;
      c->outq.ifq_tail->b_next = m;
      c->outq.ifq_tail = m;
    }

    c->outq.ifq_len++;
    result = 0;
  }
  SEMUNLOCK(semaphore);

  return result;
}

int queue_free(void *x)
{
  struct skipcache *c = (struct skipcache *)x;
  mblk_t *walk = NULL, *next;

  SEMLOCK(semaphore);
  for (walk = c->inq.ifq_head; walk; walk = next)
  {
    next = walk->b_next;
    freemsg(walk);
  }
  c->inq.ifq_head = c->inq.ifq_tail = NULL;
  c->inq.ifq_len = 0;

  for (walk = c->outq.ifq_head; walk; walk = next)
  {
    next = walk->b_next;
    freemsg(walk);
  }
  c->outq.ifq_head = c->outq.ifq_tail = NULL;
  c->outq.ifq_len = 0;
  SEMUNLOCK(semaphore);

  return 0;
}

int queue_feed(void *x)
{
  struct skipcache *c = (struct skipcache *)x;
  mblk_t *walk = NULL, *next;

  SEMLOCK(semaphore);
  for (walk = c->inq.ifq_head; walk; walk = next)
  {
    next = walk->b_next;
    interface_infeedback(c->interface, walk);
  }
  c->inq.ifq_head = c->inq.ifq_tail = NULL;
  c->inq.ifq_len = 0;

  for (walk = c->outq.ifq_head; walk; walk = next)
  {
    next = walk->b_next;
    interface_outfeedback(c->interface, walk);
  }
  c->outq.ifq_head = c->outq.ifq_tail = NULL;
  c->outq.ifq_len = 0;
  SEMUNLOCK(semaphore);

  return 0;
}

int queue_init(void)
{
  SEMALLOC(semaphore);
  SEMINIT(semaphore);
  return 0;
}

int queue_exit(void)
{
  SEMFREE(semaphore);
  return 0;
}
