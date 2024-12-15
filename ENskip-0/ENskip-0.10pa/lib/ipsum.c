/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

/*
 * routine used to emulate the assembler command addc
 */
static u_short addc(u_short val, u_short sum)
{
  if ((unsigned int)(val + sum) > 0xffff)
    sum++;      /* overflow */
  sum += val;
  return sum;
}

/*
 * calculate ip checksum
 */
u_short ipsum(struct ip *h)
{
  u_short *ips = (u_short *)h;
  u_short ck = 0, old = h->ip_sum;
  short len = (h->ip_hl * 4);

  h->ip_sum = 0;
  for (; len > 1; len -= 2)
    ck = addc(SKIP_NTOHS(*ips++), ck);
  h->ip_sum = old;

  if (len)
    ck = addc(SKIP_NTOHS(*ips) & 0xff00,ck);

  return (~ck);
}
