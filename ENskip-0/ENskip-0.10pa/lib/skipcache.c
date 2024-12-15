/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <errno.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "crypt.h"
#include "random.h"

#define HASHTBLSIZE	73	/* size of hash table */
#define TIMERINTERVAL	5	/* expire interval in seconds */
#define INCTTL		1	/* value ttl is incremented after use */

#ifdef DEBUG
#define PRINTIPADDR(str, x)	printf(">> %s %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\n", str, x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9], x[10], x[11], x[12], x[13], x[14], x[15]);
#else
#define PRINTIPADDR(str, x)	
#endif

skipcache_stat_t skipcache_stat;
static struct skipcache *tbl[HASHTBLSIZE];
static SEMTYPE semaphore;
static int maxent, maxttl;

static int skipcache_index(u_char *IPaddr)
{
  int walk, sum;

  for (walk = sum = 0; walk < IPADDRSIZE; walk++)
    sum += IPaddr[walk];

  return (sum % HASHTBLSIZE);
}

static int skipcache_feedqueues(struct skipcache *c)
{
  SKIP_FEEDQ(c);
  return 0;
}

static int skipcache_freequeues(struct skipcache *c)
{
  SKIP_FREEQ(c);
  return 0;
}

static int skipcache_freeentry(struct skipcache *c)
{
  skipcache_freequeues(c);
  KFREE(c, (c->datalen + sizeof(struct skipcache)));
  return 0;
}

static int skipcache_removeentry(struct skipcache *c)
{
  /* rip out of hash chain */
  if (c->prev)
  {
    c->prev->next = c->next;
    if (c->next)
      c->next->prev = c->prev;
  }
  else
  {
    int idx = skipcache_index(c->IPaddr);

    tbl[idx] = c->next;
    if (c->next)
      c->next->prev = NULL;
  }
  skipcache_stat.entries--;
  c->next = c->prev = NULL;

  return 0;
}

static int skipcache_replaceentry(struct skipcache *old, struct skipcache *new)
{
  new->next = old->next;
  new->prev = old->prev;

  if (old->prev)
    old->prev->next = new;
  else
    tbl[skipcache_index(old->IPaddr)] = new;

  if (old->next)
    old->next->prev = new;

  old->next = old->prev = NULL;

  return 0;
}

/*
 * the function decrements all entries' time to live value and
 * removes all entries with a time to live value equal or below zero.
 * the number of expired entries is returned.
 */
static int skipcache_age(int times)
{
  int walk, cnt = skipcache_stat.entries;

  for (walk = 0; walk < HASHTBLSIZE; walk++)
  {
    struct skipcache *c, *next;

    for (c = tbl[walk]; c; c = next)
    {
      next = c->next;

#ifdef DEBUG
      printf(">> checking entry with len %d and ttl %d\n", c->datalen, c->ttl);
#endif /* DEBUG */

      if ((c->flags & SKIP_DONTAGE) == 0)
      {
        /* is entry expired and not referenced? */
        if (c->ttl && (c->ref <= 1) && ((c->ttl -= times) <= 0))
        {
          skipcache_stat.timedout++;
          skipcache_removeentry(c);
          if (--c->ref <= 0)
            skipcache_freeentry(c);
        }
        else  /* Check time for packet key change */
        {
          if ((c->enskip_Kp_ttl > 0) && ((c->enskip_Kp_ttl -= times) == 0))
            c->enskip_Kp_ttl = -1;
        }
      }
    }
  }

  return (cnt - skipcache_stat.entries);
}

/*
 * remove entry with lowest time to live from cache
 */
static int skipcache_removelru(void)
{
  int walk;
  struct skipcache *lru = NULL;

  /* find entry with lowest ttl */
  for (walk = 0; walk < HASHTBLSIZE; walk++)
  {
    struct skipcache *c;

    for (c = tbl[walk]; c; c = c->next)
    {
      /* has entry lower ttl and is not (externally) referenced? */
      if ((c->ref <= 1) && c->ttl && ((lru == NULL) || (lru->ttl > c->ttl)))
        lru = c;
    }
  }

  if (lru)
  {
    skipcache_removeentry(lru);
    if (--lru->ref <= 0)
      skipcache_freeentry(lru);
    return 0;
  }
  else
    return -ENOENT;
}

static int skipcache_insertentry(struct skipcache *new)
{
  int idx = skipcache_index(new->IPaddr);

  if (maxent && (skipcache_stat.entries > maxent) &&
      (skipcache_removelru() < 0))
    return -EAGAIN;

  /* link in hash list */
  new->next = tbl[idx];
  new->prev = NULL;

  if (tbl[idx])
    tbl[idx]->prev = new;
  tbl[idx] = new;
  skipcache_stat.entries++;

  return 0;
}

/*
 * returns a pointer to the cache structure for a given IP address
 * if no entry is found with a matching IP address, NULL is returned
 */
struct skipcache *skipcache_lookup(u_char *IPaddr)
{
  struct skipcache *walk;

  PRINTIPADDR("lookup", IPaddr);

  SEMLOCK(semaphore);
  for (walk = tbl[skipcache_index(IPaddr)]; walk; walk = walk->next)
    if (MEMCMP(walk->IPaddr, IPaddr, IPADDRSIZE) == 0)
      break;

  if (walk == NULL)
  {
    /* send reuqest to daemon if no entry was found */
    skipcache_stat.lookuprequests++;
  }
  else
  {
    /* inc reference count of entry */
    walk->ref++;
    skipcache_stat.lookuphits++;
  }

  SEMUNLOCK(semaphore);

  PRINTIPADDR("lookup end", IPaddr);

  return walk;
}

/*
 * release entry
 * if there are no more references, update and remove is checked.
 * if the entry is neither updated nor removed, its ttl is incremented
 */
int skipcache_release(struct skipcache *c)
{
  if (c == NULL)
    return -1;

  SEMLOCK(semaphore);
  if (--c->ref <= 0)
  {
    SEMUNLOCK(semaphore);
    if ((c->flags & SKIP_REMOVE) == 0)
      skipcache_feedqueues(c);
    skipcache_freeentry(c);
    return 0;
  }
  else if (c->flags & (SKIP_VALIDKEY | SKIP_NOKEY))
  {
    c->ttl += INCTTL;
    if (c->ttl > c->maxttl)
      c->ttl = c->maxttl;
  }
  SEMUNLOCK(semaphore);

  return 0;
}

int skipcache_getrequests(u_char *buf, int len)
{
  int walk, used;

  SEMLOCK(semaphore);

  for (walk = used = 0; walk < HASHTBLSIZE; walk++)
  {
    struct skipcache *c;

    for (c = tbl[walk]; c; c = c->next)
    {
      if ((c->flags & (SKIP_VALIDKEY | SKIP_NOKEY | SKIP_REQSENT)) == 0)
      {
	if (used <= (len - IPADDRSIZE))
	{
	  c->flags |= SKIP_REQSENT;
	  MEMCPY(buf, c->IPaddr, IPADDRSIZE);
	  buf += IPADDRSIZE;
	  used += IPADDRSIZE;
	}
	else
	{
	  SEMUNLOCK(semaphore);
	  return used;
	}
      }
    }
  }

  SEMUNLOCK(semaphore);

  return used;
}

/*
 * inserts the given entry in the hash table
 * if the entry already existed, the old entry's queues
 * are handed over to the IPSP object that treats them
 * with the updated keying information
 */
int skipcache_update(struct skipcache *new)
{
  int idx;
  struct skipcache *walk;

  /* no time to live? */
  if (new->ttl < 0)
    return -EINVAL;

  /* time to live to large */
  if (maxttl && (new->maxttl > maxttl))
    new->maxttl = maxttl;

  if (new->maxttl && (new->ttl > new->maxttl))
    new->ttl = new->maxttl;

  PRINTIPADDR("update", new->IPaddr);

  /* set starting ref count, init update pointer */
  new->ref = 1;

  /* Generate random packet key Kp for new entry */
  random_randomKp(new);


  idx = skipcache_index(new->IPaddr);

  SEMLOCK(semaphore);
  /*
   * search entry in hash table and remember last one to fix
   * the linked list afterwards.
   */
  for (walk = tbl[idx]; walk; walk = walk->next)
    if (MEMCMP(walk->IPaddr, new->IPaddr, IPADDRSIZE) == 0)
      break;

  /* already an entry in skipcache? */
  if (walk)
  {
    skipcache_stat.updated++;
    skipcache_replaceentry(walk, new);

    if (--walk->ref <= 0)
    {
      SEMUNLOCK(semaphore);
      skipcache_feedqueues(walk);
      skipcache_freeentry(walk);
      return 0;
    }
  }
  else
  {
    skipcache_stat.inserted++;
    skipcache_insertentry(new);
  }

  SEMUNLOCK(semaphore);
  return 0;
}

/*
 * removes an entry with given IP address from cache
 * returns 0 on success, -1 on failure
 */
int skipcache_remove(u_char *IPaddr)
{
  int idx;
  struct skipcache *walk, *last;

  idx = skipcache_index(IPaddr);

  SEMLOCK(semaphore);
  /*
   * search entry in hash table and remember last one to fix
   * the linked list afterwards.
   */
  for (walk = tbl[idx], last = NULL; walk; last = walk, walk = walk->next)
    if (MEMCMP(walk->IPaddr, IPaddr, IPADDRSIZE) == 0)
      break;

  if (walk)
  {
    skipcache_stat.removed++;
    skipcache_removeentry(walk);
    if (--walk->ref <= 0)
      skipcache_freeentry(walk);
    else
      walk->flags |= SKIP_REMOVE;

    SEMUNLOCK(semaphore);
    return 0;
  }

  SEMUNLOCK(semaphore);
  return -ENOENT;
}

static int skipcache_timeout(void *arg)
{
  static int times = 1;

  /*
   * this function should not sleep
   * so we just try to lock the semaphore
   * if we do not succeed, we expire the entries
   * another time.
   */
  if (SEMLOCKTRY(semaphore) == 0)
  {
    skipcache_age(times);
    SEMUNLOCK(semaphore);
    times = 0;
  }

  times += TIMERINTERVAL;
 

  TIMEOUT(skipcache_timeout, NULL, TIMERINTERVAL);
  return 0;
}

static int skipcache_reset(void)
{
  int walk;

  for (walk = 0; walk < HASHTBLSIZE; walk++)
    tbl[walk] = NULL;

  MEMZERO(&skipcache_stat, sizeof(skipcache_stat));

  return 0;
}

/* 
 * Flushes all entries from skipcache. Note: Also clears the cache statistics
 */
int skipcache_flush(void)
{
  int walk;

  SEMLOCK(semaphore);

  for (walk = 0; walk < HASHTBLSIZE; walk++)
  {
    struct skipcache *c, *next;

    for (c = tbl[walk]; c; c = next)
    {
      next = c->next;
      skipcache_freeentry(c);
    }
  }

  skipcache_reset();
  SEMUNLOCK(semaphore);

  return 0;
}

/*
 * initalize hash table
 */
int skipcache_init(int mttl, int ment)
{
  maxttl = mttl;
  maxent = ment;

  skipcache_reset();
  SEMALLOC(semaphore);
  SEMINIT(semaphore);
  TIMEOUT(skipcache_timeout, NULL, TIMERINTERVAL);

  return 0;
}

/*
 * free all entries in hash table
 */
int skipcache_exit(void)
{
  UNTIMEOUT(skipcache_timeout, NULL);
  skipcache_flush();
  SEMFREE(semaphore);

  return 0;
}
