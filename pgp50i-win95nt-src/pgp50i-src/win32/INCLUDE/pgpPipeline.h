/*
* pgpPipeline.h -- This is the code that is the public part of a pipeline
* module, and defines everything that all the pipelines
* should export.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function/Structure Header.
*
* $Id: pgpPipeline.h,v 1.3.2.1 1997/06/07 09:51:17 mhw Exp $
*/

#ifndef PGPPIPELINE_H
#define PGPPIPELINE_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline {
			/* A means to check if this is a valid module -- put a magic
			* number in the structure for each different pipeline module
			* and compare it. Simple, eh?
			*/
			word32	magic;

			/* Exported Pipeline Functions that all Pipelines support */
			size_t	(*write) (struct PgpPipeline *myself, byte const *buffer,
						size_t size, int *error);
			int		(*flush) (struct PgpPipeline *myself);
			int		(*annotate) (struct PgpPipeline *myself,
						struct PgpPipeline *origin, int type,
						byte const *string, size_t size);
			int		(*sizeAdvise) (struct PgpPipeline *myself,
							 unsigned long bytes);
			void	 (*teardown) (struct PgpPipeline *myself);

			/* Some exported information about this particular module */
			char const *	 name;

			/* A pointer to module-specfic data */
			void 	*priv;
	};
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

#ifdef __cplusplus
}
#endif

#endif /* PGPPIPELINE_H */
