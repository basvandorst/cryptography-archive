/* tailor.h -- Not copyrighted 1991 Mark Adler */
/*
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpZTailor.h,v 1.3 1997/11/06 20:00:21 lloyd Exp $
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
 * Likewise we don't need DOS kludges for 32 bit windows versions
 */
#ifdef PGP_WIN32
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
#  define far  __far
#  define near __near

#else /* !MSDOS */

/* This used to do #define DYN_ALLOC 0, but since #ifdef DYN_ALLOC is used
 * everywhere the effect was to turn on dynamic allocation, which did not
 * seem likely to be what was intended, because of the 0.  So for now we
 * will leave DYN_ALLOC in whatever defined or undefined state it has been
 * set in previously.
 */

#  define far
#  define near

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
