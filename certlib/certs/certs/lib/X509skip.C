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
#pragma ident "@(#)X509skip.C	1.10 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include "Bstream.h"
#include "Bigint.h"
#include "SkipCert.h"
#include "X509Cert.h"
#include "X509skip.h"

X509SkipCert::skip_params(Bigint &g, Bigint &p)
{
	int retval;
	if (CertType != SKIP_CERT_X509)
		return 1;
	PubKey dh_pub_value=getpubkey();
        retval = asn1_der_decode_dh_params(dh_pub_value.keytype.params, p, g);
	return retval;
}

Bigint X509SkipCert::skip_pubkey()
{
	int retval;
	PubKey dh_pub_value=getpubkey();
        Bigint dh_pub_int;

	if (CertType != SKIP_CERT_X509)
		return Bigint((short)0);

        retval = asn1_der_decode_integer(dh_pub_value.key, dh_pub_int);
	if (retval != 0)
		return 1;
	return dh_pub_int;
}

u_long X509SkipCert::skip_notvalidafter()
{
	if (CertType != SKIP_CERT_X509)
		return 0;
	return getnotafter().ntptime();
}

u_long X509SkipCert::skip_notvalidbefore()
{
	if (CertType != SKIP_CERT_X509)
		return 0;
	return getnotbefore().ntptime();
}


X509SkipCert::decode(Bstream Cert)
{
	int retval;

	CertType=SKIP_CERT_X509;
	validated = 0;
	orig=Cert;
        retval = asn1_der_decode_cert(Cert, *this);
        if (retval != SUCCESS)
                return 1;
	return 0;
}

X509SkipCert::isValid(Bstream ca_certstr)
{
	PubKey ca_pub_key;
	X509SkipCert ca_self_cert;
	int retval;

	retval=asn1_der_decode_cert(ca_certstr, ca_self_cert);
        ca_pub_key = ca_self_cert.getpubkey();
 
        VerifyResult result = verify(ca_pub_key);
        if (result != VALID) {
		validated=0;
		return 0;
	}
	validated=1;
        return 1;
}

Bstream X509SkipCert::skip_name()
{
	String ip;
	Bstream nullstr;

	ip = getsubject().getip();
	if (ip.length() == 0)		// If not encoded as ip, try cn
		ip = getsubject().getcn();

        // We assume that the boxid is the IP Address
 
        // The IPA is a printable hex string OR a dot
	// notation IP address. We convert the string
	// into its binary representation as a Bstream.

	if (CertType != SKIP_CERT_X509)
		return nullstr;
	return  Bigint_to_Bstr(Bigint((const char *)ip));
}

Bstream X509SkipCert::encode()
{
	return orig;
}

void X509SkipCert::print()
{
	X509Cert::print();
}
