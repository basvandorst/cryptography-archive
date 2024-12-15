/*
 * Low-level packet-handling routines for keyring management.
 *
 * $Id: pgpRngPkt.h,v 1.7 1997/06/25 19:40:45 lloyd Exp $
 */
#ifndef Included_pgpRngPkt_h
#define Included_pgpRngPkt_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPFile;

#ifdef DEADCODE
extern char const * const packet_type[16];
#endif /* DEADCODE */
 
int pktByteGet(PGPFile *f, PGPUInt32 *lenp, PGPUInt32 *posp);
int pktBytePut(PGPFile *f, PGPByte pktbyte, PGPUInt32 len);
int pktByteLen (PGPByte pktbyte, PGPUInt32 len);
int fileUnequalBuf(PGPFile *f, char const *buf, size_t len);

PGP_END_C_DECLARATIONS
 
#endif /* Included_pgpRngPkt_h */
