/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "tc.h"
#include "cms.h"
#include "cms_proto.h"

#define INT(a,b) (((a)-'0')*10+((b)-'0'))

static const unsigned char DaysPerMonth[12] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

#define SECSPERMIN 60
#define SECSPERHOUR (SECSPERMIN * 60)
#define SECSPERDAY (SECSPERHOUR * 24)
#define SECSPERYEAR (SECSPERDAY * 365)
/*#define SECSPERYEAR 31536000
#define SECSPERDAY 86400 */

#define isLeapYear(x) ((x)%4 == 0 && (((x)%100 != 0) || ((x)%400 == 0)))

static time_t tc_gtime (struct tm *t)
{
    int i;
    time_t g = 0;

    /* Epoch time on Unix machines starts at 00:00:00 UTC, January  1, 
       1970, so time_t will always start at that too?  Ie, for Win32
       machines does time_t start at 1970?  So, this assumes tm_year
       will always be >= 70 */
    for (i=70; i < t->tm_year; i++) {
	g += SECSPERYEAR;
	/* tm_year field is year since 1900 */
	if (isLeapYear (1900 + i))
	    g += SECSPERDAY;
    }

    for (i=0; i < t->tm_mon; i++)
	g += DaysPerMonth [i] * SECSPERDAY;

    g += (t->tm_mday - 1) * SECSPERDAY;
    g += t->tm_hour * SECSPERHOUR;
    g += t->tm_min * SECSPERMIN;
    g += t->tm_sec;

    return (g);
}

static time_t tc_decode_timezone (const unsigned char *p, size_t len)
{
  if (p[0] == 'Z')
    return 0;

  if (len < 5)
    return (-1); /* not long enough to be valid */

  return ((INT (p[1], p[2]) * 3600 + INT (p[3], p[4]) * 60) *
	  (p[0] == '-' ? -1 : 1));
}

/* Per ASN.1, UTCTime is defined as one of:
 *
 * YYMMDDhhmmZ
 * YYMMDDhhmm+hh'mm'
 * YYMMDDhhmm-hh'mm'
 * YYMMDDhhmmssZ
 * YYMMDDhhmmss+hh'mm'
 * YYMMDDhhmmss-hh'mm'
 *
 * Z=+0000
 * M => 1..12
 */
time_t tc_decode_UTCTime (PKIUTCTime *ptr)
{
    struct tm tm;
    size_t offset;
    time_t tz_offset = 0;

    if (ptr->len < 11)
	return 0; /* not long enough to be valid */

    (void)memset(&tm, 0, sizeof (tm));
    tm.tm_year = INT (ptr->val[0], ptr->val[1]);

    /* According to PKIX if before 50 assume the year 2000 */
    if (tm.tm_year < 50) {
	/* Add 100 because tm_year is number of years since 1900 */
	tm.tm_year += 100;
    }

    tm.tm_mon = INT (ptr->val[2], ptr->val[3]) - 1; /* tm_mon is 0..11 */
    tm.tm_mday = INT (ptr->val[4], ptr->val[5]);
    tm.tm_hour = INT (ptr->val[6], ptr->val[7]);
    tm.tm_min = INT (ptr->val[8], ptr->val[9]);

    if (isdigit (ptr->val[10])) {
	if (ptr->len < 13)
	    return 0; /* invalid date */

	tm.tm_sec = INT (ptr->val[10], ptr->val[11]);
	offset = 12;
    }
    else
	offset = 10;
    
    tz_offset = tc_decode_timezone (ptr->val + offset, ptr->len - offset);
    if (tz_offset < 0)
	return 0;
    
    return (tc_gtime (&tm) + tz_offset);
}

/* GeneralizedTime
 *
 * YYYYMMDDhhmmss.sss
 * YYYYMMDDhhmmss.sssZ
 * YYYYMMDDhhmmss.sss+hh'mm'
 * YYYYMMDDhhmmss.sss-hh'mm'
 */
time_t tc_decode_GeneralizedTime (PKIGeneralizedTime *p)
{
  struct tm tm;
  size_t offset;
  time_t tz_offset = 0;

  (void)memset(&tm, 0, sizeof (tm));

  if (p->len < 12)
    return 0;

  /* tm_year field is years since 1900, so assuming we are always dealing
     with stuff newer than 1900 we need to subtract 1900 */
  tm.tm_year = INT (p->val[0], p->val[1]) * 100 +
               INT (p->val[2], p->val[3]) - 1900;

  tm.tm_mon = INT (p->val[4], p->val[5]) - 1; /* tm_mon is 0..11 */
  tm.tm_mday = INT (p->val[6], p->val[7]);
  tm.tm_hour = INT (p->val[8], p->val[9]);
  tm.tm_min = INT (p->val[10], p->val[11]);

  offset = 12;

  /* seconds (optional) */
  if (offset < p->len && isdigit (p->val[12]))
  {
    tm.tm_sec = INT (p->val[12], p->val[13]);
    offset += 2;

    /* ignore the decimal part of the seconds (if present) */
    if (offset < p->len && p->val[offset] == '.')
    {
      offset++; /* skip the `.' */
      while (offset < p->len && isdigit (p->val[offset]))
	offset++;
    }
  }

  tz_offset = tc_decode_timezone (p->val + offset, p->len - offset);
  if (tz_offset < 0)
      return 0;

  return (tc_gtime (&tm) + tz_offset);
}

/*****
*
* return
*    > 0 - the time int time_t
*    TC_E_INVARGS
*
*****/
time_t tc_get_choiceoftime (TC_Time *p, TC_CONTEXT *ctx)
{

  (void)ctx; /* for future use */

  if (p == NULL)
      return TC_E_INVARGS;

  switch (p->CHOICE_field_type)
  {
    case PKIID_UTCTime:
      return (tc_decode_UTCTime ((PKIUTCTime *) p->data));
    case PKIID_GeneralizedTime:
      return (tc_decode_GeneralizedTime ((PKIGeneralizedTime *) p->data));
  }
  return TC_E_INVARGS;
}

/*****
 * tc_encode_utctime
 *
 * Convert Unix time_t data into a string representation for UTCTime.
 * For PKIX the value is expressed in GMT and includes seconds even if
 * zero.  Eg. YYMMDDHHmmssZ
 *
 * The year field is interpreted as:
 *      YY >= 50, then year is 19YY
 *      yy < 50, then year is 20YY
 *
 * return
 *   0 - okay
 *   TC_E_INVARGS
 *   TC_E_NOMEMORY
 *   TC_E_INVDATE
 *
 *****/
int tc_encode_utctime (TC_UTCTime **utcTime, time_t t, TC_CONTEXT *ctx)
{
    struct tm *tm;

    if (utcTime == NULL)
	return TC_E_INVARGS;

    /* NOTE: this is a check to see if we are coming up on the limit
       of the value of time_t (big assumption is that this code will be
       in use in 2037!).  If so, then we want to return with an
       error. Hopefully this will force a look at this code and then it will
       be analyzed/fixed to use an appropriate value/type for the time.
       When this code is updated for a new time value, then, based on
       PKIX part 1, the following must be done:
           use UTCTime through 2049
	   use GeneralizedTime starting with 2050

       The updates will need to be done in the respective create calls
       for certs, CRLs, and requests.  Currently they are using UTCTime.

       The time this check will fail is:
           Mon Jul 20 08:14:07 2037
       which is 6 months before the maximum time allowed by the current
       (in 1998) time_t type.

       Of course, if the value range for time_t is expanded before
       2037, then the code can be updated to use UTCTime/GeneralizedTime
       as required and this check can be removed.
    */
    /* 2147483647 = max number of seconds in time_t
       31536000/2 = number of seconds per year div 2 = 6 months
       */
#if 0
/* Disable this test, it may be causing problems on the Mac */
    if ( t > (2147483647L - (31536000/2)) ) {
	return TC_E_INVDATE;
    }
#endif

    *utcTime = PKINewUTCTime(ctx->certasnctx);
    if (*utcTime == NULL)
	return TC_E_NOMEMORY;

    (*utcTime)->len = 13;
    (*utcTime)->val = TC_Alloc(ctx->memMgr, (*utcTime)->len + 1);
    if ( (*utcTime)->val == NULL)
	return TC_E_NOMEMORY;

    tm = gmtime (&t);
    (void)strftime( (char *)(*utcTime)->val,
		  (*utcTime)->len + 1,
		  "%y%m%d%H%M%SZ", tm);

    return 0;
}


/*****
 * tc_EncodeGeneralizedTime
 *
 * Convert Unix time_t data into a string representation for
 * PKIX GeneralizedTime.  For PKIX, the format is expressed in GMT
 * and includes seconds even if 0.  Fractional seconds are not included.
 * (eg., YYYYMMDDHHmmssZ)
 *
 * return
 *   0 - okay
 *   TC_E_INVARGS
 *   TC_E_NOMEMORY
 *
 *****/
int tc_EncodeGeneralizedTime(
    TC_GeneralizedTime **gtTime,
    time_t t,
    TC_CONTEXT *ctx)
{
    struct tm *tm;
    TC_GeneralizedTime *local;

    if (gtTime == NULL)
	return TC_E_INVARGS;
    *gtTime = NULL;

    local = PKINewGeneralizedTime(ctx->certasnctx);
    if (local == NULL)
	return TC_E_NOMEMORY;

    local->len = 15;
    local->val = TC_Alloc(ctx->memMgr, local->len + 1);
    if (local->val == NULL)
	return TC_E_NOMEMORY;

    tm = gmtime(&t);
    (void)strftime( (char *)local->val,
		    local->len + 1,
		    "%Y%m%d%H%M%SZ", tm); /* note "Y" for year w/century */
    
    *gtTime = local;

    return 0;
} /* tc_EncodeGeneralizedTime */
