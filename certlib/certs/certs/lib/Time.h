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

#pragma ident "@(#)Time.h	1.8 96/01/29 Sun Microsystems"

#ifndef TIME_H
#define TIME_H

/*
 * Time class
 */
#include "my_types.h"

struct GMtime {
	int	year;
	int	month;
	int	day;
	int	hour;
	int	min;
	int	sec;
};

class PCTime {
	// Time expressed as GMT time
	int	year;
	int	month;
	int	day;
	int	hour;
	int	min;
	int	sec;
  public:
	// Constructors
	PCTime() {year=month=day=hour=min=sec=0;};
	PCTime(int, int, int, int, int, int);
	PCTime(const GMtime&);
	// Member functions and operators
	GMtime get() const;
	friend Boolean operator <(const PCTime &, const PCTime &);
	friend Boolean operator >(const PCTime &, const PCTime &);
	friend Boolean operator !=(const PCTime &, const PCTime &);
	friend Boolean operator ==(const PCTime &, const PCTime &);
	friend Boolean operator <=(const PCTime &, const PCTime &);
	friend Boolean operator >=(const PCTime &, const PCTime &);
	void print() const;		/* Print out time val */
	String getstr() const;
	unsigned long ntptime();
};

extern PCTime timenow();

// Unix start
#define UNIXSTART	2208988800	// 1-1-70 0h in NTP time

// time now in NTP
#define NTPNOW		(time(0) + UNIXSTART)

#endif TIME_H
