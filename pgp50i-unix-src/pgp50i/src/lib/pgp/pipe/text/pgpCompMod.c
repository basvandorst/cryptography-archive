/*
* pgpCompMod.c -- General Compression and Decompression Module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpCompMod.c,v 1.3.2.1 1997/06/07 09:51:06 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpAddHdr.h"
#include "pgpCompMod.h"
#include "pgpDefMod.h"
#include "pgpInfMod.h"
#include "pgpPktByte.h"
#include "pgpCompress.h"
#include "pgpPipeline.h"

struct PgpPipeline **
pgpCompressModCreate (struct PgpPipeline **head, PgpVersion version,
		 struct PgpFifoDesc const *fd, byte type, int quality)
{
		struct PgpPipeline *newhead = NULL, **tail = &newhead;

		if (!head)
		 return NULL;

		switch (type) {
		case PGP_COMPRESSALG_ZIP:
		 tail = defModCreate (tail, quality);
		 break;
		default:
		 tail = NULL;
		}

		if (tail) {
		tail = pgpAddHeaderCreate (tail, version, fd,
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
struct PgpPipeline **
pgpDecompressModCreate (struct PgpPipeline **head, byte type)
{
 if (!head)
  return NULL;

switch (type) {
case PGP_COMPRESSALG_ZIP:
 return infModCreate (head);
default:
 return NULL;
}
}
