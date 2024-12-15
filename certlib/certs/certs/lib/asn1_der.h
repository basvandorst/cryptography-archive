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

#pragma ident "@(#)asn1_der.h	1.7 96/01/29 Sun Microsystems"

#ifndef ASN1_DER_H
#define ASN1_DER_H

/*
 * ASN.1 BER Identifier octet
 */
const int INTEGER_ID =			0x02;
const int BIT_STRING_ID	=		0x03;
const int OCTET_STRING_ID =		0x04;
const int NULL_ID =			0x05;
const int OBJECT_IDENTIFIER_ID	= 	0x06;
const int PRINTABLE_STRING_ID =		0x13;
const int T61_STRING_ID =		0x14;
const int UTCTime_ID = 			0x17;
const int SEQUENCE_ID = 		0x30;
const int SEQUENCE_OF_ID =		0x30;
const int SET_ID = 			0x31;
const int SET_OF_ID = 			0x31;

/*
 * ASN.1 DER DECODING ERRORS
 */
const int E_DER_DECODE_LEN_TOO_LONG = 	-2;
const int E_DER_DECODE_BAD_ID_OCTET = 	-3;
const int E_DER_DECODE_UNEXPECTED_EOS = -4;
const int E_DER_DECODE_BAD_BITSTRING =  -5;
const int E_DER_DECODE_UNALIGNED_BS =   -6; /* Bitstring not byte aligned */
const int E_DER_DECODE_BAD_OBJID_SUBID = -7;
const int E_DER_DECODE_BAD_TIME_STRING = -8;
const int E_DER_DECODE_BAD_NULL_VALUE = -9;


/*
 * Macros to facilitate ASN.1 decoding
 */

#define DATAP		(der_stream.getdatap())
#define CONSUME(len) 	(der_stream.consume(len))
#define MARK_LEN(mark)	(mark = der_stream.getlength())
#define	SEQUENCE   						      \
	{							      \
		retval = asn1_der_decode_sequence(der_stream, seqlen);\
		if (retval < 0) { 				      \
			/* printf("SEQUENCE decode:"); */	      \
			return (retval);			      \
		}						      \
	}

#define	BEGIN_SEQUENCE_OF 					      \
	{							      \
		retval = asn1_der_decode_sequence_of(der_stream, seqoflen);\
		if (retval < 0) { 				      \
			/* printf("SEQUENCE decode:");	*/	      \
			return (retval);			      \
		}						      \
	}							      \
	while (seqoflen > 0)	{				      \
			seqlenwhdr = der_stream.getlength();

#define END_SEQUENCE_OF consumed = seqlenwhdr - der_stream.getlength(); \
			seqoflen -= consumed;			      \
	}



#define	INTEGER(integer)  				      	      \
	{						      	      \
		retval = asn1_der_decode_integer(der_stream, integer);\
	    	if (retval < 0) { 				      \
			/* printf("INTEGER decode:"); */	      \
			return (retval);			      \
	    	}						      \
   	}

#define	ASN1_NULL				      	      	      \
	{						      	      \
		retval = asn1_der_decode_null(der_stream);   	      \
	    	if (retval < 0) { 				      \
			/* printf("ASN1_NULL decode:");	 */    	      \
			return (retval);			      \
	    	}						      \
   	}

#define	UTCTime(time)  				      	      	      \
	{						      	      \
		retval = asn1_der_decode_utctime(der_stream, time);   \
	    	if (retval < 0) { 				      \
			/* printf("UTCTime decode:"); */	      \
			return (retval);			      \
	    	}						      \
   	}

#define	BIT_STRING(bitstring)  				      	      \
	{						      	      \
		retval = asn1_der_decode_bit_string(der_stream, bitstring); \
	    	if (retval < 0) { 				      \
			/* printf("UTCTime decode:"); */	      \
			return (retval);			      \
	    	}						      \
   	}

#define ALGID(algid)						       \
	{							      \
		retval = asn1_der_decode_algid(der_stream, algid);    \
	        if (retval < 0) {				      \
			/* printf("ALGID decode:"); */		      \
			return (retval);			      \
		}						      \
	}

#define OCTET_STRING(string)					       \
	{							      \
		retval = asn1_der_decode_octet_string(der_stream, string);  \
	        if (retval < 0) {				      \
			/* printf("OCTET-STRING decode:"); */	      \
			return (retval);			      \
		}						      \
	}


/*
 * C++ linkage.
 */
extern int asn1_der_get_length(Bstream&);
extern int asn1_der_decode_integer(Bstream&, Bigint&);
extern int asn1_der_decode_utctime(Bstream&, PCTime&);
extern int asn1_der_decode_sequence(Bstream&, int&);
extern int asn1_der_decode_sequence_of(Bstream&, int&);
extern int asn1_der_decode_set(Bstream&, int&);
extern int asn1_der_decode_set_of(Bstream&, int&);
extern int asn1_der_decode_printable_string(Bstream&, Bstream&);
extern int asn1_der_decode_bit_string(Bstream&, Bstream&);
extern int asn1_der_decode_octet_string(Bstream&, Bstream&);
extern int asn1_der_decode_null(Bstream&);
extern void asn1_perror(int );

extern Bstream asn1_der_encode_integer(const Bigint&);
extern Bstream asn1_der_encode_sequence(const Bstream&);
extern Bstream asn1_der_encode_sequence_of(const Bstream&);
extern Bstream asn1_der_encode_set(const Bstream&);
extern Bstream asn1_der_encode_set_of(const Bstream&);
extern Bstream asn1_der_encode_null();
extern Bstream asn1_der_encode_bit_string(const Bstream&);
extern Bstream asn1_der_encode_octet_string(const Bstream&);
extern Bstream asn1_der_encode_printable_string(const Bstream&);
extern Bstream asn1_der_encode_T61_string(const Bstream&);
extern Bstream asn1_der_encode_utctime(const PCTime&);
extern Bstream asn1_der_encode_network_addr(unsigned long);
extern Bstream asn1_der_encode_timeticks(const Bigint&);
extern Bstream asn1_der_encode_implicit_tagged(const Bstream&, int);

extern Bstream asn1_der_set_length(int);
extern byte num_to_mask(int);

#endif ASN1_DER_H
