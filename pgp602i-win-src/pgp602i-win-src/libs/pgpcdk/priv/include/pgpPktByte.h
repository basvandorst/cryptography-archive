/*
 * pgpPktByte.h -- Definitions of packet byte types
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpPktByte.h,v 1.7 1998/06/03 20:10:42 hal Exp $
 */
#ifndef Included_pgpPktByte_h
#define Included_pgpPktByte_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#define IS_OLD_PKTBYTE(pktbyte)  (((pktbyte) & 0xC0) == 0x80)
#define IS_NEW_PKTBYTE(pktbyte) (((pktbyte) & 0xC0) == 0xC0)
#define OLD_PKTBYTE_TYPE(pktbyte) (((pktbyte) >> 2) & 0xF)
#define NEW_PKTBYTE_TYPE(pktbyte) ((pktbyte) & 0x3F)
#define PKTBYTE_TYPE(pktbyte) (IS_OLD_PKTBYTE (pktbyte) ? \
			       OLD_PKTBYTE_TYPE(pktbyte) : \
			       NEW_PKTBYTE_TYPE(pktbyte))
#define PKTBYTE_LLEN(pktbyte) ((pktbyte) & 3)
#define PKTBYTE_BUILD(type, llen) ((type)>0xf ? \
	PKTBYTE_BUILD_NEW(type) : PKTBYTE_BUILD_OLD(type, llen))
#define PKTBYTE_BUILD_OLD(type, llen) ((PGPByte)(0x80 | ((type) & 0xF) <<2 \
							| (llen)))
#define PKTBYTE_BUILD_NEW(type) ((PGPByte)(0xC0 | ((type) & 0x3F)))
#define LLEN_TO_BYTES(llen) ((1 << (llen)) & 7)

/*
 * For the new packet formats -- macros for creating the length byte(s)
 * of terminating subpackets.  The format is:
 *	Format			Length		Macro
 * 	0sssssss		0-127		PKTLEN_1BYTE()
 * 	10ssssss		128-191		PKTLEN_1BYTE()
 *	110sssss ssssssss	192-8383	PKTLEN_BYTE0(),PKTLEN_BYTE1()
 *
 * PKTLEN_ONE_BYTE() is used to determine if the length is 1 byte (use
 * PKTLEN_1BYTE()) or two bytes (use PKTLEN_BYTE0() then
 * PKTLEN_BYTE1()).  PKTLEN_TWO_BYTES() is false if it needs more than 2.
 */
#define PKTLEN_ONE_BYTE(len)	((len) < 192)
#define PKTLEN_TWO_BYTES(len)	((len) < 8384)
#define PKTLEN_1BYTE(len)	(PGPByte)(len & 0xFF)
#define PKTLEN_BYTE0(len)	(PGPByte)(0xC0 + (((len-192) >> 8) & 0x1F))
#define PKTLEN_BYTE1(len)	(PGPByte)((len-192) & 0xFF)
#define PKTLEN_PARTIAL(po2)	(PGPByte)(0xE0 + (po2))

/*
 * A non-terminating subpacket (followed by another subpacket) is
 * a power of two bytes long and has a length byte encoded as
 * follows:
 *      111xxxxx		2^0-2^31 bytes
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
	PKTBYTE_OLDLITERAL = 10,	/* pkt length doesn't include filename, etc. */
	PKTBYTE_LITERAL = 11,
	PKTBYTE_TRUST = 12,
	PKTBYTE_NAME = 13,
/* We are replacing the old comment packets with the new subkey
 * packets.  This will allow older versions of PGP to skip over the
 * subkey packets gracefully.  The new comment packets will be skipped
 * by this version.
 *
 *	PKTBYTE_COMMENT = 14,
 */
	PKTBYTE_PUBSUBKEY = 14,
	/* 15 is reserved for internal use in the ring parsing code */
	/* 16 and up use the new packet header formats */
	PKTBYTE_COMMENT = 16,
	PKTBYTE_ATTRIBUTE = 17,
	/* Update this to be the maximum known packet tag */
	PKTBYTE_MAXIMUM = PKTBYTE_ATTRIBUTE
};

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPktByte_h */
