/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <sys/proc.h>

static pid_t daemonpid;

int com_register(pid_t pid)
{
  daemonpid = pid;
  return 0;
}

int com_signal(int sig)
{
  if (daemonpid)
  {
    proc_t *proc = prfind(daemonpid);

    if (proc)
      psignal(proc, sig);
    else
      daemonpid = 0;  /* Daemon probably died, don't send any more signals */
  }

  return 0;
}
