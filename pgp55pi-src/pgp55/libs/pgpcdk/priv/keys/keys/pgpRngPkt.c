/*
 * Low-level keyring packet handling routines.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPkt.c,v 1.9 1997/06/25 19:40:45 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>	/* For memset() and memcmp() */

#include "pgpDebug.h"
#include "pgpPktByte.h"
#include "pgpRngPkt.h"
#include "pgpErrors.h"
#include "pgpUsuals.h"
#include "pgpFile.h"

#ifndef NULL
#define NULL 0
#endif

#ifdef DEADCODE
char const * const packet_type[16] = {
	"0: Unknown packet type",
	"1: Public-key encrypted packet",
	"2: Signature packet",
	"3: Unknown packet type",
	"4: Unknown packet type",
	"5: Secret key",
	"6: Public key",
	"7: Unknown packet type",
	"8: Compressed packet",
	"9: Conventional-key encrypted",
	"A: Unknown packet type",
	"B: Literal packet",
	"C: Trust packet",
	"D: Key name packet",
	"E: Comment packet",
	"F: Unknown packet type"
};
#endif

/*
 * Returns packet byte, 0 on normal EOF, or error code < 0.
 */
int
pktByteGet(PGPFile *f, PGPUInt32 *lenp, PGPUInt32 *posp)
{
	PGPByte pktbyte;
	PGPByte buf4[4];
	PGPUInt32 len = 0;

	if (posp) {
		*posp = pgpFileTell(f);
		if (*posp == (PGPUInt32)-1)
			return kPGPError_FileOpFailed;
	}

	if (pgpFileRead(&pktbyte, 1, f) != 1)
		return pgpFileError(f) ? kPGPError_ReadFailed : 0;

	if (IS_OLD_PKTBYTE(pktbyte)) {
		switch(PKTBYTE_LLEN(pktbyte)) {
		  case 3:
			/* Indeterminate length, can't handle that yet */
			return kPGPError_BadPacket;
		  case 2:
			if (pgpFileRead(buf4, 4, f) != 4)
				goto error;
			len = (PGPUInt32)buf4[0]<<24 | (PGPUInt32)buf4[1]<<16 |
				(PGPUInt32)buf4[2]<<8 | (PGPUInt32)buf4[3];
			break;
		  case 1:
			if (pgpFileRead(buf4, 2, f) != 2)
				goto error;
			len = (PGPUInt32)buf4[0]<<8 | (PGPUInt32)buf4[1];
			break;
		  case 0:
			if (pgpFileRead(buf4, 1, f) != 1)
				goto error;
			len = (PGPUInt32)buf4[0];
			break;
		}
	} else if (IS_NEW_PKTBYTE(pktbyte)) {
		if (pgpFileRead(buf4, 1, f) != 1)
			goto error;
		len = (PGPUInt32)buf4[0];
		if (len < 0xc0) {
			/* len is the length */
		} else if ((len & 0xe0) == 0xc0) {
			len &= 0x3f;
			if (pgpFileRead(buf4, 1, f) != 1)
				goto error;
			len = (len << 8) + (PGPUInt32)buf4[0] + 192;
		} else {
			/* Indeterminate length, can't handle that yet */
			return kPGPError_BadPacket;
		}
	} else
		return kPGPError_BadPacket;

	*lenp = len;
	return (int)pktbyte;

error:
	if (!pgpFileError(f))
		return kPGPError_EOF;
	pgpFileClearError(f);
	return kPGPError_ReadFailed;
}

/*
 * Returns the number of bytes which will be output by pktBytePut with the
 * corresponding args.
 */
int
pktByteLen (PGPByte pktbyte, PGPUInt32 len)
{
	if (IS_OLD_PKTBYTE(pktbyte)) {
		/* "Promote" packet byte llen field */
		/* if necessary to accomodate length */
		if (len == 0xfffffff) {
			pktbyte |= 3;
		} else if (len > 0xffff) {
			if (!(pktbyte & 2))
				pktbyte = (pktbyte & ~3) | 2;
		} else if (len > 0xff) {
			if ((pktbyte & 3) == 0)
				pktbyte |= 1;
		}
		switch(pktbyte & 3) {
		  case 3:
			  return 1;
		  case 2:
			  return 5;
		  case 1:
			  return 3;
		  case 0:
			  return 2;
		}
	} else {
		pgpAssert(IS_NEW_PKTBYTE(pktbyte));
		if (PKTLEN_ONE_BYTE(len)) {
			return 2;
		} else {
			return 3;
		}
	}
	
	/* Avoid warning */
	pgpAssert(0);
	return 0;
}

/*
 * Returns the packet byte actually written, whose llen field may be greater
 * than the one passed in, but will not be less, or error code < 0.
 */
int
pktBytePut(PGPFile *f, PGPByte pktbyte, PGPUInt32 len)
{
	PGPByte buf4[4];
	unsigned llen;

	if (IS_OLD_PKTBYTE(pktbyte)) {
		/* "Promote" packet byte llen field */
		/* if necessary to accomodate length */
		if (len == 0xfffffff) {
			pktbyte |= 3;
		} else if (len > 0xffff) {
			if (!(pktbyte & 2))
				pktbyte = (pktbyte & ~3) | 2;
		} else if (len > 0xff) {
			if ((pktbyte & 3) == 0)
				pktbyte |= 1;
		}

		if (pgpFileWrite(&pktbyte, 1, f) != 1)
			return kPGPError_WriteFailed;

		switch(pktbyte & 3) {
		  case 3:
			/* no length field */
	  		break;
		  case 2:
			buf4[0] = (PGPByte)(len >> 24);
			buf4[1] = (PGPByte)(len >> 16);
			buf4[2] = (PGPByte)(len >>  8);
			buf4[3] = (PGPByte)(len      );
			if (pgpFileWrite(buf4, 4, f) != 4)
				return kPGPError_WriteFailed;
			break;
		  case 1:
			buf4[0] = (PGPByte)(len >>  8);
			buf4[1] = (PGPByte)(len      );
			if (pgpFileWrite(buf4, 2, f) != 2)
				return kPGPError_WriteFailed;
			break;
		  case 0:
			buf4[0] = (PGPByte)len;
			if (pgpFileWrite(buf4, 1, f) != 1)
				return kPGPError_WriteFailed;
			break;
		}
	} else {
		pgpAssert(IS_NEW_PKTBYTE(pktbyte));
		buf4[0] = (PGPByte)pktbyte;
		if (PKTLEN_ONE_BYTE(len)) {
			buf4[1] = PKTLEN_1BYTE(len);
			llen = 2;
		} else {
			buf4[1] = PKTLEN_BYTE0(len);
			buf4[2] = PKTLEN_BYTE1(len);
			llen = 3;
		}
		if (pgpFileWrite(buf4, llen, f) != llen)
			return kPGPError_WriteFailed;
	}
	return pktbyte;
}

/*
 * Compare data from a file to a supplied buffer, returning < 0
 * on a read error, 0 on equal, and 1 on unequal.
 */
int
fileUnequalBuf(PGPFile *f, char const *buf, size_t len)
{
	char buf2[256];
	size_t amount;

	while (len) {
		amount = len < sizeof(buf2) ? len : sizeof(buf2);
		amount = pgpFileRead(buf2, amount, f);
		if (!amount)
			return pgpFileError(f) ? kPGPError_ReadFailed
					       : kPGPError_EOF;
		if (memcmp(buf, buf2, amount) != 0)
			return 1;

		len -= amount;
		buf += amount;
	}
	return 0;	/* Match */
}
