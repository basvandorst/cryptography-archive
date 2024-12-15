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
#pragma ident "@(#)test.C	1.12 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "my_types.h"
#include "Time.h"
#include "Bstream.h"
#include "Bigint.h"
#include "asn1_der.h"
#include "SkipCert.h"
#include "HashCert.h"


void test2();
void test1();
int bits_in_Bigint(Bigint b);

main()
{
	Bigint X("00889fba340000000000010000000000");
	Bigint Y("7089fba340000000000010000000000");
	Bigint Z("25");

	if (X != Y) printf("X != Y\n");
	else printf("X == Y\n");
	printf("X= "); X.print(); printf("\n");
	printf("Y= "); Y.print(); printf("\n");
	Bstream x=Bigint_to_Bstr(X);
	x.print();
	test2();
	printf("\n%d\n",X.bits());
	printf("\n%d\n",X.bits());
	HashCert cert(X,Y,Z, time(0)+UNIXSTART, time(0)+1000+UNIXSTART);
	HashCert  empty;
	Bstream certstr=cert.encode();
	cert.skip_name().print();
	cert.print();
        SkipCert *gencert;
	gencert=decode_SkipCert(SKIP_DH_PUBLIC, certstr);
	gencert->print();
}

void test1()
{
	Bigint a(2);
	Bigint b(65000);
	printf("a= "); a.print(); printf("\n");
	printf("b= "); b.print(); printf("\n");

}

void test2()
{
	unsigned char  buf[] = { 0x80, 0x90, 0x10, 0x91};
	int len = sizeof(buf);
	for (int i = 0; i< len; i++)
		printf("buf[%d] = 0x%x\n", i, buf[i]);
	Bigint x (buf, len);

	x.print();
}

