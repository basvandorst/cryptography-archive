/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <sys/conf.h>
#include <sys/stream.h>
#include <sys/errno.h>
#include <sys/cred.h>
#include <sys/modctl.h>
#include <sys/devops.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/stat.h>
#include <sys/cmn_err.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "random.h"
#include "queue.h"
#include "ipsp.h"
#include "crypt.h"
#include "sign.h"
#include "com.h"
#include "interface.h"

#define MINSDUSZ 1
#define MAXSDUSZ 8192

static int skipopen(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *crp);
static int skipclose(queue_t *q, int flag, cred_t *crp);
static int skipwput(queue_t *q, mblk_t *mp);

static struct module_info skip_minfo = 
{
  0x534b, "skip", MINSDUSZ, MAXSDUSZ, 0, 0
};

static struct qinit skip_rinit =
{
  NULL, NULL, skipopen, skipclose, NULL, &skip_minfo, NULL
};

static struct qinit skip_winit =
{
  skipwput, NULL, NULL, NULL, NULL, &skip_minfo, NULL
};

struct streamtab skipinfo =
{
  &skip_rinit, &skip_winit, NULL, NULL
};

extern int nulldev();
extern int nodev();

static int skip_info(dev_info_t *dip, ddi_info_cmd_t infocmd, void *arg,
			void **result);
static int skip_attach(dev_info_t * devi,  ddi_attach_cmd_t cmd);
static int skip_identify(dev_info_t * devi);
static int skip_detach(dev_info_t * devi,  ddi_detach_cmd_t cmd);

static struct cb_ops skip_ops = 
{
  nulldev,		/* cb_open */
  nulldev,		/* cb_close */
  nodev,		/* cb_strategy */
  nodev,		/* cb_print */
  nodev,		/* cb_dump */
  nodev,		/* cb_read */
  nodev,		/* cb_write */
  nodev,		/* cb_ioctl */
  nodev,		/* cb_devmap */
  nodev,		/* cb_mmap */
  nodev,		/* cb_segmap */
  nochpoll,		/* cb_chpoll */
  ddi_prop_op,		/* cb_prop_op */
  &skipinfo,		/* cb_stream */
  0			/* cb_flag */
};

static struct dev_ops dev_ops = 
{
  DEVO_REV,		/* devo_rev */
  0,			/* devo_refcnt */
  skip_info,		/* devo_getinfo */
  skip_identify,	/* devo_identify */
  nulldev,		/* devo_probe */
  skip_attach,		/* devo_attach */
  skip_detach,		/* devo_detach */
  nodev,		/* devo_reset */
  &skip_ops,		/* devo_cb_ops */
  NULL			/* devo_bus_ops */
};

static struct modldrv modldrv =
{
  &mod_driverops, "SKIP Streams device", &dev_ops
};

static struct modlinkage modlinkage =
{
  MODREV_1,
  { (void *)&modldrv, NULL }
};

static dev_info_t *skip_dev_info;

static int skip_attach(dev_info_t *devi, ddi_attach_cmd_t cmd)
{
  int result = DDI_SUCCESS;

  skip_dev_info = devi;

  if (ddi_create_minor_node(devi, "skip", S_IFCHR, 0, NULL, CLONE_DEV) ==
      DDI_FAILURE) 
  {
    ddi_remove_minor_node(devi, NULL);
    result =  -1;
  }

  return result;
}

static int skip_detach(dev_info_t *devi, ddi_detach_cmd_t cmd)
{
  return DDI_SUCCESS;
}


static int skip_info(dev_info_t *dip, ddi_info_cmd_t infocmd, void *arg,
		     void **res)
{
  int result = DDI_FAILURE;

  switch (infocmd) 
  {
    case DDI_INFO_DEVT2DEVINFO:
      if (skip_dev_info != NULL) 
      {
	*res = (void *)skip_dev_info;
	result = DDI_SUCCESS;
      }
      break;

    case DDI_INFO_DEVT2INSTANCE:
      *res = (void *)0;
      result = DDI_SUCCESS;
      break;
  }

  return result;
}

static int skip_identify(dev_info_t *devi)
{
  int result = DDI_NOT_IDENTIFIED;

  if (strcmp((char *)ddi_get_name(devi), "skip") == 0)
    result = DDI_IDENTIFIED;

  return result;
}

char _depends_on[] = "drv/ip";

int _init(void)
{
  int result;

  if ((result = mod_install(&modlinkage)) == 0)
  {
    crypt_init();
    sign_init();
    random_init();
    queue_init();
    skipcache_init(SKIPCACHE_MAXTTL, SKIPCACHE_MAXENTRIES);
    ipsp_init();
    interface_init();
  }

  return result;
}

int _fini(void)
{
  int result;

  if ((result = mod_remove(&modlinkage)) == 0)
  {
    interface_exit();
    ipsp_exit();
    skipcache_exit();
    queue_exit();
    random_exit();
    sign_exit();
    crypt_exit();
  }

  return result;
}

int _info(struct modinfo *modinfop)
{
  return mod_info(&modlinkage, modinfop);
}

static int skipopen(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *crp)
{
  return 0;
}

static int skipclose(queue_t *q, int flag, cred_t *crp)
{
  return 0;
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

static void confirmioctl(mblk_t *mp)
{
  struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

  iocp->ioc_error = 0;
  iocp->ioc_rval = 0;
  mp->b_datap->db_type = M_IOCACK;
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
	  pid_t pid;
	    
	  copymblk2data(m, (u_char *)&pid, sizeof(pid));
	  com_register(pid);
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

static int skipwput(queue_t *q, mblk_t *mp)
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
