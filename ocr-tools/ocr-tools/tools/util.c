/*
 * util.c -- Miscellaneous shared code/data
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Written by Mark H. Weaver
 *
 * $Id: util.c,v 1.11 1997/11/07 00:44:10 mhw Exp $
 */

#include <stdlib.h>
#include "util.h"

char const hexDigits[] = "0123456789abcdef";
char const radix64Digits[] =
#if 0	/* Standard */
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#else	/* Modified form that avoids hard-to-OCR characters */
	"ABCDEFGHIJKLMNPQRSTVWXYZabcdehijklmnpqtuwy145689\\^!#$%&*+=/:<>?@";
#endif

signed char hexDigitsInv[256];
signed char radix64DigitsInv[256];

/* teun: moved intitialisation of all three CRCPoly's to initUtil() */

/* CRC-CCITT: x^16 + x^12 + x^5 + 1 */
CRCPoly	crcCCITTPoly;
/*
 * PRZ's magic 24-bit polynomial - (x+1) * (irreducible of degree 23)
 * x^24 +x^23 +x^18 +x^17 +x^14 +x^11 +x^10 +x^7 +x^6 +x^5 +x^4 +x^3 +x +1
 * (Developed by Neal Glover).  Note: this is bit-reversed from the form
 * used in PGP, 0x1864cfb.
 */
CRCPoly	crc24Poly;
/* CRC-32: x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1 */
CRCPoly	crc32Poly;

EncodeFormat const	hexFormat =
{
	NULL,				/* nextFormat */
	'-',				/* headerTypeChar */
	hexDigits,			/* digits */
	hexDigitsInv,		/* digitsInv */
	4,					/* bitsPerDigit */
	16,					/* radix */
	&crcCCITTPoly,		/* lineCRC */
	&crc32Poly,			/* pageCRC */
	8,					/* runningCRCBits */
	24,					/* runningCRCShift */
	0xFF				/* runningCRCMask */
};

EncodeFormat const	radix64Format =
{
	&hexFormat,			/* nextFormat */
	'A',				/* headerTypeChar */
	radix64Digits,		/* digits */
	radix64DigitsInv,	/* digitsInv */
	6,					/* bitsPerDigit */
	64,					/* radix */
	&crc24Poly,			/* lineCRC */
	&crc32Poly,			/* pageCRC */
	12,					/* runningCRCBits */
	20,					/* runningCRCShift */
	0xFFF				/* runningCRCMask */
};

EncodeFormat const *	firstFormat = &radix64Format;


static void InitCRCPoly(CRCPoly *poly)
{
	int		i, oneBit;
	CRC		crc = 1;

	poly->table[0] = 0;
	for (oneBit = 0x80; oneBit > 0; oneBit >>= 1) {
		crc = (crc >> 1) ^ ((crc & 1) ? poly->poly : 0);
		for (i = 0; i < 0x100; i += 2 * oneBit)
			poly->table[i + oneBit] = poly->table[i] ^ crc;
	}
}

CRC CalculateCRC(CRCPoly const *poly, CRC crc,
				 byte const *buffer, size_t length)
{
	while (length--)
		crc = (crc >> 8) ^ poly->table[(crc & 0xFF) ^ (*buffer++)];
	return crc;
}

CRC ReverseCRC(CRCPoly const *poly, CRC crc, byte b)
{
	int		i, highBit = poly->highBit;

	for (i = 0; i < 8; i++) {
		if (crc & highBit)		/* highBit is 2^(poly->bits-1) */
			crc = ((crc ^ poly->poly) << 1) ^ 1;
		else
			crc <<= 1;
	}
	return crc ^ b;
}

static void InitDigitsInv(char const *digits, signed char *digitsInv)
{
	int		i;

	for (i = 0; i < 256; i++)
		digitsInv[i] = -1;
	for (i = 0; digits[i]; i++)
		digitsInv[(byte)digits[i]] = i;
}

/* Returns the number of chars encoded */
int EncodeCheckDigits(EncodeFormat const *fmt, word32 num,
					  int numBits, char *dest)
{
	int		destLen = EncodedLength(fmt, numBits);
	word32	digitMask = fmt->radix - 1;
	int		i;

	for (i = destLen - 1; i >= 0; i--)
	{
		dest[i] = EncodeDigit(fmt, num & digitMask);
		num >>= fmt->bitsPerDigit;
	}
	return destLen;
}

/* Returns 1 if there's an error */
int DecodeCheckDigits(EncodeFormat const *fmt, char const *src, char **endPtr,
					  int numBits, word32 *valuePtr)
{
	word32	value = 0;
	int		digitValue;
	int		i = EncodedLength(fmt, numBits);

	while (i--)
	{
		digitValue = DecodeDigit(fmt, *src++);
		if (digitValue < 0)
		{
			/* Invalid digit found */
			*valuePtr = 0;
			if (endPtr)
				*endPtr = NULL;
			return 1;
		}
		value = (value << fmt->bitsPerDigit) | digitValue;
	}
	*valuePtr = value;
	if (endPtr)
		*endPtr = (char *)src;
	return 0;
}

EncodeFormat const *FindFormat(char headerTypeChar)
{
	EncodeFormat const *	fmt = firstFormat;

	while (fmt && fmt->headerTypeChar != headerTypeChar)
		fmt = fmt->nextFormat;
	return fmt;
}

void InitUtil()
{
	/* teun: removed "{ }" for MS VC compile */

	crcCCITTPoly.bits = 16;
	crcCCITTPoly.poly = 0x8408;
	crcCCITTPoly.highBit = 0x8000;

	crc24Poly.bits = 24;
	crc24Poly.poly = 0xdf3261;
	crc24Poly.highBit = 0x800000;

	crc32Poly.bits = 32;
	crc32Poly.poly = 0xedb88320;
	crc32Poly.highBit = 0x80000000;

	InitCRCPoly(&crcCCITTPoly);
	InitCRCPoly(&crc24Poly);
	InitCRCPoly(&crc32Poly);
	InitDigitsInv(hexDigits, hexDigitsInv);
	InitDigitsInv(radix64Digits, radix64DigitsInv);
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
