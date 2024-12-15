/*
 * Low-level packet-handling routines for keyring management.
 *
 * $Id: pgpRngPkt.h,v 1.8 1998/01/28 22:17:46 hal Exp $
 */
#ifndef Included_pgpRngPkt_h
#define Included_pgpRngPkt_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPFile;

#ifdef DEADCODE
extern char const * const packet_type[16];
#endif /* DEADCODE */
 
int pktByteGet(PGPFile *f, PGPSize *lenp, PGPSize *len1p, PGPSize *posp);
int pktBytePut(PGPFile *f, PGPByte pktbyte, PGPSize len, PGPSize *len1p);
int pktByteLen (PGPByte pktbyte, PGPSize len);
int fileUnequalBuf(PGPFile *f, void const *buf, PGPSize len, PGPSize len1);
PGPSize pktBodyRead(void *ptr, PGPSize len, PGPSize len1, PGPFile *f);
PGPSize pktBodyWrite(void const *ptr, PGPSize len, PGPSize len1, PGPFile *f);
PGPSize pktSubpacketByteLen (PGPSize len);
PGPSize pktSubpacketCreate (PGPByte type, PGPByte const *data, PGPSize len,
	PGPByte *pktbuf);

PGP_END_C_DECLARATIONS
 
#endif /* Included_pgpRngPkt_h */