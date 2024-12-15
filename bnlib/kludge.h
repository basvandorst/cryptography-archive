#ifndef KLUDGE_H
#define KLUDGE_H

/*
 * Kludges for not-quite-ANSI systems.
 * This should always be the last file included, because it may
 * mess up some system header files.
 */

/* SunOS 4.1.x <assert.h> needs "stderr" defined, and "exit" declared... */
#ifdef assert
#if ASSERT_NEEDS_STDIO
#include <stdio.h>
#endif
#if ASSERT_NEEDS_STDLIB
#if !NO_STDLIB_H
#include <stdlib.h>
#endif
#endif
#endif

#if NO_MEMMOVE	/* memove() not in libraries */
#define memmove(dest,src,len) bcopy(src,dest,len)
#endif

#if NO_MEMCPY	/* memcpy() not in libraries */
#define memcpy(dest,src,len) bcopy(src,dest,len)
#endif

#if MEM_PROTOS_BROKEN
#define memcpy(d,s,l) memcpy((void *)(d), (void const *)(s), l)
#define memmove(d,s,l) memmove((void *)(d), (void const *)(s), l)
#define memcmp(d,s,l) memcmp((void const *)(d), (void const *)(s), l)
#define memset(d,v,l) memset((void *)(d), v, l)
#endif

/*
 * If there are no prototypes for the stdio functions, use these to
 * reduce compiler warnings.  Uses EOF as a giveaway to indicate
 * that <stdio.h> was #included.
 */
#if NO_STDIO_PROTOS	/* Missing prototypes for "simple" functions */
#if defined(EOF)
#ifdef __cplusplus
extern "C" {
#endif
int (puts)(char const *);
int (fputs)(char const *, FILE *);
void (rewind)(FILE *);
int (fflush)(FILE *);
int (fclose)(FILE *);
int (printf)(char const *, ...);
int (fprintf)(FILE *, char const *, ...);
int (fseek)(FILE *, long, int);
int (remove)(char const *);
int (rename)(char const *, char const *);
void (perror)(char const *);
int (pclose)(FILE *);
/* If we have a sufficiently old-fashioned stdio, it probably uses these... */
int (_flsbuf)(int, FILE *);
int (_filbuf)(FILE *);
int (ungetc)(int, FILE *);
size_t (fread)(char *, size_t, size_t, FILE *);
size_t (fwrite)(char const *, size_t, size_t, FILE *);
#if defined(va_start) || defined(va_arg) || defined(va_end)
int (vfprintf)(FILE *, char const *, ...);
#endif
#ifdef __cplusplus
}
#endif
#endif /* EOF */
#endif /* NO_STDIO_PROTOS */

/*
 * Borland C seems to think that it's a bad idea to decleare a
 * structure tag and not declare the contents.  I happen to think
 * it's a *good* idea to use such "opaque" structures wherever
 * possible.  So shut up.
 */
#ifdef __BORLANDC__
#pragma warn -stu
#ifndef MSDOS
#define MSDOS 1
#endif
#endif

/* Cope with people forgetting to define the OS, if possible... */

#ifndef MSDOS
#ifdef __MSDOS
#define MSDOS 1
#endif
#endif

#endif /* KLUDGE_H */
