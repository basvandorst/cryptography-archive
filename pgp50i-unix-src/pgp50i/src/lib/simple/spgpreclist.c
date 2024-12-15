/*
* spgpreclist.c -- Simple PGP API helper routines to deal with the
* recipient list
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpreclist.c,v 1.4.2.2 1997/06/07 09:51:56 mhw Exp $
*/

#include "spgp.h"
#include "spgpint.h"

#include "pgpCipher.h"

struct alginfo {
	int n;								/* Number of algorithms */
	byte *algok;						/* # recips who can accept this */
	short *votes;	 		 			/* Preference voting, lower=better */
	int counts;							/* Number of recips */
};

/* Init the structure we will use to choose the conv encryption alg */
static void
spgpInitAlgInfo (struct alginfo *alginfo)
{
	unsigned n;

	memset (alginfo, 0, sizeof (*alginfo));
	/* Count how many ciphers we know about */
	for (n=1; pgpCipherByNumber(n); ++n)
		;
	n -= 1;
	alginfo->n = n;
	alginfo->algok = pgpMemAlloc (n);
	alginfo->votes = pgpMemAlloc (2*n);
	pgpAssert (alginfo->algok && alginfo->votes);
	memset (alginfo->algok, 0, n);
	memset (alginfo->votes, 0, 2*n);
}
	
/*
* Add info for the preferred algs for one recipient. We record which algs
* are acceptable and then use preference voting.
*/
static void
spgpRecordPrefs (struct alginfo *alginfo, byte const *prefs, size_t plen)
{
	unsigned i;
	byte alg;

	++alginfo->counts;
	for (i=0; i<plen; ++i) {
		alg = prefs[i];
		if (alg > alginfo->n)
			continue;
		alginfo->votes[alg-1] += i;	 /* Preference voting, lower=better */
		++alginfo->algok[alg-1];
	}
}

/* Choose the most popular algorithm of those which were acceptable to all */
static PGPError
spgpChooseAlgorithm (struct alginfo *alginfo, PgpEnv *env)
{
	int bestvote = 0;	 		 		/* Silence warning */
	int bestalg = 0;
	int i;

	for (i=0; i<alginfo->n; ++i) {
		if (alginfo->algok[i] == alginfo->counts &&
			(!bestalg || alginfo->votes[i] < bestvote)) {
			bestvote = alginfo->votes[i];
			bestalg = i+1;
		}
	}
	if (!bestalg) {
		/* This means there was no algorithm which all could use. */
		return SIMPLEPGP_INCONSISTENTRECIPIENTSET;
	}
	pgpenvSetInt (env, PGPENV_CIPHER, bestalg, PGPENV_PRI_FORCE);
	return PGPERR_OK;
}
	
static void
spgpCleanupAlgInfo (struct alginfo *alginfo)
{
	pgpFree (alginfo->votes);
	pgpFree (alginfo->algok);
}

/*
* Return a list of PubKeys corresponding to the recipient list, or NULL
* on error, setting *error if so. rspub is the ringset to look in.
*/
PgpPubKey *
spgpRecipToPubkeys (char *RecipientList, RingSet const *rspub,
	int UseUntrustedKeys, PgpEnv *env, int *err)
{
	RingSet	*rsuser;	/* Ringset holding keys for userid */
	RingIterator	*riuser;	 /* Iterate over rsuser */
	PgpPubKey	*pkuser,	/* Public key for one recipient */
		*pubkeys; /* All public keys for recips */
	RingObject	*robuser;	/* RingObj for encryption key */
	byte	includewhat;/* Code for what is in recip list */
	byte const				*palgs;	/* Preferred algorithm array */
	size_t					palglen;	/* Length of palg */
	struct alginfo			alginfo;	/* Preferred algorithm choice */
	
	pubkeys = NULL;
	if (!RecipientList) {
		*err = SIMPLEPGPREADRECIPIENTLIST_NULLPOINTEROREMPTYLIST;
		return NULL;
	}
	includewhat = *(byte *)RecipientList++;
	if (includewhat != INCLUDE_ONLYUSERIDS &&
		includewhat != INCLUDE_ONLYKEYIDS &&
		includewhat != INCLUDE_BOTH) {
		*err = SIMPLEPGPREADRECIPIENTLIST_LISTDOESNOTSTARTWITHGOODCODECHAR;
		return NULL;
	}

	/* Skip name in recipient list if have names & userids */
	if (includewhat == INCLUDE_BOTH) {
		char *rlterm = strchr (RecipientList, '\n');
		if (!rlterm) {
			*err = SIMPLEPGPREADRECIPIENTLIST_LISTDOESNOTENDWITHNEWLINE;
			return NULL;
		}
		RecipientList = rlterm + 1;
	}

	spgpInitAlgInfo (&alginfo);

	/* Loop for each name or keyID in RecipientList */
	do {
		char *rlterm = strchr (RecipientList, '\n');
		if (!rlterm) {
			*err = SIMPLEPGPREADRECIPIENTLIST_LISTDOESNOTENDWITHNEWLINE;
			goto error;
		}
		*rlterm = '\0';

		rsuser = ringSetCreate (ringSetPool (rspub));
		if (!ringSetFilterSpec (rspub, rsuser, RecipientList,
								PGP_PKUSE_ENCRYPT)) {
			*rlterm = '\n';
			*err = KERNEL_NONEXIST_KEY_ERROR;
			ringSetDestroy (rsuser);
			goto error;
		}
		*rlterm = '\n';
		ringSetFreeze (rsuser);
		riuser = ringIterCreate (rsuser);
		ringIterNextObject (riuser, 1);
		robuser = ringIterCurrentObject (riuser, 1);
		if (!spgpKeyOKToEncrypt (rspub, robuser, UseUntrustedKeys)) {
			ringObjectRelease (robuser);
			ringIterDestroy (riuser);
			ringSetDestroy (rsuser);
			*err = SIMPLEPGPREADRECIPIENTLIST_CANNOTUSEUNTRUSTEDKEY;
			goto error;
		}
		palgs = ringKeyFindSubpacket (robuser, rsuser, SIGSUB_PREFERRED_ALGS,
			0, &palglen, NULL, NULL, NULL, NULL, err);
		if (!palgs && *err) {
			ringObjectRelease (robuser);
			ringIterDestroy (riuser);
			ringSetDestroy (rsuser);
			goto error;
		}
		if (!palgs) {
			byte alg = PGP_CIPHER_IDEA;
			spgpRecordPrefs (&alginfo, &alg, 1);
		} else {
			spgpRecordPrefs (&alginfo, palgs, palglen);
		}
		pkuser = ringKeyPubKey (rsuser, robuser, PGP_PKUSE_ENCRYPT);
		ringObjectRelease (robuser);
		if (!pkuser || !pkuser->encrypt) {
			*err = SIMPLEPGP_RECIPIENTKEYSIGNATUREONLY;
			if (pkuser) {
				/* Key with no encryption part, should not happen */
				pgpPubKeyDestroy (pkuser);
			} else {
				/* Problem with key itself; if UNIMP, use high level error */
				PGPError err1 = ringSetError(rsuser)->error;
				if (err1 != PGPERR_PUBKEY_UNIMP) {
					*err = err1;
				}
			}
			ringIterDestroy (riuser);
			ringSetDestroy (rsuser);
			goto error;
		}
		pgpPubKeyNext (pkuser) = pubkeys;
		pubkeys = pkuser;
		ringIterDestroy (riuser);
		ringSetDestroy (rsuser);
		RecipientList = rlterm + 1;
		if (includewhat == INCLUDE_BOTH) {
			rlterm = strchr (RecipientList, '\n');
			if (!rlterm) {
				*err = SIMPLEPGPREADRECIPIENTLIST_LISTDOESNOTENDWITHNEWLINE;
				goto error;
			}
			RecipientList = rlterm + 1;
			if (*RecipientList == '\0')
				break;
		}
	} while (*RecipientList != '\0');

	/* Choose the conventional encryption algorithm, put in env */
	*err = spgpChooseAlgorithm (&alginfo, env);
	if (*err)
		goto error;

	spgpCleanupAlgInfo (&alginfo);

	return pubkeys;

error:

	while (pubkeys) {
		PgpPubKey *pk1 = pgpPubKeyNext (pubkeys);
		pgpPubKeyDestroy (pubkeys);
		pubkeys = pk1;
	}
	
	spgpCleanupAlgInfo (&alginfo);

	return NULL;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
