/*
 # $Id: spans.c,v 1.5 1998/03/10 14:11:25 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
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

#include <stdlib.h>
#include "spans.h"
#include "log.h"

int
do_spans(char *span, int (*handler)(), void *p1, void *p2, void *p3)
{
	int             rtn = 0;
	long			start = 0;
	long			stop = -1;

	if (span == NULL || *span == '\0')
		span = "0-";
	while (span && *span && rtn == 0) {
		start = strtol(span, &span, 0);

		if (*span == '\0')
			stop = start;
		else if (*span == ',')
			stop = start;
		else if (*span == '-' || *span == ':') {
			stop = (*span == ':')? start: 0;
			span++;
			if (*span == '\0' || *span == ',')
				stop = -1;
			else {
				stop += strtol(span, &span, 0);
			}
		}
		if (*span == ',')
			span++;

		if (start < 0 || (stop < start && stop >= 0)) {
			Log(0, "t0>> Warning: bad range ignored: %ld to %ld\n",
				start, stop
			);
		}
		else {
			rtn = (*handler)(start, stop, p1, p2, p3);
		}
	}
        return rtn;
}
