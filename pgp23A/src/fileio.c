/*	fileio.c  - I/O routines for PGP.
	PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

	(c) Copyright 1990-1992 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use
	of this software, even if the damage results from defects in this
	software.  No warranty is expressed or implied.

	All the source code Philip Zimmermann wrote for PGP is available for
	free under the "Copyleft" General Public License from the Free
	Software Foundation.  A copy of that license agreement is included in
	the source release package of PGP.  Code developed by others for PGP
	is also freely available.  Other code that has been incorporated into
	PGP from other sources was either originally published in the public
	domain or was used with permission from the various authors.  See the
	PGP User's Guide for more complete information about licensing,
	patent restrictions on certain algorithms, trademarks, copyrights,
	and export controls.  

	Modified 16 Apr 92 - HAJK
	Mods for support of VAX/VMS file system

	Modified 17 Nov 92 - HAJK
	Change to temp file stuff for VMS.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _BSD
#include <sys/param.h>
#endif
extern int errno;
#endif /* UNIX */
#ifdef VMS
#include <file.h>
#endif
#include "random.h"
#include "mpilib.h"
#include "mpiio.h"
#include "fileio.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
#include "charset.h"
#include "system.h"
#if defined(MSDOS) || defined(OS2)
#include <io.h>
#include <fcntl.h>
#endif

#if (defined(VMS) || defined(UNIX)) && !defined(F_OK)
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4
#endif /* !F_OK */

/* 1st character of temporary file extension */
#define	TMP_EXT	'$'		/* extensions are '.$##' */

/* The PGPPATH environment variable */

static char PGPPATH[] = "PGPPATH";

/* Disk buffers, used here and in crypto.c */
byte textbuf[DISKBUFSIZE];
static byte textbuf2[2*DISKBUFSIZE];

boolean file_exists(char *filename)
/*	Returns TRUE iff file exists. */
{
#if defined(UNIX) || defined(VMS)
	return(access(filename, F_OK) == 0);
#else
	FILE *f;
	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(filename,FOPRBIN)) == NULL)
		return(FALSE);
	fclose(f);
	return(TRUE);
#endif
}	/* file_exists */

static boolean is_regular_file(char *filename)
{
#ifdef S_ISREG
	struct stat st;
	return(stat(filename, &st) != -1 && S_ISREG(st.st_mode));
#else
	return TRUE;
#endif
}

static int wipeout(FILE *f)
{	/*	Completely overwrite and erase file, so that no sensitive
		information is left on the disk.
		NOTE:  File MUST be open for read/write.
	*/

	long flength;
	int count = 0;

	fseek(f, 0L, SEEK_END);
	flength = ftell(f);
	rewind(f);

	fill0(textbuf, sizeof(textbuf));
	while (flength > 0L)
	{	/* write zeros to the whole file... */
		if (flength < (word32) DISKBUFSIZE)
			count = (int)flength;
		else
			count = DISKBUFSIZE;
		fwrite(textbuf,1,count,f);
		flength -= count;
	}
	rewind(f);	/* maybe this isn't necessary */
	return(0);	/* normal return */
}	/* wipeout */


int wipefile(char *filename)
{	/*	Completely overwrite and erase file, so that no sensitive
		information is left on the disk.
	*/
	FILE *f;
	/* open file f for read/write, in binary (not text) mode...*/
	if ((f = fopen(filename,FOPRWBIN)) == NULL)
		return(-1);	/* error - file can't be opened */
	wipeout(f);
	fclose(f);
	return(0);	/* normal return */
}	/* wipefile */

char	*file_tail (char *filename)
/*	Returns the after-slash part of filename.  Also skips backslashes,
 *  colons, and right brackets.	*/
{
	char *bslashPos = strrchr(filename, '\\');/* Find last bslash in filename */
	char *slashPos = strrchr(filename, '/');/* Find last slash in filename */
	char *colonPos = strrchr(filename, ':');
	char *rbrakPos = strrchr(filename, ']');

	if (!slashPos  ||  bslashPos > slashPos)
		slashPos = bslashPos;
	if (!slashPos  ||  colonPos > slashPos)
		slashPos = colonPos;
	if (!slashPos  ||  rbrakPos > slashPos)
		slashPos = rbrakPos;
	return( slashPos?(slashPos+1):filename );
}
/* Define BSLASH for machines that use backslash, FSLASH for machines
 * that use forward slash as separators.
 */

#ifdef MSDOS
#define	BSLASH
#endif
#ifdef ATARI
#define BSLASH
#endif
#ifdef UNIX
#define FSLASH
#define MULTIPLE_DOTS
#endif
#ifdef AMIGA
#define FSLASH
#define MULTIPLE_DOTS
#endif

boolean has_extension(char *filename, char *extension)
{	/* return TRUE if extension matches the end of filename */
	int lf = strlen(filename);
	int lx = strlen(extension);

	if (lf <= lx)
		return(FALSE);
	return(!strcmp(filename + lf - lx, extension));
}

boolean is_tempfile(char *path)
{	/* return TRUE if path is a filename created by tempfile() */
	char *p;
	
	return((p = strrchr(path, '.')) != NULL &&
			p[1] == TMP_EXT && strlen(p) == 4);
}

boolean no_extension(char *filename)
/*	Returns TRUE if user left off file extension, allowing default. */
{
#ifdef MULTIPLE_DOTS	/* filename can have more than one dot */
	if (has_extension(filename, ASC_EXTENSION) ||
		has_extension(filename, PGP_EXTENSION) ||
		has_extension(filename, SIG_EXTENSION) ||
		is_tempfile(filename))
		return(FALSE);
	else
		return(TRUE);
#else
#ifdef BSLASH
	char *slashPos = strrchr(filename, '\\');	/* Find last slash in filename */

	/* Look for the filename after the last slash if there is one */
	return(strchr((slashPos != NULL ) ? slashPos : filename, '.') == NULL);
#else
#ifdef FSLASH
	char *slashPos = strrchr(filename, '/');	/* Find last slash in filename */

	/* Look for the filename after the last slash if there is one */
	return(strchr((slashPos != NULL ) ? slashPos : filename, '.') == NULL);
#else
#ifdef VMS
	char *slashPos = strrchr(filename,']');		/* Locate end of directory spec */

	/* Look for last period in filename */
	return(strrchr((slashPos != NULL) ? slashPos : filename, '.') == NULL );
#else
	return( (strrchr(filename,'.')==NULL) ? TRUE : FALSE );
#endif /* VMS */
#endif /* FSLASH */
#endif /* BSLASH */
#endif /* MULTIPLE_DOTS */
}	/* no_extension */


void drop_extension(char *filename)
{	/* deletes trailing ".xxx" file extension after the period. */
	if (!no_extension(filename))
		*strrchr(filename,'.') = '\0';
}	/* drop_extension */


void default_extension(char *filename, char *extension)
{	/* append filename extension if there isn't one already. */
	if (no_extension(filename))
		strcat(filename,extension);
}	/* default_extension */

#ifndef MAX_NAMELEN
#if defined(AMIGA) || defined(NeXT) || (defined(BSD) && BSD > 41)
#define	MAX_NAMELEN	255
#else
#include <limits.h>
#endif
#endif

static void truncate_name(char *path, int ext_len)
{	/* truncate the filename so that an extension can be tacked on. */
#ifdef UNIX		/* for other systems this is a no-op */
	char *p;
#ifdef MAX_NAMELEN	/* overrides the use of pathconf() */
	int namemax = MAX_NAMELEN;
#else
	int namemax;
#ifdef _PC_NAME_MAX
	char dir[MAX_PATH];

	strcpy(dir, path);
	if ((p = strrchr(dir, '/')) == NULL)
		strcpy(dir, ".");
	else
	{	if (p == dir)
			++p;
		*p = '\0';
	}
	if ((namemax = pathconf(dir, _PC_NAME_MAX)) <= ext_len)
		return;
#else
#ifdef NAME_MAX
	namemax = NAME_MAX;
#else
	namemax = 14;
#endif /* NAME_MAX */
#endif /* _PC_NAME_MAX */
#endif /* MAX_NAMELEN */

	if ((p = strrchr(path, '/')) == NULL)
		p = path;
	else
		++p;
	if (strlen(p) > namemax - ext_len)
	{
		if (verbose)
			fprintf(pgpout, "Truncating filename '%s' ", path);
		p[namemax - ext_len] = '\0';
		if (verbose)
			fprintf(pgpout, "to '%s'\n", path);
	}
#endif /* UNIX */
}

void force_extension(char *filename, char *extension)
{	/* change the filename extension. */
	drop_extension(filename);	/* out with the old */
	truncate_name(filename, strlen(extension));
	strcat(filename,extension);	/* in with the new */
}	/* force_extension */


boolean getyesno(char default_answer)
/*	Get yes/no answer from user, returns TRUE for yes, FALSE for no. 
	First the translations are checked, if they don't match 'y' and 'n'
	are tried.
*/
{	char buf[8];
	static char yes[8], no[8];

	if (yes[0] == '\0')
	{	strncpy(yes, PSTR("y"), 7);
		strncpy(no, PSTR("n"), 7);
	}
	if (!batchmode) /* return default answer in batchmode */
	{	flush_input();
		getstring(buf,6,TRUE);	/* echo keyboard input */
		strlwr(buf);
		if (!strncmp(buf, no, strlen(no)))
			return(FALSE);
		if (!strncmp(buf, yes, strlen(yes)))
			return(TRUE);
		if (buf[0] == 'n')
			return(FALSE);
		if (buf[0] == 'y')
			return(TRUE);
	}
	return(default_answer == 'y' ? TRUE : FALSE);
}	/* getyesno */



char *maybe_force_extension(char *filename, char *extension)
{	/* if user consents to it, change the filename extension. */
	static char newname[MAX_PATH];
	if (!has_extension(filename,extension))
	{	strcpy(newname,filename);
		force_extension(newname,extension);
		if (!file_exists(newname))
		{	fprintf(pgpout,PSTR("\nShould '%s' be renamed to '%s' [Y/n]? "),
				filename,newname);
			if (getyesno('y'))
				return(newname);
		}
	}
	return(NULL);
}	/* maybe_force_extension */

char *buildfilename(char *result, char *fname)
/*	Builds a filename with a complete path specifier from the environmental
	variable PGPPATH.
*/
{	char *s = getenv(PGPPATH);

	if ( s==NULL || strlen(s) > 50) /* undefined, or too long to use */
		s="";
	strcpy(result,s);
	if (strlen(result) != 0)
#ifdef BSLASH
		if (result[strlen(result)-1] != '\\')
			strcat(result,"\\");
#else
#ifdef FSLASH
		if (result[strlen(result)-1] != '/')
			strcat(result,"/");
#else
#ifdef VMS
		if (result[strlen(result)-1] != ']')
			strcat(result,"]");
#endif
#endif
#endif /* Various OS-specific defines */
	strcat(result,fname);
	return(result);
}	/* buildfilename */


void file_to_canon(char *filename)
{	/* Convert filename to canonical form, with slashes as separators */
#ifdef BSLASH
	while (*filename) {
		if (*filename == '\\')
			*filename = '/';
		++filename;
	}
#endif
}


int write_error(FILE *f)
{
	fflush(f);
	if (ferror(f))
	{
#ifdef ENOSPC
		if (errno == ENOSPC)
			fprintf(pgpout, PSTR("\nDisk full.\n"));
		else
#endif
			fprintf(pgpout, PSTR("\nFile write error.\n"));
		return -1;
	}
	return 0;
}

int copyfile(FILE *f, FILE *g, word32 longcount)
{	/* copy file f to file g, for longcount bytes */
	int count, status = 0;
	do	/* read and write the whole file... */
	{
		if (longcount < (word32) DISKBUFSIZE)
			count = (int)longcount;
		else
			count = DISKBUFSIZE;
		count = fread(textbuf,1,count,f);
		if (count>0)
		{
			if (CONVERSION != NO_CONV)
			{       int i;
				for (i = 0; i < count; i++)
					textbuf[i] = (CONVERSION == EXT_CONV) ?
						     EXT_C(textbuf[i]) :
						     INT_C(textbuf[i]);
			}
			if (fwrite(textbuf,1,count,g) != count )
			{   /* Problem: return error value */
				status = -1;
				break;
			}
			longcount -= count;
		}
		/* if text block was short, exit loop */
	} while (count==DISKBUFSIZE);
	burn(textbuf);	/* burn sensitive data on stack */
	return(status);
}	/* copyfile */

int copyfilepos (FILE *f, FILE *g, word32 longcount, word32 fpos)
/* Like copyfile, but takes a position for file f.  Returns with
 * f and g pointing just past the copied data.
 */
{
	fseek (f, fpos, SEEK_SET);
	return copyfile (f, g, longcount);
}


int copyfile_to_canon (FILE *f, FILE *g, word32 longcount)
{	/* copy file f to file g, for longcount bytes.  Convert to
	   canonical form as we go.  f is open in text mode.  Canonical
	   form uses crlf's as line separators. */
	int count, status = 0;
	byte c, *tb1, *tb2;
	int i, nbytes;
	int nspaces = 0;
	do	/* read and write the whole file... */
	{
		if (longcount < (word32) DISKBUFSIZE)
			count = (int)longcount;
		else
			count = DISKBUFSIZE;
		count = fread(textbuf,1,count,f);
		if (count>0)
		{	/* Convert by adding CR before LF */
			tb1 = textbuf;
			tb2 = textbuf2;
			for (i=0; i<count; ++i)
			{       switch (CONVERSION) {
				case EXT_CONV:
				    c = EXT_C(*tb1++);
				    break;
				case INT_CONV:
				    c = INT_C(*tb1++);
				    break;
				default:
				    c = *tb1++;
				}
				if (strip_spaces)
				{
					if (c == ' ')	/* Don't output spaces yet */
						nspaces += 1;
					else
					{	if (c == '\n')
						{	*tb2++ = '\r';
							nspaces = 0;	/* Delete trailing spaces */
						}
						if (nspaces)	/* Put out spaces here */
						{	do
								*tb2++ = ' ';
							while (--nspaces);
						}
						*tb2++ = c;
					}
				}
				else
				{	if (c == '\n')
						*tb2++ = '\r';
					*tb2++ = c;
				}
			}
			nbytes = tb2 - textbuf2;
			if (fwrite(textbuf2,1,nbytes,g) != nbytes )
			{   /* Problem: return error value */
				status = -1;
				break;
			}
			longcount -= count;
		}
		/* if text block was short, exit loop */
	} while (count==DISKBUFSIZE);
	burn(textbuf);	/* burn sensitive data on stack */
	burn(textbuf2);
	return(status);
}	/* copyfile_to_canon */


int copyfile_from_canon (FILE *f, FILE *g, word32 longcount)
{	/* copy file f to file g, for longcount bytes.  Convert from
	   canonical to local form as we go.  g is open in text mode.  Canonical
	   form uses crlf's as line separators. */
	int count, status = 0;
	byte c, *tb1, *tb2;
	int i, nbytes;
	do	/* read and write the whole file... */
	{
		if (longcount < (word32) DISKBUFSIZE)
			count = (int)longcount;
		else
			count = DISKBUFSIZE;
		count = fread(textbuf,1,count,f);
		if (count>0)
		{	/* Convert by removing CR's */
			tb1 = textbuf;
			tb2 = textbuf2;
			for (i=0; i<count; ++i)
			{       switch (CONVERSION) {
				case EXT_CONV:
				    c = EXT_C(*tb1++);
				    break;
				case INT_CONV:
				    c = INT_C(*tb1++);
				    break;
				default:
				    c = *tb1++;
				}
				if (c != '\r')
					*tb2++ = c;
			}
			nbytes = tb2 - textbuf2;
			if (fwrite(textbuf2,1,nbytes,g) != nbytes )
			{   /* Problem: return error value */
				status = -1;
				break;
			}
			longcount -= count;
		}
		/* if text block was short, exit loop */
	} while (count==DISKBUFSIZE);
	burn(textbuf);	/* burn sensitive data on stack */
	burn(textbuf2);
	return(status);
}	/* copyfile_from_canon */


int copyfiles_by_name(char *srcFile, char *destFile)
/*	Copy srcFile to destFile  */
{
	FILE *f, *g;
	int status = 0;
	long fileLength;

	if (((f=fopen(srcFile,FOPRBIN)) == NULL) ||
		((g=fopen(destFile,FOPWBIN)) == NULL))
		/* Can't open files */
		return(-1);

	/* Get file length and copy it */
	fseek(f,0L,SEEK_END);
	fileLength = ftell(f);
	rewind(f);
	status = copyfile(f,g,fileLength);
	fclose(f);
	if (write_error(g))
		status = -1;
	fclose(g);
	return(status);
}	/* copyfiles_by_name */


int make_canonical(char *srcFile, char *destFile)
/*	Copy srcFile to destFile, converting to canonical text form  */
{
	FILE *f, *g;
	int status = 0;
	long fileLength;

	if (((f=fopen(srcFile,FOPRTXT)) == NULL) ||
		((g=fopen(destFile,FOPWBIN)) == NULL))
		/* Can't open files */
		return(-1);

	/* Get file length and copy it */
	fseek(f,0L,SEEK_END);
	fileLength = ftell(f);
	rewind(f);
	CONVERSION = INT_CONV;
	status = copyfile_to_canon(f,g,fileLength);
	CONVERSION = NO_CONV;
	fclose(f);
	if (write_error(g))
		status = -1;
	fclose(g);
	return(status);
}	/* make_canonical */


int rename2(char *srcFile, char *destFile)
/*	Like rename() but will try to copy the file if the rename fails.
	This is because under OS's with multiple physical volumes if the
	source and destination are on different volumes the rename will fail */
{
	FILE *f, *g;
	int status = 0;
	long fileLength;

#ifdef VMS
	if (rename(srcFile,destFile) != 0)
#else
	if (rename(srcFile,destFile) == -1)
#endif
	{	/* Rename failed, try a copy */
		if (((f=fopen(srcFile,FOPRBIN)) == NULL) ||
			((g=fopen(destFile,FOPWBIN)) == NULL))
			/* Can't open files */
			return(-1);

		/* Get file length and copy it */
		fseek(f,0L,SEEK_END);
		fileLength = ftell(f);
		rewind(f);
		status = copyfile(f,g,fileLength);
		if (write_error(g))
			status = -1;

		/* Zap source file if the copy went OK, otherwise zap the (possibly
		   incomplete) destination file */
		if (status >= 0)
		{	wipeout(f);		/* Zap source file */
			fclose(f);
			remove(srcFile);
			fclose(g);
		}
		else
		{	if (is_regular_file(destFile))
			{	wipeout(g);		/* Zap destination file */
				fclose(g);
				remove(destFile);
			} else
				fclose(g);
			fclose(f);
		}
	}
	return(status);
}


int readPhantomInput(char *filename)
/* read the data from stdin to the phantom input file */
{	FILE *outFilePtr;
	byte buffer[ 512 ];
	int bytesRead, status = 0;

	if (verbose)
		fprintf(pgpout, "writing stdin to file %s\n", filename);
	if ((outFilePtr = fopen(filename,FOPWBIN)) == NULL)
		return(-1);

#if defined(MSDOS) || defined(OS2)
	/* Under DOS must set input stream to binary mode to avoid data mangling */
	setmode(fileno(stdin), O_BINARY);
#endif /* MSDOS || OS2 */
	while ((bytesRead = fread (buffer, 1, 512, stdin)) > 0)
		if (fwrite (buffer, 1, bytesRead, outFilePtr) != bytesRead)
		{	status = -1;
			break;
		}
	if (write_error(outFilePtr))
		status = -1;
	fclose (outFilePtr);
#if defined(MSDOS) || defined(OS2)
	setmode(fileno(stdin), O_TEXT);	/* Reset stream */
#endif /* MSDOS || OS2 */
	return(status);
}


int writePhantomOutput(char *filename)
/* write the data from the phantom output file to stdout */
{	FILE *outFilePtr;
	byte buffer[ 512 ];
	int bytesRead, status = 0;

	if (verbose)
		fprintf(pgpout, "writing file %s to stdout\n", filename);
	/* this can't fail since we just created the file */
	outFilePtr = fopen(filename,FOPRBIN);

#if defined(MSDOS) || defined(OS2)
	setmode(fileno(stdout), O_BINARY);
#endif /* MSDOS || OS2 */
	while ((bytesRead = fread (buffer, 1, 512, outFilePtr)) > 0)
		if (fwrite (buffer, 1, bytesRead, stdout) != bytesRead)
		{	status = -1;
			break;
		}
	fclose (outFilePtr);
	fflush(stdout);
	if (ferror(stdout))
	{	status = -1;
		fprintf(pgpout, PSTR("\007Write error on stdout.\n"));
	}
#if defined(MSDOS) || defined(OS2)
	setmode(fileno(stdout), O_TEXT);
#endif /* MSDOS || OS2 */

	return(status);
}

/* Return the size from the current position of file f to the end */
word32 fsize (FILE *f)
{
	long fpos = ftell (f);
	long fpos2;

	fseek (f, 0L, SEEK_END);
	fpos2 = ftell (f);
	fseek (f, fpos, SEEK_SET);
	return (word32)(fpos2 - fpos);
}

/* Return TRUE if file filename looks like a pure text file */
int is_text_file (char *filename)
{
	FILE	*f = fopen(filename,"r");	/* FOPRBIN gives problem with VMS */
	int		i, n, bit8 = 0;
	unsigned char buf[512];
	unsigned char *bufptr = buf;
	unsigned char c;

	if (!f)
		return(FALSE);	/* error opening it, so not a text file */
	i = n = fread (buf, 1, sizeof(buf), f);
	fclose(f);
	if (n <= 0)
		return(FALSE);	/* empty file or error, not a text file */
	if (compressSignature(buf) >= 0)
		return(FALSE);
	while (i--)
	{	c = *bufptr++;
		if (c & 0x80)
			++bit8;
		else /* allow BEL BS HT LF VT FF CR EOF control characters */
			if (c < '\007' || (c > '\r' && c < ' ' && c != '\032'))
				return(FALSE);	/* not a text file */
	}
	if (strcmp(language, "ru") == 0)
		return(TRUE);
	/* assume binary if more than 1/4 bytes have 8th bit set */
	return(bit8 < n / 4);
} /* is_text_file */

VOID *xmalloc(unsigned size)
{	VOID *p;
	if (size == 0)
		++size;
	if ((p = malloc(size)) == NULL)
	{	fprintf(stderr, PSTR("\n\007Out of memory.\n"));
		exitPGP(1);
	}
	return(p);
}

/*----------------------------------------------------------------------
 *	temporary file routines
 */


#define MAXTMPF 8

#define	TMP_INUSE	2

static struct
{	char path[MAX_PATH];
	int flags;
	int num;
} tmpf[MAXTMPF];

static char tmpdir[256];	/* temporary file directory */
static char outdir[256];	/* output directory */
static char tmpbasename[64] = "pgptemp";	/* basename for temporary files */


/*
 * set directory for temporary files.  path will be stored in
 * tmpdir[] with an appropriate trailing path separator.
 */
void settmpdir(char *path)
{
	char *p;

	if (path == NULL || *path == '\0')
	{	tmpdir[0] = '\0';
		return;
	}
	strcpy(tmpdir, path);
	p = tmpdir + strlen(tmpdir)-1;
	if (*p != '/' && *p != '\\' && *p != ']' && *p != ':')
	{	/* append path separator, either / or \ */
		if ((p = strchr(tmpdir, '/')) == NULL &&
			(p = strchr(tmpdir, '\\')) == NULL)
			p = "/";	/* path did not contain / or \, use / */
		strncat(tmpdir, p, 1);
	}
}

/*
 * set output directory to avoid a file copy when temp file is renamed to
 * output file.  the argument filename must be a valid path for a file, not
 * a directory.
 */
void setoutdir(char *filename)
{
	char *p;

	if (filename == NULL)
	{	strcpy(outdir, tmpdir);
		return;
	}
	strcpy(outdir, filename);
	p = file_tail(outdir);
	strcpy(tmpbasename, p);
	*p = '\0';
	drop_extension(tmpbasename);
#if !defined(BSD42) && !defined(BSD43) && !defined(sun)
	/* we don't depend on pathconf here, if it returns an incorrect value
	 * for NAME_MAX (like Linux 0.97 with minix FS) finding a unique name
	 * for temp files can fail.
	 */
	tmpbasename[10] = '\0';	/* 14 char limit */
#endif
}

/*
 * return a unique temporary file name
 */
char *tempfile(int flags)
{
	int i, j;
	int num;
	int fd;
#ifndef UNIX
	FILE *fp;
#endif

	for (i = 0; i < MAXTMPF; ++i)
		if (tmpf[i].flags == 0)
			break;
	
	if (i == MAXTMPF)
	{	/* message only for debugging, no need for PSTR */
		fprintf(stderr, "\n\007Out of temporary files\n");
		return(NULL);
	}

again:
	num = 0;
	do {
		for (j = 0; j < MAXTMPF; ++j)
			if (tmpf[j].flags && tmpf[j].num == num)
				break;
		if (j < MAXTMPF)
			continue;	/* sequence number already in use */
		sprintf(tmpf[i].path, "%s%s.%c%02d",
			((flags & TMP_TMPDIR) && *tmpdir ? tmpdir : outdir),
			tmpbasename, TMP_EXT, num);
		if (!file_exists(tmpf[i].path))
			break;
	}
	while (++num < 100);

	if (num == 100)
	{	fprintf(pgpout, "\n\007tempfile: cannot find unique name\n");
		return(NULL);
	}

#if defined(UNIX) || defined(VMS)
	if ((fd = open(tmpf[i].path, O_EXCL|O_RDWR|O_CREAT, 0600)) != -1)
		close(fd);
#else
	if ((fp = fopen(tmpf[i].path, "w")) != NULL)
		fclose(fp);
	fd = (fp == NULL ? -1 : 0);
#endif

	if (fd == -1)
	{	if (!(flags & TMP_TMPDIR))
		{	flags |= TMP_TMPDIR;
			goto again;
		}
#ifdef UNIX
		else if (tmpdir[0] == '\0')
		{	strcpy(tmpdir, "/tmp/");
			goto again;
		}
#endif
	}
	if (fd == -1)
	{	fprintf(pgpout, PSTR("\n\007Cannot create temporary file '%s'\n"), tmpf[i].path);
		user_error();
	}
#ifdef VMS
	remove(tmpf[i].path);
#endif

	tmpf[i].num = num;
	tmpf[i].flags = flags | TMP_INUSE;
	if (verbose)
		fprintf(pgpout, "tempfile: created '%s'\n", tmpf[i].path);
	return(tmpf[i].path);
}	/* tempfile */

/*
 * remove temporary file, wipe if necessary.
 */
void rmtemp(char *name)
{
	int i;

	for (i = 0; i < MAXTMPF; ++i)
		if (tmpf[i].flags && strcmp(tmpf[i].path, name) == 0)
			break;

	if (i < MAXTMPF)
	{	if (strlen(name) > 3 && name[strlen(name)-3] == TMP_EXT)
		{	/* only remove file if name hasn't changed */
			if (verbose)
				fprintf(pgpout, "rmtemp: removing '%s'\n", name);
			if (tmpf[i].flags & TMP_WIPE)
				wipefile(name);
			if (!remove(name)) {
			    tmpf[i].flags = 0;
			} else if (verbose) {
			    fprintf(stderr,"\nrmtemp: Failed to remove %s",name);			    
			    perror ("\nError");
			}
		} else if (verbose)
			fprintf(pgpout, "rmtemp: not removing '%s'\n", name);
	}
}	/* rmtemp */

/*
 * make temporary file permanent, returns the new name.
 */
char *savetemp(char *name, char *newname)
{
	int i, overwrite;
	static char buf[MAX_PATH];

	if (strcmp(name, newname) == 0)
		return(name);

	for (i = 0; i < MAXTMPF; ++i)
		if (tmpf[i].flags && strcmp(tmpf[i].path, name) == 0)
			break;

	if (i < MAXTMPF)
	{	if (strlen(name) < 4 || name[strlen(name)-3] != TMP_EXT)
		{	if (verbose)
				fprintf(pgpout, "savetemp: not renaming '%s' to '%s'\n",
						name, newname);
			return(name);	/* return original file name */
		}
	}

	while (file_exists(newname))
	{
		if (batchmode && !force_flag)
		{	fprintf(pgpout,PSTR("\n\007Output file '%s' already exists.\n"),newname);
			return NULL;
		}
		if (is_regular_file(newname))
		{	
			if (force_flag)
			{	/* remove without asking */
				remove(newname);
				break;
			}
			fprintf(pgpout,PSTR("\n\007Output file '%s' already exists.  Overwrite (y/N)? "),
				newname);
			overwrite = getyesno('n');
		}
		else
		{	fprintf(pgpout,PSTR("\n\007Output file '%s' already exists.\n"),newname);
			if (force_flag)	/* never remove special file */
				return NULL;
			overwrite = FALSE;
		}

		if (!overwrite)
		{	fprintf(pgpout, PSTR("\nEnter new file name: "));
			getstring(buf, MAX_PATH - 1, TRUE);
			if (buf[0] == '\0')
				return(NULL);
			newname = buf;
		}
		else
			remove(newname);
	}
	if (verbose)
		fprintf(pgpout, "savetemp: renaming '%s' to '%s'\n", name, newname);
	if (rename2(name, newname) < 0)
	{	/* errorLvl = UNKNOWN_FILE_ERROR; */
		fprintf(pgpout, PSTR("Can't create output file '%s'\n"), newname);
		return(NULL);
	}
	if (i < MAXTMPF)
		tmpf[i].flags = 0;
	return(newname);
}	/* savetemp */

/*
 * like savetemp(), only make backup of destname if it exists
 */
int savetempbak(char *tmpname, char *destname)
{
	char bakpath[MAX_PATH];
#ifdef UNIX
	int mode = -1;
#endif

	if (is_tempfile(destname))
		remove(destname);
	else
	{	if (file_exists(destname))
		{
#ifdef UNIX
			struct stat st;
			if (stat(destname, &st) != -1)
				mode = st.st_mode & 07777;
#endif
			strcpy(bakpath, destname);
			force_extension(bakpath, BAK_EXTENSION);
			remove(bakpath);
#ifdef VMS
			if (rename(destname, bakpath) != 0)
#else
			if (rename(destname, bakpath) == -1)
#endif
				return(-1);
		}
	}
	if (savetemp(tmpname, destname) == NULL)
		return(-1);
#ifdef UNIX
	if (mode != -1)
		chmod(destname, mode);
#endif
	return(0);
}

/*
 * remove all temporary files and wipe them if necessary
 */
void cleanup_tmpf(void)
{
	int i;

	for (i = 0; i < MAXTMPF; ++i)
		if (tmpf[i].flags)
			rmtemp(tmpf[i].path);
}	/* cleanup_tmpf */
