/*
 * pgpCompMod.c -- General Compression and Decompression Module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCompMod.c,v 1.7 1997/06/25 19:40:14 lloyd Exp $
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
	PGPByte			type)
{
	if (!head)
		return NULL;

	switch (type) {
	case PGP_COMPRESSALG_ZIP:
		return infModCreate ( cdkContext, head);
	default:
		return NULL;
	}
}
