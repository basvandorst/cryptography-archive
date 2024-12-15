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

#pragma ident "@(#)HashCert.C	1.22 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include "Bstream.h"
#include "Bigint.h"
#include "ObjId.h"
#include "SkipCert.h"
#include "X509Cert.h"
#include "Sig.h"
#include "HashCert.h"

HashCert::HashCert(Bigint n_g, Bigint n_p, Bigint key, u_long validFrom, 
		   u_long validTo)
{
	CertType=SKIP_DH_PUBLIC;
	g=n_g;
	p=n_p;
	pubKey=key;
	notValidBefore=validFrom;
	notValidAfter=validTo;
	orig=encode();
}

HashCert::HashCert()
{
	CertType=SKIP_DH_PUBLIC;
	Bigint x("0");
	g=p=pubKey=x;
	notValidBefore=notValidAfter=0;
	validated=0;	
}

int HashCert::decode(Bstream cert)
{
	u_short primelen;
	u_short genlen;
	u_short publen;
	u_char *buf;

	CertType=SKIP_DH_PUBLIC;
	validated = 0;
	orig=cert;

	buf=cert.getdatap();

	memcpy(&notValidBefore, buf, 4);
	notValidBefore=ntohl(notValidBefore);
	buf+=4;

	memcpy(&notValidAfter, buf, 4);
	notValidAfter=ntohl(notValidAfter);
	buf+=4;

	memcpy(&primelen,buf, 2);
	primelen=ntohs(primelen);
	buf+=2;
	p=Bigint(buf, primelen);
	buf+=primelen;

	memcpy(&genlen,buf, 2);
	genlen=ntohs(genlen);
	buf+=2;
	g=Bigint(buf, genlen);
	buf+=genlen;

	memcpy(&publen, buf, 2);
	publen=ntohs(publen);
	buf+=2;
	pubKey=Bigint(buf, publen);
	return 0;
}

int HashCert::isValid(Bstream hash)
{
	Bstream name;
	if (NTPNOW < notValidBefore ||  NTPNOW > notValidAfter ) {
		return 2;
	}

	name=MD5(encode());	
	if (hash.getlength() != 0 && hash != name) 
		return 0;
	validated=1;
	return 1;
}

Bstream HashCert::encode()
{
	Bstream cert;
	u_short length;
	Bstream nullstr;
	Bstream temp;

	if (CertType != SKIP_DH_PUBLIC)  // Uninitialized
		return nullstr;

	cert = Bstream(4, (u_char *)&notValidBefore);
	cert += Bstream(4, (u_char *)&notValidAfter);


	temp = Bigint_to_Bstr(p);
	length=htons((u_short)temp.getlength());
	cert += Bstream(2, (u_char *)&length);
	cert += temp;

	temp = Bigint_to_Bstr(g);
	length=htons((u_short)temp.getlength());
	cert += Bstream(2, (u_char *)&length);
	cert += temp;


	temp = Bigint_to_Bstr(pubKey);
	length=htons((u_short)temp.getlength());
	cert += Bstream(2, (u_char *)&length);
	cert += temp;

	return cert;
}


HashCert::skip_params (Bigint &newg, Bigint &newp)
{
	if (CertType != SKIP_DH_PUBLIC)  // Uninitialized
		return 1;
	newg=g;
	newp=p;
	return 0;
}

Bigint HashCert::skip_pubkey()
{
	if (CertType != SKIP_DH_PUBLIC)  // Uninitialized
		return Bigint((short)0);
	return pubKey;
}

Bstream HashCert::skip_name()
{
	Bstream nullstr;
	if (CertType != SKIP_DH_PUBLIC)  // Uninitialized
		return nullstr;
	return MD5(encode());
}

u_long HashCert::skip_notvalidafter() {
	if (CertType != SKIP_DH_PUBLIC)  // Uninitialized
		return 0;
	return notValidAfter;
}

u_long HashCert::skip_notvalidbefore() {
	if (CertType != SKIP_DH_PUBLIC)  // Uninitialized
		return 0;
	return notValidBefore;
}

void HashCert::print()
{
	time_t temp;
	printf("Format: Hashed Public Key (MD5)\n");
	printf("Name/Hash: ");
	skip_name().print();
	temp=notValidBefore-UNIXSTART;
	printf("Not valid Before: %s", ctime(&temp));
	temp=notValidAfter-UNIXSTART;
	printf("Not valid After: %s", ctime(&temp));
	printf("g: "); g.print();
	printf("\n");
	printf("p: "); p.print();
	printf("\n");
	printf("public key: "); pubKey.print();
	printf("\n");
}
