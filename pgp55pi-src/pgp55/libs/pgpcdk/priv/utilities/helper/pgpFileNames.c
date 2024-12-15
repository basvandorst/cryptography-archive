/*
 * pgpFileNames.c
 * Utility functions to work with filenames in different OS's
 * These functions all return newly allocated character arrays
 * which it is the responsibility of the caller to free.
 *
 * Based on pgpAppFile,
 *  written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFileNames.c,v 1.13 1997/08/29 03:36:00 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpFileNames.h"
#include "pgpMem.h"


/*
 * DIRSEPS is a string of possible directory-separation characters
 * The first one is the preferred one, which goes in between
 * PGPPATH and the file name if PGPPATH is not terminated with a
 * directory separator.
 *
 * PATHSEP is a the directory separator within a PATH of directories.
 * Only one character per platform, please
 */

#if defined(MSDOS) || defined(ATARI) || defined(_WIN32)
static char const DIRSEPS[] = "\\/:";
#define PATHSEP ';'
#define SINGLE_EXT 1

#elif defined(PGP_UNIX)
static char const DIRSEPS[] = "/";
#define PATHSEP ':'

#elif defined(AMIGA)
static char const DIRSEPS[] = "/:";
#define PATHSEP ';'

#elif defined(VMS)
static char const DIRSEPS[] = "]:";	/* Any more? */
#define PATHSEP ';'
#define SINGLE_EXT 1

#elif defined(PGP_MACINTOSH)
static char const DIRSEPS[] = ":";
#define PATHSEP ';'

#else
#error Unknown operating system - need one of\
 PGP_UNIX, MSDOS, AMIGA, ATARI, VMS, or PGP_MACINTOSH.
#endif

/*
 * Build a path name out of a number of segments.
 * Each segment is separated from the next with DIRSEPS[0] if
 * it does not end in a directory separator already.
 *
 * A first pass over the argument list adds up the length of the result,
 * then space is allocated for it, then a second pass copies the result
 * to the allocated space.
 *
 * The first pass records in "flag" whether the previous segment did NOT end
 * in a directory separator and, if the current segment exists, adds space
 * for a separator if flag is true.
 *
 * Then, if there was only one segment of note, that is returned directly.
 * Otherwise, space is allocated in the MiscPool.
 * Finally, the loop is repeated, copying to the allocated space and inserting
 * directory separators as needed.
 *
 * This function always returns a newly allocated buffer.  It is the
 * responsibility of the caller to free it when finished.
 */
char *
pgpFileNameBuild(PGPContextRef cdkContext, char const *seg, ...)
{
	va_list ap;
	unsigned seglen, len, flag;
	char const *curseg, *prevseg;
	char *p, *ret;

	/* Start by counting the length of the result */
	va_start (ap, seg);
	prevseg = 0;
	len = seglen = flag = 0;
	for (curseg = seg; curseg; curseg = va_arg(ap, char const *)) {
		seglen = strlen (curseg);
		if (!seglen)
			continue;
		prevseg = curseg;
		len += seglen + flag;
		/* Does curseg need a directory separator added? */
		flag = (strchr(DIRSEPS, curseg[seglen-1]) == 0);
	}
	va_end (ap);

	/* Allocate space for the combined string */
	p = (char *)pgpContextMemAlloc (cdkContext, len+1,
						 0); /* +1 for trailing null */
	if (!p)
		return 0;
	ret = p;

	/* Combine the elements together */
	va_start (ap, seg);
	flag = 0;
	for (curseg = seg; curseg; curseg = va_arg (ap, char const *)) {
		seglen = strlen (curseg);
		if (!seglen)
			continue;
		if (flag)
			*p++ = DIRSEPS[0];
		memcpy (p, curseg, seglen);
		p += seglen;
		/* Does curseg need a directory separator added? */
		flag = (strchr (DIRSEPS, curseg[seglen-1]) == 0);
	}
	va_end(ap);

	*p = '\0';	/* Null-terminate the string */
	
#if PGP_MACINTOSH
	pgpAssert( strlen( ret ) <= 255 );
#endif
	return ret;
}


#if 0
char *
pgpFileNameExtend (PGPContextRef cdkContext, char const *base, char const *ext)
{
	char *name = (char *)pgpContextMemAlloc (cdkContext,
				  strlen (base) + strlen (ext) + 1, 0);

	if (!name)
		return NULL;

	strcpy (name, base);
	strcat (name, ext);
	
#if PGP_MACINTOSH
	pgpAssert( strlen( name ) <= 255 );
#endif

	return name;
}

#endif

char *
pgpFileNameContract (PGPContextRef cdkContext, char const *base)
{
	char *p;
	char *name = (char *)pgpContextMemAlloc (cdkContext, strlen (base) + 1,
								   0);

	if (!name)
		return NULL;

	strcpy (name, base);
	/* remove an ending */
	p = strrchr (name, '.');
	if (p)
		*p = '\0';

	return name;
}

/* Return the directory part of the path holding the file */
char *
pgpFileNameDirectory (PGPContextRef cdkContext, char const *base)
{
	char *end1=0, *end2=0;
	char const *ds = DIRSEPS;
	char c;
	char *dir;

	while ((c = *ds++) != '\0') {
		if ((end2 = strrchr (base, c)) != NULL) {
			if (!end1 || end1 < end2)
				end1 = end2;
		}
	}
	/* If no path info, return . */
	if (!end1) {
		dir = (char *)pgpContextMemAlloc (cdkContext, 2,  0);
		if ( IsntNull( dir ) )
		{
			dir[0] = '.';
			dir[1] = '\0';
		}
		return dir;
	}
	
	dir = (char *)
		pgpContextMemAlloc (cdkContext, end1 - base + 1, 0);
	if ( IsntNull( dir ) )
	{
		strncpy (dir, base, end1-base);
		dir[end1-base] = '\0';
	}
	return dir;
}


char *
pgpFileNameNextDirectory (PGPContextRef cdkContext, char const *path,
						  char const **rest)
{
	char const *d;
	char *name;
	unsigned len;
	
	d = strchr (path, PATHSEP);
	if (d) {
		len = d - path;
		d++;
	} else
		len = strlen (path);

	if (rest)
		*rest = d;

	name = (char *)pgpContextMemAlloc (cdkContext, len + 1,
									   0);
	if (!name)
		return NULL;

	memcpy (name, path, len);
	name[len] = '\0';
	return name;
}

char *
pgpFileNameExtendPath (PGPContextRef cdkContext, char const *path,
					   char const *dir, int front)
{
	char *	p	= NULL;
	char *	t	= NULL;
	PGPUInt32	length;

	pgpAssert (dir);

	length	= IsntNull( path ) ? strlen (path) : 0;
	length	+= strlen (dir) + 2;
	
	p = (char *)pgpContextMemAlloc ( cdkContext,
		length, 0);
	if ( IsntNull( p ) ) {
		t = p;
		if (front) {
			memcpy (t, dir, strlen (dir));
			t += strlen (dir);
			if (path) {
				*(t++) = PATHSEP;
				memcpy (t, path, strlen (path));
				t += strlen (path);
			}
			*t = '\0';
		} else {
			if (path) {
				memcpy (t, path, strlen (path));
				t += strlen (path);
				*(t++) = PATHSEP;
			}
			memcpy (t, dir, strlen (dir));
			t += strlen (dir);
			*t = '\0';
		}
	}
	return p;
}

/*
 * This returns the filename (not including directory path)
 * It returns a pointer within <path>, rather than dynamically allocating it
 */
char *
pgpFileNameTail (PGPContextRef cdkContext, char const *path)
{
	char const *	tail;
	char const *	lastTail = path;
	char const *	dirSep;
	
	(void)cdkContext;

	for (dirSep = DIRSEPS; *dirSep; dirSep++)
	{
		tail = strrchr(lastTail, *dirSep);
		if (tail != NULL)
			lastTail = tail + 1;
	}
	return (char *)lastTail;
}

