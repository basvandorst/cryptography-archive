/*
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

#pragma ident	"@(#)print_cert.C	1.6 96/01/29"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream.h>

#include <Bstream.h>
#include <Bigint.h>
#include <SkipCert.h>

void usage(char *argv){
	fprintf(stderr,
		"%s -t type certificate-file-name\n", argv);
	exit(1);
}
	
main(int argc, char *argv[])
{
	int c;
	SkipCert *cert;
	Bstream x;
	int type = SKIP_CERT_X509;
	extern char *optarg;
	extern int  optind;

	while ((c = getopt(argc, argv, "t:")) != EOF) 
	switch(c) {
		case 't':
			type=atoi(optarg);
			if (type == 0)
				type=Certname_to_num(optarg);
			if (type == 0){
				fprintf(stderr,
				"unsupported certificate type\n");
				exit(1);
			}
			break;
		default:
			usage(argv[0]);
	}
	if (optind >= argc) 
		usage(argv[0]);
	x=File_to_Bstr(argv[optind]);
	if (x.getlength() == 0) {
		fprintf(stderr,
			"Certificate %s not found\n",
			argv[optind]);
		exit(1);
	}
	cert = decode_SkipCert(type, x);
	cert->print();
}
