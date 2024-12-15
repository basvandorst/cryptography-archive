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
#pragma ident "@(#)cert_bn_glue.C	1.5 96/01/29 Sun Microsystems"

//
// This file contains coversion routines to and from BigNum
// to Bigint/Bstream.
//

#include <stdlib.h>
#include "Bigint.h"
#include "Bstream.h"
#include "bn.h"

void
Bigint_to_BigNum(const Bigint *bint, BigNum *bn)
{
        int n;
        unsigned char *ptr;
        Bstream temp;

        n = (*bint).bits()/8;
        temp = Bigint_to_Bstr(*bint);     // this temp is necessary!
        ptr = temp.getdatap();
        bnInsertBigBytes(bn, ptr, 0,  n);
}

void
BigNum_to_Bigint(const BigNum *bn, Bigint *bint)
{
        unsigned char *s;
        unsigned n = (bnBits(bn)+7)/8;

        s = (unsigned char *)malloc(n);
        bnExtractBigBytes(bn, s, 0, n);
	*bint = Bigint(s, n);
	free((char *)s);
	
}

void
Bstream_to_BigNum(const Bstream *str, BigNum *bn)
{
        int n;
        unsigned char *ptr;

        n = str->getlength();
        ptr = str->getdatap();
        bnInsertBigBytes(bn, ptr, 0,  n);
}

void
BigNum_to_Bstream(const BigNum *bn, Bstream *str)
{
        unsigned char *s;
        unsigned n = (bnBits(bn)+7)/8;

        s = (unsigned char *)malloc(n);
        bnExtractBigBytes(bn, s, 0, n);
        *str = Bstream(n, s);
	free((char *)s);
}
