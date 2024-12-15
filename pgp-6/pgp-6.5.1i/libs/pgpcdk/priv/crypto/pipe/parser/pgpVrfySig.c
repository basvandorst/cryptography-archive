/*
 * pgpVrfySig.c -- Code shared between readann and verifyra to deal with
 * checking and verifying signatures.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpVrfySig.c,v 1.28 1998/12/09 18:20:36 hal Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpVrfySig.h"
#include "pgpDevNull.h"
#include "pgpHashPriv.h"
#include "pgpHashMod.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpSig.h"
#include "pgpTextFilt.h"
#include "pgpContext.h"


#define kPGPError_VERBOSE_0	0


int
pgpSigSetupPipeline (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPHashListRef *hashes,
	PGPByte const *hashlist,
	PGPUInt16 hashlen,
	PGPByte const *charmap)
{
	int num;
	PGPPipeline *mod = NULL, **tail = &mod;
	PGPError error;
	PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( cdkContext );

	pgpAssertAddrValid( head, PGPPipeline );
	pgpAssertAddrValid( hashes, PGPHashListRef );
	pgpAssertAddrValid( hashlist, PGPByte );
	
	if (!head || !hashes || !hashlist || hashlen == 0 )
		return 0;

	error = pgpHashListCreate ( memoryMgr, hashlist, hashes, hashlen);
	if( IsPGPError( error ) )
		return error;

	num = pgpHashListGetSize( *hashes );
	if (num <= 0)
		return 0;

	if (charmap) {
		tail = pgpTextFiltCreate ( cdkContext,
			tail, charmap, 0, kPGPLineEnd_CRLF);
		if (!tail) {
			pgpHashListDestroy (*hashes);
			return kPGPError_OutOfMemory;
		}
	}

	tail = pgpHashModListCreate ( cdkContext, tail, *hashes, num);
	if (!tail) {
		pgpHashListDestroy (*hashes);
		mod->teardown (mod);
		return kPGPError_OutOfMemory;
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
pgpSigSetupHashes (PGPContextRef cdkContext,
	PGPPipeline **tail,
	PGPEnv const *env,
	PGPHashListRef *hashlist,
	PGPSig const *siglist,
	PGPUICb const *ui,
	void *ui_arg)
{
	PGPUInt16 len;
	int err;
	PGPByte *buf;
	PGPHashListRef hashes;
	PGPByte const *charmap;
	PGPByte const *extra;
	unsigned extralen;

	if (!tail || !siglist || !hashlist || !ui)
		return kPGPError_BadParams;

	*hashlist = NULL;

	len = pgpSigDistinctHashCount (siglist);
	buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
		len, kPGPMemoryMgrFlags_Clear);
	if (!buf)
		return kPGPError_OutOfMemory;
	pgpSigDistinctHashes (siglist, buf);

	if (!pgpDevNullCreate (cdkContext, tail)) {
		ui->message (ui_arg, kPGPError_OutOfMemory, PGPMSG_DEVNULL_CREATE, 0);
		pgpContextMemFree( cdkContext, buf);
		return kPGPError_OutOfMemory;
	}

	extra = pgpSigExtra (siglist, &extralen);
	charmap = (PGPByte const *)pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1,
						  NULL);
	err = pgpSigSetupPipeline ( cdkContext, tail, &hashes, buf, len, 
				   ((extra && extra[0]) ? charmap : NULL));
	pgpContextMemFree( cdkContext, buf);
	if ( IsPGPError( err ) ) {
		(*tail)->teardown (*tail);
		*tail = NULL;
	}

	if ( pgpHashListGetSize( hashes ) == 0) {
		(*tail)->teardown (*tail);
		*tail = NULL;
		ui->message (ui_arg, kPGPError_VERBOSE_0, PGPMSG_SIG_NO_CHECK, 0);
		pgpHashListDestroy( hashes );
		return kPGPError_CantHash;
	}

	*hashlist = hashes;
	return 0;
}

/*
 * Verify a Signature with a list of hashes
 */
	PGPError
pgpSigVerify (PGPSig const *siglist, PGPHashListRef hashes,
	 PGPUICb const *ui, void *ui_arg)
{
	unsigned				i;
	unsigned				len, extralen;
	PGPByte const *			buf;
	PGPByte const *			extra;
	PGPHashContext *		temp_hc;
	PGPSig const *			sig;
	PGPError				err = kPGPError_NoErr;
	PGPUInt32				numHashes;

	PGPValidatePtr( siglist );
	PGPValidatePtr( hashes );
	PGPValidatePtr( ui );

	/* Now, verify the signatures */
	numHashes	= pgpHashListGetSize( hashes );
	len = 0;
	for (i = 0; i < numHashes; i++)
	{
		PGPHashVTBL const *	vtbl;
		PGPHashContext *	hashContext;
		
		hashContext	= pgpHashListGetIndHash( hashes, i );
		vtbl		= pgpHashGetVTBL( hashContext );
		
		if (len < vtbl->hashsize)
			len = vtbl->hashsize;
	}
	
	for (sig = siglist; IsntNull( sig ); sig = pgpSigNext (sig))
	{
		PGPHashContextRef	hc	= NULL;
	
		hc = pgpHashListFind (hashes, pgpSigHash (sig));
		if ( IsNull( hc ) )
		{
			/* not necessarily an error; could be a future format */
			ui->message (ui_arg, kPGPError_BadHashNumber,
				     PGPMSG_SIG_BADHASH, 0);
			continue;
		}
		
		err = PGPCopyHashContext(hc, &temp_hc );
		if ( IsPGPError( err ) )
			break;

		extra = pgpSigExtra (sig, &extralen);

		PGPContinueHash (temp_hc, extra, extralen);
		buf		= (PGPByte *) pgpHashFinal (temp_hc);
		/* buf is not allocated so we don't need to free it */
		err 	= ui->sigVerify (ui_arg, sig, buf);
		PGPFreeHashContext (temp_hc);
		if ( IsPGPError( err ) )
			break;
	}
	
	return err;
}

int
pgpSepsigVerify (PGPSig const *siglist, PGPEnv const *env,
		 PGPUICb const *ui, void *ui_arg)
{
	PGPPipeline *head = 0, **tail = &head;
	PGPHashListRef	hashes = NULL;
	int err = 0;

	if (!siglist) {
		ui->message (ui_arg, kPGPError_BadParams, PGPMSG_SIG_NOSIGS, 0);
		return 0;
	}

	ui->message (ui_arg, kPGPError_VERBOSE_0, PGPMSG_SEPSIG, 0);

	err = pgpSigSetupHashes ( pgpenvGetContext( env ),
			tail, env, &hashes, siglist, ui, ui_arg);
	if (err)
		goto cleanup2;
		
	/* Ask the UI to input the message to verify the signature */
	err = ui->needInput (ui_arg, head);
	if (err)
		goto cleanup2;

	err = pgpSigVerify (siglist, hashes, ui, ui_arg);

cleanup2:
	if (head)
		head->teardown (head);
	pgpHashListDestroy (hashes);
	return err;
}
/*	requires pgpSigListDestroy (); */
