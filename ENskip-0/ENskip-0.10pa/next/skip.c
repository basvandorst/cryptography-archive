/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <errno.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/netif.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skip.h"
#include "skipcache.h"
#include "ipsp.h"
#include "queue.h"
#include "com.h"
#include "interface.h"

static int mbuf2memblk(struct mbuf *m, struct memblk *mb, struct memseg *ms)
{
  int hdrlen = ipsp_maxheadergrowth();
  int trllen = ipsp_maxtrailergrowth();

  mb->len = m->m_len;
  mb->offset = hdrlen;
  mb->ms = ms;

  ms->ptr = (u_char *)(((u_char *)m) + m->m_off - hdrlen);
  ms->len = (m->m_len + hdrlen + trllen);

  return 0;
}

static int memblk2mbuf(struct mbuf *m, struct memblk *mb)
{
  int hdrlen = ipsp_maxheadergrowth();

  m->m_len = mb->len;
  m->m_off -= (hdrlen - mb->offset);

  return 0;
}

static int skip_queue(struct skipcache *c, u_char *IPaddr, void *feedback,
		      struct mbuf *m, struct mbuf *header, int dir)
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
    u_char *p = ((u_char *)m) + m->m_off;

    /*
     * copy feedback information before packet
     */
    MEMCPY(p - sizeof(struct interface_outfbparam), feedback, sizeof(struct interface_outfbparam));
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
    if (queue_enqueuein(c, header, m) == 0)
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
		       void *feedback, struct mbuf *header, struct mbuf *m, int dir)
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

static int skip_convert(struct skipcache *c, struct mbuf *m, int dir)
{
  int ret = SKIP_PROCESSED;
  struct ip *ip;

  if (c->flags & SKIP_VALIDKEY)
  {
    struct memseg ms;
    struct memblk mb;

    ret = SKIP_DISCARD;

    if (mbuf2memblk(m, &mb, ms) == 0)
    {
      int (*convert)() = skip_enskip;

      ip = (struct ip *)BLKSTART(&mb);
      if (ip->ip_p == IPPROTO_IPSP)
        convert = skip_deskip;

      /* Process */
      if ((*convert)(c, &mb, ip->ip_p, dir) == 0)
      {
        ret = SKIP_PROCESSED;
        memblk2mbuf(m, &mb);
      }
    }

    skipcache_release(c);
  }

  return ret;
}

int skip_process(void *feedback, struct mbuf *m, struct mbuf *header, int dir)
{
  int ret = SKIP_PROCESSED;
  struct ip *ip = (struct ip *)(((u_char *)m) + m->m_off);
  struct skipcache *c;

  /*
   * TODO: reassembly depending on dir!!
   */

  /* Look up cache entry, cache if necessary */
  if ((ret = skip_lookup((dir == SKIP_OUTPUT) ? &ip->ip_dst : &ip->ip_src, &c,
			 feedback, header, m, dir)) != SKIP_PROCESSED)
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
    if ((ret = skip_lookup(&ip->ip_src, &c2, feedback, header, m, dir)) !=
	SKIP_PROCESSED)
      return ret;

    ret = skip_convert(c2, m, dir);

    if (ret == SKIP_PROCESSED)
    {
      ip = (struct ip *)(((u_char *)m) + m->m_off);
      if (FILTER_CHECK(c2->filter_after[dir], ip->ip_p))
	ret = SKIP_DISCARD;
    }
  }

  /* Convert for delivery */
  if (ret == SKIP_PROCESSED)
  {
    if ((ret = skip_convert(c, m, dir)) == SKIP_PROCESSED)
    {
      ip = (struct ip *)(((u_char *)m) + m->m_off);
      if (FILTER_CHECK(c->filter_after[dir], ip->ip_p))
	ret = SKIP_DISCARD;
    }
  }

  return ret;
}

