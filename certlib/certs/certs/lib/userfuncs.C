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

#pragma ident "@(#)userfuncs.C	1.9 96/01/29 Sun Microsystems"

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
#include "userfuncs.h"

Boolean  camode = FALSE;

//
// Fetch the user's certificate
//
X509Cert
getUserCert()
{
	X509Cert cert;
	Bstream nullstr;
	String certfile;
	if (camode == TRUE) {
		String cahomedir;
		char *cahptr = getenv("CA_HOME");
		if (cahptr == NULL) {
			cahomedir = ".";
		} else {
		    	cahomedir = (String)cahptr;
		}
		certfile = cahomedir + DIR_MARKER + CA_CERT_FILE;
	} else {
		String homedir = (String)getenv("HOME");
		certfile = homedir + DIR_MARKER + SECURITY_DIR + 
				DIR_MARKER + CA_CERT_FILE;
	}
	Bstream certstr = File_to_Bstr(certfile);
	if (certstr == nullstr)
		return cert;

	int retval = asn1_der_decode_cert(certstr, cert);
	if (retval < 0) {
		fprintf(stderr, "Unable to decode user certificate %s\n",
				 (const char *)certfile);
		asn1_perror(retval);
	}
	return (cert);
}

#ifdef RW
#define LLtype RWTValDlist<X509Cert>
#else
#ifdef LIBG
#define LLtype SLList<X509Cert>
#endif
#endif

#if 0				/* CHECK THIS */
LLtype getUserIssuers()
{
#define MAX_ISSUERS 100
	LLtype  issuers;
	char buf[10];
	Bstream nullstr;
	String issuerdir;
	if (camode == TRUE) {
		String cahomedir;
		char *cahptr = getenv("CA_HOME");
		if (cahptr == NULL) {
			cahomedir = ".";
		} else {
		    	cahomedir = (String)cahptr;
		}
		issuerdir = cahomedir + DIR_MARKER + "issuers" + DIR_MARKER;
	} else {
		String homedir = (String)getenv("HOME");
		issuerdir = homedir + DIR_MARKER + SECURITY_DIR + 
				DIR_MARKER + USER_ISSUER_DIR + DIR_MARKER;
	}
	for (int i=0; i < MAX_ISSUERS; i++) {
		X509Cert cert;
		bzero(buf, sizeof(buf));
		sprintf(buf, "%d", i);
		String certfile = issuerdir + buf;
		Bstream certstr = File_to_Bstr(certfile);
		if (certstr == nullstr)
			break;
		int retval = asn1_der_decode_cert(certstr, cert);
		if (retval < 0) {
			fprintf(stderr, 
				"Unable to decode user certificate %s\n",
				 (const char *)certfile);
			asn1_perror(retval);
		}
#ifdef RW
		issuers.insert(cert);
#else
#ifdef LIBG
		issuers.append(cert);
#endif
#endif
	}
	return issuers;
}
#endif
