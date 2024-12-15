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
#pragma ident "%w% 96/01/29 Sun Microsystems"

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
#include "dsa.h"
#include "utils.h"

AlgId&
AlgId::operator =(const AlgId& a)
{
	algid = a.algid;
	params = a.params;
	return (*this);
}

/*
 * X509Cert class implementation.
 */

X509Cert::X509Cert()
{
	// Rely on private member constructors	
}

X509Cert::~X509Cert()
{
	// Rely on private member destructors	
}

X509Cert::X509Cert(const X509Cert &a)
{
	serialnum = a.serialnum;
	sigalg = a.sigalg;
	issuer = a.issuer;
	notbefore = a.notbefore;
	notafter = a.notafter;
	subject = a.subject;
	pubkey = a.pubkey;
	certdata = a.certdata;
	certsigdata = a.certsigdata;
	sigdata = a.sigdata;
	return;
}

X509Cert&
X509Cert:: operator =(const X509Cert &a)
{
	serialnum = a.serialnum;
	sigalg = a.sigalg;
	issuer = a.issuer;
	notbefore = a.notbefore;
	notafter = a.notafter;
	subject = a.subject;
	pubkey = a.pubkey;
	certdata = a.certdata;
	certsigdata = a.certsigdata;
	sigdata = a.sigdata;
	return (*this);
}

X509Cert::X509Cert(const Bigint& ser, const AlgId& sigalgid,
		const Name& issuername, const PCTime& notb,
		const PCTime& nota, const Name& subname,
		const PubKey& subpub)
{
	serialnum = ser;
	sigalg = sigalgid;
	issuer = issuername;
	notbefore = notb;
	notafter = nota;
	subject = subname;
	pubkey = subpub;
	return;
}

X509Cert::X509Cert(const Bigint& ser, const AlgId& sigalgid,
			const Name& issuername, const PCTime& notb,
			const PCTime& nota, const Name& subname,
			const PubKey& subpub, const Bstream& cert, 
			const Bstream& certsigpart, const Bstream& casig)
{
	serialnum = ser;
	sigalg = sigalgid;
	issuer = issuername;
	notbefore = notb;
	notafter = nota;
	subject = subname;
	pubkey = subpub;
	certdata = cert;
	certsigdata = certsigpart;
	sigdata = casig;
	return;
}

Boolean operator ==(const X509Cert& a, const X509Cert& b)
{
	if (a.certdata == b.certdata)
		return BOOL_TRUE;
	else
		return BOOL_FALSE;
}

VerifyResult
X509Cert::verify(const PubKey& pubca) const
{
	VerifyResult v;
	v = verify_sig(certsigdata, sigdata, pubca, sigalg);
	if (v)
		return v;

	PCTime now = timenow();
	if (now < notbefore)
		return (INVALID_NOTBEFORE);
	else if (now > notafter)
		return (INVALID_EXPIRED);
#ifdef RW
	if (cert_in_CRL(*this) == BOOL_TRUE)
		return (INVALID_REVOKED);
#endif
	return (VALID);
}


Bstream
X509Cert::sign_and_encode(const Bstream& privkey)
{
	Bstream val, tmpstr, tmpstr2, algid;

	val = asn1_der_encode_integer(serialnum);
	tmpstr = asn1_der_encode_null();
	tmpstr = algid = // Signature Algorithm ID
	   asn1_der_encode_sequence(sigalg.algid.encode() + tmpstr);
	val = val + tmpstr;
	val = val + issuer.encode();
	tmpstr = asn1_der_encode_utctime(notbefore);
	tmpstr2 = asn1_der_encode_utctime(notafter);
	tmpstr = // Validity
	   asn1_der_encode_sequence(tmpstr + tmpstr2);
	val = val + tmpstr;
	val = val + subject.encode();
	tmpstr = pubkey.keytype.algid.encode() + pubkey.keytype.params;
 	// Subject Public Key Algorithm ID
   	tmpstr = asn1_der_encode_sequence(tmpstr);
   	tmpstr2 = asn1_der_encode_bit_string(pubkey.key);
	// Subject Public Key Info
   	tmpstr = asn1_der_encode_sequence(tmpstr + tmpstr2);
	val = val + tmpstr;
   	val = asn1_der_encode_sequence(val);	// ToBeSigned
	certsigdata = val;			// Save this for signature

	tmpstr = algid; 	// Signature Algorithm ID, again
	val = val + tmpstr;
// XXX  -- change this to support different sig algs
	ObjId md_oid = dsaWithSHA;
	tmpstr = sign(certsigdata, privkey, md_oid);
	sigdata = tmpstr;			// Save the signature

	tmpstr = // Encrypted hash, as a BIT-STRING
	   asn1_der_encode_bit_string(tmpstr);

	val = val + tmpstr;
	val = // SIGNED, final certificate
	   asn1_der_encode_sequence(val);
	certdata = val;				// Save the final form
	return (val);
}

Bigint
X509Cert::getserialnum() const
{
	return (serialnum);
}

Name
X509Cert::getsubject() const
{
	return (subject);
}

Name
X509Cert::getissuer() const
{
	return (issuer);
}

AlgId
X509Cert::getsigalgid() const
{
	return (sigalg);
}

PCTime
X509Cert::getnotbefore() const
{
	return (notbefore);
}

PCTime
X509Cert::getnotafter() const
{
	return (notafter);
}

PubKey
X509Cert::getpubkey() const
{
	return (pubkey);
}

Bstream
X509Cert::getcertdata() const
{
	return (certdata);
}

void
X509Cert::print()
{
	printf("Serial Number = "); serialnum.print(); printf("\n");
	printf("X.509 Certificate Signature Algorithm ID:\n\t");
	sigalg.algid.print(); 
	if (sigalg.algid == md2WithRSAEncryption)
		printf(" == MD2-WITH-RSA-ENCRYPTION");
	else if (sigalg.algid == dsaWithSHA)
		printf(" == SHA-WITH-DSA-SIGNATURE");
	printf("\n");
	printf("X.509 Certificate Signature Algorithm parameters:");
	int retval;
	Bstream der_stream = sigalg.params;
	retval = asn1_der_decode_null(der_stream);
   	if (retval < 0) {
		sigalg.params.print(); printf("\n");
	} else {
		printf(" NONE\n");
	}
	printf("Issuer Name:\t\t"); issuer.print(); printf("\n");
	printf("Not Valid Before:\t"); notbefore.print(); printf("\n");
	printf("Not Valid After:\t"); notafter.print(); printf("\n");
	printf("Subject Name:\t\t"); subject.print(); printf("\n");
	printf("Subject Public Key Algorithm:\t"); 
	pubkey.keytype.algid.print(); 
	if (pubkey.keytype.algid == dsa)
		printf(" == DSA");
	else if (pubkey.keytype.algid == dhKeyAgreement ||
			pubkey.keytype.algid == sundhKeyAgreement)
		printf(" == Diffie-Hellman");
	else if (pubkey.keytype.algid == rsaEncryption)
		printf(" == RSA-ENCRYPTION");
	else if (pubkey.keytype.algid == rsa)
		printf(" == RSA");
	printf("\n");
	printf("Subject Public Key Algorithm parameters: ");
	der_stream = pubkey.keytype.params;
	if (pubkey.keytype.algid == rsa) {
		Bigint	keysize;
		printf("Key Size = ");
		retval = asn1_der_decode_integer(der_stream, keysize);
		if (retval < 0) {
			printf("RSA parameters INTEGER decode error");
			asn1_perror(retval);
			pubkey.keytype.params.print(); printf("\n");
		} else {
			keysize.printd(); printf(" bits\n");
		}
	} else if (pubkey.keytype.algid == dhKeyAgreement ||
			pubkey.keytype.algid == sundhKeyAgreement) {
		Bigint Mod, Base;

		int retval=asn1_der_decode_dh_params(der_stream, Mod, Base);
		printf("\nDiffie-Hellman Modulus (p): "); Mod.printd();
		printf("\nDiffie-Hellman Generator (g): "); Base.printd();
		printf("\n");
	} else if (pubkey.keytype.algid == dsa) {
		Bigint prime, subprime, gen;

		int retval=asn1_der_decode_dsa_params(der_stream, prime,
				subprime, gen);
		printf("\np: "); prime.printd();
		printf("\nq: "); subprime.printd();
		printf("\ng:  "); gen.printd();
		printf("\n");
	} else {
		retval = asn1_der_decode_null(der_stream);
		if (retval < 0) {
			printf("\n");
			pubkey.keytype.params.print(); printf("\n");
		} else {
			printf("NONE\n");
		}
	}
	printf("Subject Public Key: ");
	int  seqlen;
	Bigint modulus, exponent, pub;
	der_stream = pubkey.key;
	if (pubkey.keytype.algid == rsa ||
			pubkey.keytype.algid == rsaEncryption) {
		retval = asn1_der_decode_sequence(der_stream, seqlen);
		if (retval < 0) {printf("Bad Key encoding\n"); return;}
		{
			retval = asn1_der_decode_integer(der_stream, modulus);
			if (retval < 0) {printf("Bad Key encoding\n"); return;}
			retval = asn1_der_decode_integer(der_stream, exponent);
			if (retval < 0) {printf("Bad Key encoding\n"); return;}
			printf("\nRSA modulus = "); modulus.printd();
			printf("\nRSA exponent = "); exponent.printd();
			printf("\n");
		} 
	} else if (pubkey.keytype.algid == dhKeyAgreement ||
			pubkey.keytype.algid == sundhKeyAgreement ||
			pubkey.keytype.algid == dsa) {
		retval = asn1_der_decode_integer(der_stream, pub);
		printf("\nDiffie-Hellman public value = "); pub.printd();
		printf("\n");
	} else {
		retval = asn1_der_decode_null(der_stream);
		if (retval < 0) {
			printf("\n");
			pubkey.key.print(); printf("\n");
		} else {
			printf("NONE\n");
		}
	}
}

/*
 * X.500 X509Certificate DER decoding
 */
int asn1_der_decode_cert(Bstream& der_stream, X509Cert& cert)
{
	byte tmp = 0;
	int  seqlen, retval;
	Bigint serialnum;
	AlgId  sigalgid, sigalgid2;
	PCTime notbefore, notafter;
	Name issuername, subjectname;
	PubKey psubject;
	Bstream casig, certsigdata, allcert;

	Bstream saved_stream = der_stream;
	SEQUENCE		// SIGNED Macro
	{	
		certsigdata = der_stream;
		SEQUENCE	// ToBeSigned
		{
			// version -- Context specific [0]; for the moment,
			// we assume only 1988 format, and by DER
			// decoding rules, this field is absent.

			INTEGER(serialnum);
			SEQUENCE // X509Cert signature Algorithm Identifier
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
			NAME(issuername);
			SEQUENCE // Validity
			{
				UTCTime(notbefore);
				UTCTime(notafter);
			}
			NAME(subjectname);
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
		int tobesignedlen = certsigdata.getlength() - 
						der_stream.getlength();
		int trunclen = certsigdata.getlength() - tobesignedlen;
		certsigdata.truncate(trunclen);
		SEQUENCE	// Alg ID of cert signing
		{
			int begin, end, used, anylen;
			MARK_LEN(begin);
			OBJECT_IDENTIFIER(sigalgid2.algid);
			MARK_LEN(end);
			used = begin - end;
			if ((anylen = seqlen - used) > 0) {
				Bstream any(anylen, DATAP);
				CONSUME(anylen);
				sigalgid2.params = any;
			}
		}
		//  encrypted hash, i.e SIGNATURE
		BIT_STRING(casig);
	}
	int newlen = saved_stream.getlength() - der_stream.getlength();
	int trunclen = saved_stream.getlength() - newlen;
	saved_stream.truncate(trunclen);
	X509Cert tmpcert(serialnum, sigalgid, issuername, notbefore, notafter,
		     subjectname, psubject, saved_stream, certsigdata, casig);
	cert = tmpcert;
	return (SUCCESS);
}


Bstream 
asn1_der_encode_certreq(const char *name, const Bstream& publickey,
			const Bstream& privkey)
{
	Bstream tmpstr, certreqinfo, signature, nullbstr;
	Name nullname;

	tmpstr = asn1_der_encode_integer((short)0); // Version #0 (PKCS# 10)
	Name subject(name);
	if (subject == nullname)
		return (nullbstr);
	tmpstr = tmpstr + subject.encode();
	tmpstr = tmpstr + publickey;
	// XXX No attribute encoding for now
	certreqinfo = asn1_der_encode_sequence(tmpstr);
	tmpstr = sign(certreqinfo, privkey, dsaWithSHA);
// XXX - remove this 
//	tmpstr = sign(certreqinfo, privkey, md2WithRSAEncryption);
	signature = asn1_der_encode_bit_string(tmpstr);
	tmpstr = asn1_der_encode_null();
// XXX - change this
//	tmpstr = md2WithRSAEncryption.encode() + tmpstr;
	tmpstr = dsaWithSHA.encode() + tmpstr;
	tmpstr = asn1_der_encode_sequence(tmpstr);
	tmpstr = certreqinfo + tmpstr + signature;
	return (asn1_der_encode_sequence(tmpstr));
}

int
asn1_der_decode_algid(Bstream& der_stream, AlgId& algid)
{
	int retval, seqlen;

	SEQUENCE {
		int begin, end, used, anylen;
		MARK_LEN(begin);
		OBJECT_IDENTIFIER(algid.algid);
		MARK_LEN(end);
		used = begin - end;
		if ((anylen = seqlen - used) > 0) {
			Bstream any(anylen, DATAP);
			CONSUME(anylen);
			algid.params = any;
		}
	}
}

int
asn1_der_decode_dh_params(Bstream der_stream, Bigint& mod, Bigint& base)
{
	byte tmp = 0;
        int seqlen, retval;
        ObjId oid;
        Bigint p, g, pvl;
 
        SEQUENCE
        {
                OBJECT_IDENTIFIER(oid);
                SEQUENCE
                {
                        INTEGER(p);
                        INTEGER(g);
                        INTEGER(pvl);
                }
        }
        mod = p;
        base = g;
        return (0);
}
