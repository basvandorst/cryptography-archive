/*********************************************************************
 * Filename:      error.c
 * Description:
 * Author:      Thomas Roessler <roessler@guug.de>
 * Modified at:   Fri Dec  5 14:52:15 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) Thomas Roessler
 * 	See the file COPYING for details
 *
 ********************************************************************/


#define _ERROR_C

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "pgplib.h"

const char *pgplib_errlist[] = {
    "No error.",
    "Bad ctb length.",
    "Bad ctb.",
    "Invalid argument.",
    "Compressed packet.",
    "End of file.",
    "Impossible error condition.",
    "Bad packet length.",
    "Bad CRC.",
    "Bad algorithm.",
    "Zip error.",
    "Wrong pass phrase.",
    "Fatal db error.",
    "database not open.",
    NULL
};

int pgplib_errno;

const char *pgplib_strerror(int val)
{
    if (val == PGPLIB_ERROR_SYSERR)
	return strerror(errno);
    if (val < 0 || val > PGPLIB_ERROR_MAX)
	return NULL;

    return pgplib_errlist[val];
}
