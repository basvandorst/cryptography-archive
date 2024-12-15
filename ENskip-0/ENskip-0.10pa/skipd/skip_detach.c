/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "ipsp.h"
#include "com.h"
#include "iaddr.h"

int main(int argc, char *argv[], char *envp[])
{
  u_char IPaddr[IPADDRSIZE];
  u_long addr;

  if (com_init() < 0)
  {
    fprintf(stderr, "%s: SKIP module not found\n", argv[0]);
    return 1;
  }

  if (argc < 2)
  {
    fprintf(stderr, "Usage: skip_detach <ipaddr>\n");
    return 1;
  }

  MEMZERO(IPaddr, sizeof(IPaddr));
  addr = iaddr(argv[1]);
  MEMCPY(IPaddr, &addr, sizeof(addr));

  if (com_detach(IPaddr) == 0)
  {
    printf("Detached from interface\n");
    return 0;
  }
  else
  {
    printf("Failed to detach from interface\n");
    return 2;
  }
}
