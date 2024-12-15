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

#pragma ident "@(#)utils.C	1.11 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>

#ifdef RW
#include <rw/rstream.h>
#include <rw/tvdlist.h>
#include <rw/ctoken.h>
#include <rw/regexp.h>
#endif

#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"
#include "Name.h"
#include "X509Cert.h"
#include "CRL.h"
#include "Sig.h"

#include "my_types.h"
#include "userfuncs.h"
#include "utils.h"
#include "ca.h"

// Utility functions
#ifdef RW
Boolean
cert_in_CRL(const X509Cert& cert)
{
	// Fetch CRL from local crl cache. 
	String crlsdir = get_byzantine_dir() + CRLS_DIR + DIR_MARKER;
	String issuercrlfile = convertDNtoFileName(cert.getissuer()) + 	
				CRL_SUFFIX;
	issuercrlfile = crlsdir + issuercrlfile;

	CRL crl(File_to_Bstr(issuercrlfile));

	// XXX Need to verify the CRL prior to
	// using.

	if (crl.isRevoked(cert.getserialnum()))
		return (BOOL_TRUE);
	return (BOOL_FALSE);
}
#endif

String
get_byzantine_dir()
{
       String byzantinedir, cahomedir;

       if (camode == TRUE) {
	  char *cahptr = getenv("CA_HOME");
	  if (cahptr == NULL) {
	    cahomedir = ".";
	  } else {
	    cahomedir = (String)cahptr;
	  }
	  byzantinedir = cahomedir + DIR_MARKER;
	} else {
		char *cahptr=getenv("HOME");
		if (cahptr == NULL) 
			cahomedir="/";
		else
			cahomedir=(String)cahptr;
          	byzantinedir = cahomedir + DIR_MARKER + SECURITY_DIR + 
				DIR_MARKER;
	}

	return byzantinedir;
}

String
convertDNtoFileName(const Name& name)
{
	String dnstr = name.getDN();
	Bstream md5hash = messageDigest(dnstr, md5);
	String md5hexstr = md5hash.gethexstr();

	// XXX For DOS based systems we need to
	// truncate this down to 8 characters.

	return (md5hexstr);
}
