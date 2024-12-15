/*
 * config_win32.h -- Configuration for PGPlib. This file contains
 * the configuration information for PGP, and it should be
 * included in all PGP source files.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: config_win32.h,v 1.6.2.2 1997/06/11 22:29:44 mhw Exp $
 */

/* Define to empty if the compiler does not support 'const' variables. */
/* #undef const */

/* Define to `long' if <sys/types.h> doesn't define. */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> doesn't define. */
/* #undef size_t */

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Checks for various specific header files */
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDLIB_H 1
#undef HAVE_UNISTD_H
#undef HAVE_SYS_IOCTL_H
#undef HAVE_SYS_TIME_H
#define HAVE_SYS_TIMEB_H 1
#undef HAVE_SYS_PARAM_H

/* Check if <sys/time.h> is broken and #includes <time.h> wrong */
#undef TIME_WITH_SYS_TIME

/* Checks for various functions */
#define HAVE_GETHRTIME 0
#define HAVE_CLOCK_GETTIME 0
#define HAVE_CLOCK_GETRES 0
#define HAVE_GETTIMEOFDAY 0
#define HAVE_GETITIMER 0
#define HAVE_SETITIMER 0
#define HAVE_FTIME 1

/* Define "UNIX" if we are on UNIX and "UNIX" is not already defined */
#if defined(unix) || defined(__unix) || defined (__unix__)
#ifndef UNIX
#define UNIX 1
#endif
#endif

#if (_MSC_VER>=1000) && defined(_WIN32)
#define _POSIX_ 1
#define pclose _pclose
#define popen _popen
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/* Tags for exported functions */
#if defined(_WIN32) && !defined(NODLL)
# if defined(PGPLIB)
# define PGPExport __declspec( dllexport )
# else
# define PGPExport __declspec( dllimport )
# endif
# if defined(PGPTTYLIB)
# define PGPTTYExport __declspec( dllexport )
# else
# define PGPTTYExport __declspec( dllimport )
# endif
# if defined(PGPKDBLIB)
# define PGPKDBExport __declspec( dllexport )
# else
# define PGPKDBExport __declspec( dllimport )
# endif
# if defined(SPGPLIB)
# define SPGPExport __declspec( dllexport )
# else
# define SPGPExport __declspec( dllimport )
# endif
#else
# define PGPExport
# define PGPTTYExport
# define PGPKDBExport
# define SPGPExport
#endif
