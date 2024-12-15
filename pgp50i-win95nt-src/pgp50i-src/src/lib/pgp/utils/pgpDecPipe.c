/*
 * pgpDecPipe.c -- setup the Decryption Pipeline, given the arguments
 * of the UI. This is just a helper function to setup the decryption
 * pipeline. It would be just as easy for an application to do it.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpDecPipe.c,v 1.3.2.1 1997/06/07 09:51:39 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDecPipe.h"
#include "pgpFIFO.h"
#include "pgpPrsAsc.h"
#include "pgpReadAnn.h"

struct PgpPipeline **
pgpDecryptPipelineCreate (struct PgpPipeline **head,
							struct PgpEnv *env,
							struct PgpFifoDesc const *fd,
							struct PgpUICb const *ui,
							void *ui_arg)
	{
			if (!head)
				 return NULL;

			if (!fd)
				 fd = &pgpByteFifoDesc;

			head = pgpParseAscCreate (head, env, fd, ui, ui_arg);
			if (!head)
				 return NULL;

			return pgpAnnotationReaderCreate (head, env, ui, ui_arg);
	}
