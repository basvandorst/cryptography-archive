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
#include	"fips_def.h"
#include	"fips_P.h"

/*
 * This program generates a macro for us as the permutation.
 *
 *		P(R)
 *
 * The input is an unsigned long `R' with the least significant bit
 * as the first.
 * The output is an unsigned long with the least significant bit as the
 * first.
 */

main()
{
	register int	r, c, block_bit, shift, max_s, min_s;
	unsigned long	st[DES_BITS];


	for (shift = 0; shift < DES_BITS; shift++)
		st[shift] = 0x0l;

	min_s = DES_L_BITS;
	max_s = -DES_L_BITS;
	for (block_bit = r = 0; r < P_ROWS; r++)
		for (c = 0; c < P_COLUMNS; c++) {
			shift = ++block_bit - p[r][c];
			st[DES_L_BITS + shift] |= 0x1l << (p[r][c] - 1);
			if (max_s < shift)
				max_s = shift;
			if (min_s > shift)
				min_s = shift;
		}

        VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");
	VOID printf("#define P(R) \\\n( \\\n");
	for (shift = min_s; shift <= max_s; shift++)
		if (st[DES_L_BITS + shift]) {
			VOID printf("\t((R & 0x%08lxl) ",
				    st[DES_L_BITS + shift]);
			if (shift > 0)
				VOID printf("<< %2d)", shift);
			else if (shift < 0)
				VOID printf(">> %2d)", -shift);
			else
				VOID printf("     )");
			if (shift == max_s)
				VOID printf(" \\\n");
			else
				VOID printf(" | \\\n");
		}
	VOID printf(")\n");

	exit(0);
}
