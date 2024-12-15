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

int main(int argc, char **argv)
{
  ipsp_stat_t ipsp;
  skipcache_stat_t cache;

  if (com_init() < 0)
  {
    fprintf(stderr, "%s: SKIP module not found\n", argv[0]);
    return 1;
  }

  if (com_stat(&ipsp, &cache) == 0)
  {
    printf("ipsp:\n");
    printf("\t%lu total packets received\n", ipsp.in);
    printf("\t%lu encrypted\n", ipsp.crypt_in);
    printf("\t%lu authenticated\n", ipsp.auth_in);
    printf("\t%lu sequenced\n", ipsp.seq_in);
    printf("\t%lu compressed\n", ipsp.comp_in);
    printf("\t%lu encapsulated\n", ipsp.encaps_in);
    printf("\t%lu with bad ip checksum\n", ipsp.badipsum);
    printf("\t%lu with bad length (too short)\n", ipsp.badpktlen);
    printf("\t%lu with policy violation\n", ipsp.badpolicy);
    printf("\t%lu with unknown Kij algorithm\n", ipsp.badKijalg);
    printf("\t%lu with unknown Kp algorithm\n", ipsp.badKpalg);
    printf("\t%lu with unassigned block length\n", ipsp.badblklen);
    printf("\t%lu with unknown ICV algorithm\n", ipsp.badICValg);
    printf("\t%lu with incorrect ICV\n", ipsp.badICV);
    printf("\t%lu with bad sequence number\n\n", ipsp.badseq);

    printf("\t%lu total packets sent\n", ipsp.out);
    printf("\t%lu encrypted\n", ipsp.crypt_out);
    printf("\t%lu authenticated\n", ipsp.auth_out);
    printf("\t%lu sequenced\n", ipsp.seq_out);
    printf("\t%lu compressed\n", ipsp.comp_out);
    printf("\t%lu encapsulated\n\n", ipsp.encaps_out);

    printf("\t%lu packets dropped due to queue limits\n", ipsp.queuelimit);
    printf("\t%lu packets fragments dropped\n", ipsp.fragdrop);

    printf("\ncache:\n");
    printf("\t%lu entries in cache\n", cache.entries);
    printf("\t%lu entries inserted\n", cache.inserted);
    printf("\t%lu entries updated\n", cache.updated);
    printf("\t%lu entries timed out\n", cache.timedout);
    printf("\t%lu entries removed\n", cache.removed);
    printf("\t%lu lookup hits\n", cache.lookuphits);
    printf("\t%lu lookup requests\n", cache.lookuprequests);
  }
  else
  {
    fprintf(stderr, "Failed to retrieve SKIP statistics.\n");
    return 1;
  }

  return 0;
}
