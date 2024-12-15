/*
* pgpSigMod.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpSigMod.c,v 1.5.2.1 1997/06/07 09:51:04 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpCopyMod.h"
#include "pgpMakeSig.h"
#include "pgpPktByte.h"
#include "pgpSigMod.h"
#include "pgpAnnotate.h"
#include "pgpHash.h"
#include "pgpHashMod.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErr.h"
#include "pgpSigSpec.h"
#include "pgpUsuals.h"

#define SIGMODMAGIC 0x519a10fe

#define MAX_EXTRA 5

struct PerSignature {
 struct PerSignature *next;
 struct PgpSigSpec const *spec;
 struct PgpHashContext *hc;
};

struct Context {
		struct PgpPipeline *tail;
		struct PgpPipeline *join, *joinlast, *newjoin;
		struct PgpPipeline **jointail;
		struct PgpPipeline **texttail;
		struct PerSignature *siglist;
		struct PerSignature *sigptr;
		struct PgpRandomContext const *rc;
		PgpVersion version;
		byte *buf;
		unsigned buflen;
		byte const *sig;
		int siglen;
		int scope_depth;
		byte onepass;
	};

/*
* For each signature on the signature list, get rid of it.
* The fact that duplicate PgpHashContexts are allowed complicates
* this a little bit.
*/

static void
TeardownSigList(struct PerSignature *siglist)
	{
		struct PerSignature *ps;
		struct PgpHashContext *hc;

		while (siglist) {
			hc = siglist->hc;
			if (hc) {
				/* Walk to end of list deleting duplicates */
				for (ps = siglist->next; ps; ps = ps->next)
				 if (ps->hc == hc)
				  ps->hc = 0;
				pgpHashDestroy(hc);
			}
			ps = siglist->next;
			memset(siglist, 0, sizeof(*siglist));
			pgpMemFree(siglist);
			siglist = ps;
		}
	}

static size_t
Write(struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
		struct Context *context;

		pgpAssert(myself);
		pgpAssert(myself->magic == SIGMODMAGIC);
		pgpAssert(error);

		context = (struct Context *)myself->priv;
		pgpAssert(context);
		pgpAssert(context->tail);

		/*
		* If we're doing 1-pass signatures we might need to open a
		* new input (for the real signature data) which is appended
		* to the current data using pgpSigTextInsert, just like the
		* data did. Then we need to close the current input, tear it
		* down, and replace it with the new one. Finally, we need to
		* set a flag that says that we did this, so we don't do it
		* again. */

		if (context->version > PGPVERSION_2_6 &&
		context->joinlast != context->join) {
			/*
			* Do the switch if we have new version packets and
			* pgpSigTextInsert() was called
			*/
			struct PgpPipeline *head = NULL, **tail;

			switch (context->onepass) {
			case 0:
			 /* Create the new module... */
			 tail = pgpSigTextInsert (myself, &head);
				if (!tail) {
				 *error = PGPERR_NOMEM;
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

static int
Annotate(struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		struct Context *context;
		int error;

pgpAssert(myself);
pgpAssert(myself->magic == SIGMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert(context);
pgpAssert(context->tail);

		error = context->tail->annotate(context->tail, origin, type,
		string, size);
		if (!error)
		 PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
		pgpAssert(context->scope_depth != -1);

		return error;
	}

static int
SizeAdvise(struct PgpPipeline *myself, unsigned long bytes)
{
 struct Context *context;
 struct PerSignature *ps;
 int i, len, error;
size_t size;

pgpAssert(myself);
pgpAssert(myself->magic == SIGMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert(context);
pgpAssert(context->tail);

		i = context->tail->sizeAdvise(context->tail, bytes);
		if (i || bytes || context->scope_depth)
		 return i;

		/*
		* Not inside any scope and got a sizeAdvise(0): EOF!
		* So write out all the signatures. */

		/* Find out how big a buffer we need and allocate it */
		if (context->sigptr && !context->buf) {
			len = 0;
			for (ps = context->sigptr; ps; ps = ps->next) {
			 i = pgpMakeSigMaxSize(ps->spec);
			 if (len < i)
			  len = i;
			}

			context->buf = (byte *)pgpMemAlloc((unsigned)len+3);
			if (!context->buf)
			 return PGPERR_NOMEM;
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
			 return i;

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
					context->buf[2] = (byte)i;
					context->sig = context->buf+1;
					context->siglen = (unsigned)i+2;
				} else {}
	#endif
				context->buf[0] = PKTBYTE_BUILD(PKTBYTE_SIG, 1);
				context->buf[1] = (byte)(i>>8);
				context->buf[2] = (byte)i;
				context->sig = context->buf;
				context->siglen = (unsigned)i+3;
			}
			/* Okay, signature made */
			context->sigptr = ps = ps->next;
		}

		TeardownSigList(context->siglist);
		context->siglist = 0;

		if (context->buf) {
		 memset(context->buf, 0, context->buflen);
		 pgpMemFree(context->buf);
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

		return 0;
}


static int
Flush(struct PgpPipeline *myself)
	{
		struct Context *context;
		int error;

		pgpAssert(myself);
		pgpAssert(myself->magic == SIGMODMAGIC);

		context = (struct Context *)myself->priv;
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
		return 0;
	}

static void
Teardown(struct PgpPipeline *myself)
	{
		struct Context *context;

		pgpAssert(myself);
		pgpAssert(myself->magic == SIGMODMAGIC);

		context = (struct Context *)myself->priv;
		pgpAssert(context);
		if (context->tail)
		 context->tail->teardown(context->tail);
		if (context->join)
		 context->join->teardown(context->join);
		TeardownSigList(context->siglist);
		if (context->buf) {
		 memset(context->buf, 0, context->buflen);
		 pgpMemFree(context->buf);
		}

		memset(context, 0, sizeof(*context));
		pgpMemFree(context);
		memset(myself, 0, sizeof(*myself));
		pgpMemFree(myself);
	}

struct PgpPipeline **
pgpSigCreate(struct PgpPipeline **head, PgpVersion version,
 struct PgpFifoDesc const *fd, struct PgpRandomContext const *rc)
{
 struct PgpPipeline *mod;
		struct PgpPipeline **tail;
		struct Context *context;

		if (!head)
		  return NULL;

		context = (struct Context *)pgpMemAlloc(sizeof(*context));
		if (!context)
		 return NULL;
		mod = (struct PgpPipeline *)pgpMemAlloc(sizeof(*mod));
		if (!mod) {
		 pgpMemFree(context);
		 return NULL;
		}

		/* Create the join module for stuffing the signatures into. */
		memset(context, 0, sizeof(*context));
		context->jointail = pgpJoinCreate(&context->join, fd);
		if (!context->jointail) {
		 pgpMemFree(mod);
		 pgpMemFree(context);
		 return NULL;
		}
		context->joinlast = context->join;
		context->rc = rc;
		context->version = version;

		/* Tack a copy mod onto the output to provide a stable tail pointer. */
		context->tail = *head;
		tail = pgpCopyModCreate(&context->tail);
		if (!tail) {
		 context->join->teardown(context->join);
		 pgpMemFree(mod);
		 pgpMemFree(context);
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

		*head = mod;
		return tail;
}

/*
* This adds one signature to the signing module. It returns a tail pointer
* which the signatures will come out of. Each call returns the same
* tail pointer - this joins them together internally.
*/
static struct PgpPipeline **
addSignature (struct PgpPipeline *myself, struct PgpSigSpec *spec,
 byte clearsig, byte nest)
{
struct Context *context = (struct Context *)myself->priv;
struct PerSignature *ps, *ps2;
struct PgpHash const *hash = pgpSigSpecHash(spec);
struct PgpHashContext *hc;

pgpAssert(myself);
pgpAssert(myself->magic == SIGMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert(context);

ps = (struct PerSignature *)pgpMemAlloc(sizeof(*ps));

/* If we already have a request for this hash, use it */
hc = 0;
for (ps2 = context->siglist; ps2; ps2 = ps2->next) {
		if (ps2->hc->hash == hash) {
		 hc = ps2->hc;
		 break;
		}
	}

/* If not, generate a new one, and a module to make it. */
if (!hc) {
		hc = pgpHashCreate(hash);
		if (!hc) {
		 pgpMemFree(ps);
		 return 0;
		}
		if (!pgpHashModCreate(&context->tail, hc)) {
			pgpHashDestroy(hc);
			pgpMemFree(ps);
			return 0;
		}
	}

/* Fill in the PerSignature structure */
ps->spec = spec;
ps->hc = hc;

		if (!clearsig && context->version > PGPVERSION_2_6) {
	 		/*
	 		* Add the 1-pass signature header here. It gets
	 		* buffered in the order the signatures come in, in a
	 		* LIFO stack. For clearsigs we don't do this, there is
	 		* just the sig at the end. The hash info goes in the
			* clear.
	 		*/
	 		byte *buf;
	 		int len;

	 		len = pgpMakeSigHeaderMaxSize (spec);
	 		pgpAssert (PKTLEN_ONE_BYTE(len));
	 		buf = (byte *)pgpMemAlloc (len+2);
			if (!buf)
	 		 return NULL;
	 		len = pgpMakeSigHeader (buf+2, spec, nest);
			buf[0] = PKTBYTE_BUILD_NEW (PKTBYTE_1PASSSIG);
			buf[1] = PKTLEN_1BYTE (len);

			len -= pgpJoinBuffer (context->join, buf, len+2);
			memset (buf, 0, sizeof (buf));
			pgpMemFree (buf);
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
	 * Set up to create a(nother) signature. Clearsig tells us not to try to
	 * use the 1-pass sigs.
	*/
struct PgpPipeline **
pgpSigSignature(struct PgpPipeline *myself, struct PgpSigSpec *spec,
		 byte clearsig)
	{
		struct Context *context;
		struct PgpSigSpec *ss;
		struct PgpPipeline **tail;

		pgpAssert(myself);
		pgpAssert(myself->magic == SIGMODMAGIC);

		/* Just checking ! */
		context = (struct Context *)myself->priv;
		pgpAssert(context);

		/* Only one signature on a PGPVERSION_2_6 packet */
		if (context->version <= PGPVERSION_2_6)
		 return addSignature (myself, spec, clearsig, 0);

		/* PGPVERSION_3 packets can have multiple signatures */
		tail = NULL;
		for (ss = spec; ss; ss = pgpSigSpecNext (ss)) {
			tail = addSignature (myself, ss, clearsig,
			 	((pgpSigSpecNext (ss))?0:1));
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
struct PgpPipeline **
pgpSigTextInsert(struct PgpPipeline *myself, struct PgpPipeline **head)
	{
		struct Context *context;
		struct PgpPipeline *newjoin;

		pgpAssert(myself);
		pgpAssert(myself->magic == SIGMODMAGIC);

		context = (struct Context *)myself->priv;
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
