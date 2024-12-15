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

#pragma ident "@(#)X509Cert.h	1.12 96/01/29 Sun Microsystems"

#ifndef CERT_H
#define CERT_H

#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"


/*
 * Abstract Data Type for
 * an X.509/PEM style certificate
 */

enum VerifyResult {VALID, 		// Valid X509Certificate
		   INVALID_SIG,         // Signature invalid
		   INVALID_REVOKED, 	// X509Certificate revoked
		   INVALID_NOTBEFORE, 	// X509Certificate not yet valid
		   INVALID_EXPIRED};	// X509Certificate validity expired

struct AlgId {
	ObjId	algid;
	Bstream params;
	AlgId& operator =(const AlgId&);
};

struct Key {
	AlgId		keytype;
	Bstream		key;
};

typedef Key	PubKey;
typedef Key 	PrivKey;

class X509Cert {
	Bigint		serialnum;
	AlgId		sigalg;
	Name		issuer;
	PCTime		notbefore;
	PCTime		notafter;
	Name		subject;
	PubKey		pubkey;
	Bstream		certdata;	// Entire contents of encoded cert
	Bstream		certsigdata;	// Part over which sig. is computed
	Bstream		sigdata;	// The encrypted hash, i.e sig value
public:
	 // Constructors 
	X509Cert();
	~X509Cert();
	X509Cert(const X509Cert& );
	X509Cert& operator =(const X509Cert &);
	X509Cert(const Bigint&, const AlgId&, const Name&,
	     const PCTime& , const PCTime&, const Name&, const PubKey&);
	X509Cert(const Bigint&, const AlgId&, const Name&,
	     const PCTime& , const PCTime&, const Name&, const PubKey&,
	     const Bstream&, const Bstream&, const Bstream&);
	// Member functions and operators
	friend Boolean operator ==(const X509Cert&, const X509Cert&);
	VerifyResult verify(const PubKey& ) const;
	Bstream sign_and_encode(const Bstream&);
	Bigint getserialnum() const;
	Name getsubject() const;
	Name getissuer() const;
	AlgId getsigalgid() const;
	PCTime getnotbefore() const;
	PCTime getnotafter() const;
	PubKey getpubkey() const;
	Bstream getcertdata() const;
	void print();
};

extern int asn1_der_decode_cert(Bstream&, X509Cert&);
extern int asn1_der_decode_algid(Bstream&, AlgId&);
extern Bstream asn1_der_encode_certreq(const char *, const Bstream&,
					const Bstream&);
extern int asn1_der_decode_dh_params(Bstream, Bigint&, Bigint&);

#endif CERT_H
