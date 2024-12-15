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

#pragma ident "@(#)Signed.h	1.5 96/01/29 Sun Microsystems"

#ifndef SIGNED_H
#define SIGNED_H

#include "X509Cert.h"

// Base class for deriving objects that are digitally signed
// and encoded in ASN.1 according to the X.509 SIGNED macro.
// "Pure" base class; cannot be instantiated.

class Signed {
 protected:
	Bstream signature;
	AlgId	sigalgid;
	Bstream input;
	Bstream encoded_signed;	// ASN.1 encoding of an X.509 SIGNED object.
				// This contains the signature, input and 
				// sigalgid per the SIGNED macro.
 public:
	// Constructors & Destructors
	Signed();
	Signed(const Bstream&);			// Supply a signed object
	virtual ~Signed();
	Signed(const Signed& );
	Signed& operator =(const Signed &);
	// Member functions and operators
	virtual decode(const Bstream&);		// Supply a signed object
	virtual Bstream sign(const Bstream&,  // tobesigned, key and algid.
		       const Bstream&, const AlgId&);
						   // returns encoded_signed
	virtual VerifyResult verify(const PubKey&) const;
	virtual Bstream getsignature() const;
	virtual Bstream getinput() const;
	virtual Bstream getSigned() const;
	virtual AlgId getsigalgid() const;
	virtual Bstream encode() = 0;		// derived-class specific
};

extern Bstream
asn1_der_encode_signed(const Bstream&, const AlgId&, const Bstream&);

extern int
asn1_der_decode_signed(Bstream&, Bstream&, AlgId&, Bstream&);

#endif SIGNED_H
