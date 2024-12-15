/*
 # $Id: pidata.c,v 1.2 1998/04/10 10:29:58 fbm Exp fbm $
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

#include <assert.h>
#include "includes.h"

#include "pidata.h"
#include "log.h"

#define opcode(b)	(((b) >> 5) & 0x7)
#define	N_OPS		5
#define OP_ZERO		0
#define OP_BCOPY	1
#define OP_RPT		2
#define OP_RPTBCOPY	3
#define OP_RPTZERO	4

#define FETCH_ARG(c, s, slen)								\
			for (i = 0; i < 4 && ((*(s)) & 0x80); i++) {			\
				assert((slen) > 0);					\
				(c) <<= 7;						\
				(c) |= ((*(s)) & 0x7f);					\
				(s)++;							\
				(slen)--;						\
			}								\
			if (i == 4 && ((*(s)) & 0x80)) {				\
				Log(1, "t1>> Numeric arg too large.\n");		\
				exit(1);						\
			}								\
			assert((slen) > 0);						\
			(c) <<= 7;							\
			(c) |= ((*(s)) & 0x7f);						\
			(s)++;								\
			(slen)--;

#define MEMCPY(t, tlen, s, nbytes, slen)						\
			assert((nbytes) <= (slen));					\
			assert((nbytes) <= (tlen));					\
			memcpy((void *) (t), (void *) (s), (size_t) (nbytes));		\
			(t) += (nbytes);						\
			(tlen) -= (nbytes);

#define MEMCLR(t, tlen, nbytes)								\
			assert((nbytes) <= (tlen));					\
			memset((void *) (t), 0, (size_t) (nbytes));			\
			(t) += (nbytes);						\
			(tlen) -= (nbytes);

#define count(b)	((b) & 31)

void
pidata_unpack(char *s, int slen, char *t, int tlen)
{
	int	i, op, c, rpt, commonSize, customSize;
	int	opcount[N_OPS];
	char	*commonData;

	memset((void *) opcount, 0, sizeof(opcount));

	while (slen > 0) {
		op = opcode(*s);
		c = count(*s);
		s++;
		slen--;

		if (c == 0) {
			FETCH_ARG(c, s, slen);
		}

		switch (op) {
		case OP_ZERO:
			/* Log(6, "t6>> OP_ZERO: %d\n", c); */
			if (c == 0) {
				/* Log(6, "t6>> slen=%d tlen=%d\n", slen, tlen); */
				exit(1);
			}
			MEMCLR(t, tlen, c);
			break;
		case OP_BCOPY:
			/* Log(6, "t6>> OP_BCOPY: %d\n", c); */
			MEMCPY(t, tlen, s, c, slen);
			s += c;
			slen -= c;
			break;
		case OP_RPT:
			rpt = 0;
			FETCH_ARG(rpt, s, slen);
			rpt++;
			/* Log(6, "t6>> OP_RPT: blocksz=%d repeat=%d\n", c, rpt); */
			for (i = 0; i < rpt; i++) {
				MEMCPY(t, tlen, s, c, slen);
			}
			s += c;
			slen -= c;
			break;
		case OP_RPTBCOPY:
		case OP_RPTZERO:
			commonSize = c;
			customSize = rpt = 0;
			FETCH_ARG(customSize, s, slen);
			FETCH_ARG(rpt, s, slen);

			/* Log(6, "t6>> %s: commonSize=%d customSize=%d repeat=%d\n",
				(op == OP_RPTBCOPY)? "OP_RPTBCOPY": "OP_RPTZERO",
				commonSize, customSize, rpt); */

			if (op == OP_RPTZERO) {
				MEMCLR(t, tlen, commonSize);
				for (i = 0; i < rpt; i++) {
					MEMCPY(t, tlen, s, customSize, slen);
					s += customSize;
					slen -= customSize;
					MEMCLR(t, tlen, commonSize);
				}
			}
			else {
				commonData = s;
				MEMCPY(t, tlen, commonData, commonSize, slen);
				s += commonSize;
				slen -= commonSize;
				for (i = 0; i < rpt; i++) {
					MEMCPY(t, tlen, s, customSize, slen);
					s += customSize;
					slen -= customSize;
					MEMCPY(t, tlen, commonData, commonSize, commonSize);
				}
			}
			break;
		default:
			Log(1, "t1>> Foreign opcode in pidata: %d\n", op);
			exit(1);
			break;
		}

		opcount[op]++;
	}

	Log(4, "t4>> Op counts:\n");
	for (i = 0; i < N_OPS; i++)
		Log(4, "t4>>   opcount[%d]: %d\n", i, opcount[i]);
}
