/*
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
#pragma ident "@(#)X509toHashCert.C	1.4 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream.h>
#include "Bstream.h"
#include "Bigint.h"
#include "SkipCert.h"
#include "HashCert.h"

void usage(){
	cerr<<"Usage: X509toHashCert <X509Cert> >outfile\n";
	exit(1);
}

// Convert an X509 certificate to a HashCert certificate

main(int argc, char *argv[])
{
	SkipCert *oldcert;
	Bigint g,p;
	Bstream pubstr;
	Bigint pub;
	u_long before, after;

	if (argc < 2)  {
		usage();
	}
	Bstream cert = File_to_Bstr(argv[1]);
	oldcert = decode_SkipCert(SKIP_CERT_X509,cert);
	if (oldcert == NULL) {
		fprintf(stderr,"Error decoding certificate");
		exit(1);
	}
	oldcert->skip_params(g,p);
	pub=oldcert->skip_pubkey();
	before=oldcert->skip_notvalidbefore();
	after=oldcert->skip_notvalidafter();
	HashCert newcert=HashCert(g, p, pub, before, after);
	Bstream newcertstr=newcert.encode();
	newcertstr.write(stdout);
}
