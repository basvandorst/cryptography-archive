
/*-
 * Copyright (c) 1995 The Apache Group. All rights reserved.
 * 
 *
 * Apache httpd license
 * ====================
 * 
 *
 * This is the license for the Apache Server. It covers all the
 * files which come in this distribution, and should never be removed.
 * 
 * The "Apache Group" has based this server, called "Apache", on
 * public domain code distributed under the name "NCSA httpd 1.3".
 * 
 * NCSA httpd 1.3 was placed in the public domain by the National Center 
 * for Supercomputing Applications at the University of Illinois 
 * at Urbana-Champaign.
 * 
 * As requested by NCSA we acknowledge,
 * 
 *  "Portions developed at the National Center for Supercomputing
 *   Applications at the University of Illinois at Urbana-Champaign."
 *
 * Copyright on the sections of code added by the "Apache Group" belong
 * to the "Apache Group" and/or the original authors. The "Apache Group" and
 * authors hereby grant permission for their code, along with the
 * public domain NCSA code, to be distributed under the "Apache" name.
 * 
 * Reuse of "Apache Group" code outside of the Apache distribution should
 * be acknowledged with the following quoted text, to be included with any new
 * work;
 * 
 * "Portions developed by the "Apache Group", taken with permission 
 *  from the Apache Server   http://www.apache.org/apache/   "
 *
 *
 * Permission is hereby granted to anyone to redistribute Apache under
 * the "Apache" name. We do not grant permission for the resale of Apache, but
 * we do grant permission for vendors to bundle Apache free with other software,
 * or to charge a reasonable price for redistribution, provided it is made
 * clear that Apache is free. Permission is also granted for vendors to 
 * sell support for Apache. We explicitly forbid the redistribution of 
 * Apache under any other name.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/*
 * Resource allocation code... the code here is responsible for making
 * sure that nothing leaks.
 *
 * rst --- 4/95 --- 6/95
 */

#include "conf.h"
#include "alloc.h"

#include <stdarg.h>
#include <memory.h>

/*****************************************************************
 *
 * Managing free storage blocks...
 */

union align
{
  /* Types which are likely to have the longest RELEVANT alignment
   * restrictions... we don't do much with doubles.
   */
  
  char *cp;
  void (*f)();
  long l;
  FILE *fp;
};

#define CLICK_SZ (sizeof(union align))

union block_hdr
{
  union align a;
  
  /* Actual header... */
  
  struct {
    char *endp;
    union block_hdr *next;
    char *first_avail;
  } h;
};

union block_hdr *block_freelist = NULL;



/* Get a completely new block from the system pool */

union block_hdr *malloc_block (int size)
{
  union block_hdr *blok =
    (union block_hdr *)malloc(size + sizeof(union block_hdr));

  if (blok == NULL) return NULL;
  
  blok->h.next = NULL;
  blok->h.first_avail = (char *)(blok + 1);
  blok->h.endp = size + blok->h.first_avail;
  
  return blok;
}



void chk_on_blk_list (union block_hdr *blok, union block_hdr *free_blk)
{
  /* Debugging code.  Left in for the moment. */
    
  while (free_blk) {
    if (free_blk == blok) {
      fprintf (stderr, "Ouch!  Freeing free block\n");
      exit (1);
    }
    free_blk = free_blk->h.next;
  }
}

/* Free a chain of blocks --- must be called with alarms blocked. */

void free_blocks (union block_hdr *blok)
{
  /* First, put new blocks at the head of the free list ---
   * we'll eventually bash the 'next' pointer of the last block
   * in the chain to point to the free blocks we already had.
   */
  
  union block_hdr *old_free_list = block_freelist;

  if (blok == NULL) return;	/* Sanity check --- freeing empty pool? */
  
  block_freelist = blok;
  
  /*
   * Next, adjust first_avail pointers of each block --- have to do it
   * sooner or later, and it simplifies the search in new_block to do it
   * now.
   */

  while (blok->h.next != NULL) {
    chk_on_blk_list (blok, old_free_list);
    blok->h.first_avail = (char *)(blok + 1);
    blok = blok->h.next;
  }

  chk_on_blk_list (blok, old_free_list);
  blok->h.first_avail = (char *)(blok + 1);

  /* Finally, reset next pointer to get the old free blocks back */

  blok->h.next = old_free_list;
}




/* Get a new block, from our own free list if possible, from the system
 * if necessary.  Must be called with alarms blocked.
 */

union block_hdr *new_block (int min_size)
{
  union block_hdr **lastptr = &block_freelist;
  union block_hdr *blok = block_freelist;
  
  /* First, see if we have anything of the required size
   * on the free list...
   */

  min_size += BLOCK_MINFREE;

  while (blok != NULL) {
    if (min_size <= blok->h.endp - blok->h.first_avail) {
      
      *lastptr = blok->h.next;
      blok->h.next = NULL;
      return blok;
    }
    else {
      lastptr = &blok->h.next;
      blok = blok->h.next;
    }
  }

  /* Nope. */

  return malloc_block (min_size);
}



/* Accounting */

long bytes_in_block_list (union block_hdr *blok)
{
  long size = 0;

  while (blok) {
    size += blok->h.endp - (char *)(blok + 1);
    blok = blok->h.next;
  }

  return size;
}


/*****************************************************************
 *
 * Pool internals and management...
 * NB that subprocesses are not handled by the generic cleanup code,
 * basically because we don't want cleanups for multiple subprocesses
 * to result in multiple three-second pauses.
 */

struct process_chain;
struct cleanup;

static void run_cleanups (struct cleanup *);
static void free_proc_chain (struct process_chain *);

struct pool {
  union block_hdr *first;
  union block_hdr *last;
  struct cleanup *cleanups;
  struct process_chain *subprocesses;
  struct pool *sub_pools;
  struct pool *sub_next;
  struct pool *sub_prev;
  struct pool *parent;
  char *free_first_avail;
};

pool *permanent_pool;

/* Each pool structure is allocated in the start of its own first block,
 * so we need to know how many bytes that is (once properly aligned...).
 * This also means that when a pool's sub-pool is destroyed, the storage
 * associated with it is *completely* gone, so we have to make sure it
 * gets taken off the parent's sub-pool list...
 */

#define POOL_HDR_CLICKS (1 + ((sizeof(struct pool) - 1) / CLICK_SZ))
#define POOL_HDR_BYTES (POOL_HDR_CLICKS * CLICK_SZ)			 

struct pool *make_sub_pool (struct pool *p)
{
  union block_hdr *blok;
  pool *new_pool;

  block_alarms();
  
  blok = new_block (0);
  new_pool = (pool *)blok->h.first_avail;
  blok->h.first_avail += POOL_HDR_BYTES;

  memset ((char *)new_pool, '\0', sizeof (struct pool));
  new_pool->free_first_avail = blok->h.first_avail;
  new_pool->first = new_pool->last = blok;
    
  if (p) {
    new_pool->parent = p;
    new_pool->sub_next = p->sub_pools;
    if (new_pool->sub_next) new_pool->sub_next->sub_prev = new_pool;
    p->sub_pools = new_pool;
  }
  
  unblock_alarms();
  
  return new_pool;
}

void init_alloc() { permanent_pool = make_sub_pool (NULL); }

void clear_pool (struct pool *a)
{
  block_alarms();
  
  while (a->sub_pools)
    destroy_pool (a->sub_pools);
    
  a->sub_pools = NULL;
  
  run_cleanups (a->cleanups);        a->cleanups = NULL;
  free_proc_chain (a->subprocesses); a->subprocesses = NULL;
  free_blocks (a->first->h.next);    a->first->h.next = NULL;

  a->last = a->first;
  a->first->h.first_avail = a->free_first_avail;

  unblock_alarms();
}

void destroy_pool (pool *a)
{
  block_alarms();
  clear_pool (a);

  if (a->parent) {
    if (a->parent->sub_pools == a) a->parent->sub_pools = a->sub_next;
    if (a->sub_prev) a->sub_prev->sub_next = a->sub_next;
    if (a->sub_next) a->sub_next->sub_prev = a->sub_prev;
  }
  
  free_blocks (a->first);
  unblock_alarms();
}

long bytes_in_pool (pool *p) { return bytes_in_block_list (p->first); }
long bytes_in_free_blocks () { return bytes_in_block_list (block_freelist); }

/*****************************************************************
 *
 * Allocating stuff...
 */


void *palloc (struct pool *a, int reqsize)
{
  /* Round up requested size to an even number of alignment units (core clicks)
   */
  
  int nclicks = 1 + ((reqsize - 1) / CLICK_SZ);
  int size = nclicks * CLICK_SZ;
  
  /* First, see if we have space in the block most recently
   * allocated to this pool
   */
  
  union block_hdr *blok = a->last; 
  char *first_avail = blok->h.first_avail;
  char *new_first_avail;

  if (size <= 0) size = 1;
  new_first_avail = first_avail + size;
  
  if (new_first_avail <= blok->h.endp) {
    blok->h.first_avail = new_first_avail;
    return (void *)first_avail;
  }

  /* Nope --- get a new one that's guaranteed to be big enough */
  
  block_alarms();
  blok = new_block (size);
  a->last->h.next = blok;
  a->last = blok;
  unblock_alarms();

  first_avail = blok->h.first_avail;
  blok->h.first_avail += size;

  return (void *)first_avail;
}

void *pcalloc(struct pool *a, int size)
{
  void *res = palloc (a, size);
  memset (res, '\0', size);
  return res;
}

char *pstrdup(struct pool *a, char *s)
{
  char *res;
  if (s == NULL) return NULL;
  res = palloc (a, strlen(s) + 1);
  strcpy (res, s);
  return res;
}

char *pstrcat(pool *a, ...)
{
  char *cp, *argp, *res;
  
  /* Pass one --- find length of required string */
  
  int len = 0;
  va_list adummy;
  
  va_start (adummy, a);

  while ((cp = va_arg (adummy, char *)) != NULL) 
    len += strlen(cp);

  va_end (adummy);

  /* Allocate the required string */

  res = (char *)palloc(a, len + 1);
  cp = res;

  /* Pass two --- copy the argument strings into the result space */

  va_start (adummy, a);
  
  while ((argp = va_arg (adummy, char *)) != NULL) {
    strcpy (cp, argp);
    cp += strlen(argp);
  }

  va_end (adummy);

  /* Return the result string */

  return res;
}


/*****************************************************************
 *
 * The 'array' functions...
 */

array_header *make_array (pool *p, int nelts, int elt_size)
{
  array_header *res = (array_header *)palloc(p, sizeof(array_header));

  if (nelts < 1) nelts = 1;	/* Assure sanity if someone asks for
				 * array of zero elts.
				 */
  
  res->elts = pcalloc (p, nelts * elt_size);
  
  res->pool = p;
  res->elt_size = elt_size;
  res->nelts = 0;		/* No active elements yet... */
  res->nalloc = nelts;		/* ...but this many allocated */

  return res;
}

void *push_array (array_header *arr)
{
  if (arr->nelts == arr->nalloc) {
    char *new_data = pcalloc (arr->pool, arr->nalloc * arr->elt_size * 2);

    memcpy (new_data, arr->elts, arr->nalloc * arr->elt_size);
    arr->elts = new_data;
    arr->nalloc *= 2;
  }

  ++arr->nelts;
  return arr->elts + (arr->elt_size * (arr->nelts - 1));
}

void array_cat (array_header *dst, array_header *src)
{
  int elt_size = dst->elt_size;
  
  if (dst->nelts + src->nelts > dst->nalloc) {
    int new_size = dst->nalloc * 2;
    char *new_data;

    if (new_size == 0) ++new_size;
    
    while (dst->nelts + src->nelts > new_size)
      new_size *= 2;

    new_data = pcalloc (dst->pool, elt_size * new_size);
    memcpy (new_data, dst->elts, dst->nalloc * elt_size);
    
    dst->elts = new_data;
    dst->nalloc = new_size;
  }

  memcpy (dst->elts + dst->nelts * elt_size, src->elts, elt_size * src->nelts);
  dst->nelts += src->nelts;
}

array_header *copy_array (pool *p, array_header *arr)
{
  array_header *res = make_array (p, arr->nalloc, arr->elt_size);

  memcpy (res->elts, arr->elts, arr->elt_size * arr->nelts);
  res->nelts = arr->nelts;
  return res;
}

/* This cute function copies the array header *only*, but arranges
 * for the data section to be copied on the first push or arraycat.
 * It's useful when the elements of the array being copied are
 * read only, but new stuff *might* get added on the end; we have the
 * overhead of the full copy only where it is really needed.
 */

array_header *copy_array_hdr (pool *p, array_header *arr)
{
  array_header *res = (array_header *)palloc(p, sizeof(array_header));

  res->elts = arr->elts;
  
  res->pool = p;
  res->elt_size = arr->elt_size;
  res->nelts = arr->nelts;
  res->nalloc = arr->nelts;	/* Force overflow on push */

  return res;
}

/* The above is used here to avoid consing multiple new array bodies... */

array_header *append_arrays (pool *p,
			     array_header *first, array_header *second)
{
  array_header *res = copy_array_hdr (p, first);

  array_cat (res, second);
  return res;
}


/*****************************************************************
 *
 * The "table" functions.
 */

table *make_table (pool *p, int nelts) {
    return make_array (p, nelts, sizeof (table_entry));
}

table *copy_table (pool *p, table *t) {
    return copy_array (p, t);
}

array_header *table_elts (table *t) { return t; }

char *table_get (table *t, char *key)
{
    table_entry *elts = (table_entry *)t->elts;
    int i;

    for (i = 0; i < t->nelts; ++i)
        if (!strcasecmp (elts[i].key, key))
	    return elts[i].val;

    return NULL;
}

void table_set (table *t, char *key, char *val)
{
    table_entry *elts = (table_entry *)t->elts;
    int i;

    for (i = 0; i < t->nelts; ++i)
        if (!strcasecmp (elts[i].key, key)) {
	    elts[i].val = pstrdup (t->pool, val);
	    return;
	}

    elts = (table_entry *)push_array(t);
    elts->key = pstrdup (t->pool, key);
    elts->val = pstrdup (t->pool, val);
}

void table_merge (table *t, char *key, char *val)
{
    table_entry *elts = (table_entry *)t->elts;
    int i;

    for (i = 0; i < t->nelts; ++i)
        if (!strcasecmp (elts[i].key, key)) {
	    elts[i].val = pstrcat (t->pool, elts[i].val, ", ", val, NULL);
	    return;
	}

    elts = (table_entry *)push_array(t);
    elts->key = pstrdup (t->pool, key);
    elts->val = pstrdup (t->pool, val);
}

table* overlay_tables (pool *p, table *overlay, table *base)
{
    return append_arrays (p, overlay, base);
}

/*****************************************************************
 *
 * Managing generic cleanups.  
 */

struct cleanup {
  void *data;
  void (*plain_cleanup)(void *);
  void (*child_cleanup)(void *);
  struct cleanup *next;
};

void register_cleanup (pool *p, void *data, void (*plain_cleanup)(void *),
		       void (*child_cleanup)(void *))
{
  struct cleanup *c = (struct cleanup *)palloc(p, sizeof (struct cleanup));
  c->data = data;
  c->plain_cleanup = plain_cleanup;
  c->child_cleanup = child_cleanup;
  c->next = p->cleanups;
  p->cleanups = c;
}

void kill_cleanup (pool *p, void *data, void (*cleanup)(void *))
{
  struct cleanup *c = p->cleanups;
  struct cleanup **lastp = &p->cleanups;
    
  while (c) {
    if (c->data == data && c->plain_cleanup == cleanup) {
      *lastp = c->next;
      break;
    }

    lastp = &c->next;
    c = c->next;
  }
}

void run_cleanup (pool *p, void *data, void (*cleanup)(void *))
{
  block_alarms();		/* Run cleanup only once! */
  (*cleanup)(data);
  kill_cleanup (p, data, cleanup);
  unblock_alarms();
}

static void run_cleanups (struct cleanup *c)
{
  while (c) {
    (*c->plain_cleanup)(c->data);
    c = c->next;
  }
}

static void run_child_cleanups (struct cleanup *c)
{
  while (c) {
    (*c->child_cleanup)(c->data);
    c = c->next;
  }
}

static void cleanup_pool_for_exec (pool *p)
{
  run_child_cleanups (p->cleanups);
  p->cleanups = NULL;

  for (p = p->sub_pools; p; p = p->sub_next)
    cleanup_pool_for_exec (p);
}

void cleanup_for_exec()
{
  block_alarms();
  cleanup_pool_for_exec (permanent_pool);
  unblock_alarms();
}

/*****************************************************************
 *
 * Files and file descriptors; these are just an application of the
 * generic cleanup interface.
 */

static void fd_cleanup (void *fdv) { close ((int)fdv); }

void note_cleanups_for_fd (pool *p, int fd) {
  register_cleanup (p, (void *)fd, fd_cleanup, fd_cleanup);
}

int popenf(struct pool *a, char *name, int flg, int mode)
{
  int fd;

  block_alarms();
  fd = open(name, flg, mode);
  if (fd >= 0) note_cleanups_for_fd (a, fd);
  unblock_alarms();
  return fd;
}

int pclosef(struct pool *a, int fd)
{
  int res;
  
  block_alarms();
  res = close(fd);
  kill_cleanup(a, (void *)fd, fd_cleanup);
  unblock_alarms();
  return res;
}

/* Note that we have separate plain_ and child_ cleanups for FILE *s,
 * since fclose() would flush I/O buffers, which is extremely undesirable;
 * we just close the descriptor.
 */

static void file_cleanup (void *fpv) { fclose ((FILE *)fpv); }
static void file_child_cleanup (void *fpv) { close (fileno ((FILE *)fpv)); }

void note_cleanups_for_file (struct pool *p, FILE *fp) {
  register_cleanup (p, (void *)fp, file_cleanup, file_child_cleanup);
}

FILE *pfopen(struct pool *a, char *name, char *mode)
{
  FILE *fd;

  block_alarms();
  fd = fopen(name, mode);
  if (fd != NULL) note_cleanups_for_file (a, fd);
  unblock_alarms();
  return fd;
}

int pfclose(struct pool *a, FILE *fd)
{
  int res;
  
  block_alarms();
  res = fclose(fd);
  kill_cleanup(a, (void *)fd, file_cleanup);
  unblock_alarms();
  return res;
}

/*****************************************************************
 *
 * More grotty system stuff... subprocesses.  Frump.  These don't use
 * the generic cleanup interface because I don't want multiple
 * subprocesses to result in multiple three-second pauses; the
 * subprocesses have to be "freed" all at once.  If someone comes
 * along with another resource they want to allocate which has the
 * same property, we might want to fold support for that into the
 * generic interface, but for now, it's a special case
 */

struct process_chain {
  pid_t pid;
  enum kill_conditions kill_how;
  struct process_chain *next;
};

void note_subprocess (pool *a, int pid, enum kill_conditions how)
{
  struct process_chain *new =
    (struct process_chain *)palloc(a, sizeof(struct process_chain));

  new->pid = pid;
  new->kill_how = how;
  new->next = a->subprocesses;
  a->subprocesses = new;
}

int spawn_child (pool *p, void (*func)(void *), void *data,
		 enum kill_conditions kill_how,
		 FILE **pipe_in, FILE **pipe_out)
{
  int pid;
  int in_fds[2];
  int out_fds[2];

  block_alarms();
  
  if (pipe_in && pipe (in_fds) < 0) return 0;
  
  if (pipe_out && pipe (out_fds) < 0) {
    if (pipe_in) {
      close (in_fds[0]); close (in_fds[1]);
    }
    unblock_alarms();
    return 0;
  }

  if ((pid = fork()) < 0) {
    if (pipe_in) {
      close (in_fds[0]); close (in_fds[1]);
    }
    if (pipe_out) {
      close (out_fds[0]); close (out_fds[1]);
    }
    unblock_alarms();
    return 0;
  }

  if (!pid) {
    /* Child process */
    
    if (pipe_out) {
      close (out_fds[0]);
      dup2 (out_fds[1], STDOUT_FILENO);
      close (out_fds[1]);
    }

    if (pipe_in) {
      close (in_fds[1]);
      dup2 (in_fds[0], STDIN_FILENO);
      close (in_fds[0]);
    }

    /* HP-UX SIGCHLD fix goes here, if someone will remind me what it is... */
    signal (SIGCHLD, SIG_DFL);	/* Was that it? */
    
    func (data);
    exit (0);			/* Should never get here... */
  }

  /* Parent process */

  note_subprocess (p, pid, kill_how);
  
  if (pipe_out) {
    close (out_fds[1]);
    *pipe_out = fdopen (out_fds[0], "r");
    
    if (*pipe_out) note_cleanups_for_file (p, *pipe_out);
  }

  if (pipe_in) {
    close (in_fds[0]);
    *pipe_in = fdopen (in_fds[1], "w");
    
    if (*pipe_in) note_cleanups_for_file (p, *pipe_in);
  }

  unblock_alarms();
  return pid;
}

static void free_proc_chain (struct process_chain *procs)
{
  /* Dispose of the subprocesses we've spawned off in the course of
   * whatever it was we're cleaning up now.  This may involve killing
   * some of them off...
   */

  struct process_chain *p;
  int need_timeout = 0;
  int status;

  if (procs == NULL) return;	/* No work.  Whew! */

  /* First, check to see if we need to do the SIGTERM, sleep, SIGKILL
   * dance with any of the processes we're cleaning up.  If we've got
   * any kill-on-sight subprocesses, ditch them now as well, so they
   * don't waste any more cycles doing whatever it is that they shouldn't
   * be doing anymore.
   */

  for (p = procs; p; p = p->next) {
    if (p->kill_how == kill_after_timeout) {
      /* Subprocess may be dead already.  Only need the timeout if not. */
      if (kill (p->pid, SIGTERM) != -1)
	need_timeout = 1;
    } else if (p->kill_how == kill_always) {
      kill (p->pid, SIGKILL);
    }
  }

  /* Sleep only if we have to... */

  if (need_timeout) sleep (3);

  /* OK, the scripts we just timed out for have had a chance to clean up
   * --- now, just get rid of them, and also clean up the system accounting
   * goop...
   */

  for (p = procs; p; p = p->next){
    
    if (p->kill_how == kill_after_timeout) 
      kill (p->pid, SIGKILL);

    if (p->kill_how != kill_never)
      waitpid (p->pid, &status, 0);
  }
}
