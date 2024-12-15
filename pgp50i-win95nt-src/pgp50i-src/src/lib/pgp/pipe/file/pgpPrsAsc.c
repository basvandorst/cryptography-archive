/*
 * pgpPrsAsc.c -- ascii dearmor parser.  This is fairly complicated.  Read
 * 	the comment a page down for a description of how this system works.
 *	The nice part is that you can input multipart armor in any order
 *	and it will work.  In fact, you can intersperse armor parts from
 *	multiple messages and it will work.  Moreover, you can input binary
 *	PGP messages and it will still work!  My god, can this man think
 *	of everything or what?
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsAsc.c,v 1.13.2.3 1997/06/07 09:50:57 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* [Hal] I am working on adding PGP/MIME parsing to this. */
#ifndef MIMEPARSE
#define MIMEPARSE		0
#endif

#include "pgpDebug.h"
#include "pgpCharMap.h"		/* for charMapIdentity */
#include "pgpCopyMod.h"
#include "pgpCRC.h"
#include "pgpPrsAsc.h"
#include "pgpRadix64.h"
#include "pgpAnnotate.h"
#if MIMEPARSE
#include "pgpDeMimeMod.h"
#endif
#include "pgpFIFO.h"
#include "pgpFileType.h"
#include "pgpHash.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpPrsBin.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpPipeline.h"
#include "pgpSplit.h"
#include "pgpTextFilt.h"
#include "pgpUsuals.h"
#include "pgpVerifyRa.h"

/*
 * This is a complicated piece of code.  Less politely, this code is a
 * bitch.  It needs to be able to keep track of many different things
 * at the same time in order to combine pieces of armor into whole messages.
 *
 * There are three entities defined here. First is an armor part, which
 * is defined as the range from BEGIN PGP MESSAGE to the END. Second is
 * a message, which is PGP message made up of one or more parts.
 * And third, there are files, which can contain parts.
 *
 * Parts must be fully contained within a file, however there can be
 * multiple parts in a single file. On the other hand, messages can
 * cross file boundaries.
 *
 * This code will output PGP messages in a stream of input "files"
 * wherever the first part of the message came in the stream. In a
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
 * A,B,C are 3 PGP messages. The numbers are the actual multipart parts
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
 * part 1 is located. Other parts are replaced with an annotation that just
 * notes the deletion.
 *
 * While decryting message A, all the data between A1 and A3 (including
 * annotations) needs to be buffered. In the usual case where the text
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

struct Message;			/* forward reference */

struct MsgPart {
	unsigned num;		/* Which part number is this? */
	struct PgpPipeline *mod; /* Pointer to the join module for this part */
	struct PgpPipeline *text; /* clearsigned: the text goes here */
	struct PgpPipeline *sig; /* clearsigned: the sig goes here */
	struct Message *msg;	/* What message does this belong to? */
	struct MsgPart *next;	/* Pointer to the next part */
	byte first;		/* Is this the first section left to do? */
	byte done;		/* Is this part complete? */
};

struct Message {
	char *name;		/* The name of this message */
	unsigned size;		/* How many parts does this have, if known */
	struct PgpPipeline **tail; /* Tail pointer for this message */
	struct MsgPart *parts;	/* The message parts */
	struct Message *next;	/* The next message */
	int msg_number;		/* What is this message number */
	byte writing;		/* Are we writing? */
	byte foundpart1;	/* Did we find part 1? */
};

struct Context {
	struct PgpFifoDesc const *fifod;	/* Fifo Descriptor */
	struct PgpFifoContext *ann; /* Fifo to hold the saved annotations */
	struct PgpFifoContext *data; /* Fifo to hold saved non-PGP data */
	struct Message *msgs;	/* List of current messages */
	struct MsgPart *part;	/* The current message part */
	struct PgpPipeline *myself; /* Pointer to this pipeline module */
	struct PgpPipeline *tail; /* Tail pointer for rest of the pipeline */
	struct PgpUICb const *ui;
	struct PgpEnv const *env;
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
	byte databuf[51];	/* Space to hold raw data 48+slush */
	byte *dataptr;		/* Pointer into the data buffer */
	unsigned datalen;	/* length of data buffer */
	byte armorline[LINE_LEN]; /* line of armor */
	byte *armorptr;		/* pointer into armorline */
	unsigned armorlen;	/* length of line */
	unsigned long crc;	/* CRC Checksum */
	byte hashlist[255];	/* List of hashes */
	byte hashlen;		/* Length of the list of hashes */
	byte cmdarg[kPGPPrsAscCmdBufSize];	/* buffered command */
	unsigned cmdlen;	/* length of buffered command */
	int msg_count;		/* A count of the number of messages */
	byte command;		/* the command that is buffered */
	byte buffering;		/* A flag -- are we buffering everything? */
	byte eol;		/* Our readLine EOL flag */
	byte eob;		/* End of Buffer flag (from readLine) */
	byte expectcrc;		/* Are we expecting the CRC line next? */
	byte crlf;		/* The type of crlf we have */
	byte saved_crlf;	/* Save it off... */
	byte firstheader;	/* True if looking for 1st header line */
	byte doublespace;	/* True if every other line is blank */
#if MIMEPARSE
	byte mime_signed;	/* Dealing with a PGP/MIME clearsigned message */
	byte no_mime_headers;		/* Don't have mime headers, look in body */
	byte mime_boundary[256];	/* MIME boundary with "--" */
	int mime_bound_len;	/* strlen(mime_boundary) */
#endif
};

static char dearmorTable[256];
static int inited = 0;

/* forward references */
static int writeMessage (struct Context *ctx, struct Message *msg);
static int sendAnnotate (struct Context *ctx, struct PgpPipeline *origin,
			int type, byte const *string, size_t size);
static int flushMessage (struct Context *ctx, struct Message *msg);

/*
 * Close this part.  This makes sure that this message is being written
 * before it does this.  It will call a sizeAdvise (0) on the join
 * module and then possibly a teardown, too.
 */
static int
closePart (struct MsgPart *part)
{
	int error;

	if (! part->msg->writing)
		return 0;

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
	return 0;
}

/*
 * Free a message part; this tears down the join module, which must be
 * unhooked from the pipeline first.
 */
static void
freePart (struct MsgPart *part)
{
	struct MsgPart **partp = &part->msg->parts;

	if (part->mod)
		part->mod->teardown (part->mod);
	if (part->text)
		part->text->teardown (part->text);
	if (part->sig)
		part->sig->teardown (part->sig);

	while (*partp) {
		if (*partp == part) {
			*partp = part->next;
			memset (part, 0, sizeof (*part));
			pgpMemFree (part);
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
static int
donePart (struct Context *ctx)
{
	struct MsgPart *part = ctx->part;
	int error;

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
	return 0;
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
static struct MsgPart *
getPart (struct Context *ctx, struct Message *msg, unsigned num)
{
	struct MsgPart *temp = NULL, **part = &msg->parts;
	struct PgpPipeline *last = NULL;
	unsigned highest = 0;

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
		temp = (struct MsgPart *)pgpMemAlloc (sizeof (*temp));
		if (!temp)
			return NULL;

		memset (temp, 0, sizeof (*temp));
		temp->num = highest;
		temp->msg = msg;

		if (last) {
			temp->mod = pgpJoinAppend (last);
		} else {
			msg->tail = pgpJoinCreate (&(temp->mod), ctx->fifod);
			temp->first = 1;
			msg->tail = pgpParseBinCreate (msg->tail, ctx->env);
			/*
			 * Binary parser may add modules after itself, and in some
			 * error recovery situations they are still there when we try
			 * to close things.  Create a copy module to act as a stable
			 * predecessor to context->tail.
			 */
			msg->tail = pgpCopyModCreate (msg->tail);

			/* connect the join module to the rest of the pipe */
			if (msg->tail)
				*(msg->tail) = ctx->tail;
		}

		if (!temp->mod) {
			pgpMemFree (temp);
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
 * going of any particular size (number of parts). I don't consider
 * this a big problem, since interleaved messages are rarely a
 * problem.  Out-of-order messages, on the other hand.....
 */
static struct Message *
getMessage (struct Context *ctx, char const *name, unsigned size)
{
	struct Message **msg = &ctx->msgs;
	char *newname;

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

	*msg = (struct Message *)pgpMemAlloc (sizeof (**msg));
	if (! *msg)
		return NULL;

	memset (*msg, 0, sizeof (**msg));
	if (name) {
		newname = (char *)pgpMemAlloc (strlen (name) + 1);
		if (!newname) {
			pgpMemFree (*msg);
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
freeMessage (struct Context *ctx, struct Message *msg)
{
	struct Message **msgp = &ctx->msgs;
	struct MsgPart *part, *temp;

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
				pgpMemFree (msg->name);
			ctx->buffering = 0;
			memset (msg, 0, sizeof (*msg));
			pgpMemFree (msg);

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
static int
bufferCommand (struct Context *ctx, byte cmd, const byte *arg, unsigned arglen)
{
	/* Make sure the command argument fits the buffer size */
	if (arglen > sizeof (ctx->cmdarg))
		return PGPERR_CMD_TOOBIG;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, (byte const *)&ctx->written,
			sizeof (ctx->written)) != sizeof (ctx->written))
		return PGPERR_NOMEM;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, &cmd, sizeof (cmd))
		!= sizeof (cmd))
		return PGPERR_NOMEM;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, (byte const *)&arglen,
			  sizeof (arglen)) != sizeof (arglen))
		return PGPERR_NOMEM;

	if (pgpFifoWrite (ctx->fifod, ctx->ann, arg, arglen) != arglen)
		return PGPERR_NOMEM;


	ctx->written = 0;
	return 0;
}

/* Buffer up a write command for a particular message */
static int
bufferWriteCommand (struct Context *ctx, struct Message *msg)
{
	byte arg[sizeof (msg)];
	memcpy (arg, (byte const *)&msg, sizeof (arg));

	return bufferCommand (ctx, CMD_WRITE, arg, sizeof (arg));
}

/* Process a buffered Write command for a particular message */
static int
parseWriteCommand (struct Context *ctx)
{
	struct Message *msg;

	pgpAssert (ctx->cmdlen == sizeof (msg));
	memcpy ((byte *)&msg, ctx->cmdarg, sizeof (msg));
	return writeMessage (ctx, msg);
}

/* Buffer up an annotation */
static int
bufferAnnotation (struct Context *ctx, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
{
	byte *arg;
	byte *argp;
	int retval;

	arg = argp = (byte *)pgpMemAlloc (sizeof (origin) + sizeof (type) +
					  size + sizeof (size));
	if (!arg)
		return PGPERR_NOMEM;

	memcpy (argp, (byte const *)&origin, sizeof (origin));
	argp += sizeof (origin);

	memcpy (argp, (byte const *)&type, sizeof (type));
	argp += sizeof (type);

	memcpy (argp, (byte const *)&size, sizeof (size));
	argp += sizeof (size);

	memcpy (argp, string, size);
	argp += size;

	retval = bufferCommand (ctx, CMD_ANNOTATE, arg, argp-arg);
	pgpMemFree (arg);
	return retval;
}

static int
parseAnnotation (struct Context *ctx)
{
	struct PgpPipeline *origin;
	int type;
	size_t size;
	byte *string, *argp = ctx->cmdarg;
	int retval;

	pgpAssert (ctx->cmdlen >= sizeof (origin) + sizeof (type) +
		sizeof (size));

	memcpy ((byte *)&origin, argp, sizeof (origin));
	argp += sizeof (origin);

	memcpy ((byte *)&type, argp, sizeof (type));
	argp += sizeof (type);

	memcpy ((byte *)&size, argp, sizeof (size));
	argp += sizeof (size);

	/* make sure we have the right number of bytes remaining */
	pgpAssert ((ctx->cmdlen - size) == (unsigned)(argp - ctx->cmdarg));

	string = (byte *)pgpAlloc(size);
	if (string == NULL)
		return PGPERR_NOMEM;

	pgpCopyMemory (argp, string, size);

	retval = sendAnnotate (ctx, origin, type, string, size);

	pgpFree(string);
	return retval;
}

/*
 * Given a buffered command, process it appropriately.  The cmd, args,
 * and arglen are currently held in the ctx.  Set arglen to 0 and
 * return 0 on success, otherwise return an error.
 */
static int
processCommand (struct Context *ctx)
{
	int error = 0;

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
writePartData (struct MsgPart *part, byte const *buf, size_t size, int *error)
{
	struct Message *msg = part->msg;

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
writeExtraData (struct Context *ctx, byte const *buf, size_t size, int *error)
{
	size_t written;

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
 * This will start with the first part that we have. For each part,
 * it will see if it is done. If not, return 0. If it is done, then
 * see if there is another part sitting after it. If so, close this
 * part, return an error if one exists, or move on to the next part if
 * there was no error closing the part. If there is not another part
 * after this one, then only close this one if this is the last part
 * (msg->size && part->num == msg->size). Otherwise return 0.
 *
 * If we flushed the whole message, free it.
 */
static int
flushMessage (struct Context *ctx, struct Message *msg)
{
	struct MsgPart *temp, *part = msg->parts;
	int error = 0;

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
 * Set the state so that we are writing out this message. It means that
 * we are probably going to be buffering everything else until this message
 * is done. If we are already "buffering" everything, then buffer up this
 * command for later execution.
 */
static int
writeMessage (struct Context *ctx, struct Message *msg)
{
	int error;

	msg->foundpart1 = 1;
	if (ctx->buffering)
		return bufferWriteCommand (ctx, msg);

	error = ctx->tail->annotate (ctx->tail, ctx->myself,
				     PGPANN_ARMOR_BEGIN, NULL, 0);
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
static int
flushBuffers (struct Context *ctx)
{
	unsigned datalen, cmdlen, len;
	byte const *ptr;
	int error = 0;
	size_t written;

	pgpAssert (! ctx->buffering);

	datalen = pgpFifoSize (ctx->fifod, ctx->data);
	cmdlen = pgpFifoSize (ctx->fifod, ctx->ann);

	/* If nothing is buffered, just return ok */
	if (!datalen && !cmdlen)
		return 0;

	/* Now try to flush the buffers... */
	do {
		/*
		 * If buffering gets turned on processing a command,
		 * just exit this function so more data can be read in.
		 */
		if (ctx->buffering)
			return 0;

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
		 * Now, read in the data for the next command. The
		 * syntax is: <offset><cmd><arglen><arg>
		 */
		if (cmdlen) {
			if (pgpFifoRead (ctx->fifod, ctx->ann,
					(byte *)&ctx->left,
					sizeof (ctx->left)) !=
			    sizeof (ctx->left))
				return PGPERR_FIFO_READ;
			cmdlen -= sizeof (ctx->left);

			if (pgpFifoRead (ctx->fifod, ctx->ann, &ctx->command,
					 sizeof (ctx->command)) !=
			    sizeof (ctx->command))
				return PGPERR_FIFO_READ;
			cmdlen -= sizeof (ctx->command);

			if (pgpFifoRead (ctx->fifod, ctx->ann,
					 (byte *)&ctx->cmdlen,
					 sizeof (ctx->cmdlen)) !=
			    sizeof (ctx->cmdlen))
				return PGPERR_FIFO_READ;
			cmdlen -= sizeof (ctx->cmdlen);

			if (pgpFifoRead (ctx->fifod, ctx->ann, ctx->cmdarg,
					 ctx->cmdlen) != ctx->cmdlen)
				return PGPERR_FIFO_READ;
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

	return 0;
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
static int
createClearsig (struct Context *ctx, struct Message *msg, struct MsgPart *part)
{
	struct PgpPipeline *join = NULL, *split = NULL, *text = NULL;
	struct PgpPipeline **tail, **temp;
	struct PgpPipeline *sighead = NULL, **sigtail;

	/* First, build up a replacement "join/split" module */
	tail = pgpJoinCreate (&join, ctx->fifod);
	if (!tail)
		return PGPERR_NOMEM;

	temp = tail;
	tail = pgpSplitCreate (temp);
	if (!tail) {
		return PGPERR_NOMEM;
	}
	split = *temp;

	/* Splice in the join/split in place of the old join/parser */
	*(msg->tail) = NULL;
	part->mod->teardown (part->mod);

	part->mod = join;
	msg->tail = tail;

#if MIMEPARSE
	if (ctx->mime_signed) {
		/* Clean up MIME body part, remove quoted-printable etc. */
		msg->tail = pgpDeMimeModCreate (msg->tail);
	}
#endif

	/*
	 * Binary parser may add modules after itself, and in some
	 * error recovery situations they are still there when we try
	 * to close things. Create a copy module to act as a stable
	 * predecessor to context->tail.
	 */
	msg->tail = pgpCopyModCreate (msg->tail);

	/* And connect the tail */
	if (msg->tail)
		*(msg->tail) = ctx->tail;

	/* Create the text input */
	text = pgpJoinAppend (join);
	if (! text) {
		return PGPERR_NOMEM;
	}

	/* add a textfilt module */
	tail = pgpSplitAdd (split);
	if (!tail) {
		text->teardown (text);
		return PGPERR_NOMEM;
	}

#if MIMEPARSE
	tail = pgpTextFiltCreate (tail, charMapIdentity, 1,
			(ctx->mime_signed ? PGP_TEXTFILT_CRLF : PGP_TEXTFILT_NONE));
#else
	tail = pgpTextFiltCreate (tail, charMapIdentity, 1, PGP_TEXTFILT_NONE);
#endif
	if (!tail) {
		text->teardown (text);
		return PGPERR_NOMEM;
	}

	/* Create the signature parser... */
	sigtail = pgpParseBinCreate (&sighead, ctx->env);
	if (!sigtail) {
		text->teardown (text);
		return PGPERR_NOMEM;
	}

	/* ...and the signature verifier */
	if (!pgpVerifyReaderCreate (tail, sigtail, ctx->env, ctx->fifod,
				    (ctx->hashlen ? ctx->hashlist : NULL),
				    ctx->hashlen, (ctx->hashlen ? 1 : 0),
				    ctx->ui, ctx->ui_arg)) {
		text->teardown (text);
		sighead->teardown (sighead);
		return PGPERR_NOMEM;
	}

	/* Now put it all together in this message */
	part->text = text;
	part->sig = sighead;

	return 0;
}

/*
 * Build up the Ascii Parser Table in order to dearmor data.  For all
 * values, set it to -1 if it is not a valid armor character and then
 * set the valid characters out of the armorTable.  Inited == 0 implies
 * we need to init; Inited == 1 implies we are processing or done.
 */
static void
parseAscInit (void)
{
	int i;

	if (inited++)
		return;

	for (i = 0; i < 256; i++)
		dearmorTable[i] = -1;

	for (i = 0; i < (int)(sizeof(armorTable) - 1); i++)
		dearmorTable[armorTable[i] & 0xff] = (char)i;

	inited = 1;
	return;
}

/*
 * Convert input bytes to output bytes.  Returns the number of
 * input bytes successfully converted.  The number of output
 * bytes available is one less than this number.
 */
static int
dearmorMorsel (byte const in[4], byte out[3])
{
	signed char c0, c1;

	c0 = dearmorTable[in[0] & 255];
	if (c0 < 0)
		return 0;
	c1 = dearmorTable[in[1] & 255];
	if (c1 < 0)
		return 1;

	out[0] = (c0 & 63u) << 2 | (c1 & 63u) >> 4;

	c0 = dearmorTable[in[2] & 255];
	if (c0 < 0)
		return 2;

	out[1] = (c1 & 63u) << 4 | (c0 & 63u) >> 2;

	c1 = dearmorTable[in[3] & 255];
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
static int
dearmorLine (struct Context *ctx, unsigned inlen)
{
	byte const *in = ctx->armorline;
	byte *out = ctx->databuf;
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
			if (in[3] == '=' && in[3] == '3' && in[4] == 'D')
				return outlen + 2;
		}
		break;
	}
	/* None of the above - we have an error */
	return -1;

}

/*
 * Given a line thought to contain a CRC, this returns the 24-bit
 * CRC, or -1 on error.  Handles possible MIME expansion
 * of "=" to "=3D".
 */
static long
dearmorCrc (struct Context *ctx)
{
	byte const *in = ctx->armorline;
	unsigned inlen = ctx->armorlen;
	byte buf[3];

	/* skip trailing white space */
	while (inlen && isspace (in[inlen - 1]))
		inlen--;

	if (*in != '=')
		return -1;
	if (inlen == 5)
		if (dearmorMorsel (in + 1, buf) != 4)
			return -1;
		else if (inlen == 7)
			if (in[1] != '3' || in[2] != 'D' ||
			    dearmorMorsel (in + 3, buf) != 4)
				return -1;
			else
				return -1;
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
 * In doublespace mode, we use two more values.  eol==3 means that
 * we saw one line ending (LF or CRLF) and now need more data to see
 * another.  eol==4 means that we saw CR-LF-CR and are now looking for
 * the LF.  (doublespace means we skip a blank line after every non-blank
 * line.  A third party application accidentally munges the files to that
 * form.)
 *
 * The return value is the number of bytes used.  If all the bytes are
 * used and EOL is not set, then more data is required and readLine
 * should be called with more data.
 */
static size_t
readLine (struct Context *ctx, byte const *buf, size_t size)
{
	size_t size0 = size;
	unsigned t, retlen;
	byte eol = ctx->eol;
	byte *ptr = ctx->armorline + ctx->armorlen;

	if (!size)
		return 0;

	/*
	 * This test is needed in case the '\r' and '\n' come in
	 * different write calls.
	 */
	if (eol >= 2)
		goto lineend;

	/* try to fill the input buffer with a line */
	t = (unsigned)min (LINE_LEN-1 - ctx->armorlen, size);
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
	 * \n or something else. If so, copy that in and set eol=1.
	 * Then do the doublespace logic.  If we get here with eol > 2 that
	 * can only happen from entry, and we use kludgy goto's to fall into
	 * the right place in the eol logic below.
	 */
lineend:
	if (eol) {
		if (eol == 3)
			goto eol3;
		if (eol == 4)
			goto eol4;
		ctx->crlf = (eol == 2 ? PGP_TEXTFILT_CR : PGP_TEXTFILT_LF);

		if (eol == 2 && size) {
			if (*buf == '\n') {
				*ptr++ = *buf++;
				ctx->armorlen++;
				ctx->crlf = PGP_TEXTFILT_CRLF;
				size--;
			}
			eol = 1;
		}
		/*
		 * Don't do the doublespace transformation to blank lines!
		 * This is ad hoc but that is how the failing case presents
		 * itself.  Unfortunately it is hard to know here that we are
		 * looking at a blank line.  The calling code parses lines as soon
		 * as we return with eol > 0.  Then later it calls us to "clean up"
		 * (get to eol==1),
		 * but it has reset the armorlen index to 0.  It becomes hard to
		 * distinguish between a CR-LF at the end of the line, where we got
		 * split between them, and a CR-LF at the beginning.  In each case
		 * our eol > 0 call looks much the same.
		 * To fix this, we have fixed the caller in the relevant portions
		 * (parsing of ascii armored messages/keys) to call us with an
		 * armorlen index of 1 while cleaning up.  This lets us peek at
		 * the original first char of the line and determine whether we
		 * are looking at a blank line.
		 */
		if (ctx->doublespace && eol==1 && ctx->armorline[0]!='\n' &&
			ctx->armorline[0]!='\r') {
			/* Ignore any following blank line */
eol3:
			if (!size) {
				/* LF or CR-LF and no more data.  Remember state 3 */
				eol = 3;
			} else if (*buf == '\n') {
				/* Got a blank line, just ignore it */
				size--;
				eol = 1;
			} else if (*buf == '\r') {
				size--;
				buf++;
				if (ctx->crlf == PGP_TEXTFILT_CR) {
					/* Got a blank line, just ignore it */
					eol = 1;
				} else {
					/* Want to see CR-LF-CR-LF, got the first three... */
eol4:
					if (!size) {
						/* CR-LF-CR and no more data. Remember state 4 */
						eol = 4;
					} else if (*buf == '\n') {
						/* OK, got all four, just ignore the line */
						eol = 1;
						size--;
					} else {
						/* CR-LF-CR-XX, turn off doublespace */
						ctx->doublespace = FALSE;
						size++;	/* Give back CR we took */
						eol = 1;
					}
				}
			} else {
				/* LF or CR-LF then XX */
				ctx->doublespace = FALSE;
				eol = 1;
			}
		}
	}

	/* Save off the EOL and return the number of bytes used */
	ctx->eol = eol;
	return size0-size;
}

/* Work like memcmp but ignore material in angle brackets for 1st arg */
/* On a match (return of 0) put number of removed chars in *removelen */
static int
bracketcmp(const char *orig, size_t origlen, const char *test, size_t testlen,
	size_t *removelen)
{
	char buf[LINE_LEN];
	int inbracket = 0;
	int rtn;
	unsigned i, j;

	/* Copy orig to buf, removing bracketed material */
	pgpAssert (origlen <= sizeof(buf));
	for (i=0,j=0; i<origlen && j < testlen; ++i) {
		if (inbracket && orig[i]=='>')
			inbracket = 0;
		else if (!inbracket && orig[i]=='<')
			inbracket = 1;
		else if (!inbracket)
			buf[j++] = orig[i];
	}
	rtn = memcmp(buf, test, testlen);
	if (!rtn && removelen)
		*removelen = i - j;
	return rtn;
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
parseHeader (struct Context *ctx, byte const *buf, size_t size)
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
	if (!isalpha (*ptr++))
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
		if (!isalnum(c) && c != '-')
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
			min ((strlen (ptr) + 1), sizeof (ctx->messageid)));

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
 *	 1 -  5		Waiting for start of PGP data
 *
 *	 9 - 10		Process -----BEGIN PGP line
 *
 *	11 - 13		Process headers after -----BEGIN PGP
 *
 *	14 - 16		Setup to begin processing armored data
 *
 *	17 - 20		Process PGP armored data
 *
 *	21 - 23		Process CRC at end of armored data
 *
 *	24 - 27		Process -----END PGP line, finish up, back to 0
 *
 *	30 - 33		Clearsigned message, process Hash: lines if any
 *
 *	34 - 36		Setup to begin processing clearsigned message
 *
 *	39 - 43		Process clearsigned message body, then to 9 for sig
 *
 *	50 - 53		Process binary message, to 26 to finish up when done
 *
 *	60 - 63		Scan MIME headers, get boundary string
 *
 *	65 - 68		Look for initial MIME boundary string, to 34 when found
 *
 *	70 - 72		Check apparent initial MIME separator, to 34 if looks OK
 *
 *	75 - 77		Look for start of PGP sig in MIME clearsign sig subpart, to 9
 *
 *	80 - 83		Look for final MIME clearsign boundary after PGP sig
 *
 */
static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
	struct Context *ctx;
	struct Message *msg;
	struct MsgPart *part;
	byte *ptr, *num;
	static char const prefix1[] = "-----BEGIN PGP ";
	static char const prefix2[] = ", PART ";
	static char const prefix3[] = "-----END PGP ";
	static char const suffix[] = "-----";	/* With trailing nul */
	static char const signedmsg[] = "SIGNED MESSAGE-----";
	static char const sigprefix[] = "-----BEGIN PGP SIGNATURE-----";
#if MIMEPARSE
	byte *ptr2;
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
	size_t bracketlen = 0;
	long crc;
	char temp_c;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);
	pgpAssert (error);

	ctx = (struct Context *)myself->priv;
	pgpAssert (ctx);
	pgpAssert (ctx->tail);

	switch (ctx->state) {
	case 0:
case_0:
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

		ctx->state++;
		/* FALLTHROUGH */
	case 1:
case_1:
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 2:
		/*
		 * check the line for BEGIN.  If it is not a begin
		 * then we should output this line. If it is a begin,
		 * then we need to parse it as an armor beginning.
		 * Use the special "bracketcmp" routine which ignores material in
		 * angle brackets "<>" as some mailers may stick those in front of
		 * our message to turn off some rich text features.
		 */

		if (ctx->armorlen > sizeof (prefix1) - 1 &&
			0==bracketcmp (ctx->armorline, ctx->armorlen,
						   prefix1, sizeof (prefix1) - 1, &bracketlen)) {
			ctx->state = 9;
			goto case_9;
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
				ctx->state = 50;
				goto case_50;
			}
		}

#if MIMEPARSE
		/*
		 * Look for either the MIME content-type message, or what looks
		 * like a MIME message boundary (in case we don't have headers).
		 */
		if (ctx->armorlen > sizeof (prefix_mime1) &&
			!strncasecmp (ctx->armorline, prefix_mime1,
						  sizeof (prefix_mime1) - 1)) {
			/* MIME header */
			ctx->state = 60;
			goto case_60;
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
			ctx->state = 70;
			goto case_70;
		}
#endif

		ctx->state++;
		/* FALLTHROUGH */
	case 3:
case_3:
		/* send annotation if we need to */
		if (! ctx->annotation) {
			*error = sendAnnotate (ctx, myself,
					       PGPANN_NONPGP_BEGIN, NULL, 0);
			if (*error)
				break;
			ctx->annotation = PGPANN_NONPGP_END;
			ctx->depth_at_ann = ctx->scope_depth;
		}
		ctx->state++;
		/* FALLTHROUGH */
	case 4:
case_4:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 5:
		/*
		 * if EOL == 1, clear state and goto state 1, otherwise
		 * read data and return to state 4 to output the line.
		 */
		if (ctx->eol == 1) {
			ctx->eol = 0;
			ctx->eob = 0;
			ctx->state = 1;
			goto case_1;
		}

		if (!size)
			break;

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		ctx->state = 4;
		goto case_4;

	case 9:
case_9:
		/*
		 * parse begin line.  If this is not a valid begin
		 * line, assume that it is not and output it as
		 * non-PGP text (in state 3).
		 * bracketlen holds extra characters to skip from our parsing
		 */
		ptr = ctx->armorline + sizeof (prefix1) - 1 + bracketlen;
		lineused = ctx->armorlen - (sizeof (prefix1) - 1) - bracketlen;
		bracketlen = 0;
		thispart = maxparts = 0;

		do {
			if (lineused == 0) {
				/* not a valid BEGIN line */
				ctx->state = 3;
				goto case_3;
			}

		} while (isspace (ptr[--lineused]));

		/* temporarily null-terminate this string */
		temp_c = ptr[++lineused];
		ptr[lineused] = '\0';

		if (!memcmp (ptr, signedmsg, sizeof (signedmsg))) {
			/* This is a clearsigned message */

			if (ctx->eol) {
				/*
				 * This looks like a clearsigned
				 * message. jump into the clearsigned
				 * parser and parse it.
				 */
				ctx->state = 30;
				goto case_30;
			}
			/*
			 * Someone is playing with us.. This is an
			 * error.
			 */
			goto err_case_9;
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
				goto err_case_9;
			ptr += sizeof (prefix2) - 1;
			lineused -= sizeof (prefix2) - 1;

			thispart = strtoul ((char *)ptr, (char **)&num, 10);
			if (num == NULL || thispart < 0 || thispart > 999)
				goto err_case_9;

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
					goto err_case_9;
				lineused -= (num - ptr);
				ptr = num;
			}
		} else {
			thispart = maxparts = 1;
		}
		if (memcmp (ptr, suffix, sizeof (suffix)))
			goto err_case_9;
		ctx->thispart = thispart;
		ctx->maxparts = maxparts;
		ctx->state = 10;
		goto case_10;
		err_case_9:
		ptr[lineused] = temp_c;
		ctx->state = 3;
		goto case_3;

	case 10:
case_10:
		/* Send end-annotation (if we sent a begin annotation) */
		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
			*error = sendAnnotate (ctx, myself, ctx->annotation,
					       NULL, 0);
			if (*error)
				break;
			ctx->annotation = 0;
		}
		ctx->firstheader = TRUE; /* Help us recognize doubleline mode */
		ctx->state++;
		/* FALLTHROUGH */
	case 11:
case_11:
	/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 1;	/* Preserve 1st char for doublespace logic */
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
		ctx->state++;
		/* FALLTHROUGH */
	case 12:
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 13:
		/*
		 * If this line is blank, go to state 15. If it is
		 * not blank, process it as a header and then goto
		 * state 11.
		 * If it is blank and firstheader is true, go into doublespace mode
		 * and keep parsing.
		 */

		ptr = ctx->armorline;
		lineused = ctx->armorlen;

		/* Find trailing white space... */
		while (lineused && isspace (ptr[lineused - 1]))
			lineused--;

		if (ctx->firstheader) {
			ctx->firstheader = FALSE;
			if (lineused == 0) {
				/* First line is blank after ----BEGIN PGP, doublespace mode */
				ctx->doublespace = 1;
				ctx->state = 11;
				goto case_11;
			}
		}

		if (lineused) {
			int err;
			/* temporarily null-terminate this string */
			temp_c = ptr[lineused];
			ptr[lineused] = '\0';

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

			ctx->state = 11;
			goto case_11;
		}
		/* this is an empty line; now go deal with Armorlines */

		ctx->state++;
		/* FALLTHROUGH */
	case 14:
		/* state 14 is obsolete */
		ctx->state++;
		/* FALLTHROUGH */
	case 15:
		/* get the message and part information */

		if (!ctx->part) {
		/* This is true for all normal armor messages */
			msg = getMessage (ctx, ctx->messageid, ctx->maxparts);
			if (!msg) {
				/* This is an error */
				*error = PGPERR_NOMEM;
				break;
			}
			part = getPart (ctx, msg, ctx->thispart);
			if (!part) {
				/* This is an error too */
				*error = PGPERR_NOMEM;
				break;
			}
			ctx->part = part;
		}
		ctx->expectcrc = 0;
		ctx->state++;
		/* FALLTHROUGH */
	case 16:
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
			byte *b;

			b = (byte *)pgpAlloc(kPGPPrsAscCmdBufSize);
			if (b == NULL)
			{
				*error = PGPERR_NOMEM;
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
			pgpFree(b);
			if (*error)
				break;
		}
		ctx->state++;
		/* FALLTHROUGH */
	case 17:
case_17:
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 1;	/* Preserve 1st char for doublespace logic */
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
		ctx->state++;
		/* FALLTHROUGH */
	case 18:
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 19:
		/*
		 * make sure this is not a CRC line. If it is a CRC
		 * then goto state 21. if it is not a CRC line, then
		 * it must be an armorline. So, process it as an
		 * armor line and then fallthrough to state 20 to
		 * output it.
		 */

		if (ctx->armorline[0] == '=') {
			/* This looks like a CRC */
			ctx->state = 21;
			goto case_21;
		}

		/* Make sure we're not expecting a CRC */
		if (ctx->expectcrc) {
			*error = sendAnnotate (ctx, myself, PGPANN_ARMOR_NOCRC,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;

			/* Well, maybe this is an end? */
			ctx->state = 24;
			goto case_24;
		}

		/* find the end of the armorline */
		inlen = ctx->armorlen;
		while (inlen && isspace (ctx->armorline[inlen - 1]))
			inlen--;

		/* check the length of the armorline */
		if (inlen > 68) { /* 68 == 64 + 2*"=3D" extra space */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_TOOLONG,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;

			ctx->state = 26;
			goto case_26;
		}

		i = dearmorLine (ctx, inlen);
		if (i > 48 || i < 1) {
			/* error in deamorline */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_BADLINE,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;
			ctx->state = 26;
			goto case_26;
		}
		ctx->expectcrc = (i < 48);
		ctx->crc = crcUpdate (ctx->crc, ctx->databuf, i);
		ctx->datalen = i;
		ctx->dataptr = ctx->databuf;
		ctx->state++;
		/* FALLTHROUGH */
	case 20:
		/* output the dearmored data, then goto state 17 */

		part = ctx->part;
		while (ctx->datalen) {
			retval = writePartData (part, ctx->dataptr,
						ctx->datalen, error);
			ctx->dataptr += retval;
			ctx->datalen -= retval;
			if (*error)
				return written;
		}
		ctx->state = 17;
		goto case_17;

	case 21:
case_21:
		/* Deal with CRC */

		crc = dearmorCrc (ctx);
		if (crc < 0) {
			/* error */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_CRCCANT,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;
			ctx->state = 26;
			goto case_26;
		}
		if ((word32)crc != ctx->crc) {
			/* mismatch */
			*error = sendAnnotate (ctx, myself,
					       PGPANN_ARMOR_CRCBAD,
					       ctx->armorline, ctx->armorlen);
			if (*error)
				break;
			ctx->state = 26;
			goto case_26;
		}
		ctx->state++;
		/* FALLTHROUGH */
	case 22:
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 1;	/* Preserve 1st char for doublespace logic */
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
		ctx->state++;
		/* FALLTHROUGH */
	case 23:
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 24:
case_24:
		/* check for END */

		if (ctx->armorlen < sizeof (prefix3) - 1 ||
		    memcmp (ctx->armorline, prefix3, sizeof (prefix3) - 1)) {
			/* missing end -- do what??? */
			;
		}

#if MIMEPARSE
		if (ctx->mime_signed) {
			/* Special cleanup for MIME, wait for final boundary */
			ctx->state = 80;
			goto case_80;
		}
#endif

		ctx->state++;
		/* FALLTHROUGH */
	case 25:
		/* read until EOL to get to end of the last line */

		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		while (ctx->eol != 1) {
			ctx->armorlen = 1;	 /* Preserve 1st char for doublespace logic */
			retval = readLine (ctx, buf, size);
			written += retval;
			buf += retval;
			size -= retval;

			if (!size)
				return written;
		}

		ctx->state++;
		/* FALLTHROUGH */
	case 26:
case_26:
		/*
		 * finish this part, send an end annotation if we need
		 * to, then clear the context and go to state 0
		 */

		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
			*error = sendAnnotate (ctx, myself, ctx->annotation,
						   NULL, 0);
			if (*error)
				break;
			ctx->annotation = 0;
		}

		ctx->state++;
		/* FALLTHROUGH */

	case 27:
		part = ctx->part;
		*error = donePart (ctx);
		if (*error)
			break;

		ctx->part = NULL;
		ctx->thispart = 0;
		ctx->maxparts = 0;
		memset (ctx->messageid, 0, sizeof (ctx->messageid));
		ctx->dataptr = ctx->databuf;
		ctx->datalen = 0;
		ctx->armorptr = ctx->armorline;
		ctx->armorlen = 0;
		ctx->expectcrc = 0;
		ctx->crlf = 0;
		ctx->doublespace = 0;
		ctx->firstheader = 0;
		ctx->saved_crlf = 0;
		ctx->hashlen = 0;
		ctx->eol = 0;
		ctx->eob = 0;
		ctx->state = 0;
		goto case_0;

		/* BEGIN CLEARSIGNED MESSAGE BELOW HERE */
	case 30:
case_30:
		/* Send end-annotation (if we sent a begin annotation) */
		if (ctx->annotation) {
			pgpAssert (ctx->depth_at_ann == ctx->scope_depth);
			*error = sendAnnotate (ctx, myself, ctx->annotation,
						   NULL, 0);
			if (*error)
				break;
			ctx->annotation = 0;
		}
		ctx->state++;
		/* FALLTHROUGH */
	case 31:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 32:
		/* read the line after the begin: call readline until
		 * we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 33:
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
				ctx->state = 3;
				goto case_3;
			}
			lineused -= 6;
			ptr += 6;

			while (lineused) {
				struct PgpHash const *h;
				byte const *comma = (byte const *)
					strchr ((char const *) ptr, ',');
				unsigned len = comma ? comma-ptr : lineused;
				
				h = pgpHashByName ((char const *)ptr, len);
				if (!h) {
					/* unknown hash.. now what? */
					/*XXX: annotate?*/;
				} else
					ctx->hashlist[ctx->hashlen++] =
						h->type;
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
				ctx->state = 3;
				goto case_3;
			}

			/* Now we need to get an *extra* line, the separator */
			ctx->state = 30;
			goto case_30;
		} /* if (lineused && !ctx->hashlen) */

		if (!ctx->eol || lineused) {
			/*
			* We either have a long line or non
			* white-space. I bet someone's trying to
			* trick us into believeing this is a real
			* clearsigned message, but they've added some
			* text to it. I dont buy it. Output as
			* non-pgp text; goto state 3.
			 		*/

			/* XXX: Should send an annotation: bad clearsig */
			ctx->hashlen = 0;
			ctx->state = 3;
			goto case_3;
		}
		ctx->state++;
		/* FALLTHOUGH */
#if MIMEPARSE
case_34:
#endif
	case 34:
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
		ctx->crlf = 0;
		ctx->state++;
		/* FALLTHROUGH */
	case 35:
		/* Ok, this is a clearsigned message. Send a new
		* begin annotation
		*/
		*error = sendAnnotate (ctx, myself, PGPANN_CLEARSIG_BEGIN,
				NULL, 0);
		if (*error)
			break;
		ctx->annotation = PGPANN_CLEARSIG_END;
		ctx->depth_at_ann = ctx->scope_depth;
		ctx->state++;
		/* FALLTHROUGH */
	case 36:
		/* Get a new message part */
		ctx->thispart = 1;
		ctx->maxparts = 1;
		msg = getMessage (ctx, NULL, 1);
		if (!msg) {
			/* This is an error */
			*error = PGPERR_NOMEM;
			break;
		}
		part = getPart (ctx, msg, ctx->thispart);
		if (!part) {
			/* This is an error too */
			*error = PGPERR_NOMEM;
			break;
		}

		/* And set it up as a clearsigned message */
		*error = createClearsig (ctx, msg, part);
		if (*error)
			break;

		ctx->part = part;
		ctx->state = 39;
		/* FALLTHROUGH */

		/*
		* Below starts the main processing loop for
		* clearsigned messages. It reads in a line. If it
		* is a BEGIN line it converts to the signature output
		* and starts de-armoring the signature. Otherwise it
		* will output any saved CRLF chars, then output this
		* new line, saving (but not outputting) the CRLF
		* chars at the end of the line. Then the loop
		* continues.
		*/
case_39:
	case 39:
		/* call readline until we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 40:
		/*
		* Check if this is a begin line. If so, start
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
				ctx->state = 75;
				goto case_75;
			}
		} else {
#endif
		if (lineused == sizeof (sigprefix) - 1 &&
		!memcmp (ptr, sigprefix, sizeof (sigprefix) - 1)) {
			ctx->state = 9;
			goto case_9;
		}

		if (!memcmp (ctx->armorline, "- ", 2)) {
			ctx->armorptr = ctx->armorline + 2;
			ctx->armorlen -= 2;
		}
#if MIMEPARSE
		}
#endif
		ctx->state++;
		/* FALLTHROUGH */
	case 41:
		/* output CRLF if we need to */

		while (ctx->saved_crlf) {

			if (ctx->saved_crlf & PGP_TEXTFILT_CR) {
				i = writeExtraData (ctx, (byte const *)"\r",
						1, error);
				if (i)
					ctx->saved_crlf &= ~PGP_TEXTFILT_CR;
				if (*error)
					break;
			}

			if (ctx->saved_crlf & PGP_TEXTFILT_LF) {
				i = writeExtraData (ctx, (byte const *)"\n",
						1, error);
				if (i)
					ctx->saved_crlf &= ~PGP_TEXTFILT_LF;
				if (*error)
					break;
			}
		}

		ctx->state++;
		/* FALLTHROUGH */
	case 42:
case_42:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 43:
		/*
		* if EOL == 1, clear state and goto state 39, otherwise
		* read data and return to state 41 to output the line.
		*/
		if (ctx->eol == 1) {
			ctx->eol = 0;
			ctx->eob = 0;
			ctx->saved_crlf = ctx->crlf;
			ctx->crlf = 0;
			ctx->state = 39;
			goto case_39;
		}

		if (!size)
			break;

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		ctx->state = 42;
		goto case_42;

		/* BEGIN BINARY MESSAGE HERE */
	case 50:
case_50:
		/* Send a new begin annotation */
		*error = sendAnnotate (ctx, myself, PGPANN_INPUT_BEGIN,
				NULL, 0);
		if (*error)
			break;
		ctx->annotation = PGPANN_INPUT_END;
		ctx->depth_at_ann = ctx->scope_depth;
		ctx->state++;
		/* FALLTHROUGH */
	case 51:
		/* Get a new message part */
		pgpAssert (!ctx->part);
		ctx->thispart = 1;
		ctx->maxparts = 1;
		msg = getMessage (ctx, NULL, 1);
		if (!msg) {
			/* This is an error */
			*error = PGPERR_NOMEM;
			break;
		}
		part = getPart (ctx, msg, ctx->thispart);
		if (!part) {
			/* This is an error too */
			*error = PGPERR_NOMEM;
			break;
		}

		*error = writeMessage (ctx, msg);
		if (*error)
			break;
		ctx->part = part;
		ctx->state++;
		/* FALLTHROUGH */
	case 52:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 53:
		/*
		* Check if this scope has ended. If so, then close
		* this part by jumping to state 26
		*/
		if (!ctx->annotation) {
			ctx->state = 26;
			goto case_26;
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
case_60:
	case 60:
		/*
		* Here on a MIME multipart/signed message.  Must check protocol
		* and boundary.
		*/
		/* Case sensitive OK? */
		if ((ptr = strstr (ctx->armorline, prefix_mime2)) != NULL) {
			/* Have protocol, we know it's ours */
			ctx->mime_signed = 1;
		}

		/* Case sensitive OK? */
		if ((ptr = strstr (ctx->armorline, prefix_mime3)) != NULL) {
			/* Get boundary */
			int need_quote;
			ptr += strlen (prefix_mime3);
			need_quote = (*ptr == '"');
			if (need_quote)
				++ptr;
			ptr2 = strchr (ptr, need_quote?'"':';');
			if (!ptr2) {
				/* Header too complex for us to parse, rely on heuristic */
				ctx->mime_signed = 0;
				ctx->mime_bound_len = 0;
				ctx->state = 3;
				goto case_3;
			}
			if (ptr2-ptr+5 > (int)sizeof(ctx->mime_boundary)) {
				/* Too big boundary, can't handle it */
				ctx->state = 3;
				goto case_3;
			}
			ctx->mime_boundary[0] = '-';
			ctx->mime_boundary[1] = '-';
			strncpy (ctx->mime_boundary+2, ptr, ptr2-ptr);
			ctx->mime_bound_len = ptr2 - ptr + 2;
			ctx->mime_boundary[ctx->mime_bound_len] = '\0';
		}

#if 0
		/*
		* This micalg finding code may not get used if boundary and proto
		* are on a line before the micalg. Also won't work with comma
		* separated micalgs.
		* Need to improve this, however
		* it is just a speed optimization to have a micalg anyway.
		*/
		ptr = ctx->armorline;
		while ((ptr = strstr (ctx->armorline, prefix_mime4)) != NULL) {
			/* Have mic_alg */
			struct PgpHash const *h;
			ptr += strlen(prefix_mime4);
			if (strncmp (ptr, prefix_mime5, strlen(prefix_mime5)) != 0)
				continue;		 /* Not a PGP hash */
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
			ctx->state = 65;
			goto case_65;
		}

		ctx->state++;
		/* FALLTHROUGH */
	case 61:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 62:
		/* read the line after the begin: call readline until
		* we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 63:
		/*
		* See if a continuation of previous header line. Must start with
		* whitespace followed by non-whitespace.
		*/
		if (ctx->armorlen > 1 && isspace (ctx->armorline[0])) {
			for (i=1; i< (int)ctx->armorlen; ++i) {
				if (!isspace (ctx->armorline[i])) {
					ctx->state = 60;
					goto case_60;
				}
			}
		}
		/*
		* Here we had a mime-signed header, but not our protocol or boundary.
		* Maybe another flavor of mime. We will go back to our initial
		* state. This means that we will have eaten one or more header
		* lines.
		*/
		ctx->mime_signed = 0;
		ctx->mime_bound_len = 0;
		goto case_3;

case_65:
	case 65:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 66:
		/* read the line after the begin: call readline until
		* we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 67:
		if (ctx->armorlen < (unsigned)ctx->mime_bound_len ||
			!ctx->eol ||
		memcmp (ctx->armorline, ctx->mime_boundary, ctx->mime_bound_len)) {
			/* Not yet at boundary for first part */
			ctx->state = 65;
			goto case_65;
		}
		ctx->state++;
		/* FALLTHROUGH */
case_68:
	case 68:
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
		ctx->state = 34;
		goto case_34;

case_70:
	case 70:
		/* Here if we see a "--" line while parsing, may be mime boundary */
		pgpAssert (ctx->eol);
		strcpy (ctx->mime_boundary, ctx->armorline);
		ctx->mime_bound_len = strlen (ctx->mime_boundary);
		ctx->state++;
		/* FALLTHROUGH */
	case 71:
		/* read the line after the boundary: call readline until
		* we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;
	case 72:
		/* MUST be a MIME header, or we are lost */
		if (!ctx->eol || ctx->armorlen < 4 || isspace(ctx->armorline[0]) ||
			!(ptr = strchr (ctx->armorline, ':')) || !isspace(ptr[1])) {
			/*
			* If no good, we go back to our initial state. This means we
			* will have eaten the boundary line.
			*/
			ctx->state = 3;
			goto case_3;
		}

		/* Valid mime header, go on and begin processing message */
		ctx->mime_signed = 1;

		/*
		* Fall into common clearsigned code above. Flags we have set will
		* change its state-leaving behavior.
		*/
		ctx->state = 68;
		goto case_68;

case_75:
	case 75:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 76:
		/* read the next line: call readline until
		* we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 77:
		/* See if have BEGIN PGP SIGNATURE line, if so go process it */
		if ((ctx->armorlen > sizeof (sigprefix) - 1 &&
		!memcmp (ctx->armorline, sigprefix, sizeof (sigprefix) - 1)) ||
			(ctx->armorlen > sizeof (prefix_mimesig2) - 1 &&
		!memcmp (ctx->armorline, prefix_mimesig2,
					sizeof (prefix_mimesig2) - 1))) {
			/* Have BEGIN line for signature */
			ctx->state = 9;
			goto case_9;
		}

		/* Else loop back */
		ctx->state = 75;
		goto case_75;

case_80:
	case 80:
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
		ctx->state++;
		/* FALLTHROUGH */
case_81:
	case 81:
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
		ctx->state++;
		/* FALLTHROUGH */
	case 82:
		/* read the line after the begin: call readline until
		* we have EOB or EOL */

		retval = readLine (ctx, buf, size);
		size -= retval;
		buf += retval;
		written += retval;

		if (ctx->eol || ctx->eob)
			ctx->state++;
		else
			break;

		/* FALLTHROUGH */
	case 83:
		/* Check for final boundary */
		if (ctx->eol && ctx->armorlen >= (unsigned)ctx->mime_bound_len &&
		!memcmp (ctx->armorline, ctx->mime_boundary, ctx->mime_bound_len)) {
			/* Clean up mime flags, go to clean-up state */
			ctx->mime_signed = 0;
			ctx->mime_bound_len = 0;
			goto case_26;
		}
		
		/* Keep looping until we see it */
		ctx->state = 81;
		goto case_81;

#endif

	default:
		/* Every state should be enumerated above */
		pgpAssert (0);
	}

	return written;
}

static int
Flush (struct PgpPipeline *myself)
{
	struct Context *context;
	int error = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	(void) Write (myself, NULL, 0, &error);
	if (error)
		return error;

	return context->tail->flush (context->tail);
}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
	struct Context *context;
	int error = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (bytes)
		return 0;

	/* There are no more bytes. Therefore this is the EOL. */
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
		* incomplete. If it is, kill it now!
		*/
		if (context->part->sig && !context->part->msg->foundpart1)
			(void)0;
		
		context->state = 26;
		(void) Write (myself, NULL, 0, &error);
		if (error)
			return error;
	}

			/*
			* If we have data buffered at this point, we return an error
			* code to let the caller know that. If we miss a part of a
			* multipart message, this will be the case. The user will
			* receive the error message and can then send annotations to
			* figure out what is missing and possible send it or at least
			* inform the user.
			*
	* This keeps the data consisent and doesn't propagate a
	* sizeAdvise() in the wrong scope.
			*/

	if (context->scope_depth)
		return 0;

	if (context->msgs)
		return PGPERR_PARSEASC_INCOMPLETE;

	/* Bytes is zero */
	return context->tail->sizeAdvise (context->tail, bytes);
}

static struct Message *
getMessageNumber (struct Context *ctx, int num)
{
	struct Message *msg;

	msg = ctx->msgs;
	while (msg) {
		if (msg->msg_number == num)
			return msg;
		msg = msg->next;
	}
	return NULL;
}

static int
sendStatus (struct PgpPipeline *myself, int type, byte const *string,
	size_t size)
{
	struct Context *ctx = (struct Context *)myself->priv;
	struct Message *msg;
	struct MsgPart *part;
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
static int
sendAnnotate (struct Context *ctx, struct PgpPipeline *origin, int type,
	byte const *string, size_t size)
{
	pgpAssert (ctx->tail);

	if (ctx->buffering)
		return bufferAnnotation (ctx, origin, type, string, size);

	return ctx->tail->annotate (ctx->tail, origin, type, string, size);
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	byte const *string, size_t size)
{
	struct Context *context;
	int error;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	switch (type) {
	case PGPANN_PARSEASC_MSG_COUNT:
	case PGPANN_PARSEASC_MSG_LEFT:
	case PGPANN_PARSEASC_MSG_CURRENT:
	case PGPANN_PARSEASC_MSG_PARTS:
	case PGPANN_PARSEASC_MSG_PARTINFO:
		return sendStatus (myself, type, string, size);
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

static void
Teardown (struct PgpPipeline *myself)
{
	struct Context *context;
	struct Message *msg, *temp;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);

	for (msg = context->msgs; msg; msg = temp) {
		temp = msg->next;
		freeMessage (context, msg);
	}

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpFifoDestroy (context->fifod, context->data);
	pgpFifoDestroy (context->fifod, context->ann);

	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

struct PgpPipeline **
pgpParseAscCreate (struct PgpPipeline **head, struct PgpEnv const *env,
		struct PgpFifoDesc const *fd,
		struct PgpUICb const *ui, void *ui_arg)
{
	struct PgpPipeline *mod;
	struct Context *context;
	struct PgpFifoContext *data, *ann;

	if (!head || !env)
		return NULL;

	parseAscInit ();

	context = (struct Context *)pgpMemAlloc (sizeof (*context));
	if (!context)
		return NULL;
	mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
	if (!mod) {
		pgpMemFree (context);
		return NULL;
	}
	data = pgpFifoCreate (fd);
	if (!data) {
		pgpMemFree (context);
		pgpMemFree (mod);
		return NULL;
	}
	ann = pgpFifoCreate (fd);
	if (!ann) {
		pgpFifoDestroy (fd, data);
		pgpMemFree (context);
		pgpMemFree (mod);
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

	memset (context, 0, sizeof (*context));
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


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
