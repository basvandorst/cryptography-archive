/*
 * $Id: pgpSigMod.c,v 1.26 1997/10/08 02:10:50 lloyd Exp $
 */

#include "pgpConfig.h"
#include <string.h>

#include "pgpDebug.h"
#include "pgpCopyMod.h"
#include "pgpMakeSig.h"
#include "pgpPktByte.h"
#include "pgpSigMod.h"
#include "pgpAnnotate.h"
#include "pgpHashPriv.h"
#include "pgpHashMod.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErrors.h"
#include "pgpSigSpec.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#define SIGMODMAGIC 0x519a10fe


typedef struct PerSignature {
	struct PerSignature *next;
	PGPSigSpec const *spec;
	PGPHashContext *hc;
	DEBUG_STRUCT_CONSTRUCTOR( PerSignature )
} PerSignature;

typedef struct SigModContext {
	PGPPipeline	pipe;
	
	PGPPipeline *tail;
	PGPPipeline *join, *joinlast, *newjoin;
	PGPPipeline **jointail;
	PGPPipeline **texttail;
	PerSignature *siglist;
	PerSignature *sigptr;
	PGPRandomContext const *rc;
	PgpVersion version;
	PGPByte *buf;
	unsigned buflen;
	PGPByte const *sig;
	int siglen;
	int scope_depth;
	PGPByte onepass;
	DEBUG_STRUCT_CONSTRUCTOR( SigModContext )
} SigModContext;

/*
 * For each signature on the signature list, get rid of it.
 * The fact that duplicate PgpHashContexts are allowed complicates
 * this a little bit.
 */

static PGPError
TeardownSigList(
	PGPContextRef	cdkContext,
	PerSignature *siglist)
{
	PerSignature *ps;
	PGPHashContext *hc;

	while (siglist) {
		hc = siglist->hc;
		if (hc) {
			/* Walk to end of list deleting duplicates */
			for (ps = siglist->next; ps; ps = ps->next)
				if (ps->hc == hc)
					ps->hc = 0;
			PGPFreeHashContext(hc);
		}
		ps = siglist->next;
		pgpClearMemory( siglist,  sizeof(*siglist));
		pgpContextMemFree( cdkContext, siglist);
		siglist = ps;
	}
	
	return kPGPError_NoErr;
}

static size_t
Write(PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	SigModContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);
	pgpAssert(error);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	/*
	 * If we're doing 1-pass signatures we might need to open a
	 * new input (for the real signature data) which is appended
	 * to the current data using pgpSigTextInsert, just like the
	 * data did.  Then we need to close the current input, tear it
	 * down, and replace it with the new one.  Finally, we need to
	 * set a flag that says that we did this, so we don't do it
	 * again.  */

	if (context->version > PGPVERSION_2_6 && 
	    context->joinlast != context->join) {
		/*
		 * Do the switch if we have new version packets and
		 * pgpSigTextInsert() was called
		 */
		PGPPipeline *head = NULL, **tail;

		switch (context->onepass) {
		case 0:
			/* Create the new module... */
			tail = pgpSigTextInsert (myself, &head);
			if (!tail) {
				*error = kPGPError_OutOfMemory;
				return 0;
			}
			context->newjoin = head;
			context->onepass++;
			/* FALLTHROUGH */
		case 1:
			/* close the old one... */
			*error = context->join->sizeAdvise (context->join, 0);
			if (*error)
				return 0;
			context->onepass++;
			/* FALLTHROUGH */
		case 2:
			/* ...and shut it down... */
			context->join->teardown (context->join);

			/* inserting the new one in its place */
			context->join = context->newjoin;
			context->onepass++;
		default:
			break;
		}
	}

	return context->tail->write(context->tail, buf, size, error);
}

static PGPError
Annotate(PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	SigModContext *context;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	error = context->tail->annotate(context->tail, origin, type,
	                                string, size);
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return error;
}

static PGPError
SizeAdvise(PGPPipeline *myself, unsigned long bytes)
{
	SigModContext *context;
	PerSignature *ps;
	PGPError	error;
	int			i, len;
	size_t size;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	i = context->tail->sizeAdvise(context->tail, bytes);
	if (i || bytes || context->scope_depth)
		return (PGPError)i;

	/*
	 * Not inside any scope and got a sizeAdvise(0): EOF!
	 * So write out all the signatures.  */

	/* Find out how big a buffer we need and allocate it */
	if (context->sigptr && !context->buf) {
		len = 0;
		for (ps = context->sigptr; ps; ps = ps->next) {
			i = pgpMakeSigMaxSize(ps->spec);
			if (len < i)
				len = i;
		}

		context->buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
			(unsigned)len+3, kPGPMemoryFlags_Clear);
		if (!context->buf)
			return kPGPError_OutOfMemory;
		context->buflen = (unsigned)len+3;
	}

	/* Walk the list of signatures writing them out. */
	ps = context->sigptr;
	while (ps || context->siglen) {
		/* Write any partial signature */
		if (context->siglen) {
			pgpAssert(context->sig);
			pgpAssert(context->join);
			/* Dump it down the join module */
			size = context->join->write(context->join,
						    context->sig,
						    context->siglen,
						    &error);
			context->sig += size;
			context->siglen -= size;
			if (error)
				return error;
			continue;
		}

		/* Okay, now, make a new signature. */
		pgpAssert(context->buflen >= 3u+pgpMakeSigMaxSize(ps->spec));
		i = pgpMakeSig(context->buf+3, ps->spec, context->rc, ps->hc);
		if (i < 0)
			return (PGPError)i;

		if (context->version > PGPVERSION_2_6) {
			if (PKTLEN_ONE_BYTE(i)) {
				context->buf[1] =
					PKTBYTE_BUILD_NEW(PKTBYTE_SIG);
				context->buf[2] = PKTLEN_1BYTE(i);
				context->sig = context->buf+1;
				context->siglen = (unsigned)i+2;
			} else {
				context->buf[0] =
					PKTBYTE_BUILD_NEW(PKTBYTE_SIG);
				context->buf[1] = PKTLEN_BYTE0(i);
				context->buf[2] = PKTLEN_BYTE1(i);
				context->sig = context->buf;
				context->siglen = (unsigned)i+3;
			}
		} else {
#if 0
/* PGP 2.6.2 screws up on this */
			if (i < 256) {
				context->buf[1] =
					PKTBYTE_BUILD(PKTBYTE_SIG, 0);
				context->buf[2] = (PGPByte)i;
				context->sig = context->buf+1;
				context->siglen = (unsigned)i+2;
			} else {}
#endif
			context->buf[0] = PKTBYTE_BUILD(PKTBYTE_SIG, 1);
			context->buf[1] = (PGPByte)(i>>8);
			context->buf[2] = (PGPByte)i;
			context->sig = context->buf;
			context->siglen = (unsigned)i+3;
		}
		/* Okay, signature made */
		context->sigptr = ps = ps->next;
	}

	TeardownSigList( cdkContext, context->siglist);
	context->siglist = 0;

	if (context->buf) {
		pgpClearMemory(context->buf, context->buflen);
		pgpContextMemFree( cdkContext, context->buf);
		context->buf = 0;
	}

	if (context->join) {
		error = context->join->sizeAdvise(context->join, 0);
		if (error)
			return error;
#if 0
/* Take this out; simple pgp lib later queries downstream memmod */
		context->join->teardown(context->join);
		context->join = 0;
#endif
	}

	return kPGPError_NoErr;
}


static PGPError
Flush(PGPPipeline *myself)
{
	SigModContext *context;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);

	/* XXX Do nothing */
	if (context->tail) {
		error = context->tail->flush(context->tail);
		if (error)
			return error;
	}
	if (context->join) {
		error = context->join->flush(context->join);
		if (error)
			return error;
	}
	return kPGPError_NoErr;
}

static PGPError
Teardown(PGPPipeline *myself)
{
	SigModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);
	if (context->tail)
		context->tail->teardown(context->tail);
	if (context->join)
		context->join->teardown(context->join);
	TeardownSigList( cdkContext, context->siglist);
	if (context->buf) {
		pgpClearMemory(context->buf, context->buflen);
		pgpContextMemFree( cdkContext, context->buf);
	}

	pgpClearMemory( context,  sizeof(*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpSigCreate(
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PgpVersion version,
	PGPFifoDesc const *fd,
	PGPRandomContext const *rc)
{
	PGPPipeline *mod;
	PGPPipeline **tail;
	SigModContext *context;

	if (!head)
		return NULL;

	context = (SigModContext *)pgpContextMemAlloc( cdkContext,
		sizeof(*context), kPGPMemoryFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	/* Create the join module for stuffing the signatures into. */
	context->jointail = pgpJoinCreate( cdkContext, &context->join, fd);
	if (!context->jointail) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	context->joinlast = context->join;
	context->rc = rc;
	context->version = version;

	/* Tack a copy mod onto the output to provide a stable tail pointer. */
	context->tail = *head;
	tail = pgpCopyModCreate( cdkContext, &context->tail);
	if (!tail) {
		context->join->teardown(context->join);
		pgpContextMemFree( cdkContext, context);
	}

	/* Finally, fill in all the values */
	mod->magic = SIGMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Signature";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	*head = mod;
	return tail;
}

/*
 * This adds one signature to the signing module.  It returns a tail pointer
 * which the signatures will come out of.  Each call returns the same
 * tail pointer - this joins them together internally.
 */
static PGPPipeline **
addSignature (PGPPipeline *myself, PGPSigSpec *spec,
	      PGPByte clearsig, PGPByte nest)
{
	SigModContext *context = (SigModContext *)myself->priv;
	PerSignature *ps, *ps2;
	PGPHashVTBL const *hash = pgpSigSpecHash(spec);
	PGPHashContext *hc;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);

	ps = (PerSignature *)pgpContextMemAlloc( cdkContext,
		sizeof(*ps), kPGPMemoryFlags_Clear);
	if ( IsNull( ps ) )
	{
		return( NULL );
	}
	
	/* If we already have a request for this hash, use it */
	hc = 0;
	for (ps2 = context->siglist; ps2; ps2 = ps2->next) {
		if ( pgpHashGetVTBL( ps2->hc ) == hash) {
			hc = ps2->hc;
			break;
		}
	}

	/* If not, generate a new one, and a module to make it. */
	if (!hc) {
		hc = pgpHashCreate(myself->cdkContext, hash);
		if (!hc) {
			pgpContextMemFree( cdkContext, ps);
			return 0;
		}
		if (!pgpHashModCreate( myself->cdkContext, &context->tail, hc)) {
			PGPFreeHashContext(hc);
			pgpContextMemFree( cdkContext, ps);
			return 0;
		}
	}

	/* Fill in the PerSignature structure */
	ps->spec = spec;
	ps->hc = hc;

	if (!clearsig && context->version > PGPVERSION_2_6) {
		/*
		 * Add the 1-pass signature header here.  It gets
		 * buffered in the order the signatures come in, in a
		 * LIFO stack.  For clearsigs we don't do this, there is
		 * just the sig at the end.  The hash info goes in the
		 * clear.
		 */
		PGPByte *buf;
		int len;
		
		len = pgpMakeSigHeaderMaxSize (spec);
		pgpAssert (PKTLEN_ONE_BYTE(len));
		buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
			len+2, kPGPMemoryFlags_Clear);
		if (!buf)
			return NULL;
		len = pgpMakeSigHeader (buf+2, spec, nest);
		buf[0] = PKTBYTE_BUILD_NEW (PKTBYTE_1PASSSIG);
		buf[1] = PKTLEN_1BYTE (len);
		
		len -= pgpJoinBuffer (context->join, buf, len+2);
		pgpClearMemory( buf,  sizeof (buf));
		pgpContextMemFree( cdkContext, buf);
		if (len != -2)
			return NULL;
	}

	/* Add to the beginning of the list, reversing the stack */
	ps->next = context->siglist;
	context->siglist = ps;
	context->sigptr = context->siglist;

	/* And we're done... */
	return context->jointail;
}

/*
 * Set up to create a(nother) signature.  Clearsig tells us not to try to
 * use the 1-pass sigs.
 */
PGPPipeline **
pgpSigSignature(PGPPipeline *myself, PGPSigSpec *spec,
		PGPByte clearsig)
{
	SigModContext *context;
	PGPSigSpec *ss;
	PGPPipeline **tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	/* Just checking ! */
	context = (SigModContext *)myself->priv;
	pgpAssert(context);

	/* Only one signature on a PGPVERSION_2_6 packet */
	if (context->version <= PGPVERSION_2_6)
		return addSignature (myself, spec, clearsig, 0);

	/* PGPVERSION_3 packets can have multiple signatures */
	tail = NULL;
       	for (ss = spec; ss; ss = pgpSigSpecNext (ss)) {
		tail = addSignature (myself, ss, clearsig,
				     (PGPBoolean)((pgpSigSpecNext (ss))?0:1));
		if (!tail)
			return tail;
	}
	return tail;
}

/*
 * This inserts some text into the signature output in the appropriate place
 * to make a signed message, as opposed to separate signatures.
 * It is possible that you might take the output of the signature from
 * pgpSigCreate, apply some processing (such as compression, or encapsulation
 * in a literal which PGP does right now) and feed it back into here.
 * (Should we hide *that* processing, too?)
 */
PGPPipeline **
pgpSigTextInsert(PGPPipeline *myself, PGPPipeline **head)
{
	SigModContext *context;
	PGPPipeline *newjoin;

	pgpAssert(myself);
	pgpAssert(myself->magic == SIGMODMAGIC);

	context = (SigModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->join);
	pgpAssert(context->joinlast);

	if (!head)
		return NULL;

	newjoin = pgpJoinAppend(context->joinlast);
	if (newjoin) {
		*head = context->joinlast = newjoin;
		return context->jointail;
	} else {
		return 0;
	}
}
