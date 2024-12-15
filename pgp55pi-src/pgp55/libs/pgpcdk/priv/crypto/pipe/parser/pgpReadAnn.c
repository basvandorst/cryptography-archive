/*
 * pgpReadAnn.c -- Annotation Reader
 *
 * Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpReadAnn.c,v 1.24 1997/08/27 02:15:15 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpReadAnn.h"
#include "pgpVrfySig.h"
#include "pgpAnnotate.h"
#include "pgpESK.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpPipeline.h"
#include "pgpSig.h"
#include "pgpTextFilt.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#define READANNOTATIONMAGIC	0xf00b0b

/* should never have more than this many nested scopes! */
#define MAX_RECURSE 50

typedef struct ReadAnnContext {
	PGPPipeline		pipe;
	
	int stack[MAX_RECURSE];
	int sp;
	int file_sp;
	int type;
	char *name;
	PGPPipeline *tail;
	PGPUICb const *ui;
	PGPEnv const *env;
	void *ui_arg;
	PGPESK *esklist;
	PGPSig *siglist[MAX_RECURSE];
	PGPByte sepsig[MAX_RECURSE];
	/* For the sigverify callback-callback to check the signature */
	PGPSig const *cursig;
	/* flag -- do we need a new output? */
	PGPByte newoutput;
	DEBUG_STRUCT_CONSTRUCTOR( ReadAnnContext )
} ReadAnnContext;

typedef struct TryCB {
	PGPPipeline *myself, *origin;
	int reply;
} TryCB;

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	ReadAnnContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (error);

	if (context->newoutput) {
		*error = (PGPError)context->ui->newOutput (context->ui_arg,
						 &context->tail,
						 context->type,
						 context->name);
		if (*error)
			return kPGPError_NoErr;

		/* Remember scope to close output */
		context->file_sp = context->sp;
		context->newoutput = 0;
	}

	pgpAssert (context->tail);
	return context->tail->write (context->tail, buf, size, error);	
}

	static PGPError
sighash(PGPPipeline *myself, PGPPipeline *origin, int reply)
{
	ReadAnnContext *context;
	PGPByte *buf;
	PGPError	 err;
	unsigned len;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	/* Build list of hashes we need */
	len = pgpSigDistinctHashCount(context->siglist[context->sp-1]);
	buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
		len, kPGPMemoryFlags_Clear);
	if (!buf)
		return kPGPError_OutOfMemory;
	pgpSigDistinctHashes(context->siglist[context->sp-1], buf);

	/* Tell the parser to do the hashes */
	err = origin->annotate (origin, myself, reply, buf, len);
	pgpContextMemFree( cdkContext, buf);
	return err;
}

/* Try out all the hashes */
/* (Note that this is called with context->sp already decremented) */
static int
sigtry (PGPPipeline *myself, PGPPipeline *origin)
{
	ReadAnnContext *context;
	PGPByte buf[100];
	PGPByte const *extra;
	unsigned extralen;
	PGPSig const *sig;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	for (sig = context->siglist[context->sp]; sig;
					  sig = pgpSigNext (sig)) {
		buf[0] = pgpSigHash(sig)->algorithm;
		extra = pgpSigExtra(sig, &extralen);
		memcpy(buf+1, extra, extralen);
		context->cursig = sig;
		/* This ends up invoking sigverify, below */
		error = origin->annotate (origin, myself,
					  PGPANN_HASH_REQUEST,
					  buf, extralen+1);
		if (error) {
			context->ui->message (context->ui_arg, error,
					      PGPMSG_SIG_ERROR, 0);
		}
	}
	context->cursig = 0;
	pgpSigFreeList (context->siglist[context->sp]);
	context->siglist[context->sp] = 0;
	return 0;
}

/* Get the callback from the callback... */
/* (Note that this is called with context->sp already decremented) */
static int
sigverify(PGPPipeline *myself, PGPByte const *string, size_t size)
{
	ReadAnnContext *context;
	PGPHashVTBL const *hash;

	(void)size;		/* Avoid warning */
	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->cursig);

	hash = pgpSigHash (context->cursig);
	pgpAssert (hash);
	pgpAssert (size == hash->hashsize);

	return context->ui->sigVerify (context->ui_arg, context->cursig,
				       string);
}

static int
sepsighash (ReadAnnContext *context)
{
	int err;

	err = pgpSepsigVerify (context->siglist[context->sp-1], context->env,
			       context->ui, context->ui_arg);
	pgpSigFreeList (context->siglist[context->sp-1]);
	context->siglist[context->sp-1] = 0;

	return err;
}

/*
 * Try a single key to see if it works.
 */
static int
eskTryKey (void *arg, PGPByte const *key, PGPSize keylen)
{
	TryCB const *cb = (TryCB *)arg;

	pgpAssert (cb);
	pgpAssert (cb->myself);
	pgpAssert (cb->myself->magic == READANNOTATIONMAGIC);
	pgpAssert (cb->origin);

	return cb->origin->annotate (cb->origin, cb->myself, cb->reply,
				     key, keylen);
}

/*
 * Decrypt an PGPESK from an esklist -- this calls the UI function to
 * decrypt an esklist.  Uses the eskTryKey function to try the keys,
 * and pre-allocates a buffer which is the largest possible PGPESK from
 * all the PgpESKs in the list.  Returns 0 or an error.
 */
static int
eskdecrypt (PGPPipeline *myself, PGPPipeline *origin, int reply)
{
	TryCB cb;
	ReadAnnContext *context;
	PGPESK const *esk;
	PGPByte *buf = 0;
	int i, buflen = 0;
	PGPSize keylen = 0;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	cb.myself = myself;
	cb.origin = origin;
	cb.reply = reply;

	/* Figure out the size of the largest PGPESK */
	for (esk = context->esklist; esk; esk = pgpEskNext(esk)) {
		i = pgpEskMaxKeySize (esk);

		if (buflen < i)
			buflen = i;
	}

	if (buflen) {
		buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
			buflen, kPGPMemoryFlags_Clear);
		if (!buf)
			return kPGPError_OutOfMemory;
	} else
		return kPGPError_BadParams;

	keylen = (size_t)buflen;
	i = context->ui->eskDecrypt (context->ui_arg, context->esklist, buf,
				     &keylen, eskTryKey, &cb);

	pgpClearMemory( buf,  buflen);
	pgpContextMemFree( cdkContext, buf);
	pgpEskFreeList (context->esklist);
	context->esklist = 0;

	if (i) {
		/* Cannot decrypt...  Either eatit or passthrough */
		if (i != PGPANN_PARSER_EATIT && i != PGPANN_PARSER_PASSTHROUGH)
			return i;

		cb.reply = i;	/* What to do? */
		i = eskTryKey (&cb, NULL, 0);
	}

	return i;
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	ReadAnnContext *context;
	PGPError	error = kPGPError_NoErr;
	int i;
	PgpUICbArg msgarg1;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	if (PGP_IS_SCOPE_MARKER (type)) {
		if (PGP_IS_BEGIN_SCOPE (type)) {
			context->stack[context->sp++] = type;
		} else {
			pgpAssert(PGP_IS_END_SCOPE (type));

			if (context->stack[context->sp - 1] != type - 1)
				context->ui->message (context->ui_arg,
						      kPGPError_UnbalancedScope,
						      PGPMSG_UNBALANCED_SCOPE,
						      0);
			else {
				context->stack[--context->sp] = 0;
				context->sepsig[context->sp] = 0;
			}

		}
		/*
		 * For clearsigned messages, preserve type.  See comments
		 * re PGPANN_CLEARSIG_BEGIN/END below.  Otherwise go back
		 * to type of next annotation on the stack.
		 */
		if (!(type==PGPANN_ARMOR_BEGIN &&
					      context->type==PGP_LITERAL_TEXT))
			if (context->sp)
				context->type = context->stack[context->sp-1];
	}

	if (context->ui->annotate) {
		error = (PGPError)context->ui->annotate (context->ui_arg, origin,
					       type, string, size);
		if (error)
			return error;
	}

	switch (type) {
	case PGPANN_COMMIT:
		do {
			i = context->ui->doCommit 
				(context->ui_arg,
				 context->stack[context->sp - 1]);

			/* Allow commit function to abort us */
			if (i < 0) {
				return i;
			}

			if (i != PGPANN_PARSER_PASSTHROUGH &&
			    i != PGPANN_PARSER_EATIT &&
			    i != PGPANN_PARSER_PROCESS &&
			    i != PGPANN_PARSER_RECURSE) {
				msgarg1.type = PGP_UI_ARG_INT;
				msgarg1.val.i = i;
				context->ui->message (context->ui_arg,
						      kPGPError_InvalidCommit,
						      PGPMSG_COMMIT_INVALID,
						      1,
						      &msgarg1);
				continue;
			}

			if (i < PGPANN_PARSER_PROCESS) {
				error = origin->annotate (origin, myself, i,
				                          0, 0);
				if (error) 
					context->ui->message
						(context->ui_arg, error,
						 PGPMSG_ANNOTATE_ERROR, 0);
			} else if (context->stack[context->sp-1] ==
			                          PGPANN_CIPHER_BEGIN) {
				error = (PGPError)eskdecrypt(myself, origin, i);
				if (error) {
					context->ui->message
						(context->ui_arg, error,
						 PGPMSG_ESK_NODECRYPT, 0);
					break;
				}
			} else if (context->stack[context->sp-1] ==
			                          PGPANN_SIGNED_BEGIN) {
				if (context->sepsig[context->sp-1]) {
					error = (PGPError)sepsighash(context);
					if (error) {
						context->ui->message
						   (context->ui_arg, error,
						    PGPMSG_SEPSIG_NOVERIFY, 0);
						break;
					}
				} else {
					error = (PGPError)sighash(myself, origin, i);
					if (error) {
						context->ui->message
						   (context->ui_arg, error,
						    PGPMSG_SIG_NOVERIFY, 0);
						break;
					}
				}
			} else {
				error = origin->annotate (origin, myself, i,
				                          0, 0);
				if (error)
					context->ui->message
						(context->ui_arg, error,
						 PGPMSG_ANNOTATE_ERROR, 0);
			}
		} while (error);
		break;
	case PGPANN_PKCIPHER_ESK:
	case PGPANN_SKCIPHER_ESK:
		i = pgpEskAdd( cdkContext, &context->esklist, type, string, size);
		if (i != 0) {
			context->ui->message (context->ui_arg, i,
					      PGPMSG_ESK_ADD_ERROR, 0);
		}
		break;

	case PGPANN_SIGNED_SIG:
	case PGPANN_SIGNED_SIG2:
		/* Add signature for */
		i = pgpSigAdd ( cdkContext, &context->siglist[context->sp-1], type,
			       string, size);
		if (i != 0) {
			context->ui->message (context->ui_arg, i,
					      PGPMSG_SIG_ADD_ERROR, 0);
		}
		break;

	case PGPANN_SIGNED_SEP:
		/* This signature context is for a separate signature */
		context->sepsig[context->sp-1] = 1;
		break;
	case PGPANN_SIGNED_END:
		error = (PGPError)sigtry(myself, origin);
		break;
	case PGPANN_HASH_VALUE:
		error = (PGPError)sigverify(myself, string, size);
		break;
	case PGPANN_LITERAL_TYPE:
		if (size)
			context->type = string[0];
		break;
	case PGPANN_LITERAL_NAME:
		if (size) {
			if (context->name)
				pgpContextMemFree( cdkContext, context->name);
			context->name = (char *)pgpContextMemAlloc( cdkContext,
				size+1, kPGPMemoryFlags_Clear);
			if (context->name)
			{
				memcpy (context->name, string, size);
				context->name[size] = 0;
			}
			else
				error = kPGPError_OutOfMemory;
		}
		break;
	/*
	 * The actual sequence is: PGPANN_CLEARSIG_BEGIN, PGPANN_CLEARSIG_END,
	 * PGPANN_ARMOR_BEGIN, and then we do a Write when we want our type
	 * to be PGP_LITERAL_TEXT.  So it takes some careful work here and
	 * above.
	 */
	case PGPANN_CLEARSIG_BEGIN:
	case PGPANN_CLEARSIG_END:
		/* Clearsigned messages are always text! */
		context->type = PGP_LITERAL_TEXT;
		break;
	default:
		; /* Do nothing */
	}

	/* At end of scope, shut down output */
	if (!error && PGP_IS_END_SCOPE (type) && context-> sp < context->file_sp)
	{
		/* We are now closing tail during the annotation callback */
		if (context->tail)
			error = context->tail->sizeAdvise (context->tail, 0);
		if (!error) {
			if (context->name) {
				pgpContextMemFree( cdkContext, context->name);
				context->name = NULL;
			}
			context->type = -1;
			context->newoutput = 1;
		}
	}

	return error;
}

static PGPError
Flush (PGPPipeline *myself)
{
	ReadAnnContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	if (!context->tail)
		return( kPGPError_NoErr );

	return context->tail->flush (context->tail);
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	ReadAnnContext *context;
	PGPError	error = kPGPError_NoErr;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	if (!bytes && context->tail) {
		error = context->tail->sizeAdvise (context->tail, bytes);
		if (!error) {
			if (context->name) {
				pgpContextMemFree( cdkContext, context->name);
				context->name = NULL;
			}
			context->type = -1;
			context->newoutput = 1;
		}
	}
	return error;
}

static PGPError
Teardown (PGPPipeline *myself)
{
	ReadAnnContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	context = (ReadAnnContext *)myself->priv;
	pgpAssert (context);

	pgpEskFreeList (context->esklist);
	context->esklist = 0;
	if (context->name)
		pgpContextMemFree( cdkContext, context->name);
	context->name = 0;
	if (context->tail)
		context->tail->teardown (context->tail);

	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpAnnotationReaderCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
			   PGPEnv const *env,
			   PGPUICb const *ui, void *arg)
{
	PGPPipeline *mod;
	ReadAnnContext *context;

	if (!head || !env)
		return NULL;

	*head = 0;

	context = (ReadAnnContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	mod->magic = READANNOTATIONMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Simple Annotation Reader";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->ui = ui;
	context->ui_arg = arg;
	context->env = env;
	context->newoutput = 1;
	context->type = -1;

	*head = mod;
	return &context->tail;
}