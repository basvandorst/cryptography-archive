/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/signalvar.h>
#include <netinet/in.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "ipsp.h"
#include "interface.h"
#include "com.h"

struct skipcallparam
{
  int cmd;
  u_char *par;
  int len;
};

static pid_t daemonpid;

int com_signal(int sig)
{
  if (daemonpid)
  {
    struct proc *proc = pfind(daemonpid);

    if (proc)
      psignal(proc, sig);
    else
      daemonpid = 0;  /* Daemon probably died, don't send any more signals */
  }

  return 0;
}

int com_skipcall(struct proc *p, struct skipcallparam *uap, int retval[])
{
  int result = EINVAL;
  u_char ip[4];

  if (suser(p->p_ucred, &p->p_acflag))
    return EPERM;

  switch (uap->cmd)
  {
    case SKIPCMD_UPDATE:
    {
      struct skipcache *c;
      int userlen = sizeof(*c) - CACHE_QSIZE;
      int kernellen = uap->len + CACHE_QSIZE;
      int datalen = uap->len - userlen;

      if ((MALLOC(c, struct skipcache *, kernellen,  M_TEMP, M_WAITOK)))
      {
        if ((copyin(uap->par, c, userlen) == 0) &&
	    (copyin(uap->par + userlen, c + 1, datalen) == 0))
        {
	  MEMZERO(((u_char *)c) + userlen, CACHE_QSIZE);
          c->data = (u_char *)(c + 1);
          result = -skipcache_update(c);
	}
      }
      else
	result = ENOMEM;

      break;
    }

    case SKIPCMD_REMOVE:
    {
      if (copyin(uap->par, ip, sizeof(ip)) == 0)
        result = -skipcache_remove(ip);
      break;
    }

    case SKIPCMD_ATTACH:
    {
      if (copyin(uap->par, ip, sizeof(ip)) == 0)
        result = -interface_attach(ip);
      break;
    }

    case SKIPCMD_DETACH:
    {
      if (copyin(uap->par, ip, sizeof(ip)) == 0)
        result = -interface_detach(ip);
      break;
    }

    case SKIPCMD_GETREQUESTS:
    {
      u_char *buf;
      int used = 0;

      if ((buf = KALLOC(uap->len)))
      {
	if ((used = skipcache_getrequests(buf, uap->len - sizeof(used))) > 0)
        {
          if (copyout(&used, uap->par, sizeof(used)) == 0)
  	    result = copyout(buf, uap->par + sizeof(used), used);
        }
	KFREE(buf, uap->len);
      }
      break;
    }

    case SKIPCMD_REGISTER:
    {
      daemonpid = p->p_pid;
      break;
    }

    case SKIPCMD_STATIPSP:
    {
      result = copyout(&ipsp_stat, uap->par, sizeof(ipsp_stat));
      break;
    }

    case SKIPCMD_STATCACHE:
    {
      result = copyout(&skipcache_stat, uap->par, sizeof(skipcache_stat));
      break;
    }

    case SKIPCMD_FLUSH:
    {
      result = skipcache_flush();
      break;
    }
  }

  return result;
}

