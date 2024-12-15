/*
------------------------------------------------------------------
  Copyright
  Sun Microsystems, Inc.


  Copyright (C) 1994, 1995, 1996 Sun Microsystems, Inc.  All Rights
  Reserved.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software or derivatives of the Software, and to 
  permit persons to whom the Software or its derivatives is furnished 
  to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT.  IN NO EVENT SHALL SUN MICROSYSTEMS, INC., BE LIABLE
  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR DERIVATES OF THIS SOFTWARE OR 
  THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Except as contained in this notice, the name of Sun Microsystems, Inc.
  shall not be used in advertising or otherwise to promote
  the sale, use or other dealings in this Software or its derivatives 
  without prior written authorization from Sun Microsystems, Inc.
*/

#pragma ident "@(#)my_types.h	1.5 96/01/29 Sun Microsystems"

#ifndef MY_TYPES_H
#define	MY_TYPES_H

#include <stdio.h>

#ifdef RW
#include <rw/slistcol.h>
#include <rw/cstring.h>

typedef RWSlistCollectables	Slist;
typedef RWCollectable		ListElem;
typedef RWSlistCollectablesIterator SlistIterator;
typedef RWCString		String;
typedef RWBoolean		Boolean;
#define	SLList	Slist

#else

#ifdef LIBG
//enum Bool {BOOL_FALSE, BOOL_TRUE};
//typedef RWBoolean boolean;
#include <String.h>
#include <Pix.h>
#include <SLList.h>

#define TRUE		1
#define FALSE		0

typedef int		Boolean;
#endif /* LIBG */
#endif /* RW */

#define	BOOL_TRUE	TRUE
#define	BOOL_FALSE	FALSE

typedef unsigned char	byte;

#ifdef SOLARIS2
#define bzero(b, len)	memset(b, 0, len)
#define bcopy(b1, b2, len)  memcpy(b2, b1, len)
#define bcmp(b1, b2, len)  memcmp(b1, b2, len)
#else

extern "C" {
//	int write(int, void *, int);
//	int read(int, void *, int);
//	void close(int);
//	void bzero(void *, int);
//	int bcopy(void *, void *, int);
//	int bcmp(void *, void *, int);
//	int strlen(const char *);
//	char *strcpy(char *, char *);
//	int strcmp(char *, char *); 
//	char *calloc(unsigned int, unsigned int);
	char *getenv(const char *);
//	int system(char *);
};
#endif 

#endif MY_TYPES_H
