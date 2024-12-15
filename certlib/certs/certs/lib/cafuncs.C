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

#pragma ident "@(#)cafuncs.C	1.14 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"
#include "X509Cert.h"
#include "Sig.h"

#include "ca.h"
#include "utils.h"

int
get_certreq_params(const Bstream& certreq, Name& name, PubKey& pubkey)
{
	int retval, seqlen;
	PubKey psubject;
	AlgId sigalgid;
	Name subname;
	Bstream sigdata, signature;
	Bigint version;
	VerifyResult result = INVALID_SIG;

	Bstream der_stream = certreq;
	SEQUENCE
	{
		sigdata = der_stream;
		SEQUENCE	// X509CertReqInfo --- ToBeSigned
		{
			INTEGER(version);
			NAME(subname);
			SEQUENCE // Subject Public Key info
			{
				SEQUENCE // Subject Public Key Alg ID
				{
					int begin, end, used, anylen;
					MARK_LEN(begin);
				     OBJECT_IDENTIFIER(psubject.keytype.algid);
					MARK_LEN(end);
					used = begin - end;
					if ((anylen = seqlen - used) > 0) {
						Bstream any(anylen, DATAP);
						CONSUME(anylen);
						psubject.keytype.params = any;
					}

				}
				//  Subject Public Key
				BIT_STRING(psubject.key);
			}
		}
		int tobesignedlen = sigdata.getlength() - 
						der_stream.getlength();
		int trunclen = sigdata.getlength() - tobesignedlen;
		sigdata.truncate(trunclen);
		SEQUENCE	// Alg ID of certreq signing
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
	// Verify the X509Certificate request. This is done
	// by using the supplied public key to verify
	// the signature, since certificate requests are
	// self-signed.
	// Unfortunately, Diffie-Hellman X.509 certificate requests
	// are not self-signed ...
	if (psubject.keytype.algid == dhKeyAgreement) {
		pubkey = psubject;
		name = subname;
		result = VALID;
	} else {
		result = verify_sig(sigdata, signature, psubject, sigalgid);
		if (result == VALID) {
			pubkey = psubject;
			name = subname;
		} else {
			fprintf(stderr,
				"Signature on certificate request invalid\n");
		}
	}
	return ((int)result);
}

// Get the serial number for the certificate being issued.
// Increments the value of serialnum stored in NEXT_SERIALNUM_FILE
// to return the present value + 1, for the next certificate
// to be issued.

Bigint
getSerialnum()
{
	Bstream serialstr = File_to_Bstr(get_byzantine_dir() + NEXT_SERIALNUM_FILE);
	Bigint serialnum(serialstr.getdatap(), serialstr.getlength());
	if (serialnum == (short)0)
		serialnum = serialnum + 1;
	serialstr = Bigint_to_Bstr(serialnum + 1);
	serialstr.store(get_byzantine_dir() + NEXT_SERIALNUM_FILE);
	return (serialnum);
}

X509Cert
getX509CertbySerialNumber(const Bigint& serialnum)
{
	X509Cert cert;
	Bstream nullbstr;
	String certfilename = (String)CERT_DATABASE_DIR + "/" + 
				serialnum.getnumstr();
	String certpath = get_byzantine_dir() + certfilename;
	Bstream certstr = File_to_Bstr(certpath);

	if (certstr == nullbstr) {
		fprintf(stderr, "Unable to read certificate file '%s'.\n",
			(const char *)certpath);
		return (cert);
	}
	int retval = asn1_der_decode_cert(certstr, cert);
	if (retval < 0) {
		fprintf(stderr, "Unable to get certificate for serial number ");
		serialnum.print();
		fprintf(stderr, "\n");
		asn1_perror(retval);
	}
	return (cert);
}

X509Cert
getCAcert()
{
	X509Cert cert;
	Bstream nullstr;
	String certfile = get_byzantine_dir() + CA_CERT_FILE;
	Bstream certstr = File_to_Bstr(certfile);

	if (certstr == nullstr) {
		fprintf(stderr, "No contents in CA self certificate file %s\n",
			(const char *)certfile);
		return (cert);
	}
	int retval = asn1_der_decode_cert(certstr, cert);

	if (retval < 0) {
		fprintf(stderr, "Unable to decode CA certificate in file %s\n",
			(const char *)certfile);
		asn1_perror(retval);
	}
	return (cert);
}

Boolean
allow_certification(Name& subject, PubKey& pub_subject)
{
	// get the current serial number
	Bstream serialstr = File_to_Bstr(get_byzantine_dir() + NEXT_SERIALNUM_FILE);
	Bigint cur_serialnum(serialstr.getdatap(), serialstr.getlength());

	X509Cert cert;
	Bstream certstr, nullbstr;
	String certdir = get_byzantine_dir() + (String)CERT_DATABASE_DIR + "/";
	PubKey pubkey;
	int retval;

	for (Bigint i = 1; i < cur_serialnum; i = i + 1) {
		certstr = File_to_Bstr(certdir + i.getnumstr());
		retval = asn1_der_decode_cert(certstr, cert);
		if (retval < 0) {
			asn1_perror(retval);
			fprintf(stderr, "skipping .... \n");
			continue;
		}
		// check key collisions
		pubkey = cert.getpubkey();
		if (pubkey.key == pub_subject.key) {
			fprintf(stderr, "Public key collision!!\n");
			return(BOOL_FALSE);
		}

		// now check to make sure the subject does not already have
		// a key with the same parameters
		if (cert.getsubject() == subject) {
			// found a certificate for this subject
			// now check the params => should be different
			if (pubkey.keytype.algid == pub_subject.keytype.algid) {
				if ((pubkey.keytype.params ==
					pub_subject.keytype.params)
// XXX check for different exponent lengths
//						|| ()
					) {
					fprintf(stderr,
	"Subject already has a public value with same DH params\n");
					return(BOOL_FALSE);
				}
			}
		}
	}
	return (BOOL_TRUE);
}
