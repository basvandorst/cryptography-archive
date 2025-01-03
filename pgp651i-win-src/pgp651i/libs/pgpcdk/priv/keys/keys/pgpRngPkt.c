/*
 * Low-level keyring packet handling routines.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPkt.c,v 1.11 1998/03/14 00:28:25 hal Exp $
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
#include "pgpMem.h"

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
 * Helper function for pktByteGet; read new-style length, returning
 * length and also length of first subpacket.  If no subpackets are used
 * the second length will be the same as the first.
 */
static int
pktLengthNew(PGPFile *f, PGPSize *lenp, PGPSize *len1p)
{
	PGPBoolean final = FALSE;
	long fpos = 0;
	PGPByte c;				/* Length character we read */
	PGPByte arr4[4];		/* For reading four byte lengths */
	PGPSize len = 0;		/* Length we return */
	PGPSize len0 = 0;		/* Length of subpacket */
	PGPSize lenfirst = 0;	/* Length of first subpacket, if more than 1 */
	PGPError err = kPGPError_NoErr;

	do {
		if (pgpFileRead(&c, 1, f) != 1)
			goto error;
		len0 = c;
		if (len0 < 0xc0) {
			/* len0 is the length */
			len += len0;
			final = TRUE;
		} else if ((len0 & 0xe0) == 0xc0) {
			len0 &= 0x3f;
			if (pgpFileRead(&c, 1, f) != 1)
				goto error;
			len += (len0 << 8) + (PGPSize)c + 192;
			final = TRUE;
		} else if (len0 == 0xff) {
			if (pgpFileRead(arr4, 4, f) != 4)
				goto error;
			len += (((((arr4[0]<<8)|arr4[1])<<8)|arr4[2])<<8)|arr4[3];
			final = TRUE;
		} else {
			/* Indeterminate length.  Count partial size and add up to total */
			len0 = 1 << (len0 & 0x1f);
			len += len0;
			/* Remember first packet position and size */
			if (lenfirst == 0) {
				pgpAssert (len0 != 0);
				fpos = pgpFileTell(f);
				lenfirst = len0;
			}
			err = pgpFileSeek( f, len0, 1 );
			if( IsPGPError( err ) ) {
				pgpFileSeek( f, fpos, 0 );
				return err;
			}
			/* Loop for next subpacket */
		}
	} while (!final);

	if (lenfirst != 0) {
		err = pgpFileSeek( f, fpos, 0 );
		if( IsPGPError( err ) )
			return err;
		*len1p = lenfirst;
	} else {
		*len1p = len;
	}
	*lenp = len;
	return kPGPError_NoErr;

error:
	if (!pgpFileError(f))
		return kPGPError_EOF;
	pgpFileClearError(f);
	return kPGPError_ReadFailed;
}

/*
 * Returns packet byte, 0 on normal EOF, or error code < 0.
 * Returns length of whole packet, plus length of first sub-packet for
 * case of indefinite-length packets.
 */
int
pktByteGet(PGPFile *f, PGPSize *lenp, PGPSize *len1p, PGPSize *posp)
{
	PGPByte pktbyte;
	PGPByte buf4[4];
	PGPSize len = 0;

	if (posp) {
		*posp = pgpFileTell(f);
		if (*posp == (PGPSize)-1)
			return kPGPError_FileOpFailed;
	}

	if (pgpFileRead(&pktbyte, 1, f) != 1)
		return pgpFileError(f) ? kPGPError_ReadFailed : 0;

	if (IS_OLD_PKTBYTE(pktbyte)) {
		switch(PKTBYTE_LLEN(pktbyte)) {
		  case 3:
			/* Indeterminate length, can't handle that */
			return kPGPError_BadPacket;
		  case 2:
			if (pgpFileRead(buf4, 4, f) != 4)
				goto error;
			len = (PGPSize)buf4[0]<<24 | (PGPSize)buf4[1]<<16 |
						 (PGPSize)buf4[2]<<8 | (PGPSize)buf4[3];
			break;
		  case 1:
			if (pgpFileRead(buf4, 2, f) != 2)
				goto error;
			len = (PGPSize)buf4[0]<<8 | (PGPSize)buf4[1];
			break;
		  case 0:
			if (pgpFileRead(buf4, 1, f) != 1)
				goto error;
			len = (PGPSize)buf4[0];
			break;
		}
	} else if (IS_NEW_PKTBYTE(pktbyte)) {
		PGPError err = pktLengthNew( f, lenp, len1p );
		if( IsPGPError( err ) )
			return err;
		return (int)pktbyte;
	} else
		return kPGPError_BadPacket;

	*lenp = *len1p = len;
	return (int)pktbyte;

error:
	if (!pgpFileError(f))
		return kPGPError_EOF;
	pgpFileClearError(f);
	return kPGPError_ReadFailed;
}


/*
 * Read the body of a ring packet.  Len is the length of the whole packet,
 * and len1 is the length of the first subpacket.  If these are equal we
 * just read raw data.  Else we read len1 bytes and then deal with the
 * additional subpackets.  We return the number of bytes read.
 */
PGPSize
pktBodyRead(void *ptr, PGPSize len, PGPSize len1, PGPFile *f)
{
	PGPBoolean final;
	PGPSize nread = 0;
	PGPSize n;
	PGPByte c;
	PGPByte arr4[4];

	if (len == len1) {
		return pgpFileRead (ptr, len, f);
	}

	/* Must read multiple subpackets */
	final = FALSE;
	for ( ; ; ) {
		n = pgpFileRead (ptr, len1, f);
		if (n != len1)
			break;
		nread += len1;
		if (final)
			break;
		ptr = (PGPByte *)ptr + len1;
		if (pgpFileRead(&c, 1, f) != 1)
			break;
		len1 = c;
		if (len1 < 0xc0) {
			/* One byte length, final packet */
			final = TRUE;
		} else if ((len1 & 0xe0) == 0xc0) {
			/* Two byte length, final packet */
			len1 &= 0x3f;
			if (pgpFileRead(&c, 1, f) != 1)
				break;
			len1 = (len1 << 8) + (PGPSize)c + 192;
			final = TRUE;
		} else if (len1 == 0xff) {
			/* Four byte length, final packet */
			if (pgpFileRead(arr4, 4, f) != 4)
				break;
			len1 = (((((arr4[0]<<8)|arr4[1])<<8)|arr4[2])<<8)|arr4[3];
			final = TRUE;
		} else {
			/* Indeterminate length, non-final packet */
			len1 = 1 << (len1 & 0x1f);
		}
	}

	return nread;
}


/*
 * Returns the number of bytes which will be output by pktBytePut with the
 * corresponding args.
 */
int
pktByteLen (PGPByte pktbyte, PGPSize len)
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
		} else if (PKTLEN_TWO_BYTES(len)) {
			return 3;
		} else {
			return 6;
		}
	}
	
	/* Avoid warning */
	pgpAssert(0);
	return 0;
}

/*
 * Returns the packet byte actually written, whose llen field may be greater
 * than the one passed in, but will not be less, or error code < 0.
 * Also optionally returns the size of the first subpacket, for cases where
 * multiple subpackets are being used.
 */
int
pktBytePut(PGPFile *f, PGPByte pktbyte, PGPSize len, PGPSize *len1p)
{
	PGPByte		buf6[6];
	PGPUInt32	llen;
	PGPByte		len1;

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
			buf6[0] = (PGPByte)(len >> 24);
			buf6[1] = (PGPByte)(len >> 16);
			buf6[2] = (PGPByte)(len >>  8);
			buf6[3] = (PGPByte)(len      );
			if (pgpFileWrite(buf6, 4, f) != 4)
				return kPGPError_WriteFailed;
			break;
		  case 1:
			buf6[0] = (PGPByte)(len >>  8);
			buf6[1] = (PGPByte)(len      );
			if (pgpFileWrite(buf6, 2, f) != 2)
				return kPGPError_WriteFailed;
			break;
		  case 0:
			buf6[0] = (PGPByte)len;
			if (pgpFileWrite(buf6, 1, f) != 1)
				return kPGPError_WriteFailed;
			break;
		}
		if (len1p)
			*len1p = len;
	} else {
		pgpAssert(IS_NEW_PKTBYTE(pktbyte));
		buf6[0] = (PGPByte)pktbyte;
		if (PKTLEN_TWO_BYTES(len)) {
			if (PKTLEN_ONE_BYTE(len)) {
				buf6[1] = PKTLEN_1BYTE(len);
				llen = 2;
			} else {
				buf6[1] = PKTLEN_BYTE0(len);
				buf6[2] = PKTLEN_BYTE1(len);
				llen = 3;
			}
			if (len1p)
				*len1p = len;
		} else {
#if 0
// Must change value in pktBodyWrite also
			/* Use four byte mode */
			buf6[1] = 0xff;
			buf6[2] = (PGPByte)(len >> 24);
			buf6[3] = (PGPByte)(len >> 16);
			buf6[4] = (PGPByte)(len >>  8);
			buf6[5] = (PGPByte)(len >>  0);
			llen = 6;
			if (len1p)
				*len1p = len;
#else
/* This is used if we are using variable length subpackets */
			/* Just output length for first subpacket */
			len1 = 0;
			while (len>>=1)
				++len1;
			buf6[1] = PKTLEN_PARTIAL(len1);
			llen = 2;
			pgpAssert(len1p);
			*len1p = 1<<len1;
#endif
		}
		if (pgpFileWrite(buf6, llen, f) != llen)
			return kPGPError_WriteFailed;
	}
	return pktbyte;
}

/*
 * Write the body of a ring packet.  Len is the length of the whole packet,
 * and len1 is the length of the first subpacket.  If these are equal we
 * just rite raw data.  Else we write len1 bytes and then deal with the
 * additional subpackets.  We return the number of bytes written.
 */
PGPSize
pktBodyWrite(void const *ptr, PGPSize len, PGPSize len1, PGPFile *f)
{
	PGPSize nwritten = 0;
	PGPSize n;
	PGPByte buf5[5];
	PGPUInt32 llen;

	if (len == len1) {
		return pgpFileWrite (ptr, len, f);
	}

	/* Must write multiple subpackets */
	for ( ; ; ) {
		n = pgpFileWrite (ptr, len1, f);
		if (n != len1)
			break;
		nwritten += len1;
		ptr = (PGPByte *)ptr + len1;
		len -= len1;
		if (len == 0)
			break;
		if (PKTLEN_TWO_BYTES(len)) {
			if (PKTLEN_ONE_BYTE(len)) {
				buf5[0] = PKTLEN_1BYTE(len);
				llen = 1;
			} else {
				buf5[0] = PKTLEN_BYTE0(len);
				buf5[1] = PKTLEN_BYTE1(len);
				llen = 2;
			}
			len1 = len;
		} else {
#if 0
// Must change value in pktBytePut also
			buf5[0] = 0xff;
			buf5[1] = (PGPByte)(len >> 24);
			buf5[2] = (PGPByte)(len >> 16);
			buf5[3] = (PGPByte)(len >>  8);
			buf5[4] = (PGPByte)(len >>  0);
			llen = 5;
			len1 = len;
#else
/* This is used if we are using variable length subpackets */
			/* Output length for next subpacket */
			PGPSize tlen = len;
			len1 = 0;
			while (tlen>>=1)
				++len1;
			buf5[0] = PKTLEN_PARTIAL(len1);
			llen = 1;
			len1 = 1 << len1;
#endif
		}
		if (pgpFileWrite(buf5, llen, f) != llen)
			break;
	}

	return nwritten;
}



/*
 * Compare data from a file to a supplied buffer, returning < 0
 * on a read error, 0 on equal, and 1 on unequal.  Len is the length
 * of the data, len1 is the size of the first subpacket to compare to.
 * We may be called with len less than the size of the whole packet.
 */
int
fileUnequalBuf(PGPFile *f, void const *buf, PGPSize len, PGPSize len1)
{
	char buf2[256];
	PGPSize amount;
	PGPByte c;
	PGPByte arr4[4];

	while (len) {
		if (len < len1)
			len1 = len;
		len -= len1;
		while (len1) {
			amount = len1 < sizeof(buf2) ? len1 : sizeof(buf2);
			amount = pgpFileRead(buf2, amount, f);
			if (!amount)
				return pgpFileError(f) ? kPGPError_ReadFailed
							   : kPGPError_EOF;
			if (memcmp(buf, buf2, amount) != 0)
				return 1;

			len1 -= amount;
			buf = (PGPByte *)buf + amount;
		}
		if (len) {
			/* Read next subpacket size */
			if (pgpFileRead(&c, 1, f) != 1)
				return pgpFileError(f) ? kPGPError_ReadFailed
							   : kPGPError_EOF;
			len1 = c;
			if (len1 < 0xc0) {
				/* One byte length, final packet */
			} else if ((len1 & 0xe0) == 0xc0) {
				/* Two byte length, final packet */
				len1 &= 0x3f;
				if (pgpFileRead(&c, 1, f) != 1)
					return pgpFileError(f) ? kPGPError_ReadFailed
							   : kPGPError_EOF;
				len1 = (len1 << 8) + (PGPSize)c + 192;
			} else if (len1 == 0xff) {
				/* Four byte length, final packet */
				if (pgpFileRead(arr4, 4, f) != 4)
					return pgpFileError(f) ? kPGPError_ReadFailed
							   : kPGPError_EOF;
				len1 = (((((arr4[0]<<8)|arr4[1])<<8)|arr4[2])<<8)|arr4[3];
			} else {
				/* Indeterminate length, non-final packet */
				len1 = 1 << (len1 & 0x1f);
			}
		}
	}
	return 0;	/* Match */
}




/*
 * Returns the number of size bytes which will be output by
 * pktSubpacketCreate with the corresponding length.
 */
PGPSize
pktSubpacketByteLen (PGPSize len)
{

#if 1
	/* Use fixed size headers: 0xff then four bytes of length */
	(void) len;
	return 5;
#else
	/* Use variable sized headers */
	PGPSize count = 0;

	for ( ; ; ) {
		PGPSize tlen;
		PGPUInt32 len1;

		if (PKTLEN_ONE_BYTE(len)) {
			return count+1;
		} else {
			return count+2;
		}

		/* Calculate length for next subpacket */
		tlen = len;
		len1 = 0;
		while (tlen>>=1)
			++len1;
		++count;
		pgpAssert ((PGPSize)(1 << len1) <= len);
		len -= 1 << len1;
		if (len == 0)
			return count;
	}
#endif
}


/*
 * Create a subpacket with the given type and data, outputting to the
 * specified buffer.  Return number of bytes written, which should equal
 * len + 1 plus the amount returned by pktSubpacketByteLen( len+1 ).
 */
PGPSize
pktSubpacketCreate (PGPByte type, PGPByte const *data, PGPSize len,
	PGPByte *pktbuf)
{
#if 1
	/* Use fixed size buffer headers */
	*pktbuf++ = 0xff;
	*pktbuf++ = (PGPByte) ((len+1) >> 24);
	*pktbuf++ = (PGPByte) ((len+1) >> 16);
	*pktbuf++ = (PGPByte) ((len+1) >>  8);
	*pktbuf++ = (PGPByte) ((len+1) >>  0);
	*pktbuf++ = type;
	pgpCopyMemory (data, pktbuf, len);
	return len + 1 + 5;
#else
	/* Use variable sized headers */
	PGPSize nwritten = 0;
	PGPSize len1 = 0;
	PGPBoolean first;

	/* Count extra type byte */
	len += 1;
	first = 1;

	while (len) {
		if (PKTLEN_TWO_BYTES(len)) {
			if (PKTLEN_ONE_BYTE(len)) {
				*pktbuf++ = PKTLEN_1BYTE(len);
				nwritten += 1;
			} else {
				*pktbuf++ = PKTLEN_BYTE0(len);
				*pktbuf++ = PKTLEN_BYTE1(len);
				nwritten += 2;
			}
			len1 = len;
		} else {
			/* Output length for next subpacket */
			PGPSize tlen = len;
			len1 = 0;
			while (tlen>>=1)
				++len1;
			*pktbuf++ = PKTLEN_PARTIAL(len1);
			len1 = 1 << len1;
			nwritten += 1;
		}
		if( first ) {
			*pktbuf++ = type;
			len1 -= 1;
			len -= 1;
			nwritten += 1;
			first = FALSE;
		}
		pgpCopyMemory (data, pktbuf, len1);
		pktbuf += len1;
		data += len1;
		nwritten += len1;
		len -= len1;
	}

	return nwritten;
#endif
}
