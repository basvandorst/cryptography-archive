/*
 * pgpKeyRings.c -- opens all the known keyrings
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpKeyRings.c,v 1.5.2.3.2.1 1997/07/14 16:29:54 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* For unlink */
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>		/* For O_CREAT, O_EXCL */
#endif

#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpEnv.h"
#include "pgpFile.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"

#include "pgpAppFile.h"
#include "pgpKeyRings.h"
#include "pgpOutput.h"

/* Maintain linked list of open keyrings and keyfiles so they can be
   rewritten if required. */

static struct OpenFile {
  struct OpenFile    *next;
  struct PgpFile     *pfp;
  struct RingFile    *ringfile;
  struct RingSet     *newset;
  int                 trusted;
  int                 flags;
  char                filename[1];
} openfiles = {NULL, NULL, NULL, 0, 0};

/*
 * Create a temp key file which is not used by anyone else.
 * Return a stream pointer to the file, and set the file name in
 * *pnfile.  Base the file name on the given file.
 * The returned file name needs to be free'd with pgpMemFree, and of
 * course the returned file handle needs to be closed with fclose.
 * The implementation of this tends to be OS (and possibly file system)
 * dependent.
 */
static FILE *
tmpkeyfile (char const *file, char **pnfile, char const *mode)
{
	char *tmp = fileNameContract(file);
	char *nfile = fileNameExtend(tmp, ".pg0");
	char *nfilep = nfile + strlen(file) - 1;
	FILE *stdFile;
	int fd;
	
	pgpAssert (file && pnfile && mode);
	pgpMemFree(tmp);
	for ( ; ; ) {
#if MACINTOSH
		if ((stdFile = fopen(nfile, mode)) != NULL)
			break;
#else
		if ((fd = open(nfile, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0)
			break;
#endif
		if ((*nfilep)++ > 'z')
			return NULL;
	}
#if defined(_WIN32) || defined(MSDOS)
	_setmode (fd, _O_BINARY);
#endif
	*pnfile = nfile;
#if !MACINTOSH
	stdFile = fdopen (fd, mode);
#endif
	return stdFile;
}

/*
 * This writes out a RingSet which is a replacement for a RingFile
 * which is open.  We do it carefully since on some OS's we can't
 * rename open files.  This closes the RingFile and the RingSet
 * in the process, so the RingSet must be one for
 * which this is legal (not from ringFileSet for example).
 * Renames existing file.ext to file.bak.
 */
static int
mainWriteoutSet (char const *file, int flags, struct RingSet *set,
		 struct RingFile *oldrfile, PgpVersion version,
		 struct PgpFile *pfp)
{
	struct PgpFile *pfp2;
	char *tmp, *tmp2, *bak;
	FILE *fp2;
	int error = 0;

	tmp = fileNameContract (file);
	bak = fileNameExtend (tmp, ".bak");
	pgpMemFree(tmp);
	fp2 = tmpkeyfile (file, &tmp2, "wb");
	if (!fp2) {
		pgpMemFree(bak);
		return PGPERR_NO_FILE;
	}
	/* Write out new data to tmp2 file */
	pfp2 = pgpFileWriteOpen (fp2, NULL);
	error = ringSetWrite (set, pfp2, NULL,
			      version, flags);
	pgpFileClose(pfp2);	/* closes fp2 */
	ringSetDestroy(set);
	if (error) {
		unlink (tmp2);
		pgpMemFree(tmp2);
		pgpMemFree(bak);
		return error;
	}
	if (ringFileClose (oldrfile) != 0) {
		/* Have a problem here, it should have closed */
		/* Probably due to a pesky RingSet leak! */
		/* Actually happens a lot with pgp3, it doesn't care */
/*		fprintf (stderr, "Warning: %s didn't close!\n", file); */
	}
	/* Close the PgpFile regardless */
	if (pfp)
		pgpFileClose (pfp);
	/* Now rename the files */
	unlink (bak);
	rename (file, bak);
	unlink (file);
	if (rename (tmp2, file) < 0) {
		fprintf (stderr, "Failed to rename %s to %s\n", tmp2, file);
		perror ("rename");
	}
	pgpMemFree (tmp2);
	pgpMemFree (bak);
	return 0;
}

static int
updateRingfile (struct OpenFile *openfile, int update, int force,
		int newvers)
{
        int error;
	int rc;
	PgpVersion version = PGPVERSION_3;

	/* If caller has not asked for the rings to be written out as
	   a new version, get the original version of the ringfile.
	   Otherwise, we writeout in PGP 3 format with additional
	   trust bytes (if PGPTRUSTMODEL>0) */

	if (!newvers && openfile->ringfile)
	        version = ringFileVersion (openfile->ringfile);

	if (openfile->newset) {
		/* Have a replacement for the RingFile */
		error = mainWriteoutSet (openfile->filename,
			    openfile->flags, openfile->newset,
			    openfile->ringfile, version,
  			    openfile->pfp);
		return error;
	}

	/*
	 * Here we are just closing the same as the ringFile we
	 * opened.  See if it needs to be written.
	 */
        if (update && openfile->trusted &&
	    (ringFileIsDirty (openfile->ringfile) ||
	     ringFileIsTrustChanged (openfile->ringfile))) {
	       return mainWriteoutSet (openfile->filename,
			openfile->flags,
		        ringSetCopy (ringFileSet (openfile->ringfile)),
			openfile->ringfile, version, openfile->pfp);
	}

	/* Close if we can, or if the force flag is set.  If file cannot
	   be closed (because it is the last 'home' of one or more objects),
	   mark it as pending by setting the filename to zero length, and
	   setting the trust flag to untrusted.  This will ensure no attempt
	   will be made to rewrite it later. */

	if ((rc = ringFileClose (openfile->ringfile)) == 0)
	        pgpFileClose (openfile->pfp);
	else {
/*
		fprintf (stderr, "Error, file %s did not close!\n",
			openfile->filename);
 */
		if (force) {
		        pgpFileClose (openfile->pfp);
		} else {
		        openfile->filename[0] = '\0';
			openfile->trusted = 0;
		}
	}
	return rc;
}

int
mainRingNewSet (char const *filename, int flags, struct RingSet *set)
{
        struct OpenFile *openfile;

	openfile = openfiles.next;
	while (openfile != NULL &&
	       strcmp (openfile->filename, filename) != 0) {
		   openfile = openfile->next;
	}
	if (!openfile) {
		openfile = (struct OpenFile *)pgpMemAlloc (
				sizeof (struct OpenFile) + strlen (filename));
		if (!openfile)
			return PGPERR_NOMEM;
		strcpy (openfile->filename, filename);
		openfile->ringfile = NULL;
		openfile->pfp = NULL;
		openfile->trusted = 0;
		openfile->newset = NULL;
		openfile->next = openfiles.next;
		openfiles.next = openfile;
	}
	if (openfile->newset) {
		ringSetDestroy(openfile->newset);
	}
	openfile->newset = set;
	openfile->flags = flags;
	return 0;
}


/* Close all ringfiles on the linked list, updating them if requested
   and required. */

void
mainCloseKeyrings (int update, int newvers)
{
        struct OpenFile *openfile;
	int error;

	openfile = openfiles.next;
	while (openfile) {
	        error = updateRingfile (openfile, update, 1, newvers);
		if (error < 0) {
			if (error == -1) {
				/*
				 * This is the unable to close keyring error,
				 * which is all too common and so we won't
				 * print anything.
				 */
			} else {
			    PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, error);
			}
		}
		openfiles.next = openfile->next;
		pgpMemFree (openfile);
		openfile = openfiles.next;
	}
}
			 				
			

struct RingSet *
mainOpenRingfile (struct PgpEnv const *env, struct RingPool *ringpool,
		  char const *filename, char const *ringtype, int trusted)
{
	struct PgpFile *pfp;
	struct RingFile *ring;
	FILE *fp;
	struct OpenFile *openfile;
	int error = 0;
	int verbose = pgpenvGetInt (env, PGPENV_VERBOSE, NULL, NULL);

	/* Check if file already open */

	for (openfile = openfiles.next; openfile; openfile = openfile->next) {
	        if (strcmp (filename, openfile->filename) == 0)
		  return (ringSetCopy (ringFileSet (openfile->ringfile)));
	}

	fp = fopen (filename, "rb");
	pfp = pgpFileReadOpen (fp, NULL, NULL);  /* chain errors */
	if (pfp) {
		if (verbose > PGPERR_VERBOSE_1)
			fprintf (stderr, "Reading %s keyring \"%s\"\n",
				 ringtype, filename);
		ring = ringFileOpen (ringpool, pfp, trusted, &error);
		if (error && verbose > PGPERR_VERBOSE_0) {
		    StatusOutput(TRUE, "GENERIC_KEYRING_ERROR", filename);
		    PGPErrCodeOutput(TRUE, LEVEL_CRITICAL, error);
		}
		if (ring) {
		        if (!(openfile = (struct OpenFile *)pgpMemAlloc (
			        sizeof (struct OpenFile) + strlen (filename))))
			        return NULL;
			openfile->ringfile = ring;
			openfile->pfp = pfp;
			openfile->trusted = trusted;
			openfile->flags = 0;
			openfile->newset = NULL;
			if (strcmp (ringtype, "public") == 0 && trusted)
			          openfile->flags = PGP_WRITETRUST_PUB;
			strcpy (openfile->filename, filename);
			openfile->next = openfiles.next;
			openfiles.next = openfile;
			return ringSetCopy (ringFileSet (ring));
		}
	} else if (verbose > PGPERR_VERBOSE_0) {
		fprintf (stderr, "Cannot open %s keyring \"%s\"\n",
			 ringtype, filename);
	}
	
	return NULL;
}

/*
 * Open up the keyrings, as given by the environment, and then
 * set the UI Argument to the new ringpool.
 */
int
mainOpenKeyrings (struct PgpEnv const *env, struct RingPool *ringpool,
		  int trusted_only, struct RingSet **ringsetp)
{
	struct RingSet *ringset = NULL, *tempset = NULL;
	char const *filename, *path, *temppath;
	char *dir, *fn;

	if (!ringpool || !ringsetp)
	  return -1;

	/* Open Secret Keyring */
	filename = pgpenvGetString (env, PGPENV_SECRING, NULL, NULL);
	if (filename && *filename)
		ringset = mainOpenRingfile (env, ringpool, filename,
					    "secret", 0);

	/* Open Public Keyring */
	filename = pgpenvGetString (env, PGPENV_PUBRING, NULL, NULL);
	if (filename && *filename)
		tempset = mainOpenRingfile (env, ringpool, filename,
					    "public", 1);
	if (tempset) {
		if (ringset) {
			struct RingSet *tset;
			tset = ringSetUnion (ringset, tempset);
			ringSetDestroy(tempset);
			ringSetDestroy(ringset);
			ringset = tset;
		} else
			ringset = tempset;
	}

	/* Now, open all the keyrings in my PGPPATH and PGPUPATH */
	path = pgpenvGetString (env, PGPENV_PGPPATH, NULL, NULL);
	pgpAssert (path);

	while ((dir = fileNameNextDirectory (path, &temppath)) != NULL) {
		tempset = NULL;	/* Don't re-open the same pubring */
		fn = fileNameBuild (dir, "pubring.pkr", NULL);
		if (strcmp (fn, filename) != 0) {
			FILE *fp = fopen (fn, "rb");
			if (fp) {
				fclose(fp);
				tempset = mainOpenRingfile (env, ringpool, fn,
							    "public", 1);
			}
		}
		pgpMemFree (fn);
		if (tempset) {
			if (ringset) {
				struct RingSet *tset;
				tset = ringSetUnion (ringset, tempset);
				ringSetDestroy(tempset);
				ringSetDestroy(ringset);
				ringset = tset;
			} else
				ringset = tempset;
		}
		path = temppath;
		if (!path)
			break;
	}

	path = pgpenvGetString (env, PGPENV_UPATH, NULL, NULL);
	if (path && !trusted_only)
		while ((dir = fileNameNextDirectory (path, &temppath)) !=
		       NULL) {
			FILE *fp;
			tempset = NULL;
			fn = fileNameBuild (dir, "pubring.pkr", NULL);
			fp = fopen (fn, "rb");
			if (fp) {
				fclose(fp);
				tempset = mainOpenRingfile (env, ringpool, fn,
						    "untrusted public", 0);
			}
			pgpMemFree (fn);
			if (tempset) {
				if (ringset) {
					struct RingSet *tset;
					tset = ringSetUnion (ringset, tempset);
					ringSetDestroy(tempset);
					ringSetDestroy(ringset);
					ringset = tset;
				} else
					ringset = tempset;
			}
			path = temppath;
			if (!path)
				break;
		}
	
	*ringsetp = ringset;

	return 0;
}
