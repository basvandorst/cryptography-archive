/*
* spgpdecrypt.c -- Simple PGP API helper -- Return status of decryption
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpdecrypt.c,v 1.3.2.1 1997/06/07 09:51:54 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

/* Helper routine to append (non-null-terminated) strings */
static PGPError
sappend (byte **ptr, size_t *ptrlen, byte const *new, size_t newlen)
{
	if (pgpRealloc ((void **)ptr, *ptrlen + newlen + 1))
		return PGPERR_NOMEM;
	memcpy (*ptr+*ptrlen, new, newlen);
	*ptrlen += newlen;
	return PGPERR_OK;
}

/*
* Fill in pointer fields with decryption status info, set return value
* appropriately from the ui_arg structure.
*/
PGPError
spgpDecryptStatus (SPgpSimpUI *ui_arg, RingSet const *rspub, char *OtherBuf,
	size_t *OtherBufLen)
{
	PGPError			err;
	byte				namebuf[11];	/* Holds 0xXXXXXXXX */
	byte			*buf;
	size_t				buflen;
	int					i;

	if (ui_arg->encfail) {
		if (ui_arg->failesk)
			err = SIMPLEPGP_BADKEYPASSPHRASE;
		else if (ui_arg->failpass)
			err = SIMPLEPGP_BADCONVENTIONALPASSPHRASE;
		else
			err = SIMPLEPGP_NONEXISTENTDECRYPTIONKEY;
	} else if (ui_arg->keyfail) {
		err = SIMPLEPGPRECEIVEBUFFER_ISKEY;
	} else if (ui_arg->outref || ui_arg->outbuf) {
		/* These are set to NULL by callback code unless we got no PGP data */
		err = SIMPLEPGP_NOPGPDATA;
	} else {
		err = 0;
		if (OtherBuf && OtherBufLen) {
			/* Return info about other recipients */
			if (ui_arg->nesk == 0 && ui_arg->npass == 0) {
				/* No other recipients */
				*OtherBuf = '\0';
			} else {
				buf = pgpMemAlloc (1);
			if (!buf)
					return PGPERR_NOMEM;
				*buf = '\0';
				buflen = 0;
				for (i=0; i<min(ui_arg->nesk, SPGPUI_ESKMAX); ++i) {
					char const *sname;
					size_t namelen;
					sname = spgpNameByID8 (rspub, ui_arg->otheralg[i],
										ui_arg->otheresk[i], &namelen);
					if (!sname) {
						namebuf[0] = '0';
						namebuf[1] = 'x';
						spgpKeyIDText8 (namebuf+2, ui_arg->otheresk[i]);
						sname = (char const *)namebuf;
						namelen = 10;
					}
					if (i > 0) {
						if (sappend (&buf, &buflen, ", ", 2)) {
							pgpFree (buf);
							return PGPERR_NOMEM;
						}
					}
					if (sappend (&buf, &buflen, sname, namelen)) {
						pgpFree (buf);
						return PGPERR_NOMEM;
					}
				}
				if (i < ui_arg->nesk) {
					byte tmpbuf[120];
					if (ui_arg->npass) {
						sprintf (tmpbuf,
						", plus %d other recipients and %d pass phrases.",
						ui_arg->nesk - i, ui_arg->npass);
					} else {
						sprintf (tmpbuf, ", plus %d other recipients.",
								ui_arg->nesk - i);
					}
					if (sappend (&buf, &buflen, tmpbuf, strlen(tmpbuf))) {
						pgpFree (buf);
						return PGPERR_NOMEM;
					}
				}
				memcpy (OtherBuf, buf, min(*OtherBufLen, buflen));
				*OtherBufLen = buflen;
				pgpFree (buf);
			}
		}
	}
	return err;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
