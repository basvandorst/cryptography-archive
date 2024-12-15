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
#pragma ident "@(#)asn1_der_encode.C	1.8 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdarg.h>
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

/*
 * ASN.1 DER length octets encoding.
 * We assume that the number of content
 * octets will never exceed 2**32 octets.
 * Take in number of bytes and return
 * BER encoded length octets.
 */
Bstream
asn1_der_set_length(int len)
{
	int val = len;
	byte *bytep = (byte *)&val;

#if	defined(sparc)
	if (len < 128) {
		byte lenoctets[1];
		lenoctets[0] = bytep[3];
		return Bstream(sizeof(lenoctets), lenoctets);
	}
	if (len < 256) {
		byte lenoctets[2];
		lenoctets[0] = 0x81;
		lenoctets[1] = bytep[3];
		return Bstream(sizeof(lenoctets), lenoctets);
	} 
	if (bytep[0] == 0 && bytep[1] == 0) {
		byte lenoctets[3];
		lenoctets[0] = 0x82;
		lenoctets[1] = bytep[2];	// MSB here
		lenoctets[2] = bytep[3];	// LSB here
		return Bstream(sizeof(lenoctets), lenoctets);
	} 
	if (bytep[0] == 0) {
		byte lenoctets[4];
		lenoctets[0] = 0x83;
		lenoctets[1] = bytep[1];	// MSB here
		lenoctets[2] = bytep[2];
		lenoctets[3] = bytep[3];	// LSB here
		return Bstream(sizeof(lenoctets), lenoctets);
	} else {
		byte lenoctets[5];
		lenoctets[0] = 0x84;
		lenoctets[1] = bytep[1];
		lenoctets[2] = bytep[2];
		lenoctets[3] = bytep[3];
		lenoctets[4] = bytep[4];
		return Bstream(sizeof(lenoctets), lenoctets);
	}
#else
#if defined(i386)
        if (len < 128) {
                byte lenoctets[1];
                lenoctets[0] = bytep[0];
                return Bstream(sizeof(lenoctets), lenoctets);
        }
        if (len < 256) {
                byte lenoctets[2];
                lenoctets[0] = 0x81;
                lenoctets[1] = bytep[0];
                return Bstream(sizeof(lenoctets), lenoctets);
        }
        if (bytep[3] == 0 && bytep[2] == 0) {
                byte lenoctets[3];
                lenoctets[0] = 0x82;
                lenoctets[1] = bytep[1];        // MSB here
                lenoctets[2] = bytep[0];        // LSB here
                return Bstream(sizeof(lenoctets), lenoctets);
        }
	if (bytep[3] == 0) {
                byte lenoctets[4];
                lenoctets[0] = 0x83;
                lenoctets[1] = bytep[3];        // MSB here
                lenoctets[2] = bytep[2];
                lenoctets[3] = bytep[1];        // LSB here
                return Bstream(sizeof(lenoctets), lenoctets);
        } else {
                byte lenoctets[5];
                lenoctets[0] = 0x84;
                lenoctets[1] = bytep[3];
                lenoctets[2] = bytep[2];
                lenoctets[3] = bytep[1];
                lenoctets[4] = bytep[0];
                return Bstream(sizeof(lenoctets), lenoctets);
        }
#else
	return "Archictecture not supported";
#endif /* i386 */
#endif /* sparc */

}

/*
 * Encode an TAG value 
 */
Bstream
asn1_der_encode_implicit_tagged(const Bstream& val, int tagvalue)
{
	Bstream tmpstr(val);
	byte oldtype;
	(void)tmpstr.fetchbyte(oldtype);
	byte ID = 0x80 | (oldtype & 0x20) | (tagvalue & 0x1f) ;
	Bstream IDstr(1, &ID);
	return (IDstr + tmpstr);
}

/*
 * Encode an SNMP TimeTicks
 */
Bstream
asn1_der_encode_timeticks(const Bigint& val)
{
	byte ID = 0x43;
	Bstream IDstr(1, &ID);
	Bstream intstr = Bigint_to_Bstr(val);
	Bstream lenstr = asn1_der_set_length(intstr.getlength());
	return (IDstr + lenstr + intstr);
}

/*
 * Encode an SNMP NetworkAddress
 */
Bstream
asn1_der_encode_network_addr(unsigned long addr)
{
	byte ID = 0x40;
	Bstream IDstr(1, &ID);
	Bstream intstr(sizeof(addr), (byte *)&addr);
	Bstream lenstr = asn1_der_set_length(intstr.getlength());
	return (IDstr + lenstr + intstr);
}

/*
 * Encode an INTEGER
 */
Bstream
asn1_der_encode_integer(const Bigint& val)
{
	byte ID = INTEGER_ID;
	Bstream IDstr(1, &ID);
	Bstream intstr = Bigint_to_Bstr(val);
	Bstream lenstr = asn1_der_set_length(intstr.getlength());
	return (IDstr + lenstr + intstr);
}

/*
 * Encode a SEQUENCE
 */
Bstream
asn1_der_encode_sequence(const Bstream& val)
{
	byte ID = SEQUENCE_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}

Bstream
asn1_der_encode_sequence_of(const Bstream& val)
{
	return (asn1_der_encode_sequence(val));
}

/*
 * Encode a SET
 */
Bstream
asn1_der_encode_set(const Bstream& val)
{
	byte ID = SET_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}

/*
 * Encode NULL type
 */
Bstream
asn1_der_encode_null()
{
	byte ID = NULL_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(0);
	return (IDstr + lenstr);
}

/*
 *
 * Encode a SET-OF
 */
Bstream
asn1_der_encode_set_of(const Bstream& val)
{
	byte ID = SET_OF_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}


/*
 * Encode a BIT-STRING
 */
Bstream
asn1_der_encode_bit_string(const Bstream& val)
{
	byte ID = BIT_STRING_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength()+1);
	byte unusedbits = 0;
	Bstream unused(1, &unusedbits);
	return (IDstr + lenstr + unused + val);
}


/*
 * Encode an OCTET-STRING
 */
Bstream
asn1_der_encode_octet_string(const Bstream& val)
{
	byte ID = OCTET_STRING_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}

/*
 * Encode a PRINTABLE-STRING
 */
Bstream
asn1_der_encode_printable_string(const Bstream& val)
{
	byte ID = PRINTABLE_STRING_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}

/*
 * Encode a T61-STRING
 */
Bstream
asn1_der_encode_T61_string(const Bstream& val)
{
	byte ID = T61_STRING_ID;
	Bstream IDstr(1, &ID);
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}

inline int century(int year)
{
	return ((year/100)*100);
}

/*
 * Encode a UTCTime
 */
Bstream
asn1_der_encode_utctime(const PCTime& time)
{
	byte ID = UTCTime_ID;
	Bstream IDstr(1, &ID);
	char	buf[13];
	GMtime gtime = time.get();
	gtime.year = gtime.year - century(gtime.year);
	sprintf(buf, "%.2d", gtime.year);
	sprintf(buf+2, "%.2d", gtime.month);
	sprintf(buf+4, "%.2d", gtime.day);
	sprintf(buf+6, "%.2d", gtime.hour);
	sprintf(buf+8, "%.2d", gtime.min);
	sprintf(buf+10, "%.2d", gtime.sec);
	sprintf(buf+12, "Z");
	Bstream lenstr = asn1_der_set_length(sizeof(buf));
	Bstream val =  Bstream(sizeof(buf), (byte *)buf);
	return (IDstr + lenstr + val);
}

/*
 * Encode an OBJECT-IDENTIFIER
 */
Bstream
asn1_der_encode_obj_id(const ObjId& objid)
{
	return (objid.encode());
}

