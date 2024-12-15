/*
 * pgpErr.c
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpErr.c,v 1.3.2.1 1997/06/07 09:50:02 mhw Exp $
 */

#include <stdio.h>      /* For sprintf() */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* List of all error codes */
struct pgperr_t {
        int error;
        char const *string;
};

static struct pgperr_t const pgperrlist[] = {
#define PGPERR(err, string) { err, string },
#include "pgpErr.h"
        { 0, 0 }           /* End-of-list placeholder */
};

/* Get rid of warning from gcc about missing prototype */
char const PGPExport *  pgperrString(int code);

char const *
pgperrString(int code)
{
        unsigned i;
        static char buf[] = "Unknown error code -xxxxxxxxx";
        /*                   1234567890123456789 */

        for (i = 0; i < sizeof(pgperrlist)/sizeof(*pgperrlist); i++) {
                if (pgperrlist[i].error == code)
                        return pgperrlist[i].string;
        }
        sprintf(buf+20, "%+d", code);
        return buf;
}
