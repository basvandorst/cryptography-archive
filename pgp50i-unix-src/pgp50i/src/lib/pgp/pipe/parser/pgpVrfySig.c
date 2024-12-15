/*
* pgpVrfySig.c -- Code shared between readann and verifyra to deal with
* checking and verifying signatures.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:  Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpVrfySig.c,v 1.5.2.1 1997/06/07 09:51:02 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpVrfySig.h"
#include "pgpDevNull.h"
#include "pgpHash.h"
#include "pgpHashMod.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpSig.h"
#include "pgpTextFilt.h"
#include "pgpUsuals.h"

int
pgpSigSetupPipeline (struct PgpPipeline **head, int *numhashes,
			struct PgpHashContext **hashes,
			byte const *hashlist, unsigned hashlen,
			byte const *charmap)
	{
		int num;
struct PgpPipeline *mod = NULL, **tail = &mod;

if (!head || !hashes || !hashlist || !hashlen || !numhashes)
 return 0;

num = *numhashes = pgpHashListCreate (hashlist, hashes, hashlen);
if (num <= 0)
 return 0;

if (charmap) {
			tail = pgpTextFiltCreate (tail, charmap, 0, PGP_TEXTFILT_CRLF);
			if (!tail) {
			 pgpHashListDestroy (*hashes, num);
			 return PGPERR_NOMEM;
			}
		}

		tail = pgpHashModListCreate (tail, *hashes, num);
		if (!tail) {
		 pgpHashListDestroy (*hashes, num);
		 mod->teardown (mod);
		 return PGPERR_NOMEM;
		}

		/* splice in the modules */
		*tail = *head;
		*head = mod;
		return 0;
	}

/*
* Given a bunch of signatures, setup a string of hash modules and
* hashes in order to process the signatures.
*/
int
pgpSigSetupHashes (struct PgpPipeline **tail, struct PgpEnv const *env,
		 struct PgpHashContext **hashlist, int *hashnum,
		 struct PgpSig const *siglist,
		struct PgpUICb const *ui, void *ui_arg)
	{
		unsigned len;
		int numhashes, err;
		byte *buf;
		struct PgpHashContext *hashes;
		byte const *charmap;
		byte const *extra;
		unsigned extralen;

		if (!tail || !siglist || !hashlist || !hashnum || !ui)
		 return PGPERR_BADPARAM;

		*hashlist = NULL;
		*hashnum = 0;

		len = pgpSigDistinctHashCount (siglist);
		buf = (byte *)pgpMemAlloc (len);
		if (!buf)
		 return PGPERR_NOMEM;
		pgpSigDistinctHashes (siglist, buf);

		if (!pgpDevNullCreate (tail)) {
		 ui->message (ui_arg, PGPERR_NOMEM, PGPMSG_DEVNULL_CREATE, 0);
		 pgpMemFree (buf);
		 return PGPERR_NOMEM;
		}

		extra = pgpSigExtra (siglist, &extralen);
		charmap = (byte const *)pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1,
		     NULL, NULL);
		err = pgpSigSetupPipeline (tail, &numhashes, &hashes, buf, len,
		   ((extra && extra[0]) ? charmap : NULL));
		pgpMemFree (buf);
		if (err) {
		 (*tail)->teardown (*tail);
		}
		if (numhashes < 0) {
		 (*tail)->teardown (*tail);
		 return numhashes;
		}
if (numhashes == 0) {
 (*tail)->teardown (*tail);
 ui->message (ui_arg, PGPERR_VERBOSE_0, PGPMSG_SIG_NO_CHECK, 0);
 return PGPERR_CANNOT_HASH;
}

*hashlist = hashes;
*hashnum = numhashes;
return 0;
}

/*
* Verify a Signature with a list of hashes
*/
int
pgpSigVerify (struct PgpSig const *siglist, struct PgpHashContext *hashes,
		int numhashes, struct PgpUICb const *ui, void *ui_arg)
	{
		int i;
		unsigned len, extralen;
		byte const *buf;
		byte const *extra;
		struct PgpHashContext const *hc;
		struct PgpHashContext *temp_hc;
		struct PgpSig const *sig;
		int err = 0;

		if (!siglist || !hashes || !ui)
		 return 0;

		/* Now, verify the signatures */
		len = 0;
		for (i = 0; i < numhashes; i++) {
		 if (len < hashes[i].hash->hashsize)
		  len = hashes[i].hash->hashsize;
		}
		for (sig = siglist; sig; sig = pgpSigNext (sig)) {
			hc = pgpHashListFind (hashes, numhashes, pgpSigHash (sig));
			if (!hc) {
				ui->message (ui_arg, PGPERR_BAD_HASHNUM,
				 PGPMSG_SIG_BADHASH, 0);
				continue;
			}

			temp_hc = pgpHashClone (hc);
			if (!temp_hc) {
			 ui->message (ui_arg, PGPERR_NOMEM,
			  PGPMSG_ALLOC_HASH_CLONE, 0);
			 continue;
			}
			extra = pgpSigExtra (sig, &extralen);

		pgpHashUpdate (temp_hc, extra, extralen);
			buf = pgpHashFinal (temp_hc);
			err = ui->sigVerify (ui_arg, sig, buf);
			pgpHashDestroy (temp_hc);
		}
		return 0;
}

int
pgpSepsigVerify (struct PgpSig const *siglist, struct PgpEnv const *env,
		 struct PgpUICb const *ui, void *ui_arg)
	{
		struct PgpPipeline *head = 0, **tail = &head;
		struct PgpHashContext *hashes = NULL;
		int err = 0, numhashes = 0;

		if (!siglist) {
		 ui->message (ui_arg, PGPERR_SIG_ERROR, PGPMSG_SIG_NOSIGS, 0);
		 return 0;
		}

		ui->message (ui_arg, PGPERR_VERBOSE_0, PGPMSG_SEPSIG, 0);

		err = pgpSigSetupHashes (tail, env, &hashes, &numhashes, siglist,
		 ui, ui_arg);
		if (err)
		 goto cleanup2;

		/* Ask the UI to input the message to verify the signature */
		err = ui->needInput (ui_arg, head);
		if (err)
		 goto cleanup2;

		err = pgpSigVerify (siglist, hashes, numhashes, ui, ui_arg);

cleanup2:
		if (head)
		 head->teardown (head);
		pgpHashListDestroy (hashes, numhashes);
		return err;
	}
	/*	requires pgpSigListDestroy (); */
