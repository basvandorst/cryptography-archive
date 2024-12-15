/*
 * spgpABuf.c -- Simple PGP API Analyze buffer
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpABuf.c,v 1.7.2.1 1997/06/07 09:51:46 mhw Exp $
 */

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int
SimplePGPAnalyzeBufferX (char *Buffer, size_t Len,
	SPGPProgressCallBack callBack, void *callBackArg)
	{
		PgpEnv	*env;	/* Dummy environment handle */
		PgpPipeline	*head,			/* Pipeline head pointer */
			**tail;	/* Pipeline tail pointer */
		PgpUICb	ui;			/* Callback functions */
		SPgpSimpUI	ui_arg;	/* Parameters for callbacks */
		int	err;	 		/* Error variable */

	head = NULL;
	env = NULL;

	if (!Buffer) {
		return SIMPLEPGPANALYZEBUFFER_BUFFERPOINTERISNULL;
	}
	if (!Len) {
		return SIMPLEPGPANALYZEBUFFER_BUFFERLENGTHISZERO;
	}

	/* Setup the UI callback functions & args */
	spgpUISetup (&ui, &ui_arg);
	ui_arg.analyze = -1;					/* Request analysis */

	/* Set up pipeline */
	head = NULL;
	env = pgpenvCreate ();
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	err = spgpMemPump (head, (byte *)Buffer, Len, callBack, callBackArg);
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
SimplePGPAnalyzeBuffer (char *Buffer, size_t Len)
{
	return SimplePGPAnalyzeBufferX (Buffer, Len, NULL, NULL);
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
