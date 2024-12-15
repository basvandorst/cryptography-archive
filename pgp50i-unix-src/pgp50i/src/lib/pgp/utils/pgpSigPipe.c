/*
 * pgpSigPipe.c -- Create a Signature Verification Pipeline
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigPipe.c,v 1.2.2.1 1997/06/07 09:51:42 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpSigPipe.h"
#include "pgpFIFO.h"
#include "pgpPrsAsc.h"
#include "pgpVerifyRa.h"

struct PgpPipeline *
pgpSignatureVerifyCreate (struct PgpPipeline **texthead,
							struct PgpPipeline **sighead,
							struct PgpEnv const *env,
							struct PgpFifoDesc const *fd,
							byte const *hashlist, unsigned hashlen,
							int textmode,
							struct PgpUICb const *ui, void *ui_arg)
{
if (!texthead || !sighead)
	return NULL;

if (!fd)
	fd = &pgpByteFifoDesc;

sighead = pgpParseAscCreate (sighead, env, fd, ui, ui_arg);
if (!sighead)
	return NULL;

return pgpVerifyReaderCreate (texthead, sighead, env, fd,
			hashlist, hashlen, textmode, ui, ui_arg);
}
