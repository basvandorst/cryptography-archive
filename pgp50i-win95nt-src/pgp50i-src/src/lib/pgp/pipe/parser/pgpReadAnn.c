/*
* pgpReadAnn.c -- Annotation Reader
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:  Colin Plumb and Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpReadAnn.c,v 1.5.2.1 1997/06/07 09:51:01 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>

#include "pgpDebug.h"
#include "pgpReadAnn.h"
#include "pgpVrfySig.h"
#include "pgpAnnotate.h"
#include "pgpESK.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpPipeline.h"
#include "pgpSig.h"
#include "pgpTextFilt.h"
#include "pgpUsuals.h"

#define READANNOTATIONMAGIC 0xf00b0b

/* should never have more than this many nested scopes! */
#define MAX_RECURSE 50

struct Context {
          int stack[MAX_RECURSE];
		int sp;
		int file_sp;
		int type;
		char *name;
		struct PgpPipeline *tail;
		struct PgpUICb const *ui;
		struct PgpEnv const *env;
		void *ui_arg;
		struct PgpESK *esklist;
		struct PgpSig *siglist[MAX_RECURSE];
		byte sepsig[MAX_RECURSE];
		/* For the sigverify callback-callback to check the signature */
		struct PgpSig const *cursig;
		/* flag -- do we need a new output? */
		byte newoutput;
	};

struct TryCB {
  struct PgpPipeline *myself, *origin;
  int reply;
};

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
	 	struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == READANNOTATIONMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (error);

if (context->newoutput) {
	 	*error = context->ui->newOutput (context->ui_arg,
	 	     	 	&context->tail,
	 	     	 	context->type,
	 	     	 	context->name);
	 	if (*error)
	 	  return 0;

	 	/* Remember scope to close output */
	 	context->file_sp = context->sp;
	 	context->newoutput = 0;
	  }

	 	pgpAssert (context->tail);
	 	return context->tail->write (context->tail, buf, size, error);
	}

static int
sighash(struct PgpPipeline *myself, struct PgpPipeline *origin, int reply)
	{
	 	struct Context *context;
	 	byte *buf;
	 	int err;
	 	unsigned len;

	 	pgpAssert (myself);
	 	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	 	context = (struct Context *)myself->priv;
	 	pgpAssert (context);

	 	/* Build list of hashes we need */
	 	len = pgpSigDistinctHashCount(context->siglist[context->sp-1]);
	 	buf = (byte *)pgpMemAlloc(len);
	 	if (!buf)
	 	  return PGPERR_NOMEM;
	 	pgpSigDistinctHashes(context->siglist[context->sp-1], buf);

	 	/* Tell the parser to do the hashes */
	 	err = origin->annotate (origin, myself, reply, buf, len);
	 	pgpMemFree(buf);
	 	return err;
	}

/* Try out all the hashes */
/* (Note that this is called with context->sp already decremented) */
static int
sigtry (struct PgpPipeline *myself, struct PgpPipeline *origin)
	{
	 	struct Context *context;
		byte buf[100];
	 	byte const *extra;
	 	unsigned extralen;
	 	struct PgpSig const *sig;
	 	int error;

	 	pgpAssert (myself);
	 	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	 	context = (struct Context *)myself->priv;
	 	pgpAssert (context);

	 	for (sig = context->siglist[context->sp]; sig;
			  sig = pgpSigNext (sig)) {
	 	 	buf[0] = pgpSigHash(sig)->type;
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
sigverify(struct PgpPipeline *myself, byte const *string, size_t size)
	{
	 	struct Context *context;
	 	struct PgpHash const *hash;

(void)size;    /* Avoid warning */
pgpAssert (myself);
pgpAssert (myself->magic == READANNOTATIONMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->cursig);

hash = pgpSigHash (context->cursig);
pgpAssert (hash);
pgpAssert (size == hash->hashsize);

  return context->ui->sigVerify (context->ui_arg, context->cursig,
       string);
}

static int
sepsighash (struct Context *context)
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
eskTryKey (void *arg, byte const *key, size_t keylen)
	{
	 	struct TryCB const *cb = (struct TryCB *)arg;

	 	pgpAssert (cb);
	 	pgpAssert (cb->myself);
	 	pgpAssert (cb->myself->magic == READANNOTATIONMAGIC);
	 	pgpAssert (cb->origin);

return cb->origin->annotate (cb->origin, cb->myself, cb->reply,
      key, keylen);
}

/*
* Decrypt an PgpESK from an esklist -- this calls the UI function to
* decrypt an esklist. Uses the eskTryKey function to try the keys,
* and pre-allocates a buffer which is the largest possible PgpESK from
* all the PgpESKs in the list. Returns 0 or an error.
*/
static int
eskdecrypt (struct PgpPipeline *myself, struct PgpPipeline *origin, int reply)
	{
	 	struct TryCB cb;
	 	struct Context *context;
	 	struct PgpESK const *esk;
	 	byte *buf = 0;
	 	int i, buflen = 0;
	 	size_t keylen = 0;

pgpAssert (myself);
pgpAssert (myself->magic == READANNOTATIONMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

cb.myself = myself;
	 	cb.origin = origin;
	 	cb.reply = reply;

	 	/* Figure out the size of the largest PgpESK */
	 	for (esk = context->esklist; esk; esk = pgpEskNext(esk)) {
	 	  i = pgpEskMaxKeySize (esk);

	 	  if (buflen < i)
	 	    buflen = i;
	 	}

	 	if (buflen) {
	 	 	buf = (byte *)pgpMemAlloc(buflen);
	 	 	if (!buf)
	 	 	  return PGPERR_NOMEM;
	 	} else
			  return PGPERR_ESK_NODECRYPT;

	 	keylen = (size_t)buflen;
	 	i = context->ui->eskDecrypt (context->ui_arg, context->esklist, buf,
	 	      &keylen, eskTryKey, &cb);

	 	memset(buf, 0, buflen);
	 	pgpMemFree(buf);
	 	pgpEskFreeList (context->esklist);
	 	context->esklist = 0;

	 	if (i) {
	 	 	/* Cannot decrypt... Either eatit or passthrough */
	 	 	if (i != PGPANN_PARSER_EATIT && i != PGPANN_PARSER_PASSTHROUGH)
	 	 	  return PGPERR_CB_INVALID;

	 	 	cb.reply = i;  /* What to do? */
	 	 	i = eskTryKey (&cb, NULL, 0);
	 	}

	 	return i;
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	 	byte const *string, size_t size)
	{
		struct Context *context;
	 	int error = 0;
	 	int i;
	 	struct PgpUICbArg msgarg1;

	 	pgpAssert (myself);
	 	pgpAssert (myself->magic == READANNOTATIONMAGIC);

	 	context = (struct Context *)myself->priv;
	 	pgpAssert (context);

	 	if (PGP_IS_SCOPE_MARKER (type)) {
	 	  if (PGP_IS_BEGIN_SCOPE (type)) {
	 	    context->stack[context->sp++] = type;
	 	  } else {
	 	 	 	pgpAssert(PGP_IS_END_SCOPE (type));

	 	 	 	if (context->stack[context->sp - 1] != type - 1)
	 	 	 	 	context->ui->message (context->ui_arg,
	 	 	 	 	    PGPERR_UNBALANCED_SCOPE,
	 	 	 	 	    PGPMSG_UNBALANCED_SCOPE,
	 	 	 	 	    0);
	 	 	 	else {
	 	 	 	 	context->stack[--context->sp] = 0;
	 	 	 	 	context->sepsig[context->sp] = 0;
	 	 	 	}

	 	 	}
	 		/*
	 	 	* For clearsigned messages, preserve type. See comments
	 	 	* re PGPANN_CLEARSIG_BEGIN/END below. Otherwise go back
	 	 	* to type of next annotation on the stack.
	 	 	*/
	 	 	if (!(type==PGPANN_ARMOR_BEGIN &&
	 		      context->type==PGP_LITERAL_TEXT))
	 	 	  if (context->sp)
	 	 	    context->type = context->stack[context->sp-1];
	 	}

	 	if (context->ui->annotate) {
	 	 	error = context->ui->annotate (context->ui_arg, origin,
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

	 	 	 	if (i != PGPANN_PARSER_PASSTHROUGH &&
	 	 	 	i != PGPANN_PARSER_EATIT &&
	 	 	 	i != PGPANN_PARSER_PROCESS &&
	 	 	 	i != PGPANN_PARSER_RECURSE) {
	 	 	 	 	msgarg1.type = PGP_UI_ARG_INT;
	 	 	 	 	msgarg1.val.i = i;
	 	 	 	 	context->ui->message (context->ui_arg,
	 	 	 	 	   	PGPERR_COMMIT_INVALID,
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
	 	 	error = eskdecrypt(myself, origin, i);
	 	 	if (error) {
	 	 	 	context->ui->message
	 	 	 	  (context->ui_arg, error,
	 	 	 	  PGPMSG_ESK_NODECRYPT, 0);
	 	 	 	break;
	 	 	}
	 	} else if (context->stack[context->sp-1] ==
	 	PGPANN_SIGNED_BEGIN) {
		  if (context->sepsig[context->sp-1]) {
		   	error = sepsighash(context);
		   	if (error) {
		   	 	context->ui->message
		   	 	(context->ui_arg, error,
		   	 	PGPMSG_SEPSIG_NOVERIFY, 0);
		   	 	break;
		   	}
		  } else {
		 	error = sighash(myself, origin, i);
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
		  i = pgpEskAdd(&context->esklist, type, string, size);
		  if (i != 0) {
		    context->ui->message (context->ui_arg, i,
		       				PGPMSG_ESK_ADD_ERROR, 0);
		  }
		  break;

case PGPANN_SIGNED_SIG:
case PGPANN_SIGNED_SIG2:
		  /* Add signature for */
		  i = pgpSigAdd (&context->siglist[context->sp-1], type,
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
	 	  error = sigtry(myself, origin);
		  break;
		case PGPANN_HASH_VALUE:
	 	  error = sigverify(myself, string, size);
	 	  break;
	 	case PGPANN_LITERAL_TYPE:
	 	  if (size)
	 	    context->type = string[0];
	 	  break;
		case PGPANN_LITERAL_NAME:
		 	if (size) {
	 	 	 	if (context->name)
	 	 	 	  pgpMemFree (context->name);
	 	 	 	context->name = (char *)pgpMemAlloc (size+1);
	 	 	 	if (context->name) {
	 	 	 	 	memcpy (context->name, string, size);
	 	 	 	 	context->name[size] = 0;
		 	 	} else
		 	 	 	error = PGPERR_NOMEM;
		 	}
		 	break;
	 	/*
	 	* The actual sequence is: PGPANN_CLEARSIG_BEGIN, PGPANN_CLEARSIG_END,
	 	* PGPANN_ARMOR_BEGIN, and then we do a Write when we want our type
		* to be PGP_LITERAL_TEXT. So it takes some careful work here and
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
	 	if (!error && context->tail && PGP_IS_END_SCOPE (type) &&
	 	context-> sp < context->file_sp)
	 	{
	 	 	error = context->tail->sizeAdvise (context->tail, 0);
	 	 	if (!error) {
	 	 	 	if (context->name) {
	 	 	 	  pgpMemFree (context->name);
	 	 	 	  context->name = NULL;
	 	 	 	}
	 	 	 	context->type = -1;
	 	 	 	context->newoutput = 1;
	 	 	}
	 	}

	 	return error;
}

static int
Flush (struct PgpPipeline *myself)
{
struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == READANNOTATIONMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

if (!context->tail)
  return 0;

return context->tail->flush (context->tail);
}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
	 	struct Context *context;
		int error = 0;

pgpAssert (myself);
pgpAssert (myself->magic == READANNOTATIONMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

if (!bytes && context->tail) {
	 	error = context->tail->sizeAdvise (context->tail, bytes);
	 	if (!error) {
	 	 	if (context->name) {
	 	 	  pgpMemFree (context->name);
	 	 	  context->name = NULL;
		 	}
		 	context->type = -1;
		 	context->newoutput = 1;
		}
	}
return error;
}

static void
Teardown (struct PgpPipeline *myself)
	{
	 	struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == READANNOTATIONMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

	 	pgpEskFreeList (context->esklist);
		context->esklist = 0;
	 	if (context->name)
	 	  pgpMemFree (context->name);
	 	context->name = 0;
	 	if (context->tail)
	 	  context->tail->teardown (context->tail);

	 	memset (context, 0, sizeof (*context));
	 	pgpMemFree (context);
	 	memset (myself, 0, sizeof (*myself));
	 	pgpMemFree (myself);
	}

struct PgpPipeline **
pgpAnnotationReaderCreate (struct PgpPipeline **head,
	 	    struct PgpEnv const *env,
	 	    struct PgpUICb const *ui, void *arg)
	{
	 	struct PgpPipeline *mod;
	 	struct Context *context;

	 	if (!head || !env)
	 	  return NULL;

		*head = 0;

		context = (struct Context *)pgpMemAlloc (sizeof (*context));
		if (!context)
		  return NULL;
		mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
		if (!mod) {
		  pgpMemFree (context);
		  return NULL;
		}

		mod->magic = READANNOTATIONMAGIC;
		mod->write = Write;
		mod->flush = Flush;
		mod->sizeAdvise = SizeAdvise;
		mod->annotate = Annotate;
		mod->teardown = Teardown;
		mod->name = "Simple Annotation Reader";
		mod->priv = context;

		memset (context, 0, sizeof (*context));
		context->ui = ui;
		context->ui_arg = arg;
		context->env = env;
		context->newoutput = 1;
		context->type = -1;

		*head = mod;
		return &context->tail;
	}
