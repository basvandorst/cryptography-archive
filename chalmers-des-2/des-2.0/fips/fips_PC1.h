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

/*
 * The permutation choise 1 PC1 as described in FIPS 46 1977 January 15.
 *
 * The bits of KEY is numberd 1 .. 64. The bits of C0 are
 * 57, 49, ... 36 of KEY. The bits of D0 are 63, 55, .. 4
 * of KEY.
 */

#define PC1_ROWS	8
#define PC1_COLUMNS	7

const unsigned char	pc1[PC1_ROWS][PC1_COLUMNS] = {
				/* C0 */
				{ 57, 49, 41, 33, 25, 17,  9 },
				{  1, 58, 50, 42, 34, 26, 18 },
				{ 10,  2, 59, 51, 43, 35, 27 },
				{ 19, 11,  3, 60, 52, 44, 36 },

				/* D0 */
				{ 63, 55, 47, 39, 31, 23, 15 },
				{  7, 62, 54, 46, 38, 30, 22 },
				{ 14,  6, 61, 53, 45, 37, 29 },
				{ 21, 13,  5, 28, 20, 12,  4 }
			};
