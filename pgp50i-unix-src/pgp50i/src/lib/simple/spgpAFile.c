/*
 * spgpAFile.c -- Simple PGP API Analyze file
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpAFile.c,v 1.10.2.1 1997/06/07 09:51:46 mhw Exp $
 */

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"


int
SimplePGPAnalyzeFileX (PGPFileRef *Fileref, SPGPProgressCallBack callBack,
		void *callBackArg)
	{
		PgpEnv	*env;	/* Dummy environment handle */
		PgpFileRead	*pfrin;		/* Data file handle */
		PgpPipeline	*head,			/* Pipeline head pointer */
			**tail;	/* Pipeline tail pointer */
		PgpUICb	ui;			/* Callback functions */
		SPgpSimpUI	ui_arg;	/* Parameters for callbacks */
		int	err;	 		/* Error variable */

	env = NULL;
	head = NULL;

	/* Setup the UI callback functions & args */
	spgpUISetup (&ui, &ui_arg);
	ui_arg.analyze = -1;			/* Request analysis */

	/* Open input file */
	pfrin = pgpFileRefReadCreate (Fileref, kPGPFileOpenReadPerm, NULL);
	if (!pfrin) {
		err = SIMPLEPGPANALYZEFILE_FILENAMEDOESNOTEXIST;
		goto error;
	}
	pgpFileReadSetCallBack(pfrin, callBack, callBackArg);

	/* Set up pipeline */
	head = NULL;
	env = pgpenvCreate ();
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	err = pgpFileReadPump (pfrin, head);
	pgpFileReadDestroy (pfrin);
	head->sizeAdvise (head, 0);
	head->teardown (head);
	head = NULL;

	if (err != PGPERR_OK && err != PGPERR_INTERRUPTED)
		goto error;

	err = SIMPLEANALYZE_UNKNOWN;
	if (ui_arg.analyze == PGPANN_PGPKEY_BEGIN) {
		err = SIMPLEANALYZE_KEY;
	} else if (ui_arg.analyze == PGPANN_CIPHER_BEGIN) {
		err = SIMPLEANALYZE_ENCR;
	} else if (ui_arg.analyze == PGPANN_CLEARSIG_BEGIN ||
			ui_arg.analyze == PGPANN_SIGNED_BEGIN ||
			ui_arg.analyze == PGPANN_COMPRESSED_BEGIN) {
		err = SIMPLEANALYZE_SIGN;
	} else if (ui_arg.analyze == PGPANN_SIGNED_SEP) {
		err = SIMPLEANALYZE_DETACHEDSIG;
	}

error:
	if (head)
		head->teardown (head);
	if (env)
		pgpenvDestroy (env);
	return err;
}


int
SimplePGPAnalyzeFile (char *Filename)
{
	PGPFileRef			*ref;
	PGPError			err;

	if (!Filename) {
		return SIMPLEPGPANALYZEFILE_FILENAMEPOINTERISNULL;
	}
	if (!Filename[0]) {
		return SIMPLEPGPANALYZEFILE_FILENAMESTRINGISEMPTY;
	}

	ref = pgpNewFileRefFromFullPath (Filename);
	if (!ref) {
		err = SIMPLEPGPANALYZEFILE_NOTENOUGHMEMORY;
	} else {
		err = SimplePGPAnalyzeFileX (ref, NULL, NULL);
	}

	if (ref)
		pgpFreeFileRef (ref);
	return err;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
