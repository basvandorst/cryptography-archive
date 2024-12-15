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

#pragma ident "@(#)ObjId.C	1.4 96/01/29 Sun Microsystems"

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
 * ObjId class implementation.
 */

#ifndef __GNUC__
ObjId::ObjId(int num_elems, ...)

{
	Bigint		elem_int;
	va_list		ap;

	obj_id_list = NULL;
	va_start(ap, num_elems);
	while (num_elems--) {
		elem_int = va_arg(ap, Bigint);
		add_element(elem_int);
	}
	va_end(ap);
}
#else
ObjId::ObjId(int num_elems,
                Bigint b1,Bigint b2, Bigint b3, Bigint b4, Bigint b5, Bigint b6,
                Bigint b7,Bigint b8, Bigint b9, Bigint b10, Bigint b11,
                Bigint b12)
{
        obj_id_list = NULL;
        if (num_elems > 0 )
                add_element(b1);
        if (num_elems > 1)
                add_element(b2);
        if (num_elems > 2)
                add_element(b3);
        if (num_elems > 3)
                add_element(b4);
        if (num_elems > 4)
                add_element(b5);
        if (num_elems > 5)
                add_element(b6);
        if (num_elems > 6)
                add_element(b7);
        if (num_elems > 7)
                add_element(b8);
        if (num_elems > 8)
                add_element(b9);
        if (num_elems > 9)
                add_element(b10);
        if (num_elems > 10)
                add_element(b11);
}
#endif

ObjId::~ObjId()
{
	obj_id_elem	*oidp = obj_id_list;
	while (oidp != NULL) {
		obj_id_elem	*tmpoidp = oidp->next;
		delete oidp;
		oidp = tmpoidp;
	}
	obj_id_list = NULL;
}

ObjId::ObjId(const ObjId &a)
{
	obj_id_elem	*oidp = a.obj_id_list;
	obj_id_elem	*tmpoidp, *lastoidp;

	lastoidp = NULL;
	while (oidp != NULL) {
		tmpoidp = new obj_id_elem;
		tmpoidp->elemval = oidp->elemval;
		tmpoidp->next = NULL;
		if (lastoidp == NULL) {
			obj_id_list = tmpoidp;
		} else {
			lastoidp->next = tmpoidp;
		}
		lastoidp = tmpoidp;
		oidp = oidp->next;
	}
	return;
}

ObjId&
ObjId:: operator =(const ObjId &a)
{
	if (obj_id_list == a.obj_id_list)	// Check for a = a;
		return (*this);

	// Release any old ObjId info that may be present
	// XXX memory leak?
#ifndef __GNUC__
	ObjId::~ObjId();
#endif
	obj_id_elem	*oidp = a.obj_id_list;
	obj_id_elem	*tmpoidp, *lastoidp;

	lastoidp = NULL;
	while (oidp != NULL) {
		tmpoidp = new obj_id_elem;
		tmpoidp->elemval = oidp->elemval;
		tmpoidp->next = NULL;
		if (lastoidp == NULL) {
			obj_id_list = tmpoidp;
		} else {
			lastoidp->next = tmpoidp;
		}
		lastoidp = tmpoidp;
		oidp = oidp->next;
	}
	return (*this);

}

Boolean operator ==(const ObjId& a, const ObjId& b)
{
	obj_id_elem	*aoidp = a.obj_id_list;
	obj_id_elem	*boidp = b.obj_id_list;

	while (aoidp) {
		if (boidp == NULL)
			return (BOOL_FALSE);
		if (aoidp->elemval != boidp->elemval)
			return (BOOL_FALSE);
		boidp = boidp->next;
		aoidp = aoidp->next;
	}
	if (boidp != NULL)
		return (BOOL_FALSE);
	return (BOOL_TRUE);
}

Boolean operator !=(const ObjId& a, const ObjId& b)
{
	if ( a == b ) 
		return BOOL_FALSE;
	else 
		return (BOOL_TRUE);
}


void
ObjId::add_element(const Bigint &newval)
{
	obj_id_elem	*oidp = obj_id_list;
	obj_id_elem	*lastoidp;
	obj_id_elem	*newoidp = new obj_id_elem;

	newoidp->next = NULL;
	newoidp->elemval = newval;
	if (oidp == NULL) {
		obj_id_list = newoidp;
		return;
	}
	while (oidp != NULL) {
		lastoidp = oidp;
		oidp = oidp->next;
	}
	lastoidp->next = newoidp;
	return;
}



// Make integer sub ID into BER encoding of subid.
// Tie together subid bytes using Bit 8 as end-of-subid

Bstream
subid_to_bstr(const Bigint& elem)
{
	Bstream subid = Bigint_to_Bstr(elem);
	if (elem < 128) {
		return (subid);
	}
	int intlen = subid.getlength();
	byte *intstr = subid.getdatap();
	int len = intlen + 1 + (intlen/8);
	byte *subidber = new byte[len];
	byte *deletethis = subidber;
	byte carry = 0;
	byte hibit = 0;
	for (int i = len - 1, j = intlen - 1, k=0; i >= 0; i--, j--, k++) {
		byte part;

		if (j < 0) {
			part = carry | hibit;
		} else {
			part = ((intstr[j] & ~num_to_mask(k)) << (k%7));
		}
		subidber[i] = part | carry | hibit;
		if (subidber[i] == 0x80 && i == 0) {
			len--;
			subidber++;
			break;
		}
		carry = ((byte)(intstr[j] & num_to_mask(k)) >> (7-(k%7)));
		if (hibit == 0) 
			hibit = 0x80;
	}
	Bstream subidberstr = Bstream(len, subidber);
	delete deletethis;
	return (subidberstr);
}

// return ASN.1 BER encoding of an OBJECT-IDENTIFIER

Bstream
ObjId::encode() const
{
	byte ID = OBJECT_IDENTIFIER_ID;
	Bstream IDstr(1, &ID);
	Bigint X, Y;
	obj_id_elem	*oidp = obj_id_list;

	X = oidp->elemval;
	oidp = oidp->next;
	Y = oidp->elemval;
	oidp = oidp->next;
	Bigint subid = X*40 + Y;
	Bstream val = subid_to_bstr(subid);
	while (oidp) {
		val = val + subid_to_bstr(oidp->elemval);
		oidp = oidp->next;
	}
	Bstream lenstr = asn1_der_set_length(val.getlength());
	return (IDstr + lenstr + val);
}

void
ObjId::print() const
{
	obj_id_elem	*oidp = obj_id_list;

	printf("{ ");
	while (oidp) {
		oidp->elemval.printd();
		oidp = oidp->next;
		printf(" ");
	}
	printf("}");
}

String
ObjId::getoidstr() const
{
	String oidstr;
	obj_id_elem	*oidp = obj_id_list;

	oidstr += "{ ";
	while (oidp) {
		oidstr += oidp->elemval.getnumstrd();
		oidp = oidp->next;
		oidstr += " ";
	}
	oidstr += "}";
	return (oidstr);
}
