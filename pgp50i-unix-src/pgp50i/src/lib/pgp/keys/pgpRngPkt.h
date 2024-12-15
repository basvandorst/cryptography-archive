/*
* pgpRngPkt.h -- Low-level packet-handling routines for keyring management.
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRngPkt.h,v 1.3.2.1 1997/06/07 09:50:38 mhw Exp $
*/
#ifndef PGPRNGPKT_H
#define PGPRNGPKT_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpFile;

#ifdef DEADCODE
extern char const * const packet_type[16];
#endif /* DEADCODE */

int pktByteGet(struct PgpFile *f, word32 *lenp, word32 *posp);
int pktBytePut(struct PgpFile *f, byte pktbyte, word32 len);
int pktByteLen (byte pktbyte, word32 len);
int fileUnequalBuf(struct PgpFile *f, char const *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNGPKT_H */
