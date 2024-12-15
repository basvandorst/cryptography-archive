/*
 * pgpLiteral.c -- create a literal packet
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpLiteral.c,v 1.10 1997/09/18 01:35:24 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpAddHdr.h"
#include "pgpLiteral.h"
#include "pgpPktByte.h"
#include "pgpPipeline.h"
#include "pgpMem.h"

PGPPipeline **
pgpLiteralCreate (
	PGPContextRef		cdkContext,
	PGPPipeline **		head,
	PgpVersion			version,
	PGPFifoDesc const *	fd,
	PGPByte				type,
	PGPByte *			name,
	PGPByte				len,
	PGPUInt32			timestamp)
{
	PGPByte header[261];	/* Maximum size of a literal header */
	int i;
	PGPPipeline **mod;

	if (!head)
		return NULL;

	header[0] = type;
	header[1] = len;
	memcpy (header+2, name, len);
	len += 2;
	for (i=3; i > 0; i--) {
		header[len+i] = (PGPByte) (timestamp & 0xff);
		timestamp >>= 8;
	}

	mod = pgpAddHeaderCreate ( cdkContext,
			head, version, fd, PKTBYTE_LITERAL, 0, header, len+4);

	pgpClearMemory( header,  sizeof (header));
	if (mod)
		(*head)->name = "Literal Module";

	return mod;
}
