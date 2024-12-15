/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/netif.h>
#include <net/netisr.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <kernserv/machine/spl.h>
#include <kernserv/prototypes.h>
#include <kernserv/sched_prim.h>

#include "config.h"
#include "skip_defs.h"
#include "interface.h"
#include "skipcache.h"
#include "skip.h"

static int interface_input(netif_t, netif_t, netbuf_t, void *);

struct interface
{
  u_char IPaddr[IPADDRSIZE];

  netif_t ifp;			/* interface identifier */

  /* function handles */
  int (*input)(netif_t ifp, netif_t rifp, netbuf_t nb, void *extra);
  int (*output)(netif_t netif, netbuf_t packet, void *address);
  netbuf_t (*getbuf)(netif_t netif);

  struct interface *next, *prev;
};

static struct interface *head = NULL;
static thread_t inthread;

/* prototype */
void inet_queue(netif_t netif, netbuf_t netbuf);

static struct ifnet *interface_find(u_char *IPaddr)
{
  struct in_ifaddr *in_ifa;

  for (in_ifa = in_ifaddr; in_ifa; in_ifa = in_ifa->ia_next)
  {
    struct sockaddr_in *sin = (struct sockaddr_in *)&in_ifa->ia_addr;

    if ((sin->sin_family == AF_INET) &&
        (MEMCMP(&sin->sin_addr, IPaddr, 4) == 0) && 
        (in_ifa->ia_ifp->if_input != interface_input))
    {
      break;  /* Found interface with IPaddr which is not already patched */
    }
  }

  return in_ifa ? in_ifa->ia_ifp : NULL;
}

static struct interface *interface_search(u_char *IPaddr)
{
  struct interface *walk;

  for (walk = head; walk; walk = walk->next)
    if (MEMCMP(walk->IPaddr, IPaddr, 4) == 0)
      break;

  return walk;
}

static struct interface *interface_searchifp(netif_t ifp)
{
  struct interface *walk;

  for (walk = head; walk; walk = walk->next)
    if (ifp == walk->ifp)
      break;

  return walk;
}

static int interface_doinput(struct mbuf *m)
{
  struct mbuf *n = m->m_next;
  int old;

  m->m_next = NULL;
  switch (skip_process(NULL, n, m, SKIP_INPUT))
  {
    default:
    case SKIP_DISCARD:
      SKIP_MFREE(n);
      SKIP_MFREE(m);
      return 0;

    case SKIP_PROCESSED:
      old = spl6();
      m->m_next = n;
      IF_ENQUEUE(&ipintrq, m);
      splx(old);
      return 0;

    case SKIP_QUEUED:
      return 0;
  }
}

static int interface_input(netif_t ifp, netif_t rifp, netbuf_t nb, void *extra)
{
  /*
   * we cannot process the packet before we call
   * the interface's input function, because there
   * are some unknown headers in front of the IP packet.
   * therefore we call the input function and
   * dequeue the packet from the IP interrupt queue
   * for processing afterwards.
   */
  struct interface *i = interface_searchifp(ifp);
  struct mbuf *tail = ipintrq.ifq_tail;
  int r, old;

  if (i == NULL)
    return -ENOENT;

  old = spl7();

  if ((r = i->input(ifp, rifp, nb, extra)) == 0)
  {
    struct mbuf *m = ipintrq.ifq_tail;

    if (tail != m)
    {
      if (ipintrq.ifq_len == 1)
        ipintrq.ifq_tail = ipintrq.ifq_head = NULL;
      else
      {
        ipintrq.ifq_tail = tail;
        tail->m_act = NULL;
      }
      ipintrq.ifq_len--;

      splx(old);
      interface_doinput(m);
    }
    else
      splx(old);
  }
  else
    splx(old);

  return r;
}

static int interface_output(netif_t ifp, netbuf_t nb, void *address)
{
  struct interface *i = interface_searchifp(ifp);
  struct mbuf *m = (struct mbuf *)nb;
  struct sockaddr_in *sin = (struct sockaddr_in *)address;
  u_char *before = (u_char *)(((u_char *)m) + m->m_off), *base = before - ipsp_maxheadergrowth();

  if (i == NULL)
    return -ENOENT;

  if (sin->sin_family == AF_INET)
  {
    struct interface_outfbparam ofp;

    ofp.address = *sin;
    ofp.ifp = ifp;

    switch (skip_process(&ofp, m, NULL, SKIP_OUTPUT))
    {
      default:
      case SKIP_DISCARD:
        SKIP_MFREE(m);
        return 0;

      case SKIP_PROCESSED:
      {
#if 1
        return i->output(ifp, nb, address);
#endif
      }

      case SKIP_QUEUED:
        return 0;
    }  
  }
  else
    return i->output(ifp, nb, address);
}

int interface_outfeedback(struct mbuf *m)
{
  u_char *p = ((u_char *)m) + m->m_off;
  struct interface_outfbparam ofp = *(struct interface_outfbparam *)(p - sizeof(struct interface_outfbparam));

  return interface_output(ofp.ifp, (netbuf_t)m, (struct sockaddr *)&ofp.address);
}

int interface_infeedback(struct mbuf *m)
{
  return interface_doinput(m);
}

static netbuf_t interface_getbuf(netif_t ifp)
{
  struct interface *i = interface_searchifp(ifp);
  netbuf_t nb;

  if (i == NULL)
    return NULL;

  nb = i->getbuf(ifp);

  nb_shrink_top(nb, ipsp_maxheadergrowth());
  nb_shrink_bot(nb, ipsp_maxtrailergrowth());

  return nb;
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
    new->ifp = (netif_t)ifn;
    new->input = ifn->if_input;
    new->output = ifn->if_output;
    new->getbuf = ifn->if_getbuf;

    ifn->if_input = interface_input;
    ifn->if_output = interface_output;
    ifn->if_getbuf = interface_getbuf;
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

  ifn->if_input = i->input;
  ifn->if_output = i->output;
  ifn->if_getbuf = i->getbuf;
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
