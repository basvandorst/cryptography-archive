/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include "skip_defs.h"
#include "skipcache.h"
#include "memblk.h"
#include "ipsp.h"
#include "com.h"
#include "parse.h"
#include "crypt.h"
#include "random.h"

#define CONFIGFILE "/etc/skipd.conf"

static int getreqflag;
static int saveconfigflag;
static int reinitflag;

void signal_handler(int sig)
{
  signal(sig, signal_handler);

  switch(sig)
  {
    case SIGIO: getreqflag = 1; break;
    case SIGUSR1: saveconfigflag = 1; break;
    case SIGHUP: reinitflag = 1; break;
  }
}

static void usage(char *name)
{
  fprintf(stderr, "Usage: %s [-fhv] [-c configfile]\n", name);
  fprintf(stderr, " -c <file>  alternative configurtion file (default is " CONFIGFILE ")\n");
  fprintf(stderr, " -f         flush primary (kernel) cache\n");
  fprintf(stderr, " -v         verbose, print a message for each lookup on stdout\n");
  fprintf(stderr, " -h         this help text\n");
  exit(1);
}

int main(int argc, char **argv)
{
  u_char buf[256];
  int reqs;
  struct skipcache *c;
  int ch;
  extern char *optarg;
  extern int optind;
  int errflg = 0, flush = 0,  verbose = 0;
  char *cnffile = CONFIGFILE;

  while ((ch = getopt(argc, argv, "c:fh?v")) != EOF)
  {
    switch (ch) 
    {
      case 'c':
        cnffile = optarg;
	break;

      case 'h': case '?': default:
        errflg++;
	break;

      case 'f':
	flush = 1;
	break;
	
      case 'v':
        verbose++;
	break;
    }
  }

  if (errflg || (optind < argc))
    usage(argv[0]);

  signal(SIGIO, signal_handler);
  signal(SIGHUP, signal_handler);

  crypt_init();
  random_init();
  skipcache_init(0, 0);  /* No maxttl, no maxentries */

  if (com_init() < 0)
  {
    fprintf(stderr, "%s: SKIP module not found\n", argv[0]);
    return 1;
  }

  /* Only flush primary (kernel) cache and exit */
  if (flush)
  {
    com_flush();
    return 0;
  }

  com_register();

  for (reinitflag = 1; reinitflag;)
  {
    reinitflag = 0;

    /* Flush both primary (kernel) and secondary (user) cache */
    com_flush();
    skipcache_flush();

    if ((c = parse_parse(cnffile)) == NULL)
      exit(1);

    while (c->next)  /* Fill skipd cache, except last which is default entry */
    {
      struct skipcache *next = c->next;

      c->flags |= SKIP_DONTAGE;
      skipcache_update(c);

      c = next;
    }

    /* Main loop, handling requests from kernel */
    while (!reinitflag)
    {
      u_char *walk;

      getreqflag = 0;

      while (!(getreqflag || saveconfigflag || reinitflag))
        pause();  /* Wait for signals */

      reqs = com_getrequests(buf, sizeof(buf));

      for (walk = buf; reqs > 0; reqs--, walk += IPADDRSIZE)
      {
        struct skipcache *l;

        if (verbose)
          printf("  lookup request: %u.%u.%u.%u\n", walk[0], walk[1], walk[2], walk[3]);

        if ((l = skipcache_lookup(walk)) == NULL)
        {
          /* Feed default entry */
          MEMCPY(c->IPaddr, walk, IPADDRSIZE);
          l = c;
        }
        else
	  l->flags &= ~SKIP_DONTAGE;

        if (com_update(l) < 0)
	  fprintf(stderr, "skipd: primary cache feed failed\n");

        if (l != c)
        {
	  l->flags |= SKIP_DONTAGE;
          skipcache_release(l);
        }
      }
    }

    /* Free default entry */
    parse_free(c);
  }

  return 0;
}
