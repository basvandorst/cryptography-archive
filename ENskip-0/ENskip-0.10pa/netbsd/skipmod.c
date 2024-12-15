/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/conf.h>
#include <sys/mount.h>
#include <sys/exec.h>
#include <sys/lkm.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <netinet/in.h>

#include "interface.h"
#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "random.h"
#include "ipsp.h"
#include "crypt.h"
#include "sign.h"

#define NUMENT (sizeof(newent) / sizeof(newent[0]))

extern int com_skipcall();

/* New system calls */
static struct sysent newent[] = 
{
  { 3, com_skipcall },  /* # of args, function pointer*/
};

/*
 * Miscellaneous modules must have their own save areas...
 */
static struct sysent  oldent[NUMENT];  /* save are for old callslot entry */

MOD_MISC("skipmod")


static int skipmod_wedge(struct lkm_misc *args, int num,
                         struct sysent *new, struct sysent *old)
{
  extern int nsysent;     /* init_sysent.c*/
  extern int lkmnosys();  /* allocable slot*/
  int i, j, result = ENFILE;  /* Default: No slot found */

  /*
   * Search the table looking for <num> free contiguous slots...
   */
  for (i = 0; i <= (nsysent - num); i++)
  {
    for (j = 0; j < num; j++)
    {
      if (sysent[i + j].sy_call != lkmnosys)
        break;  /* not found */
    }

    if (j == num)
      break;  /* found */
  }

  /* found allocable slots?*/
  if (i <= (nsysent - num)) 
  {
    args->lkm_offset = i;  /* slot in sysent[]*/
    bcopy(&sysent[i], old, num * sizeof(struct sysent));  /* Save old */
    bcopy(new, &sysent[i], num * sizeof(struct sysent));  /* Wedge in new */
    result = 0;
  }

  return result;
}

static int skipmod_init(void)
{
  crypt_init();
  sign_init();
  random_init();
  skipcache_init(SKIPCACHE_MAXTTL, SKIPCACHE_MAXENTRIES);
  ipsp_init();
  interface_init();

  return 0;
}

static int skipmod_exit(void)
{
  interface_exit();
  ipsp_exit();
  skipcache_exit();
  sign_exit();
  crypt_exit();

  return 0;
}

/*
 * This function is called each time the module is loaded or unloaded.
 * Since we are a miscellaneous module, we have to provide whatever
 * code is necessary to patch ourselves into the area we are being
 * loaded to change.
 *
 * For the system call table, we duplicate the code in the kern_lkm.c
 * file for patching into the system call table.  We can tell what
 * has been allocated, etc. by virtue of the fact that we know the
 * criteria used by the system call loader interface.  We still
 * kick out the copyright to the console here (to give an example).
 *
 * The stat information is basically common to all modules, so there
 * is no real issue involved with stat; we will leave it nosys(),
 * cince we don't have to do anything about it.
 */
static int skipmod_handle(struct lkm_table *lkmtp, int cmd)
{
  extern int lkmexists(struct lkm_table *);
  int i;
  struct lkm_misc *args = lkmtp->private.lkm_misc;
  int err = 0;            /* default = success*/

  switch (cmd) 
  {
    case LKM_E_LOAD:

      /*
       * Don't load twice! (lkmexists() is exported by kern_lkm.c)
       */
      if (lkmexists(lkmtp))
      {
        err = EEXIST;
        break;
      }

      if ((err = interface_init()))
        break;  /* Could not initialize interface - voodoo failed :-\ */
      skipmod_init();

      if ((err = skipmod_wedge(args, NUMENT, newent, oldent)))
      {
        /* Could not initialize system calls */
        skipmod_exit();
        break;  /* No slots found */
      }
    
      /* if we make it to here, print copyright on console*/
      printf( "Skipmod loaded\n");
      break;    /* Success*/

    case LKM_E_UNLOAD:
      skipmod_exit();

      /* current slot...*/
      i = args->lkm_offset;

      /* replace current slot contents with old contents*/
      bcopy(oldent, &sysent[i], sizeof(oldent));
      break;    /* Success*/

    default:  /* we only understand load/unload*/
      err = EINVAL;
      break;
  }

  return err;
}


/*
 * External entry point; should generally match name of .o file.  The
 * arguments are always the same for all loaded modules.  The "load",
 * "unload", and "stat" functions in "DISPATCH" will be called under
 * their respective circumstances unless their value is "nosys".  If
 * called, they are called with the same arguments (cmd is included to
 * allow the use of a single function, ver is included for version
 * matching between modules and the kernel loader for the modules).
 *
 * Since we expect to link in the kernel and add external symbols to
 * the kernel symbol name space in a future version, generally all
 * functions used in the implementation of a particular module should
 * be static unless they are expected to be seen in other modules or
 * to resolve unresolved symbols alread existing in the kernel (the
 * second case is not likely to ever occur).
 *
 * The entry point should return 0 unless it is refusing load (in which
 * case it should return an errno from errno.h).
 */
int skipmod(struct lkm_table *lkmtp, int cmd, int ver)
{
  extern int lkmdispatch(struct lkm_table *, int);

  DISPATCH(lkmtp, cmd, ver, skipmod_handle, skipmod_handle, nosys)
}
