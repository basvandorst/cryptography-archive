/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <sys/stream.h>
#include <inet/common.h>
#include <inet/ip.h>
#include <inet/ip_if.h>
#include <inet/ip_ire.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "skip.h"
#include "ipsp.h"
#include "interface.h"

static int interface_input(queue_t *q, mblk_t *mp);

struct interface
{
  u_char IPaddr[IPADDRSIZE];
  ipif_t *ipif;			/* interface identifier */
  int hdrlen;			/* Header length of lower layer (from ill) */

  /* old put functions */
  queue_t *queue_input;
  queue_t *queue_output;
  int (*putp_input)();
  int (*putp_output)();

  struct interface *next, *prev;
};

static struct interface *head = NULL;

static ipif_t *interface_find(u_char *IPaddr)
{
  ipif_t *ipif = NULL;
  ill_t *ill;

  for (ill = ill_g_head; ill; ill = ill->ill_next)
  {
    if ((ipif = ipif_lookup(ill, *((u32 *)IPaddr))) && 
        (ill->ill_rq->q_qinfo->qi_putp != interface_input))
    {
      break;  /* Found interface with IPaddr which is not already patched */
    }
  }

  return ill ? ipif : NULL;
}

static struct interface *interface_search(u_char *IPaddr)
{
  struct interface *walk;

  for (walk = head; walk; walk = walk->next)
    if (MEMCMP(walk->IPaddr, IPaddr, 4) == 0)
      break;

  return walk;
}

static struct interface *interface_searchqinit(queue_t *q)
{
  struct interface *walk;

  for (walk = head; walk; walk = walk->next)
  {
    if ((walk->queue_input->q_qinfo == q->q_qinfo) ||
	(walk->queue_output->q_qinfo == q->q_qinfo))
      break;
  }

  return walk;
}


static int interface_input(queue_t *q, mblk_t *mp)
{
  struct interface *i = interface_searchqinit(q);

  if (i == NULL)  /* SHOULD NOT HAPPEN :-) */
  {
    freemsg(mp);
    return 0;
  }

  if (mp->b_datap->db_type == M_DATA)
  {
    switch(skip_process(i, &mp, NULL, SKIP_INPUT))
    {
      default:
      case SKIP_DISCARD:
        freemsg(mp);
	return 0;

      case SKIP_PROCESSED:
	return (*i->putp_input)(q, mp);

      case SKIP_QUEUED:
	return 0;
    }
  }
  
  return (*i->putp_input)(q, mp);  /* 'putnext' */
}

static int interface_output(queue_t *q, mblk_t *mp)
{
  struct interface *i = interface_searchqinit(q);
  mblk_t *nodata, *nodata_tail;
  int hdrlen;

  if (i == NULL)  /* SHOULD NOT HAPPEN :-) */
  {
    freemsg(mp);
    return 0;
  }

  /*
   * 'putnext' message if not from IP
   */
  if (q != i->queue_output)
    return (*i->putp_output)(q, mp);

  /*
   * Separate M_* from M_DATA mblks
   */
  for (nodata = mp, nodata_tail = NULL; mp; nodata_tail = mp, mp = mp->b_cont)
  {
    if (mp->b_datap->db_type == M_DATA)
      break;  /* NOTE: nodata_tail is NULL if only M_DATA mblks there */
  }

  /*
   * Have to remove lower level header if
   * a) data left  b) hdrlen > 0  c) ll header not included in nodata
   */
  hdrlen = i->hdrlen;

  if (mp && hdrlen && !nodata_tail)
  {
    if ((mp->b_wptr - mp->b_rptr) == hdrlen) /* Separate lower level hdr */
    {
      nodata = nodata_tail = mp;
      mp = mp->b_cont;
      nodata_tail->b_cont = NULL;
    }
    else
    {
      if ((nodata = nodata_tail = allocb(hdrlen, BPRI_LO)))
      {
	MEMCPY(nodata->b_rptr, mp->b_rptr, hdrlen);
	nodata->b_wptr += hdrlen;
	mp->b_rptr += hdrlen;
      }
      else  /* Could not get mblk for lower level header */
      {
	freemsg(mp);
	return 0;
      }
    }
  }
  else
    nodata_tail->b_cont = NULL;

  /*
   * Finally mp should point to an IP packet :-}
   */
  if (mp)  /* Any M_DATA? */
  {
  
    switch(skip_process(i, &mp, nodata, SKIP_OUTPUT))
    {
      default:
      case SKIP_DISCARD:
        freemsg(nodata);
        freemsg(mp);
	return 0;

      case SKIP_PROCESSED:
	/* Prepend nodata header again */
        if (nodata_tail)
        {
	  nodata_tail->b_cont = mp;
          mp = nodata;
        }
	return (*i->putp_output)(q, mp);

      case SKIP_QUEUED:
	return 0;
    }
  }

  /* 'putnext' message without M_DATA */
  return (*i->putp_output)(q, nodata);
}

int interface_infeedback(void *feedbackif, mblk_t *mp)
{
  struct interface *i = feedbackif;

  return interface_input(i->queue_input, mp);
}

int interface_outfeedback(void *feedbackif, mblk_t *mp)
{
  struct interface *i = feedbackif;

  return interface_output(i->queue_output, mp);
}

int interface_attach(u_char *IPaddr)
{
  ipif_t *ipif;
  ill_t *ill;
  ire_t *ire;
  struct interface *new;
  queue_t *iq, *oq;
  int mtu_dec = ipsp_maxheadergrowth() + ipsp_maxtrailergrowth();
  int c;

  if (interface_search(IPaddr))
    return -EEXIST;

  for (c = 0; (c < 2) && (ipif = interface_find(IPaddr)); c++)
  {
    if ((ill = ipif->ipif_ill) == NULL)
      return -ENOENT;

    if ((iq = ill->ill_rq) == NULL)
      return -ENOENT;

    if ((oq = ill->ill_wq->q_next) == NULL)
      return -ENOENT;

    if ((new = KALLOC(sizeof(*new))) == NULL)
      return -ENOMEM;

    MEMZERO(new, sizeof(*new));
    MEMCPY(new->IPaddr, IPaddr, 4);
    new->ipif = ipif;
    new->hdrlen = ill->ill_hdr_length;
    new->queue_input = iq;
    new->queue_output = oq;
    new->putp_input = iq->q_qinfo->qi_putp;
    new->putp_output = oq->q_qinfo->qi_putp;

    ipif->ipif_mtu -= mtu_dec;
    ill->ill_max_frag -= mtu_dec;
    if ((ire = ipif_to_ire(ipif)))
      ire->ire_max_frag -= mtu_dec;
    iq->q_qinfo->qi_putp = interface_input;
    oq->q_qinfo->qi_putp = interface_output;

    new->next = head;
    new->prev = NULL;
    if (head)
      head->prev = new;
    head = new;
  }

  return c ? 0 : -ENOENT;
}

static int interface_remove(struct interface *i)
{
  ipif_t *ipif = i->ipif;
  ill_t *ill = ipif->ipif_ill;
  ire_t *ire;
  int mtu_inc = ipsp_maxheadergrowth() + ipsp_maxtrailergrowth();

  i->queue_input->q_qinfo->qi_putp = i->putp_input;
  i->queue_output->q_qinfo->qi_putp = i->putp_output;
  ill->ill_max_frag += mtu_inc;
  ipif->ipif_mtu += mtu_inc;
  if ((ire = ipif_to_ire(ipif)))
    ire->ire_max_frag += mtu_inc;

  if (i->next)
    i->next->prev = i->prev;
  if (i->prev)
    i->prev->next = i->next;
  else
    head = i->next;

  KFREE(i, sizeof(*i));
  return 0;
}

int interface_detach(u_char *IPaddr)
{
  struct interface *i;
  int c;

  for (c = 0; (i = interface_search(IPaddr)); c++)
    interface_remove(i);

  return c ? 0 : -ENOENT;
}

int interface_init()
{
  head = NULL;

  return 0;
}

int interface_exit()
{
  struct interface *walk, *next;

  for (walk = head; walk; walk = next)
  {
    next = walk->next;
    interface_remove(walk);
  }

  head = NULL;

  return 0;
}
