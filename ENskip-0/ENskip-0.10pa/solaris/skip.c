/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <errno.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/stream.h>
#include <sys/cmn_err.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skip.h"
#include "skipcache.h"
#include "ipsp.h"
#include "queue.h"
#include "com.h"

/*
 * Prepends <offset> bytes and ensures <len> bytes are contiguous afterwards
 * A trailer of <trllen> bytes is appended
 * Type of given mblk chain is ignored, newly allocated head is M_DATA
 */
static mblk_t *skip_pullup(mblk_t *m, int offset, int hdrlen, int trllen)
{
  mblk_t *hdr = NULL, *trl = NULL, *t;

  /* Enough space in front of data and enough data contiguous? */
  if (m && (((m->b_rptr - m->b_datap->db_base) < offset) ||
      ((m->b_wptr - m->b_rptr) < hdrlen)))
  {
    /* No, prepare new mblk */
    if ((hdr = allocb(offset + hdrlen, BPRI_LO)) != NULL)
    {
      hdr->b_rptr = hdr->b_wptr = hdr->b_rptr + offset;

      while (m && (hdrlen > 0))
      {
        /* Calculate amount of data to copy from next mblk */
        int l = (hdrlen <= m->b_wptr - m->b_rptr) ? 
                hdrlen : (m->b_wptr - m->b_rptr);

        MEMCPY(hdr->b_wptr, m->b_rptr, l);
        hdr->b_wptr += l;
        m->b_rptr += l;

        /* No data left in mblk -> free it and skip to next */
        if (m->b_rptr == m->b_wptr)
        {
          t = m->b_cont;
          freeb(m);
          m = t;
        }
      
        hdrlen -= l;
      }

      hdr->b_cont = m;
    }
  }
  else
    hdr = m;

  if (hdr)
  {
    if (trllen)
    {
      if ((trl = allocb(trllen, BPRI_LO)))
      {
        for (t = hdr; t->b_cont; t = t->b_cont)
	  ;
	t->b_cont = trl;
      }
      else
      {
        freeb(hdr);
        hdr = NULL;
      }
    }
  }

  return hdr;
}


static int mblk2memblk(mblk_t *m, struct memblk *mb, struct memseg *ms)
{
  int count = 1;

  /* Construct memblk description */
  mb->dynamic = NULL;
  mb->dynamiclen = 0;
  mb->ms = ms;
  mb->offset = m->b_rptr - m->b_datap->db_base;

  /* First mblk is special case (offset & header pulled up) */
  ms->ptr = m->b_datap->db_base;
  ms->len = m->b_wptr - ms->ptr;
  mb->len = ms->len - mb->offset;
  ms++;

  if ((m = m->b_cont))  /* More than one mblk? */
  {
    while (m->b_cont)  /* Process complex message */
    {
      int ms_len;

      if ((ms_len = m->b_wptr - m->b_rptr))  /* Ignore empty mblks */
      {
	if (count == (STATIC_MEMSEGS - 1))  /* Need dynamic mseg :-\ */
	{
	  mblk_t *t;
	  int len;

	  for (len = count * sizeof(struct memseg), t = m; t; t = t->b_cont)
	    len += sizeof(struct memseg);

	  mb->dynamiclen = len;
	  if ((mb->dynamic = KALLOC(len)) == NULL)
	    return -1;

	  MEMCPY(mb->dynamic, mb->ms, count * sizeof(struct memseg));
	  mb->ms = mb->dynamic;
	  ms = mb->dynamic + count;
	}

        ms->ptr = m->b_rptr;
	ms->len = ms_len;
        mb->len += ms_len;  /* Length of whole message */
        ms++;
	count++;
      }

      m = m->b_cont;
    }

    /* Last mblk is special case (trailer pulled up) */
    ms->ptr = m->b_rptr;
    ms->len = m->b_datap->db_lim - ms->ptr;
    mb->len += m->b_wptr - m->b_rptr;
  }


  return 0;
}

static int memblk2mblk(mblk_t *m, struct memblk *mb)
{
  int len;


  m->b_rptr = BLKSTART(mb);  /* Set new start of first mblk */

  for (len = mb->len;
       m->b_cont && ((m->b_wptr - m->b_rptr) < len);
       m = m->b_cont)
  {
    if (m->b_wptr > m->b_rptr)  /* Ignore empty mblks again */
      len -= m->b_wptr - m->b_rptr;
  }

  m->b_wptr = m->b_rptr + len;  /* Set size of last mblk containing data */

  while ((m = m->b_cont))
    m->b_wptr = m->b_rptr;  /* Set size of trailing mblks to 0 */

  return 0;
}

static int skip_queue(struct skipcache *c, u_char *IPaddr, void *feedback,
		      mblk_t *m, mblk_t *header, int dir)
{
  int ret = SKIP_DISCARD;

  if (c == NULL)
  {
    if ((c = KALLOC(sizeof(*c))) == NULL)
      return SKIP_DISCARD;

    MEMZERO(c, sizeof(*c));
    MEMCPY(c->IPaddr, IPaddr, IPADDRSIZE);
    c->ttl = c->maxttl = QUEUETTL;
    c->inq.ifq_maxlen = c->outq.ifq_maxlen = QUEUESIZE;
    c->interface = feedback;
    if (skipcache_update(c) < 0)
    {
      KFREE(c, sizeof(*c));
      return SKIP_DISCARD;
    }
    com_signal(SIGIO);
  }

  if (dir == SKIP_OUTPUT)
  {
    if (queue_enqueueout(c, header, m) == 0)
    {
      ipsp_stat.enqueued_out++;
      ret = SKIP_QUEUED;
    }
    else
      ipsp_stat.queuelimit++;
  }
  else
  {
    /* Queue input */
    if (queue_enqueuein(c, m) == 0)
    {
      ipsp_stat.enqueued_in++;
      ret = SKIP_QUEUED;
    }
    else
      ipsp_stat.queuelimit++;
  }

  return ret;
}

static int skip_enskip(struct skipcache *c, struct memblk *m, int p, int dir)
{
  int mode;

  switch (p)
  {
    case IPPROTO_TCP: mode = ENSKIPMODE_TCP; break;
    case IPPROTO_UDP: mode = ENSKIPMODE_UDP; break;
    case IPPROTO_ICMP: mode = ENSKIPMODE_ICMP; break;
    default: mode = ENSKIPMODE_OTHER; break;
  }

  return (c->enskip_mode[dir] & mode) ? ipsp_ip2ipsp(c, m) : 0;
}

static int skip_deskip(struct skipcache *c, struct memblk *m, int p, int dir)
{
  return (c->deskip_mode[dir] == DESKIPMODE_DESKIP) ? ipsp_ipsp2ip(c, m, dir) : 0;
}


static int skip_lookup(struct in_addr *IPaddr, struct skipcache **c,
		       void *feedback, mblk_t *header, mblk_t *m, int dir)
{
  int ret = SKIP_PROCESSED;
  u_char ipaddr[IPADDRSIZE];

  /*
   * copy address 'cause cache uses IPv6 addresses
   */
  MEMZERO(ipaddr, sizeof(ipaddr));
  MEMCPY(ipaddr, IPaddr, sizeof(struct in_addr));

  /*
   * lookup address in cache
   */
  *c = skipcache_lookup(ipaddr);

  /*
   * enqueue packet if no valid key was found, else process it
   */
  if ((*c == NULL) || (((*c)->flags & (SKIP_VALIDKEY | SKIP_NOKEY)) == 0))
  {
    ret = skip_queue(*c, ipaddr, feedback, m, header, dir);
    skipcache_release(*c);
  }

  return ret;
}

static int skip_convert(struct skipcache *c, mblk_t **m, int dir)
{
  int ret = SKIP_PROCESSED;
  struct ip *ip;

  if (c->flags & SKIP_VALIDKEY)
  {
    mblk_t *t;
    struct memseg ms[STATIC_MEMSEGS];
    struct memblk mb;
    int hdrlen, trllen, contlen;

    if (c->enskip_mode[dir] != ENSKIPMODE_NONE)
    {
      hdrlen = ipsp_maxheadergrowth();
      trllen = ipsp_maxtrailergrowth();
      contlen = sizeof(struct ip) + 2;
    }
    else
    {
      hdrlen = trllen = 0;
      contlen = ipsp_maxheadergrowth() + sizeof(struct ip) + 2;
    }

    ret = SKIP_DISCARD;

    if ((t = skip_pullup(*m, hdrlen, contlen, trllen)))
    {
      *m = t;

      if (mblk2memblk(*m, &mb, ms) == 0)
      {
	int (*convert)() = skip_enskip;

	ip = (struct ip *)BLKSTART(&mb);
	if (ip->ip_p == IPPROTO_IPSP)
	  convert = skip_deskip;

        /* Process */
	if ((*convert)(c, &mb, ip->ip_p, dir) == 0)
	{
	  ret = SKIP_PROCESSED;
	  memblk2mblk(*m, &mb);
	}
	if (mb.dynamic)
	  KFREE(mb.dynamic, mb.dynamiclen);
      }
    }

    skipcache_release(c);
  }

  return ret;
}

int skip_process(void *feedback, mblk_t **m, mblk_t *header, int dir)
{
  int ret = SKIP_PROCESSED;
  struct ip *ip = (struct ip *)(*m)->b_rptr;
  struct skipcache *c;


  /*
   * TODO: reassembly depending on dir!!
   */
  

  /* Look up cache entry, cache if necessary */
  if ((ret = skip_lookup((dir == SKIP_OUTPUT) ? &ip->ip_dst : &ip->ip_src, &c,
			 feedback, header, *m, dir)) != SKIP_PROCESSED)
    return ret;

  if (FILTER_CHECK(c->filter_before[dir], ip->ip_p))
  {
    skipcache_release(c);
    return SKIP_DISCARD;
  }

  /* Gateway de- & enskipping on output? XXX: Should be tested */
  if ((dir == SKIP_OUTPUT) && (c->flags & SKIP_VALIDKEY) &&
      (c->deskip_mode[dir] == DESKIPMODE_DESKIP))
  {
    struct skipcache *c2;

    /* Look up cache entry, cache if necessary */
    if ((ret = skip_lookup(&ip->ip_src, &c2, feedback, header, *m, dir)) !=
	SKIP_PROCESSED)
      return ret;

    ret = skip_convert(c2, m, dir);

    if (ret == SKIP_PROCESSED)
    {
      ip = (struct ip *)(*m)->b_rptr;
      if (FILTER_CHECK(c2->filter_after[dir], ip->ip_p))
	ret = SKIP_DISCARD;
    }
  }

  /* Convert for delivery */
  if (ret == SKIP_PROCESSED)
  {
    if ((ret = skip_convert(c, m, dir)) == SKIP_PROCESSED)
    {
      ip = (struct ip *)(*m)->b_rptr;
      if (FILTER_CHECK(c->filter_after[dir], ip->ip_p))
	ret = SKIP_DISCARD;
    }
  }


  return ret;
}

