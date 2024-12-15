/*
 * pgpCRC.h -- this is used for CRC Checksums.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpCRC.h,v 1.4 1997/06/04 16:17:46 lloyd Exp $
 */

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

PGPUInt32 crcUpdate(PGPUInt32 crc, PGPByte const *buf, unsigned len);

PGP_END_C_DECLARATIONS

#define CRC_INIT 0xb704ceu
