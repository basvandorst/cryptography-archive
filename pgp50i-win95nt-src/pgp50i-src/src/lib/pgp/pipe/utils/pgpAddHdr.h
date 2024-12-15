/*
* pgpAddHdr.h -- header file for a module to add a packet header.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpAddHdr.h,v 1.3.2.1 1997/06/07 09:51:14 mhw Exp $
*/

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
struct PgpFifoDesc;

struct PgpPipeline **
pgpAddHeaderCreate (struct PgpPipeline **head, PgpVersion version,
				struct PgpFifoDesc const *fd, byte pkttype, byte llen,
				byte *header, size_t hdrlen);

#ifdef __cplusplus
}
#endif
