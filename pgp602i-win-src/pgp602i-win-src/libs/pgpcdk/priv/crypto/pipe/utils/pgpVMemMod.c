/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Module to output to a variable-sized memory buffer

	We do allow a maximum size to be specified.  If we overflow that size,
	we discard the rest.
	We provide an annotation to tell how much we wrote or would have written.

	$Id: pgpVMemMod.c,v 1.23.4.1.2.1 1998/11/12 03:21:28 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpVMemMod.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"
#include "pgpFIFO.h"
#include "pgpUtilitiesPriv.h"
#include "pgpContext.h"

#define VMEMMODMAGIC	0xdeafbabe

/*
 * Note that byte_count may come to exceed buf_size, in which case we
 * discard the overflow.  We provide an annotation to read the total number
 * of bytes written.
 */
typedef struct vmemModContext {
	PGPPipeline	pipe;
	
	int scope_depth;
	PGPBoolean enable;			/* True if accepting data, false to discard */
	size_t max_size;			/* Max number of bytes we will accept */
	size_t byte_count;			/* Number of bytes we have been given */
	size_t fifo_count;			/* Number of bytes in fifo */
	PGPFifoDesc const *fd;		/* Type of fifo we use */
	PGPFifoContext *fifo;		/* Fifo we store into */
	PGPBoolean secure;			/* Use secure memory allocations */
	DEBUG_STRUCT_CONSTRUCTOR( vmemModContext )
} vmemModContext;


static PGPError
vmemFlush (PGPPipeline *myself)
{
	vmemModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VMEMMODMAGIC);

	context = (vmemModContext *)myself->priv;
	pgpAssert (context);

	return( kPGPError_NoErr );
}

/* Write what is given, but disard any overflows. */
static size_t
vmemWrite (PGPPipeline *myself, PGPByte const *buffer, size_t size,
	   PGPError *error)
{
	vmemModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == VMEMMODMAGIC);

	context = (vmemModContext *)myself->priv;
	pgpAssert (context);

	/* If disabled, discard data */
	if (!context->enable)
		return size;

	if (context->max_size > context->byte_count) {
		size_t avail = context->max_size - context->byte_count;
		size_t to_write = pgpMin (avail, size);

		while (to_write) {
			size_t written = pgpFifoWrite (context->fd, context->fifo,
										   buffer, to_write);
			context->fifo_count += written;
			buffer += written;
			to_write -= written;
		}
	}

	if (error)
		*error = kPGPError_NoErr;
	context->byte_count += size;
	return size;
}

/*
 * We accept an annotation to return the number of bytes we were asked
 * to write.  We also provide an annotation to return the max size of the
 * buffer.  Plus there is one to return the number of bytes in the FIFO.
 * Another annotation makes us copy our FIFO data to a buffer and return
 * its pointer.
 * We return our results in the passed-in string pointer, which must be
 * of size sizeof(size_t).
 * The ENABLE annotation causes us to turn on and off whether we accept data.
 */
static PGPError
vmemAnnotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	      PGPByte const *string, size_t size)
{
	vmemModContext *context;
	PGPByte const *	fbuf;
	PGPMemoryMgrRef	memoryMgr	= NULL;
	PGPError		err	= kPGPError_NoErr;

	(void)origin;	/* Avoid warning */
	(void)size;

	pgpAssert (myself);
	pgpAssert (myself->magic == VMEMMODMAGIC);

	context = (vmemModContext *)myself->priv;
	pgpAssert (context);
	
	memoryMgr	= PGPGetContextMemoryMgr( myself->cdkContext );

	switch (type) {
	case PGPANN_MEM_BYTECOUNT:
		/* Number of bytes we were asked to write */
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->byte_count;
		return( kPGPError_NoErr );
	case PGPANN_MEM_BUFSIZE:
		/* Return number of bytes in FIFO */
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->fifo_count;
		return( kPGPError_NoErr );
	case PGPANN_MEM_MAXSIZE:
		/* Return maximum size of buffer */
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->max_size;
		return( kPGPError_NoErr );
		
	case PGPANN_MEM_PREPEND:
		/* This provides some data to stick into fifo */
		while (size) {
			size_t written = pgpFifoWrite (context->fd, context->fifo,
										   (PGPByte *)string, size);
			context->fifo_count += written;
			context->byte_count += written;
			string += written;
			size -= written;
		}
		return( kPGPError_NoErr );

	case PGPANN_MEM_MEMBUF:
	{
		PGPByte *	membuf	= NULL;
		PGPByte *	tbuf	= NULL;
		PGPSize		length;
	
		/* Return memory buffer with fifo data. This removes data from fifo */
		/* Produces a null pointer if couldn't allocate data */
		/* Always stores a null byte beyond data (not counted in length) */
		pgpAssert (string);
		pgpAssert (size == sizeof(PGPByte *));
		
		/* this data is returned to the caller so we use PGPNewData */
		if( context->secure ) {
			tbuf = membuf = (PGPByte *)
				PGPNewSecureData( memoryMgr, context->fifo_count + 1, 0);
		} else {
			tbuf = membuf = (PGPByte *)
				PGPNewData( memoryMgr, context->fifo_count + 1, 0);
		}
		if ( IsntNull( membuf ) )
		{
			membuf[ context->fifo_count ] = '\0';	/* Null beyond buffer */
				
			fbuf = pgpFifoPeek( context->fd, context->fifo, &length );
			while( length != 0 )
			{
				pgpCopyMemory( fbuf, tbuf, length );
				if ( IsntNull( membuf ) ) {
					tbuf += length;
					pgpFifoSeek( context->fd, context->fifo, length );
				}
				
				fbuf	= pgpFifoPeek( context->fd,
							context->fifo, &length );
			}
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
		
		*(PGPByte **)string = membuf;
		return( err );
	}
	
	case PGPANN_MEM_ENABLE:
		pgpAssert (string);
		pgpAssert (size == 1);
		context->enable = *(PGPByte *)string;
		return( kPGPError_NoErr );
	default:
		;		/* do nothing */
	}

	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return( kPGPError_NoErr );
}

static PGPError
vmemSizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	vmemModContext *context;

	(void)bytes;

	pgpAssert (myself);
	pgpAssert (myself->magic == VMEMMODMAGIC);

	context = (vmemModContext *)myself->priv;
	pgpAssert (context);

	return( kPGPError_NoErr );
}

static PGPError
vmemTeardown (PGPPipeline *myself)
{
	vmemModContext *context;
	
	pgpAssertAddrValid( myself, PGPPipeline );

	pgpAssert (myself);
	pgpAssert (myself->magic == VMEMMODMAGIC);

	context = (vmemModContext *)myself->priv;
	pgpAssert (context);

	pgpFifoDestroy( context->fd, context->fifo );
	
	pgpClearMemory( context,  sizeof (*context));
	PGPFreeData( context );
	
	return( kPGPError_NoErr );
}

static PGPPipeline *
vmemCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	head,
	size_t			max_size,
	PGPBoolean		secure)
{
	PGPPipeline *mod;
	vmemModContext *context;
	PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( cdkContext );
	
	if (!head)
		return NULL;

	*head = 0;

	context = (vmemModContext *)PGPNewData( memoryMgr,
		sizeof(*context), kPGPMemoryMgrFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	mod->magic = VMEMMODMAGIC;
	mod->write = vmemWrite;
	mod->flush = vmemFlush;
	mod->sizeAdvise = vmemSizeAdvise;
	mod->annotate = vmemAnnotate;
	mod->teardown = vmemTeardown;
	mod->name = secure	? "Securely write variable-sized memory"
						: "Write variable-sized memory";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->fd = secure ? &pgpSecureByteFifoDesc : &pgpByteFifoDesc;
	context->fifo = pgpFifoCreate( cdkContext, context->fd );
	context->max_size = max_size;
	context->enable = 1;
	context->secure = secure;

	*head = mod;
	return mod;
}


PGPPipeline *
pgpVariableMemModCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	head,
	size_t			max_size)
{
	return vmemCreate (cdkContext, head, max_size, FALSE);
}

PGPPipeline *
pgpSecureVariableMemModCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	head,
	size_t			max_size)
{
	return vmemCreate (cdkContext, head, max_size, TRUE);
}
