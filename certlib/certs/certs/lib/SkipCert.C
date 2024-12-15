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
#pragma ident "@(#)SkipCert.C	1.7 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdio.h>

#include "Bstream.h"
#include "Bigint.h"
#include "SkipCert.h"
#include "X509Cert.h"
#include "X509skip.h"
#include "HashCert.h"

SkipCert *decode_SkipCert(int type, Bstream certstr)
{
	SkipCert *result;
	switch(type) {
		case SKIP_CERT_X509:
			result = new X509SkipCert;
			result->decode(certstr);
			break;


		case SKIP_DH_PUBLIC:
			result = new HashCert;
			result->decode(certstr);
			break;

		case SKIP_CERT_PGP:
		case SKIP_CERT_DNS:
		default:
			return NULL;
	}
	return result;
}

int 
Certname_to_num(char *s)
{
	if (strcasecmp(s,"x509")==0)
		return SKIP_CERT_X509;
	if (strcasecmp(s,"x.509")==0)
		return SKIP_CERT_X509;
	if (strcasecmp(s,"PGP")==0)
		return SKIP_CERT_PGP;
	if (strcasecmp(s,"securedns")==0)
		return SKIP_CERT_DNS;
	if (strcasecmp(s,"secure dns")==0)
		return SKIP_CERT_DNS;
	if (strcasecmp(s,"Unsigned DH Public Value")==0)
		return SKIP_DH_PUBLIC;
	if (strcasecmp(s,"dhpublic")==0)
		return SKIP_DH_PUBLIC;
	if (strcasecmp(s,"public")==0)
		return SKIP_DH_PUBLIC;
	return 0;
}	

char *num_to_Certname(int n)
{
	switch(n){
		case 0:
			return "X.509";
		case 1: 
			return "PGP";
		case 2:
			return "SecureDNS";
		case 3:
			return "DHpublic";
		default:	
			return "";
	}
}
