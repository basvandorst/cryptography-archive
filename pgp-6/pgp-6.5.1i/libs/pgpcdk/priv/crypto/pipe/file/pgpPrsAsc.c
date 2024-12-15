/*
 * pgpPrsAsc.c -- ascii dearmor parser.  This is fairly complicated.  Read
 * 	the comment a page down for a description of how this system works.
 *	The nice part is that you can input multipart armor in any order
 *	and it will work.  In fact, you can intersperse armor parts from
 *	multiple messages and it will work.  Moreover, you can input binary
 *	PGP messages and it will still work!  My god, can this man think
 *	of everything or what?
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsAsc.c,v 1.50.6.1 1999/06/03 23:26:00 heller Exp $
 */

#include "pgpConfig.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifndef MIMEPARSE
#define MIMEPARSE		1
#endif

#include "pgpDebug.h"
#include "pgpCharMap.h"		/* for charMapIdentity */
#include "pgpCopyMod.h"
#include "pgpCRC.h"
#include "pgpPrsAsc.h"
#include "pgpRadix64.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpFileType.h"
#include "pgpHashPriv.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpPrsBin.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpSplit.h"
#include "pgpTextFilt.h"
#include "pgpUsuals.h"
#include "pgpVerifyRa.h"
#include "pgpContext.h"

/*
 * This is a complicated piece of code.  Less politely, this code is a
 * bitch.  It needs to be able to keep track of many different things
 * at the same time in order to combine pieces of armor into whole messages.
 *
 * There are three entities defined here.  First is an armor part, which
 * is defined as the range from BEGIN PGP MESSAGE to the END.  Second is
 * a message, which is PGP message made up of one or more parts.
 * And third, there are files, which can contain parts.
 *
 * Parts must be fully contained within a file, however there can be
 * multiple parts in a single file.  On the other hand, messages can
 * cross file boundaries.
 *
 * This code will output PGP messages in a stream of input "files"
 * wherever the first part of the message came in the stream.  In a
 * multipart message, this will require the least amount of buffering
 * since buffering is only required when pieces come out of order.
 * This can also handle the degenerate cases of interleaved messages,
 * although that requires possibly large amounts of buffering.
 * That case happens rarely enough that the resource requirement is
 * not onsidered a problem.
 *
 * A picture best describes this behavior:
 *
 *  W1          W2   X1     X2      X3   Y1      Y2      Y3   Z1    Z2
 * |  |   A2   |  | |  | B |  | A1 |  | |  | C2 |  | C1 |  | |  |A3|  |
 * |  +--------+  | |  +---+  +----+  | |  +----+  +----+  | |  +--+  |
 * +--------------+ +-----------------+ +------------------+ +--------+
 *     File 1              File 2              File 3          File 4
 *                                 ^                            ^
 *                                 |   Everything is buffered   |
 *                                 +----------------------------+
 *
 * W,X,Y,Z are file header and footer text which is outside the PGP armor.
 * The numbers are used to associate where in the data stream they occur.
 *
 * A,B,C are 3 PGP messages.  The numbers are the actual multipart parts
 * which may occur out of order.
 *
 * If this input stream were given to this armor parser, the following
 * output would occur:
 *
 *  | W1 ~ W2 | X1 B X2 A X3 | Y1 ~ Y2 C Y3 | Z1 ~ Z2 |
 *
 * Where:	| == A file separation annotation (or multiple annotations)
 *			~ == An armor annotation (an armor part occured here)
 *			W,X,Y,Z == The exact unencrypted text that arrived in the
 *			           input.
 *			A,B,C == The full messages, wrapped in "PGP data" annotations,
 *			         which will be sent to a binary parser for decryption.
 *
 * As you can see, the messages are output in the order B, A, C, each
 * message occurring in the place in the unenecrpted text stream where
 * part 1 is located.  Other parts are replaced with an annotation that just
 * notes the deletion.
 *
 * While decrypting message A, all the data between A1 and A3 (including
 * annotations) needs to be buffered.  In the usual case where the text
 * between parts is short (e-mail headers, mostly) and there aren't
 * multiple interleaved parts, not very much needs to be buffered.
 * Not much can be done to improve the worst case.
 */

#define kPGPPrsAscCmdBufSize	4096

#define DEARMORMAGIC	0xdea4304

/* List of commands */
#define CMD_WRITE	1
#define CMD_ANNOTATE	2

#define LINE_LEN	256	/* 256 bytes is MORE than Ascii armorsize! */

/* Some explanations of the various pipeline pointers (as usual, the
 * "tail" pointers are PGPPipeline **, that is, pointers to the "next"
 * pointer in the relevant object):
 *
 * context->tail	Points at next stage set up by our caller, usually
 *					an annotation reader.  Downstream of msg->tail.
 * context->myself	Points at this very pipeline module.
 * msg->tail		Points at a copy module which precedes context->tail.
 *					Part of downstream pipeline of part->mod.
 * part->mod		Points at the 1st-nth stage of a join input, which leads
 *					to a binary parser, 1 stage for each part.  Now NULL for
 *					clearsigned messages.
 * part->text		Used for clearsigned text, points at a split which
 *					feeds into both the sig verifier (via a textfilt) and
 *					into the main msg->tail output (anno reader).  (Used to
 *					point to 2nd stage of join input, where part->mod pointed
 *					at first stage.  This caused unnecessary buffering.)
 *					and we pass the raw data to context->tail.
 * part->sig		Used for clearsigned signatures, points at a binary
 *					parser that feeds into a sig verifier.
 */

typedef struct Message	Message;

typedef struct MsgPart	MsgPart;
struct MsgPart {
	PGPContextRef	cdkContext;
	
	unsigned num;		/* Which part number is this? */
	PGPPipeline *mod; /* Pointer to the join module for this part */
	PGPPipeline *text; /* clearsigned: the text goes here */
	PGPPipeline *sig; /* clearsigned: the sig goes here */
	Message *msg;	/* What message does this belong to? */
	MsgPart *next;	/* Pointer to the next part */
	PGPByte first;		/* Is this the first section left to do? */
	PGPByte done;		/* Is this part complete? */
	DEBUG_STRUCT_CONSTRUCTOR( MsgPart )
};

struct Message {
	PGPContextRef	cdkContext;
	
	char *name;		/* The name of this message */
	unsigned size;		/* How many parts does this have, if known */
	PGPPipeline **tail; /* Tail pointer for this message */
	MsgPart *parts;	/* The message parts */
	Message *next;	/* The next message */
	int msg_number;		/* What is this message number */
	PGPByte writing;		/* Are we writing? */
	PGPByte foundpart1;	/* Did we find part 1? */
	DEBUG_STRUCT_CONSTRUCTOR( Message )
} ;

typedef struct PrsAscContext {
	PGPPipeline		pipe;
	
	PGPFifoDesc const *fifod;	/* Fifo Descriptor */
	PGPFifoContext *ann; /* Fifo to hold the saved annotations */
	PGPFifoContext *data; /* Fifo to hold saved non-PGP data */
	Message *msgs;	/* List of current messages */
	MsgPart *part;	/* The current message part */
	PGPPipeline *myself; /* Pointer to this pipeline module */
	PGPPipeline *tail; /* Tail pointer for rest of the pipeline */
	PGPUICb const *ui;
	PGPEnv const *env;
	void *ui_arg;
	unsigned long written;	/* Fifo delta from last command */
	unsigned long left;	/* Bytes left in fifo until next command */
	unsigned thispart;	/* This part, obtained from the PGP header */
	unsigned maxparts;	/* Max parts, obtained from the PGP header */
	int state;		/* FSM state */
	int annotation;		/* Annotation State */
	int depth_at_ann;	/* Scope depth when we sent the annotation */
	int scope_depth;	/* Count of scopes we're inside */
	char messageid[LINE_LEN]; /* Message ID, obtained from the PGP header*/
	PGPByte databuf[65];	/* Space to hold raw data 60+slush */
	PGPByte *dataptr;		/* Pointer into the data buffer */
	unsigned datalen;	/* length of data buffer */
	PGPByte armorline[LINE_LEN]; /* line of armor */
	PGPByte *armorptr;		/* pointer into armorline */
	unsigned armorlen;	/* length of line */
	unsigned long crc;	/* CRC Checksum */
	PGPByte hashlist[255];	/* List of hashes */
	PGPByte hashlen;		/* Length of the list of hashes */
	PGPByte cmdarg[kPGPPrsAscCmdBufSize];	/* buffered command */
	unsigned cmdlen;	/* length of buffered command */
	int msg_count;		/* A count of the number of messages */
	PGPByte command;		/* the command that is buffered */
	PGPByte buffering;		/* A flag -- are we buffering everything? */
	PGPByte eol;		/* Our readLine EOL flag */
	PGPByte eob;		/* End of Buffer flag (from readLine) */
	PGPByte expectcrc;		/* Are we expecting the CRC line next? */
	PGPByte noarmorblanks;	/* Armor data starts on next line after BEGIN */
	PGPLineEndType crlf;	/* The type of crlf we have */
	PGPByte saved_crlf;	/* Save it off... */
	PGPSize bytesread;		/* Number of bytes read, total */
	PGPSize prevlineoff;	/* bytesread value at start of last line */
	PGPSize sectoff;		/* bytesread value at start of last section */
	size_t	skipPrefixLength; /* Number of chars preceding -----BEGIN PGP... */
	PGPByte	skipPrefix[LINE_LEN]; /* Chars preceding -----BEGIN PGP... */
	PGPByte crcendline;		/* CRC at end of last armor line */
	PGPByte	passthrough;	/* Passing armor or cleartext data unchanged */
	PGPByte passthroughcleartext;	/* Request passthrough on cleartext */
	PGPByte passthroughkey;	/* Request passthrough on key blocks */
#if MIMEPARSE
	PGPByte mime_signed;	/* Dealing with a PGP/MIME clearsigned message */
	PGPByte no_mime_headers;		/* Don't have mime headers, look in body */
	PGPByte mime_boundary[256];	/* MIME boundary with "--" */
	int mime_bound_len;	/* strlen(mime_boundary) */
#endif
	DEBUG_STRUCT_CONSTRUCTOR( PrsAscContext )
} PrsAscContext;


static char const *	sDearmorTable = NULL;	/* enforce 'const' access */

/* forward references */
static PGPError writeMessage (PrsAscContext *ctx, Message *msg);
static PGPError sendAnnotate (PrsAscContext *ctx, PGPPipeline *origin,
					 int type, PGPByte const *string, size_t size);
static PGPError flushMessage (PrsAscContext *ctx, Message *msg);


/* Case-insensitive compare routine */
static int
strncmp_ignorecase (const char *str1, const char *str2, int len)
{
	while (len--) {
		char c1 = tolower(*str1++);
		char c2 = tolower(*str2++);
		if (c1 != c2) {
			return (int)c1 - (int)c2;
		}
	}
	return 0;
}


/*
 * Close this part.  This makes sure that this message is being written
 * before it does this.  It will call a sizeAdvise (0) on the join
 * module and then possibly a teardown, too.
 */
static PGPError
closePart (MsgPart *part)
{
	PGPError	error;

	if (! part->msg->writing)
		return kPGPError_NoErr;

	if (part->sig) {
		error = part->sig->sizeAdvise (part->sig, 0);
		if (error)
			return error;
	}

	if (part->mod) {
		error = part->mod->sizeAdvise (part->mod, 0);
		if (error)
			return error;
	}

	if (part->text) {
		error = part->text->sizeAdvise (part->text, 0);
		if (error)
			return error;
	}

	part->done = 2;
	return kPGPError_NoErr;
}

/*
 * Free a message part; this tears down the join module, which must be
 * unhooked from the pipeline first.
 */
static void
freePart (MsgPart *part)
{
	MsgPart **partp = &part->msg->parts;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( *partp, MsgPart );
	cdkContext	= (*partp)->cdkContext;

	if (part->mod)
		part->mod->teardown (part->mod);
	if (part->text)
		part->text->teardown (part->text);
	if (part->sig)
		part->sig->teardown (part->sig);

	while (*partp) {
		if (*partp == part) {
			*partp = part->next;
			pgpClearMemory( part,  sizeof (*part));
			pgpContextMemFree( cdkContext, part);
			return;
		}
	}
	/* no match found */
	return;
}

/*
 * We are done with the current part, so mark it as done.  We should
 * also close this part if we are writing this message and either
 * there is a next part or this is the last part.  Then try to flush
 * the message, in case we have anything buffered.  Finally, clear the
 * part from the context so we know we don't have a current part.
 */
static PGPError
donePart (PrsAscContext *ctx)
{
	MsgPart *part = ctx->part;
	PGPError	error;

	pgpAssert (part);

	part->done = 1;
	if (part->msg->writing) {
		if (part->next || part->num == part->msg->size) {
			error = closePart (part);
			if (error)
				return error;
		}
		error = flushMessage (ctx, part->msg);
		if (error)
			return error;
	}

	ctx->part = NULL;
	return( kPGPError_NoErr );
}

/*
 * We're going to start a new part.  Therefore we need to pass in the
 * message that it is a part of, and then create the new part or
 * return the pre-created part.  If we ask for a part number in this
 * message that is beyond the highest number in the list, then we
 * create that many more parts up to the number passed in.
 *
 * Should we close old parts if we can?  I don't know, yet.  If we've
 * already written out part one and we just got part two, should I close
 * part one here?  I don't know, yet.
 */
static MsgPart *
getPart (PrsAscContext *ctx, Message *msg, unsigned num)
{
	MsgPart *temp = NULL, **part = &msg->parts;
	PGPPipeline *last = NULL;
	unsigned highest = 0;
	PGPContextRef	cdkContext;

	pgpAssert( IsntNull( ctx ) && IsntNull( ctx->myself ) );
	cdkContext	= ctx->myself->cdkContext;
	
	/* Initialize the CRC counter! */
	ctx->crc = CRC_INIT;

	while (*part) {
		if (num == (*part)->num)
			return *part;
		highest = (*part)->num;
		last = (*part)->mod;
		part = &(*part)->next;
	}

	/* We need to allocate a bunch of parts */
	for (highest++; highest <= num; highest++) {
		temp = (MsgPart *)pgpContextMemAlloc( cdkContext,
			sizeof (*temp), kPGPMemoryMgrFlags_Clear);
		if (!temp)
			return NULL;

		temp->num = highest;
		temp->msg = msg;
		temp->cdkContext	= cdkContext;

		if (last) {
			temp->mod = pgpJoinAppend (last);
		} else {
			msg->tail = pgpJoinCreate ( cdkContext, &(temp->mod), ctx->fifod);
			temp->first = 1;
			msg->tail = pgpParseBinCreate ( cdkContext, msg->tail, ctx->env,
											ctx->ui, ctx->ui_arg );
			/*
			 * Binary parser may add modules after itself, and in some
			 * error recovery situations they are still there when we try
			 * to close things.  Create a copy module to act as a stable
			 * predecessor to context->tail.
			 */
			msg->tail = pgpCopyModCreate ( cdkContext, msg->tail);

			/* connect the join module to the rest of the pipe */
			if (msg->tail)
				*(msg->tail) = ctx->tail;
		}

		if (!temp->mod) {
			pgpContextMemFree( cdkContext, temp);
			return NULL;
		}

		*part = temp;
		last = temp->mod;
		part = &temp->next;
	}
			
	return temp;
}

/*
 * We have a part of a message "name" of size "size" -- lets try to
 * find the message that this is a part of, and either return that
 * message pointer or create a new one and return that.  If name is
 * non-NULL, then it will compare the name passed in with older names
 * (i.e., a messageID).  If name is NULL, then it will compare sizes
 * for old-style multipart armor.  Do not return messages that have
 * only one part.
 *
 * This means that you can only have one old-style multipart message
 * going of any particular size (number of parts).  I don't consider
 * this a big problem, since interleaved messages are rarely a
 * problem.  Out-of-order messages, on the other hand.....
 */
static Message *
getMessage (PrsAscContext *ctx, char const *name, unsigned size)
{
	Message **msg = &ctx->msgs;
	char *newname;
	PGPContextRef		cdkContext	= ctx->pipe.cdkContext;

	/* Just remove name if it is a NULL string */
	if (name && !*name)
		name = NULL;

	if (!name && !size)
		return NULL;

	while (*msg) {
		if (name && (*msg)->name && !memcmp (name, (*msg)->name,
		    strlen (name))) {
			/* Update the message size if we know it */
			if (!(*msg)->size && size)
				(*msg)->size = size;
			return *msg;
		}
		if (!name && !(*msg)->name && size == (*msg)->size &&
		    size != 1)
			return *msg;
		msg = &((*msg)->next);
	}

	*msg = (Message *)pgpContextMemAlloc( cdkContext,
		sizeof (**msg), kPGPMemoryMgrFlags_Clear);
	if (! *msg)
		return NULL;

	(*msg)->cdkContext	= cdkContext;
	
	if (name) {
		newname = (char *)pgpContextMemAlloc( cdkContext,
			strlen (name) + 1, kPGPMemoryMgrFlags_Clear);
		if (!newname) {
			pgpContextMemFree( cdkContext, *msg);
			*msg = NULL;
			return NULL;
		}
		memcpy (newname, name, strlen (name) + 1);
	} else
		newname = NULL;

	(*msg)->name = newname;
	(*msg)->size = size;
	(*msg)->msg_number = ++(ctx->msg_count);

	return *msg;
}

/*
 * We're done with this message so we can remove it from our queue
 * of pending messages.  Free any remaining message parts in the process.
 * We can also turn buffering off now.
 */
static void
freeMessage (PrsAscContext *ctx, Message *msg)
{
	Message **msgp = &ctx->msgs;
	MsgPart *part, *temp;
	PGPContextRef		cdkContext	= ctx->pipe.cdkContext;

	while (*msgp) {
		if (*msgp == msg) {
			*msgp = msg->next;
			part = msg->parts;
			if (msg->tail)
				/* Disconnect join module */
				*(msg->tail) = NULL;
			while (part) {
				temp = part;
				part = part->next;
				freePart (temp);
			}
			if (msg->name)
				pgpContextMemFree( cdkContext, msg->name);
			ctx->buffering = 0;
			pgpClearMemory( msg,  sizeof (*msg));
			pgpContextMemFree( cdkContext, msg);
				
			return;
		}
	}
	/* no match found */
	return;
}

/*
 * Buffer a command at the current spot in the data stream.  This is only
 * called when ctx->buffering is true.  What it does is find the delta
 * of this command in the data fifo from the last command in order to replay
 * the commands at the proper place.  It saves off context->written as
 * the offset and then resets the value to 0.
 *
 * The return value is 0 on success or an error code.
 *
 * Command Byte Syntax: <offset><cmd><arglen><arg>
 */
static PGPError
bufferCommand (
	PrsAscContext *ctx, PGPByte cmd, const PGPByte *arg, unsigned arglen)
{
	/* Make sure the command argument fits the buffer size */
	if (arglen > sizeof (ctx->cmdarg))
		return kPGPError_BadParams;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, (PGPByte const *)&ctx->written,
			  sizeof (ctx->written)) != sizeof (ctx->written))
		return kPGPError_OutOfMemory;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, &cmd, sizeof (cmd))
	    != sizeof (cmd))
		return kPGPError_OutOfMemory;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, (PGPByte const *)&arglen,
			  sizeof (arglen)) != sizeof (arglen))
		return kPGPError_OutOfMemory;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, arg, arglen) != arglen)
		return kPGPError_OutOfMemory;


	ctx->written = 0;
	return( kPGPError_NoErr );
}

/* Buffer up a write command for a particular message */
static PGPError
bufferWriteCommand (PrsAscContext *ctx, Message *msg)
{
	PGPByte arg[sizeof (msg)];
	memcpy (arg, (PGPByte const *)&msg, sizeof (arg));

	return bufferCommand (ctx, CMD_WRITE, arg, sizeof (arg));
}

/* Process a buffered Write command for a particular message */
static PGPError
parseWriteCommand (PrsAscContext *ctx)
{
	Message *msg;

	pgpAssert (ctx->cmdlen == sizeof (msg));
	memcpy ((PGPByte *)&msg, ctx->cmdarg, sizeof (msg));
	return writeMessage (ctx, msg);
}

/* Buffer up an annotation */
static PGPError
bufferAnnotation (PrsAscContext *ctx, PGPPipeline *origin, int type,
		  PGPByte const *string, size_t size)
{
	PGPByte *arg;
	PGPByte *argp;
	PGPError retval;
	PGPContextRef		cdkContext	= ctx->pipe.cdkContext;

	arg = argp = (PGPByte *)pgpContextMemAlloc( cdkContext,
			sizeof (origin) + sizeof (type) + size + sizeof (size),
			kPGPMemoryMgrFlags_Clear );
	if (!arg)
		return kPGPError_OutOfMemory;

	memcpy (argp, (PGPByte const *)&origin, sizeof (origin));
	argp += sizeof (origin);

	memcpy (argp, (PGPByte const *)&type, sizeof (type));
	argp += sizeof (type);

	memcpy (argp, (PGPByte const *)&size, sizeof (size));
	argp += sizeof (size);

	memcpy (argp, string, size);
	argp += size;

	retval = bufferCommand (ctx, CMD_ANNOTATE, arg, argp-arg);
	pgpContextMemFree( cdkContext, arg);
	return retval;
}

static PGPError
parseAnnotation (PrsAscContext *ctx)
{
	PGPPipeline *origin;
	int type;
	size_t size;
	PGPByte *string, *argp = ctx->cmdarg;
	PGPError retval;
	PGPContextRef		cdkContext	= ctx->pipe.cdkContext;

	pgpAssert (ctx->cmdlen >= sizeof (origin) + sizeof (type) +
		sizeof (size));

	memcpy ((PGPByte *)&origin, argp, sizeof (origin));
	argp += sizeof (origin);

	memcpy ((PGPByte *)&type, argp, sizeof (type));
	argp += sizeof (type);

	memcpy ((PGPByte *)&size, argp, sizeof (size));
	argp += sizeof (size);

	/* make sure we have the right number of bytes remaining */
	pgpAssert ((ctx->cmdlen - size) == (unsigned)(argp - ctx->cmdarg));

	string = (PGPByte *)pgpContextMemAlloc( cdkContext,
		size, kPGPMemoryMgrFlags_Clear);
	if (string == NULL)
		return kPGPError_OutOfMemory;

	pgpCopyMemory (argp, string, size);

	retval = sendAnnotate (ctx, origin, type, string, size);

	pgpContextMemFree( cdkContext, string);
	return retval;
}

/*
 * Given a buffered command, process it appropriately.  The cmd, args,
 * and arglen are currently held in the ctx.  Set arglen to 0 and
 * return 0 on success, otherwise return an error.
 */
static PGPError
processCommand (PrsAscContext *ctx)
{
	PGPError	error = kPGPError_NoErr;

	switch (ctx->command) {
	case CMD_WRITE:
		error = parseWriteCommand (ctx);
		break;
	case CMD_ANNOTATE:
		error = parseAnnotation (ctx);
		break;
	default:
		/* unknown command type.  This should NEVER happen */
		pgpAssert (0);
	}

	if (!error)
		ctx->cmdlen = 0;
	return error;
}

/*
 * Write a buffer of data which is a message part.  This will either
 * write or buffer depending on what the current state of the world
 * is.
 *
 * This will either perform a write() or a pgpJoinBuffer() depending
 * on what it is and what the state is.  We perform a write if this is
 * the writing message, otherwise we just buffer it.
 */
static size_t
writePartData (MsgPart *part, PGPByte const *buf, size_t size, PGPError *error)
{
	Message *msg = part->msg;

	/* Ignore parts that we've already finished */
	if (part->done)
		return size;

	if (part->sig)
		return part->sig->write (part->sig, buf, size, error);

	if (msg->writing)
		return part->mod->write (part->mod, buf, size, error);

	return pgpJoinBuffer (part->mod, buf, size);
}

/*
 * This will write 'extra' data out.  In general this is just
 * non-Ascii armor data.  However if we are within a "buffering
 * everything" mode, then this will get buffered into the ctx->data
 * fifo and ctx->written will be increased.
 */
static size_t
writeExtraData (
	PrsAscContext *ctx, PGPByte const *buf, size_t size, PGPError *error)
{
	size_t written;

	/* Handle clearsign text */
	if (ctx->part && ctx->part->text)
		return ctx->part->text->write (ctx->part->text, buf, size,
					       error);

	pgpAssert (ctx->tail);

	if (ctx->buffering) {
		written = pgpFifoWrite (ctx->fifod, ctx->data, buf, size);
		ctx->written += written;
		return written;
	}

	return ctx->tail->write (ctx->tail, buf, size, error);
}

/*
 * This will start with the first part that we have.  For each part,
 * it will see if it is done.  If not, return 0.  If it is done, then
 * see if there is another part sitting after it.  If so, close this
 * part, return an error if one exists, or move on to the next part if
 * there was no error closing the part.  If there is not another part
 * after this one, then only close this one if this is the last part
 * (msg->size && part->num == msg->size).  Otherwise return 0.
 *
 * If we flushed the whole message, free it.
 */
static PGPError
flushMessage (PrsAscContext *ctx, Message *msg)
{
	MsgPart *temp, *part = msg->parts;
	PGPError	error = kPGPError_NoErr;

	do {
		if (!part->done)
			break;

		if (part->next) {
			error = closePart (part);
			if (error)
				break;
			temp = part->next;
			freePart (part);
			part = temp;

		} else if (part->num == msg->size) {
			pgpAssert (!part->next);

			error = closePart (part);
			if (error)
				break;
			error = ctx->tail->annotate (ctx->tail, ctx->myself,
						     PGPANN_ARMOR_END, NULL,
						     0);
			if (error)
				break;
			freeMessage (ctx, msg);
			/* freeMessage will free the last part */
			break;
		} else
			break;

	} while (part);

	return error;
} 

/*
 * Set the state so that we are writing out this message.  It means that
 * we are probably going to be buffering everything else until this message
 * is done.  If we are already "buffering" everything, then buffer up this
 * command for later execution.
 */
static PGPError
writeMessage (PrsAscContext *ctx, Message *msg)
{
	PGPError	error;

	msg->foundpart1 = 1;
	if (ctx->buffering)
		return bufferWriteCommand (ctx, msg);

	error = ctx->tail->annotate (ctx->tail,
		ctx->myself, PGPANN_ARMOR_BEGIN,
		(PGPByte *)&ctx->sectoff, sizeof(ctx->sectoff));
	if (error)
		return error;

	ctx->buffering = 1;
	msg->writing = 1;

	/* Start flushing the message now */

	return flushMessage (ctx, msg);
}

/*
 * This is the main logic in flushing out the buffered data.  This
 * function will try to flush the data fifo and command fifo in order
 * to create the appropriate output stream.
 *
 * There are two streams which need to be merged.  Since the command
 * stream gives deltas into the datastream, we need to use the command
 * stream as the control.  No problem.
 *
 * First, we flush out any data in the data stream until the next
 * command, until ctx->left == 0.  Next, if ctx->cmdlen != 0 we
 * process the command in the ctx command buffer.  If that returns
 * without error we read in the next command/offset into ctx (->left
 * and ->cmdlen) and then repeat.  It at any time an error occurs,
 * stop and return the error code.
 */
static PGPError
flushBuffers (PrsAscContext *ctx)
{
	PGPSize datalen, cmdlen, len;
	PGPByte const *ptr;
	PGPError	error = kPGPError_NoErr;
	size_t written;

	pgpAssert (! ctx->buffering);

	datalen = pgpFifoSize (ctx->fifod, ctx->data);
	cmdlen = pgpFifoSize (ctx->fifod, ctx->ann);

	/* If nothing is buffered, just return ok */
	if (!datalen && !cmdlen)
		return( kPGPError_NoErr );

	/* Now try to flush the buffers... */
	do {
		/*
		 * If buffering gets turned on processing a command,
		 * just exit this function so more data can be read in.
		 */
		if (ctx->buffering)
			return( kPGPError_NoErr );

		/* First flush any data until the next command */
		while (ctx->left) {
			ptr = pgpFifoPeek (ctx->fifod, ctx->data, &len);
			pgpAssert (len);
			pgpAssert (ptr);

			if (len > ctx->left)
				len = ctx->left;
			written = writeExtraData (ctx, ptr, len, &error);
			pgpFifoSeek (ctx->fifod, ctx->data, written);
			ctx->left -= written;
			datalen -= written;
			if (error)
				return error;
		}

		/* Process the buffered command, if any */
		if (ctx->cmdlen) {
			error = processCommand (ctx);
			if (error)
				return error;
		}

		/*
		 * Now, read in the data for the next command.  The
		 * syntax is: <offset><cmd><arglen><arg>
		 */
		if (cmdlen) {
			if (pgpFifoRead (ctx->fifod, ctx->ann,
					 (PGPByte *)&ctx->left,
					 sizeof (ctx->left)) !=
			    sizeof (ctx->left))
				return kPGPError_FIFOReadError;
			cmdlen -= sizeof (ctx->left);

			if (pgpFifoRead (ctx->fifod, ctx->ann, &ctx->command,
					 sizeof (ctx->command)) !=
			    sizeof (ctx->command))
				return kPGPError_FIFOReadError;
			cmdlen -= sizeof (ctx->command);

			if (pgpFifoRead (ctx->fifod, ctx->ann,
					 (PGPByte *)&ctx->cmdlen,
					 sizeof (ctx->cmdlen)) !=
			    sizeof (ctx->cmdlen))
				return kPGPError_FIFOReadError;
			cmdlen -= sizeof (ctx->cmdlen);

			if (pgpFifoRead (ctx->fifod, ctx->ann, ctx->cmdarg,
					 ctx->cmdlen) != ctx->cmdlen)
				return kPGPError_FIFOReadError;
			cmdlen -= ctx->cmdlen;
		} else {
			/*
			 * If this can happen then we need to do this
			 * and keep track of the amount of data left in
			 * the fifo.
			 */

			ctx->left = datalen;
			ctx->written = datalen;
		}

	} while (datalen || cmdlen || ctx->cmdlen);

	/* If we get here then everything has been written out. */
	ctx->written = 0;

	return( kPGPError_NoErr );
}

/*
 * TBD for MIME support:
 * Add a filter to remove some MIME transforms.  To wit:
 *	First read headers, looking for blank line.  If see any header other
 *	than Content-Type: text/plain... or Content-Transfer-Encoding
 *	other than quoted-printable (or whatever the plain one is), we can't
 *	handle it, and we should just pass the whole thing through.  Otherwise
 *	we delete the header and trailing blank line.  We then run through the
 *	filter which undoes the content-transfer-encoding transformation if
 *	necessary, or else just passes data through.
 * This filter should be added just before the copymod, below.  It might
 * even replace the copymod, but I don't remember what it is for.  Something
 * to do with some error recovery state, where we needed to know we were
 * pointing at the last thing in the pipeline before context->tail?
 */

/*
 * This is a clearsigned message.  We need to setup the verification
 * pipeline to get this to work.  First we have a join module.  It is a
 * placeholder for the message.  The clearsigned text goes into the
 * second input of the join module.  The output of the join module goes
 * to a split.  The first output of the split goes to the annotation
 * reader.  The second output of the split goes into a textfilt module
 * to strip the spaces and then into a signature verification module.
 */
static PGPError
createClearsig (
	PGPContextRef cdkContext,
	PrsAscContext *ctx,
	Message *msg,
	MsgPart *part)
{
	/*
	 * We don't need a join module for clearsigning.  That causes the
	 * entire clearsigned part to be buffered in the join module until
	 * we get to the cleanup phase!  Redo that.
	 */
#if 0
	PGPPipeline *join = NULL, *split = NULL, *text = NULL;
	PGPPipeline **tail, **temp;
	PGPPipeline *sighead = NULL, **sigtail;

	/* First, build up a replacement "join/split" module */
	tail = pgpJoinCreate (cdkContext, &join, ctx->fifod);
	if (!tail)
		return kPGPError_OutOfMemory;

	temp = tail;
	tail = pgpSplitCreate (cdkContext, temp);
	if (!tail) {
		return kPGPError_OutOfMemory;
	}
	split = *temp;

	/* Splice in the join/split in place of the old join/parser */
	*(msg->tail) = NULL;
	part->mod->teardown (part->mod);

	part->mod = join;
	msg->tail = tail;

	/*
	 * Binary parser may add modules after itself, and in some
	 * error recovery situations they are still there when we try
	 * to close things.  Create a copy module to act as a stable
	 * predecessor to context->tail.
	 */
	msg->tail = pgpCopyModCreate ( cdkContext, msg->tail);

	/* And connect the tail */
	if (msg->tail)
		*(msg->tail) = ctx->tail;

	/* Create the text input */
	text = pgpJoinAppend (join);
	if (! text) {
		return kPGPError_OutOfMemory;
	}

	/* add a textfilt module */
	tail = pgpSplitAdd (split);
	if (!tail) {
		text->teardown (text);
		return kPGPError_OutOfMemory;
	}

#if MIMEPARSE
	tail = pgpTextFiltCreate (cdkContext, tail, charMapIdentity, 1,
			(ctx->mime_signed ? kPGPLineEnd_CRLF : kPGPLineEnd_Default));
#else
	tail = pgpTextFiltCreate ( cdkContext,
		tail, charMapIdentity, 1, kPGPLineEnd_Default);
#endif
	if (!tail) {
		text->teardown (text);
		return kPGPError_OutOfMemory;
	}

	/* Create the signature parser... */
	sigtail = pgpParseBinCreate ( cdkContext, &sighead, ctx->env,
								  ctx->ui, ctx->ui_arg );
	if (!sigtail) {
		text->teardown (text);
		return kPGPError_OutOfMemory;
	}

	/* ...and the signature verifier */
	if (!pgpVerifyReaderCreate ( cdkContext,
				tail, sigtail, ctx->env, ctx->fifod,
				    (ctx->hashlen ? ctx->hashlist : NULL),
				    ctx->hashlen, (ctx->hashlen ? 1 : 0),
				    ctx->ui, ctx->ui_arg)) {
		text->teardown (text);
		sighead->teardown (sighead);
		return kPGPError_OutOfMemory;
	}

	/* Now put it all together in this message */
	part->text = text;
	part->sig = sighead;
#else
	PGPPipeline *text = NULL;
	PGPPipeline **tail;
	PGPPipeline *sighead = NULL, **sigtail;

	tail = pgpSplitCreate (cdkContext, &text);

	/* Splice in the split in place of the old join/parser */
	*(msg->tail) = NULL;
	part->mod->teardown (part->mod);

	part->mod = NULL;		/* No join module */
	msg->tail = tail;

	/*
	 * Binary parser may add modules after itself, and in some
	 * error recovery situations they are still there when we try
	 * to close things.  Create a copy module to act as a stable
	 * predecessor to context->tail.
	 */
	msg->tail = pgpCopyModCreate ( cdkContext, msg->tail);

	/* And connect the tail */
	if (msg->tail)
		*(msg->tail) = ctx->tail;

	/* add a textfilt module */
	tail = pgpSplitAdd (text);
	if (!tail) {
		return kPGPError_OutOfMemory;
	}

	/* Not clear about using Default on non-MIME here */
	tail = pgpTextFiltCreate (cdkContext, tail, charMapIdentity, 1,
			(ctx->mime_signed ? kPGPLineEnd_CRLF : kPGPLineEnd_Default));
	if (!tail) {
		return kPGPError_OutOfMemory;
	}

	/* Create the signature parser... */
	sigtail = pgpParseBinCreate ( cdkContext, &sighead, ctx->env,
								  ctx->ui, ctx->ui_arg);
	if (!sigtail) {
		return kPGPError_OutOfMemory;
	}

	/* ...and the signature verifier */
	if (!pgpVerifyReaderCreate ( cdkContext,
				tail, sigtail, ctx->env, ctx->fifod,
				    (ctx->hashlen ? ctx->hashlist : NULL),
				    ctx->hashlen, (ctx->hashlen ? 1 : 0),
				    ctx->ui, ctx->ui_arg)) {
		sighead->teardown (sighead);
		return kPGPError_OutOfMemory;
	}

	/* Now put it all together in this message */
	part->text = text;
	part->sig = sighead;
#endif

	return( kPGPError_NoErr );
}

/*
 * Build up the Ascii Parser Table in order to dearmor data.  For all
 * values, set it to -1 if it is not a valid armor character and then
 * set the valid characters out of the armorTable.
 *
 * To make it thread-safe, init using local table, then copy table
 * to global.
 */
void
pgpParseAscInit (void)
{
	#define kDearmorTableSize		( (PGPSize)256 )
	static char 		sDearmorTableStorage[ kDearmorTableSize ];
	static PGPBoolean	sInited = FALSE;
	
	int		i;
	char	tempTable[ kDearmorTableSize ];

	if ( sInited )
		return;

	for (i = 0; i < (int)kDearmorTableSize; i++)
		tempTable[i] = -1;

	for (i = 0; i < (int)(sizeof(armorTable) - 1); i++)
		tempTable[armorTable[i] & 0xff] = (char)i;

	pgpCopyMemory( tempTable, sDearmorTableStorage, sizeof(tempTable) );
	sDearmorTable	= &sDearmorTableStorage[ 0 ];
	
	/* set 'sInited' only after everything has been setup above */
	sInited			= 1;
	return;
}

/*
 * Convert input bytes to output bytes.  Returns the number of
 * input bytes successfully converted.  The number of output
 * bytes available is one less than this number.
 */
static int
dearmorMorsel (PGPByte const in[4], PGPByte out[3])
{
	signed char c0, c1;

	c0 = sDearmorTable[in[0] & 255];
	if (c0 < 0)
		return 0;
	c1 = sDearmorTable[in[1] & 255];
	if (c1 < 0)
		return 1;

	out[0] = (c0 & 63u) << 2 | (c1 & 63u) >> 4;

	c0 = sDearmorTable[in[2] & 255];
	if (c0 < 0)
		return 2;

	out[1] = (c1 & 63u) << 4 | (c0 & 63u) >> 2;

	c1 = sDearmorTable[in[3] & 255];
	if (c1 < 0)
		return 3;

	out[2] = (c0 & 63u) << 6 | (c1 & 63u);

	return 4;
}

/*
 * Given a line of a certain length, convert to binary and
 * return the number of binary bytes that result, or -1 on error.
 *
 * This is very fussy about trailing junk and whatnot.
 * There is some complexity due to accepting "=3D" in place
 * of a normal "=".  This is to allow MIME-encapsulated messages
 * to be input directly, without having MIME unencapsulate them
 * first.
 *
 * This will ignore trailing white space and give an error if there
 * is too much data on the line.
 */
	int
pgpDearmorLine (PGPByte const *in, PGPByte *out, unsigned inlen)
{
	int outlen = 0;
	int t;

	while ((t = dearmorMorsel (in, out)) == 4) {
		in += 4;
		out += 3;
		inlen -= 4;
		outlen += 3;
		if (inlen < 4)
			return inlen ? -1 : outlen;
	}
	switch (t) {
	  case 2:
		if (inlen == 4) {
			if (in[2] == '=' && in[3] == '=')
				return outlen + 1;
		} else if (inlen == 8) {
			if (in[2] == '=' && in[3] == '3' && in[4] == 'D' &&
			    in[5] == '=' && in[6] == '3' && in[7] == 'D')
				return outlen + 1;
		}
		break;
	  case 3:
		if (inlen == 4) {
			if (in[3] == '=')
				return outlen + 2;
		} else if (inlen == 6) {
			if (in[3] == '=' && in[4] == '3' && in[5] == 'D')
				return outlen + 2;
		}
		break;
	}
	/* None of the above - we have an error */
	return -1;

}

/*
 * Return true if the line is a valid candidate as an armor line,
 * one with no bad characters.  This will allow us to have ascii armor
 * with no headers and no blanks before the armor data.
 * A better check would look for mults of 4 or trailing '=', but we don't
 * do that yet.
 */
static int
validArmorLine (PGPByte const *in, unsigned inlen)
{
	while (inlen--) {
		/* Fail if bad character */
		if( sDearmorTable[in[0] & 255] < 0  && in[0] != '=')
			return FALSE;
		++in;
	}
	return TRUE;
}

/*
 * Given a line thought to contain a CRC, this returns the 24-bit
 * CRC, or -1 on error.  Handles possible MIME expansion
 * of "=" to "=3D".
 */
static long
dearmorCrc (PGPByte const *in, unsigned inlen)
{
	PGPByte buf[3];

	/* skip trailing white space */
	while (inlen && isspace (in[inlen - 1]))
		inlen--;

	if (*in != '=')
		return -1;
	if (inlen == 5)
	{
		if (dearmorMorsel (in + 1, buf) != 4)
		{
			return -1;
		}
		else if (inlen == 7)
		{
			if (in[1] != '3' || in[2] != 'D' ||
			    dearmorMorsel (in + 3, buf) != 4)
			{
				return -1;
			}
			else
			{
				return -1;
			}
		}
	}
	
	/*
	 * Welcome to the famous ANSI C glitch.  ANSI C
	 * promotes to signed values where possible when preserving
	 * the value.  Thus, buf[1]<<8 is promoted to signed, then
	 * shifted, then promoted to long (on a 16-bit int machine,
	 * this causes sign-extension!), and merged with the other
	 * values.  Not Good.
	 * I'm beginning to see why people preferred the K&R unsigned-
	 * preserving rules.  Sigh.
	 */
	return (long)buf[0] << 16 | (unsigned)(buf[1] << 8 | buf[2]);
}

/*
 * try to fill up ctx->armorline up to LINE_LEN-1 bytes or a newline,
 * whichever comes first.
 * 
 * This will set ctx->eol when an EOL condition occurs.  eol == 1
 * means armorline is ready for processing.  eol == 2 means that
 * readLine needs to be called with more data (looking for \n).
 *
 * The return value is the number of bytes used.  If all the bytes are
 * used and EOL is not set, then more data is required and readLine
 * should be called with more data.
 */
static size_t
readLine (PrsAscContext *ctx, PGPByte const *buf, size_t size)
{
	size_t size0 = size;
	unsigned t, retlen;
	PGPByte eol = ctx->eol;
	PGPByte *ptr = ctx->armorline + ctx->armorlen;

	if (!size)
		return 0;

	/* If start of new line, remember total byte offset */
	if (eol == 0) {
		ctx->prevlineoff = ctx->bytesread;
	}

	/*
	 * This test is needed in case the '\r' and \'n' come in
	 * different write calls.
	 */
	if (eol == 2) {
		/* Must have hit a '\r' as last char in previous input buffer */
		if (*buf == '\n') {
			*ptr = *buf;
			ctx->armorlen++;
			ctx->eol = 1;
			ctx->crlf = kPGPLineEnd_CRLF;
			ctx->bytesread++;
			return 1;
		}
		/* 
		 * Bug fix here - wasn't doing this.  This happens when we have CR
		 * terminated lines and a CR right before end of buffer.  Should just
		 * reset our state on start of next buffer.
		 */
		ctx->eol = 1;
		return 0;
	}

	/* try to fill the input buffer with a line */
	t = (unsigned)pgpMin (LINE_LEN-1 - ctx->armorlen, size);
	for (eol = 0, retlen = 0; retlen < t && !eol; retlen++) {
		*ptr++ = *buf;
		if (*buf == '\r' || *buf == '\n')
			eol = (*buf == '\r' ? 2 : 1);
		buf++;
	}
	size -= retlen;
	ctx->armorlen += retlen;

	/* Set EOB if we hit the end of the armorline buffer */
	if (ctx->armorlen >= LINE_LEN-1)
		ctx->eob = 1;

	/*
	 * At this point we are either at EOL or not.  If not, then we
	 * either ran out of input data or ran out of buffer space.  If
	 * we are at EOL and eol==2 and we have more data, check if its a
	 * \n or something else.  If so, copy that in and set eol=1.
	 */
	if (eol) {
		ctx->crlf = (eol == 2 ? kPGPLineEnd_CR : kPGPLineEnd_LF);

		if (eol == 2 && size) {
			if (*buf == '\n') {
				*ptr = *buf;
				ctx->armorlen++;
				ctx->crlf = kPGPLineEnd_CRLF;
				size--;
			}
			eol = 1;
		}
	}

	/* Save off the EOL and return the number of bytes used */
	ctx->eol = eol;
	ctx->bytesread += size0-size;
	return size0-size;
}


/* memCmpPrefix works like memcmp except it allows certain material to
 * prefix the 1st arg.  The intention is to generalize the search for
 * "-----BEGIN PGP MESSAGE-----" so that quoted versions can be matched.
 * The rule presently is that material in angle brackets is ignored, like
 * "<bigger>", because Eudora in richtext mode sometimes puts those in.
 * Likewise non-alphabetic characters will be allowed, because people may
 * be quoting a PGP message or key with "> " and similar conventions.
 * Only the number of chars specified in testlen must match.
 * In addition to returning the error code, this function returns the
 * number of skipped chars in *skiplen.
 */
static int
memCmpPrefix(const char *orig, size_t origlen, const char *test,
			 size_t testlen, size_t *skiplen)
{
	char ctest, corig;
	size_t len = 0;
	PGPBoolean inbracket = FALSE;

	pgpAssert (IsntNull(skiplen));
	pgpAssert (testlen > 0);
	
	*skiplen = 0;
	if (origlen == 0)
		return 0;

	ctest = test[0];
	for ( ; ; ) {
		if (origlen-len < testlen)
			return 1;			/* Failure, no match */
		corig = orig[len];
		if (!inbracket) {
			if (corig == ctest && memcmp (orig+len, test, testlen) == 0) {
				*skiplen = len;
				return 0;			/* Success */
			}
			if (isalnum((int) corig))
				return 1;		/* Failure, not a legal prefix */
			if (corig == '<')
				inbracket = TRUE;
		} else {
			if (corig == '>')
				inbracket = FALSE;
		}
		++len;
	}
}


/*
 * Contained in buf (of length size) should be a Header-line.  Parse
 * it appropriately, and store it as necessary.
 *
 * A headerline must fit in the armorline buffer (I.e., it must be
 * less that LINE_LEN-1 bytes long), it must start with an Alpha
 * character and then have up to 64 AlphaNumeric characters or dashes,
 * followed by a colon and then a space.  A header that does not
 * conform to this is an error.
 * 
 * Returns 0 on success, or a HEADER_ error code.
 */
#define HEADER_TOOLONG	-1
#define HEADER_INVALID	-2
static int
parseHeader (PrsAscContext *ctx, PGPByte const *buf, size_t size)
{
	int i;
	char const *ptr = (char const *)buf;
	static char const messageid[] = "MessageID:";
	char c;

	(void)size;		/* So, why do I have this arg? */

	/* Make sure this is the WHOLE line -- if not, it is an error */
	if (!ctx->eol)
		return HEADER_TOOLONG;

	/* The first character must be an Alpha character */
	if (!isalpha ((int) (*ptr++)))
		return HEADER_INVALID;

	i = 1;
	for (;;) {
		c = *ptr++;
		/* header ends in a colon */
		if (c == ':')
			break;
		/* Arbitrary label length restriction */
		if (++i > 64)
			return HEADER_TOOLONG;	/* Error */
		/* Following chars much be alphanumeric or '-' */
		if (!isalnum((int) c) && c != '-')
			return HEADER_INVALID;	/* Error */
	}

	/* Must be a space after the ':' */
	if (*ptr++ != ' ')
		return HEADER_INVALID;

	/*
	 * Now process the header
	 *
	 * ptr points to the value; buf points to the key (of length
	 * (ptr - buf - 2), plus a colon)
	 */
	i = ptr - (char const *)buf - 1;

	/* Process message ID header */
	if (i == sizeof (messageid) - 1 && !memcmp (buf, messageid, i))
		memcpy (ctx->messageid, ptr,
			pgpMin ((strlen (ptr) + 1), sizeof (ctx->messageid)));

	/* XXX: signal an unknown header? */

	return 0;
}

/* Write data to be dearmored.
 *
 * The main FSM to control dearmoring.  The actual FSM is kind of
 * complicated so I'll try to explain it by comments in the code.
 * Suffice it to say that there are two main parts of the machine.
 * The first part deals with armor'ed data, and the second part deals
 * with out-of-armored text.  There is a little glue between them, and
 * there you have it...
 *
 * Here are the states, categorized into general phases of activity:
 *
 *	 0			Start state, outside PGP data
 *
 *	 10 -  50		Waiting for start of PGP data
 *
 *	 90 - 100		Process -----BEGIN PGP line
 *
 *	110 - 130		Process headers after -----BEGIN PGP
 *
 *	140 - 160		Setup to begin processing armored data
 *
 *	170 - 200		Process PGP armored data
 *
 *	210 - 230		Process CRC at end of armored data
 *
 *	240 - 270		Process -----END PGP line, finish up, back to 0
 *
 *	300 - 330		Clearsigned message, process Hash: lines if any
 *
 *	340 - 360		Setup to begin processing clearsigned message
 *
 *	390 - 430		Process clearsigned message body, then to 90 for sig
 *
 *	500 - 530		Process binary message, to 260 to finish up when done
 *
 *	600 - 630		Scan MIME headers, get boundary string
 *
 *	650 - 680		Look for initial MIME boundary string, to 340 when found
 *
 *	700 - 720		Check apparent initial MIME separator, to 340 if looks OK
 *
 *	750 - 770		Look for start of PGP sig in MIME clearsig sig part, to 90
 *
 *	800 - 830		Look for final MIME clearsign boundary after PGP sig
 *
 */
static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	PrsAscContext *ctx;
	Message *msg;
	MsgPart *part;
	PGPByte *ptr, *num;
	static char const prefix1[] = "-----BEGIN PGP ";
	static char const prefix2[] = ", PART ";
	static char const prefix3[] = "-----END PGP ";
	static char const suffix[] = "-----";	/* With trailing nul */
	static char const signedmsg[] = "SIGNED MESSAGE";
	static char const pubkey[] = "PUBLIC KEY";
	static char const privkey[] = "PRIVATE KEY";
	static char const sigprefix[] = "-----BEGIN PGP SIGNATURE-----";
	static char const signedprefix[] = "-----BEGIN PGP SIGNED MESSAGE-----";
#if MIMEPARSE
	PGPByte *ptr2;
	static char prefix_mimesig2[] = "-----BEGIN PGP MESSAGE-----";
	static char prefix_mime1[] = "content-type: multipart/signed;";
	static char prefix_mime2[] = "protocol=\"application/pgp-signature\"";
	static char prefix_mime3[] = "boundary=";
#if 0
	static char prefix_mime4[] = "micalg=";
	static char prefix_mime5[] = "pgp-";
#endif /* 0 */
#endif /* MIMEPARSE */
	int i, thispart, maxparts, lineused;
	size_t retval, written = 0;
	unsigned inlen;
	long crc;
	char temp_c;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);
	pgpAssert (error);

	ctx = (PrsAscContext *)myself->priv;
	pgpAssert (ctx);
	pgpAssert (ctx->tail);

	switch (ctx->state) {
	case 0:
state_start:
		ctx->state = 0;
		/*
		 * This is the "start" position.  Check if we are
		 * buffering and flush buffers if we are not.  Then go
		 * to the next state.
		 */

		if (!ctx->buffering) {
			*error = flushBuffers (ctx);
			if (*error)
				break;
		}

		/* FALLTHROUGH */
	case 10:
state_wait_for_pgp:
		ctx->state = 10;
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 20:
		ctx->state = 20;
		/*
		 * check the line for BEGIN.  If it is not a begin
		 * then we should output this line.  If it is a begin,
		 * then we need to parse it as an armor beginning.
		 * Use special compare routine which skips some prefixes of the
		 * test line so we can recognize "- -----BEGIN PGP" and such.
		 */

		if (ctx->armorlen > sizeof (prefix1) - 1 &&
		    !memCmpPrefix ((char *)ctx->armorline, ctx->armorlen,
						   prefix1, sizeof (prefix1) - 1,
						   &ctx->skipPrefixLength)) {
			/*
			 * Don't allow a prefix on BEGIN PGP SIGNATURE or BEGIN PGP
			 * SIGNED MESSAGE.  People often quote signed messages in replies
			 * and we don't want to trigger on those.  And the SIGNATURE
			 * is the last part of the signed message.
			 */
			if (ctx->skipPrefixLength == 0 ||
				(memcmp(ctx->armorline+ctx->skipPrefixLength, sigprefix,
										sizeof(sigprefix)-1)!=0 &&
				 memcmp(ctx->armorline+ctx->skipPrefixLength, signedprefix,
						  				sizeof(signedprefix)-1)!=0 )) {
				if (ctx->skipPrefixLength > 0) {
					pgpAssert (ctx->skipPrefixLength <
							   				sizeof(ctx->skipPrefix));
					pgpCopyMemory (ctx->armorline, ctx->skipPrefix,
								   ctx->skipPrefixLength);
					ctx->skipPrefix[ctx->skipPrefixLength] = '\0';
				}
				goto state_pgp_begin;
			}
		}
		/*
		 * check the line to see if it is the beginning of a
		 * PGP binary message.  This check is only done the
		 * first time through this loop, using ctx->annotation
		 * as the loop detector.  If this is a PGP message,
		 * then treat it as such and buffer the data for
		 * processing through a parser.
		 */
		 
		if (! ctx->annotation) {
			if (pgpFileTypePGP (ctx->armorline, ctx->armorlen))
			{
				/*
				 * ooh, a binary PGP message fed into the
				 * ascii armor parser!
				 */
				goto state_pgp_binary;
			}
		}

#if MIMEPARSE
		/*
		 * Look for either the MIME content-type message, or what looks
		 * like a MIME message boundary (in case we don't have headers).
		 */
		if (ctx->armorlen > sizeof (prefix_mime1) - 1 &&
			!strncmp_ignorecase ((char *)ctx->armorline, prefix_mime1,
						  sizeof (prefix_mime1) - 1)) {
			/* MIME header */
			goto state_pgpmime;
		}

		/*
		 * If no_mime_headers, we have only a body, so assume that a line
		 * starting with -- is a pgp/mime signed part.  This is not a very
		 * good assumption in general, so we will assume that we had some
		 * reason to think so.
		 */
		if (ctx->no_mime_headers && ctx->eol &&
			ctx->armorlen > 2 && !memcmp (ctx->armorline, "--", 2) &&
			ctx->eol) {
			/* Potential MIME initial body indicator */
			goto state_pgpmime_body_noheaders;
		}
#endif

		/* FALLTHROUGH */
	case 30:
state_nonpgp:
		ctx->state = 30;
		/* send annotation if we need to */
		/* Get here with armorlen==0 only on flush call, don't annotate then */
		ctx->sectoff = ctx->prevlineoff;
 		if (!ctx->annotation && ctx->armorlen!=0) {
 			*error = sendAnnotate (ctx, myself, PGPANN_NONPGP_BEGIN,
								(PGPByte *)&ctx->sectoff, sizeof(ctx->sectoff));
 			if (*error)
 				break;
 			ctx->annotation = PGPANN_NONPGP_END;
			ctx->depth_at_ann = ctx->scope_depth;
 		}
		/* FALLTHROUGH */
	case 40:
state_nonpgp_output:
		ctx->state = 40;
		/* output armorline buffer */
		while (ctx->armorlen) {
			retval = writeExtraData (ctx, ctx->armorptr,
						 ctx->armorlen, error);
			ctx->armorptr += retval;
			ctx->armorlen -= retval;
			if (*error)
				return written;
		}
		ctx->armorptr = ctx->armorline;
		/* FALLTHROUGH */
	case 50:
		ctx->state = 50;
		/*
		 * if EOL == 1, clear state and goto state wait_for_pgp, otherwise
		 * read data and return to state nonpgp_output to output the line.
		 */
		if (ctx->eol == 1) {
			ctx->eol = 0;
			ctx->eob = 0;
			goto state_wait_for_pgp;
		}

		if (!size)
			break;

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		goto state_nonpgp_output;

	case 90:
state_pgp_begin:
		ctx->state = 90;
		/* 
		 * parse begin line.  If this is not a valid begin
		 * line, assume that it is not and output it as
		 * non-PGP text (in state nonpgp).
		 * skipPrefixLength holds extra chars to skip at beginning
		 */
		ctx->sectoff = ctx->prevlineoff;
		ptr = ctx->armorline + sizeof (prefix1) - 1 + ctx->skipPrefixLength;
		lineused = ctx->armorlen - (sizeof (prefix1) - 1)
									- ctx->skipPrefixLength;
		thispart = maxparts = 0;

		do {
			if (lineused == 0) {
				/* not a valid BEGIN line */
				goto state_nonpgp;
			}

		} while (isspace (ptr[--lineused]));

		/* temporarily null-terminate this string */
		temp_c = ptr[++lineused];
		ptr[lineused] = '\0';

		if (!memcmp (ptr, signedmsg, sizeof (signedmsg)-1)) {
			/* This is a clearsigned message */

			if (ctx->eol) {
				/*
				 * This looks like a clearsigned
				 * message.  jump into the clearsigned
				 * parser and parse it.
				 */
				ptr[lineused] = temp_c;		/* restore terminator */
				goto state_cleartext;
			}
			/*
			 * Someone is playing with us.. This is an
			 * error.  
			 */
			goto err_state_pgp_begin;
		}

		if (ctx->passthroughkey &&
			(memcmp (ptr, pubkey, sizeof(pubkey)-1) == 0 ||
			 memcmp (ptr, privkey, sizeof(privkey)-1) == 0)) {
			/* Go into passthrough mode on key block */
			ctx->passthrough = TRUE;
			ptr[lineused] = temp_c;
 			*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
						   ctx->armorline, ctx->armorlen);
 			if (*error)
 				break;
			ptr[lineused] = '\0';
		}

		/* skip the "whatever" in "-----BEGIN PGP whatever" */
		while (isalnum (*ptr) || *ptr == ' ') {
			ptr++;
			lineused--;
		}

		/* check for multipart */
		if (*ptr == ',') {
			/* "-----BEGIN PGP whatever, PART x[/y]-----" */
			if (memcmp (ptr, prefix2, sizeof (prefix2) - 1))
				goto err_state_pgp_begin;
			ptr += sizeof (prefix2) - 1;
			lineused -= sizeof (prefix2) - 1;

			thispart = strtoul ((char *)ptr, (char **)&num, 10);
			if (num == NULL || thispart < 0 || thispart > 999)
				goto err_state_pgp_begin;

			lineused -= (num - ptr);
			ptr = num;

			/* 
			 * now check for "/y", if it exists.  It is
			 * legal for it not to exist, but it must have
			 * either a 'y' here or a messageID.
			 */

			if (*ptr == '/') {
				maxparts = strtoul ((char *)ptr+1,
						    (char **)&num, 10);
				if (!num || maxparts < 2 || maxparts > 999)
					goto err_state_pgp_begin;
				lineused -= (num - ptr);
				ptr = num;
			}
		} else {
			thispart = maxparts = 1;
		}
		if (memcmp (ptr, suffix, sizeof (suffix)))
			goto err_state_pgp_begin;
		ctx->thispart = thispart;
		ctx->maxparts = maxparts;
		goto state_pgp_end_anno;
err_state_pgp_begin:
		ptr[lineused] = temp_c;
		goto state_nonpgp;

 	case 100:
state_pgp_end_anno:
		ctx->state = 100;
 		/* Send end-annotation (if we sent a begin annotation) */
 		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
 			*error = sendAnnotate (ctx, myself, ctx->annotation,
 					       NULL, 0);
 			if (*error)
 				break;
 			ctx->annotation = 0;
 		}
 		/* FALLTHROUGH */
	case 110:
state_pgp_armor_header:
		ctx->state = 110;
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 120:
		ctx->state = 120;
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 125:
		ctx->state = 125;
		/* Send annotation of raw data if in passthrough mode */
 		if (ctx->passthrough) {
 			*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
						   ctx->armorline, ctx->armorlen);
 			if (*error)
 				break;
 		}

		/* FALLTHROUGH */
	case 130:		
		ctx->state = 130;
		/* 
		 * Process header line, then back to state armor_header,
		 * or detect end of headers, and go to state armor_nonheader.
		 */

		ptr = ctx->armorline;
		lineused = ctx->armorlen;

		/* Skip past prefix from "-----BEGIN PGP" if it matches */
		if (ctx->skipPrefixLength > 0 &&
			(PGPSize)lineused > ctx->skipPrefixLength &&
			0 == memcmp (ptr, ctx->skipPrefix, ctx->skipPrefixLength)) {
			ptr += ctx->skipPrefixLength;
			lineused -= ctx->skipPrefixLength;
		}

		/* Find trailing white space... */
		while (lineused && isspace (ptr[lineused - 1]))
			lineused--;

		ctx->noarmorblanks = FALSE;
		if (lineused) {
			int err;
			/* temporarily null-terminate this string */
			temp_c = ptr[lineused];
			ptr[lineused] = '\0';

			/*
			 * Change states once we see a non-header line.  This is
			 * now the only way of changing to the state where we are
			 * parsing the ascii armor.  It deals with the case where
			 * there are no blanks between the headers and the armored
			 * portion, as well as the normal case where there is a blank
			 * line, and also munging which inserts blank lines.
			 */
			 if( validArmorLine( ptr, lineused ) ) {
				ptr[lineused] = temp_c;
				ctx->noarmorblanks = TRUE;
				goto state_pgp_armor_nonheader;
			}

			err = parseHeader (ctx, ptr, lineused);
			if (err) {
				*error = sendAnnotate (ctx, myself,
						       PGPANN_ARMOR_BADHEADER,
						       ctx->armorline,
						       lineused);
				ptr[lineused] = temp_c;
				if (*error)
					break;

				/* XXX: Ignore bad headers? */
			}

			goto state_pgp_armor_header;
		}

		/*
		 * No longer change states on blank lines.  Instead use
		 * the validArmorLine test above.  This deals with the munging
		 * done by Eudora which makes every other line a blank line.
		 */
		goto state_pgp_armor_header;

		/* Not reached */
		/* FALLTHROUGH */
	case 150:
state_pgp_armor_nonheader:
		ctx->state = 150;
		/* get the message and part information */

		if (!ctx->part) {
			/* This is true for all normal armor messages */
			msg = getMessage (ctx, ctx->messageid, ctx->maxparts);
			if (!msg) {
				/* This is an error */
				*error = kPGPError_OutOfMemory;
				break;
			}
			part = getPart (ctx, msg, ctx->thispart);
			if (!part) {
				/* This is an error too */
				*error = kPGPError_OutOfMemory;
				break;
			}
			ctx->part = part;
		}
		ctx->expectcrc = 0;
		/* FALLTHROUGH */
	case 160:
		ctx->state = 160;
		/*
		 * Either try to make the message writable or send an
		 * annotation saying we have a message part.
		 */

		msg = ctx->part->msg;
		if (ctx->thispart == 1) {
			*error = writeMessage (ctx, msg);
			if (*error)
				break;
		} else {
			PGPByte *b;

			b = (PGPByte *)pgpContextMemAlloc( cdkContext,
				kPGPPrsAscCmdBufSize, kPGPMemoryMgrFlags_Clear);
			if (b == NULL)
			{
				*error = kPGPError_OutOfMemory;
				break;
			}
			
			i = 0;
			memcpy (b, &ctx->thispart, sizeof (ctx->thispart));
  			i += sizeof (ctx->thispart);

			memcpy (b+i, &ctx->maxparts, sizeof (ctx->maxparts));
			i += sizeof (ctx->maxparts);

			memcpy (b+i, ctx->messageid,
				strlen (ctx->messageid));
			i += strlen (ctx->messageid);

			*error = sendAnnotate (ctx, myself, PGPANN_ARMOR_PART,
					       b, i);
			pgpContextMemFree( cdkContext, b);
			if (*error)
				break;
		}
		/* If we had no blank lines, we already have an armor buffer */
		if( ctx->noarmorblanks ) {
			ctx->noarmorblanks = FALSE;
			goto state_pgp_armor_line;
		}
		/* FALLTHROUGH */
	case 170:
state_pgp_armor_loop:
		ctx->state = 170;
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 180:
		ctx->state = 180;
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 185:
		ctx->state = 185;
		/* Send annotation of raw data if in passthrough mode */
 		if (ctx->passthrough) {
 			*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
						   ctx->armorline, ctx->armorlen);
 			if (*error)
 				break;
 		}

		/* FALLTHROUGH */
	case 190:
state_pgp_armor_line:
		ctx->state = 190;
		/* 
		 * make sure this is not a CRC line.  If it is a CRC
		 * then goto state armor_crc.  if it is not a CRC line, then
		 * it must be an armorline.  So, process it as an
		 * armor line and then fallthrough to next state to
		 * output it.
		 */

		ptr = ctx->armorline;
		inlen = ctx->armorlen;

		/* Skip past prefix from "-----BEGIN PGP" if it matches */
		if (ctx->skipPrefixLength > 0 && inlen >= ctx->skipPrefixLength &&
			0 == memcmp (ptr, ctx->skipPrefix, ctx->skipPrefixLength)) {
			ptr += ctx->skipPrefixLength;
			inlen -= ctx->skipPrefixLength;
		}

		if (ptr[0] == '=') {
			/* This looks like a CRC */
			goto state_pgp_armor_crc;
		}

		/* find the end of the armorline */
		while (inlen && isspace (ptr[inlen - 1]))
			inlen--;

		/* check the length of the armorline */
		if (inlen > 80) { 
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_TOOLONG,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;

			goto state_pgp_finish;
		}

		if (inlen == 0) {
			/* Blank lines are just skipped, some mailers put blanks in */
			goto state_pgp_armor_loop;
		}

		/* Make sure we're not expecting a CRC */
		if (ctx->expectcrc) {
			*error = sendAnnotate (ctx, myself, PGPANN_ARMOR_NOCRC,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;

			/* Well, maybe this is an end? */
			goto state_pgp_armor_end;
		}

		if (inlen >= 9 && ptr[inlen-5] == '=') {
			/* Handle CRC at end of armor line */
			ctx->crcendline = TRUE;
			i = pgpDearmorLine (ptr, ctx->databuf, inlen - 5);
		} else {
			i = pgpDearmorLine (ptr, ctx->databuf, inlen);
		}
		
		if (i < 1 || i > 60) {
			/* error in deamorline */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_BADLINE,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;
			goto state_pgp_finish;
		}
		ctx->expectcrc = (i < 48);
		ctx->crc = crcUpdate (ctx->crc, ctx->databuf, i);
		ctx->datalen = i;
		ctx->dataptr = ctx->databuf;
		/* FALLTHROUGH */
	case 200:
		ctx->state = 200;
		/* output the dearmored data, then goto state armor_loop */
		/* But check for CRC on end of armor line and to armor_crc if so */

		part = ctx->part;
		while (ctx->datalen) {
			retval = writePartData (part, ctx->dataptr,
						ctx->datalen, error);
			ctx->dataptr += retval;
			ctx->datalen -= retval;
			if (*error)
				return written;
		}

		/* If had CRC at end of last armor line, go process it now */
		if (ctx->crcendline) {
			ctx->crcendline = FALSE;
			goto state_pgp_armor_crc;
		}

		goto state_pgp_armor_loop;

	case 210:
state_pgp_armor_crc:
		ctx->state = 210;
		/* Deal with CRC */

		ptr = ctx->armorline;
		inlen = ctx->armorlen;

		/* Skip past prefix from "-----BEGIN PGP" if it matches */
		if (ctx->skipPrefixLength > 0 && inlen >= ctx->skipPrefixLength &&
			0 == memcmp (ptr, ctx->skipPrefix, ctx->skipPrefixLength)) {
			ptr += ctx->skipPrefixLength;
			inlen -= ctx->skipPrefixLength;
		}

		/* find the end of the armorline */
		while (inlen && isspace (ptr[inlen - 1]))
			inlen--;

		if (ptr[0] != '=') {
			/* Must be CRC at end of line */
			pgpAssert(inlen >= 9 && ptr[inlen-5] == '=');
			ptr += inlen - 5;
			inlen = 5;
		}

		crc = dearmorCrc (ptr, inlen);
		if (crc < 0) {
			/* error */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_CRCCANT,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;
			goto state_pgp_finish;
		}
		if ((PGPUInt32)crc != ctx->crc) {
			/* mismatch */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_CRCBAD,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;
			goto state_pgp_finish;
		}
		/* FALLTHROUGH */
	case 220:
state_pgp_armor_end_loop:
		ctx->state = 220;
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 230:
		ctx->state = 230;
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 235:
		ctx->state = 235;
		/* Send annotation of raw data if in passthrough mode */
 		if (ctx->passthrough) {
 			*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
						   ctx->armorline, ctx->armorlen);
 			if (*error)
 				break;
 		}

		/* FALLTHROUGH */
	case 240:
state_pgp_armor_end:
		ctx->state = 240;
		/* check for END */

		ptr = ctx->armorline;
		inlen = ctx->armorlen;

		/* Skip past prefix from "-----BEGIN PGP" if it matches */
		if (ctx->skipPrefixLength > 0 && inlen >= ctx->skipPrefixLength &&
			0 == memcmp (ptr, ctx->skipPrefix, ctx->skipPrefixLength)) {
			ptr += ctx->skipPrefixLength;
			inlen -= ctx->skipPrefixLength;
		}

		/* ignore blank line here */
		while (inlen && isspace (ctx->armorline[inlen - 1]))
			inlen--;

		if (inlen==0) {
			goto state_pgp_armor_end_loop;
		}

		/* Look for END PGP line */
		if (inlen < sizeof (prefix3) - 1 ||
		    memcmp (ptr, prefix3, sizeof (prefix3) - 1)) {
			/* missing end -- do what??? */
			;
		}

#if MIMEPARSE
		if (ctx->mime_signed) {
			/* Special cleanup for MIME, wait for final boundary */
			goto state_pgpmime_sig_end;
		}
#endif

		/* FALLTHROUGH */
	case 250:
		ctx->state = 250;
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}

		/* FALLTHROUGH */
	case 260:
state_pgp_finish:
		ctx->state = 260;
		/*
		 * finish this part, send an end annotation if we need
		 * to, then clear the context and go to state start.
		 * Be aware that this state number is hard-coded in SizeAdvise,
		 * below.
		 */

 		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
 			*error = sendAnnotate (ctx, myself, ctx->annotation,
 					       NULL, 0);
 			if (*error)
 				break;
 			ctx->annotation = 0;
 		}

		/* FALLTHROUGH */

	case 270:
		ctx->state = 270;
		part = ctx->part;
		*error = donePart (ctx);
		if (*error)
			break;

		ctx->part = NULL;
		ctx->thispart = 0;
		ctx->maxparts = 0;
		pgpClearMemory (ctx->messageid, sizeof (ctx->messageid));
		ctx->dataptr = ctx->databuf;
		ctx->datalen = 0;
		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		ctx->expectcrc = 0;
		ctx->crlf = kPGPLineEnd_Default;
		ctx->saved_crlf = 0;
		ctx->hashlen = 0;
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->skipPrefixLength = 0;
		ctx->passthrough = 0;
		goto state_start;
		
		/* BEGIN CLEARSIGNED MESSAGE BELOW HERE */
	case 300:
state_cleartext:
		ctx->state = 300;

		/* See if going into passthrough mode, send BEGIN line if so */
		if (ctx->passthroughcleartext) {
			ctx->passthrough = TRUE;
			/* Send annotation of raw data if in passthrough mode */
			if (ctx->passthrough) {
				*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
							   ctx->armorline, ctx->armorlen);
				if (*error)
					break;
			}
		}

		/* FALLTHROUGH */
	case 305:
		ctx->state = 305;
 		/* Send end-annotation (if we sent a begin annotation) */
 		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
 			*error = sendAnnotate (ctx, myself, ctx->annotation, NULL, 0);
 			if (*error)
 				break;
 			ctx->annotation = 0;
 		}
		/* See if BEGIN line has a header line at the end */
		ptr = ctx->armorline + sizeof (prefix1) - 1 + sizeof (signedmsg) - 1;
		if (!memcmp (ptr, ", ", 2)) {
			/* Treat remainder of begin line as the first header line */
			ptr += 2;
			ctx->armorlen -= ptr - ctx->armorline;
			memcpy (ctx->armorline, ptr, ctx->armorlen);
			goto state_cleartext_header;
		}
		/* Else skip rest of begin line */

		/* FALLTHROUGH */
	case 310:
state_cleartext_header_loop:
		ctx->state = 310;
		/* Read until EOL == 1 to get to the end of the begin line */
		pgpAssert (ctx->eol);

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 320:
		ctx->state = 320;
		/* read the line after the begin: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 325:
		ctx->state = 325;
		/* Send annotation of raw data if in passthrough mode */
 		if (ctx->passthrough) {
 			*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
						   ctx->armorline, ctx->armorlen);
 			if (*error)
 				break;
 		}

		/* FALLTHROUGH */
	case 330:
state_cleartext_header:
		ctx->state = 330;
		/* Make sure first line after "BEGIN" is empty or "Hash:" */
		lineused = ctx->armorlen;
		ptr = ctx->armorline;

		/* Find trailing white space... */
		while (lineused && isspace (ptr[lineused - 1]))
			lineused--;

		if (lineused && !ctx->hashlen) {
			/* The line MUST be of the form:
			 *	Hash: <ID>[, <ID>]*
			 */
			/* Check for "Hash:" here, put hashes into
			 * ctx->hashlist and number of hashes into
			 * ctx->hashlen
			 */
			if (memcmp (ctx->armorline, "Hash: ", 6)) {
				/* Not a Hash -- Error out */
				/* XXX: Should annotate here */
				goto state_nonpgp;
			}
			lineused -= 6;
			ptr += 6;

			while (lineused) {
				PGPHashVTBL const *h;
				PGPByte const *comma = (PGPByte const *)
					strchr ((char const *) ptr, ',');
				unsigned len;

				if( comma - ptr > lineused )
					comma = NULL;
				len = comma ? comma-ptr : lineused;
				h = pgpHashByName ((char const *)ptr, len);
				if (!h) {
					/* unknown hash.. now what? */
					/*XXX: annotate?*/;
				} else
					ctx->hashlist[ctx->hashlen++] =
						h->algorithm;
				ptr += len;
				lineused -= len;
				if (comma) {
					if (memcmp (comma, ", ", 2)) {
						/* invalid format */
						/* XXX: annotate here */;
					}
					ptr += 2;
					lineused -= 2;
				}
			} /* while */

			if (!ctx->eol) {
				/* Too long a line */
				/* XXX: annotate: bad clearsig */
				ctx->hashlen = 0;
				goto state_nonpgp;
			}

			/* Now we need to get an *extra* line, the separator */
			goto state_cleartext_header_loop;
		} /* if (lineused && !ctx->hashlen) */

		if (!ctx->eol || lineused) {
			/*
			 * We either have a long line or non
			 * white-space.  I bet someone's trying to
			 * trick us into believeing this is a real
			 * clearsigned message, but they've added some
			 * text to it.  I dont buy it.  Output as
			 * non-pgp text; goto state nonpgp.
			 */

			/* XXX: Should send an annotation: bad clearsig */
			ctx->hashlen = 0;
			goto state_nonpgp;
		}
		/* FALLTHOUGH */
	case 340:
#if MIMEPARSE
state_cleartext_body:
#endif
		ctx->state = 340;
		/* Read until EOL == 1 to get to the next line */
		pgpAssert (ctx->eol);

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		ctx->saved_crlf = 0;
		ctx->crlf = kPGPLineEnd_Default;
		/* FALLTHROUGH */
	case 350:
		ctx->state = 350;
		/* Ok, this is a clearsigned message.  Send a new
		 * begin annotation
		 */
		*error = sendAnnotate (ctx, myself, PGPANN_CLEARSIG_BEGIN,
 					       (PGPByte *)&ctx->sectoff, sizeof(ctx->sectoff));
		if (*error)
			break;
		ctx->annotation = PGPANN_CLEARSIG_END;
		ctx->depth_at_ann = ctx->scope_depth;
		/* FALLTHROUGH */
	case 360:
		ctx->state = 360;
		/* Get a new message part */
		ctx->thispart = 1;
		ctx->maxparts = 1;
		msg = getMessage (ctx, NULL, 1);
		if (!msg) {
			/* This is an error */
			*error = kPGPError_OutOfMemory;
			break;
		}
		part = getPart (ctx, msg, ctx->thispart);
		if (!part) {
			/* This is an error too */
			*error = kPGPError_OutOfMemory;
			break;
		}

		/* And set it up as a clearsigned message */
		*error = createClearsig ( myself->cdkContext, ctx, msg, part);
		if (*error)
			break;

		ctx->part = part;
		/* FALLTHROUGH */

		/*
		 * Below starts the main processing loop for
		 * clearsigned messages.  It reads in a line.  If it
		 * is a BEGIN line it converts to the signature output
		 * and starts de-armoring the signature.  Otherwise it
		 * will output any saved CRLF chars, then output this
		 * new line, saving (but not outputting) the CRLF
		 * chars at the end of the line.  Then the loop
		 * continues.
		 */
	case 390:
state_cleartext_body_loop:
		ctx->state = 390;
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 395:
		ctx->state = 395;
		/* Send annotation of raw data if in passthrough mode */
 		if (ctx->passthrough) {
 			*error = sendAnnotate (ctx, myself, PGPANN_CLEARDATA,
						   ctx->armorline, ctx->armorlen);
 			if (*error)
 				break;
 		}

		/* FALLTHROUGH */
	case 400:
		ctx->state = 400;
		/* 
		 * Check if this is a begin line.  If so, start
		 * parsing the signature by jumping to case 9.
		 * Otherwise, if the line begins with '- ', move
		 * beyond that and fallthrough
		 */

		lineused = ctx->armorlen;
		ptr = ctx->armorline;
		
		while (lineused && isspace (ptr[lineused - 1]))
			lineused--;
		
#if MIMEPARSE
		if (ctx->mime_signed) {
			if (lineused == ctx->mime_bound_len &&
				!memcmp (ptr, ctx->mime_boundary, ctx->mime_bound_len)) {
				/* Done with MIME body part, go look for sig */
				goto state_pgpmime_clearsig;
			}
		} else {
#endif
		if (lineused == sizeof (sigprefix) - 1 &&
		    !memcmp (ptr, sigprefix, sizeof (sigprefix) - 1)) {
			goto state_pgp_begin;
		}

		if (!memcmp (ctx->armorline, "- ", 2)) {
			ctx->armorptr = ctx->armorline + 2;
			ctx->armorlen -= 2;
		}
#if MIMEPARSE
		}
#endif
		/* FALLTHROUGH */
	case 410:
		ctx->state = 410;
		/* output CRLF if we need to */

		while (ctx->saved_crlf) {

			if (ctx->saved_crlf & kPGPLineEnd_CR) {
				i = writeExtraData (ctx, (PGPByte const *)"\r",
						    1, error);
				if (i)
					ctx->saved_crlf &= ~kPGPLineEnd_CR;
				if (*error)
					break;
			}

			if (ctx->saved_crlf & kPGPLineEnd_LF) {
				i = writeExtraData (ctx, (PGPByte const *)"\n",
						    1, error);
				if (i)
					ctx->saved_crlf &= ~kPGPLineEnd_LF;
				if (*error)
					break;
			}
		}

		/* FALLTHROUGH */
	case 420:
state_cleartext_body_output:
		ctx->state = 420;
		/*
		 * output armorline buffer; do not output crlf until
		 * the next line has been shown to not be the
		 * delimiter.
		 */
		ptr = ctx->armorptr + ctx->armorlen - 1;
		while (*ptr == '\r' || *ptr == '\n') {
			ptr--;
			ctx->armorlen--;
		}

		while (ctx->armorlen) {
			retval = writeExtraData (ctx, ctx->armorptr,
						 ctx->armorlen, error);
			ctx->armorptr += retval;
			ctx->armorlen -= retval;
			if (*error)
				return written;
		}
		ctx->armorptr = ctx->armorline;
		/* FALLTHROUGH */
	case 430:
		ctx->state = 430;
		/*
		 * if EOL == 1, clear state and goto state body_loop, otherwise
		 * read data and return to state body_output to output the line.
		 */
		if (ctx->eol == 1) {
			ctx->eol = 0;
			ctx->eob = 0;
			ctx->saved_crlf = ctx->crlf;
			ctx->crlf = kPGPLineEnd_Default;
			goto state_cleartext_body_loop;
		}

		if (!size)
			break;

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		goto state_cleartext_body_output;

		/* BEGIN BINARY MESSAGE HERE */
	case 500:
state_pgp_binary:
		ctx->state = 500;
		/* Send a new begin annotation */
		*error = sendAnnotate (ctx, myself, PGPANN_INPUT_BEGIN,
				       NULL, 0);
		if (*error)
			break;
		ctx->annotation = PGPANN_INPUT_END;
		ctx->depth_at_ann = ctx->scope_depth;
		/* FALLTHROUGH */
	case 510:
		ctx->state = 510;
		/* Get a new message part */
		pgpAssert (!ctx->part);
		ctx->thispart = 1;
		ctx->maxparts = 1;
		msg = getMessage (ctx, NULL, 1);
		if (!msg) {
			/* This is an error */
			*error = kPGPError_OutOfMemory;
			break;
		}
		part = getPart (ctx, msg, ctx->thispart);
		if (!part) {
			/* This is an error too */
			*error = kPGPError_OutOfMemory;
			break;
		}

		*error = writeMessage (ctx, msg);
		if (*error)
			break;
		ctx->part = part;
		/* FALLTHROUGH */
	case 520:
		ctx->state = 520;
		/* Write out data cached in ctx->armorline */
		part = ctx->part;
		while (ctx->armorlen) {
			retval = writePartData (part, ctx->armorptr,
						ctx->armorlen, error);
			ctx->armorptr += retval;
			ctx->armorlen -= retval;
			if (*error)
				return written;
		}
		ctx->armorptr = ctx->armorline;
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 530:
		ctx->state = 530;
		/*
		 * Check if this scope has ended.  If so, then close
		 * this part by jumping to state pgp_finish
		 */
		if (!ctx->annotation) {
			goto state_pgp_finish;
		}

		/* Otherwise, write out data from this write call. */
		part = ctx->part;
		while (size) {
			retval = writePartData (part, buf, size, error);
			buf += retval;
			size -= retval;
			written += retval;
			if (*error)
				return written;
		}

		/* Can we get here any way other than not having data? */
		pgpAssert (!size);
		break;

#if MIMEPARSE
	case 600:
state_pgpmime:
		ctx->state = 600;
		/*
		 * Here on a MIME multipart/signed message.  Must check protocol
		 * and boundary.
		 */
		/* Case sensitive OK? */
		if ((ptr = (PGPByte *)
			strstr ((char *)ctx->armorline, prefix_mime2)) != NULL) {
			/* Have protocol, we know it's ours */
			ctx->mime_signed = 1;
		}

		/* Case sensitive OK? */
		if ((ptr = (PGPByte *)strstr
			((char *)ctx->armorline, prefix_mime3)) != NULL) {
			/* Get boundary */
			int need_quote;
			ptr += strlen (prefix_mime3);
			need_quote = (*ptr == '"');
			if (need_quote)
				++ptr;
			ptr2 = (PGPByte *)strchr ((char *)ptr, need_quote?'"':';');
			if (!ptr2) {
				/* Header too complex for us to parse, rely on heuristic */
				ctx->mime_signed = 0;
				ctx->mime_bound_len = 0;
				goto state_nonpgp;
			}
			if (ptr2-ptr+5 > (int)sizeof(ctx->mime_boundary)) {
				/* Too big boundary, can't handle it */
				goto state_nonpgp;
			}
			ctx->mime_boundary[0] = '-';
			ctx->mime_boundary[1] = '-';
			strncpy ((char *)ctx->mime_boundary+2, (char *)ptr, ptr2-ptr);
			ctx->mime_bound_len = ptr2 - ptr + 2;
			ctx->mime_boundary[ctx->mime_bound_len] = '\0';
		}

#if 0
		/*
		 * This micalg finding code may not get used if boundary and proto
		 * are on a line before the micalg.  Also won't work with comma
		 * separated micalgs.
		 * Need to improve this, however
		 * it is just a speed optimization to have a micalg anyway.
		 */
		ptr = ctx->armorline;
		while ((ptr = strstr (ctx->armorline, prefix_mime4)) != NULL) {
			/* Have mic_alg */
			PGPHashVTBL const *h;
			ptr += strlen(prefix_mime4);
			if (strncmp (ptr, prefix_mime5, strlen(prefix_mime5)) != 0)
				continue;		/* Not a PGP hash */
			ptr += strlen(prefix_mime5);
			ptr2 = strchr (ptr, ';');
			if (!ptr2)
				break;			/* No terminator, ignore it? */
			for (i=0; i<ptr2-ptr; ++i) {
				ptr[i] = toupper(ptr[i]);
			}
			h = pgpHashByName ((char const *)ptr, ptr2-ptr);
			if (h) {
				ctx->hashlist[ctx->hashlen++] = h->type;
			}
			ptr = ptr2;
		}
#endif

		if (ctx->mime_signed && ctx->mime_bound_len) {
			/* Got the info we need, now go look for first boundary */
			goto state_pgpmime_initial_boundary;
		}

		/* FALLTHROUGH */
	case 610:
		ctx->state = 610;
		/* Read until EOL == 1 to get to the end of the header line */
		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 620:
		ctx->state = 620;
		/* read the line after the begin: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 630:
		ctx->state = 630;
		/*
		 * See if a continuation of previous header line.  Must start with
		 * whitespace followed by non-whitespace.
		 */
		if (ctx->armorlen > 1 && isspace (ctx->armorline[0])) {
			for (i=1; i< (int)ctx->armorlen; ++i) {
				if (!isspace (ctx->armorline[i])) {
					goto state_pgpmime;
				}
			}
		}
		/*
		 * Here we had a mime-signed header, but not our protocol or boundary.
		 * Maybe another flavor of mime.  We will go back to our initial
		 * state.  This means that we will have eaten one or more header
		 * lines.
		 */
		ctx->mime_signed = 0;
		ctx->mime_bound_len = 0;
		goto state_nonpgp;

	case 650:
state_pgpmime_initial_boundary:
		ctx->state = 650;
		/*
		 * Here when we have a good PGP/MIME signed header, with boundary.
		 * Look for that boundary.
		 * Read until EOL == 1 to get to the end of the header line.
		 */
		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 660:
		ctx->state = 660;
		/* read the line after the begin: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 670:
		ctx->state = 670;
		if (ctx->armorlen < (unsigned)ctx->mime_bound_len ||
			!ctx->eol ||
		   memcmp (ctx->armorline, ctx->mime_boundary, ctx->mime_bound_len)) {
			/* Not yet at boundary for first part */
			goto state_pgpmime_initial_boundary;
		}
		/* FALLTHROUGH */
	case 680:
state_pgpmime_to_cleartext:
		ctx->state = 680;
 		/* Send end-annotation (if we sent a begin annotation) */
 		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
 			*error = sendAnnotate (ctx, myself, ctx->annotation,
 					       NULL, 0);
 			if (*error)
 				break;
 			ctx->annotation = 0;
 		}
		/* Here at first boundary, fall into common clearsign code above */
		goto state_cleartext_body;

	case 700:
state_pgpmime_body_noheaders:
		ctx->state = 700;
		/* Here if we see a "--" line while parsing, may be mime boundary */
		pgpAssert (ctx->eol);
		strcpy ((char *)ctx->mime_boundary, (char *)ctx->armorline);
		ctx->mime_bound_len = strlen ((char *)ctx->mime_boundary);
		/* FALLTHROUGH */
	case 710:
		ctx->state = 710;
		/* read the line after the boundary: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;
	case 720:
		ctx->state = 720;
		/* MUST be a MIME header, or we are lost */
		if (!ctx->eol || ctx->armorlen < 4 || isspace(ctx->armorline[0]) ||
		   !(ptr = (PGPByte *)
		   		strchr ((char *)ctx->armorline, ':')) || !isspace(ptr[1])) {
			/*
			 * If no good, we go back to our initial state.  This means we
			 * will have eaten the boundary line.
			 */
			goto state_nonpgp;
		}

		/* Valid mime header, go on and begin processing message */
		ctx->mime_signed = 1;

		/*
		 * Fall into common clearsigned code above.  Flags we have set will
		 * change its state-leaving behavior.
		 */
		goto state_pgpmime_to_cleartext;

	case 750:
state_pgpmime_clearsig:
		ctx->state = 750;
		/*
		 * Here when we have seen the boundary at the end of the clearsign.
		 * Must wait for signed part.
		 * Read until EOL == 1 to get to the end of the boundary line.
		 */
		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 760:
		ctx->state = 760;
		/* read the next line: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 770:
		ctx->state = 770;
		/* See if have BEGIN PGP SIGNATURE line, if so go process it */
		if ((ctx->armorlen > sizeof (sigprefix) - 1 &&
		    !memcmp (ctx->armorline, sigprefix, sizeof (sigprefix) - 1)) ||
			(ctx->armorlen > sizeof (prefix_mimesig2) - 1 &&
		    !memcmp (ctx->armorline, prefix_mimesig2,
					 sizeof (prefix_mimesig2) - 1))) {
			/* Have BEGIN line for signature */
			goto state_pgp_begin;
		}

		/* Else loop back */
		goto state_pgpmime_clearsig;

	case 800:
state_pgpmime_sig_end:
		ctx->state = 800;
		/*
		 * Here after -----END PGP MESSAGE----- in a mime signed message.
		 * Look for terminal boundary line.
		 *
		 * Read until EOL == 1 to get to the end of the header line.
		 * Append two '-' chars to boundary to look like final boundary
		 */
		ctx->mime_boundary [ctx->mime_bound_len++] = '-';
		ctx->mime_boundary [ctx->mime_bound_len++] = '-';
		ctx->mime_boundary [ctx->mime_bound_len] = '\0';
		/* FALLTHROUGH */
	case 810:
state_pgpmime_sig_end_loop:
		ctx->state = 810;
		/* Read till end of previous line */
		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 0;
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->armorlen = 0;
		/* FALLTHROUGH */
	case 820:
		ctx->state = 820;
		/* read the line after the begin: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			;
		else
			break;

		/* FALLTHROUGH */
	case 830:
		ctx->state = 830;
		/* Check for final boundary */
		if (ctx->eol && ctx->armorlen >= (unsigned)ctx->mime_bound_len &&
		  !memcmp (ctx->armorline, ctx->mime_boundary, ctx->mime_bound_len)) {
			/* Clean up mime flags, go to clean-up state */
			ctx->mime_signed = 0;
			ctx->mime_bound_len = 0;
			goto state_pgp_finish;
		}
		
		/* Keep looping until we see it */
		goto state_pgpmime_sig_end_loop;

#endif

	default:
		/* Every state should be enumerated above */
		pgpAssert (0);
	}

	return written;
}

static PGPError
Flush (PGPPipeline *myself)
{
	PrsAscContext *context;
	PGPError	error = kPGPError_NoErr;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (PrsAscContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	(void) Write (myself, NULL, 0, &error);
	if (error)
		return error;

	return context->tail->flush (context->tail);
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	PrsAscContext *context;
	PGPError	error = kPGPError_NoErr;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (PrsAscContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (bytes)
		return( kPGPError_NoErr );

	/* There are no more bytes.  Therefore this is the EOL. */
	context->eol = 1;

	/* Call write to purge out anything we might have buffered. */
	(void) Write (myself, NULL, 0, &error);
	if (error)
		return error;

	if (context->part) {
		/*
		 * If we have a part, end it, flush the current
		 * message, and reset the context state.
		 */

		/*
		 * Check if this is a clearsigned message that is
		 * incomplete.  If it is, kill it now!
		 */
		if (context->part->sig && !context->part->msg->foundpart1)
			(void)0;
		
		/* Set to finish state */
		context->state = 260;
		(void) Write (myself, NULL, 0, &error);
		if (error)
			return error;
	}

	/*
	 * If we have data buffered at this point, we return an error
	 * code to let the caller know that.  If we miss a part of a
	 * multipart message, this will be the case.  The user will
	 * receive the error message and can then send annotations to
	 * figure out what is missing and possible send it or at least
	 * inform the user.
	 *
	 * This keeps the data consisent and doesn't propagate a
	 * sizeAdvise() in the wrong scope.
	 */

	if (context->scope_depth)
		return( kPGPError_NoErr );

	if (context->msgs)
		return kPGPError_AsciiParseIncomplete;

	/* Bytes is zero */
	return context->tail->sizeAdvise (context->tail, bytes); 
}

static Message *
getMessageNumber (PrsAscContext *ctx, int num)
{
	Message *msg;

	msg = ctx->msgs;
	while (msg) {
		if (msg->msg_number == num)
			return msg;
		msg = msg->next;
	}
	return NULL;
}

static int
sendStatus (PGPPipeline *myself, int type, PGPByte const *string,
	    size_t size)
{
	PrsAscContext *ctx = (PrsAscContext *)myself->priv;
	Message *msg;
	MsgPart *part;
	int count;

	(void)size;		/* Avoid warning */
	switch (type) {
	case PGPANN_PARSEASC_MSG_COUNT:
		return ctx->msg_count;
	case PGPANN_PARSEASC_MSG_LEFT:
		msg = ctx->msgs;
		count = 0;
		while (msg) {
			count++;
			msg = msg->next;
		}
		return count;
	case PGPANN_PARSEASC_MSG_CURRENT:
		/* Return the pointer to the currently writing message */
		msg = ctx->msgs;
		while (msg) {
			if (msg->writing)
				break;
			msg = msg->next;
		}
		if (msg)
			return msg->msg_number;
		return 0;
	case PGPANN_PARSEASC_MSG_PARTS:
		/* Return the number of parts in the message */
		pgpAssert (size == sizeof (int));
		msg = getMessageNumber (ctx, ((int *) string)[0]);
		if (!msg)
			return 0;
		return msg->size;
	case PGPANN_PARSEASC_MSG_PARTINFO:
		/* Return information about part M of message N */
		pgpAssert (size == (2 * sizeof (int)));
		msg = getMessageNumber (ctx, ((int *) string)[0]);
		if (!msg)
			return 0;
		count = ((int *) string)[1];
		part = msg->parts;
		while (part) {
			if (part->num == (unsigned)count)
				return (part->done ? 1 : 2);
			if (part->num > (unsigned)count)
				return 1;
			part = part->next;
		}
		pgpAssert (!part);
		return 2;
	}
	return 0;
}

/* Send an annotation */
static PGPError
sendAnnotate (PrsAscContext *ctx, PGPPipeline *origin, int type,
	      PGPByte const *string, size_t size)
{
	pgpAssert (ctx->tail);

	/* Buffer annotation if buffering flag is true, except we will pass
	 * CLEARDATA through if we are writing this part.
	 */
	if (ctx->buffering && ((type != PGPANN_CLEARDATA) ||
						   (ctx->part == NULL) ||
						   !ctx->part->msg->writing))
		return bufferAnnotation (ctx, origin, type, string, size);

	return ctx->tail->annotate (ctx->tail, origin, type, string, size);
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	PrsAscContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (PrsAscContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	switch (type) {
	case PGPANN_PARSEASC_MSG_COUNT:
	case PGPANN_PARSEASC_MSG_LEFT:
	case PGPANN_PARSEASC_MSG_CURRENT:
	case PGPANN_PARSEASC_MSG_PARTS:
	case PGPANN_PARSEASC_MSG_PARTINFO:
		return (PGPError)sendStatus (myself, type, string, size);
	/* Enable/disable passthrough modes */
	case PGPANN_PASSTHROUGH_CLEARSIGN:
		context->passthroughcleartext = *(PGPBoolean *)string;
		break;
	case PGPANN_PASSTHROUGH_KEYS:
		context->passthroughkey = *(PGPBoolean *)string;
		break;
	}

	if (PGP_IS_END_SCOPE (type) && context->annotation &&
	    context->scope_depth == context->depth_at_ann) {
		error = sendAnnotate (context, myself, context->annotation,
				      NULL, 0);
		if (error)
			return error;
		context->annotation = 0;
	}

	error = sendAnnotate (context, origin, type, string, size);
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE (context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

static PGPError
Teardown (PGPPipeline *myself)
{
	PrsAscContext *context;
	Message *msg, *temp;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (PrsAscContext *)myself->priv;
	pgpAssert (context);

	for (msg = context->msgs; msg; msg = temp) {
		temp = msg->next;
		freeMessage (context, msg);
	}

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpFifoDestroy (context->fifod, context->data);
	pgpFifoDestroy (context->fifod, context->ann);

	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

	PGPPipeline **
pgpParseAscCreate  (
	PGPContextRef		cdkContext,
	PGPPipeline **		head,
	PGPEnv const *		env,
	PGPFifoDesc const *	fd,
	PGPUICb const *		ui,
	void *				ui_arg)
{
	PGPPipeline *mod;
	PrsAscContext *context;
	PGPFifoContext *data, *ann;

	if (!head || !env)
		return NULL;

	pgpParseAscInit ();

	context = (PrsAscContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear);
	if (!context)
		return NULL;
	
	mod = &context->pipe;
	mod->cdkContext	= cdkContext;
	
	data = pgpFifoCreate (cdkContext, fd);
	if (!data) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	ann = pgpFifoCreate (cdkContext, fd);
	if (!ann) {
		pgpFifoDestroy (fd, data);
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}

	mod->magic = DEARMORMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Ascii Armor Parser Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->fifod = fd;
	context->data = data;
	context->ann = ann;
	context->dataptr = context->databuf;
	context->armorptr = context->armorline;
	context->myself = mod;
	context->env = env;
	context->ui = ui;
	context->ui_arg = ui_arg;
#if MIMEPARSE
	context->no_mime_headers = pgpenvGetInt (env, PGPENV_PGPMIMEPARSEBODY,
			NULL, NULL);
#endif

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
