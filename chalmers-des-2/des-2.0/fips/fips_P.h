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
 * The permutation P as described in FIPS 46 1977 January 15.
 *
 * The permuted input has bit 16 pf the input block as its first bit,
 * bit 7 as it second bit, and so on with bit 25 as its last bit.
 */

#define P_ROWS		8
#define P_COLUMNS	4

const unsigned char	p[P_ROWS][P_COLUMNS] = {
				{ 16,  7, 20, 21 },
				{ 29, 12, 28, 17 },
				{  1, 15, 23, 26 },
				{  5, 18, 31, 10 },
				{  2,  8, 24, 14 },
				{ 32, 27,  3,  9 },
				{ 19, 13, 30,  6 },
				{ 22, 11,  4, 25 }
			};