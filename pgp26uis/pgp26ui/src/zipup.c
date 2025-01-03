/*

 Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup Gailly.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included
 unmodified, that it is not sold for profit, and that this copyright notice
 is retained.

*/

/*
 *  zipup.c by Mark Adler. Includes modifications by Jean-loup Gailly.
 */

#define NOCPYRT         /* this is not a main module */
#include <ctype.h>
#include "zip.h"
#include "zrevisio.h"

/* Use the raw functions for MSDOS and Unix to save on buffer space.
   They're not used for VMS since it doesn't work (raw is weird on VMS).
   (This sort of stuff belongs in fileio.c, but oh well.) */
#ifdef VMS
   typedef FILE *ftype;
#  define fhow FOPR
#  define fbad NULL
#  define zopen(n,p) fopen(n,p)
#  define zread(f,b,n) fread(b,1,n,f)
#  define zclose(f) fclose(f)
#  define zerr(f) ferror(f)
#  define zrew(f) rewind(f)
#  define zstdin stdin
#else /* !VMS */
#  ifdef MSDOS
#    include <io.h>
#    include <fcntl.h>
#    define fhow (O_RDONLY|O_BINARY)
#  else /* !MSDOS */
#ifndef AMIGA
     off_t lseek();
#endif /* AMIGA */
#    define fhow 0
#  endif /* ?MSDOS */
   typedef int ftype;
#  define fbad (-1)
#  define zopen(n,p) open(n,p)
#  define zread(f,b,n) read(f,b,n)
#  define zclose(f) close(f)
#  define zerr(f) (k==(extent)(-1L))
#  define zrew(f) lseek(f,0L,0)
#  define zstdin 0
#endif /* ?VMS */

/* Local data */

local ftype ifile;		/* file to compress */

void lm_free();
void ct_free();

int zipup(FILE *inFile, FILE *y)
/* Compress the file fileName and write it to the file *y. Return an error
   code in the ZE_ class.  Also, update tempzn by the number of bytes written. */
/* ??? Does not yet handle non-seekable y */
{
  int m;				/* method for this entry */
  off_t q = -1L;			/* size returned by filetime */
  ush att;				/* internal file attributes (dummy only) */
  ush flg;				/* gp compresion flags (dummy only) */

	/* Set input file and find its size */
#ifdef VMS
	ifile = inFile;
	fseek(ifile, 0L, SEEK_END);
	q = ftell(ifile);
	fseek(ifile, 0L, SEEK_SET);
#else
	ifile = fileno( inFile );
	q = lseek(ifile, 0L, SEEK_END);
	lseek(ifile, 0L, SEEK_SET);
#endif /* VMS */

	m = (q == 0) ? STORE : DEFLATE;

  if (m == DEFLATE) {
	 bi_init(y);
	 att = UNKNOWN;
	 ct_init(&att, &m);
	 lm_init(level, &flg);
	 /* s = */ deflate();
  }
  lm_free();
  ct_free();

  return(0);
}

int read_buf(buf, size)
  char far *buf;
  unsigned size;
/* Read a new buffer from the current input file, and update the crc and
 * input file size.
 * IN assertion: size >= 2 (for end-of-line translation) */
{
  unsigned len;

  len = zread(ifile, buf, size);
  if (len == (unsigned)EOF || len == 0) return len;
  return len;
}
