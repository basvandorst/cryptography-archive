/*
 # $Id: cp.c,v 1.16 1998/04/10 10:27:56 fbm Exp fbm $
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

#include "cp.h"
#include "misc.h"
#include "morpher.h"
#include "callback.h"
#include "msgs.h"

int
cp(const char *from, const char *to, int flags, void (*meterfn)(void *, int, int), void *vp, int range)
{
	int		ifd = 0;
	int		ofd = 0;
	int		nb, nr, nw;
	char		*p, buf[16*1024];
	int		filesz;
	struct stat	st;
	int		rtn = -1;

	if ((ifd = open(from, OPENFL(O_RDONLY|flags), 0666)) == -1) {
		report(R_ERR, Msgs_get("MFOPENERR"), from, syserr());
		goto done;
	}
	if (fstat(ifd, &st) == -1) {
		report(R_ERR, Msgs_get("MFSTATERR"), from, syserr());
		goto done;
	}
	filesz = st.st_size;

	if ((ofd = open(to, OPENFL(O_WRONLY|O_CREAT|O_TRUNC|flags), 0666)) == -1) {
		report(R_ERR, Msgs_get("MFOPENERR"), to, syserr());
		goto done;
	}

	for (nb = 0; (nr = read(ifd, buf, sizeof(buf))) > 0;) {
		for (p = buf; nr > 0;) {
			nw = write(ofd, p, nr);
			if (nw <= 0) {
				report(R_ERR, Msgs_get("MWRITEERR"), to, syserr());
				unlink(to);
				goto done;
			}
			p += nw;
			nr -= nw;
			nb += nw;
			if (meterfn && ((nb * range) / filesz > ((nb - nw) * range) / filesz)) {
				(*meterfn)(vp, range, (nb * range) / filesz);
			}
		}
	}
	if (nr == 0) {
		rtn = 0;
	}
	else {
		report(R_ERR, Msgs_get("MREADERR"), from, syserr());
		unlink(to);
	}
done:
	close(ifd);
	close(ofd);
	return rtn;
}
