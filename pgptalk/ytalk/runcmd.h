#ifndef _runcmd_h_
#  define _runcmd_h_
/* runcmd.h - run an external command via /bin/sh.  The difference to
   system(3) is that 1. /bin/sh is used always, and 2. that PATH is
   removed from the environment via putenv().  */
     
extern int run_shell_command ();

#endif /* _runcmd_h_ */
