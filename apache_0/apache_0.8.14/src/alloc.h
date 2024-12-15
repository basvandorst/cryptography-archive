
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/*
 * Resource allocation routines...
 *
 * designed so that we don't have to keep track of EVERYTHING so that
 * it can be explicitly freed later (a fundamentally unsound strategy ---
 * particularly in the presence of die()).
 *
 * Instead, we maintain pools, and allocate items (both memory and I/O
 * handlers) from the pools --- currently there are two, one for per
 * transaction info, and one for config info.  When a transaction is over,
 * we can delete everything in the per-transaction pool without fear, and
 * without thinking too hard about it either.
 *
 * rst
 */

/* Arenas for configuration info and transaction info
 * --- actual layout of the pool structure is private to 
 * alloc.c.  
 */

typedef struct pool pool;

extern pool *permanent_pool;
void init_alloc();		/* Set up everything */
pool *make_sub_pool (pool *);	/* All pools are subpools of permanent_pool */
void destroy_pool (pool *);

/* Clearing out EVERYTHING in an pool... destroys any sub-pools */

void clear_pool (struct pool *);

/* Preparing for exec() --- close files, etc., but *don't* flush I/O
 * buffers, *don't* wait for subprocesses, and *don't* free any memory.
 */

void cleanup_for_exec ();

/* routines to allocate memory from an pool... */

void *palloc(struct pool *, int nbytes);
void *pcalloc(struct pool *, int nbytes);
char *pstrdup(struct pool *, char *s);
char *pstrcat(struct pool *, ...); /* all '...' must be char* */

/* array and alist management... keeping lists of things.
 * Common enough to want common support code ...
 */

typedef struct {
    pool *pool;
    int elt_size;
    int nelts;
    int nalloc;
    char *elts;
} array_header;

array_header *make_array (pool *p, int nelts, int elt_size);
void *push_array (array_header *);
void array_cat (array_header *dst, array_header *src);
array_header *append_arrays (pool *, array_header *, array_header *);

/* copy_array copies the *entire* array.  copy_array_hdr just copies
 * the header, and arranges for the elements to be copied if (and only
 * if) the code subsequently does a push or arraycat.
 */
     
array_header *copy_array (pool *p, array_header *src);
array_header *copy_array_hdr (pool *p, array_header *src);
			   

/* Tables.  Implemented alist style, for now, though we try to keep
 * it so that imposing a hash table structure on top in the future
 * wouldn't be *too* hard...
 *
 * Note that key comparisons for these are case-insensitive, largely
 * because that's what's appropriate and convenient everywhere they're
 * currently being used...
 */

typedef array_header table;     
     
typedef struct {
    char *key;			/* maybe NULL in future;
				 * check when iterating thru table_elts
				 */
    char *val;
} table_entry;

table *make_table (pool *p, int nelts);
table *copy_table (pool *p, table *);     
char *table_get (table *, char *);
void table_set (table *, char *name, char *val);
void table_merge (table *, char *name, char *more_val);

table *overlay_tables (pool *p, table *overlay, table *base);     

array_header *table_elts (table *);     

/* routines to remember allocation of other sorts of things...
 * generic interface first.  Note that we want to have two separate
 * cleanup functions in the general case, one for exec() preparation,
 * to keep CGI scripts and the like from inheriting access to things
 * they shouldn't be able to touch, and one for actually cleaning up,
 * when the actual server process wants to get rid of the thing,
 * whatever it is.  
 *
 * kill_cleanup disarms a cleanup, presumably because the resource in
 * question has been closed, freed, or whatever, and it's scarce
 * enough to want to reclaim (e.g., descriptors).  It arranges for the
 * resource not to be cleaned up a second time (it might have been
 * reallocated).  run_cleanup does the same, but runs it first.
 *
 * Cleanups are identified for purposes of finding & running them off by the
 * plain_cleanup and data, which should presumably be unique.
 *
 * NB any code which invokes register_cleanup or kill_cleanup directly
 * is a critical section which should be guarded by block_alarms() and
 * unblock_alarms() below...
 */

void register_cleanup (pool *p, void *data,
		       void (*plain_cleanup)(void *),
		       void (*child_cleanup)(void *));

void kill_cleanup (pool *p, void *data, void (*plain_cleanup)(void *));

/* The time between when a resource is actually allocated, and when it
 * its cleanup is registered is a critical section, during which the
 * resource could leak if we got interrupted or timed out.  So, anything
 * which registers cleanups should bracket resource allocation and the
 * cleanup registry with these.  (This is done internally by run_cleanup).
 *
 * NB they are actually implemented in http_main.c, since they are bound
 * up with timeout handling in general...
 */

extern void block_alarms();
extern void unblock_alarms();

/* Common cases which want utility support..
 * the note_cleanups_for_foo routines are for 
 */

FILE *pfopen(struct pool *, char *name, char *fmode);
int popenf(struct pool *, char *name, int flg, int mode); 

void note_cleanups_for_file (pool *, FILE *);
void note_cleanups_for_fd (pool *, int);

/* routines to note closes... file descriptors are constrained enough
 * on some systems that we want to support this.
 */

int pfclose(struct pool *, FILE *);
int pclosef(struct pool *, int fd);

/* ... even child processes (which we may want to wait for,
 * or to kill outright, on unexpected termination).
 *
 * spawn_child is a utility routine which handles an awful lot of
 * the rigamarole associated with spawning a child --- it arranges
 * for pipes to the child's stdin and stdout, if desired (if not,
 * set the associated args to NULL).  It takes as args a function
 * to call in the child, and an argument to be passed to the function.
 */
     
enum kill_conditions { kill_never, kill_always, kill_after_timeout, just_wait};

int spawn_child (pool *, void (*)(void *), void *,
		 enum kill_conditions, FILE **pipe_in, FILE **pipe_out);

/* magic numbers --- only one so far, min free bytes in a new pool block */

#define BLOCK_MINFREE 8192     

/* Finally, some accounting */

long bytes_in_pool(pool *p);
long bytes_in_free_blocks();
