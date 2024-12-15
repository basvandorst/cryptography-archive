/*____________________________________________________________________________
    fileio.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    I/O routines for PGP

    PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

    Modified 16 Apr 92 - HAJK
    Mods for support of VAX/VMS file system

    Modified 17 Nov 92 - HAJK
    Change to temp file stuff for VMS.

    $Id: fileio.c,v 1.19 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if PGP_UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef _BSD
#include <sys/param.h>
#endif
extern int errno;
#endif /* PGP_UNIX */
#ifdef VMS
#include <file.h>
#include <assert.h>
#endif

#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpDiskWiper.h"
#include "pgpClientErrors.h"

#include "config.h"
#include "usuals.h"
//#include "stubs.h"
#include "globals.h"

#include "fileio.h"
#include "language.h"
#include "pgp.h"

#if PGP_WIN32 || defined(MSDOS) || defined(OS2)
#include <io.h>
#include <fcntl.h>
#endif

#ifndef F_OK
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif /* !F_OK */

/*
 * DIRSEPS is a string of possible directory-separation characters
 * The first one is the preferred one, which goes in between
 * PGPPATH and the file name if PGPPATH is not terminated with a
 * directory separator.
 */

#if defined(MSDOS) || defined(__MSDOS__) || defined(OS2)
static char const DIRSEPS[] = "\\/:";
#if PGP_WIN32
#define MULTIPLE_DOTS
#endif
#define BSLASH

#elif defined(ATARI)
static char const DIRSEPS[] = "\\/:";
#define BSLASH

#elif defined(PGP_UNIX)
static char const DIRSEPS[] = "/";
#define MULTIPLE_DOTS

#elif defined(AMIGA)
static char const DIRSEPS[] = "/:";
#define MULTIPLE_DOTS

#elif defined(VMS)
static char const DIRSEPS[] = "]:"; /* Any more? */

#else
/* #error is not portable, this has the same effect */
#include "Unknown OS"
#endif

/* 1st character of temporary file extension */
#define TMP_EXT '$'  /* extensions are '.$##' */

/* The PGPPATH environment variable */

static char PGPPATH[] = "PGPPATH";

/* Disk buffers, used here and in crypto.c */
static PGPByte textbuf[DISKBUFSIZE];
static unsigned textbuf2[2 * DISKBUFSIZE / sizeof(unsigned)];

#if 0
/* Don't forget to change 'pgp60' whenever you update relVersion past 6.0 */
static char const *const manual_dirs[] =
{
#if defined(VMS)
    "$PGPPATH", "", "[pgp]", "[pgp60]", "[pgp600]",
    PGP_SYSTEM_DIR, "SYS$LOGIN:", "SYS$LOGIN:[pgp]",
    "SYS$LOGIN:[pgp60]", "SYS$LOGIN:[pgp600]", "[-]",
#elif defined(PGP_UNIX)
    "$PGPPATH", "", "pgp", "pgp60", "pgp600", PGP_SYSTEM_DIR,
    "$HOME/.pgp", "$HOME", "$HOME/pgp", "$HOME/pgp60", "..",
#elif defined(AMIGA)
    "$PGPPATH", "", "pgp", "pgp60", ":pgp", ":pgp60", ":pgp600",
    ":", "/",
#else /* MSDOS or ATARI */
    "$PGPPATH", "", "pgp", "pgp60", "\\pgp", "\\pgp60", "\\pgp600",
    "\\", "..", "c:\\pgp", "c:\\pgp60",
#endif
    0};
#endif

static struct pgpfileBones _fileBones;

void initFileBones(struct pgpmainBones *mainbPtr)
{
    struct pgpfileBones *filebPtr = &_fileBones;

    filebPtr->HLP_EXTENSION = ".hlp";
    filebPtr->PGP_EXTENSION = ".pgp";
    filebPtr->ASC_EXTENSION = ".asc";
    filebPtr->SIG_EXTENSION = ".sig";
    filebPtr->BAK_EXTENSION = ".bak";
    filebPtr->PKR_EXTENSION = ".pkr";
    filebPtr->SKR_EXTENSION = ".skr";

    filebPtr->HELP_FILENAME = "pgp.hlp";
    filebPtr->CONSOLE_FILENAME = "_CONSOLE";

    filebPtr->moreon=FALSE;
    filebPtr->piping=FALSE;
    filebPtr->savepgpout=NULL;

    filebPtr->envbPtr = mainbPtr->envbPtr;
    filebPtr->mainbPtr = mainbPtr;
    mainbPtr->filebPtr = filebPtr;
}

PGPBoolean fileExists(char *filename)
/* Returns TRUE iff file exists. */
{
    return access(filename, F_OK) == 0;
} /* file_exists */

static PGPBoolean isRegularFile(char *filename)
{
#ifdef S_ISREG
    struct stat st;
    return stat(filename, &st) != -1 && S_ISREG(st.st_mode);
#else
    return TRUE;
#endif
}

/*
 * This wipes a file using the improved wiping strategy. The purpose of
 * this is to make sure no sensitive information is left on the disk.
 *
 * Note that the file MUST be open for read/write.
 *
 * It may not work to eliminate everything from non-volatile storage
 * if the OS you're using does its own paging or swapping.  Then
 * it's an issue of how the OS's paging device is wiped, and you can
 * only hope that the space will be reused within a few seconds.
 *
 * Also, some file systems (in particular, the Logging File System
 * for Sprite) do not write new data in the same place as old data,
 * defeating this wiping entirely.  Fortunately, such systems
 * usually don't need a swap file, and for small temp files, they
 * do write-behind, so if you create and delete a file fast enough,
 * it never gets written to disk at all.
 */

static void wipeout( struct pgpfileBones *filebPtr, FILE *fp )
{
    PGPContextRef context = filebPtr->mainbPtr->pgpContext;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPError err = kPGPError_NoErr;
    PGPDiskWipeRef wipeRef = kPGPInvalidRef;
    PGPInt32 passes = kPGPNumPatterns;
    PGPInt32 buffer[256];
#if PGP_DEBUG
    PGPInt32 pri;
#endif /* PGP_DEBUG */

    err = PGPCreateDiskWiper( context, &wipeRef, passes);

    if( IsntPGPError(err) )
    {
        while( IsntPGPError( PGPGetDiskWipeBuffer(wipeRef, buffer) ) )
        {
#if HAVE_FTELLO
            PGPInt64 len;
#else
            PGPInt32 len;
#endif
            int i = 1;
#if PGP_DEBUG
            if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
                fprintf(filebPtr->pgpout, LANG("Pass #%d\r\n"), i++);
#endif /* PGP_DEBUG */
            /*
               Write pattern to disk here, repeating until end of the
               file. We should round the file size up to next page
               boundary to be safe. Also make sure to sync the file
               after writing each pattern so that they actually get
               written to disk and not just an IO buffer.
             */

            /* Get the file size */
            fseek(fp, 0L, SEEK_END);
#if HAVE_FTELLO
            len = ftello(fp);
                /* instead of wiping very long files you really should
                degauss the media, but we aim to please. */
#else
            len = ftell(fp);
#endif
            rewind(fp);

            while (len > 0) {
                /* Write it out - yes, we're ignoring errors */
                fwrite((char const *) buffer, sizeof(buffer), 1, fp);
                len -= sizeof(buffer);
            }
            fflush( fp );

            /* try to sync it to the disk.. */
#ifdef PGP_UNIX
            sync();
#endif
        }
        err = PGPDestroyDiskWiper(wipeRef);
        pgpAssertNoErr(err);
    }

    if(IsPGPError(err))
    {
        char buf[256];
        PGPGetClientErrorString( err,256, buf );
        fprintf( filebPtr->pgpout, LANG("Error!!!\r\n%s\r\n"), buf);
    }
}


/*
 * Completely overwrite and erase file, so that no sensitive
 * information is left on the disk.
 */
int wipefile(struct pgpfileBones *filebPtr, char *filename)
{
    FILE *fp;
#if PGP_DEBUG
    fprintf( filebPtr->pgpout, LANG("wiping file %s"),filename);
#endif
    /* open file f for read/write, in binary (not text) mode... */
    if ((fp = fopen(filename, FOPRWBIN)) == NULL)
        return -1;  /* error - file can't be opened */

    wipeout(filebPtr, fp);

    fclose(fp);
    return 0; /* normal return */
} /* wipefile */

/*
   Returns the part of a filename after all directory specifiers.
   Note: some functions depend on the side-effect that the returned
   pointer rests _within_ the source string.
 */
char *fileTail(char *filename)
{
    char *p;
    char const *s = DIRSEPS;

    while (*s) {
        p = strrchr(filename, *s);
        if (p)
            filename = p + 1;
        s++;
    }

    return filename;
}

PGPBoolean dropFilename(char *filename)
{
	char *p = NULL;
	char const *s = DIRSEPS;


	p = strrchr(filename, *s);
	if(p)
	{
		p[1] = '\0';
		return TRUE;
	}
	return FALSE;
}

/* return TRUE if extension matches the end of filename */
PGPBoolean hasExtension(char *filename, char *extension)
{
    int lf = strlen(filename);
    int lx = strlen(extension);

    if (lf <= lx)
        return FALSE;
    return !strcmp(filename + lf - lx, extension);
}

/* return TRUE if path is a filename created by tempFile() */
/* Filename matches "*.$[0-9][0-9]" */
PGPBoolean isTempFile( struct pgpfileBones *filebPtr, char *path)
{
    char *p = strrchr(path, '.');

    return p != NULL && p[1] == TMP_EXT &&
        isdigit(p[2]) && isdigit(p[3]) && p[4] == '\0';
}

/*
 * Returns TRUE if user left off file extension, allowing default.
 * Note that the name is misleading if multiple dots are allowed.
 * not_pgp_extension or something would be better.
 */
PGPBoolean noExtension( struct pgpfileBones *filebPtr, char *filename )
{
#ifdef MULTIPLE_DOTS  /* filename can have more than one dot */
    if (hasExtension(filename, filebPtr->ASC_EXTENSION) ||
            hasExtension(filename, filebPtr->PGP_EXTENSION) ||
            hasExtension(filename, filebPtr->PKR_EXTENSION) ||
            hasExtension(filename, filebPtr->SKR_EXTENSION) ||
            hasExtension(filename, filebPtr->SIG_EXTENSION) ||
            isTempFile(filebPtr, filename))
        return FALSE;
    else
        return TRUE;
#else
    filename = fileTail(filename);

    return strrchr(filename, '.') == NULL;
#endif
} /* no_extension */

/* deletes trailing ".xxx" file extension after the period. */
void dropExtension(struct pgpfileBones *filebPtr, char *filename )
{
    if (!noExtension( filebPtr, filename ))
        *strrchr(filename, '.') = '\0';
} /* drop_extension */

/* append filename extension if there isn't one already. */
void defaultExtension( struct pgpfileBones *filebPtr, char *filename,
        char *extension )
{
    if (noExtension( filebPtr, filename ))
        strcat(filename, extension);
} /* default_extension */

#ifndef MAX_NAMELEN
#if defined(AMIGA)||defined(NeXT)||(defined(BSD)&&BSD>41)\
    ||(defined(sun)&&defined(i386))
#define MAX_NAMELEN 255
#else
#include <limits.h>
#endif
#endif

/* truncate the filename so that an extension can be tacked on. */
static void truncateName( struct pgpfileBones *filebPtr, char *path,
int ext_len )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
#ifdef PGP_UNIX /* for other systems this is a no-op */
    PGPInt32 pri;
    PGPError err;
    char *p;
#ifdef MAX_NAMELEN /* overrides the use of pathconf() */
    int namemax = MAX_NAMELEN;
#else
    int namemax;
#ifdef _PC_NAME_MAX
    char dir[MAX_PATH];

    strcpy(dir, path);
    if ((p = strrchr(dir, '/')) == NULL) {
        strcpy(dir, ".");
    } else {
        if (p == dir)
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
    if (strlen(p) > namemax - ext_len) {
        if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
            fprintf(filebPtr->pgpout, LANG("Truncating filename '%s' "),
                    path);

        p[namemax - ext_len] = '\0';
        if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
            fprintf(filebPtr->pgpout, LANG("to '%s'\n"), path);
    }
#endif /* PGP_UNIX */
}

/* change the filename extension. */
void forceExtension( struct pgpfileBones *filebPtr, char *filename,
        char *extension )
{
    dropExtension( filebPtr, filename ); /* out with the old */
    truncateName( filebPtr, filename, strlen(extension) );
    strcat(filename, extension); /* in with the new */
} /* force_extension */

/*
 * Get yes/no answer from user, returns TRUE for yes, FALSE for no.
 * First the translations are checked, if they don't match 'y' and 'n'
 * are tried.
 */
PGPBoolean getyesno(struct pgpfileBones *filebPtr, char default_answer,
        PGPBoolean batchmode)
{
    char buf[8];
    static char yes[8], no[8];
    FILE *promptfp;
    if(filebPtr)
        promptfp=filebPtr->pgpout;
    else
        promptfp=stderr;

    if (yes[0] == '\0') {
        strncpy(yes, LANG("y"), 7);
        strncpy(no, LANG("n"), 7);
    }
    if (!batchmode) { /* return default answer in batchmode */
        pgpTtyGetString(buf, 6, promptfp); /* echo keyboard input */
        strlwr(buf);
        if (!strncmp(buf, no, strlen(no)))
            return FALSE;
        if (!strncmp(buf, yes, strlen(yes)))
            return TRUE;
        if (buf[0] == 'n')
            return FALSE;
        if (buf[0] == 'y')
            return TRUE;
    }
    return default_answer == 'y' ? TRUE : FALSE;
}/* getyesno */

/* if user consents to it, change the filename extension. */
char *maybeForceExtension( struct pgpfileBones *filebPtr, char *filename,
        char *extension )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;
    static char newname[MAX_PATH];

    if (!hasExtension(filename, extension)) {
        strcpy(newname, filename);
        forceExtension( filebPtr, newname, extension );
        if (!fileExists(newname)) {
            fprintf(filebPtr->pgpout,
                    LANG("\nShould '%s' be renamed to '%s' [Y/n]? "),
                    filename, newname);
            if (getyesno(filebPtr, 'y', (PGPBoolean)pgpenvGetInt( env,
                    PGPENV_BATCHMODE, &pri, &err )))
                return newname;
        }
    }
    return NULL;
}/* maybe_force_extension */

/*
 * Add a trailing directory separator to a name, if absent.
 */
static void addslash(char *name)
{
    int i = strlen(name);

    if (i != 0 && !strchr(DIRSEPS, name[i - 1])) {
        name[i] = DIRSEPS[0];
        name[i + 1] = '\0';
    }
}

/*
 * Builds a filename with a complete path specifier from the environmental
 * variable PGPPATH.
 */
char *buildFileName(char *result, const char *fname)
{
    char const *s = getenv(PGPPATH);

    result[0] = '\0';

    if (s && strlen(s) <= 50) {
        strcpy(result, s);
    }
#ifdef PGP_UNIX
    /* On Unix, default to $HOME/.pgp, otherwise, current directory. */
    else {
        s = getenv("HOME");
        if (s && strlen(s) <= 50) {
            strcpy(result, s);
            addslash(result);
            strcat(result, ".pgp");
        }
    }
#else if PGP_WIN32
    /* on Win32 (NT), try %USERPROFILE\Application Data\pgp,
       otherwise, %SYSTEMROOT\pgp directory. */
    else if ((s = getenv("USERPROFILE")) != NULL)
    {
        if (strlen(s) <= 50) {
            strcpy(result, s);
            strcat(result, "\\Application Data\\pgp");
        }
    }
    else
    {
        s = getenv("SYSTEMROOT");
        if(s && strlen(s) <= 50) {
            strcpy(result, s);
            strcat(result, "\\pgp");
        }
    }
#endif /* PGP_UNIX */
    addslash(result);
    strcat(result, fname);
    return result;
} /* buildFileName */

char *buildSysFileName(char *result, char *fname)
{
    buildFileName(result, fname);
#ifdef PGP_SYSTEM_DIR
    if (fileExists(result))
        return result;
    strcpy(result, PGP_SYSTEM_DIR);
    strcat(result, fname);
    if (fileExists(result))
        return result;
    buildFileName(result, fname); /* Put name back for error */
#endif
    return result;
}

/* Convert filename to canonical form, with slashes as separators */
void fileToCanon(char *filename)
{
#ifdef BSLASH
    while (*filename) {
        if (*filename == '\\')
            *filename = '/';
        ++filename;
    }
#endif
}

int writeError( struct pgpfileBones *filebPtr, FILE * f )
{
    fflush(f);
    if (ferror(f)) {
#ifdef ENOSPC
        if (errno == ENOSPC)
            fprintf(filebPtr->pgpout, LANG("\nDisk full.\n"));
        else
#endif
            fprintf(filebPtr->pgpout, LANG("\nFile write error.\n"));
        return -1;
    }
    return 0;
}

/* copy file f to file g, for longcount bytes */
int copyFile(FILE * f, FILE * g, PGPUInt32 longcount)
{
    int count, status = 0;
    do { /* read and write the whole file... */
        if (longcount < (PGPUInt32) DISKBUFSIZE)
            count = (int) longcount;
        else
            count = DISKBUFSIZE;
        count = fread(textbuf, 1, count, f);
        if (count > 0) {
            if (get_CONVERSION() != NO_CONV) {
                int i;
                for (i = 0; i < count; i++)
                    textbuf[i] = (get_CONVERSION() == EXT_CONV) ?
                        EXT_C(textbuf[i]) :
                        INT_C(textbuf[i]);
            }
            if (fwrite(textbuf, 1, count, g) != (size_t)count) {
                /* Problem: return error value */
                status = -1;
                break;
            }
            longcount -= count;
        }
        /* if text block was short, exit loop */
    } while (count == DISKBUFSIZE);
    burn(textbuf); /* burn sensitive data on stack */
    return status;
} /* copyFile */

/*
 * Like copyFile, but takes a position for file f.  Returns with
 * f and g pointing just past the copied data.
 */
int copyFilePos(FILE * f, FILE * g, PGPUInt32 longcount, PGPUInt32 fpos)
{
    fseek(f, fpos, SEEK_SET);
    return copyFile(f, g, longcount);
}

/* copy file f to file g, for longcount bytes.  Convert to
 * canonical form as we go.  f is open in text mode.  Canonical
 * form uses crlf's as line separators.
 */
int copyFileToCanon(FILE * f, FILE * g, PGPUInt32 longcount, PGPBoolean
        strip_spaces)
{
    int count, status = 0;
    PGPByte c, *tb1, *tb2;
    int i, nbytes;
    int nspaces = 0;
    do { /* read and write the whole file... */
        if (longcount < (PGPUInt32) DISKBUFSIZE)
            count = (int) longcount;
        else
            count = DISKBUFSIZE;
        count = fread(textbuf, 1, count, f);
        if (count > 0) {
            /* Convert by adding CR before LF */
            tb1 = textbuf;
            tb2 = (PGPByte *) textbuf2;
            for (i = 0; i < count; ++i) {
                switch (get_CONVERSION()) {
                    case EXT_CONV:
                        c = EXT_C(*tb1++);
                        break;
                    case INT_CONV:
                        c = INT_C(*tb1++);
                        break;
                    default:
                        c = *tb1++;
                }
                if (strip_spaces) {
                    if (c == ' ') {
                        /* Don't output spaces yet */
                        nspaces += 1;
                    } else {
                        if (c == '\n') {
                            *tb2++ = '\r';
                            nspaces = 0; /* Delete trailing spaces */
                        }
                        if (nspaces) {
                            /* Put out spaces now */
                            do
                                *tb2++ = ' ';
                            while (--nspaces);
                        }
                        *tb2++ = c;
                    }
                } else {
                    if (c == '\n')
                        *tb2++ = '\r';
                    *tb2++ = c;
                }
            }
            nbytes = tb2 - (PGPByte *) textbuf2;
            if (fwrite(textbuf2, 1, nbytes, g) != (size_t)nbytes) {
                /* Problem: return error value */
                status = -1;
                break;
            }
            longcount -= count;
        }
        /* if text block was short, exit loop */
    } while (count == DISKBUFSIZE);
    burn(textbuf); /* burn sensitive data on stack */
    burn(textbuf2);
    return status;
} /* copyFileToCanon */

/* copy file f to file g, for longcount bytes.  Convert from
 * canonical to local form as we go.  g is open in text mode.  Canonical
 * form uses crlf's as line separators.
 */
int copyFileFromCanon(FILE * f, FILE * g, PGPUInt32 longcount)
{
    int count, status = 0;
    PGPByte c, *tb1, *tb2;
    int i, nbytes;
    do { /* read and write the whole file... */
        if (longcount < (PGPUInt32) DISKBUFSIZE)
            count = (int) longcount;
        else
            count = DISKBUFSIZE;
        count = fread(textbuf, 1, count, f);
        if (count > 0) {
            /* Convert by removing CR's */
            tb1 = textbuf;
            tb2 = (PGPByte *) textbuf2;
            for (i = 0; i < count; ++i) {
                switch (get_CONVERSION()) {
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
            nbytes = tb2 - (PGPByte *) textbuf2;
            if (fwrite(textbuf2, 1, nbytes, g) != (size_t)nbytes) {
                /* Problem: return error value */
                status = -1;
                break;
            }
            longcount -= count;
        }
        /* if text block was short, exit loop */
    } while (count == DISKBUFSIZE);
    burn(textbuf); /* burn sensitive data on stack */
    burn(textbuf2);
    return status;
} /* copyFileFromCanon */

/* Copy srcFile to destFile */
int copyFilesByName( struct pgpfileBones *filebPtr, char *srcFile,
        char *destFile )
{
    FILE *f, *g;
    int status = 0;
    long fileLength;

    f = fopen(srcFile, FOPRBIN);
    if (f == NULL)
        return -1;
    g = fopen(destFile, FOPWBIN);
    if (g == NULL) {
        fclose(f);
        return -1;
    }
    /* Get file length and copy it */
    fseek(f, 0L, SEEK_END);
    fileLength = ftell(f);
    rewind(f);
    status = copyFile(f, g, fileLength);
    fclose(f);
    if (writeError( filebPtr, g ))
        status = -1;
    fclose(g);
    return status;
} /* copyFilesByName */

/* Copy srcFile to destFile, converting to canonical text form  */
int makeCanonical( struct pgpfileBones *filebPtr, char *srcFile, char
        *destFile )
{
    FILE *f, *g;
    int status = 0;
    long fileLength;

    if (((f = fopen(srcFile, FOPRTXT)) == NULL) ||
        ((g = fopen(destFile, FOPWBIN)) == NULL))
        /* Can't open files */
        return -1;

    /* Get file length and copy it */
    fseek(f, 0L, SEEK_END);
    fileLength = ftell(f);
    rewind(f);
    set_CONVERSION(INT_CONV);
    status = copyFileToCanon(f, g, fileLength, filebPtr->stripSpaces);
    set_CONVERSION(NO_CONV);
    fclose(f);
    if (writeError( filebPtr, g ))
        status = -1;
    fclose(g);
    return status;
} /* make_canonical */

/*
 * Like rename() but will try to copy the file if the rename fails.
 * This is because under OS's with multiple physical volumes if the
 * source and destination are on different volumes the rename will fail
 */
int rename2( struct pgpfileBones *filebPtr, char *srcFile, char *destFile )
{
    FILE *f, *g;
    int status = 0;
    long fileLength;

#ifdef VMS
    if (rename(srcFile, destFile) != 0)
#else
    if (rename(srcFile, destFile) == -1)
#endif
    {
        /* Rename failed, try a copy */
        if (((f = fopen(srcFile, FOPRBIN)) == NULL) ||
            ((g = fopen(destFile, FOPWBIN)) == NULL))
            /* Can't open files */
            return -1;

        /* Get file length and copy it */
        fseek(f, 0L, SEEK_END);
        fileLength = ftell(f);
        rewind(f);
        status = copyFile(f, g, fileLength);
        if (writeError( filebPtr, g ))
            status = -1;

        /* Zap source file if the copy went OK, otherwise zap the (possibly
           incomplete) destination file */
        if (status >= 0) {
            wipeout(filebPtr, f); /* Zap source file */
            fclose(f);
            remove(srcFile);
            fclose(g);
        } else {
            if (isRegularFile(destFile)) {
                wipeout(filebPtr, g); /* Zap destination file */
                fclose(g);
                remove(destFile);
            } else {
                fclose(g);
            }
            fclose(f);
        }
    }
    return status;
}

/* read the data from stdin to the phantom input file */
int readPhantomInput( struct pgpfileBones *filebPtr, char *filename )
{
    FILE *outFilePtr;
    PGPByte buffer[512];
    int bytesRead, status = 0;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;

    if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
        fprintf(filebPtr->pgpout, LANG("writing stdin to file %s\n"),
                filename);
    if ((outFilePtr = fopen(filename, FOPWBIN)) == NULL)
        return -1;

#if defined(MSDOS) || defined(OS2)
    /* Under DOS must set input stream to binary mode to avoid data
       mangling */
    setmode(fileno(stdin), O_BINARY);
#endif /* MSDOS || OS2 */
    while ((bytesRead = fread(buffer, 1, 512, stdin)) > 0)
        if (fwrite(buffer, 1, bytesRead, outFilePtr) != (size_t)bytesRead) {
            status = -1;
            break;
        }
    if (writeError( filebPtr, outFilePtr ))
        status = -1;
    fclose(outFilePtr);
#if defined(MSDOS) || defined(OS2)
    setmode(fileno(stdin), O_TEXT); /* Reset stream */
#endif /* MSDOS || OS2 */
    return status;
}

/* write the data from the phantom output file to stdout */
int writePhantomOutput( struct pgpfileBones *filebPtr, char *filename )
{
    FILE *outFilePtr;
    PGPByte buffer[512];
    int bytesRead, status = 0;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;

    if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
        fprintf(filebPtr->pgpout, LANG("writing file %s to stdout\n"),
        filename);

    /* this can't fail since we just created the file */
    outFilePtr = fopen(filename, FOPRBIN);

#if defined(MSDOS) || defined(OS2)
    setmode(fileno(stdout), O_BINARY);
#endif /* MSDOS || OS2 */
    while ((bytesRead = fread(buffer, 1, 512, outFilePtr)) > 0)
        if (fwrite(buffer, 1, bytesRead, stdout) != (size_t)bytesRead) {
            status = -1;
            break;
        }
    fclose(outFilePtr);
    fflush(stdout);
    if (ferror(stdout)) {
        status = -1;
        fprintf(filebPtr->pgpout, LANG("\007Write error on stdout.\n"));
    }
#if defined(MSDOS) || defined(OS2)
    setmode(fileno(stdout), O_TEXT);
#endif /* MSDOS || OS2 */

    return status;
}

/* Return the size from the current position of file f to the end */
PGPUInt32 fsize(FILE * f)
{
    long fpos = ftell(f);
    long fpos2;

    fseek(f, 0L, SEEK_END);
    fpos2 = ftell(f);
    fseek(f, fpos, SEEK_SET);
    return (PGPUInt32) (fpos2 - fpos);
}

/* Return TRUE if file filename looks like a pure text file */
int isTextFile( struct pgpfileBones *filebPtr, char *filename )
{
    FILE *f = fopen(filename, "r"); /* FOPRBIN gives problem with VMS */
    int i, n, bit8 = 0;
    unsigned char buf[512];
    unsigned char *bufptr = buf;
    unsigned char c;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;

    if (!f)
        return FALSE; /* error opening it, so not a text file */
    i = n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (n <= 0)
        return FALSE; /* empty file or error, not a text file */
    if (compressSignature(buf,NULL,NULL) >= 0)
        return FALSE;
    while (i--) {
        c = *bufptr++;
        if (c & 0x80)
            ++bit8;
        else /* allow BEL BS HT LF VT FF CR EOF control characters */
          if (c < '\007' || (c > '\r' && c < ' ' && c != '\032'))
            return FALSE; /* not a text file */
    }
    if (strcmp( pgpenvGetCString( env, PGPENV_LANGUAGE, &pri ), "ru") == 0)
        return TRUE;
    /* assume binary if more than 1/4 bytes have 8th bit set */
    return bit8 < n / 4;
} /* is_text_file */


/*
   temporary file routines
 */

#define MAXTMPF 8

#define TMP_INUSE 2

static struct {
    char path[MAX_PATH];
    int flags;
    int num;
} tmpf[MAXTMPF];

static char tmpdir[256]; /* temporary file directory */
static char outdir[256]; /* output directory */
static char tmpbasename[64] = "pgptemp"; /* basename for temporary files */

/*
 * set directory for temporary files.  path will be stored in
 * tmpdir[] with an appropriate trailing path separator.
 */
void setTempDir( struct pgpfileBones *filebPtr, char *path )
{
    char *p;

    if (path == NULL || *path == '\0') {
        tmpdir[0] = '\0';
        return;
    }
    strcpy(tmpdir, path);
    p = tmpdir + strlen(tmpdir) - 1;
    if (*p != '/' && *p != '\\' && *p != ']' && *p != ':') {
        /* append path separator, either / or \ */
        if ((p = strchr(tmpdir, '/')) == NULL &&
            (p = strchr(tmpdir, '\\')) == NULL)
            p = "/"; /* path did not contain / or \, use / */
        strncat(tmpdir, p, 1);
    }
}

/*
 * set output directory to avoid a file copy when temp file is renamed to
 * output file.  the argument filename must be a valid path for a file, not
 * a directory.
 */
void setOutDir( struct pgpfileBones *filebPtr, char *filename )
{
    char *p;

    if (filename == NULL) {
        strcpy(outdir, tmpdir);
        return;
    }
    strcpy(outdir, filename);
    p = fileTail(outdir);
    strcpy(tmpbasename, p);
    *p = '\0';
    dropExtension( filebPtr, tmpbasename );
#if !defined(BSD42) && !defined(BSD43) && !defined(sun)
    /*
     *  we don't depend on pathconf here, if it returns an incorrect value
     * for NAME_MAX (like Linux 0.97 with minix FS) finding a unique name
     * for temp files can fail.
     */
    tmpbasename[10] = '\0'; /* 14 char limit */
#endif
}

PGPError touchFile( char *filename, int mode )
{
    int fd;
#ifndef PGP_UNIX
    FILE *fp;
#endif

#if defined(PGP_UNIX) || defined(VMS)
    if ((fd = open(filename, O_RDWR | O_CREAT, mode)) != -1)
        close(fd);
#else
    if ((fp = fopen(filename, "w")) != NULL)
        fclose(fp);
    fd = (fp == NULL ? -1 : 0);
#endif
    return fd;
}

/*
 * return a unique temporary file name
 */
char *tempFile( struct pgpfileBones *filebPtr, int flags, int *perrorLvl)
{
    int i, j;
    int num;
    int fd;
#ifndef PGP_UNIX
    FILE *fp;
#endif
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;

    for (i = 0; i < MAXTMPF; ++i)
        if (tmpf[i].flags == 0)
            break;

    if (i == MAXTMPF) {
        /* message only for debugging, no need for LANG */
        fprintf(stderr, LANG("\n\007Out of temporary files\n"));
        return NULL;
    }
  again:
    num = 0;
    do {
        for (j = 0; j < MAXTMPF; ++j)
            if (tmpf[j].flags && tmpf[j].num == num)
                break;
        if (j < MAXTMPF)
            continue; /* sequence number already in use */
        sprintf(tmpf[i].path, "%s%s.%c%02d",
                ((flags & TMP_TMPDIR) && *tmpdir ? tmpdir : outdir),
                tmpbasename, TMP_EXT, num);
        if (!fileExists(tmpf[i].path))
            break;
    }
    while (++num < 100);

    if (num == 100) {
        fprintf(filebPtr->pgpout,
                LANG("\n\007tempfile: cannot find unique name\n"));
        return NULL;
    }
#if defined(PGP_UNIX) || defined(VMS)
    if ((fd = open(tmpf[i].path, O_EXCL | O_RDWR | O_CREAT, 0600)) != -1)
        close(fd);
#else
    if ((fp = fopen(tmpf[i].path, "w")) != NULL)
        fclose(fp);
    fd = (fp == NULL ? -1 : 0);
#endif

    if (fd == -1) {
        if (!(flags & TMP_TMPDIR)) {
            flags |= TMP_TMPDIR;
            goto again;
        }
#ifdef PGP_UNIX
        else if (tmpdir[0] == '\0') {
            strcpy(tmpdir, "/tmp/");
            goto again;
        }
#endif
    }
    if (fd == -1) {
        fprintf(filebPtr->pgpout,
                LANG("\n\007Cannot create temporary file '%s'\n"),
                tmpf[i].path);
        userError(filebPtr, perrorLvl);
        return NULL;
    }
#ifdef VMS
    remove(tmpf[i].path);
#endif

    tmpf[i].num = num;
    tmpf[i].flags = flags | TMP_INUSE;
    if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
        fprintf(filebPtr->pgpout, LANG("tempfile: created '%s'\n"),
                tmpf[i].path);
    return tmpf[i].path;
} /* tempfile */

/*
 * remove temporary file, wipe if necessary.
 */
void rmTemp( struct pgpfileBones *filebPtr, char *name )
{
    int i;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;

    for (i = 0; i < MAXTMPF; ++i)
        if (tmpf[i].flags && strcmp(tmpf[i].path, name) == 0)
            break;

    if (i < MAXTMPF) {
        if (strlen(name) > 3 && name[strlen(name) - 3] == TMP_EXT) {
            /* only remove file if name hasn't changed */
            if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
                fprintf(filebPtr->pgpout, LANG("rmtemp: removing '%s'\n"),
                        name);
            if (tmpf[i].flags & TMP_WIPE)
                wipefile(filebPtr, name);
            if (!remove(name)) {
                tmpf[i].flags = 0;
            } else if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err )) {
                fprintf(stderr, LANG("\nrmtemp: Failed to remove %s"),
                        name);
                perror(LANG("\nError"));
            }
        } else if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
            fprintf(filebPtr->pgpout, LANG("rmtemp: not removing '%s'\n"),
                    name);
    }
} /* rmtemp */

/*
 * make temporary file permanent, returns the new name.
 */
char *saveTemp( struct pgpfileBones *filebPtr, char *name, char *newname )
{
    int i, overwrite;
    static char buf[MAX_PATH];
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;

    if (strcmp(name, newname) == 0)
        return name;

    for (i = 0; i < MAXTMPF; ++i)
        if (tmpf[i].flags && strcmp(tmpf[i].path, name) == 0)
            break;

    if (i < MAXTMPF) {
        if (strlen(name) < 4 || name[strlen(name) - 3] != TMP_EXT) {
            if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
                fprintf(filebPtr->pgpout,
                        LANG("savetemp: not renaming '%s' to '%s'\n"),
                        name, newname);
            return name; /* return original file name */
        }
    }
    while (fileExists(newname)) {
        if (pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err ) &&
                !pgpenvGetInt( env, PGPENV_FORCE, &pri, &err )) {
            fprintf(filebPtr->pgpout,
                    LANG("\n\007Output file '%s' already exists.\n"),
                    newname);
            return NULL;
        }
        if (isRegularFile(newname)) {
            if (pgpenvGetInt( env, PGPENV_FORCE, &pri, &err )) {
                /* remove without asking */
                remove(newname);
                break;
            }
            fprintf(filebPtr->pgpout, LANG(
"\n\007Output file '%s' already exists.  Overwrite (y/N)? "),
                    newname);
            overwrite = getyesno(filebPtr,'n', (PGPBoolean)pgpenvGetInt( env,
                    PGPENV_BATCHMODE, &pri, &err ));

        } else {
            fprintf(filebPtr->pgpout, LANG(
"\n\007Output file '%s' already exists.\n"), newname);

            if (pgpenvGetInt( env, PGPENV_FORCE, &pri, &err ))
                /* never remove special file */
                return NULL;
            overwrite = FALSE;
        }

        if (!overwrite) {
            fprintf(filebPtr->pgpout, LANG("\nEnter new file name: "));
            pgpTtyGetString(buf, MAX_PATH - 1, filebPtr->pgpout);
            if (buf[0] == '\0')
                return NULL;
            newname = buf;
        } else {
            remove(newname);
        }
    }
    if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
        fprintf(filebPtr->pgpout, LANG("savetemp: renaming '%s' to '%s'\n"),
                name, newname);
    if (rename2( filebPtr, name, newname ) < 0) {
        /* errorLvl = UNKNOWN_FILE_ERROR; */
        fprintf(filebPtr->pgpout, LANG("Can't create output file '%s'\n"),
                newname);

        return NULL;
    }
    if (i < MAXTMPF)
        tmpf[i].flags = 0;
    return newname;
}/* savetemp */

/*
 * like saveTemp(), only make backup of destname if it exists
 */
int saveTempBak( struct pgpfileBones *filebPtr, char *tmpname, char
        *destname )

{
    char bakpath[MAX_PATH];
#ifdef PGP_UNIX
    int mode = -1;
#endif

    if (isTempFile(filebPtr, destname)) {
        remove(destname);
    } else {
        if (fileExists(destname)) {
#ifdef PGP_UNIX
            struct stat st;
            if (stat(destname, &st) != -1)
                mode = st.st_mode & 07777;
#endif
            strcpy(bakpath, destname);
            forceExtension( filebPtr, bakpath, filebPtr->BAK_EXTENSION );
            remove(bakpath);
#ifdef VMS
            if (rename(destname, bakpath) != 0)
#else
            if (rename(destname, bakpath) == -1)
#endif
                return -1;
        }
    }
    if (saveTemp( filebPtr, tmpname, destname ) == NULL)
        return -1;
#ifdef PGP_UNIX
    if (mode != -1)
        chmod(destname, mode);
#endif
    return 0;
}

/*
 * remove all temporary files and wipe them if necessary
 */
void cleanupTempFiles(struct pgpfileBones *filebPtr)
{
    int i;

    for (i = 0; i < MAXTMPF; ++i)
        if (tmpf[i].flags)
            rmTemp( filebPtr, tmpf[i].path );
} /* cleanup_tmpf */

#if 0
/* search for manuals is disabled at the request of Jon Callas */
/*
 * Routines to search for the manuals.
 *
 * Why all this code?
 *
 * Some people may object to PGP insisting on finding the manual somewhere
 * in the neighborhood to generate a key.  They bristle against this
 * seemingly authoritarian attitude.  Some people have even modified PGP
 * to defeat this feature, and redistributed their hotwired version to
 * others.  That creates problems for me (PRZ).
 *
 * Here is the problem.  Before I added this feature, there were maimed
 * versions of the PGP distribution package floating around that lacked
 * the manual.  One of them was uploaded to Compuserve, and was
 * distributed to countless users who called me on the phone to ask me why
 * such a complicated program had no manual.  It spread out to BBS systems
 * around the country.  And a freeware distributor got hold of the package
 * from Compuserve and enshrined it on CD-ROM, distributing thousands of
 * copies without the manual.  What a mess.
 *
 * Please don't make my life harder by modifying PGP to disable this
 * feature so that others may redistribute PGP without the manual.  If you
 * run PGP on a palmtop with no memory for the manual, is it too much to
 * ask that you type one little extra word on the command line to do a key
 * generation, a command that is seldom used by people who already know
 * how to use PGP?  If you can't stand even this trivial inconvenience,
 * can you suggest a better method of reducing PGP's distribution without
 * the manual?
 */

static unsigned extMissing(char *prefix)
{
    static char const *const extensions[] =
#ifdef VMS
        { ".doc", ".txt", ".man", ".tex", ".", 0 };
#else
        { ".doc", ".txt", ".man", ".tex", "", 0 };
#endif
    char const *const *p;
    char *end = prefix + strlen(prefix);

    for (p = extensions; *p; p++) {
        strcpy(end, *p);
#if 0 /* Debugging code */
        fprintf(filebPtr->pgpout, LANG("Looking for \"%s\"\n"), prefix);
#endif
        if (fileExists(prefix))
            return 0;
    }
    return 1;
}

/*
 * Returns mask of files missing
 */
static unsigned filesMissing(char *prefix)
{
    static char const *const names[] =
    {"pgpdoc1", "pgpdoc2", 0};
    char const *const *p;
    unsigned bit, mask = 3;
    int len = strlen(prefix);

#ifndef VMS
    /*
     * Optimization: if directory doesn't exist, stop.  But access()
     * (used internally by fileExists()) doesn't work on dirs under VMS.
     */
    if (prefix[0] && !fileExists(prefix)) /* Directory doesn't exist? */
        return mask;
#endif /* VMS */
    if (len && strchr(DIRSEPS, prefix[len - 1]) == 0)
        prefix[len++] = DIRSEPS[0];
    for (p = names, bit = 1; *p; p++, bit <<= 1) {
        strcpy(prefix + len, *p);
        if (!extMissing(prefix))
            mask &= ~bit;
    }

    return mask; /* Bitmask of which files exist */
}

/*
 * Search prefix directory and doc subdirectory.
 */
static unsigned docMissing(char *prefix)
{
    unsigned mask;
    int len = strlen(prefix);

    mask = filesMissing(prefix);
    if (!mask)
        return 0;
#ifdef VMS
    if (len && prefix[len - 1] == ']') {
        strcpy(prefix + len - 1, ".doc]");
    } else {
        assert(!len || prefix[len - 1] == ':');
        strcpy(prefix + len, "[doc]");
    }
#else
    if (len && prefix[len - 1] != DIRSEPS[0])
        prefix[len++] = DIRSEPS[0];
    strcpy(prefix + len, "doc");
#endif

    mask &= filesMissing(prefix);

    prefix[len] = '\0';
    return mask;
}

/*
 * Expands a leading environment variable.  Returns 0 on success;
 * <0 if there is an error.
 */
static int expandEnv(char const *src, char *dest)
{
    char const *var, *suffix;
    unsigned len;

    if (*src != '$') {
        strcpy(dest, src);
        return 0;
    }
    /* Find end of variable */
    if (src[1] == '{') { /* ${FOO} form */
        var = src + 2;
        len = strchr(var, '}') - var;
        suffix = src + 2 + len + 1;
    } else { /* $FOO form - allow $ for VMS */
        var = src + 1;
        len = strspn(var, "ABCDEFGHIJKLMNOPQRSTUVWXYZ$_");
        suffix = src + 1 + len;
    }

    memcpy(dest, var, len); /* Copy name */
    dest[len] = '\0'; /* Null-terminate */

    var = getenv(dest);
    if (!var || !*var)
        return -1; /* No env variable */

    /* Copy expanded form to destination */
    strcpy(dest, var);

    /* Add tail */
    strcat(dest, suffix);

    return 0;
}

unsigned manualsMissing(void)
{
    char buf[256];
    unsigned mask = ~((unsigned)0);
    char const *const *p;

    for (p = manual_dirs; *p; p++) {
        if (expandEnv(*p, buf) < 0)
            continue; /* Ignore */
        mask &= docMissing(buf);
        if (!mask)
            break;
    }

    return mask;
}

/*
 * Why all this code?
 *
 * See block of comments above.
 */
#endif



/* Reads the first count bytes from infile into header. */
int getHeaderInfoFromFile(char *infile,  PGPByte *header, int count)
{
        FILE *f;
        fill0(header,count);
        /* open file f for read, in binary (not text) mode...*/
        if ((f = fopen(infile,FOPRBIN)) == NULL)
                return -1;
        /* read Cipher Type Byte, and maybe more */
        count = fread(header,1,count,f);
        fclose(f);
        return count; /* normal return */
} /* get_header_info_from_file */

