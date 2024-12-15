/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <errno.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skip.h"
#include "skipcache.h"
#include "ipsp.h"
#include "interface.h"
#include "queue.h"
#include "com.h"

#define MARGINSIZE 128  /* Cheating, but pullup gives enough margin anyway */

/*
 * Prepends <offset> bytes and ensures <hdrlen> bytes are contiguous afterwards
 * A trailer of <trllen> bytes is appended
 * Type of given mbuf chain is ignored, newly allocated head is M_DATA
 * NOTE: Given mbuf chain is freed on failure
 */
static struct mbuf *skip_pullup(struct mbuf *m, int off, int hdrlen, int trllen)
{
  struct mbuf *hdr = NULL, *trl = NULL, *t;


  /* Enough space in front of data and enough data contiguous? */
  if (m && ((0 < off) || (m->m_len < hdrlen)))
  {
    /* No, prepare new mbuf */
    MGET(hdr, M_WAIT, MT_DATA);
    if (hdr)
    {
      u_char *data;

      /* Make new header, old header becomes data */
      hdr->m_type = m->m_type;
      m->m_type = MT_DATA;
      hdr->m_off += off;
      hdr->m_len = 0;
      data = mtod(hdr, u_char *);

      while (m && (hdrlen > 0))
      {
        /* Calculate amount of data to copy from next mbuf */
        int l = (hdrlen <= m->m_len) ? hdrlen : m->m_len;

        MEMCPY(data, mtod(m, u_char *), l);
	data += l;
        hdr->m_len += l;
        m->m_off += l;
	m->m_len -= l;

        /* No data left in mbuf -> free it and skip to next */
        if (m->m_len == 0)
        {
          MFREE(m, t);
          m = t;
        }

        hdrlen -= l;
      }

      hdr->m_next = m;
    }
  }
  else
    hdr = m;

  if (hdr)
  {
    if (trllen)
    {
      MGET(trl, M_WAIT, MT_DATA);
      if (trl)
      {
	trl->m_len = 0;
        for (t = hdr; t->m_next; t = t->m_next)
          ;
        t->m_next = trl;
      }
      else
      {
        MFREE(hdr, t);
        hdr = NULL;
      }
    }
  }

  return hdr;
}


static int mbuf2memblk(struct mbuf *m, struct memblk *mb, struct memseg *ms)
{
  int count = 1;

  /* Construct memblk description */
  mb->dynamic = NULL;
  mb->dynamiclen = 0;
  mb->ms = ms;
  mb->offset = MARGINSIZE;
  mb->len = m->m_len;

  /* First mbuf is special case (offset & header pulled up) */
  ms->ptr = mtod(m, u_char *) - mb->offset;
  ms->len = m->m_len + mb->offset;
  ms++;


  if ((m = m->m_next))  /* More than one mbuf? */
  {
    while (m->m_next)
    {
      int ms_len = m->m_len;

      if (ms_len > 0)  /* Ignore empty mbufs */
      {
	if (count == (STATIC_MEMSEGS - 1))  /* Need dynamic mseg :-\ */
	{
	  struct mbuf *t;
	  int len;

	  for (len = count * sizeof(struct memseg), t = m; t; t = t->m_next)
	    len += sizeof(struct memseg);

	  mb->dynamiclen = len;
	  if ((mb->dynamic = KALLOC(len)) == NULL)
	    return -1;

	  MEMCPY(mb->dynamic, mb->ms, count * sizeof(struct memseg));
	  mb->ms = mb->dynamic;
	  ms = mb->dynamic + count;
	}

        ms->ptr = mtod(m, u_char *);
        ms->len = ms_len;
        mb->len += ms_len;
        ms++;
        count++;
      }

      m = m->m_next;
    }

    /* Last mbuf is special case (trailer pulled up) */
    mb->len += m->m_len;
    ms->ptr = mtod(m, u_char *);
    ms->len = m->m_len + MARGINSIZE;
  }


  return 0;
}

static int memblk2mbuf(struct mbuf *m, struct memblk *mb)
{
  int len;


  len = mtod(m, u_char *) - BLKSTART(mb);  /* Decrement value */
  m->m_len += len;  /* Set new start of first mbuf */
  m->m_off -= len;
  
  for (len = mb->len; m->m_next && (m->m_len < len); m = m->m_next)
    len -= m->m_len;

  m->m_len = len;  /* Amount left in last mbuf */

  while ((m = m->m_next))
    m->m_len = 0;  /* Set size of trailing mbufs to 0 */

  return 0;
}

static int skip_queue(struct skipcache *c, u_char *IPaddr, void *feedback,
		      struct mbuf *m, int dir)
{
  int ret = SKIP_DISCARD;
  struct mbuf *hdr;

  if (c == NULL)
  {
    if ((c = KALLOC(sizeof(*c))) == NULL)
      return SKIP_DISCARD;

    MEMZERO(c, sizeof(*c));
    MEMCPY(c->IPaddr, IPaddr, IPADDRSIZE);
    c->ttl = c->maxttl = QUEUETTL;
    queue_init(c);
    if (skipcache_update(c) < 0)
    {
      KFREE(c, sizeof(*c));
      return SKIP_DISCARD;
    }
    com_signal(SIGIO);
  }

  /* Prepend interface_input/output parameters for feedback */
  MGET(hdr, M_WAIT, MT_DATA);
  if (hdr)
  {
    hdr->m_next = m;
    MEMCPY(mtod(hdr, void *), feedback, sizeof(struct interface_fbparam));

    if (dir == SKIP_OUTPUT)
    {
      if (queue_enqueueout(c, hdr) == 0)
      {
        ipsp_stat.enqueued_out++;
        ret = SKIP_QUEUED;
      }
      else
        ipsp_stat.queuelimit++;
    }
    else
    {
      if (queue_enqueuein(c, hdr) == 0)
      {
        ipsp_stat.enqueued_in++;
        ret = SKIP_QUEUED;
      }
      else
        ipsp_stat.queuelimit++;
    }
  }
  else
    ipsp_stat.queuelimit++;

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
		       void *feedback, struct mbuf *m,
                       int dir)
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
    ret = skip_queue(*c, ipaddr, feedback, m, dir);
    skipcache_release(*c);
  }

  return ret;
}

static int skip_convert(struct skipcache *c, struct mbuf **m, int dir)
{
  int ret = SKIP_PROCESSED;
  struct ip *ip;

  if (c->flags & SKIP_VALIDKEY)
  {
    struct mbuf *t;
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

      if (mbuf2memblk(*m, &mb, ms) == 0)
      {
	int (*convert)(struct skipcache *c, struct memblk *m, int p, int dir);

	ip = (struct ip *)BLKSTART(&mb);
        convert = (ip->ip_p == IPPROTO_IPSP) ? skip_deskip : skip_enskip;

        /* Process */
	if ((*convert)(c, &mb, ip->ip_p, dir) == 0)
	{
	  ret = SKIP_PROCESSED;
	  memblk2mbuf(*m, &mb);
	}
	if (mb.dynamic)
	  KFREE(mb.dynamic, mb.dynamiclen);
      }
    }

    skipcache_release(c);
  }

  return ret;
}

int skip_process(void *feedback, struct mbuf **m, int dir)
{
  int ret = SKIP_PROCESSED;
  struct ip *ip = mtod(*m, struct ip *);
  struct skipcache *c;

  /*
   * TODO: reassembly depending on dir!!
   */

  /* Look up cache entry, cache if necessary */
  if ((ret = skip_lookup((dir == SKIP_OUTPUT) ? &ip->ip_dst : &ip->ip_src, &c,
			 feedback, *m, dir)) != SKIP_PROCESSED)
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
    if ((ret = skip_lookup(&ip->ip_src, &c2, feedback, *m, dir)) !=
	SKIP_PROCESSED)
      return ret;

    ret = skip_convert(c2, m, dir);

    if (ret == SKIP_PROCESSED)
    {
      ip = mtod(*m, struct ip *);
      if (FILTER_CHECK(c2->filter_after[dir], ip->ip_p))
	ret = SKIP_DISCARD;
    }
  }

  /* Convert for delivery */
  if (ret == SKIP_PROCESSED)
  {
    if ((ret = skip_convert(c, m, dir)) == SKIP_PROCESSED)
    {
      ip = mtod(*m, struct ip *);
      if (FILTER_CHECK(c->filter_after[dir], ip->ip_p))
	ret = SKIP_DISCARD;
    }
  }

  return ret;
}

