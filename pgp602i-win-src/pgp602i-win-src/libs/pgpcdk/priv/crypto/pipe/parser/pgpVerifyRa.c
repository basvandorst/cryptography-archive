/*
 * pgpVerifyRa.c -- An Annotation Reader to do Signature Verification
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpVerifyRa.c,v 1.24 1997/12/12 01:14:04 heller Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpVerifyRa.h"
#include "pgpVrfySig.h"
#include "pgpAnnotate.h"
#include "pgpDevNull.h"
#include "pgpHashPriv.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpPipeline.h"
#include "pgpSig.h"
#include "pgpContext.h"

#define VERIFYRAMAGICS	0x519c43c7
#define VERIFYRAMAGICT	0x519c43c8

typedef struct VerifyRaContext {
	PGPPipeline		smod;
	PGPPipeline		tmod;
	
	PGPPipeline *	joinhead;
	PGPPipeline *	jointext;
	PGPPipeline	**	jointail;
	
	PGPHashListRef hashes;
	PGPSig *siglist;
	PGPEnv const *env;
	PGPUICb const *ui;
	void *ui_arg;
	int numhashes;
	int scope_depth;
	int ignore_depth;
	PGPByte ignore_text;
	PGPByte textclosed;
	PGPByte sigclosed;
	PGPByte textdone;
	PGPByte sigdone;
	DEBUG_STRUCT_CONSTRUCTOR( VerifyRaContext )
} VerifyRaContext;

static PGPError
verifySizeAdvise (VerifyRaContext *ctx)
{
	PGPError	error = kPGPError_NoErr;

	/* when both text and sig are done, verify the signature */
	if (	ctx->textclosed &&
			ctx->sigclosed &&
			IsntNull( ctx->siglist ) &&
			IsntNull( ctx->hashes ) )
	{
		error	= pgpSigVerify (ctx->siglist, ctx->hashes,
				      ctx->ui, ctx->ui_arg);

		pgpSigFreeList(ctx->siglist);
		ctx->siglist	= NULL;
		
		pgpHashListDestroy(ctx->hashes );
		ctx->hashes		= NULL;
		ctx->numhashes	= 0;
	}

	return error;
}

static void
verifyTeardown (PGPPipeline *myself)
{
	VerifyRaContext *ctx = (VerifyRaContext *)myself->priv;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (ctx->joinhead);
	pgpAssert (ctx->jointext);

	if (!ctx->textdone || !ctx->sigdone)
		return;

	ctx->joinhead->teardown (ctx->joinhead);
	ctx->joinhead	= NULL;
	ctx->jointext->teardown (ctx->jointext);
	ctx->jointext	= NULL;

	if (ctx->hashes) {
		pgpHashListDestroy (ctx->hashes);
		ctx->hashes = NULL;
	}		

	if (ctx->siglist) {
		pgpSigFreeList (ctx->siglist);
		ctx->siglist = 0;
	}

	pgpClearMemory( ctx,  sizeof (*ctx));
	pgpContextMemFree( cdkContext, ctx);
}

static PGPError
tFlush (PGPPipeline *myself)
{
	VerifyRaContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICT);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->jointext);

	return context->jointext->flush (context->jointext);
}

static size_t
tWrite (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	VerifyRaContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICT);
	pgpAssert (error);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->jointext);

	return context->jointext->write (context->jointext, buf, size, error);
}

static PGPError
tAnnotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	   PGPByte const *string, size_t size)
{
	VerifyRaContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICT);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->jointext);

	return context->jointext->annotate (context->jointext, origin, type,
					    string, size);
}

static PGPError
tSizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	VerifyRaContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICT);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	if (bytes)
		return kPGPError_NoErr;

	error = context->jointext->sizeAdvise (context->jointext, 0);
	if (error)
		return error;
	
	context->textclosed = 1;

	return verifySizeAdvise (context);
}

static PGPError
tTeardown (PGPPipeline *myself)
{
	VerifyRaContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICT);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	context->textdone = 1;
	verifyTeardown (myself);
	
	return kPGPError_NoErr;
}

static PGPError
sFlush (PGPPipeline *myself)
{
	VerifyRaContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICS);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	return kPGPError_NoErr;
}

static size_t
sWrite (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	VerifyRaContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICS);
	pgpAssert (error);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	(void) buf;

	if (size) {
		if (context->ignore_text)
			return size;

		*error = kPGPError_BadParams;
		return 0;
	}

	return 0;
}

static PGPError
sAnnotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	   PGPByte const *string, size_t size)
{
	VerifyRaContext *context;
	PGPError	error = kPGPError_NoErr;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICS);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	(void)origin;
	/*
	 * We only accept a few annotations here.  Things we dont know
	 * are considered an error.  Only a known set of annotations are
	 * accepted, and most of these are ignored.  We are allowed to be
	 * in armor, and only a sepsig is allowed.
	 *
	 * Text is only ignored if it is within a NONPGP scope.  Otherwise
	 * it is not ignored and an error will be returned.
	 *
	 * Commit requests are ignored, too.
	 */

	switch (type) {
	case PGPANN_ARMOR_BEGIN:
	case PGPANN_ARMOR_END:
	case PGPANN_FILE_BEGIN:
	case PGPANN_FILE_END:
	case PGPANN_INPUT_BEGIN:
	case PGPANN_INPUT_END:
	case PGPANN_COMMIT:
	case PGPANN_SIGNED_BEGIN:
	case PGPANN_SIGNED_END:
	case PGPANN_SIGNED_SEP:
		break;

	case PGPANN_NONPGP_BEGIN:
		if (context->ignore_text)
			break;

		context->ignore_text = 1;
		context->ignore_depth = context->scope_depth + 1;
		break;

	case PGPANN_NONPGP_END:
		pgpAssert (context->ignore_text);

		if (context->ignore_depth == context->scope_depth)
			context->ignore_text = 0;

		break;

	case PGPANN_SIGNED_SIG:
		error = (PGPError)pgpSigAdd ( cdkContext,
			&context->siglist, type, string, size);
		if (error) {
			context->ui->message (context->ui_arg, error,
					      PGPMSG_SIG_ADD_ERROR, 0);
		}
		break;

	default:
		error = kPGPError_BadParams;
	}

	PGP_SCOPE_DEPTH_UPDATE (context->scope_depth, type);
	pgpAssert (context->scope_depth != -1);

	return error;
}

static PGPError
sSizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	VerifyRaContext *context;
	PGPError	error = kPGPError_NoErr;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICS);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	if (bytes)
		return kPGPError_NoErr;

	/* setup the hashes, then close the join module */
	if (!context->siglist)
		return kPGPError_NoErr;	/* No signatures??? */
	
	if (!context->numhashes)
		error = (PGPError)pgpSigSetupHashes(
			myself->cdkContext,
			context->jointail, context->env,
			 &context->hashes,
			 context->siglist, context->ui, context->ui_arg);
	if (error)
		return error;
	context->numhashes	= pgpHashListGetSize( context->hashes );
	
	/* now turn on the pipe to verify the text */
	error = context->joinhead->sizeAdvise (context->joinhead, 0);
	if (error)
		return error;
	
	context->sigclosed = 1;

	return verifySizeAdvise (context);
}

static PGPError
sTeardown (PGPPipeline *myself)
{
	VerifyRaContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == VERIFYRAMAGICS);

	context = (VerifyRaContext *)myself->priv;
	pgpAssert (context);

	context->sigdone = 1;
	verifyTeardown (myself);
	
	return kPGPError_NoErr;
}

PGPPipeline *
pgpVerifyReaderCreate (
	PGPContextRef cdkContext,
	PGPPipeline **texthead,
	PGPPipeline **sighead,
	PGPEnv const *env, PGPFifoDesc const *fd,
	PGPByte const *hashlist, unsigned hashlen, int textmode,
	PGPUICb const *ui, void *ui_arg)
{
	PGPPipeline *smod, *tmod;
	PGPPipeline *	joinhead = NULL;
	PGPPipeline *	jointext	= NULL;
	PGPPipeline **	jointail	= NULL;
	VerifyRaContext *context	= NULL;
	PGPHashListRef hashes = NULL;
	int numhashes = 0, err;

	if (!texthead || !sighead || !env)
		return NULL;

	context = (VerifyRaContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear);
	if (!context)
		return NULL;
	smod = &context->smod;
	tmod = &context->tmod;
	
	jointail = pgpJoinCreate ( cdkContext, &joinhead, fd);
	if (!jointail) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	jointext = pgpJoinAppend (joinhead);
	if (!jointext) {
		joinhead->teardown (joinhead);
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	if (hashlist && hashlen) {
		PGPByte const *charmap;

		if (!pgpDevNullCreate ( cdkContext, jointail)) {
			joinhead->teardown (joinhead);
			jointext->teardown (jointext);
			pgpContextMemFree( cdkContext, context);
			return NULL;
		}
		charmap = (PGPByte const *)
			pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL);
		err = pgpSigSetupPipeline ( cdkContext, jointail, &hashes,
					   hashlist, (PGPUInt16)hashlen,
					   (textmode ? charmap : NULL));
		numhashes	= IsntNull( hashes ) ? pgpHashListGetSize( hashes ) : 0;
		if (err || numhashes <= 0) {
			joinhead->teardown (joinhead);
			jointext->teardown (jointext);
			pgpContextMemFree( cdkContext, context);
			return NULL;
		}
		err = joinhead->sizeAdvise (joinhead, 0);
		if (err) {
			joinhead->teardown (joinhead);
			jointext->teardown (jointext);
			pgpHashListDestroy (hashes);
			pgpContextMemFree( cdkContext, context);
			return NULL;
		}
	}

	smod->magic = VERIFYRAMAGICS;
	smod->write = sWrite;
	smod->flush = sFlush;
	smod->sizeAdvise = sSizeAdvise;
	smod->annotate = sAnnotate;
	smod->teardown = sTeardown;
	smod->name = "Signature Verification Annotation Reader (sighead)";
	smod->priv = context;
	smod->cdkContext	= cdkContext;

	tmod->magic = VERIFYRAMAGICT;
	tmod->write = tWrite;
	tmod->flush = tFlush;
	tmod->sizeAdvise = tSizeAdvise;
	tmod->annotate = tAnnotate;
	tmod->teardown = tTeardown;
	tmod->name = "Signature Verification Annotation Reader (texthead)";
	tmod->priv = context;
	tmod->cdkContext	= cdkContext;

	context->joinhead = joinhead;
	context->jointext = jointext;
	context->jointail = jointail;
	context->env = env;
	context->ui = ui;
	context->ui_arg = ui_arg;
	context->hashes = hashes;
	context->numhashes = numhashes;

	*sighead = smod;
	*texthead = tmod;
	return smod;
}
