/*
 *                                        _
 * Copyright (c) 1990,1991,1992,1993 Stig Ostholm.
 * All Rights Reserved
 *
 *
 * The author takes no responsibility of actions caused by the use of this
 * software and does not guarantee the correctness of the functions.
 *
 * This software may be freely distributed and modified for non-commercial use
 * as long as the copyright notice is kept. If you modify any of the files,
 * pleas add a comment indicating what is modified and who made the
 * modification.
 *
 * If you intend to use this software for commercial purposes, contact the
 * author.
 *
 * If you find any bugs or porting problems, please inform the author about
 * the problem and fixes (if there are any).
 *
 *
 * Additional restrictions due to national laws governing the use, import or
 * export of cryptographic software is the responsibility of the software user,
 * importer or exporter to follow.
 *
 *
 *                                              _
 *                                         Stig Ostholm
 *                                         Chalmers University of Technology
 *                                         Department of Computer Engineering
 *                                         S-412 96 Gothenburg
 *                                         Sweden
 *                                         ----------------------------------
 *                                         Email: ostholm@ce.chalmers.se
 *                                         Phone: +46 31 772 1703
 *                                         Fax:   +46 31 772 3663
 */

#include	<stdio.h>
#include	"config.h"
#include	"fips_S.h"

/*
 * Bit rotate for the four least significant bits.
 */

static const unsigned char	r[16] = {
					0x0, /* 0000 -> 0000 */
					0x8, /* 0001 -> 1000 */
					0x4, /* 0010 -> 0100 */
					0xc, /* 0011 -> 1100 */
					0x2, /* 0100 -> 0010 */
					0xa, /* 0101 -> 1010 */
					0x6, /* 0110 -> 0110 */
					0xe, /* 0111 -> 1110 */
					0x1, /* 1000 -> 0001 */
					0x9, /* 1001 -> 1001 */
					0x5, /* 1010 -> 0101 */
					0xd, /* 1011 -> 1101 */
					0x3, /* 1100 -> 0011 */
					0xb, /* 1101 -> 1011 */
					0x7, /* 1110 -> 0111 */
					0xf  /* 1111 -> 1111 */
				};

/*
 * This program generates a macro for us as the S - selection.
 *
 *		S(B)
 *
 * The input is 48 bits stored in an array of unsigned character `B[0--7]'
 * with six bit in the six least significant bits of each character.
 * The least significant bit of each character is the first of each with
 * the least significant bit of `B[0]' as the first in the input block.
 *
 * The output is 32 bits in an unsigned long with the least significant bit
 * as the first.
 */

#define DES_KS_BITS_PER_BYTE \
	(UNSIGNED_CHAR_BITS - ((DES_BITS - DES_KS_BITS) / DES_BLOCK_BYTES))

#define DES_KS_BYTE_MAX = ((0x1 << DES_KS_BITS_PER_BYTE) - 1)

main()
{
	register int	i, j, sb, n, v;


        VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");
	VOID printf("static const unsigned long\ts[%d][%d] = {\n", S_BOXES,
		DES_KS_BYTE_MAX + 1);
	for (sb = 0; sb < S_BOXES; sb++) {
		VOID printf("\t\t\t{ /* S%d                (i,  j) -> out */\n",
			    sb + 1);
		for (n = 0; n < DES_KS_BYTE_MAX + 1; n++) {
			i = ((n & 0x01) << 1) | ((n & 0x20) >> 5);
			j = r[(n & 0x1e) >> 1];
			v = s[sb][i][j];
			VOID printf("\t\t\t\t0x%08lxl%c /* (%d, %2d) -> %2d */\n",
				    ((unsigned long) r[v]) <<
				    (sb * (UNSIGNED_CHAR_BITS / 2)),
				    (n + 1 < DES_BITS) ? ',' : ' ', i, j, v);
		}
		VOID printf(((sb + 1) < S_BOXES) ? "\t\t\t},\n" : "\t\t\t}\n");
	}
	VOID printf("\t\t};\n\n#define S(B) ( \\\n");
	for (sb = 0; sb < S_BOXES; sb++) {
		VOID printf("\ts[%d][B[%d]]", sb, sb);
		VOID printf((sb + 1 < S_BOXES) ? " | \\\n" : " \\\n");
	}
	VOID printf(")\n");
	exit(0);
}
