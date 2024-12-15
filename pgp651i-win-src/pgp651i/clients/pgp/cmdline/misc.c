/*____________________________________________________________________________
    misc.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    miscellaneous user interface code from PGP2.6.2

    $Id: misc.c,v 1.9 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include "pgpBase.h"
#include "usuals.h"
#include "prototypes.h"

/*
   Given timestamp as seconds elapsed since 1970 Jan 1 00:00:00,
   returns year (1970-2106), month (1-12), day (1-31).
   Not valid for dates after 2100 Feb 28 (no leap day that year).
   Also returns day of week (0-6) as functional return.
 */
static int date_ymd(PGPUInt32 *tstamp, int *year, int *month, int *day)
{
    PGPUInt32 days,y;
    int m,d,i;
    static short mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    days = (*tstamp)/(unsigned long)86400L; /* day 0 is 1970/1/1 */
    days -= 730L;   /* align days relative to 1st leap year, 1972 */
    y = ((days*4)/(unsigned long)1461L);    /* 1972 is year 0 */
    /* reduce to days elapsed since 1/1 last leap year: */
    d = (int) (days - ((y/4)*1461L));
    *year = (int)(y+1972);
    for (i=0; i<48; i++) {  /* count months 0-47 */
        m = i % 12;
        d -= mdays[m] + (i==1); /* i==1 is the only leap month */
        if (d < 0) {
            d += mdays[m] + (i==1);
            break;
        }
    }
    *month = m+1;
    *day = d+1;
    i = (int)((days-2) % (unsigned long)7L); /* compute day of week 0-6 */
    return i;       /* returns weekday 0-6; 0=Sunday, 6=Saturday */
}       /* date_ymd */

/*      Return date string, given pointer to 32-bit timestamp */
static char *cdate(PGPUInt32 *tstamp)
{
    int month,day,year;
    static char datebuf[20];
    if (*tstamp == 0)
        return "          ";
    (void) date_ymd(tstamp,&year,&month,&day);
    sprintf(datebuf,"%4d/%02d/%02d", year, month, day);
    return (datebuf);
}       /* cdate */

/* Return date and time string, given pointer to 32-bit timestamp */
char *ctdate(PGPUInt32 *tstamp)
{
    int hours,minutes;
    static char tdatebuf[40];
    long seconds;
    seconds = (*tstamp) % (unsigned long)86400L;
    /* seconds past midnight today */
    minutes = (int)((seconds+30L) / 60L);
    /* round off to minutes past midnight */
    hours = minutes / 60; /* hours past midnight */
    minutes = minutes % 60; /* minutes past the hour */
    sprintf(tdatebuf,"%s %02d:%02d GMT", cdate(tstamp), hours, minutes);
    return (tdatebuf);
} /* ctdate */

#if !defined(MSDOS) && !defined(ATARI)
#include <ctype.h>
/*#include "charset.h"*/
char *strlwr(char *s)
{ /*
     Turns string s into lower case.
   */
    int c;
    char *p = s;
    while ((c = *p))
        *p++ = toLower(c);
    return(s);
}

char *strupr(char *s)
{ /*
     Turns string s into upper case.
   */
    int c;
    char *p = s;
    while ((c = *p))
        *p++ = toUpper(c);
    return(s);
}
#endif /* !MSDOS && !ATARI */

int toUpper(int c)
{
    return (c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c);
}

int toLower(int c)
{
    return (c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c);
}