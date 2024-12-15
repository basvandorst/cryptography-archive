/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef STDC_HEADERS

#if PGP_MACINTOSH || PGP_WIN32 || PGP_UNIX

#if PGP_MACINTOSH
#include <unix.h>
#include "pgpStrings.h"
#endif

#define strcasecmp    pgpCompareStringsIgnoreCase
#else
#include <sys/types.h>
#endif

#include <time.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include <stdio.h>

void *malloc (size_t);
int fclose (FILE *);
size_t fread (void *, size_t, size_t, FILE *);
size_t strftime (char *, size_t, const char *, const struct tm *);
int strcasecmp (const char *, const char *);
int getopt (int argc, char * const argv[], const char *optstring);

extern int optind;
extern char *optarg;

#endif /* STDC_HEADERS */

#define SKIPWS(c) while (isspace ((int) (*(c)))) c++
