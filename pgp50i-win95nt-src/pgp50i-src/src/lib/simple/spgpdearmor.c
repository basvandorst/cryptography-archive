/*
* spgpdearmor.c -- Simple PGP API helper
* Remove ascii armor from key files (or mem buffers)
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpdearmor.c,v 1.10.2.2 1997/06/07 09:51:54 mhw Exp $
*/

#include "spgp.h"
#include "spgpint.h"


/*
* Common code for key dearmoring to a dynamically allocated memory buffer.
* Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
* passing to pgpMemFree. *OutputBufferLen holds the size of the buffer.
* fFilein determines whether we take the key data from InputFileName or
* from InputBuffer/InputBufferLen.
*/
static int
spgpDearmorKey (PgpEnv *env, int fFilein, PGPFileRef *InputFileRef,
	byte *InputBuffer, size_t InputBufferLen,
	byte **OutputBuffer, size_t *OutputBufferLen)
	{
		PgpPipeline	*head,		/* Pipeline head */
				**tail;		/* Pipeline tail pointer */
		PgpFileRead	*pfrin;		/* Input file handle */
		PgpUICb	ui;		/* Callback functions */
		SPgpSimpUI	ui_arg;		/* Parameters for callbacks */
		byte		*outbuf;	/* Allocated output buffer */
		size_t	outbuflen;	/* Size of outbuf */
		int		err;		/* Error variable */
		byte		dum1[1];	/* Dummy array for sizing pass */

		head = NULL;

	/* Setup the UI callback functions */
	spgpUISetup (&ui, &ui_arg);

	/* Set up the Simple UI argument. First pass will just get length. */
		ui_arg.env = env;						/* Environment */
		ui_arg.addkey = 1;		 				/* Request key addition */
		ui_arg.outbuf = dum1;					/* Dummy output buffer */
		ui_arg.outbufsize = 0;		/* Will get length afterwards */

	/* Set up pipeline */
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	if (fFilein) {
		/* Open input file */
		pfrin = pgpFileRefReadCreate(InputFileRef,kPGPFileOpenReadPerm,NULL);
		if (!pfrin) {
			err = SIMPLEPGPADDKEY_INPUTFILENAMEDOESNOTEXIST;
			goto error;
		}
		err = pgpFileReadPump (pfrin, head);
		pgpFileReadDestroy (pfrin);
		head->sizeAdvise (head, 0);
	} else {
		/* Send data through */
		err = spgpMemPump (head, InputBuffer, InputBufferLen, NULL, NULL);
	}
	if (!err) {
		outbuflen = 0;
		(void) spgpMemOutput (ui_arg.pipebuf, ui_arg.outbufsize, &outbuflen);
	}
	head->teardown (head);
	head = NULL;
	ui_arg.pipebuf = NULL;
	if (err)
		goto error;
	if (outbuflen == 0) {
		err = SIMPLEPGP_NOPGPDATA;
		goto error;
	}

	/* Now we know the size needed, allocate a buffer to hold it */
	outbuf = (byte *)pgpMemAlloc (outbuflen);
	if (!outbuf) {
		err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	ui_arg.outbuf = outbuf;
	ui_arg.outbufsize = outbuflen;

	/* Send data through again, this time with enough room */
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);
	if (fFilein) {
		pfrin = pgpFileRefReadCreate(InputFileRef,kPGPFileOpenReadPerm,NULL);
		if (!pfrin) {
			err = SIMPLEPGPADDKEY_INPUTFILENAMEDOESNOTEXIST;
			goto error;
		}
		err = pgpFileReadPump (pfrin, head);
		pgpFileReadDestroy (pfrin);
		head->sizeAdvise (head, 0);
	} else {
		err = spgpMemPump (head, InputBuffer, InputBufferLen, NULL, NULL);
	}
	if (!err) {
		err = spgpMemOutput (ui_arg.pipebuf, ui_arg.outbufsize,
			&outbuflen);
	}
	head->teardown (head);
	head = NULL;
	if (err) {
		pgpMemFree (outbuf);
		goto error;
	}

	/* Return resulting buffer and size */
	*OutputBuffer = outbuf;
	*OutputBufferLen = outbuflen;

error:

	if (head)
		head->teardown (head);

	return err;
}

/*
* Dearmor a key from a file and put it into a dynamically allocated memory
* buffer.
* Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
* passing to pgpMemFree. *OutputBufferLen holds the size of the buffer.
*/
int
spgpDearmorKeyFile (PgpEnv *env, PGPFileRef *InputFileRef,
	byte **OutputBuffer, size_t *OutputBufferLen)
{
	return spgpDearmorKey (env, TRUE, InputFileRef, NULL, (size_t)0,
		OutputBuffer, OutputBufferLen);
}

/*
* Dearmor a key from a buffer and put it into a dynamically allocated memory
* buffer.
* Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
* passing to pgpMemFree. *OutputBufferLen holds the size of the buffer.
*/
int
spgpDearmorKeyBuffer (PgpEnv *env, byte *InputBuffer, size_t InputBufferLen,
	byte **OutputBuffer, size_t *OutputBufferLen)
{
	return spgpDearmorKey (env, FALSE, NULL, InputBuffer, InputBufferLen,
		OutputBuffer, OutputBufferLen);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
