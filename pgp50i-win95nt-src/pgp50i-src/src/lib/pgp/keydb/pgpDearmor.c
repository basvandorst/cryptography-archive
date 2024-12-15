/*
* pgpDearmor.c -- Simple dearmoring module, with armoring routines to
* remove ascii armor from key files (or mem buffers)
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpDearmor.c,v 1.16.2.3 1997/06/07 09:50:21 mhw Exp $
*/

	#ifdef HAVE_CONFIG_H
	#include "config.h"
	#endif

	#include "pgpKDBint.h"

	#include "pgpAnnotate.h"
	#include "pgpArmor.h"
	#include "pgpDecPipe.h"
	#include "pgpDevNull.h"
	#include "pgpEnv.h"
	#include "pgpESK.h"
	#include "pgpFIFO.h"
	#include "pgpFile.h"
	#include "pgpFileMod.h"
	#include "pgpMemMod.h"
	#include "pgpPipeline.h"
	#include "pgpRngRead.h"
	#include "pgpSig.h"
	#include "pgpTextFilt.h"
	#include "pgpUI.h"

	/* Define line endings */
	#if defined(MSDOS) || defined(WIN32)
	# define LINEEND	PGP_TEXTFILT_CRLF
	#elif defined(MACINTOSH)
	# define LINEEND	PGP_TEXTFILT_CR
	#else
	# define LINEEND	PGP_TEXTFILT_LF
	#endif

	/* Struct for callbacks */

	typedef struct PGPDearmorUI_ {
		byte *outbuf;			/* Output buffer, alt. to filename */
		size_t outbufsize;		/* Initial/final outbuf count */
		PgpPipeline *pipebuf;			/* Output buf pipeline pointer */
	} PGPDearmorUI;


	/* Callbacks for dearmor pass */

	static int
pgpDearmorMessage (void *arg, int type, int msg, unsigned numargs, ...)
{
	/* Avoid warnings */
	(void)arg;
	(void)type;
	(void)msg;
	(void)numargs;
	return 0;
}

static int
pgpDearmorDoCommit (void *arg, int scope)
{
	/* Avoid warnings */
	(void)arg;
	(void)scope;
	return PGPANN_PARSER_RECURSE;
}

static int
pgpDearmorNeedInput (void *arg, struct PgpPipeline *head)
{
	/* Avoid warnings */
	(void)arg;
	(void)head;
	return PGPERR_NO_FILE;
}

static int
pgpDearmorSigVerify (void *arg, struct PgpSig const *sig, byte const *hash)
{
	/* Avoid warnings */
	(void)arg;
	(void)sig;
	(void)hash;
	return 0;
}

static int
pgpDearmorEskDecrypt (void *arg, struct PgpESK const *esklist, byte *key,
		size_t *keylen,
		int (*tryKey) (void *arg, byte const *key, size_t keylen),
		void *tryarg)
{
	/* Avoid warnings */
	(void)arg;
	(void)esklist;
	(void)key;
	(void)keylen;
	(void)tryKey;
	(void)tryarg;
	return PGPANN_PARSER_EATIT;
}

/* Send keys to outbuf, everything else to bit bucket */
static int
pgpDearmorNewOutput (void *arg, struct PgpPipeline **output, int type,
	char const *suggested_name)
{
	PGPDearmorUI *ui_arg; /* Parameters for callbacks */
	int nullflag;	 /* True if discarding output */

	(void)suggested_name;	 /* Avoid warning */

	ui_arg = (PGPDearmorUI *) arg;
	nullflag = 0;

	/* Try to keep output open if we have multiple key packets in a row */
	if (*output && ui_arg->pipebuf && ui_arg->outbuf==NULL) {
		byte enable;
		if (type==PGPANN_NONPGP_BEGIN) {
				/*
			* Non-key material found between the two packets. We send a
			* disable annotation to tell the pipebuf module to discard this
			* intermediate data.
				*/
			enable = 0;
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_ENABLE, &enable, 1);
			return 0;
		} else if (type==PGPANN_PGPKEY_BEGIN) {
				/*
			* A second or later key packet after first. We may have disabled
			* the mem buffer if there was nonkey material, so re-enable it
			* now.
				*/
			enable = 1;
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_ENABLE, &enable, 1);
			return 0;
		}
	}
	/* Discard non-key packets */
	if (type != PGPANN_PGPKEY_BEGIN || ui_arg->outbuf==NULL) {
		nullflag = 1;
	}

	/* Close old output pipeline */
	if (*output) {
		if (ui_arg->pipebuf) {
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_BYTECOUNT, (byte *)&ui_arg->outbufsize,
				sizeof(ui_arg->outbufsize));
			ui_arg->pipebuf = 0;
		}
		(*output)->sizeAdvise (*output, 0);
		(*output)->teardown (*output);
		*output = NULL;
	}

	/* Handle discarded data */
	if (nullflag) {
		pgpDevNullCreate (output);
		return 0;
	}

	pgpAssert (ui_arg->outbuf);
	ui_arg->pipebuf = pgpMemModCreate (output, (char *)ui_arg->outbuf,
		ui_arg->outbufsize);
	ui_arg->outbuf = NULL;		/* Discard any following keys */
	if (!ui_arg->pipebuf)
		return PGPERR_NOMEM;

return PGPERR_OK;
}





/*
* Common code for key dearmoring to a dynamically allocated memory buffer.
* Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
* passing to pgpMemFree. *OutputBufferLen holds the size of the buffer.
* fFilein determines whether we take the key data from InputFileName or
* from InputBuffer/InputBufferLen.
*/
static int
pgpDearmorKey (PgpEnv *env, int fFilein, PGPFileRef *InputFileRef,
	byte *InputBuffer, size_t InputBufferLen,
	byte **OutputBuffer, size_t *OutputBufferLen)
	{
		PgpPipeline 	*head,		/* Pipeline head */
				**tail;		/* Pipeline tail pointer */
		PgpFileRead	*pfrin;		/* Input file handle */
		PgpUICb 	ui;		/* Callback functions */
		PGPDearmorUI 	ui_arg;		/* Parameters for callbacks */
		byte 		*outbuf;	/* Allocated output buffer */
		size_t	outbuflen; 	/* Size of outbuf */
		int		err;	   	/* Error variable */
		byte		dum1[1]; 	/* Dummy array for sizing pass */

		head = NULL;

	/* Setup the UI callback functions */
	ui.message		= pgpDearmorMessage;
	ui.doCommit	= pgpDearmorDoCommit;
	ui.newOutput	= pgpDearmorNewOutput;
	ui.needInput	= pgpDearmorNeedInput;
	ui.sigVerify	= pgpDearmorSigVerify;
	ui.eskDecrypt	 = pgpDearmorEskDecrypt;
	ui.annotate	= NULL;

	/* Set up the UI argument. First pass will just get length. */
	memset (&ui_arg, 0, sizeof (ui_arg));
	ui_arg.outbuf = dum1;	 		 /* Dummy output buffer */
	ui_arg.outbufsize = 0; /* Will get length afterwards */

	/* Set up pipeline */
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	if (fFilein) {
		/* Open input file */
		pfrin = pgpFileRefReadCreate(InputFileRef,kPGPFileOpenReadPerm,&err);
		if (!pfrin)
			goto error;
		err = pgpFileReadPump (pfrin, head);
		pgpFileReadDestroy (pfrin);
		head->sizeAdvise (head, 0);
	} else {
		/* Send data through */
		byte *ibuf = InputBuffer;
		size_t ibuflen = InputBufferLen;
		err = head->sizeAdvise (head, ibuflen);
		while (ibuflen && !err) {
			size_t written;
			written = head->write (head, ibuf, ibuflen, &err);
			ibuf += written;
			ibuflen -= written;
		}
		head->sizeAdvise (head, 0);
	}
	if (!err) {
		/* Find out how many bytes we wrote */
		if (!ui_arg.pipebuf) {
			/*
			* May have been closed in callback on multipart, or there may
			* have been no ascii armor part at all.
			*/
			outbuflen = ui_arg.outbufsize;
		} else {
			(ui_arg.pipebuf)->annotate(ui_arg.pipebuf, NULL,
				PGPANN_MEM_BYTECOUNT, (byte *)&outbuflen, sizeof(outbuflen));
		}
	}
	head->teardown (head);
	head = NULL;
	ui_arg.pipebuf = NULL;
	if (err)
		goto error;

	/* Now we know the size needed, allocate a buffer to hold it */
	outbuf = (byte *)pgpMemAlloc (outbuflen);
	if (!outbuf) {
		err = PGPERR_NOMEM;
		goto error;
	}
	ui_arg.outbuf = outbuf;
	ui_arg.outbufsize = outbuflen;

	/* Send data through again, this time with enough room */
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);
	if (fFilein) {
		pfrin = pgpFileRefReadCreate(InputFileRef,kPGPFileOpenReadPerm,&err);
		if (!pfrin)
			goto error;
		err = pgpFileReadPump (pfrin, head);
		pgpFileReadDestroy (pfrin);
		head->sizeAdvise (head, 0);
	} else {
		/* Send data through */
		byte *ibuf = InputBuffer;
		size_t ibuflen = InputBufferLen;
		err = head->sizeAdvise (head, ibuflen);
		while (ibuflen && !err) {
			size_t written;
			written = head->write (head, ibuf, ibuflen, &err);
			ibuf += written;
			ibuflen -= written;
		}
		head->sizeAdvise (head, 0);
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
pgpDearmorKeyFile (PgpEnv *env, PGPFileRef *InputFileRef,
	byte **OutputBuffer, size_t *OutputBufferLen)
{
	return pgpDearmorKey (env, TRUE, InputFileRef, NULL, (size_t)0,
		OutputBuffer, OutputBufferLen);
}

/*
* Dearmor a key from a buffer and put it into a dynamically allocated memory
* buffer.
* Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
* passing to pgpMemFree. *OutputBufferLen holds the size of the buffer.
*/
int
pgpDearmorKeyBuffer (PgpEnv *env, byte *InputBuffer, size_t InputBufferLen,
	byte **OutputBuffer, size_t *OutputBufferLen)
{
	return pgpDearmorKey (env, FALSE, NULL, InputBuffer, InputBufferLen,
		OutputBuffer, OutputBufferLen);
}




/* Some armoring routines too */



/*
* Write out the specified RingSet in ascii armor form to pipeline.
*/
static int
pgpWriteArmoredSetPipe (PgpPipeline *pipe, RingSet const *set, PgpEnv *env)
{
	PgpFile *pfp;
	int rslt;

	if (!pgpArmorWriteCreate(&pipe, env, &pgpByteFifoDesc,
				NULL, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		pipe->teardown(pipe);
		return PGPERR_NOMEM;
	}
	pfp = pgpFilePipelineOpen(pipe);
	if (!pfp) {
		pipe->teardown(pipe);
		return PGPERR_NOMEM;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, 0);
	pgpFileClose (pfp);
	return rslt;
}

/* Write out the specified RingSet in ascii armor form to fp. */
int
pgpWriteArmoredSetFile (FILE *fp, RingSet const *set, PgpEnv *env)
{
	PgpFile *pfp;
	PgpPipeline *ppl = NULL;
	
	pfp = pgpFileWriteOpen(fp, NULL);
	if (!pfp) {
		pgpStdIOClose(fp);
		return PGPERR_NOMEM;
	}
	if (!pgpFileWriteCreate (&ppl, pfp, 1)) {
		pgpFileClose(pfp);
		return PGPERR_NOMEM;
	}
	return pgpWriteArmoredSetPipe (ppl, set, env);
}

/*
* Write out the specified RingSet in ascii armor form to buf.
* Set *buflen to number of bytes we wrote or would have written.
* Read the size of buf from *buflen on entry.
*/
int
pgpWriteArmoredSetBuffer (byte *buf, size_t *buflen, RingSet const *set,
		PgpEnv *env)
	{
	PgpPipeline	*ppl = NULL,
			*pipebuf;
		PgpFile	*pfp;
		size_t	nbuf = *buflen,
			written;
	int rslt;
	
	pipebuf = pgpMemModCreate (&ppl, (char *) buf, nbuf);

	/* Convert to local line endings */
	if (!pgpTextFiltCreate (&ppl,
			pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL, NULL),
			0, LINEEND)) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}

	if (!pgpArmorWriteCreate(&ppl, env, &pgpByteFifoDesc,
				NULL, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}
	pfp = pgpFilePipelineOpen(ppl);
	if (!pfp) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, 0);
	if (!rslt) {
		/* Figure out how many bytes would be written */
		ppl->sizeAdvise (ppl, 0);
		pipebuf->annotate (pipebuf, NULL, PGPANN_MEM_BYTECOUNT,
				(byte *)&written, sizeof (written));
		*buflen = written;
	}
	pgpFileClose (pfp);
	return rslt;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
