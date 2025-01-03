/* Generated automatically from ./config.c.in by makesetup. */
/* -*- C -*- ***********************************************
Copyright 1991, 1992, 1993, 1994 by Stichting Mathematisch Centrum,
Amsterdam, The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/* Universal Python configuration file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "myproto.h"
#include "mymalloc.h"
#include "osdefs.h"
#include "intrcheck.h"


#ifndef NO_MAIN

/* Normally, the main program is called from here (so everything else
   can be in libPython.a).  We save a pointer to argv[0] because it
   may be needed for dynamic loading of modules in import.c.  If you
   have your own main program and want to use non-SunOS dynamic
   loading, you will have to provide your own version of
   getprogramname(). */

static char *argv0;

main(argc, argv)
	int argc;
	char **argv;
{
#ifdef macintosh
	wargs(&argc, &argv);
#endif
	argv0 = argv[0];
	realmain(argc, argv);
}

char *
getprogramname()
{
	return argv0;
}

#endif


/* Return the initial python search path.  This is called once from
   initsys() to initialize sys.path.
   The environment variable PYTHONPATH is fetched and the default path
   appended.  (The Mac has no environment variables, so there the
   default path is always returned.)  The default path may be passed
   to the preprocessor; if not, a system-dependent default is used. */

#ifndef PYTHONPATH
#ifdef macintosh
#define PYTHONPATH ": :Lib :Lib:stdwin :Demo"
#endif /* macintosh */
#endif /* !PYTHONPATH */

#ifndef PYTHONPATH
#if defined(MSDOS) || defined(NT)
#define PYTHONPATH ".;..\\lib;\\python\\lib"
#endif /* MSDOS || NT */
#endif /* !PYTHONPATH */

#ifndef PYTHONPATH
#define PYTHONPATH ".:/usr/local/lib/python"
#endif /* !PYTHONPATH */

extern char *getenv();

char *
getpythonpath()
{
#ifdef macintosh
	return PYTHONPATH;
#else /* !macintosh */
	char *path = getenv("PYTHONPATH");
	char *defpath = PYTHONPATH;
	char *buf;
	char *p;
	int n;

	if (path == 0 || *path == '\0')
		return defpath;
	n = strlen(path) + strlen(defpath) + 2;
	buf = malloc(n);
	if (buf == NULL)
		return path; /* XXX too bad -- but not likely */
	strcpy(buf, path);
	p = buf + strlen(buf);
	*p++ = DELIM;
	strcpy(p, defpath);
	return buf;
#endif /* !macintosh */
}


/* Table of built-in modules.
   These are initialized when first imported.
   Note: selection of optional extensions is now generally done by the
   makesetup script. */

extern void initarray();
extern void initmath();
extern void initparser();
extern void initregex();
extern void initstrop();
extern void initstruct();
extern void initfcntl();
extern void initmd5();
extern void initrotor();
extern void initnews();

/* -- ADDMODULE MARKER 1 -- */

extern void initmarshal();

struct {
	char *name;
	void (*initfunc)();
} inittab[] = {

	{"array", initarray},
	{"math", initmath},
	{"parser", initparser},
	{"regex", initregex},
	{"strop", initstrop},
	{"struct", initstruct},
	{"md5", initmd5},
	{"rotor", initrotor},
	{"news", initnews},

/* -- ADDMODULE MARKER 2 -- */

	/* This module "lives in" with marshal.c */
	{"marshal", initmarshal},

	/* These entries are here for sys.builtin_module_names */
	{"__main__", NULL},
	{"__builtin__", NULL},
	{"sys", NULL},

	/* Sentinel */
	{0, 0}
};

#ifdef USE_FROZEN
#include "frozen.c"
#else
struct frozen {
	char *name;
	char *code;
	int size;
} frozen_modules[] = {
	{0, 0, 0}
};
#endif
