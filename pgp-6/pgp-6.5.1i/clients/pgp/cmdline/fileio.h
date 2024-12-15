/*____________________________________________________________________________
    fileio.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: fileio.h,v 1.4 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/

#define DISKBUFSIZE 4096 /* Size of I/O buffers */

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#ifdef VMS
#define PGP_SYSTEM_DIR "PGP$LIBRARY:"
#define FOPRBIN  "rb","ctx=stm"
#define FOPRTXT  "r"
#if 0
#define FOPWBIN  "ab","fop=cif"
#define FOPWTXT  "a","fop=cif"
#else
#define FOPWBIN  "wb"
#define FOPWTXT  "w"
#endif
#define FOPRWBIN "r+b","ctx=stm"
#define FOPWPBIN "w+b","ctx=stm"
#else
#ifdef PGP_UNIX
/*
 * Directory for system-wide files.  Must end in a /, ready for
 * dumb appending of the filename.  (If not defined, it's not used.)
 */
#define PGP_SYSTEM_DIR "/usr/local/lib/pgp/"
#define FOPRBIN  "r"
#define FOPRTXT  "r"
#define FOPWBIN  "w"
#define FOPWTXT  "w"
#define FOPRWBIN "r+"
#define FOPWPBIN "w+"
#else /* !PGP_UNIX && !VMS */
#define FOPRBIN  "rb"
#define FOPRTXT  "r"
#define FOPWBIN  "wb"
#define FOPWTXT  "w"
#define FOPRWBIN "r+b"
#define FOPWPBIN "w+b"
#endif /* PGP_UNIX */
#endif /* VMS */

#define TMP_WIPE  1
#define TMP_TMPDIR  4

#define equalBuffers(buf1,buf2,count) !memcmp( buf1, buf2, count )

/* Returns TRUE iff file is can be opened for reading. */
PGPBoolean fileExists(char *filename);

/* Returns TRUE iff file can be opened for writing. Does not harm file! */
PGPBoolean fileOkWrite(char *filename);

/* Completely overwrite and erase file of given name, so that no sensitive
   information is left on the disk */
int wipefile(struct pgpfileBones *filebPtr, char *filename);

/* Return the after-slash part of the filename */
char *fileTail (char *filename);

PGPBoolean dropFilename(char *filename);

struct pgpfileBones;
/* Returns TRUE if user left off file extension, allowing default */
PGPBoolean noExtension(struct pgpfileBones *filebPtr, char *filename);

/* Deletes trailing ".xxx" file extension after the period */
void dropExtension(struct pgpfileBones *filebPtr, char *filename);

/* Append filename extension if there isn't one already */
void defaultExtension(struct pgpfileBones *filebPtr,
        char *filename, char *extension );

/* Change the filename extension */
void forceExtension(struct pgpfileBones *filebPtr,
        char *filename, char *extension );

/* If luser consents to it, change the filename extension */
char *maybeForceExtension(struct pgpfileBones *filebPtr,
        char *filename, char *extension );

/* Builds a filename with a complete path specifier from the environmental
   variable PGPPATH */
char *buildFileName(char *result, const char *fname);

/* The same, but also searches PGP_SYETEM_DIR */
char *buildSysFileName(char *result, char *fname);

/* Build a path for fileName based on origPath */
int buildPath(char *path, char *fileName, char *origPath);

/* Convert filename to canonical form, with slashes as separators */
void fileToCanon(char *filename);

/* Convert filename from canonical to local form */
void fileFromCanon(char *filename);

/* Copy file f to file g, for longcount bytes */
int copyFile(FILE *f, FILE *g, PGPUInt32 longcount);

/* Copy file f to file g, for longcount bytes, positioning f at fpos */
int copyFilePos (FILE *f, FILE *g, PGPUInt32 longcount, PGPUInt32 fpos);

/* Copy file f to file g, for longcount bytes.  Convert to canonical form
   as we go.  f is open in text mode.  Canonical form uses crlf's as line
   separators */
int copyFileToCanon (FILE *f, FILE *g, PGPUInt32 longcount,
        PGPBoolean strip_spaces);

/* Copy file f to file g, for longcount bytes.  Convert from canonical to
   local form as we go.  g is open in text mode.  Canonical form uses crlf's
   as line separators */
int copyFileFromCanon (FILE *f, FILE *g, PGPUInt32 longcount);

/* Copy srcFile to destFile */
int copyFilesByName(struct pgpfileBones *filebPtr, char *srcFile,
        char *destFile);

/* Copy srcFile to destFile, converting to canonical text form */
int makeCanonical(struct pgpfileBones *filebPtr, char *srcFile,
        char *destFile);

/* Like rename() but will try to copy the file if the rename fails. This is
   because under OS's with multiple physical volumes if the source and
   destination are on different volumes the rename will fail */
int rename2(struct pgpfileBones *filebPtr, char *srcFile, char *destFile);

/* Read the data from stdin to the phantom input file */
int readPhantomInput(struct pgpfileBones *filebPtr, char *filename);

/* Write the data from the phantom output file to stdout */
int writePhantomOutput(struct pgpfileBones *filebPtr, char *filename);

/* Return the size from the current position of file f to the end */
PGPUInt32 fsize (FILE *f);

/* Return TRUE if file filename is a text file */
int isTextFile (struct pgpfileBones *filebPtr, char *filename);

FILE *fopenbin(char *, char *);
FILE *fopentxt(char *, char *);

char *tempFile(struct pgpfileBones *filebPtr, int, int *perrorLvl);
void rmTemp(struct pgpfileBones *filebPtr, char *);
char *saveTemp(struct pgpfileBones *filebPtr, char *, char *);
void cleanupTempFiles(struct pgpfileBones *filebPtr);
int saveTempBak(struct pgpfileBones *filebPtr, char *, char *);

extern int writeError(struct pgpfileBones *filebPtr, FILE *f);
extern void setTempDir(struct pgpfileBones *filebPtr, char *path);
extern void setOutDir(struct pgpfileBones *filebPtr, char *filename);
extern PGPBoolean isTempFile(struct pgpfileBones *filebPtr, char *path);
extern PGPBoolean hasExtension(char *filename, char *extension);

/* Directories to search for the manuals */
/*extern char const * const manual_dirs[];*/
/* Returns non-zero if any manuals are missing */
/*unsigned manualsMissing(void);*/

