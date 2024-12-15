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
#pragma ident "@(#)Time.C	1.7 96/01/29 Sun Microsystems"

#include <sys/types.h>
#include <stdio.h>
#include "Time.h"
#include <time.h>
#include <sys/time.h>

PCTime::PCTime(int y, int mo, int d, int h, int mi, int s)
{
	year = y;
	month = mo;
	day = d;
	hour = h;
	min = mi;
	sec = s;
	return;
}

PCTime::PCTime(const GMtime& a)
{
	year = a.year;
	month = a.month;
	day = a.day;
	hour = a.hour;
	min = a.min;
	sec = a.sec;
	return;
}

GMtime
PCTime::get() const
{
	GMtime time;

	time.year = year;
	time.month = month;
	time.day = day;
	time.hour = hour;
	time.min = min;
	time.sec = sec;
	return (time);
}

Boolean operator <(const PCTime& a, const PCTime& b)
{
	if (a.year < b.year)
		return (BOOL_TRUE);
	else if (a.year > b.year)
		return (BOOL_FALSE);
	if (a.month < b.month)
		return (BOOL_TRUE);
	else if (a.month > b.month)
		return (BOOL_FALSE);
	if (a.day < b.day)
		return (BOOL_TRUE);
	else if (a.day > b.day)
		return (BOOL_FALSE);
	if (a.hour < b.hour)
		return (BOOL_TRUE);
	else if (a.hour > b.hour)
		return (BOOL_FALSE);
	if (a.min < b.min)
		return (BOOL_TRUE);
	else if (a.min > b.min)
		return (BOOL_FALSE);
	if (a.sec < b.sec)
		return (BOOL_TRUE);
	else if (a.sec > b.sec)
		return (BOOL_FALSE);
	return (BOOL_FALSE);
}

Boolean operator >(const PCTime& a, const PCTime& b)
{
	if (a.year > b.year)
		return (BOOL_TRUE);
	else if (a.year < b.year)
		return (BOOL_FALSE);
	if (a.month > b.month)
		return (BOOL_TRUE);
	else if (a.month < b.month)
		return (BOOL_FALSE);
	if (a.day > b.day)
		return (BOOL_TRUE);
	else if (a.day < b.day)
		return (BOOL_FALSE);
	if (a.hour > b.hour)
		return (BOOL_TRUE);
	else if (a.hour < b.hour)
		return (BOOL_TRUE);
	if (a.min > b.min)
		return (BOOL_TRUE);
	else if (a.min < b.min)
		return (BOOL_FALSE);
	if (a.sec > b.sec)
		return (BOOL_TRUE);
	else if (a.sec < b.sec)
		return (BOOL_FALSE);
	return (BOOL_FALSE);
}

Boolean operator ==(const PCTime& a, const PCTime& b)
{
	if (	(a.year == b.year) &&
		(a.month == b.month) &&
		(a.day == b.day) &&
		(a.hour == b.hour) &&
		(a.min == b.min) &&
		(a.sec == b.sec))
		return (BOOL_TRUE);
	else
		return (BOOL_FALSE);
}

Boolean operator !=(const PCTime& a, const PCTime& b)
{
	if (a == b)
		return (BOOL_FALSE);
	else
		return (BOOL_TRUE);
}

Boolean operator <=(const PCTime& a, const PCTime& b)
{
	if ((a < b) || (a == b) )
		return (BOOL_TRUE);
	else
		return (BOOL_FALSE);
}

Boolean operator >=(const PCTime& a, const PCTime& b)
{
	if ((a > b) || (a == b) )
		return (BOOL_TRUE);
	else
		return (BOOL_FALSE);
}

void
PCTime::print() const
{
	printf("%.2d:%.2d:%.2d,", hour, min, sec);
	printf(" %.2d/%.2d/%.4d GMT", month, day, year);
}

String
PCTime::getstr() const
{
	String timestr;
	char buf[80];
	bzero(buf, sizeof(buf));
	sprintf(buf, "%.2d:%.2d:%.2d,", hour, min, sec);
	sprintf(buf+strlen(buf)," %.2d/%.2d/%.4d GMT", month, day, year);
	timestr = buf;
	return (timestr);
}

PCTime
timenow()
{
	struct timeval time;
	struct tm *now;

	(void) gettimeofday(&time, NULL);
	now = gmtime((const long *)&time.tv_sec);
	if (now->tm_year > 90) {
		now->tm_year +=1900;
	} else {
		now->tm_year += 2000;	// Fix this after year 2090
	}
	// Months are from 0-11, so we need to adjust
	// by incrementing
	now->tm_mon++;
	return PCTime(now->tm_year, now->tm_mon, now->tm_mday, now->tm_hour,
		    now->tm_min, now->tm_sec);
}

u_long PCTime::ntptime()
{
	struct tm tm;
	u_long result;
	tm.tm_sec=sec;
	tm.tm_min=min;
	tm.tm_hour=hour;
	tm.tm_mon=month-1;
	tm.tm_mday=day;
	tm.tm_year=year-1900;
	tm.tm_isdst=1;
	result=mktime(&tm);
	result+=UNIXSTART;
	return result;
}	
