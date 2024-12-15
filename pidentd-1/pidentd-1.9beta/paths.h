/*
** paths.h
**
** Common path definitions for the in.identd daemon
*/

#ifdef SEQUENT
#  define _PATH_UNIX "/dynix"
#endif

#ifdef MIPS
#  define _PATH_UNIX "/unix"
#endif

#ifdef hpux
#  define _PATH_UNIX "/hp-ux"
#endif

/*
 * Some defaults...
 */
#ifndef _PATH_KMEM
#  define _PATH_KMEM "/dev/kmem"
#endif

#ifndef _PATH_UNIX
#  define _PATH_UNIX "/vmunix"
#endif
