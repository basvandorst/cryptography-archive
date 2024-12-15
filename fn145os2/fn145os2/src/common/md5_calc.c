/*
 # $Id: md5_calc.c,v 1.6 1998/04/10 10:27:18 fbm Exp fbm $
 # Copyright (C) 1997-1999 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include "includes.h"

#include "md5_calc.h"
#include "md5.h"
#include "spans.h"
#include "log.h"

#define BUFSIZE 1024*16

int
md5_calc_piece(int start, int stop, void *p1, void *p2, void *p3)
{
	int			i, posn, nb;
	int			fd = (int) p1;
        static unsigned char    buf[BUFSIZE];
	MD5_CTX			*c = (MD5_CTX *) p2;

	Log(4, "t4>> md5_calc span %#x-%#x\n", start, stop);

	posn = (long) lseek(fd, (off_t) start, SEEK_SET);

	while (posn < stop || stop < 0) {
		nb = sizeof(buf);
		if (stop > 0 && stop - posn < nb)
			nb = stop - posn;
		i = read(fd, (char *) buf, nb);
		if (i <= 0)
			break;
		MD5_Update(c, buf, (unsigned long)i);
		posn += i;
	}
	return 0;
}

char *
md5_calc(int fd, char *span)
{
        int                     i;
        MD5_CTX                 c;
        unsigned char           md[MD5_DIGEST_LENGTH];
        static char		*p, buf[BUFSIZE];

	lseek(fd, 0L, SEEK_SET);

	MD5_Init(&c);

	do_spans(span, md5_calc_piece, (void *) fd, (void *) &c, NULL);

	MD5_Final(&(md[0]),&c);

        for (i = 0, p = buf; i < MD5_DIGEST_LENGTH; i++, p += 2)
                sprintf(p, "%02x", md[i]);
        return buf;
}
