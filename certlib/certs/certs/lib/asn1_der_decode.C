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
#pragma ident "@(#)asn1_der_decode.C	1.5 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "my_types.h"
#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "asn1_der.h"
#include "ObjId.h"

/*
 * Encoding decoding routines for ASN.1 using
 * the Basic Encoding Rules subset DER
 */

void asn1_perror(int errcode)
{
	printf("ASN.1 decoding error: ");
	switch(errcode) {
	case E_DER_DECODE_LEN_TOO_LONG:
		printf("length octets exceed length of buffer\n");
		break;
	case E_DER_DECODE_BAD_ID_OCTET:
		printf("unexpected ID octet\n");
		break;
	case E_DER_DECODE_UNEXPECTED_EOS:
		printf("unexpected end-of-stream\n");
		break;
	case E_DER_DECODE_BAD_BITSTRING:
		printf("bad bitstring\n");
		break;
	case E_DER_DECODE_UNALIGNED_BS:
		printf("bitsring not octet aligned\n");
		break;
	case E_DER_DECODE_BAD_OBJID_SUBID:
		printf("bad sub-identifier of OBJECT-IDENTIFIER\n");
		break;
	case E_DER_DECODE_BAD_TIME_STRING:
		printf("bad UTCTime string\n");
		break;
	case E_DER_DECODE_BAD_NULL_VALUE:
		printf("bad null value, length not equal to zero\n");
		break;
	default:
		printf("unknown error code %d\n", errcode);
		break;
	}
	return;
}
int convert_bytes_to_int(byte *four_bytes)
{
#if defined(sparc)
	int val;
	byte *bytep = (byte *)&val;

	bytep[0] = four_bytes[3];
	bytep[1] = four_bytes[2];
	bytep[2] = four_bytes[1];
	bytep[3] = four_bytes[0];
	return (val);
#else 
#if defined(i386)
        int val;
        byte *bytep = (byte *)&val;
 
        bytep[0] = four_bytes[0];
        bytep[1] = four_bytes[1];
        bytep[2] = four_bytes[2];
        bytep[3] = four_bytes[3];
        return (val);
#else

	return "Architecture not supported";
#endif /* i386 */
#endif /* sparc */
}

/*
 * Do ASN.1 DER length octets processing.
 * We assume that the number of content
 * octets will never exceed 2**32 octets.
 */
int asn1_der_get_length(Bstream& der_stream)
{
	byte	tmp = 0;
	int 	len;
	const int MAXLENBYTES = 4; /*Greater than 32 bits should not be required*/
	byte    lenbytes[MAXLENBYTES];
	int 	i, ret;

	ret = der_stream.fetchbyte(tmp);
	if (ret == EOS)
		return (E_DER_DECODE_UNEXPECTED_EOS);

#ifdef __GNUC__
	if ((tmp & 0x80) == 0 ) {
#else
	if ((tmp & 0x80) == NULL ) {
#endif
		len = tmp;
	} else {

		int  numbytes = tmp & 0x7f;
		if (numbytes > MAXLENBYTES)
			return (E_DER_DECODE_LEN_TOO_LONG);
		if (numbytes > der_stream.getlength())
			return (E_DER_DECODE_UNEXPECTED_EOS);
		bzero(lenbytes, sizeof(lenbytes));
		for (i = numbytes - 1; i >= 0; i--)
			(void ) der_stream.fetchbyte(lenbytes[i]);
		len = convert_bytes_to_int(lenbytes);
	}
	return (len);
}

/*
 * Decode an ASN.1 DER INTEGER
 */
int asn1_der_decode_integer(Bstream& der_stream, Bigint& val)
{
	byte tmp = 0;
	int  len;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != INTEGER_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len > der_stream.getlength()) 
		return (E_DER_DECODE_UNEXPECTED_EOS);
	Bigint tmpval((unsigned char *)der_stream.getdatap(), len);
	der_stream.consume(len);	
	val = tmpval;
	return (0);
}

/*
 * Decode the ID part of a SEQUENCE
 */
int asn1_der_decode_sequence(Bstream& der_stream, int &len)
{
	byte tmp = 0;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != SEQUENCE_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len > der_stream.getlength()) {
		return (E_DER_DECODE_UNEXPECTED_EOS);
	}
	return (SUCCESS);
}

int asn1_der_decode_sequence_of(Bstream& der_stream, int &len)
{
	return (asn1_der_decode_sequence(der_stream, len));
}

/*
 * Decode the ID part of a SET
 */
int asn1_der_decode_set(Bstream& der_stream, int &len)
{
	byte tmp = 0;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != SET_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len > der_stream.getlength()) {
		return (E_DER_DECODE_UNEXPECTED_EOS);
	}
	return (SUCCESS);
}

/*
 * Decode NULL type
 */
int asn1_der_decode_null(Bstream& der_stream)
{
	byte tmp = 0;
	int  len;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != NULL_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len != 0)
		return(E_DER_DECODE_BAD_NULL_VALUE);
	return (SUCCESS);
}

int asn1_der_decode_set_of(Bstream& der_stream, int &len)
{
	return (asn1_der_decode_set(der_stream, len));
}

/*
 * Decode a BIT-STRING
 */
int asn1_der_decode_bit_string(Bstream& der_stream, Bstream& bitstring)
{
	byte tmp = 0;
	int  len, ret;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != BIT_STRING_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len == 0) return (E_DER_DECODE_BAD_BITSTRING);
	if (len > der_stream.getlength()) 
		return (E_DER_DECODE_UNEXPECTED_EOS);
	ret = der_stream.fetchbyte(tmp);
	if (ret == EOS)
		return (SUCCESS);
	if (len == 1) {
		if (tmp == 0) return (SUCCESS);
		else return (E_DER_DECODE_BAD_BITSTRING);
	}
	if (tmp > 7 ) 
		return (E_DER_DECODE_BAD_BITSTRING);

	// Special Hack for certificates, which
	// are assumed to produce octet aligned quantities
	if (tmp != 0)
		return (E_DER_DECODE_UNALIGNED_BS);

	Bstream tmpstr(len - 1, der_stream.getdatap());	
	der_stream.consume(len - 1);
	bitstring = tmpstr;
	return (SUCCESS);
}

/*
 * Decode an OCTET-STRING
 */
int asn1_der_decode_octet_string(Bstream& der_stream, Bstream& octetstring)
{
	byte tmp = 0;
	int  len;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != OCTET_STRING_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len > der_stream.getlength()) 
		return (E_DER_DECODE_UNEXPECTED_EOS);
	Bstream tmpstr(len, der_stream.getdatap());	
	der_stream.consume(len);
	octetstring = tmpstr;
	return (SUCCESS);
}

/*
 * Decode a PRINTABLE-STRING
 */
int asn1_der_decode_printable_string(Bstream& der_stream, Bstream& prstring)
{
	byte tmp = 0;
	int  len;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != PRINTABLE_STRING_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len > der_stream.getlength()) {
		return (E_DER_DECODE_UNEXPECTED_EOS);
	}
	Bstream tmpstr(len, der_stream.getdatap());	
	der_stream.consume(len);
	prstring = tmpstr;
	return (SUCCESS);
}

/*
 * Decode a T61-STRING
 */
int asn1_der_decode_T61_string(Bstream& der_stream, Bstream& prstring)
{
	byte tmp = 0;
	int  len;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != T61_STRING_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len > der_stream.getlength()) 
		return (E_DER_DECODE_UNEXPECTED_EOS);
	Bstream tmpstr(len, der_stream.getdatap());	
	der_stream.consume(len);
	prstring = tmpstr;
	return (SUCCESS);
}

inline void  get_twodigits(Bstream& tmpstr, byte *two_digits)
{
	(void) tmpstr.fetchbyte(two_digits[0]);
	(void) tmpstr.fetchbyte(two_digits[1]);
}

/*
 * Decode a UTCTime
 */
int asn1_der_decode_utctime(Bstream& der_stream, PCTime& time)
{
	byte tmp = 0;
	int  len;
	byte two_digits[3];
	char sign;
	int year, month, day, hour, min, offhr, offmin;
	int sec = 0;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	(void )der_stream.peekbyte(tmp);
	if (tmp != UTCTime_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len > der_stream.getlength()) 
		return (E_DER_DECODE_UNEXPECTED_EOS);
	Bstream tmpstr(len, der_stream.getdatap());
	der_stream.consume(len);
	// Parse the Time string
	if (len < 11 || len > 17) {
		return (E_DER_DECODE_BAD_TIME_STRING);
	}
	two_digits[2] = 0;	// null terminate 
	get_twodigits(tmpstr, two_digits);
	year = atoi((char *)two_digits);
	// Convert last two digits to full year, including century.
	// For this we assume that anything less than 80 is 20XX
	// and anything more than or equal to 80 is 19XX.
	// Is there a better way of doing this? XXXX
	if (year < 80) {
		year += 2000;	// Fix this piece of code after year 2080!!
	} else {
		year += 1900;
	}
	get_twodigits(tmpstr, two_digits);
	month = atoi((char *)two_digits);
	get_twodigits(tmpstr, two_digits);
	day = atoi((char *)two_digits);
	get_twodigits(tmpstr, two_digits);
	hour = atoi((char *)two_digits);
	get_twodigits(tmpstr, two_digits);
	min = atoi((char *)two_digits);
	if (len == 13 || len == 17) { // secs present
		get_twodigits(tmpstr, two_digits);
		sec = atoi((char *)two_digits);
	}
	(void)tmpstr.peekbyte(tmp);
	if ((char)tmp != 'Z') {
		tmpstr.fetchbyte(tmp);
		sign = (char)tmp;
		if (sign != '-' || sign != '+') {
			return (E_DER_DECODE_BAD_TIME_STRING);
		}
		get_twodigits(tmpstr, two_digits);
		offhr = atoi((char *)two_digits);
		get_twodigits(tmpstr, two_digits);
		offmin = atoi((char *)two_digits);
		if (sign == '-') {
			hour -= offhr;
			min  -= offmin;
		} else {
			hour += offhr;
			min  += offmin;
		}
	}
	if ( (month > 12) || (day > 31) || (hour > 23) || (min > 59) ||
	     (sec > 59) ) {
		return (E_DER_DECODE_BAD_TIME_STRING);
	}		
	PCTime tmptime(year, month, day, hour, min, sec);
	time = tmptime;
	return (SUCCESS);
}


byte num_to_mask(int i)
{
	switch (i % 7) {
	case 0:
		return (0x80);	// shifted left seven, top 1 bit significant
	case 1:
		return (0xC0);	// shifted left six, top 2 bits significant
	case 2:
		return (0xE0);	// shifted left five, top 3 bits significant
	case 3:
		return (0xF0);	// shifted left four, top 4 bits
	case 4:
		return (0xF8);	// etc.
	case 5:
		return (0xFC);
	case 6:
		return (0xFE);
	default:
		printf("num_to_mask: unreachable code\n");
		return (0);
	}
}

/*
 * Decode an OBJECT-IDENTIFIER
 */
int asn1_der_decode_obj_id(Bstream& der_stream, ObjId& objid)
{
	byte tmp = 0;
	byte tmp1 = 0;
	int  len, tmp_len;
	int  retval;

	if (der_stream.getlength() < 2)
		return (E_DER_DECODE_UNEXPECTED_EOS);
	retval = der_stream.peekbyte(tmp);
	if (tmp != OBJECT_IDENTIFIER_ID) {
		return (E_DER_DECODE_BAD_ID_OCTET);
	}
	(void )der_stream.fetchbyte(tmp);
	len = asn1_der_get_length(der_stream);
	if (len < 0) return len;
	if (len > der_stream.getlength()) 
		return (E_DER_DECODE_UNEXPECTED_EOS);
	int first_subid = TRUE;
	while (len > 0) {
		// For each sub id, figure out size by peeking
		// into the stream.
		int subid_len = 0;
		for ( (void) der_stream.peekbyte(tmp); (tmp & 0x80) ; 
					(void)der_stream.peekbyte(tmp)) {
			len--; 
			if (len < 0) 
				return (E_DER_DECODE_BAD_OBJID_SUBID);
			if (subid_len == 0 && tmp == 0x80)
				return (E_DER_DECODE_BAD_OBJID_SUBID);
			subid_len++;
		}
		subid_len++;			// account for last octet
		len--; 
		if (len < 0) 
			return (E_DER_DECODE_BAD_OBJID_SUBID);
		byte *subid_bits = new byte[subid_len];
		tmp_len = subid_len;
		while (tmp_len) {
			der_stream.fetchbyte(subid_bits[tmp_len - 1]);
			tmp_len--;
		}
		byte *subid_int = new byte[subid_len];
		bzero(subid_int, subid_len);
		tmp_len = subid_len;
		for (int i = 0; i < tmp_len; i++) {
			tmp = ((byte)(subid_bits[i] & 0x7f) >> (i % 7));
			if ( i == (tmp_len - 1)) { // Most significant octet 
				tmp1 = 0;
			} else {
				tmp1 = ((subid_bits[i+1] & 0x7f) << (7-(i%7)));
				tmp1 &= num_to_mask(i);
			}
			tmp |= tmp1;
			subid_int[tmp_len - (i+1)] = tmp;
		}
		Bigint I((unsigned char *)subid_int, subid_len);
		delete subid_bits;
		delete subid_int;
		if (first_subid) {
			Bigint X,Y;	// X, Y are 1st and 2nd subids rsp.
			if (I < 80) {	// X == 0, or X == 1
				if (I < 40)
					X = (short)0;
				else
					X = 1;
			} else {		// X == 2
				X = 2;
			}
 			Y = I - X*40;
			objid.add_element(X);
			objid.add_element(Y);
			first_subid = FALSE;
		} else {
			objid.add_element(I);
		}
	}
	return (SUCCESS);
}
