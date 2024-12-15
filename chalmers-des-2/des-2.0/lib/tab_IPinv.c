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
#include	"des.h"
#include	"config.h"
#include	"fips_def.h"
#include	"fips_IPinv.h"

/*
 * This program generates a macro for us as inverse initial permutation.
 *
 *		IPinv(B, L, R)
 *
 * The input are two unsigned long's `L' and `R' with `L' as the first
 * 32 bits in the block and `R' as the rest. The first bit of each `L'
 * and `R' are the least significant of each.
 * The ouput is an array of unsigned characters `B[0..7]' with the
 * least significant bit of `B[0]' as the first bit.
 *
 * The macro IPinv_DATA must be placed in the declaration section in the
 * enclosing procedure to IPinv. It contains variables used by IPinv;
 */


main()
{
#if defined(USE_IF) || defined(USE_SHIFT)
# ifdef USE_SHIFT
	register int		shift;
# else  /* USE_SHIFT */
	register unsigned long	bb;
# endif /* USE_SHIFT */
	register int		n, r, c, block_bit, byte, bit, byte_bit;
	register unsigned long	b;


        VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");
	VOID printf("#define IPinv(B, L, R) \\\n\\\n");
	for (n = byte = byte_bit = block_bit = r = 0; r < IPinv_ROWS; r++)
		for (c = 0; c < IPinv_COLUMNS; c++) {
			if (++byte_bit == 1) {
# ifdef USE_SHIFT
				VOID printf("\tB[%d] = ( \\\n", byte);
# else  /* USE_SHIFT */
				VOID printf("\tB[%d] = 0x00; \\\n", byte);
# endif /* USE_SHIFT */
			}
		
			n = (ip_inv[r][c] - 1) / DES_L_BITS;
			bit = ip_inv[r][c] - (n * DES_L_BITS);
			b = 0x1l << (bit - 1);
# ifdef USE_SHIFT
#  ifdef DES_LSB_FIRST
			shift = byte_bit - bit;
#  else  /* DES_LSB_FIRST */
			shift = (UNSIGNED_CHAR_BITS + 1 - byte_bit) - bit;
#  endif /* DES_LSB_FIRST */
			VOID printf("\t\t((%c & 0x%08lxl)", n ? 'R' : 'L', b);
			if (shift > 0) 
				VOID printf(" << %2d", shift);
			else if (shift < 0)
				VOID printf(" >> %2d", -shift);
			else
				VOID printf("      ");
			if (byte_bit == UNSIGNED_CHAR_BITS) {
				VOID printf(")   /* bit %2d */ \\\n",
					    ++block_bit);
				VOID printf((++byte < DES_BLOCK_BYTES) ?
					    "\t); \\\n\\\n" : "\t)\n");
				byte_bit = 0;
			} else
				VOID printf(") | /* bit %2d */ \\\n",
					    ++block_bit);
# else  /* USE_SHIFT */
#  ifdef DES_LSB_FIRST
			bb = 0x1l << (byte_bit - 1);
#  else  /* DES_LSB_FIRST */
			bb = 0x1l << (UNSIGNED_CHAR_BITS - byte_bit);
#  endif /* DES_LSB_FIRST */
			VOID printf("\tif (%c & 0x%08lxl) B[%d] |= 0x%02lx",
				    n ? 'R' : 'L', b, byte, bb);
			if (byte_bit == UNSIGNED_CHAR_BITS) {
				if (++byte < DES_BLOCK_BYTES)
					VOID printf("; /* %2d -> %2d */ \\\n\\\n",
						    ip_inv[r][c], ++block_bit);
				else
					VOID printf("  /* %2d -> %2d */\n",
						    ip_inv[r][c], ++block_bit);
				byte_bit = 0;
			} else
				VOID printf("; /* %2d -> %2d */ \\\n",
					    ip_inv[r][c], ++block_bit);
# endif /* USE_SHIFT */
		}
#else  /* USE_IF || USE_SHIFT */
	register unsigned long	l, r;
	register int		i, j, byte, bit, n, sb, row, col;
	unsigned long		lv[UNSIGNED_CHAR_MAX + 1];
	unsigned long		rv[UNSIGNED_CHAR_MAX + 1];


        VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");
	VOID printf("static const\tstruct {\n\t\tunsigned long\tl, r;\n\t} ip_inv[%d][%d]",
		    DES_BLOCK_BYTES, UNSIGNED_CHAR_MAX + 1);
	VOID printf(" = {\n\t\t{\n");
	*lv = 0x0l;
	*rv = 0x0l;
	for (byte = 0; byte < DES_BLOCK_BYTES; byte++) {
		for (bit = 0; bit < UNSIGNED_CHAR_BITS; bit++) {
			sb = (byte * UNSIGNED_CHAR_BITS + bit) + 1;
			/*
			 * row and col can't be used directly as index
			 * variables due to the C-compiler on PS/2.
			 */
			row = col = 0;
			for (i = 0; i < IPinv_ROWS; i++)
				for (j = 0; j < IPinv_COLUMNS; j++)
					if (sb == ip_inv[i][j]) {
						row = i;
						col = j;
						goto next;
					}
next:
			sb = row * IPinv_COLUMNS;
# ifdef DES_LSB_FIRST
			sb += col;
# else
			sb += (UNSIGNED_CHAR_BITS - 1) - col;
# endif
			if (sb >= DES_L_BITS) {
				l = 0x0l;
				r = 0x1l << (sb - DES_L_BITS);
			} else {
				l = 0x1l << sb;
				r = 0x0l;
			}
			n = 0x1 << bit;
			lv[n] = l;
			rv[n] = r;
			if (n > 1)
				for (i = 1, j = n + 1; i < n; i++, j++) {
					lv[j] = l | lv[i];
					rv[j] = r | rv[i];
				}
		}
		for (i = 0; i < UNSIGNED_CHAR_MAX; i++)
			VOID printf("\t\t\t{ 0x%08lxl, 0x%08lxl },\n",
				    lv[i], rv[i]);
		VOID printf("\t\t\t{ 0x%08lxl, 0x%08lxl }\n\t\t}",
			    lv[i], rv[i]);
		if (byte < DES_BLOCK_BYTES - 1)
			VOID printf(", {\n");
	}
	VOID printf("\n\t};\n\n");
	VOID printf("#define IPinv_DATA des_cblock\t\tobp; \\\n");
	VOID printf("\t\tregister unsigned long\tol, or\n\n");
	VOID printf("#define IPinv(B, L, R) \\\n\\\n");
	VOID printf("\tLONG_TO_CHAR_8(obp, L, R); \\\n");
	VOID printf("\tol  = ip_inv[0][obp[0]].l; or  = ip_inv[0][obp[0]].r; \\\n");
	for (i = 1; i < DES_BLOCK_BYTES; i++)
		VOID printf("\tol |= ip_inv[%d][obp[%d]].l; or |= ip_inv[%d][obp[%d]].r; \\\n",
			    i, i, i, i);
	VOID printf("\tLONG_TO_CHAR_8(B, ol, or)\n");
	
#endif /* USE_IF || USE_SHIFT */

	exit(0);
}
