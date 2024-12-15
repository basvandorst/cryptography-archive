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

#pragma ident "@(#)Name.h	1.6 96/01/29 Sun Microsystems"

#ifndef NAME_H
#define NAME_H

/*
 * Name class; Handle X.500 Distinguished Names
 */

// Attribute-Value-Assertion
 
struct Ava {
	Ava	*next;
	ObjId	attributetype;
	Bstream	attributevalue;
};

class Name {
	Ava	*avalist;
  public:
	// Constructors etc
	Name() {avalist = NULL;};
	Name(int , ...);	// variable # of AVAs, 1st arg gives #
	Name(const char *);		// String like /C=US/O=Sun/OU=SMCC etc.
	~Name();
	Name(const Name &);
	// operators and member functions
	Bstream encode();
	Name& operator =(const Name &);
	void add_element(const Ava& );
	String getip() const;		// Get IP Address
	String getcn() const;		// Get Common Name
	String getou() const;		// Get Organizational Unit Name
	String getorg() const;		// Get Organization Name
	String getstate() const;	// Get State or Province Name
	String getcountry() const;	// Get Country Name
	String getDN() const;		// Get the full DN as "/C=US/S=etc."
	String getsuffix() const;	// Get the last AVA as "/OU=Sun" etc.
	friend Boolean operator ==(const Name&, const Name&);
	friend Boolean operator !=(const Name&, const Name&);
	void print() const;
};
/*
 * Macro to decode an ASN.1 BER encoded name
 */
#define	NAME(name)  				      		      \
	{						      	      \
		retval = asn1_der_decode_name(der_stream, name);      \
	    	if (retval < 0) { 				      \
			/* printf("Name decode:"); */	      	      \
			return (retval);			      \
	    	}						      \
   	}

#define CON	"/C="
#define ORG	"/O="
#define LOC	"/L="
#define STA	"/S="
#define OGU	"/OU="
#define TIT	"/TI="
#define CNM	"/CN="
#define IPA	"/IP="

/*
 * C++ linkage
 */
extern int asn1_der_decode_name(Bstream&, Name&);

#endif NAME_H
