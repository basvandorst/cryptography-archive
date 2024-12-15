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

#pragma ident "@(#)Bigint.h	1.6 96/01/29 Sun Microsystems"

#ifndef BIGINT_H
#define BIGINT_H

#include <stdio.h>
#include "my_types.h"
#include "Bstream.h"

class Bigint {
#define USE_DOUBLE	0
#define USE_BSTREAM	1
	int	flag;
	long  	longint;
	Bstream bstrint;
public:
	// Constructors etc
	Bigint();
	Bigint( short );
	Bigint( const char * );
	Bigint( const unsigned char *, int );
	Bigint& operator =(const Bigint &);
	Bigint(const Bigint &);
	~Bigint();
	// operators and member functions
	friend Bigint operator +(const Bigint &, const Bigint &);
	friend Bigint operator *(const Bigint &, const Bigint &);
	//friend Bigint operator /(const Bigint &, const Bigint &);
	friend Bigint operator -(const Bigint &, const Bigint &);
	friend Boolean operator <(const Bigint &, const Bigint &);
	//friend Boolean operator >(const Bigint &, const Bigint &);
	friend Boolean operator !=(const Bigint &, const Bigint &);
	friend Boolean operator ==(const Bigint &, const Bigint &);
	//friend Boolean operator <=(const Bigint &, const Bigint &);
	//friend Boolean operator >=(const Bigint &, const Bigint &);
	void print() const;		// Hex output
	void printd() const;		// Decimal output
	String getnumstr() const;	// Hex output to String
	String getnumstrd() const;	// Decimal output to String
	Bstream getbstr() const;	// return Bstream aspect
	int bits() const;		// Number of bits in this bigint
};

extern Bstream Bigint_to_Bstr(const Bigint&);
int convert_Bigint_to_bytes(const Bigint, int, unsigned char *);
int bits_in_bigint(const Bigint);

#endif BIGINT_H
