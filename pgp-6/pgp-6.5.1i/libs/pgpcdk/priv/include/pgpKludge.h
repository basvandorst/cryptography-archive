/*
 * Kludges for not-quite-ANSI systems.
 * This should always be the last file included, because it may
 * mess up some system header files.
 *
 * $Id: pgpKludge.h,v 1.3 1997/05/13 01:19:26 mhw Exp $
 */

#ifndef Included_pgpKludge_h
#define Included_pgpKludge_h

PGP_BEGIN_C_DECLARATIONS

#if NO_FPOS_T	/* The whole fpos_t/fgetpos/fsetpos thing is missing */
typedef long fpos_t;
#define fgetpos(stream, pos) ((*(pos) = ftell(stream)) < 0 ? -1 : 0)
#define fsetpos(stream, pos) fseek(stream, *(pos), SEEK_SET)
#endif

#if NO_MEMMOVE	/* memove() not in libraries */
#define memmove(dest,src,len) bcopy(src,dest,len)
#endif

#if NO_STRERROR	/* strerror() not in libraries */

#ifndef sys_nerr
extern int sys_nerr;
#endif
#ifndef sys_errlist
extern char *sys_errlist[];
#endif
#define strerror(err) ((err) < sys_nerr ? sys_errlist[err] : "Unknwon error")

#endif /* NO_STRERROR */

#if NO_STRTOUL	/* strtoul() not in libraries */
#define strtoul strtol	/* Close enough */
#endif

#if NO_RAISE	/* raise() not in libraries */
#include <sys/types.h>	/* For getpid() - kill() is in <signal.h> */
#define raise(sig) kill(getpid(),sig)
#endif

/*
 * Would you believe that there exist (non-ANSI, of course) <stdio.h>'s that
 * don't define the SEEK_ macros?  If EOF is defined (a giveaway that
 * <stdio.h> has been included), define these to their usual meanings.
 */
#if defined(EOF)

#ifndef SEEK_SET
#define	SEEK_SET	0	/* Set file pointer to "offset" */
#define	SEEK_CUR	1	/* Set file pointer to current plus "offset" */
#define	SEEK_END	2	/* Set file pointer to EOF plus "offset" */
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#endif

#if NO_SIZE_T
typedef unsigned size_t; /* Actually, "int" gets used a lot... */
#endif

#if NO_STDIO_PROTOS	/* Missing prototypes for "simple" functions */
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
int (system)(char const *);	/* Really in <stdlib.h>, but this'll do... */
int (pclose)(FILE *);
/* If we have a sufficiently old-fashioned stdio, it probably uses these... */
int (_flsbuf)(unsigned char, FILE *);
int (_filbuf)(FILE *);
int (ungetc)(int, FILE *);
size_t (fread)(char *, size_t, size_t, FILE *);
size_t (fwrite)(char const *, size_t, size_t, FILE *);
#if defined(va_start) || defined(va_arg) || defined(va_end)
int (vfprintf)(FILE *, char const *, ...);
#endif
#endif /* NO_STDIO_PROTOS */

#endif /* EOF */

PGP_END_C_DECLARATIONS

#endif /* Included_pgpKludge_h */
