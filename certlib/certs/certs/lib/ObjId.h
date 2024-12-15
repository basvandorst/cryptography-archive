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

#pragma ident "@(#)ObjId.h	1.9 96/01/29 Sun Microsystems"


#ifndef OBJID_H
#define OBJID_H

/*
 * Object ID class
 */
struct obj_id_elem {
	obj_id_elem	*next;
	Bigint		elemval;
};

class ObjId {
	obj_id_elem	*obj_id_list;
  public:
	// Constructors etc
	ObjId() {obj_id_list = NULL;};
#ifndef __GNUC__
	ObjId(int , ...);	// variable # of Bigints, 1st arg gives #
#else
       ObjId(int , Bigint=Bigint((short)0), Bigint=Bigint((short)0), 
                   Bigint=Bigint((short)0), Bigint=Bigint((short)0), 
                   Bigint=Bigint((short)0), Bigint=Bigint((short)0),
                   Bigint=Bigint((short)0), Bigint=Bigint((short)0), 
                   Bigint=Bigint((short)0), Bigint=Bigint((short)0), 
                   Bigint=Bigint((short)0), Bigint=Bigint((short)0));
#endif
	~ObjId();
	ObjId(const ObjId &);
	// operators and member functions
	ObjId& operator =(const ObjId &);
	void add_element(const Bigint& );
	Bstream encode() const;
	friend Boolean operator ==(const ObjId&, const ObjId&);
	friend Boolean operator !=(const ObjId&, const ObjId&);
	void print() const;
	String getoidstr() const;
};

/*
 * selected OIDs
 */

// From X.520

const ObjId commonName	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(3));
const ObjId surName	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(4));
const ObjId countryName	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(6));
const ObjId localityName(4, Bigint(2), Bigint(5), Bigint(4), Bigint(7));
const ObjId stateName	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(8));
const ObjId streetAddress(4, Bigint(2), Bigint(5), Bigint(4), Bigint(9));
const ObjId orgName	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(10));
const ObjId orgUnitName	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(11));
const ObjId title	(4, Bigint(2), Bigint(5), Bigint(4), Bigint(12));

// From X.509

const ObjId rsa		
	(5,	Bigint(2), Bigint(5), Bigint(8), Bigint(1), Bigint(1));
const ObjId sqMod_n
	(5,	Bigint(2), Bigint(5), Bigint(8), Bigint(2), Bigint(1));
const ObjId sqMod_nWithRSA 
	(5,	Bigint(2), Bigint(5), Bigint(8), Bigint(3), Bigint(1));

// From 1994 Open Systems Environment Implementor's Workshop (OIW)
// message digest algorithms
const ObjId md2
	(6,	Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(2), 
	   	Bigint(2));
const ObjId md5
	(6,	Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(2), 
		Bigint(5));
const ObjId sha
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(18));
const ObjId rsaEncryption
	(7,	Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1), 
	   	Bigint(1), Bigint(1));
const ObjId desECB
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(6));
const ObjId desCBC
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(7));

const ObjId dsa
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(12));
const ObjId dsaWithSHA
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(13));
const ObjId dsaCommon
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(20));
const ObjId dsaCommonWithSHA
	(6,	Bigint(1), Bigint(3), Bigint(14), Bigint(3), Bigint(2),
		Bigint(21));

const ObjId md2WithRSAEncryption
	(7, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1), 
	    Bigint(1), Bigint(2));
const ObjId md5WithRSAEncryption
	(7, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1), 
	    Bigint(1), Bigint(4));

// From PKCS #3
const ObjId pkcs_3
	(6, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1),
	    Bigint(3));
const ObjId dhKeyAgreement
	(7, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1),
	    Bigint(3), Bigint(1));

// XXXX -- SunScreen v1 definition of dhKeyAgreement
const ObjId sundhKeyAgreement
	(6, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1),
	    Bigint(3));

// From PKCS #5
const ObjId pbeWithMD2AndDES_CBC
	(7, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1), 
	    Bigint(5), Bigint(1));
const ObjId pbeWithMD5AndDES_CBC
	(7, Bigint(1), Bigint(2), Bigint(840), Bigint("1bb8d"), Bigint(1), 
	    Bigint(5), Bigint(3));

// From Sun OID Space
const ObjId ipAddress
	(11,	Bigint(1), Bigint(3), Bigint(6), Bigint(1), Bigint(4), 
	    	Bigint(1), Bigint(42), Bigint(2), Bigint(11), Bigint(2),
		Bigint(1));

/*
 * Macro to decode OIDs
 */

#define	OBJECT_IDENTIFIER(oid)  				      \
	{						      	      \
		retval = asn1_der_decode_obj_id(der_stream, oid);     \
	    	if (retval < 0) { 				      \
			/* printf("OBJECT-ID decode:"); */	      \
			return (retval);			      \
	    	}						      \
   	}

// C++ linkage

extern int asn1_der_decode_obj_id(Bstream&, ObjId&);
extern Bstream asn1_der_encode_obj_id(const ObjId&);

#endif OBJID_H
