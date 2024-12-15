/* tailor.h -- Not copyrighted 1991 Mark Adler */
/*
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpZTailor.h,v 1.2 1997/06/24 07:16:25 lloyd Exp $
 */

#ifndef MSDOS
#  ifdef __MSDOS__
#    define MSDOS 1
#  endif
#endif

/* A couple of other compilers also behave like Turbo C */
#ifndef __TURBOC__
#  ifdef __POWERC                 /* For Power C too */
#    define __TURBOC__
#  endif
#  ifdef __BORLANDC__
#    define __TURBOC__
#  endif
#endif

/*
 * We use MSDOS to signal a need for 16-bit kludges and all the near/far crap,
 * so DJGPP basically isn't MSDOS.
 */
#ifdef __GNUC__
#  undef  MSDOS
#endif

/*
 * Allow far and huge allocation for small model (Microsoft C or Turbo C)
 * unless NOFAR defined (needed for ANSI mode compilations)
 * But if we're using DJGPP, and want to use the optimized 386 assembler
 * zmatch routines, we can't use DYN_ALLOC.  But gcc has virtual memory...
 */
#ifdef MSDOS

#  ifndef DYN_ALLOC
#    define DYN_ALLOC 1
#  endif
#  ifdef __TURBOC__
#    include <alloc.h>
#    ifdef __STDC__
     /* Damn Borland C won't declare these ifdef __STDC__. */
void __far * _Cdecl farcalloc(unsigned long __nunits, unsigned long __unitsz);
void        _Cdecl farfree(void __far *__block);
#    endif
#    define fcalloc farcalloc  /* Assumes that all arrays are < 64K for MSDOS*/
#    define fcfree  farfree
#  else /* !__TURBOC__ */
#    include <malloc.h>
#    define fcalloc(nitems,itemsize) halloc((long)(nitems),(itemsize))
#    define fcfree hfree
#  endif /* ?__TURBOC__ */
#  define far  __far
#  define near __near

#else /* !MSDOS */

#  ifndef DYN_ALLOC
#    define DYN_ALLOC 0
#  endif
#  define far
#  define near
#  define fcalloc(nitems,itemsize)	pgpAlloc( (long)(nitems)*(itemsize))
#  define fcfree    				pgpFree

#endif /* !MSDOS */

#include <stdlib.h>

/* Define this symbol if your target allows access to unaligned data.
 * This is not mandatory, just a speed optimization. The compressed
 * output is strictly identical.
 */
#ifndef UNALIGNED_OK
#if defined(MSDOS) || defined(M_XENIX) || defined(i386) || defined(__i386__)
#  define UNALIGNED_OK 1
#elif defined(mc68020) || defined(__mc68020__)
#  define UNALIGNED_OK 1
#elif defined(vax) || defined(__vax__)
#  define UNALIGNED_OK 1
#else
#  define UNALIGNED_OK 0
#endif
#endif

/* end of tailor.h */
