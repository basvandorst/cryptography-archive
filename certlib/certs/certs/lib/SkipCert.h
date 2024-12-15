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

#pragma ident "@(#)SkipCert.h	1.11 96/01/29 Sun Microsystems"

#define SKIP_CERT_X509          (1)
#define SKIP_CERT_PGP           (2)
#define SKIP_CERT_DNS           (3)
#define SKIP_DH_PUBLIC          (4)

class SkipCert {
public:
	int CertType;
	int validated;
	virtual skip_params(Bigint&, Bigint&)=0;
	virtual Bigint skip_pubkey()=0;
	virtual Bstream skip_name()=0;
	virtual int decode(Bstream)=0;
	virtual int isValid(Bstream)=0;
	virtual Bstream encode()=0;
	virtual u_long  skip_notvalidafter()=0;
	virtual u_long  skip_notvalidbefore()=0;
	virtual void print()=0;
};	
SkipCert *decode_SkipCert(int, Bstream);
char *num_to_Certname(int n);
int  Certname_to_num(char *);
