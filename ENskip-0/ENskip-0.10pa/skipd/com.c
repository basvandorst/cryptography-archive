/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#ifdef NeXT
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/netname.h>

#define PORTNAME        "skip"

static port_t p, hpriv;

void MsgError()
{
  fprintf(stderr, "Message error!\n");
  exit(1);
}

#include "../next/migUser.c"

static port_t portlookup()
{
  port_t port;
  kern_return_t r;

  r = netname_look_up(name_server_port, "", PORTNAME, &port);

  return (r == KERN_SUCCESS) ? port : PORT_NULL;
}

int com_update(struct skipcache *c)
{
  return skip_update(p, hpriv, (u_char *)c, (sizeof(struct skipcache) + c->datalen));
}

int com_remove(u_char *IPaddr)
{
  return skip_remove(p, hpriv, IPaddr);
}

int com_flush(void)
{
  return skip_flush(p, hpriv);
}

int com_attach(u_char *IPaddr)
{
  return skip_attach(p, hpriv, IPaddr);
}

int com_detach(u_char *IPaddr)
{
  return skip_detach(p, hpriv, IPaddr);
}

int com_getrequests(u_char *buf, int len)
{
  int r;

  r = skip_getrequests(p, hpriv, buf, &len);
  return (r > 0) ? (r / IPADDRSIZE) : r;    
}

int com_register()
{
  return skip_registerpid(p, hpriv, getpid());
}

int com_stat(ipsp_stat_t *ipspstat, skipcache_stat_t *cachestat)
{
  return skip_stat(p, ipspstat, cachestat);
}

int com_init()
{
  p = portlookup();
  hpriv = host_priv_self();
  return (p == PORT_NULL) ? -1 : 0;
}

int com_exit()
{
  return 0;
}
#endif /* NeXT */

#if defined(__svr4__) || defined(__sgi)  /* XXX: Hack: Solaris 2.x or Irix */
#include <fcntl.h>
#include <stropts.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "ipsp.h"
#ifdef __sgi
#include "../irix/com.h"
#else
#include "../solaris/com.h"
#endif /* __sgi */

static int skipstr = -1;

static int com_ioctl(int cmd, void *data, int len)
{
  struct strioctl io;
  char dummy;

  MEMZERO(&io, sizeof(io));
  io.ic_cmd = cmd;
  io.ic_timout = INFTIM;
  io.ic_len = len;

  if ((io.ic_dp = (char *)data) == NULL)  /* Avoid ioctl without data */
  {
    io.ic_dp = &dummy;
    io.ic_len = sizeof(dummy);
  }

  return (skipstr >= 0) ? ioctl(skipstr, I_STR, &io) : -1;
}

int com_update(struct skipcache *c)
{
  return com_ioctl(SKIPIOCUPDATE, c, sizeof(*c) + c->datalen);
}

int com_remove(u_char *IPaddr)
{
  return com_ioctl(SKIPIOCREMOVE, IPaddr, IPADDRSIZE);
}

int com_attach(u_char *IPaddr)
{
  return com_ioctl(SKIPIOCATTACH, IPaddr, IPADDRSIZE);
}

int com_detach(u_char *IPaddr)
{
  return com_ioctl(SKIPIOCDETACH, IPaddr, IPADDRSIZE);
}

int com_getrequests(u_char *buf, int len)
{
  int result = -1;

  if (skipstr >= 0)
  {
    struct strioctl io;

    MEMZERO(&io, sizeof(io));
    io.ic_cmd = SKIPIOCGETREQUESTS;
    io.ic_timout = INFTIM;
    io.ic_len = len;
    io.ic_dp = (char *)buf;
    
    if (ioctl(skipstr, I_STR, &io) == 0)
      result = io.ic_len / IPADDRSIZE;
  }

  return result;
}

int com_register(void)
{
  pid_t pid = getpid();

  return com_ioctl(SKIPIOCREGISTER, &pid, sizeof(pid));
}

int com_stat(ipsp_stat_t *ipspstat, skipcache_stat_t *cachestat)
{
  int result;

  if ((result = com_ioctl(SKIPIOCSTATIPSP, ipspstat, sizeof(*ipspstat))) == 0)
    result = com_ioctl(SKIPIOCSTATCACHE, cachestat, sizeof(*cachestat));

  return result;
}

int com_flush(void)
{
  return com_ioctl(SKIPIOCFLUSH, NULL, 0);
}

int com_init()
{
  if ((skipstr = open("/dev/skip", O_RDWR, 0)) < 0)
    return -1;
  return ioctl(skipstr, I_SETSIG, S_MSG);
}

int com_exit()
{
  if (skipstr >= 0)
  {
    close(skipstr);
    skipstr = -1;
  }
  return 0;
}
#endif /* __svr4__ */

#ifdef __NetBSD__
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/lkm.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "ipsp.h"
#include "../netbsd/com.h"

static int skipcall;

int com_update(struct skipcache *c)
{
  return syscall(skipcall, SKIPCMD_UPDATE, c, sizeof(*c) + c->datalen);
}

int com_remove(u_char *IPaddr)
{
  return syscall(skipcall, SKIPCMD_REMOVE, IPaddr, IPADDRSIZE);
}

int com_attach(u_char *IPaddr)
{
  return syscall(skipcall, SKIPCMD_ATTACH, IPaddr, IPADDRSIZE);
}

int com_detach(u_char *IPaddr)
{
  return syscall(skipcall, SKIPCMD_DETACH, IPaddr, IPADDRSIZE);
}

int com_getrequests(u_char *buf, int len)
{
  int result;

  if ((result = syscall(skipcall, SKIPCMD_GETREQUESTS, buf, len))== 0)
  {
    MEMCPY(&result, buf, sizeof(result));
    if (result > 0)
    {
      MEMCPY(buf, buf + sizeof(result), result);
      result /= IPADDRSIZE;
    }
  }

  return result;
}

int com_register(void)
{
  pid_t pid = getpid();

  return syscall(skipcall, SKIPCMD_REGISTER, &pid, sizeof(pid));
}

int com_stat(ipsp_stat_t *is, skipcache_stat_t *cs)
{
  int result;

  if ((result = syscall(skipcall, SKIPCMD_STATIPSP, is, sizeof(*is))) == 0)
    result = syscall(skipcall, SKIPCMD_STATCACHE, cs, sizeof(*cs));

  return result;
}

int com_flush(void)
{
  return syscall(skipcall, SKIPCMD_FLUSH, NULL, 0);
}

int com_init()
{
  int result = -1;
  int fd;

  if ((fd = open("/dev/lkm", O_RDONLY, 0)) >= 0)
  {
    struct lmc_stat ls;

    ls.id = -1;
    strcpy(ls.name, "skipmod");

    if (ioctl(fd, LMSTAT, &ls) != -1)
    {
      skipcall = ls.offset;
      result = 0;
    }

    close(fd);
  }
  
  return result;
}

int com_exit()
{
  return 0;
}
#endif /* __NetBSD__ */
