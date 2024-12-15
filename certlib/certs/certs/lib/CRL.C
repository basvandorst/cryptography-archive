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

#pragma ident "@(#)CRL.C	1.10 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef RW
#include <rw/slistcol.h>
#else
#ifdef GLIB
#include <SLList.h>
#include <Pix.h>
#endif
#endif

#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"
#include "X509Cert.h"
#include "Sig.h"
#include "CRL.h"


#ifdef RW
// XXX Convert the following to use CRL objects

Bstream
initial_CRL(const Bstream& privkey, const char *issuername, int months)
{
	Bstream tmpstr, tobesigned, signature, algid, nullbstr;
	Name nullname;
	Name issuer(issuername);

	if (issuer == nullname) {
		fprintf(stderr, "Issuer name '%s' syntax incorrect.\n");
		return (nullbstr);
	}
	tmpstr = asn1_der_encode_null();
// XXX - need to add support for different signature algs 
//	tmpstr = md2WithRSAEncryption.encode() + tmpstr;

	tmpstr = dsaWithSHA.encode() + tmpstr;
	tmpstr = algid = asn1_der_encode_sequence(tmpstr);	// Alg ID

	tmpstr = tmpstr + issuer.encode();
	PCTime lastUpdate = timenow();
	GMtime gtime = lastUpdate.get();
	{
		gtime.year += months / 12;
		gtime.month += months % 12; // 0 - 11 months
		if (gtime.month > 12) {
			gtime.year += gtime.month / 12;
			gtime.month = gtime.month % 12; // 1 - 11 months
		}
	}
	PCTime nextUpdate(gtime);
	tmpstr = tmpstr + asn1_der_encode_utctime(lastUpdate);
	tmpstr = tmpstr + asn1_der_encode_utctime(nextUpdate);

	// Initially there are no revoked certificates, so we
	// leave out the optional CRLEntry list.

	tobesigned = asn1_der_encode_sequence(tmpstr);
// XXX - need to add support for different signature algs 
//	tmpstr = sign(tobesigned, privkey, md2WithRSAEncryption);
	tmpstr = sign(tobesigned, privkey, dsaWithSHA);
	signature = asn1_der_encode_bit_string(tmpstr);
	tmpstr = tobesigned + algid + signature;
	return (asn1_der_encode_sequence(tmpstr));
}

int
asn1_der_decode_CRL(Bstream& der_stream, CRL& crl)
{
	byte tmp = 0;
	int  seqlen, seqoflen, retval;

	crl.decode(der_stream);		// Decode a generic SIGNED object
	der_stream = crl.getinput();	// Get ToBeSigned
	// Decode CRL specific data (ToBeSigned)
	SEQUENCE {
		int begin, end, used;
		MARK_LEN(begin);
		ALGID(crl.sigalg);
		NAME(crl.issuer);
		UTCTime(crl.lastUpdate);
		UTCTime(crl.nextUpdate);
		// walk down sequence-of
		// sequence-of is optional
		// check for presence
		MARK_LEN(end);
		used = begin - end;
		if (used < seqlen) {	// we have revoked certificates
			int seqlenwhdr, consumed;
			CRLentry *crlentry;
			Bigint serialNumber;
			PCTime   revocationDate;

			BEGIN_SEQUENCE_OF
				SEQUENCE {

					INTEGER(serialNumber);
					UTCTime(revocationDate);
				}
				crlentry = new CRLentry;
				crlentry->serialnum = serialNumber;
				crlentry->revocationdate = revocationDate;
				crl.revokedlist.insert(crlentry);
			END_SEQUENCE_OF
		}
	}
	return (SUCCESS);
}

// CRLentry friend function

Boolean
operator ==(const CRLentry& a, const CRLentry& b)
{
	if (a.serialnum == b.serialnum)
		return (TRUE);
	else
		return (FALSE);

}

/*
 * CRL class implementation.
 */

CRL::CRL()
{
	// Private member constructors
}

CRL::~CRL()
{
	// Private member destructors

}

CRL::CRL(const CRL &a)
{
	sigalg = a.sigalg;
	issuer = a.issuer;
	lastUpdate = a.lastUpdate;
	nextUpdate = a.nextUpdate;
	revokedlist = a.revokedlist;
	return;
}

CRL&
CRL:: operator =(const CRL &a)
{
	sigalg = a.sigalg;
	issuer = a.issuer;
	lastUpdate = a.lastUpdate;
	nextUpdate = a.nextUpdate;
	revokedlist = a.revokedlist;
	return (*this);
}

// Supply encoded CRL object. This will decode it into
// constituent parts.

CRL::CRL(const Bstream& ainput)
{
	Bstream der_stream = ainput;
	// Decode to get the rest of the fields
	(void)asn1_der_decode_CRL(der_stream, *this);
	return;
}

// Initialize a CRL from its input parts.
// Still need to do a sign on the base object
// in order to get an encoded CRL.

CRL::CRL(const AlgId& algid, const Name& aissuername, const PCTime& alast,
	 const PCTime& anext, const Slist& arevokedlist)
{
	sigalg = algid;
	issuer = aissuername;
	lastUpdate = alast;
	nextUpdate = anext;
	revokedlist = arevokedlist;
	return;
}

// Each derived class of Signed:: contains a routine to
// encode the derived class specific information. This
// routine does this for a CRL.

Bstream
CRL::encode()
{
	Bstream tmpstr;

	tmpstr = asn1_der_encode_null();	// XXX Assume NULL params
	tmpstr = sigalg.algid.encode() + tmpstr;
	tmpstr = asn1_der_encode_sequence(tmpstr);	// Alg ID

	tmpstr = tmpstr + issuer.encode();
	tmpstr = tmpstr + asn1_der_encode_utctime(lastUpdate);
	tmpstr = tmpstr + asn1_der_encode_utctime(nextUpdate);

	// Encode the CRL entries

	CRLentry	*next;
	SlistIterator iter(revokedlist);
	Bstream seq, seqof;

	while (next = (CRLentry *)iter()) {
		seq = asn1_der_encode_integer(next->serialnum);
		seq = seq + asn1_der_encode_utctime(next->revocationdate);
		seq = asn1_der_encode_sequence(seq);
		seqof = seqof + seq;
	}
	if (seqof.getlength() > 0) {
		seqof = asn1_der_encode_sequence_of(seqof);	
		tmpstr = tmpstr + seqof;
	}
	return (asn1_der_encode_sequence(tmpstr));
}

void
CRL::setRevokedList(const PCTime& last, const PCTime& next, const Slist& arl)
{
	lastUpdate = last;
	nextUpdate = next;
	revokedlist = arl;
}

AlgId
CRL::getSigAlg() const
{
	return (sigalg);
}

PCTime
CRL::getLastUpdate() const
{
	return (lastUpdate);
}

PCTime
CRL::getNextUpdate() const
{
	return (nextUpdate);
}

Name
CRL::getIssuer() const
{
	return (issuer);
}

Slist
CRL::getAllRevoked() const
{
	return (revokedlist);
}


Boolean
CRL::isRevoked(const Bigint& aserialnumber) const
{
	CRLentry dummy;
	dummy.serialnum = aserialnumber;
	CRLentry *found = (CRLentry *)revokedlist.find(&dummy);
	if (found == NULL)
		return (BOOL_FALSE);
	else 
		return (BOOL_TRUE);
}

// print the contents of the CRL
void
CRL::print() const
{
	printf("Issuer = "); issuer.print(); printf("\n");
	printf("Last Update = "); lastUpdate.print(); printf("\n");
	printf("Next Update = "); nextUpdate.print(); printf("\n");
	printf("CRL Signature Algorithm Identifier = \n"); 
	sigalg.algid.print();
	if (sigalg.algid == md2WithRSAEncryption)
		printf(" == MD2-WITH-RSA-ENCRYPTION");
	else if (sigalg.algid == dsaWithSHA)
		printf(" == SHA-WITH-DSA-SIGNATURE");
	printf("\n");
	printf("Revoked Certificates =\n");
	CRLentry *next;
#ifdef RW
	Slist list = revokedlist;
	SlistIterator iter(list);

	while (next = (CRLentry *)iter()) {
		printf("Serial # "); next->serialnum.print(); printf(" ");
		printf("Revocation Date = "); next->revocationdate.print(); 
		printf("\n");
	}
#endif
}

// Encode a CRL which has been supplied its input 
// parts. First use member function to get encoding 
// of ToBeSigned, and then sign and compute SIGNED
// encoding of object. Supply tobesigned, to base
// class Signed::sign() member function.

Bstream
asn1_der_encode_CRL(CRL& crl, const AlgId& sigalg, const Bstream& privkey)
{
	Bstream tobesigned = crl.encode();
	return (crl.sign(tobesigned, privkey, sigalg));
}

#endif
