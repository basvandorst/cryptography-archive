/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <sys/types.h>
#include <stdio.h>

u_long iaddr(u_char *addr)
{
  u_int a, b, c, d;

  if (sscanf(addr, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
    return 0;

  return (d | (c << 8) | (b << 16) | (a << 24));
}
