/*
* spgpsigner.c -- Simple PGP API helper
* Routines related to signatures and signature checking
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpsigner.c,v 1.8.2.1 1997/06/07 09:51:56 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"
#include "pgpTimeDate.h"


/* Fill in pointer fields with signature status info */
void
spgpSignStatus (SPgpSimpUI *ui_arg, RingSet const *rspub, int *psigstatus,
	byte *pkeyid, size_t keyidlen, char *psign, size_t signlen,
	char *pdate, size_t datelen)
{
	if (psigstatus) {
		if (!ui_arg->sigstatus)
			ui_arg->sigstatus = SIGSTS_NOTSIGNED;
		*psigstatus = ui_arg->sigstatus;
	}
	/* Set other fields only if we have reasonably non-bogus data */
	if (ui_arg->sigstatus==SIGSTS_VERIFIED ||
		ui_arg->sigstatus==SIGSTS_VERIFIED_UNTRUSTED ||
		ui_arg->sigstatus==SIGSTS_BADSIG) {
		if (pkeyid) {
			pgpAssert (keyidlen == 8);
			pgpAssertAddrValid (pkeyid, char);
			pgpCopyMemory (ui_arg->sigkeyid, pkeyid, keyidlen);
		}
		if (psign) {
			char const *sname;
			size_t namelen;
			sname = spgpNameByID8 (rspub, ui_arg->sigpkalg, ui_arg->sigkeyid,
								&namelen);
			if (!sname) {
				if (psigstatus)
					*psigstatus = SIGSTS_BADSIG;
			} else {
				namelen = min (namelen, signlen-1);
				pgpCopyMemory (sname, psign, namelen);
				psign[namelen] = '\0';
			}
		}
		if (pdate) {
			char *sctime = pgpCTime((PGPTime *)&ui_arg->sigtimedate);
			size_t timelen = strlen (sctime) - 1; /* strip newline at end */
			timelen = min (timelen, datelen-1);
			pgpCopyMemory (sctime, pdate, timelen);
			pdate[timelen] = '\0';
		}
	}
}

/*
* This returns a signing key. If no keyid is specified, we will use a
* default according to our rules. We will fill in the SignerKeyID buffer
* with the key ID we chose if its first char is null and it is big enough.
* If we can't find an appropriate key we return null.
*/
RingObject *
spgpGetSignerKey (char *SignerKeyID, size_t SignerBufferLen, PgpEnv *env,
	RingSet const *rset)
{
	char	*ssign;		/* Signing key string */
	RingObject				*skey;		/* Signing key object */

	/*
	* Note that we differ from the original simple pgp api in our
	* handling of an empty string for SignerKeyID. We require a
	* default userid key to have been specified in the preferences,
	* otherwise we return an error.
	*/
	skey = NULL;
	ssign = SignerKeyID;
	if (!ssign || !*ssign) {
		ssign = (char *)pgpenvGetString (env,PGPENV_MYNAME,NULL,NULL);
	}
	if (ssign && *ssign) {
		skey = ringLatestSecret (rset, ssign, pgpGetTime(), PGP_PKUSE_SIGN);
	} else {
		/* No default, see if there is a single secret key */
		RingIterator *iter = ringIterCreate (rset);
		if (!iter)
			return NULL;
		while (ringIterNextObject (iter, 1) > 0) {
			RingObject *keyobj = ringIterCurrentObject (iter, 1);
			pgpa(pgpaAssert(ringObjectType(keyobj) == RINGTYPE_KEY));
			if (ringKeyIsSec (rset, keyobj) &&
				(ringKeyUse(rset, keyobj) & PGP_PKUSE_SIGN)!=0) {
				if (skey) { /* already found one */
					ringObjectRelease (keyobj);
					skey = NULL;
					break;
				} else {
					skey = keyobj;		/* found first one */
				}
			}
		}
		ringIterDestroy (iter);
		ringObjectHold (skey);
	}
	if (!skey) {
		return NULL;
	}
	if (SignerKeyID && *SignerKeyID=='\0' && SignerBufferLen >= 11) {
		/* Return selected key ID */
		byte keyid[8], pkalg;
		ringKeyID8 (rset, skey, &pkalg, keyid);
		SignerKeyID[0] = '0';
		SignerKeyID[1] = 'x';
		spgpKeyIDText8 (SignerKeyID+2, keyid);
	}
	return skey;
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
