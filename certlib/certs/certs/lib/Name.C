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

#pragma ident "@(#)Name.C	1.6 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "Time.h"
#include "Bigint.h"
#include "Bstream.h"
#include "ObjId.h"
#include "asn1_der.h"
#include "Name.h"

/*
 * Name class implementation.
 */
#ifndef __GNUC__
Name::Name(int num_elems, ...)
{
	Ava	elem_val;
	va_list		ap;

	avalist = NULL;
	va_start(ap, num_elems);
	while (num_elems--) {
		elem_val = va_arg(ap, Ava);
		add_element(elem_val);
	}
	va_end(ap);
}
#endif
Name::~Name()
{
	Ava	*avap = avalist;
	while (avap) {
		Ava	*tmpavap = avap->next;
		delete avap;
		avap = tmpavap;
	}
	avalist = NULL;
}

ObjId
get_str_attrtype(Bstream& strname)
{
	byte id[4];

	bzero(id, sizeof(id));
	(void)strname.fetchbyte(id[0]);
	(void)strname.fetchbyte(id[1]);
	(void)strname.fetchbyte(id[2]);
	if (bcmp(id, CON, 3) == 0) {
		return (countryName);
	} else if (bcmp(id, ORG, 3) == 0) {
		return (orgName);
	} else if (bcmp(id, STA, 3) == 0) {
		return (stateName);
	} else if (bcmp(id, LOC, 3) == 0) {
		return (localityName);
	} 
	(void)strname.fetchbyte(id[3]);
	if (bcmp(id, IPA, 4) == 0) {
		return (ipAddress);
	} else if (bcmp(id, OGU, 4) == 0) {
		return (orgUnitName);
	} else if (bcmp(id, CNM, 4) == 0) {
		return (commonName);
	} else if (bcmp(id, TIT, 4) == 0) {
		return (title);
	}
	printf("about to return null OID\n");
	return (NULL);
}

Bstream
get_str_attrvalue(Bstream& strname)
{
	byte tmp = 0;
	int len, origlen;

	Bstream val = strname;
	origlen = strname.getlength();
	for (len= 0, tmp = 0; tmp != '/'; len++) {
		(void) strname.peekbyte(tmp);
		if (len == origlen)
			break;
	}
	if (len != origlen)
		len--;		// account for /, already seen
	val.truncate(origlen - len);
	strname.consume(len);
	return (val);
}

Name::Name(const char *name)
{
	ObjId nulloid;
	avalist = NULL;
	Bstream charname(strlen(name), (byte *)name);
	while (charname.getlength() > 0) {
		Ava ava;
		ava.attributetype = get_str_attrtype(charname);
		if (ava.attributetype == nulloid) {
			printf("Invalid Name syntax\n");
// XXX
#ifndef __GNUC__
			Name::~Name();
#endif
			break;
		}
		ava.attributevalue = get_str_attrvalue(charname);
		ava.next = NULL;
		add_element(ava);
	}
}

Name::Name(const Name &a)
{
	Ava	*avap = a.avalist;
	Ava	*tmpavap, *lastavap;

	lastavap = NULL;
	while (avap) {
		tmpavap = new Ava;
		tmpavap->attributetype = avap->attributetype;
		tmpavap->attributevalue = avap->attributevalue;
		tmpavap->next = NULL;
		if (lastavap == NULL) {
			avalist = tmpavap;
		} else {
			lastavap->next = tmpavap;
		}
		lastavap = tmpavap;
		avap = avap->next;
	}
	return;
}

Name&
Name:: operator =(const Name &a)
{
	if (avalist == a.avalist)	// check for a = a;
		return (*this);

	// Release any old name info that may be present
// XXX - TM
#ifndef __GNUC__
	Name::~Name();
#endif
	Ava	*avap = a.avalist;
	Ava	*tmpavap, *lastavap;

	lastavap = NULL;
	while (avap) {
		tmpavap = new Ava;
		tmpavap->attributetype = avap->attributetype;
		tmpavap->attributevalue = avap->attributevalue;
		tmpavap->next = NULL;
		if (lastavap == NULL) {
			avalist = tmpavap;
		} else {
			lastavap->next = tmpavap;
		}
		lastavap = tmpavap;
		avap = avap->next;
	}
	return (*this);

}

Boolean operator ==(const Name& a, const Name& b)
{
	Ava	*aavap = a.avalist;
	Ava	*bavap = b.avalist;

	while (aavap) {
		if (bavap == NULL)
			return (BOOL_FALSE);
		if (aavap->attributetype != bavap->attributetype)
			return (BOOL_FALSE);
		if (aavap->attributevalue != bavap->attributevalue)
			return (BOOL_FALSE);
		bavap = bavap->next;
		aavap = aavap->next;
	}
	if (bavap != NULL)
		return (BOOL_FALSE);
	return (BOOL_TRUE);
}

Boolean operator !=(const Name& a, const Name& b)
{
	if (a == b)
		return (BOOL_FALSE);
	return (BOOL_TRUE);
}

void
Name::add_element(const Ava &newval)
{
	Ava	*avap = avalist;
	Ava	*lastavap;
	Ava	*newavap = new Ava;

	newavap->next = NULL;
	newavap->attributetype = newval.attributetype;
	newavap->attributevalue = newval.attributevalue;
	if (avap == NULL) {
		avalist = newavap;
		return;
	}
	while (avap != NULL) {
		lastavap = avap;
		avap = avap->next;
	}
	lastavap->next = newavap;
	return;
}

String
Name::getsuffix() const
{
	Ava	*avap = avalist;
	Ava	*lastavap;
	String	 suffix;

	if (avap == NULL) {
		return suffix;
	}
	while (avap != NULL) {
		lastavap = avap;
		avap = avap->next;
	}
	avap = lastavap;
	if (avap->attributetype == ipAddress) {
		suffix = "/IP=";
	} else if (avap->attributetype == commonName) {
		suffix = "/CN=";
	} else if (avap->attributetype == countryName) {
		suffix = "/C=";
	} else if (avap->attributetype == stateName) {
		suffix = "/S=";
	} else if (avap->attributetype == orgName) {
		suffix = "/O=";
	} else if (avap->attributetype == orgUnitName) {
		suffix = "/OU=";
	} else if (avap->attributetype == localityName) {
		suffix = "/L=";
	} else if (avap->attributetype == title) {
		suffix = "/T=";
	} else {
		suffix = "/OID="; 
		suffix += avap->attributetype.getoidstr();
	}
	suffix += avap->attributevalue.getstr();
	return suffix;
}


void
Name::print() const
{
	Ava	*avap = avalist;

	while (avap) {
		if (avap->attributetype == ipAddress) {
			printf("/IP=");
		} else if (avap->attributetype == commonName) {
			printf("/CN=");
		} else if (avap->attributetype == countryName) {
			printf("/C=");
		} else if (avap->attributetype == stateName) {
			printf("/S=");
		} else if (avap->attributetype == orgName) {
			printf("/O=");
		} else if (avap->attributetype == orgUnitName) {
			printf("/OU=");
		} else if (avap->attributetype == localityName) {
			printf("/L=");
		} else if (avap->attributetype == title) {
			printf("/T=");
		} else {
			printf("/OID="); avap->attributetype.print();
		}
		avap->attributevalue.prints();
		avap = avap->next;
	}
}

String
Name::getDN() const
{
	String dn;
	Ava	*avap = avalist;

	while (avap) {
		if (avap->attributetype == ipAddress) {
			dn += "/IP=";
		} else if (avap->attributetype == commonName) {
			dn += "/CN=";
		} else if (avap->attributetype == countryName) {
			dn += "/C=";
		} else if (avap->attributetype == stateName) {
			dn += "/S=";
		} else if (avap->attributetype == orgName) {
			dn += "/O=";
		} else if (avap->attributetype == orgUnitName) {
			dn += "/OU=";
		} else if (avap->attributetype == localityName) {
			dn += "/L=";
		} else if (avap->attributetype == title) {
			dn += "/T=";
		} else {
			dn += "/OID="; 
			dn += avap->attributetype.getoidstr();
		}
		dn += avap->attributevalue.getstr();
		avap = avap->next;
	}
	return (dn);
}

String
Name::getip() const
{
	Ava	*avap = avalist;
	while (avap) {
		if (avap->attributetype == ipAddress) {
			Bstream retbstr = avap->attributevalue;
		 	return (String((const char *)retbstr.getdatap(),
						     retbstr.getlength()));
		} else {
			avap = avap->next;
		}
	}
	return (String(""));
}

String
Name::getcn() const
{
	Ava	*avap = avalist;
	while (avap) {
		if (avap->attributetype == commonName) {
			Bstream retbstr = avap->attributevalue;
		 	return (String((const char *)retbstr.getdatap(),
						     retbstr.getlength()));
		} else {
			avap = avap->next;
		}
	}
	return (String(""));
}

String
Name::getou() const
{
	Ava	*avap = avalist;
	while (avap) {
		if (avap->attributetype == orgUnitName) {
			Bstream retbstr = avap->attributevalue;
		 	return (String((const char *)retbstr.getdatap(),
						     retbstr.getlength()));
		} else {
			avap = avap->next;
		}
	}
	return (String(""));
}

String
Name::getorg() const
{
	Ava	*avap = avalist;
	while (avap) {
		if (avap->attributetype == orgName) {
			Bstream retbstr = avap->attributevalue;
		 	return (String((const char *)retbstr.getdatap(),
							retbstr.getlength()));
		} else {
			avap = avap->next;
		}
	}
	return (String(""));
}

String
Name::getstate() const
{
	Ava	*avap = avalist;
	while (avap) {
		if (avap->attributetype == stateName) {
			Bstream retbstr = avap->attributevalue;
		 	return (String((const char *)retbstr.getdatap(),
							retbstr.getlength()));
		} else {
			avap = avap->next;
		}
	}
	return (String(""));
}

String
Name::getcountry() const
{
	Ava	*avap = avalist;
	while (avap) {
		if (avap->attributetype == countryName) {
			Bstream retbstr = avap->attributevalue;
		 	return (String((const char *)retbstr.getdatap(),
							retbstr.getlength()));
		} else {
			avap = avap->next;
		}
	}
	return (String(""));
}

/*
 * X.500 Distinguished Name DER decoding
 */
int asn1_der_decode_name(Bstream& der_stream, Name& name)
{
	byte tmp = 0;
	int  seqoflen, setoflen, seqlen, retval;
	int consumed, setoflenwhdr, seqlenwhdr;
	Name tmpname;

	retval = asn1_der_decode_sequence_of(der_stream, seqoflen);
	if (retval < 0) return (retval);
	while (seqoflen > 0) {
		setoflenwhdr = der_stream.getlength();
		retval = asn1_der_decode_set_of(der_stream, setoflen);
		if (retval < 0) return (retval);
		while (setoflen > 0) {
			Ava	ava;

			seqlenwhdr = der_stream.getlength();
			retval = asn1_der_decode_sequence(der_stream, seqlen);
			if (retval < 0) return (retval);
			retval = asn1_der_decode_obj_id(der_stream, 
							ava.attributetype);
			if (retval < 0) return (retval);
			// switch on OID to figure out attribute value 
			// ASN1 type XXX
			retval = asn1_der_decode_printable_string(der_stream,
							ava.attributevalue);
			if (retval < 0) return (retval);
			tmpname.add_element(ava);
			consumed = seqlenwhdr - der_stream.getlength();
			setoflen -= consumed;
		}
		consumed = setoflenwhdr - der_stream.getlength();
		seqoflen -= consumed;
	}
	name = tmpname;
	return (SUCCESS);
}

/*
 * Encode an X.500 Distinguished Name in ASN.1
 */
Bstream
Name::encode()
{
	Ava *avap = avalist;
	Bstream seqval;

	while (avap) {
		Bstream oid, ava, rdn, aval;
		oid = avap->attributetype.encode();
		aval = asn1_der_encode_printable_string(avap->attributevalue);
		ava = asn1_der_encode_sequence(oid+aval);
		rdn = asn1_der_encode_set_of(ava);
		seqval = seqval + rdn;
		avap = avap->next;
	}
	Bstream rdnseq = asn1_der_encode_sequence_of(seqval);
	return (rdnseq);
}
