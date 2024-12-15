/*
* pgpLiteral.c -- create a literal packet
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpLiteral.c,v 1.2.2.1 1997/06/07 09:51:08 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpAddHdr.h"
#include "pgpLiteral.h"
#include "pgpPktByte.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"

struct PgpPipeline **
pgpLiteralCreate (struct PgpPipeline **head, PgpVersion version,
		 struct PgpFifoDesc const *fd, byte type, byte *name,
		 byte len, word32 timestamp)
	{
		byte header[261]; /* Maximum size of a literal header */
		int i;
		struct PgpPipeline **mod;

		if (!head)
		 return NULL;

		header[0] = type;
		header[1] = len;
		memcpy (header+2, name, len);
		len += 2;
		for (i=3; i > 0; i--) {
		 header[len+i] = (byte) (timestamp & 0xff);
		 timestamp >>= 8;
	}

mod = pgpAddHeaderCreate (head, version, fd, PKTBYTE_LITERAL, 0,
   header, len+4);

memset (header, 0, sizeof (header));
if (mod)
 (*head)->name = "Literal Module";

return mod;
}
