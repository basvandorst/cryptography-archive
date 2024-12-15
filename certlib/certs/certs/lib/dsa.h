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

#pragma ident "@(#)dsa.h	1.5 96/01/29 Sun Microsystems"

#ifndef DSA_H
#define DSA_H

#include <stdio.h>

extern int genRandBn(struct BigNum *bn, unsigned bits);

extern int genKosherBn(struct BigNum *bn, unsigned bits, unsigned char high,
			unsigned char low, unsigned char *seed,
			unsigned len);

extern int dsaGen(struct BigNum *p, unsigned pbits, struct BigNum *q,
			unsigned qbits, struct BigNum *g, struct BigNum *x,
			struct BigNum *y, unsigned char *seed,
			unsigned len, FILE *f);

extern VerifyResult DSA_verify(const Bstream& data, const Bstream& sig,
				const PubKey& pubkey, AlgId sigalg);

extern Bstream DSA_sign(const Bigint& p, const Bigint& q, const Bigint& g,
			const Bigint& x, const Bstream& data);

extern int dsa_newcert(char *certreqFile, char *certFile, int lifetime,
			Bstream& passphrase);

extern int asn1_der_decode_dsa_privkey(Bstream, Bigint& p, Bigint& q,
					Bigint& g, Bigint& x);
extern Bstream asn1_der_encode_dsa_privkey(Bigint& p, Bigint& q,
						Bigint& g, Bigint& x);

extern int asn1_der_decode_dsa_params(Bstream, Bigint& p, Bigint& q,
					Bigint& g);

extern int asn1_der_decode_dsa_publickey(Bstream, Bigint& p, Bigint& q,
						Bigint& g, Bigint& y);
extern Bstream asn1_der_encode_dsa_publickey(Bigint& p, Bigint& q,
						Bigint& g, Bigint& y);

extern int asn1_der_decode_dsa_signature(Bstream, Bigint& r, Bigint& s);
extern Bstream asn1_der_encode_dsa_signature(Bigint& r, Bigint& s);

#endif
