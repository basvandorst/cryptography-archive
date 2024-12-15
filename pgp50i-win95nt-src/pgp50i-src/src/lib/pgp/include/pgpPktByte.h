/*
* pgpPktByte.h -- Definitions of packet byte types
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpPktByte.h,v 1.6.2.2 1997/06/07 09:50:19 mhw Exp $
*/
#ifndef PGPPKTBYTE_H
#define PGPPKTBYTE_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_OLD_PKTBYTE(pktbyte) (((pktbyte) & 0xC0) == 0x80)
#define IS_NEW_PKTBYTE(pktbyte) (((pktbyte) & 0xC0) == 0xC0)
#define OLD_PKTBYTE_TYPE(pktbyte) (((pktbyte) >> 2) & 0xF)
#define NEW_PKTBYTE_TYPE(pktbyte) ((pktbyte) & 0x3F)
#define PKTBYTE_TYPE(pktbyte) (IS_OLD_PKTBYTE (pktbyte) ? \
		OLD_PKTBYTE_TYPE(pktbyte) : \
		NEW_PKTBYTE_TYPE(pktbyte))
#define PKTBYTE_LLEN(pktbyte) ((pktbyte) & 3)
#define PKTBYTE_BUILD(type, llen) ((type)>0xf ? \
PKTBYTE_BUILD_NEW(type) : PKTBYTE_BUILD_OLD(type, llen))
#define PKTBYTE_BUILD_OLD(type, llen) ((byte)(0x80 | ((type) & 0xF) <<2 \
	| (llen)))
#define PKTBYTE_BUILD_NEW(type) ((byte)(0xC0 | ((type) & 0x3F)))
#define LLEN_TO_BYTES(llen) ((1 << (llen)) & 7)

/*
* For the new packet formats -- macros for creating the length byte(s)
* of terminating subpackets. The format is:
*	Format					Length			Macro
* 		0sssssss				0-127			PKTLEN_1BYTE()
* 		10ssssss				128-191			PKTLEN_1BYTE()
*		110sssss ssssssss		192-8383		PKTLEN_BYTE0(),PKTLEN_BYTE1()
*
* PKTLEN_ONE_BYTE() is used to determine if the length is 1 byte (use
* PKTLEN_1BYTE()) or two bytes (use PKTLEN_BYTE0() then
* PKTLEN_BYTE1())
*/
#define PKTLEN_ONE_BYTE(len)	((len) < 192)
#define PKTLEN_1BYTE(len)	(byte)(len & 0xFF)
#define PKTLEN_BYTE0(len)	(byte)(0xC0 + (((len-192) >> 8) & 0x1F))
#define PKTLEN_BYTE1(len)	(byte)((len-192) & 0xFF)

/*
* A non-terminating subpacket (followed by another subpacket) is
* a power of two bytes long and has a length byte encoded as
* follows:
* 111xxxxx		2^0-2^31 bytes
*/

/* Types */
enum pktbyte {
				PKTBYTE_ESK = 1,
				PKTBYTE_SIG = 2,
				PKTBYTE_CONVESK = 3,
				PKTBYTE_1PASSSIG = 4,
				PKTBYTE_SECKEY = 5,
				PKTBYTE_PUBKEY = 6,
				PKTBYTE_SECSUBKEY = 7,
			PKTBYTE_COMPRESSED = 8,
				PKTBYTE_CONVENTIONAL = 9,
				PKTBYTE_OLDLITERAL = 10, /* pkt length doesn't include filename, etc */
				PKTBYTE_LITERAL = 11,
				PKTBYTE_TRUST = 12,
				PKTBYTE_NAME = 13,
/* We are replacing the old comment packets with the new subkey
* packets. This will allow older versions of PGP to skip over the
* subkey packets gracefully. The new comment packets will be skipped
* by this version.
*
*		PKTBYTE_COMMENT = 14,
*/
				PKTBYTE_PUBSUBKEY = 14,
				/* 15 is reserved for internal use in the ring parsing code */
				/* 16 and up use the new packet header formats */
				PKTBYTE_COMMENT = 16
};


/* Critical bit means we must handle this subpacket */
#define SIGSUBF_CRITICAL		0x80

#ifdef __cplusplus
}
#endif

#endif /* PGPPKTBYTE_H */
