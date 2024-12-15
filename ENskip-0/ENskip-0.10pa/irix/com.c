/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <sys/proc.h>

static struct queue *daemonqueue;

int com_register(struct queue *q)
{
  daemonqueue = q;

  return 0;
}

int com_unregister(struct queue *q)
{
  if (daemonqueue == q)
    daemonqueue = NULL;

  return 0;
}

int com_signal(int sig)
{
  mblk_t *m;

  if (daemonqueue && (m = allocb(1, BPRI_LO)))
  {
    m->b_datap->db_type = M_SIG;
    *m->b_wptr++ = sig;
    qreply(daemonqueue, m);
  }

  return 0;
}
