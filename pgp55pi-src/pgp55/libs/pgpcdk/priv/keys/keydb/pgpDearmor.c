/*
 * pgpDearmor.c
 * Simple dearmoring module, with armoring routines too
 *
 * Remove ascii armor from key files (or mem buffers)
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 * $Id: pgpDearmor.c,v 1.29 1997/09/18 01:35:34 lloyd Exp $
 */
#include <string.h>

#include "pgpConfig.h"

#include "pgpKDBInt.h"

#include "pgpAnnotate.h"
#include "pgpArmor.h"
#include "pgpDecPipe.h"
#include "pgpDevNull.h"
#include "pgpEncodePriv.h"
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
#include "pgpContext.h"


/* Struct for callbacks */

typedef struct PGPDearmorUI
{
	PGPContextRef		cdkContext;
	PGPByte             *outbuf;		  /* Output buffer, alt. to filename */
	size_t            outbufsize;	  /* Initial/final outbuf count */
	PGPPipeline      *pipebuf;		  /* Output buf pipeline pointer */
	DEBUG_STRUCT_CONSTRUCTOR( PGPDearmorUI )
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
	return( kPGPError_NoErr );
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
pgpDearmorNeedInput (void *arg, PGPPipeline *head)
{
	/* Avoid warnings */
	(void)arg;
	(void)head;
	return kPGPError_CantOpenFile;
}

static int
pgpDearmorSigVerify (void *arg, PGPSig const *sig, PGPByte const *hash)
{
	/* Avoid warnings */
	(void)arg;
	(void)sig;
	(void)hash;
	return( kPGPError_NoErr );
}

static int
pgpDearmorEskDecrypt (void *arg, PGPESK const *esklist, PGPByte *key,
		  PGPSize *keylen,
		  int (*tryKey) (void *arg, PGPByte const *key, PGPSize keylen),
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
pgpDearmorNewOutput (void *arg, PGPPipeline **output, int type,
	char const *suggested_name)
{
	PGPDearmorUI         *ui_arg;     /* Parameters for callbacks */
	int                   nullflag;	  /* True if discarding output */
	PGPContextRef		cdkContext;

	(void)suggested_name;	/* Avoid warning */

	ui_arg = (PGPDearmorUI *) arg;
	cdkContext	= ui_arg->cdkContext;
	nullflag = 0;

	/* Try to keep output open if we have multiple key packets in a row */
	if (*output && ui_arg->pipebuf && ui_arg->outbuf==NULL) {
		PGPByte enable;
		if (type==PGPANN_NONPGP_BEGIN) {
			/*
			 * Non-key material found between the two packets.  We send a
			 * disable annotation to tell the pipebuf module to discard this
			 * intermediate data.
			 */
			enable = 0;
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_ENABLE, &enable, 1);
			return( kPGPError_NoErr );
		} else if (type==PGPANN_PGPKEY_BEGIN) {
			/*
			 * A second or later key packet after first.  We may have disabled
			 * the mem buffer if there was nonkey material, so re-enable it
			 * now.
			 */
			enable = 1;
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_ENABLE, &enable, 1);
			return( kPGPError_NoErr );
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
				PGPANN_MEM_BYTECOUNT, (PGPByte *)&ui_arg->outbufsize,
				sizeof(ui_arg->outbufsize));
			ui_arg->pipebuf = 0;
		}
		(*output)->sizeAdvise (*output, 0);
		(*output)->teardown (*output);
		*output = NULL;
	}

	/* Handle discarded data */
	if (nullflag) {
		pgpDevNullCreate ( cdkContext, output);
		return( kPGPError_NoErr );
	}

	pgpAssert (ui_arg->outbuf);
	ui_arg->pipebuf = pgpMemModCreate ( ui_arg->cdkContext,
			output, (char *)ui_arg->outbuf,
		ui_arg->outbufsize);
	ui_arg->outbuf = NULL;		/* Discard any following keys */
	if (!ui_arg->pipebuf)
		return kPGPError_OutOfMemory;

	return kPGPError_NoErr;
}





/*
 * Common code for key dearmoring to a dynamically allocated memory buffer.
 * Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
 * passing to pgpContextMemFree.  *OutputBufferLen holds the size of the buffer.
 * fFilein determines whether we take the key data from InputFileName or
 * from InputBuffer/InputBufferLen.
 */
	PGPError
pgpDearmorKey (
	PGPEnv *		env,
	int				fFilein,
	PFLFileSpecRef	InputFileRef,
	PGPFileRead *	pfrin,
	PGPByte const *	InputBuffer,
	PGPSize			InputBufferLen,
	PGPByte **		OutputBuffer,
	PGPSize *		OutputBufferLen)
{
	PGPPipeline          *head,		  /* Pipeline head */
	                    **tail;       /* Pipeline tail pointer */
	PGPUICb               ui;		  /* Callback functions */
	PGPDearmorUI          ui_arg;     /* Parameters for callbacks */
	PGPByte                 *outbuf;     /* Allocated output buffer */
	size_t                outbuflen;  /* Size of outbuf */
	PGPError              err;		  /* Error variable */
	PGPByte                  dum1[1];    /* Dummy array for sizing pass */
	PGPContextRef		cdkContext	= pgpenvGetContext( env );

	head = NULL;

	/* Setup the UI callback functions */
	ui.message		= pgpDearmorMessage;
	ui.doCommit		= pgpDearmorDoCommit;
	ui.newOutput	= pgpDearmorNewOutput;
	ui.needInput	= pgpDearmorNeedInput;
	ui.sigVerify	= pgpDearmorSigVerify;
	ui.eskDecrypt	= pgpDearmorEskDecrypt;
	ui.annotate	= NULL;

	/* Set up the UI argument.  First pass will just get length. */
	pgpClearMemory (&ui_arg, sizeof (ui_arg));
	ui_arg.outbuf = dum1;			  /* Dummy output buffer */
	ui_arg.outbufsize = 0;            /* Will get length afterwards */
	ui_arg.cdkContext	= pgpenvGetContext( env );

	/* Set up pipeline */
	tail = pgpDecryptPipelineCreate ( pgpenvGetContext( env ),
				&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	if (fFilein) {
		/* Open input file if caller has not already done so */
		if( IsNull( pfrin ) )
			pfrin = pgpFileRefReadCreate (InputFileRef, kPGPFileOpenReadPerm,
										  &err);
		if( IsNull( pfrin ) )
			goto error;
		err = pgpFileReadPump (pfrin, head);
		pgpFileReadDestroy (pfrin);
		head->sizeAdvise (head, 0);
	} else {
		/* Send data through */
		PGPByte const *ibuf = InputBuffer;
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
				PGPANN_MEM_BYTECOUNT, (PGPByte *)&outbuflen, sizeof(outbuflen));
		}
	}
	head->teardown (head);
	head = NULL;
	ui_arg.pipebuf = NULL;
	if (err)
		goto error;

	/* Now we know the size needed, allocate a buffer to hold it */
	outbuf = (PGPByte *)pgpContextMemAlloc( cdkContext,
		outbuflen, kPGPMemoryFlags_Clear);
	if (!outbuf) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	ui_arg.outbuf = outbuf;
	ui_arg.outbufsize = outbuflen;

	/* Send data through again, this time with enough room */
	tail = pgpDecryptPipelineCreate ( pgpenvGetContext( env ),
			&head, env, NULL, &ui, &ui_arg);
	if (fFilein) {
		pfrin = pgpFileRefReadCreate (InputFileRef, kPGPFileOpenReadPerm, &err);
		if (!pfrin)
			goto error;
		err = pgpFileReadPump (pfrin, head);
		pgpFileReadDestroy (pfrin);
		head->sizeAdvise (head, 0);
	} else {
		/* Send data through */
		PGPByte const *ibuf = InputBuffer;
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
		pgpContextMemFree( cdkContext, outbuf);
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
 * passing to pgpContextMemFree.  *OutputBufferLen holds the size of the buffer.
 */
	PGPError
pgpDearmorKeyFile (PGPEnv *env, PFLFileSpecRef InputFileRef,
	PGPByte **OutputBuffer, PGPSize *OutputBufferLen)
{
	return pgpDearmorKey (env, TRUE, InputFileRef, NULL, NULL, (size_t)0,
		OutputBuffer, OutputBufferLen);
}

/*
 * Dearmor a key from a buffer and put it into a dynamically allocated memory
 * buffer.
 * Returns *OutputBuffer as a buffer holding the dearmored key, suitable for
 * passing to pgpContextMemFree.  *OutputBufferLen holds the size of the buffer.
 */
	PGPError
pgpDearmorKeyBuffer (
	PGPEnv *			env,
	PGPByte const *		InputBuffer,
	PGPSize				InputBufferLen,
	PGPByte **			OutputBuffer,
	PGPSize *			OutputBufferLen)
{
	return pgpDearmorKey (env, FALSE, NULL, NULL, InputBuffer, InputBufferLen,
		OutputBuffer, OutputBufferLen);
}




/* Some armoring routines too */



/* 
 * Write out the specified RingSet in ascii armor form to pipeline.
 */
static PGPError
pgpWriteArmoredSetPipe (PGPPipeline *pipe, RingSet const *set, PGPEnv *env)
{
	PGPFile          *pfp;
	PGPError         rslt;

	if (!pgpArmorWriteCreate( pgpenvGetContext( env ),
			&pipe, env, &pgpByteFifoDesc,
				 NULL, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		pipe->teardown(pipe);
		return kPGPError_OutOfMemory;
	}
	pfp = pgpFilePipelineOpen(pipe);
	if (!pfp) {
		pipe->teardown(pipe);
		return kPGPError_OutOfMemory;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, 0);
	pgpFileClose (pfp);
	return rslt;
}

/* Write out the specified RingSet in ascii armor form to fp. */
	PGPError
pgpWriteArmoredSetFile (FILE *fp, RingSet const *set, PGPEnv *env)
{
	PGPFile          *pfp;
	PGPPipeline      *ppl = NULL;
	PGPContextRef		cdkContext	= pgpenvGetContext( env );
	
	pfp = pgpFileWriteOpen( cdkContext, fp, NULL);
	if (!pfp) {
		pgpStdIOClose(fp);
		return kPGPError_OutOfMemory;
	}
	if (!pgpFileWriteCreate ( cdkContext, &ppl, pfp, 1)) {
		pgpFileClose(pfp);
		return kPGPError_OutOfMemory;
	}
	return pgpWriteArmoredSetPipe (ppl, set, env);
}

/*
 * Write out the specified RingSet in ascii armor form to buf.
 * Set *buflen to number of bytes we wrote or would have written.
 * Read the size of buf from *buflen on entry.
 */
	PGPError
pgpWriteArmoredSetBuffer (PGPByte *buf, PGPSize *buflen, RingSet const *set,
	PGPEnv *env)
{
	PGPPipeline      *ppl = NULL,
	                 *pipebuf;
	PGPFile          *pfp;
	PGPSize            nbuf = *buflen,
	                  written;
	PGPError              rslt;
	PGPLineEndType	  lineEnd = pgpGetDefaultLineEndType();
	PGPContextRef	cdkContext	= pgpenvGetContext( env );
	
	pipebuf = (PGPPipeline *)pgpMemModCreate ( cdkContext,
				&ppl, (char *)buf, nbuf);

	/* Convert to local line endings */
	if (!pgpTextFiltCreate ( cdkContext, &ppl,
			 (PGPByte *)pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL),
			 0, lineEnd)) {
		ppl->teardown(ppl);
		return kPGPError_OutOfMemory;
	}

	if (!pgpArmorWriteCreate( cdkContext, &ppl, env, &pgpByteFifoDesc,
				 NULL, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		ppl->teardown(ppl);
		return kPGPError_OutOfMemory;
	}
	pfp = pgpFilePipelineOpen(ppl);
	if (!pfp) {
		ppl->teardown(ppl);
		return kPGPError_OutOfMemory;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, 0);
	if (!rslt) {
		/* Figure out how many bytes would be written */
		ppl->sizeAdvise (ppl, 0);
		pipebuf->annotate (pipebuf, NULL, PGPANN_MEM_BYTECOUNT,
				(PGPByte *)&written, sizeof (written));
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
