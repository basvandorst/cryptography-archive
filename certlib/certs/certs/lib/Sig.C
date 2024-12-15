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

#pragma ident "@(#)Sig.C	1.12 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"
#include "X509Cert.h"

#include "Sig.h"
#include "md2.h"
#include "md5.h"
#include "sha.h"
#include "dsa.h"

VerifyResult
static RSA_verify(const Bstream&, const Bstream&, const PubKey&, AlgId);

/* Note: This is basically a stub library.  It won't do RSA Signatures  */

Bstream
messageDigest(const Bstream& in, const ObjId& sigoid)
{
	int status;
	byte digest[20];
	unsigned int digestLen;

	if (sigoid == md2WithRSAEncryption || sigoid == md2) {
// we use the public domain implementation of MD2
		xMD2Context context;

		xMD2Init(&context);
		xMD2Update(&context,in.getdatap(), in.getlength());
		xMD2Final(&context, digest);
		digestLen = 16;
	} else if (sigoid == md5WithRSAEncryption || sigoid == md5) {
// we use the public domain implementation of MD5
		xMD5Context context;

		xMD5Init(&context);
		xMD5Update(&context, in.getdatap(), in.getlength());
		xMD5Final(digest, &context);
		digestLen = 16;
	} else if (sigoid == dsaWithSHA || sigoid == sha) {
// we use the public domain implementation of SHA
		struct SHAContext context;

		shaInit(&context);
		shaUpdate(&context, in.getdatap(), in.getlength());
		shaFinal(&context, digest);
		digestLen = 20;
	} else {
		Bstream nullbstr;
		fprintf(stderr,
			"messageDigest: unrecognized signature algorithm\n");
		return (nullbstr);
	}
	Bstream hash(digestLen, digest);
	return (hash);
}

int decode_privatekey(const Bstream &privkeystr, void *privateKey)
{

	Bstream der_stream = privkeystr;
	int seqlen;
	Bigint modulus,pubexp,privexp, prime1,prime2, exponent1,
	       exponent2, coefficient;
	int retval;

	retval = asn1_der_decode_sequence(der_stream, seqlen);
	if (retval < 0) { 
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
       	retval = asn1_der_decode_integer(der_stream, modulus);

	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, pubexp);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, privexp);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, prime1);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, prime2);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, exponent1);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, exponent2);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
 	retval = asn1_der_decode_integer(der_stream, coefficient);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
	/* Fill in the struct with these values and return it*/
	return 0;
}	

// Sign binary string input, using ASN.1 encoded private key, (PKCS #1
// compliant) privkey, using signature algorithm sigoid.
Bstream
sign(const Bstream& tobesigned, const Bstream& privkeystr, const ObjId& sigoid)
{
	int status = 0;
	unsigned int signedInfoLen;
	byte signedInfo[256];
	int sigalgtype;
	Bstream nullbstr, signature;
	Bigint p, q, g, x;

	
	// decode the private key
	if (sigoid == md2WithRSAEncryption || sigoid == md5WithRSAEncryption) {

	} else if (sigoid == dsaWithSHA) {
		Bstream der_stream = privkeystr;
		status = asn1_der_decode_dsa_privkey(der_stream, p, q, g, x);
		if (status < 0) {
			fprintf(stderr,
				"sign: error decoding DSA private key\n");
			return (nullbstr);
		}
	} else {
		fprintf(stderr, "sign: unrecognized signature algorithm\n");
		return (nullbstr);
	}

	if (sigoid == md2WithRSAEncryption || sigoid == md5WithRSAEncryption) {
		printf("Won't sign\n");
	}
	if (sigoid == dsaWithSHA) {
		signature = DSA_sign(p, q, g, x, tobesigned);
	}
	return (signature);
}

VerifyResult
verify_sig(const Bstream& data, const Bstream& sig, const PubKey& pubkey, 
	   AlgId sigalg)
{
	if (sigalg.algid == md2WithRSAEncryption || 
	    sigalg.algid == md5WithRSAEncryption) 
		return RSA_verify(data, sig, pubkey, sigalg);
	else if (sigalg.algid == dsaWithSHA)
		return DSA_verify(data, sig, pubkey, sigalg);
	fprintf(stderr, "Unrecognized Signature Algorithm ID\n");
	return INVALID_SIG;
}

static VerifyResult
RSA_verify(const Bstream& data, const Bstream& sig, const PubKey& pubkey, 
	   AlgId sigalg)
{
	int alg;
	int n;
	int retval, seqlen;
	Bigint modulus, exponent;

	Bstream der_stream = pubkey.key;
	retval = asn1_der_decode_sequence(der_stream, seqlen);
	if (retval < 0) { 
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
	retval = asn1_der_decode_integer(der_stream, modulus);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
	retval = asn1_der_decode_integer(der_stream, exponent);
	if (retval < 0) {
		printf("Bad Key encoding\n"); 
		return INVALID_SIG;
	}
	fprintf(stderr,"WARNING: Certificate NOT Verified - Presumed Correct\n");
	n=0;   // We (errorneously) assume that the verification succeded
	if (n)
		return INVALID_SIG;
	else
		return VALID;
}

