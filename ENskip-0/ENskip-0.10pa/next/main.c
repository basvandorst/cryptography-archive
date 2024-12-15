/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <kernserv/kern_server_types.h>
#include <mach/std_types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include "main.h"
#include "config.h"
#include "skip_defs.h"
#include "skipcache.h"
#include "random.h"
#include "interface.h"
#include "memblk.h"
#include "ipsp.h"
#include "crypt.h"
#include "sign.h"

kern_server_t self;

void skip_load()
{
  crypt_init();
  sign_init();
  random_init();
  skipcache_init(SKIPCACHE_MAXTTL, SKIPCACHE_MAXENTRIES);
  ipsp_init();
  interface_init();
}

void skip_unload()
{
  interface_exit();
  ipsp_exit();
  skipcache_exit();
  random_exit();
  sign_exit();
  crypt_exit();
}
