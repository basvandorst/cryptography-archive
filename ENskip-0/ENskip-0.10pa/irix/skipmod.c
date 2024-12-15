/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <sys/conf.h>
#include <sys/stream.h>
#include <sys/errno.h>
#include <sys/cred.h>
#include <sys/ddi.h>
#include <sys/mload.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "random.h"
#include "ipsp.h"
#include "crypt.h"
#include "sign.h"
#include "com.h"
#include "interface.h"

#define MINSDUSZ 1
#define MAXSDUSZ 8192

int skipdevflag = D_MP;
char *skipmversion = M_VERSION;

int skipopen(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *crp);
int skipclose(queue_t *q, int flag, cred_t *crp);
int skipput(queue_t *q, mblk_t *mp);

static struct module_info skip_minfo =
{
  0x534b, "skip", MINSDUSZ, MAXSDUSZ, 0, 0, 0, 0
};

static struct qinit skip_rinit =
{
  NULL, NULL, skipopen, skipclose, NULL, &skip_minfo, NULL
};

static struct qinit skip_winit =
{
  skipput, NULL, NULL, NULL, NULL, &skip_minfo, NULL
};

struct streamtab skipinfo =
{
  &skip_rinit, &skip_winit, NULL, NULL
};

int skipstart(void)
{
  crypt_init();
  sign_init();
  random_init();
  skipcache_init(SKIPCACHE_MAXTTL, SKIPCACHE_MAXENTRIES);
  ipsp_init();
  interface_init();

  return 0;
}

int skipunload(void)
{
  interface_exit();
  ipsp_exit();
  skipcache_exit();
  random_exit();
  sign_exit();
  crypt_exit();

  return 0;
}

int skipopen(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *crp)
{
  return 0;
}

int skipclose(queue_t *q, int flag, cred_t *crp)
{
  com_unregister(q);
  return 0;
}

static void confirmioctl(mblk_t *mp)
{
  struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

  iocp->ioc_error = 0;
  iocp->ioc_rval = 0;
  mp->b_datap->db_type = M_IOCACK;
}

static void copymblk2data(mblk_t *m, u_char *data, int len)
{
  while (m)
  {
    int l = m->b_wptr - m->b_rptr;
 
    if (l > len)
      l = len;
 
    if (l > 0)
    {
      MEMCPY(data, m->b_rptr, l);
      data += l;
      if ((len -= l) <= 0)
        break;
    }
    m = m->b_cont;
  }
}
 
static void copydata2mblk(mblk_t *m, u_char *data, int len)
{
  while (m)
  {
    int l = m->b_wptr - m->b_rptr;
 
    if (l > len)
      l = len;
 
    if (l > 0)
    {
      MEMCPY(m->b_rptr, data, l);
      data += l;
      if ((len -= l) <= 0)
        break;
    }
    m = m->b_cont;
  }
}
 
/* XXX To be done: Check ioctl parameter size */
static int skipwput_ioctl(queue_t *q, mblk_t *mp)
{
  struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
  mblk_t *m = mp->b_cont;

  mp->b_datap->db_type = M_IOCNAK;

  if (m && (iocp->ioc_count != TRANSPARENT))
  {
    u_char *data;
    int len = iocp->ioc_count;

    if ((data = KALLOC(len)))
    {
      switch (iocp->ioc_cmd)
      {
	case SKIPIOCUPDATE:
        {
	  struct skipcache *old = (struct skipcache *)data, *new;
	  int user_struct_len = (sizeof(struct skipcache) - CACHE_QSIZE);

	  copymblk2data(m, data, len);
	  if ((new = KALLOC(sizeof(*new) + old->datalen)))
	  {
	    MEMCPY(new, old, user_struct_len);
	    new->data = (u_char *)(new + 1);
	    MEMZERO((((u_char *)new) + user_struct_len), CACHE_QSIZE);
	    MEMCPY(new->data, (((u_char *)old) + user_struct_len), old->datalen);

	    if (skipcache_update(new) == 0)
	      confirmioctl(mp);
	    else
	      KFREE(new, sizeof(*new) + old->datalen);
	  }
	  break;
	}

        case SKIPIOCREMOVE:
	{
	  copymblk2data(m, data, len);
	  if (skipcache_remove(data) == 0)
	    confirmioctl(mp);
	  break;
	}

	case SKIPIOCSTATCACHE:
	{
	  MEMCPY(data, &skipcache_stat, sizeof(skipcache_stat));
	  copydata2mblk(m, data, len);
	  confirmioctl(mp);
	  break;
	}

	case SKIPIOCSTATIPSP:
	{
	  MEMCPY(data, &ipsp_stat, sizeof(ipsp_stat));
	  copydata2mblk(m, data, len);
	  confirmioctl(mp);
	  break;
	}

	case SKIPIOCATTACH:
	{
	  copymblk2data(m, data, len);
	  if (interface_attach(data) == 0)
	    confirmioctl(mp);
	  break;
	}
  
	case SKIPIOCDETACH:
	{
	  copymblk2data(m, data, len);
	  if (interface_detach(data) == 0)
	    confirmioctl(mp);
	  break;
	}

	case SKIPIOCREGISTER:
	{
	  com_register(q);
	  confirmioctl(mp);
	  break;
	}

	case SKIPIOCGETREQUESTS:
	{
	  int l;

	  if ((l = skipcache_getrequests(data,  len)) > 0)
	  {
	    copydata2mblk(m, data, len);
	    iocp->ioc_count = l;
	    confirmioctl(mp);
	  }
	  break;
	}

	case SKIPIOCFLUSH:
	{
          skipcache_flush();
	  confirmioctl(mp);
	  break;
	}
      }
      KFREE(data, len);
    }
  }

  qreply(q, mp);

  return 0;
}

int skipput(queue_t *q, mblk_t *mp)
{
  int result = 0;

  switch (mp->b_datap->db_type)
  {
    case M_IOCTL:
      result = skipwput_ioctl(q, mp);
      break;

    default:
      freemsg(mp);
      break;
  }

  return result;
}
