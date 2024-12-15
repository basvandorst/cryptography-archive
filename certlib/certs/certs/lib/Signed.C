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

#pragma ident "@(#)Signed.C	1.6 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include "my_types.h"
#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"
#include "X509Cert.h"
#include "malloc.h"
#include "Sig.h"
#include "Signed.h"

/*
 * Signed class implementation.
 */

Signed::Signed()
{
	// Rely on private member constructors	
}

Signed::~Signed()
{
	// Rely on private member destructors	
}

Signed::Signed(const Signed &a)
{
	signature = a.signature;
	sigalgid = a.sigalgid;
	input = a.input;
	encoded_signed = a.encoded_signed;
	return;
}

Signed&
Signed:: operator =(const Signed &a)
{
	signature = a.signature;
	sigalgid = a.sigalgid;
	input = a.input;
	encoded_signed = a.encoded_signed;
	return (*this);
}

// Supply encoded SIGNED object. This will decode it into
// constituent parts.

Signed::Signed(const Bstream& ainput)
{
	Bstream der_stream = ainput;
	encoded_signed = ainput;	// The full encoded/signed object
	// Decode to get the rest of the fields
	(void)asn1_der_decode_signed(der_stream, signature, sigalgid, input);
	return;
}

// Supply encoded SIGNED object. This will decode it into
// constituent parts.

int
Signed::decode(const Bstream& ainput)
{
	Bstream der_stream = ainput;
	encoded_signed = ainput;	// The full encoded/signed object
	// Decode to get the rest of the fields
	return(asn1_der_decode_signed(der_stream, signature, sigalgid, input));
}

int
asn1_der_decode_signed(Bstream& der_stream, Bstream& signature, 
		       AlgId& sigalgid, Bstream& input)
{
	int retval, seqlen, len;
	byte tmp;

	// Decode an X.509 SIGNED object

	SEQUENCE
	{
		input = der_stream;		// Will be adjusted
		(void)der_stream.fetchbyte(tmp);	// skip past ID octet
 		// length of (unknown) type
		len = asn1_der_get_length(der_stream); 
		if (len < 0 || len > der_stream.getlength()) {
		    fprintf(stderr, 
			   "Bad ASN.1 encoding of signed object ToBeSigned\n");
		    return (E_DER_DECODE_LEN_TOO_LONG);
		}
		der_stream.consume(len);	// skip past unknown object
		int trunclen = der_stream.getlength();
		input.truncate(trunclen);	// Adjust input
		SEQUENCE			// AlgId
		{
			int begin, end, used, anylen;
			MARK_LEN(begin);
			OBJECT_IDENTIFIER(sigalgid.algid);
			MARK_LEN(end);
			used = begin - end;
			if ((anylen = seqlen - used) > 0) {
				Bstream any(anylen, DATAP);
				CONSUME(anylen);
				sigalgid.params = any;
			}
		}
		//  encrypted hash, i.e SIGNATURE
		BIT_STRING(signature);
	}


}

Bstream
Signed::sign(const Bstream& tobesigned, const Bstream& privkey, 
	     const AlgId& asigalgid)

{
	sigalgid = asigalgid;
	input = tobesigned;
	signature = ::sign(input, privkey, sigalgid.algid);
	encoded_signed = asn1_der_encode_signed(input, sigalgid, signature);
	return (encoded_signed);
}

VerifyResult
Signed::verify(const PubKey& pubkey) const
{
#ifdef temporary
	return (::verify(input, signature, pubkey, sigalgid));
#endif
	return VALID;
}

Bstream
asn1_der_encode_signed(const Bstream& input, const AlgId& sigalgid, 
		       const Bstream& sig)
{
	Bstream tmpstr;

	tmpstr = asn1_der_encode_null();
	tmpstr = sigalgid.algid.encode() + tmpstr;
	Bstream algid = asn1_der_encode_sequence(tmpstr);	// Alg ID
	Bstream enc_sig =  asn1_der_encode_bit_string(sig);
	return (asn1_der_encode_sequence(input+algid+enc_sig));
}

Bstream
Signed::getsignature() const
{
	return (signature);
}

Bstream
Signed::getinput() const
{
	return (input);
}

Bstream
Signed::getSigned() const
{
	return (encoded_signed);
}

AlgId
Signed::getsigalgid() const
{
	return (sigalgid);
}


