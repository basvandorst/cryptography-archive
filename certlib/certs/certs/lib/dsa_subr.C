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

#pragma ident "@(#)dsa_subr.C	1.8 96/01/30"

/*
 * Copyright (c) 1995, 1996  Colin Plumb.  All rights reserved.
 * For licensing and other legal details, see the file legal.c.
 *
 * This generates DSA primes using a (hopefully) clearly
 * defined algorithm, based on David Kravitz's "kosherizer".
 * It is not, however, identical.
 */

#include <stdio.h>
#include <string.h>
#include "Bstream.h"
#include "Bigint.h"
#include "Time.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"
#include "X509Cert.h"
#include "Sig.h"

#include "bn.h"
#include "bn_glue.h"
#include "prime.h"
#include "sha.h"
#include "dsa.h"
#include "ca.h"
#include "utils.h"

/*
 * Generate a random bignum of a specified length.
 */
int
genRandBn(struct BigNum *bn, unsigned bits)
{
	unsigned char buf[64];
	unsigned bytes = (bits+7)/8;
	unsigned l = 0;	/* Current position */
	unsigned i;

	bnSetQ(bn, 0);

	if (!bits)
		return 0;

	/* Do low-order portions */
	while (bytes > sizeof(buf)) {
		randpool_getbytes(buf, sizeof(buf));
		if (bnInsertBigBytes(bn, buf, l, sizeof(buf)) < 0)
			return -1;
		l += sizeof(buf);
	}

	/* Do the most-significant chunk */
	randpool_getbytes(buf, bytes);
	/* Mask off excess high bits */
	buf[0] &= 255 >> (-bits & 7);

	return bnInsertBigBytes(bn, buf, l, bytes);
}

/*
 * DSA signature.
 * Inputs:
 * p - a large prime
 * q - a 160-bit prime factor of (p-1).  (Actually, any length will do.)
 * g - a generator of order q modulo p, i.e. g^q == 1 (mod p)
 * x - the secret key, 1 < x < q-1
 * hash - the value to be signed
 * k - the per-signature random number, 1 < k < q-1
 * Outputs: r, s
 *
 * returns 0 on success and -1 on failure (out of memory or bogus key)
 */
static int
dsaSign(struct BigNum const *p, struct BigNum const *q,
	struct BigNum const *g, struct BigNum const *x,
	struct BigNum const *hash, struct BigNum const *k,
	struct BigNum *r, struct BigNum *s)
{
	int retval = -1;
	struct BigNum t;

	/* Sanity check all the values */
	if ((bnLSWord(p) & 1) == 0|| (bnLSWord(q) & 1) == 0 ||
	    bnCmp(p, q) <= 0 || bnCmp(p, g) <= 0 ||
	    bnCmp(q, x) <= 0 || bnCmp(q, k) <= 0 ||
	    bnBits(x) <= 1 || bnBits(k) <= 1)
		return -1;

	bnBegin(&t);

	printf("Error: Cannot Sign\n");
	retval = -1;

failed:
	bnEnd(&t);
	return retval;
}

Bstream
DSA_sign(const Bigint& p, const Bigint& q, const Bigint& g,
		const Bigint& x, const Bstream& data) 
{
	struct BigNum prime, subprime, gen, secret, k, s_r, s_s, hash;
	Bstream nullbstr, localhash;
	int retval;

	bnInit();	// just in case ...

	// locally compute the hash
	localhash = messageDigest(data, dsaWithSHA);

	// initialize bignums and convert as neccessary
	bnBegin(&prime);
	bnBegin(&subprime);
	bnBegin(&gen);
	bnBegin(&secret);
	bnBegin(&hash);
	bnBegin(&k);
	bnBegin(&s_r);
	bnBegin(&s_s);

	Bigint_to_BigNum(&p, &prime);
	Bigint_to_BigNum(&q, &subprime);
	Bigint_to_BigNum(&g, &gen);
	Bigint_to_BigNum(&x, &secret);
	Bstream_to_BigNum(&localhash, &hash);

	// generate k uniformly between 0 and q-1
	camode = TRUE;
	if (genRandBn(&k, bnBits(&subprime)+8) < 0 ||
	    bnMod(&k, &k, &subprime) <0) {
		fprintf(stderr, "DSA_sign: failed to generate 'k'\n");
		goto failed;
	}
	retval = dsaSign(&prime, &subprime, &gen, &secret, &hash, &k,
			&s_r, &s_s);
	if (retval == 0) {
		Bigint r, s;
		BigNum_to_Bigint(&s_r, &r);
		BigNum_to_Bigint(&s_s, &s);
		nullbstr = asn1_der_encode_dsa_signature(r, s);
	} else
		fprintf(stderr, "DSA_sign: failed to sign\n");

failed:
	bnEnd(&prime);
	bnEnd(&subprime);
	bnEnd(&gen);
	bnEnd(&secret);
	bnEnd(&k);
	bnEnd(&hash);
	bnEnd(&s_r);
	bnEnd(&s_s);

	return (nullbstr);
}

/*
 * DSA signature verification.
 * Inputs:
 * p - a large prime
 * q - a 160-bit prime factor of (p-1).  (Actually, any length will do.)
 * g - a generator of order q modulo p, i.e. g^q == 1 (mod p)
 * y - the public key, g^x mod p, 1 < y < p-1
 * r = the signature (first part)
 * s = the signature (second part)
 * hash - the value to be signed
 * Outputs:
 * Returns 1 for a good signature, 0 for bad, and -1 on error.
 *
 */
static int
dsaVerify(struct BigNum const *p, struct BigNum const *q,
          struct BigNum const *g, struct BigNum const *y,
          struct BigNum const *r, struct BigNum const *s,
          struct BigNum const *hash)
{
	struct BigNum v, u1, u2;
	int retval = -1;
	int i;

	/* Sanity check all the values */
	if ((bnLSWord(p) & 1) == 0|| (bnLSWord(q) & 1) == 0 ||
	    bnCmp(p, q) <= 0 || bnCmp(p, g) <= 0 ||
	    bnCmp(q, r) <= 0 || bnCmp(q, s) <= 0)
		return 0;	// Messed up -> all signatures are bad

	bnBegin(&v);
	bnBegin(&u1);
	bnBegin(&u2);

	printf("Error: Cannot Verify\n");
	retval = -1;		// An Error since we can't verify
failed:
	bnEnd(&u2);
	bnEnd(&u1);
	bnEnd(&v);
	return retval;
}

VerifyResult
DSA_verify(const Bstream& data, const Bstream& sig, const PubKey& pubkey,
		AlgId sigalg)
{
	Bigint p, q, g, y, r, s;
	Bstream localhash;
	struct BigNum prime, subprime, gen, pub, s_r, s_s, hash;
	int retval;

	bnInit();	// just in case ....

	// decode the DSA parameters, and public key
	Bstream der_stream = pubkey.keytype.params;
	retval = asn1_der_decode_dsa_params(der_stream, p, q, g);
	if (retval < 0) {
		fprintf(stderr, "DSA_verify: Bad key params encoding\n");
		return (INVALID_SIG);
	}
	der_stream = pubkey.key;
	y = Bigint(der_stream.getdatap(), der_stream.getlength());
	if (y == Bigint((short)0)) {
		fprintf(stderr, "DSA_verify: Bad publickey encoding\n");
		return (INVALID_SIG);
	}
	// decode the signature into its components
	retval = asn1_der_decode_dsa_signature(sig, r, s);
	if (retval < 0) {
		fprintf(stderr, "DSA_verify: Bad signature encoding\n");
		return (INVALID_SIG);
	}

	// Locally compute message digest over the input portion
	localhash = messageDigest(data, sigalg.algid);

	// intialize bignums and convert as necessary
	bnBegin(&prime);
	bnBegin(&subprime);
	bnBegin(&gen);
	bnBegin(&pub);
	bnBegin(&s_r);
	bnBegin(&s_s);
	bnBegin(&hash);

	Bigint_to_BigNum(&p, &prime);
	Bigint_to_BigNum(&q, &subprime);
	Bigint_to_BigNum(&g, &gen);
	Bigint_to_BigNum(&y, &pub);
	Bigint_to_BigNum(&r, &s_r);
	Bigint_to_BigNum(&s, &s_s);
	Bstream_to_BigNum(&localhash, &hash);
	
	retval = dsaVerify(&prime, &subprime, &gen, &pub, &s_r, &s_s, &hash);

	bnEnd(&prime);
	bnEnd(&subprime);
	bnEnd(&gen);
	bnEnd(&pub);
	bnEnd(&s_r);
	bnEnd(&s_s);
	bnEnd(&hash);

	if (retval <= 0) {
		fprintf(stderr, "DSA_verify: Bad signature\n");
		return (INVALID_SIG);
	}
	return (VALID);
}

// ASN.1 routines for DSA encoding/decoding
int
asn1_der_decode_dsa_privkey(Bstream der_stream, Bigint& p, Bigint& q,
				Bigint& g, Bigint& x)
{
        byte tmp = 0;
        int seqlen, retval = 0;
        ObjId oid;
        Bigint version;
	Bstream DSAprivkey;

	SEQUENCE {
		INTEGER(version);	// version of syntax
		SEQUENCE {		// privkey alg id
			OBJECT_IDENTIFIER(oid);
                	SEQUENCE {
                        	INTEGER(p);
                        	INTEGER(q);
                        	INTEGER(g);
                	}
        	}
		OCTET_STRING(DSAprivkey);
		// XXX -- leaving out the optional attributes stuff
	}
	retval = asn1_der_decode_integer(DSAprivkey, x);
        return (retval);
}

Bstream
asn1_der_encode_dsa_privkey(Bigint& prime, Bigint& subprime,
				Bigint& generator, Bigint& secret)
{
	Bstream tmpstr, params;
	ObjId	keyoid = dsa;

	// version=0
	tmpstr = asn1_der_encode_integer((short)0);

	// params
	params = asn1_der_encode_sequence(
			asn1_der_encode_integer(prime) +	
			asn1_der_encode_integer(subprime) +	
			asn1_der_encode_integer(generator));

	// subject private key alg id
	params = keyoid.encode() + params;
	params = asn1_der_encode_sequence(params);

	// subject private key
	tmpstr = tmpstr + params +
		asn1_der_encode_octet_string(
			asn1_der_encode_integer(secret));

	// XXX -- no attribute encoding for now
	tmpstr = asn1_der_encode_sequence(tmpstr);

	return (tmpstr);
}


int
asn1_der_decode_dsa_signature(Bstream der_stream, Bigint& r, Bigint& s)
{
	byte tmp = 0;
	int seqlen, retval;
 
	// wrapper BIT_STRING decoding is done in-line
	SEQUENCE {
		INTEGER(r);
		INTEGER(s);
	}
	return (0);
}

Bstream
asn1_der_encode_dsa_signature(Bigint& r, Bigint& s)
{
	Bstream tmpstr;

	tmpstr = asn1_der_encode_sequence(
			asn1_der_encode_integer(r) +	
			asn1_der_encode_integer(s));

	// final bit string encoding is done in-line
	return (tmpstr);
}

int
asn1_der_decode_dsa_params(Bstream der_stream, Bigint& p, Bigint& q, Bigint& g)
{
        byte tmp = 0;
        int seqlen, retval;
        ObjId oid;
 
	SEQUENCE {
		INTEGER(p);
		INTEGER(q);
		INTEGER(g);
	}
	return (0);
}

int
asn1_der_decode_dsa_publickey(Bstream der_stream, Bigint& p, Bigint& q,
				Bigint& g, Bigint& y)
{
        byte tmp = 0;
        int seqlen, retval = 0;
        ObjId oid;
	Bstream pub;
 
        SEQUENCE {	// subject public key info
		SEQUENCE {	// subject public key alg id
                	OBJECT_IDENTIFIER(oid);
                	SEQUENCE {
                        	INTEGER(p);
                        	INTEGER(q);
                        	INTEGER(g);
                	}
		}
		// subject public key
		BIT_STRING(pub);
        }
	retval = asn1_der_decode_integer(pub, y);
        return (retval);
}

Bstream
asn1_der_encode_dsa_publickey(Bigint& p, Bigint& q, Bigint& g, Bigint& y)
{
	Bstream tmpstr;
	ObjId	keyoid = dsa;

	// params
	tmpstr = asn1_der_encode_sequence(
			asn1_der_encode_integer(p) +	
			asn1_der_encode_integer(q) +	
			asn1_der_encode_integer(g));

	// subject public key alg id
	tmpstr = keyoid.encode() + tmpstr;
	tmpstr = asn1_der_encode_sequence(tmpstr);

	// subject public key info
	tmpstr = asn1_der_encode_sequence(tmpstr +
			asn1_der_encode_bit_string(
				asn1_der_encode_integer(y)));

	return (tmpstr);
}

/*
 * Generate a bignum of a specified length, with the given
 * high and low 8 bits. "High" is merged into the high 8 bits of the
 * number.  For example, set it to 0x80 to ensure that the number is
 * exactly "bits" bits long (i.e. 2^(bits-1) <= bn < 2^bits).
 * "Low" is merged into the low 8 bits.  For example, set it to
 * 1 to ensure that you generate an odd number.
 *
 * Then XOR the result into the input bignum.  This is to
 * accomodate the kosherizer in all its generality.
 *
 * The bignum is generated using the given seed string.  The
 * technique is from David Kravitz (of the NSA)'s "kosherizer".
 * The string is hashed, and that (with the low bit forced to 1)
 * is used for the low 160 bits of the number.  Then the string,
 * considered as a big-endian array of bytes, is incremented
 * and the incremented value is hashed to produce the next most
 * significant 160 bits, and so on.  The increment is performed
 * modulo the size of the seed string.
 *
 * The seed is returned incremented so that it may be used to generate
 * subsequent numbers.
 *
 * The most and least significant 8 bits of the returned number are forced
 * to the values passed in "high" and "low", respectively.  Typically,
 * high would be set to 0x80 to force the most significant bit to 1.
 */
int
genKosherBn(struct BigNum *bn, unsigned bits, unsigned char high,
unsigned char low, unsigned char *seed, unsigned len)
{
	unsigned char buf1[SHA_DIGESTSIZE];
	unsigned char buf2[SHA_DIGESTSIZE];
	unsigned bytes = (bits+7)/8;
	unsigned l = 0;	/* Current position */
	unsigned i;
	struct SHAContext sha;

	if (!bits)
		return 0;

	/* Generate the first bunch of hashed data */
	shaInit(&sha);
	shaUpdate(&sha, seed, len);
	shaFinal(&sha, buf1);
	/* Increment the seed, ignoring carry out. */
	i = len;
	while (i-- && (++seed[i] & 255) == 0)
		;
	/* XOR in the existing bytes */
	bnExtractBigBytes(bn, buf2, l, SHA_DIGESTSIZE);
	for (i = 0; i < SHA_DIGESTSIZE; i++)
		buf1[i] ^= buf2[i];
	buf1[SHA_DIGESTSIZE-1] |= low;

	while (bytes > SHA_DIGESTSIZE) {
		bytes -= SHA_DIGESTSIZE;
		/* Merge in low half of high bits, if necessary */
		if (bytes == 1 && (bits & 7))
			buf1[0] |= high << (bits & 7);
		if (bnInsertBigBytes(bn, buf1, l, SHA_DIGESTSIZE) < 0)
			return -1;
		l += SHA_DIGESTSIZE;

		/* Compute the next hash we need */
		shaInit(&sha);
		shaUpdate(&sha, seed, len);
		shaFinal(&sha, buf1);
		/* Increment the seed, ignoring carry out. */
		i = len;
		while (i-- && (++seed[i] & 255) == 0)
			;
		/* XOR in the existing bytes */
		bnExtractBigBytes(bn, buf2, l, SHA_DIGESTSIZE);
		for (i = 0; i < SHA_DIGESTSIZE; i++)
			buf1[i] ^= buf2[i];
	}

	/* Do the final "bytes"-long section, using the tail bytes in buf1 */
	/* Mask off excess high bits */
	buf1[SHA_DIGESTSIZE-bytes] &= 255 >> (-bits & 7);
	/* Merge in specified high bits */
	buf1[SHA_DIGESTSIZE-bytes] |= high >> (-bits & 7);
	if (bytes > 1 && (bits & 7))
		buf1[SHA_DIGESTSIZE-bytes+1] |= high << (bits & 7);
	/* Merge in the appropriate bytes of the buffer */
	if (bnInsertBigBytes(bn, buf1+SHA_DIGESTSIZE-bytes, l, bytes) < 0)
		return -1;

	return 0;
}

/* Context for printing progress dots on the screen. */
struct Progress {
	FILE *f;
	unsigned column, wrap;
};

static int
genProgress(void *arg, int c)
{
	struct Progress *p = (struct Progress *)arg;
	if (++p->column > p->wrap) {
		putc('\n', p->f);
		p->column = 1;
	}
	putc(c, p->f);
	fflush(p->f);
	return 0;
}

int
dsaGen(struct BigNum *p, unsigned pbits, struct BigNum *q, unsigned qbits,
	struct BigNum *g, struct BigNum *x, struct BigNum *y,
	unsigned char *seed, unsigned len, FILE *f)
{
	return -1;
}

// returns 0 on success or -1 on failure
int
dsa_newcert(char *certreqFile, char *certFile, int lifetime,
		Bstream& passphrase)
{
	int retval;
	Bstream nullbstr;
	PubKey pub_CA, pub_subject;
	AlgId sigalg;
	Name issuer, subject;
	String errormsg;
	sigalg.algid = dsaWithSHA;
	camode = TRUE;
	Bstream certreq = File_to_Bstr(get_byzantine_dir() + CA_CERTREQ_FILE);
  
	if (certreq == nullbstr) {
		fprintf(stderr, "Unable to read %s\n", CA_CERTREQ_FILE);
		return(-1);
	}
	if (retval = get_certreq_params(certreq, issuer, pub_CA)) {
		fprintf(stderr,
		"Error in decoding CA certreq file %s\n", CA_CERTREQ_FILE);
		asn1_perror(retval);
		return(retval);
	}
	if (retval = get_certreq_params(File_to_Bstr(certreqFile), 
			subject, pub_subject)) {
		fprintf(stderr,
			"Error in decoding subject certreq file %s\n",
			certreqFile);
		asn1_perror(retval);
		return(retval);
	}
	
	// check if a certificate is already issued for
	// this subject and public key
	if (allow_certification(subject, pub_subject) == BOOL_FALSE) {
		fprintf(stderr,
		"Cannot issue a certificate for this request.\n");
		return(-1);
	}

	PCTime notbefore = timenow();
	GMtime gtime = notbefore.get();
	gtime.year += lifetime / 12;	// lifetime is in months
	gtime.month += lifetime % 12;	// 0 - 11 months
	if (gtime.month > 12) {
		gtime.year += gtime.month / 12;
		gtime.month = gtime.month % 12; // 1 - 11 months
	}
	PCTime notafter(gtime);

// XXX Change for HW signer
	Bstream priv_CA;
//	Bstream priv_CA = getCAprivkey(passphrase, errormsg);
//	if (priv_CA == nullbstr) {
//		fprintf(stderr, "Error: %s\n", (const char *)errormsg);
//		fprintf(stderr,
//		"Unable to fetch CA private key, certificate not created\n");
//		return(-1);
//	}
	Bigint serialnum = getSerialnum();
	X509Cert cert(serialnum, sigalg, issuer, notbefore, notafter, subject,
			pub_subject);
	Bstream bercert = cert.sign_and_encode(priv_CA);

	// Verify the signature on certificate
	VerifyResult result = cert.verify(pub_CA);
	if (result != VALID) {
		fprintf(stderr, "Error in creating new certificate.\n");
		return(-1);
	}
	String certfilename = get_byzantine_dir() + (String)CERT_DATABASE_DIR +
				DIR_MARKER + serialnum.getnumstr();
  
	bercert.store(certfilename);	// Fix for decimal # use XXX
	bercert.store(certFile);	// Fix for decimal # use XXX

	String temp;
	temp = serialnum.getnumstrd();
	fprintf(stderr, "Succesfully created valid certificate # ");
	fprintf(stderr, "%s\n", (const char *)temp);
	return(0);
}
