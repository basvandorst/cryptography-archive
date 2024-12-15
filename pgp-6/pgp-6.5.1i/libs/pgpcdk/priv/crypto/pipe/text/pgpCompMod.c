/*
 * pgpCompMod.c -- General Compression and Decompression Module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCompMod.c,v 1.9 1998/07/08 20:08:33 hal Exp $
 */
#include "pgpConfig.h"

#include <stdlib.h>
#include "pgpAddHdr.h"
#include "pgpCompMod.h"
#include "pgpDefMod.h"
#include "pgpInfMod.h"
#include "pgpPktByte.h"
#include "pgpCompress.h"
#include "pgpPipeline.h"
#include "pgpErrors.h"

PGPPipeline **
pgpCompressModCreate (
	PGPContextRef		cdkContext,
	PGPPipeline **		head,
	PgpVersion			version,
	PGPFifoDesc const *	fd,
	PGPByte				type,
	int					quality)
{
	PGPPipeline *newhead = NULL, **tail = &newhead;

	if (!head)
		return NULL;

	switch (type) {
	case PGP_COMPRESSALG_ZIP:
		tail = defModCreate ( cdkContext, tail, quality);
		break;
	default:
		tail = NULL;
	}

	if (tail) {
		tail = pgpAddHeaderCreate ( cdkContext, tail, version, fd,
					   PKTBYTE_COMPRESSED, 3, &type, 1);

		if (!tail) {
			newhead->teardown (newhead);
			return NULL;
		}

		*tail = *head;
		*head = newhead;
	}
	return tail;
}

PGPPipeline **
pgpDecompressModCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	head,
	PGPByte			type,
	PGPError		*error)
{
	PGPPipeline **tail;

	if (!head)
		return NULL;

	switch (type) {
	case PGP_COMPRESSALG_ZIP:
	    tail = infModCreate ( cdkContext, head);
		if (tail == NULL)
			*error = kPGPError_OutOfMemory;
		return tail;
	default:
		*error = kPGPError_BadPacket;
		return NULL;
	}
}
