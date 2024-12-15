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

#pragma ident "@(#)Bigint.C	1.7 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#define GUIDE_INCLUDES
#include "my_types.h"
#include "Bstream.h"
#include "Bigint.h"
#include "Time.h"
#include "asn1_der.h"

#ifndef SOLARIS2
extern "C" {
	void bzero(void *, int);
};
#endif 

Bigint::Bigint() 
{
	flag = USE_DOUBLE;
	longint = 0;
}

Bigint::Bigint(short n) 
{
	flag = USE_DOUBLE;
	longint = n;
}

byte
atob(char c)
{
        if (c >= '0' && c <= '9') {
                return(c - '0');
        } else if (c >= 'a' && c <= 'f') {
                return(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
                return(c - 'A' + 10);
        } else {
                return(0xff);
        }
}

byte
bigintatob(char c)
{
	if (c >= '0' && c <= '9') {
		return(c - '0');
	} else if (c >= 'a' && c <= 'f') {
		return(c - 'a' + 10);
	} else if (c >= 'A' && c <= 'F') {
		return(c - 'A' + 10);
	} else {
		return(0xff);
	}
}

Bigint::Bigint(const char *s) 
{
	if (strlen(s) < sizeof(longint)*2) {
		flag = USE_DOUBLE;
		longint = strtol(s, NULL, 16);
	} else {
		while (*s == '0' ) 
			s++;
		flag = USE_BSTREAM;
		const char *inthexstr = s;
		int len = strlen(s);
		Boolean isodd = BOOL_FALSE;
		if (len & 0x01)	{	// odd number of digits
			len++;
			isodd = BOOL_TRUE;
		}
		int intlen = len/2;
		byte *intstr = new byte[intlen]; // a hex digit takes half an octet
		bzero(intstr, intlen);

		for (int i=0, j=0; i < intlen; i++) {
			byte hinib, lonib;
			if (isodd == BOOL_TRUE && i == 0) {
				hinib = 0x0;
				lonib = bigintatob((char )inthexstr[j]);
				j += 1;
			} else {
				hinib = bigintatob((char )inthexstr[j]);
				lonib = bigintatob((char )inthexstr[j+1]);
				j += 2;
			}
			intstr[i] = (hinib << 4) | lonib;
		}
		bstrint = Bstream(intlen, intstr);
	}
}

#ifdef BIGINT_DEBUG
int inline convert_bytes_to_int(byte *four_bytes)
{
#if defined(sparc)
	int val;
	byte *bytep = (byte *)&val;

	bytep[0] = four_bytes[3];
	bytep[1] = four_bytes[2];
	bytep[2] = four_bytes[1];
	bytep[3] = four_bytes[0];
	return (val);
#else
	printf("convert_bytes_to_int: machine architecture not supported\n");
	return (-1);
#endif
}
#else BIGINT_DEBUG
extern int convert_bytes_to_int(byte *);
#endif BIGINT_DEBUG


Bigint::Bigint(const unsigned char *d, int len)
{
	while (*d == 0 && len > 0) {
		d++;len--;
	}
	if (len > 4) {
		flag = USE_BSTREAM;
		bstrint = Bstream(len, (byte *)d);
	} else {
		int i,j;
		flag = USE_DOUBLE;
		byte fourbytes[4];
		bzero(fourbytes, sizeof(fourbytes));
		for (i = len - 1, j=0; i >= 0; i--, j++) {
			fourbytes[i] = d[j];
		}
		longint = convert_bytes_to_int(fourbytes);
	}
}

Bigint::~Bigint() 
{
	
}

Bigint::Bigint(const Bigint &a)
{
	flag = a.flag;
	longint = a.longint;
	bstrint = a.bstrint;
}

Bigint&
Bigint:: operator =(const Bigint &a)
{
	// Release memory from old information, if any
// XXX- TM
//	Bigint::~Bigint();
	flag = a.flag;
	longint = a.longint;
	bstrint = a.bstrint;
	return (*this);
}


Bigint operator + (const Bigint& a, const Bigint& b)
{
	Bigint c;

	if ((a.flag == b.flag) && (a.flag == USE_DOUBLE)) {
		c.longint = a.longint + b.longint;
		return (c);
	}
	fprintf(stderr, "Bigint: Cannot add larger than double values\n");
	abort();
	return (c);
}

Bigint operator * (const Bigint& a, const Bigint& b)
{
	Bigint c;

	if ((a.flag == b.flag) && (a.flag == USE_DOUBLE)) {
		c.longint = a.longint * b.longint;
		return (c);
	}
	fprintf(stderr, "Bigint: Cannot multiply larger than double values\n");
	abort();
	return (c);
}

Bigint operator - (const Bigint& a, const Bigint& b)
{
	Bigint c;
	if ((a.flag == b.flag) && (a.flag == USE_DOUBLE)) {
		c.longint = a.longint - b.longint;
		return (c);
	}
	fprintf(stderr, "Bigint: Cannot subtract larger than double values\n");
	abort();
	return (c);
}

Boolean operator ==(const Bigint& a, const Bigint& b)
{
	if ((a.flag == b.flag) && (a.flag == USE_DOUBLE)) {
		if (a.longint == b.longint)
			return TRUE;
		else
			return FALSE;
	} else {
		if (a.bstrint == b.bstrint)
			return TRUE;
		else
			return FALSE;
	}
}

Boolean operator !=(const Bigint& a, const Bigint& b)
{
	if (a == b)
		return FALSE;
	else
		return TRUE;
}

Boolean operator <(const Bigint& a, const Bigint& b)
{
	if ((a.flag == b.flag) && a.flag == USE_DOUBLE) {
		return (a.longint < b.longint);
	}
	fprintf(stderr, "Cannot check for < for this Bigint\n");
	abort();
	return TRUE;
}


// Hex output
void Bigint::print() const
{
	if (flag == USE_DOUBLE) {
		printf("%x", longint);
	} else {
		bstrint.printhexint();
	}
	return;
}

// Decimal output
void Bigint::printd() const
{
	if (flag == USE_DOUBLE) {
		printf("%d", longint);
	} else {
		bstrint.printdecint();
	}
	return;
}

// Hex output to a String
String Bigint::getnumstr() const
{
	char buf[100];
	bzero(buf, sizeof(buf));
	if (flag == USE_DOUBLE) {
		sprintf(buf,"%x", longint);
		String numstr = buf;
		return numstr;
	} else {
		return bstrint.gethexstr();
	}
}

// Decimal output to a String
String Bigint::getnumstrd() const
{
	char buf[100];
	bzero(buf, sizeof(buf));
	if (flag == USE_DOUBLE) {
		sprintf(buf,"%d", longint);
		String numstr = buf;
		return numstr;
	} else {
		return bstrint.getdecstr();
	}

}

Bstream Bigint::getbstr() const
{
	Bstream result;
	return bstrint;
}

int Bigint::bits() const
{
        Bstream temp;
	temp = Bigint_to_Bstr(*this);
        int n;
        byte s;
        do {
                temp.peekbyte(s);
                if (s == 0)
                        temp.consume(1);
        } while (s == 0);

        return (temp.getlength()*8);
}

// Converts Bigint to a unsigned char *, returning number of bytes in bigint
int convert_Bigint_to_bytes(const Bigint b, int n, unsigned char *buf)
{
	Bstream bstr;
	byte s;
	int diff;
	bstr = Bigint_to_Bstr(b);
// strip off leading zeros
	do {
		bstr.peekbyte(s);
		if (s == 0)
			bstr.consume(1);
	} while (s == 0);

	bzero(buf, n);
	diff = n - bstr.getlength();
	bcopy(bstr.getdatap(), buf+diff, bstr.getlength());
	return (bstr.getlength());
}	

Bstream
Bigint_to_Bstr(const Bigint& val)
{
//	G++ will produce nasty memory bugs with this line
//	const char *inthexstr = (const char *)val.getnumstr();

	String hexstr=val.getnumstr();
	const char *inthexstr = (const char *)hexstr;
	

        while (*inthexstr == '0' )
                inthexstr++;

	int len = strlen(inthexstr);
	Boolean isodd = BOOL_FALSE;
	if (len & 0x01)	{	// odd number of digits
		len++;
		isodd = BOOL_TRUE;
	}
	int intlen = len/2;
	byte *intstr = new byte[intlen]; // a hex digit takes half an octet
	bzero(intstr, intlen);

	for (int i=0, j=0; i < intlen; i++) {
		byte hinib, lonib;
		if (isodd == BOOL_TRUE && i == 0) {
			hinib = 0x0;
			lonib = atob((char )inthexstr[j]);
			j += 1;
		} else {
			hinib = atob((char )inthexstr[j]);
			lonib = atob((char )inthexstr[j+1]);
			j += 2;
		}
		intstr[i] = (hinib << 4) | lonib;
	}
	Bstream res = Bstream(intlen, intstr);
	delete intstr;
	return res;
}
