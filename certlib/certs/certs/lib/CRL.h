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

#pragma ident "@(#)CRL.h	1.5 96/01/29 Sun Microsystems"

#ifndef RW
#define CRL_H
#endif

#ifndef CRL_H
#define CRL_H

#include "Signed.h"

extern Bstream initial_CRL(const Bstream&, const char*, int);

#ifdef RW
struct CRLentry : public ListElem {
	Bigint		serialnum;
	PCTime		revocationdate;
	friend Boolean operator ==(const CRLentry&, const CRLentry&);
	virtual Boolean isEqual(const RWCollectable* t) const {
		return (*this == *(CRLentry *)t);
	}
};
#else
#ifdef GLIB
struct CRLentry : public ListElem {
	Bigint		serialnum;
	PCTime		revocationdate;
	friend Boolean operator ==(const CRLentry&, const CRLentry&);
	virtual Boolean isEqual(const RWCollectable* t) const {
		return (*this == *(CRLentry *)t);
	}
};
#endif
#endif

class CRL : public virtual Signed {	// Share base class data with children
 protected:
	Slist 			revokedlist;
	AlgId			sigalg;
	Name			issuer;
	PCTime			lastUpdate;
	PCTime			nextUpdate;
 public:
	// Constructors & Destructors
	CRL();
	virtual ~CRL();
	CRL(const CRL&);
	CRL& operator =(const CRL&);
	CRL(const Bstream&);		// Implicitly decode a CRL
	CRL(const AlgId&, const Name&, const PCTime&, const PCTime&,
	    const Slist& );
	// Member functions and operators
	Bstream encode();		// encode CRL-specific information
	virtual void setRevokedList(const PCTime&, const PCTime&, 
				    const Slist&); // Change the revokedlist
	virtual AlgId getSigAlg() const;
	virtual PCTime getLastUpdate() const;
	virtual PCTime getNextUpdate() const;
	virtual Name getIssuer() const;
	virtual Slist getAllRevoked() const;
	Boolean  isRevoked(const Bigint &) const;
	friend int asn1_der_decode_CRL(Bstream&, CRL&);
	void print() const;
};

extern Bstream 
asn1_der_encode_CRL(CRL&, const AlgId&, const Bstream&);

#define CRLS_DIR	"crls"
#define	CRL_SUFFIX	".crl"

#endif CRL_H
