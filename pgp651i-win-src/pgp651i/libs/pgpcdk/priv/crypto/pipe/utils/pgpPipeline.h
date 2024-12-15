/*
 * pgpPipeline.h -- This is the code that is the public part of a pipeline
 * module, and defines everything that all the pipelines should export.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function/Structure Header.
 *
 * $Id: pgpPipeline.h,v 1.12 1997/12/11 23:12:47 lloyd Exp $
 */

#ifndef Included_pgpPipeline_h
#define Included_pgpPipeline_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPPipeline
{
	/* A means to check if this is a valid module -- put a magic
	 * number in the structure for each different pipeline module
	 * and compare it.  Simple, eh?
	 */
	PGPUInt32	magic;

	/* Exported Pipeline Functions that all Pipelines support */
	size_t		(*write) (PGPPipeline *myself, PGPByte const *buffer,
					  size_t size, PGPError *error);
	PGPError	(*flush) (PGPPipeline *myself);
	PGPError	(*annotate) (PGPPipeline *myself,
			     PGPPipeline *origin, int type, 
			     PGPByte const *string, size_t size);
	PGPError	(*sizeAdvise) (PGPPipeline *myself,
				unsigned long bytes);
	PGPError	(*teardown) (PGPPipeline *myself);

	/* Some exported information about this particular module */
	char const *	name;

	PGPContextRef	cdkContext;
	
	/* A pointer to module-specfic data */
	PGPUserValue	priv;
};


PGP_END_C_DECLARATIONS

#endif /* Included_pgpPipeline_h */
