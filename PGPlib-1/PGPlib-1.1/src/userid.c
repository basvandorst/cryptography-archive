/*********************************************************************
 * Filename:      userid.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:42:28 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


#include <pgplib.h>

#include <stdlib.h>
#include <string.h>

void
free_uid(PGPuserid_t	*uid)
{
    if ( uid == NULL)
	return;
    /* free(NULL) is OK */
    free(uid->name);
    memset(uid, 0, sizeof(*uid));
}

/*
 * Return 0 on sucess -1 on failure.
 */

int
uid2buf(PGPuserid_t	*uid,
	u_char		**buf)
{
    int		len;
    u_char	*p;

    if ( uid == NULL || buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return (-1);
    }

    len = strlen(uid->name);
    if ( len > 255 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    /* CTB and length-byte */
    p = malloc(len+2);
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    p[0] = CTB_USERID;
    p[1] = (u_char) len;
    memcpy(p+2, uid->name, len);

    *buf = p;
    return(0);
}

/*
 * Convert a buffer into a C-string (within a struct).  We can do
 * some verification.  If there is an error, we return -1 if not, we
 * return 0.
 */
int
buf2uid(u_char		*buf,
	int 		length,
	PGPuserid_t 	*uid)
{
    char	*p;
    int		len;

    if ( buf == NULL || length < 3 || uid == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    len = packet_length(buf);
    if ( len == -1 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    if ( len > length ) {
	pgplib_errno = PGPLIB_ERROR_BAD_PACKET_LENGTH;
	return(-1);
    }
    p = malloc(len-1);
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    memcpy(p, buf+2, len - 2);
    p[len-2] = 0;

    uid->name = p;

    return(0);
}


