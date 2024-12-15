/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/netisr.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/domain.h>
#include <sys/protosw.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "ipsp.h"
#include "skip.h"
#include "ipsum.h"
#include "interface.h"

/* Avoid unnecessary softints for ipintrq, ipintr is looping anyway */
#define AVOID_SOFTINTS

static int interface_output(struct ifnet *, struct mbuf *, struct sockaddr *);

static void(*oldinput)(struct mbuf *, struct ifnet *);

struct interface
{
  u_char IPaddr[IPADDRSIZE];
  struct ifnet *ifp;			/* interface identifier */

  /* function handles */
  int (*output)(struct ifnet *ifp, struct mbuf *m, struct sockaddr *sa);

  struct interface *next, *prev;
};

static struct interface *head = NULL;

static struct ifnet *interface_find(u_char *IPaddr)
{
  struct ifnet *ifp;

  for (ifp = ifnet; ifp; ifp = ifp->if_next)
  {
    struct ifaddr *ifa;

    if (ifp->if_output == interface_output)
      continue;  /* Only interfaces not already patched by SKIP */

    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
    {
      struct sockaddr_in *sin = (struct sockaddr_in *)&ifa->ifa_addr;

      if ((sin->sin_family == AF_INET) && 
          (MEMCMP(&sin->sin_addr, IPaddr, 4) == 0))
      {
        return ifp;
      }
    }
  }

  return NULL;
}

static struct interface *interface_search(u_char *IPaddr)
{
  struct interface *walk;

  for (walk = head; walk; walk = walk->next)
    if (MEMCMP(walk->IPaddr, IPaddr, 4) == 0)
      break;

  return walk;
}

static struct interface *interface_searchifp(struct ifnet *ifp)
{
  struct interface *walk;

  for (walk = head; walk; walk = walk->next)
    if (ifp == walk->ifp)
      break;

  return walk;
}


static void interface_doinput(struct mbuf *m, struct ifnet *ifp)
{
  struct interface_fbparam fb;

  fb.ifp = ifp;

  switch (skip_process(&fb, &m, SKIP_INPUT))
  {
    default: case SKIP_DISCARD:
      m_freem(m);
      break;

    case SKIP_PROCESSED:
    {
      /* Feed to IP again */
      struct ifqueue *ifq = &ipintrq;
      struct ifheader *ifh;
      int old;

      /*
       * Prepend ifheader again (directly in front of IP packet)
       * NOTE: skip_process left enough space in front of actual packet
       *       by converting the IPSP packet to IP
       */
      m->m_off -= sizeof(struct ifheader);
      m->m_len += sizeof(struct ifheader);
      ifh = mtod(m, struct ifheader *);
      ifh->ifh_ifp = ifp;
      ifh->ifh_hdrlen = sizeof(struct ifheader);
           
      old = splimp();
      IFQ_LOCK(ifq);
      if (IF_QFULL(ifq)) 
      {
        m_freem(m);
        IF_DROP(ifq);
      } 
      else
      {
        IF_ENQUEUE_NOLOCK(ifq, m);
#ifndef AVOID_SOFTINTS
        schednetisr(NETISR_IP);
#endif
      }
      IFQ_UNLOCK(ifq);
      splx(old);
      break;
    }

    case SKIP_QUEUED:
      break;
  }
}

static void interface_input(struct mbuf *m, struct ifnet *ifp)
{
  struct ip *ip = mtod(m, struct ip *);

  if (ip->ip_p == IPPROTO_IPSP)
  {
    /* Undo changes ip_input did to our packet :-\ */
    ip->ip_len += ip->ip_hl * 4;
    /* XXX: Not sure if the following code went from BSD4.3 into IRIX */
    /* My machine is big endian, so I can't test it */
    ip->ip_len = SKIP_HTONS(ip->ip_len);
    ip->ip_id = SKIP_HTONS(ip->ip_id);
    ip->ip_off = SKIP_HTONS(ip->ip_off);

    interface_doinput(m, ifp);
  }
  else
    oldinput(m, ifp);  /* Not IPPROTO_IPSP, let old input handler do it */
}

static int interface_output(struct ifnet *ifp, struct mbuf *m,
                            struct sockaddr *address)
{
  struct interface *i = interface_searchifp(ifp);
  struct sockaddr_in *sin = (struct sockaddr_in *)address;

  if (i == NULL)
  {
    m_freem(m);
    return -ENOENT;
  }

  if (sin->sin_family == AF_INET)
  {
    struct interface_fbparam fb;

    fb.address = *sin;
    fb.ifp = ifp;

    switch (skip_process(&fb, &m, SKIP_OUTPUT))
    {
      default:
      case SKIP_DISCARD:
        m_freem(m);
        return 0;

      case SKIP_PROCESSED:
        return i->output(ifp, m, address);

      case SKIP_QUEUED:
        return 0;
    }  
  }
  else
    return i->output(ifp, m, address);
}

int interface_outfeedback(struct mbuf *m)
{
  struct mbuf *pkt;
  struct interface_fbparam *fb = mtod(m, struct interface_fbparam *);
  struct ifnet *ifp = fb->ifp;
  struct sockaddr_in sin = fb->address;

  MFREE(m, pkt);

  return interface_output(ifp, pkt, (struct sockaddr *)&sin);
}

int interface_infeedback(struct mbuf *m)
{
  struct mbuf *pkt;
  struct interface_fbparam *fb = mtod(m, struct interface_fbparam *);
  struct ifnet *ifp = fb->ifp;

  MFREE(m, pkt);

  interface_doinput(pkt, ifp);
#ifdef AVOID_SOFTINTS
  schednetisr(NETISR_IP);
#endif

  return 0;
}

int interface_attach(u_char *IPaddr)
{
  struct ifnet *ifn;
  struct interface *new;
  int c;

  if (interface_search(IPaddr))
    return -EEXIST;

  for (c = 0; (ifn = interface_find(IPaddr)); c++)
  {
    if ((new = KALLOC(sizeof(*new))) == NULL)
      return -ENOMEM;

    MEMZERO(new, sizeof(*new));
    MEMCPY(new->IPaddr, IPaddr, 4);
    new->ifp = (struct ifnet *)ifn;
    new->output = ifn->if_output;

    ifn->if_output = interface_output;
    ifn->if_mtu -= (ipsp_maxheadergrowth() + ipsp_maxtrailergrowth());

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
  struct ifnet *ifn;

  ifn = (struct ifnet *)i->ifp;

  ifn->if_output = i->output;
  ifn->if_mtu += (ipsp_maxheadergrowth() + ipsp_maxtrailergrowth());

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

int interface_init(void)
{
  int result = ENFILE;
  struct protosw *pr;

  if ((pr = pffindproto(AF_INET, IPPROTO_RAW, SOCK_RAW)))
  {
    oldinput = pr->pr_input;
    pr->pr_input = interface_input;
    result = 0;
  }

  head = NULL;

  return result;
}

int interface_exit(void)
{
  struct interface *walk, *next;
  struct protosw *pr;

  for (walk = head; walk; walk = next)
  {
    next = walk->next;
    interface_remove(walk);
  }

  if ((pr = pffindproto(AF_INET, IPPROTO_RAW, SOCK_RAW)))
  {
    if (pr->pr_input == interface_input)
      pr->pr_input = oldinput;
  }

  head = NULL;

  return 0;
}
