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

#pragma ident "@(#)ca.h	1.12 96/01/29 Sun Microsystems"

#ifndef CA_H
#define CA_H

#define CANAME_FILE		"CA_Name"
#define CA_PRIVKEY_FILE		"CA_private_key"
#define CA_CERTREQ_FILE		"CA_certreq"
#define CA_CERT_FILE		"CA_Cert"
#define NEXT_SERIALNUM_FILE 	"next_serialnum"
#define CA_CRL_FILE		"CA_CRL"
#define CERT_DATABASE_DIR	"certs"
#define CA_SEED_FILE		"CA_seed"
#define DIR_MARKER		"/"

#define NULL_SURRENDER_PTR	((A_SURRENDER_CTX *)NULL_PTR)

extern Boolean camode;

/*
 * Some defaults
 */

#define DEFAULT_CA_KEYSIZE	1024
#define DEFAULT_DH_EXPONENT_SIZE	256
#define DEFAULT_DH_EXPONENT_FILE	"dh_secret"
#define DEFAULT_DH_PARAMS_FILE	"dh_params"
#define DEFAULT_DH_CERTREQ_FILE	"dh_certreq"

/*
 * following functions are defined in cafuncs.C
 */
extern int get_certreq_params(const Bstream&, Name&, PubKey&);
extern Bigint getSerialnum();
extern Bstream getCAprivkey(const Bstream&, String&);
extern X509Cert getX509CertbySerialNumber(const Bigint&);
extern X509Cert getCAcert();
extern Boolean allow_certification(Name&, PubKey&);

#endif CA_H
