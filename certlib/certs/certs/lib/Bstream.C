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

#pragma ident "@(#)Bstream.C	1.5 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "Bstream.h"

Bstream::Bstream()
{
 	len = 0; peekindex = 0;
	deletethis = datap = NULL;
}

Bstream::Bstream(int al, byte *adp)
{
	len = al;
	deletethis = datap = new byte[len];
	(void) bcopy(adp, datap, al);
	peekindex = 0;
	return;
}

Bstream::Bstream(const char *str)
{
	len = strlen(str);
	deletethis = datap = new byte[len];
	(void) bcopy((void *)str, datap, len);
	peekindex = 0;
	return;
}

Bstream::Bstream(const Bstream& a)
{
	deletethis = datap = new byte[a.len];
	len = a.len;
	peekindex = 0;
	(void) bcopy(a.datap, datap, len);
	return;
}

Bstream::Bstream(const String& a)
{
	deletethis = datap = new byte[a.length()];
	len = a.length();
	peekindex = 0;
	(void) bcopy((byte *)((const char *)a), datap, len);
	return;
}

Bstream&
Bstream:: operator =(const Bstream& a)
{
	if (deletethis != a.deletethis)	{ // Check for a = a;
		if (deletethis != NULL)
			delete(deletethis);
		deletethis = datap = new byte[a.len];
		len = a.len;
		peekindex = 0;
		(void) bcopy(a.datap, datap, len);
	}
	return (*this);
}

Boolean operator ==(const Bstream& a, const Bstream& b)
{
	Boolean retval;
	int  ret;

	if (a.len != b.len)
		return (BOOL_FALSE);
	ret = bcmp(a.datap, b.datap, a.len);
	if (ret == 0) retval = BOOL_TRUE;
	else retval = BOOL_FALSE;
	return (retval);
}

Boolean operator !=(const Bstream& a, const Bstream& b)
{
	if (a == b)
		return (BOOL_FALSE);
	else
		return (BOOL_TRUE);
}

Bstream operator +(const Bstream& a, const Bstream& b)
{
	Bstream sumstr;
	int sumlen = a.len + b.len;
	byte *sumdatap = new byte[sumlen];
	(void) bcopy(a.datap, sumdatap, a.len);
	(void) bcopy(b.datap, sumdatap+a.len, b.len);
	sumstr = Bstream(sumlen, sumdatap);
	delete sumdatap;
	return (sumstr);
}

Bstream&
Bstream::operator +=(const Bstream& b)
{
	int sumlen = len + b.len;
	byte *sumdatap = new byte[sumlen];
	(void) bcopy(datap, sumdatap, len);
	(void) bcopy(b.datap, sumdatap + len, b.len);
	if (deletethis != NULL) {
		delete (deletethis);
	} else
		printf("deletethis was NULL\n");
	deletethis = datap = sumdatap;
	len = sumlen;
	peekindex = 0;
	return (*this);
}


Bstream::~Bstream()
{
	if (deletethis != NULL)
		delete(deletethis);
}

Bstream::operator const char*() const
{
	return ((char *)datap);
}

#ifdef ndef
Bstream::operator String() const
{
	return (String((char *)datap, len));
}
#endif ndef

String
Bstream::getstr() const
{
	return (String((char *)datap, len));
}

int Bstream::consume(int l)
{
	if (l > len)
		return (FAILURE);
	len -= l;
	datap += l;
	peekindex = 0;
	return (SUCCESS);
}

int Bstream::truncate(int l)
{
	if (l > len)
		return (FAILURE);
	len -= l;
	peekindex = 0;
	return (SUCCESS);
}

int Bstream::getlength() const
{
	return (len);
}

int Bstream::fetchbyte(byte& val)
{
	if (len == 0)
		return (FAILURE);
	val = *datap++;
	len -= 1;
	peekindex = 0;
	return (SUCCESS);
}

int Bstream::peekbyte(byte& val)
{
	if (len == 0 || peekindex > len)
		return (FAILURE);
	val = datap[peekindex++];
	return (SUCCESS);
}

byte *
Bstream::getdatap() const
{
	return (datap);
}

Bstream
Bstream::replace(const Bstream& from, const Bstream& to) const
{
	int i, fromlen, tolen;
	Bstream scan = *this;		// XXX	

	fromlen = from.getlength();
	tolen =  to.getlength();

	for (i = 0; i < scan.len; i++) {
		if (bcmp(&(scan.datap[i]), from.datap, fromlen) == 0) {
			Bstream before(i, scan.datap);
			scan.consume(i + fromlen);
			return (before + to + scan.replace(from, to));
		}
	}
	return (scan);
}

byte
Bstream::last() const
{
	if (len > 0)
		return (datap[len - 1]);
	else
		return (0);	// XXX
}

int
Bstream::store(const char *filename) const
{
	int fd = ::open(filename, O_RDWR | O_TRUNC | O_CREAT, 0644);
	if (fd < 0)
		return (fd);
	int ret = ::write(fd, (char *)datap, len);
	::close(fd);
	return (ret);
}

int
Bstream::write(FILE *f) const
{
	int ret = fwrite((char *)datap, 1, len, f);
	return (ret);
}

void 
Bstream::print() const
{
	const int LINEWIDTH = 20;

	for (int i=0; i<len; ) {
		for(int j=0; j<LINEWIDTH && ((i+j) < len); j++)
			printf("%.2x ", datap[i+j]);
		printf("\n");
		i += LINEWIDTH;
	}
}

void 
Bstream::prints() const
{
	for (int i=0; i<len; i++) {
		printf("%c", (char )datap[i]);
	}
}

void 
Bstream::printhexint() const
{
	for (int i=0; i<len; i++) {
		printf("%.2x", datap[i]);
	}
}

void 
Bstream::printdecint() const
{
	for (int i=0; i<len; i++) {
		// XXX We need to make this a decimal mp int print
		printf("%.2x", datap[i]);
	}
}

String
Bstream::gethexstr() const
{
	char *buf = new char[len*2+1];
	bzero(buf, sizeof(buf));
	for (int i=0; i<len; i++) {
		sprintf(buf+i*2, "%.2x", datap[i]);
	}
	String hexstr = buf;
	delete buf;
	return (hexstr);
}

String
Bstream::getdecstr() const
{
	// XXX This should be converted to
	// output as a decimal integer. Right
	// now it just outputs as a hex mp int.
	char *buf = new char[len*2];
	bzero(buf, sizeof(buf));
	for (int i=0; i<len; i++) {
		sprintf(buf+i*2, "%.2x", datap[i]);
	}
	String hexstr = buf;
	delete buf;
	return (hexstr);
}

Bstream
File_to_Bstr(const char *filename)
{
	Bstream retval;
	FILE	*fc;

	fc = fopen(filename, "r");
	if (fc == NULL) {
#ifdef ndef
		printf("unable to open %s\n", filename);
		perror("File_to_Bstr:");
#endif ndef
		return (retval);
	}
	fseek(fc, 0, 2);
	int len = (int)ftell(fc);
	rewind(fc);
	byte *buf = new byte[len];
	int ret = fread((char *)buf, 1, len, fc);
	if (ret != len) {
		printf("File_to_Bstr: ret != len");
		return (retval);
	}
	retval = Bstream(len, buf);
	delete buf;
	fclose(fc);
	return retval;
}

String
File_to_String(const char *filename)
{
	String null;
	FILE	*fc;

	fc = fopen(filename, "r");
	if (fc == NULL) {
		fprintf(stderr, "unable to open %s\n", filename);
		perror("File_to_String:");
		return (null);
	}
	fseek(fc, 0, 2);
	int len = (int)ftell(fc);
	rewind(fc);
	char *buf = new char[len];
	int ret = fread((char *)buf, 1, len, fc);
	if (ret != len) {
		fprintf(stderr, "File_to_String: ret != len");
		return (null);
	}
	String str(buf, len);
	delete buf;
	fclose(fc);
	return str;
}
