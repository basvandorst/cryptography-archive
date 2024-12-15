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

#pragma ident "@(#)Bstream.h	1.4 96/01/29 Sun Microsystems"

#ifndef BSTREAM_H
#define BSTREAM_H

/*
 * Binary Stream Data Type
 */

#include "my_types.h"

const int SUCCESS =	0;
const int FAILURE =	-1;
const int EOS =		FAILURE;

class Bstream {
	int		peekindex;
	byte		*deletethis;
	int 		len;
	byte		*datap;
 public:
	Bstream();
	Bstream(int, byte *);
	Bstream(const char *);
	Bstream(const String &);
	Bstream(const Bstream &);
	~Bstream();
	operator const char *() const;
	//operator String() const;
	Bstream& operator =(const Bstream &);
	friend Boolean operator ==(const Bstream&, const Bstream&);
	friend Boolean operator !=(const Bstream&, const Bstream&);
	friend Bstream operator +(const Bstream&, const Bstream&);
	Bstream& operator +=(const Bstream&);
	int consume(int );	// Consume from beginning of stream
	int truncate(int );	// truncate from end of stream
	int getlength() const;
	byte *getdatap() const;
	String getstr() const;
	int fetchbyte(byte &);
	int peekbyte(byte &);
	byte last() const;
	Bstream replace(const Bstream&, const Bstream&) const;
					// Replace string from with string to
	int store(const char *) const;	// Save binary string in file
	int write(FILE *f) const;	// Write binary string to a FILE ptr.
	void print() const;		// Hex dump
	void prints() const;		// print as ASCII string
	void printhexint() const;	// Print as a MP Hex integer
	void printdecint() const;	// print as a MP Decimal integer
	String gethexstr() const;	// Hex output as integer to a String
	String getdecstr() const;	// Decimal output as integer to a String
};

extern Bstream File_to_Bstr(const char *);
extern String File_to_String(const char *);

#endif BSTREAM_H
