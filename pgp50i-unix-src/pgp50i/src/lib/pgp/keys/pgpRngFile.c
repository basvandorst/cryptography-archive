/*
 * pgpRngFile.c -- File operations for keyrings.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngFile.c,v 1.2.2.1 1997/06/07 09:50:35 mhw Exp $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#ifdef VMS
#include <pgpAppFile.h>	/*	Does anybody know what's in here? */
#endif

#if defined(MSDOS) || defined(OS2)
#include <io.h>			/*	For access() */
#endif

#include "ring.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

/*
 * Munges "basename" in place to produce a temporary file name,
 * opens it, and returns the FILE *. Tries lots of three-letter
 * extensions before giving up. (Okay, I confess - I left that
 * while() condition in because it's hairy-looking.)
 * @@@ TODO: use open() and O_EXCL to ensure open is unique.
*/
static FILE *
fileTempNamed(char *basename)
{
		FILE *f = 0;
		char *p;
		int i;
		/* "circular" array of characters beginning and endinf wth '0' */
		static char const exts[] = "0123456789abcdefghijklmnopqrstuvwxyz0";

		/* Make temp files for stdout more legible */
		if (!basename[0] || FILENAMEDASH(basename))
				strcpy(basename, "tempfile");

		fileExtensionAdd(basename, ".000");
		p = basename + strlen(basename);		/*	Pointer to end of string */
		for (;;) {
				if (!fileExists(basename)) {
						f = fopen (basename, "w+b");
						if (f)
								break;
				}
				/*	Increment filename using exts array */
				i = 0;
				do {
						if (++i > 3)
								return 0;		/*	Failed */
				} while ((p[-i] = strchr(exts, p[-i])[1]) == '0');
		}
		return f;
}

/*
 * Rename a file to a backup file.
 */
static int
fileBackup(char const *name)
{
		int i;
		char bakname[MAXPATHLEN];

		if (!fileExists(name))
				return 0;		/*	Success */

		strcpy(bakname, name);
		fileExtensionAdd(bakname, ".bak");
		if (fileExists(bakname)) {
				i = remove(bakname);
				if (i < 0) {
						perror("Unable to remove backup file");
						return -PGPERR_RINGOUT;
				}
		}
		i = rename(name, bakname);
		if (i < 0) {
				perror("Unable to rename file to backup");
				return -PGPERR_RINGOUT;
		}
		return 0;
}

/*
 * Exceopt for ACTERR_ALLOC, all of these errors are of the
 * "can't happen" variety. That is, these errors occur when doing I/O
 * to a file that is already open. Returns appropriate error code < 0
 * if there was an error, 0 otherwise.
 */
int
ringPrintError(struct RingPool const *ring, FILE *f)
{
		char const *str, *str2;
		int code;

		switch (ring->erraction) {
		  case ACTERR_NONE:
				return 0;
		  case ACTERR_READ:
				str = USERTRANS("Error while reading");
				code = PGPERR_RINGIN;
				break;
		  case ACTERR_WRITE:
				str = USERTRANS("Error while writing");
				code = PGPERR_RINGOUT;
				break;
		  case ACTERR_SEEK:
				str = USERTRANS("Error while seeking");
				code = PGPERR_RINGIN;
				break;
		  case ACTERR_OPEN:
				str = USERTRANS("Error opening file");
				code = PGPERR_NOFILE;
				break;
		  case ACTERR_CLOSE:
				str = USERTRANS("Error closing file");
				code = PGPERR_RINGIN;
				break;
		  case ACTERR_FLUSH:
				str = USERTRANS("Error flushing file");
				code = PGPERR_RINGOUT;
				break;
		  case ACTERR_HUGE:
					str = USERTRANS("Error: keyring object is too large\n");
					code = PGPERR_VERSION;
					break;

		  case ACTERR_EOF:
				str = USERTRANS("Error: EOF encountered unexpectedly\n\
The file has been changed unexpectedly while PGP is using it.\n");
				code = PGPERR_BADFILE;
				break;
		 case ACTERR_BADPKTBYTE:
				str = USERTRANS("Error: Non-packet found where packet \
expected\n\
The file has been changed unexpectedly while PGP is using it.\n");
				code = PGPERR_BADFILE;
				break;
		  case ACTERR_WRONGPKTBYTE:
				str = USERTRANS("Error: Wrong packet encountered\n\
The file has been changed unexpectedly while PGP is using it.\n");
				code = PGPERR_BADFILE;
				break;
		  case ACTERR_WRONGLEN:
				str = USERTRANS("Error: Packet has wrong length\n\
The file has been changed unexpectedly while PGP is using it.\n");
				code = PGPERR_BADFILE;
				break;

			case ACTERR_ALLOC:
				fputs(userTrans("\aError: out of memory!\n"), f);
				code = PGPERR_NOMEM;
				return 1;
			default:
				str = USERTRANS("Error: Unknown keyring error %d \
(this is a bug in PGP)\n");
				code = PGPERR_INTERNAL;
		}

		putc('\a', f);
		fprintf(f, str, ring->erraction);
		putc('\n', f);

		/* Print all the details. */
		if (ring->errnum)
				fprintf(f, userTrans("System error \"%s\"\n"),
				strerror(ring->errnum));

		if (ring->errfile >= 0 && ring->errfile < 32) {
				str = ring->ring[ring->errfile].physname;
				str2 = ring->ring[ring->errfile].logname;
		} else {
				str = str2 = 0;
		}

		if (str) {
				fprintf(f, userTrans("In file \"%s\"\n"), str);
				if (str2 && strcmp(str, str2) != 0)
						fprintf(f, userTrans(
								"A temporary file associated with \"%s\"\n"),
							str2);
		} else if (str2) {
				fprintf(f, userTrans(
						"In a temporary file associated with \"%s\"\n"),
						str2);
		}
		if (ring->errpos != -(word32)1) {
				fprintf(f, userTrans("At file position %lu\n"),
				(unsigned long)ring->errpos);
		}

		return -code;
}

/*
 * Save the ring specified by "newbit" and get rid of "oldbit".
 * Saves "newbit" to a temporary file name associated with the
 * name of "oldbit" and returns the name in a user-supplied
 * "tmpname" buffer.
 */
int
ringSaveTemp(struct RingPool *ring, int oldbit, int newbit, int trustflags,
		char *tmpname, FILE *e)
{
		char const *name;
		FILE *f;
		int i;

		ring->ring[newbit].logname = ring->ring[oldbit].logname;

		name = ring->ring[oldbit].physname;
		if (!name)
				name = ring->ring[newbit].logname;
	
		strcpy(tmpname, name);

		f = fileTempNamed(tmpname);
		if (!f) {
				perror("Unable to create temporary file");
				return -PGPERR_RINGOUT;	/*	Unable to do the save */
		}
		ring->ring[newbit].physname = tmpname;
		ring->ring[newbit].file = f;

		i = ringPoolWriteVirt(ring, newbit, trustflags);
		if (i < 0) {
				ringPrintError(ring, e);
				fclose (f);
				remove(tmpname);
				ring->ring[newbit].physname = 0;
				ring->ring[newbit].file = 0;
				return i;
		}
		ringPoolClose(ring, (ringmask)1<<oldbit);

		return 0;
}

/*
 * Saves the ring specified by "newbit" in the file named by "oldbit",
 * shuffling the file names around. "oldbit" can be a virtual keyring
 * if desired.
 *
 * TODO: improve error handling a lot. At the moment, this doesn't
 * return any indication of what the status of oldbit and newbit
 * is, so if it fails, all you can do is shut down.
 */
int
ringSave(struct RingPool *ring, int oldbit, int newbit,
				int trustflags, FILE *e)
{
		char const *name;
		char tmpname[MAXPATHLEN];		/*	defined in <stdio.h> */
		FILE *f;
		int i;

		ring->ring[newbit].logname = ring->ring[oldbit].logname;
		name = ring->ring[oldbit].physname;

		if (ring->ring[oldbit].file == 0) {
				/* Old keyring doesn't exist, so save directly */
				if (!name)
						name = ring->ring[newbit].logname;
				if (FILENAMEDASH(name)) {
						f = stdout;
				} else {
						f = fopen (name, "w+b");
						if (!f) {
								perror("Unable to write to output file");
								return -PGPERR_RINGOUT;	/* No go */
						}
				}
				ring->ring[newbit].physname = name;
				ring->ring[newbit].file = f;
				i = ringPoolWriteVirt(ring, newbit, trustflags);
				if (i < 0) {
						ringPrintError(ring, e);
						fclose (f);
						remove(name);
						ring->ring[newbit].physname = 0;
						ring->ring[newbit].file = 0;
						return i;
				}
				ringPoolClose(ring, (ringmask)1<<oldbit);

				return 0;		/*	Success */
}

/* Old keyring exists, so save to a temp file and rename */

		i = ringSaveTemp(ring, oldbit, newbit, trustflags, tmpname, e);
		if (i < 0)
		return i;
			
		/* Now the temporary file shuffling */

		/* Move name to name.bak */
		i = fileBackup(name);
		if (i < 0)
				return i;

#ifndef MSDOS /* BRAIN DAMAGE ALERT BRAIN DAMAGE ALERT BRAIN DAMAGE ALERT */
		/*	Move tmpname to name */
		i = rename(tmpname, name);
		if (i >= 0) {
				ring->ring[newbit].physname = name;
				return 0;		/*	Success */
		}
#endif
		/*
		 * Some OSes can't rename an open file, so close, rename, and reopen.
		 *
		 * MS-DOS can't do it, and doesn't produce an error, either!!!
		 * Yes, really! You get a zero-length destination file, which
		 * is NOT what's desired. VERY VERY broken! I knew it was a
		 * brain-dead OS, but this really takes the cake. (You end up
		 * with errno=2, "Bad file number" on fclose(), but fwrite() and
		 * fflush() both succeed, even on an unbuffered stream.)
		 * Sheesh, is it too much to ask of a system that it either does
		 * it, or doesn't do it? You have to find a third choice?
		 */
		fclose (ring->ring[newbit].file);
		ring->ring[newbit].file = 0;
		i = rename(tmpname, name);
		if (i < 0) {
				perror("Can't rename new file");
				ring->ring[newbit].physname = (char const *)0;
				return -PGPERR_RINGOUT;
		}
		f = fopen (name, "rb");
		if (!f) {
				perror("Can't reopen new file");
				return -PGPERR_RINGOUT;
		}
		ring->ring[newbit].physname = name;
		ring->ring[newbit].file = f;

		return 0;		/* Success */
}
