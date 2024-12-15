/*
 * util.h -- Miscellaneous defines
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Written by Mark H. Weaver
 *
 * $Id: util.h,v 1.23 1997/11/12 23:28:56 mhw Exp $
 */

#ifndef UTIL_H
#define UTIL_H 1

typedef unsigned long	word32;
typedef unsigned short	word16;
typedef unsigned char	byte;

#define FMT32	"%08lx"
#define FMT16	"%04x"
#define FMT8	"%02x"

#define TAB_CHAR		'\244'	/* Currency symbol, like o in top of x */
#define TAB_STRING		"\244"
#define TAB_PAD_CHAR	' '		/* The fact that this is space has leaked. */
#define TAB_PAD_STRING	" "		/* It may not be freely changed. */
#define FORMFEED_CHAR	'\245'	/* Yen symbol, like = on top of Y */
#define FORMFEED_STRING	"\245"
#define SPACE_CHAR		'\267'	/* Middle dot, or bullet */
#define SPACE_STRING	"\267"
#define CONTIN_CHAR		'\266'	/* Pilcrow (paragraph symbol) */
#define CONTIN_STRING	"\266"

#define BYTES_PER_LINE	60		/* When using radix 64 */

#define LINES_PER_PAGE	72		/* Exclusive of 2 header lines */
#define LINE_LENGTH		80
#define PREFIX_LENGTH	7		/* Length of prefix, including the space */

#define HDR_PREFIX_CHAR		'-'
#define RADIX64_END_CHAR	'-'

typedef struct EncodeFormat		EncodeFormat;
typedef word32					CRC;
typedef word16					CRCFragment;

typedef struct
{
	CRC			table[256];
	int			bits;
	CRC			poly;
	CRC			highBit;
} CRCPoly;

struct EncodeFormat
{
	EncodeFormat const *nextFormat;
	char				headerTypeChar;
	char const *		digits;
	signed char const *	digitsInv;
	int					bitsPerDigit;
	int					radix;
	CRCPoly const *		lineCRC;
	CRCPoly	const *		pageCRC;
	int					runningCRCBits;
	int					runningCRCShift;
	int					runningCRCMask;
};


#define HDR_ENC_LENGTH		19		/* Length of encoded prefix on header */

#define HDR_VERSION_BITS	4
#define HDR_FLAG_BITS		8
/* Page CRC bits omitted, since it's not constant */
#define HDR_TABWIDTH_BITS	4
#define HDR_PRODNUM_BITS	12
#define HDR_FILENUM_BITS	16


/* Enough to hold one whole page of munged data */
/* There is no point making this excessively too large */
#define PAGE_BUFFER_SIZE	8192

#if PAGE_BUFFER_SIZE < (LINES_PER_PAGE + 2) * (LINE_LENGTH + PREFIX_LENGTH + 2)
#error PAGE_BUFFER_SIZE is too small
#endif


/* Header flags */
#define HDR_FLAG_LASTPAGE	0x01	/* Indicates last page of file */


#define elemsof(array) (sizeof(array)/sizeof(*(array)))


extern char const	hexDigits[];
extern char const	radix64Digits[];

extern signed char	hexDigitsInv[256];
extern signed char	radix64DigitsInv[256];

extern CRCPoly		crcCCITTPoly, crc24Poly, crc32Poly;

extern EncodeFormat const		hexFormat, radix64Format;
extern EncodeFormat const *		firstFormat;


#define HexDigitValue(ch)		hexDigitsInv[(byte)(ch)]
#define Radix64DigitValue(ch)	radix64DigitsInv[(byte)(ch)]

/* Returns the number of chars needed to encode the given number of bits */
#define EncodedLength(fmt, numBits)	\
		(((numBits) + (fmt)->bitsPerDigit - 1) / (fmt)->bitsPerDigit)
#define EncodeDigit(fmt, value)		((fmt)->digits[value])
#define DecodeDigit(fmt, digit)		((fmt)->digitsInv[(byte)digit])

#define AdvanceCRC(poly, crc, b)	\
		((crc) >> 8) ^ (poly)->table[((crc) ^ (b)) & 0xFF]

#define RunningCRCFromPageCRC(fmt, pageCRC)	\
		(((pageCRC) >> (fmt)->runningCRCShift) & (fmt)->runningCRCMask)


CRC CalculateCRC(CRCPoly const *poly, CRC crc,
				 byte const *buffer, size_t length);
CRC ReverseCRC(CRCPoly const *poly, CRC crc, byte b);

/* Returns the number of chars encoded */
int EncodeCheckDigits(EncodeFormat const *fmt, word32 num,
					  int numBits, char *dest);

/* Returns 1 if there's an error */
int DecodeCheckDigits(EncodeFormat const *fmt, char const *src, char **endPtr,
					  int numBits, word32 *valuePtr);

EncodeFormat const *FindFormat(char headerTypeChar);

void InitUtil();


#endif /* !UTIL_H */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
