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
#include	"fips_E.h"

/*
 * This program generates a macro definition for us as the expansion E.
 *
 *	E(B, R)
 *
 * The input is 32 bits in the unsigned long `R' with the least significant
 * bit as the first. The output is the least significant six bits in each
 * unsigned character B[0] .. B[7], with the first bit as the least significant
 * bit of B[0].
 */

#define USE_SHIFT

main()
{
#ifdef USE_SHIFT
	register int		n, s_min, s_max, r, c, bit, s, sh, sp, se;
	register unsigned long	b;
	unsigned long		st[E_ROWS][DES_R_BITS * 2];
	unsigned long		nb[E_ROWS][DES_R_BITS * 2];
	int			b_set[E_ROWS];


	s_min = DES_R_BITS * 2;
	s_max = 0;
	for (r = 0; r < E_ROWS; r++) {

		b_set[r] = 0;

		for (s = 0; s < DES_R_BITS * 2; s++)
			nb[r][s] = st[r][s] = 0x0l;

		for (bit = 1, c = 0; c < E_COLUMNS; bit++, c++) {
			sh = bit - e[r][c];
			s = DES_R_BITS + sh;
			st[r][s] |= 0x1l << (e[r][c] - 1);
			nb[r][s]++;
			if (s > s_max)
				s_max = s;
			if (s < s_min)
				s_min = s;
		}
	}

	VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");
	VOID printf("#define E_DATA register unsigned long\ter\n\n");
	VOID printf("#define E(B, R) \\\n");
	VOID printf("\ter = R; \\\n");
	for (n = sp = 0, s = DES_R_BITS; s <= s_max; s++)
		for (r = 0; r < E_ROWS; r++) {
			if (!st[r][s])
				continue;
			if (n++)
				VOID printf("; \\\n");
			sh = s - DES_R_BITS;
			se = sh - sp;
			b = st[r][s] << sh;
			if (nb[r][s] > 1) {
				VOID printf("\ter <<= %d; \\\n", se);
				if (b_set[r]++)
					VOID printf("\tB[%d] |= er & 0x%08lxl",
						    r, b);
				else
					VOID printf("\tB[%d] = er & 0x%08lxl",
						    r, b);
				sp = sh;
			} else {
				if (b_set[r]++) {
					VOID printf("\tif (er & 0x%08lxl) \\\n",
						    st[r][s] << sp);
					VOID printf("\t\tB[%d] |= 0x%08lxl",
						    r, b);
				} else {
					VOID printf("\tB[%d] = (er & 0x%08lxl) ? ",
						    r, st[r][s] << sp);
					VOID printf("0x%08lxl : 0x%08lxl",
						    b, 0x0l);
				}
			}
		}
	if (sp) {
		sp = 0;
		VOID printf("; \\\n\ter = R");
	}
	for (s = DES_R_BITS - 1; s >= s_min; s--)
		for (r = 0; r < E_ROWS; r++) {
			if (!st[r][s])
				continue;
			if (n++)
				VOID printf("; \\\n");
			sh = s - DES_R_BITS;
			se = sh - sp;
			b = st[r][s] >> -sh;
			if (nb[r][s] > 1) {
				VOID printf("\ter >>= %d; \\\n", -se);
				if (b_set[r]++)
					VOID printf("\tB[%d] |= er & 0x%08lxl",
						    r, b);
				else
					VOID printf("\tB[%d] = er & 0x%08lxl",
						    r, b);
				sp = sh;
			} else {
				if (b_set[r]++) {
					VOID printf("\tif (er & 0x%08lxl) \\\n",
						    st[r][s] >> -sp);
					VOID printf("\t\tB[%d] |= 0x%08lxl",
						    r, b);
				} else {
					VOID printf("\tB[%d] = (er & 0x%08lxl) ? ",
						    r, st[r][s] >> -sp);
					VOID printf("0x%08lxl : 0x%08lxl",
						    b, 0x0l);
				}
			}
		}
	VOID printf("\n");
# ifdef NOTDEF
	for (r = 0; r < E_ROWS; r++) {
		VOID printf("\tB[%d] = (", r);
		for (s = min_s[r]; s <= max_s[r]; s++)
			if (nb[r][s]) {
				VOID printf("((er & 0x%08lxl) ", st[r][s]);
				sh = s - DES_R_BITS;
				if (sh > 0)
					VOID printf("<< %2d)", sh);
				else if (sh < 0)
					VOID printf(">> %2d)", -sh);
				else
					VOID printf("     )");
				VOID printf((--nb[r][s]) ? ")" : " | ");
			}
		VOID printf((r + 1 < E_ROWS) ? "; \\\n" : "\n");
	}
# endif /* NOTDEF */
#else  /* USE_SHIFT */

#endif  /* USE_SHIFT */

	exit(0);
}
