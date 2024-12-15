/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <mach/port.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/proc.h>

#include "config.h"
#include "skip_defs.h"
#include "skipcache.h"
#include "com.h"
#include "interface.h"
#include "memblk.h"
#include "ipsp.h"

static int daemonpid = 0;

/*
 * register a port that the requests will be sent to
 */
int com_registerpid(port_t port, port_t priv, int pid)
{
  /* superuser privileges? */
  if (priv == PORT_NULL)
    return -EACCES;

  daemonpid = pid;
  return 0;
}

/*
 * update an entry in the hash table
 */
int com_update(port_t port, port_t priv, u_char *arg, u_int len)
{
  int r;
  struct skipcache *new, *c = (struct skipcache *)arg;
  int user_struct_len = (sizeof(struct skipcache) - CACHE_QSIZE);

  /* superuser privileges? */
  if (priv == PORT_NULL)
    return -EACCES;

  /* same length (without queue stuff)? */
  if (len != (c->datalen + user_struct_len))
    return -EINVAL;

  if ((new = KALLOC(sizeof(struct skipcache) + c->datalen)) == NULL)
    return -ENOMEM;

  MEMCPY(new, arg, user_struct_len);
  new->data = (((u_char *)new) + sizeof(*new));
  MEMZERO((((u_char *)new) + user_struct_len), CACHE_QSIZE);
  MEMCPY(new->data, (arg + user_struct_len), c->datalen);

  if ((r = skipcache_update(new)) < 0)
    KFREE(new, sizeof(struct skipcache) + c->datalen);

  return r;
}

/*
 * remove an entry
 */
int com_remove(port_t port, port_t priv, u_char *IPaddr)
{
  /* superuser privileges? */
  if (priv == PORT_NULL)
    return -EACCES;

  return skipcache_remove(IPaddr);
}

/*
 * flush primary cache
 */
int com_flush(port_t port, port_t priv)
{
  /* superuser privileges? */
  if (priv == PORT_NULL)
    return -EACCES;
 
  return skipcache_flush();
}

/*
 * attach SKIP to interface with given IP address
 */
int com_attach(port_t port, port_t priv, u_char *IPaddr)
{
  /* superuser privileges? */
  if (priv == PORT_NULL)
    return -EACCES;

  return interface_attach(IPaddr);
}

/*
 * detach SKIP from interface with given IP address
 */
int com_detach(port_t port, port_t priv, u_char *IPaddr)
{
  /* superuser privileges? */
  if (priv == PORT_NULL)
    return -EACCES;

  return interface_detach(IPaddr);
}

int com_stat(port_t port, ipsp_stat_t *ipspstat, skipcache_stat_t *cachestat)
{
  *ipspstat = ipsp_stat;
  *cachestat = skipcache_stat;

  return 0;
}

int com_getrequests(port_t port, port_t priv, u_char *arg, u_int *len)
{
  int r;

  r = skipcache_getrequests(arg, *len);

  return (*len = r);
}

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
