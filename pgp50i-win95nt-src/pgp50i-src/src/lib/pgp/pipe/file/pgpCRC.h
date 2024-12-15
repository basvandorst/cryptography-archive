/*
* pgpCRC.h -- this is used for CRC Checksums.
*
* Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpCRC.h,v 1.2.2.1 1997/06/07 09:50:54 mhw Exp $
*/

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

word32 crcUpdate(word32 crc, byte const *buf, unsigned len);

#ifdef __cplusplus
}
#endif

#define CRC_INIT 0xb704ceu
