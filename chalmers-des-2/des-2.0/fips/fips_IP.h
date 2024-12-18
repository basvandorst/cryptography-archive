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
 * The initial permutation IP as described in FIPS 46 1977 January 15.
 *
 * The permuted input has bit 58 pf the input block as its first bit,
 * bit 50 as it second bit, and so on with bit 7 as its last bit.
 */

#define IP_ROWS		8
#define IP_COLUMNS	8

const unsigned char	ip[IP_ROWS][IP_COLUMNS] = {
				{ 58, 50, 42, 34, 26, 18, 10,  2 },
				{ 60, 52, 44, 36, 28, 20, 12,  4 },
				{ 62, 54, 46, 38, 30, 22, 14,  6 },
				{ 64, 56, 48, 40, 32, 24, 16,  8 },
				{ 57, 49, 41, 33, 25, 17,  9,  1 },
				{ 59, 51, 43, 35, 27, 19, 11,  3 },
				{ 61, 53, 45, 37, 29, 21, 13,  5 },
				{ 63, 55, 47, 39, 31, 23, 15,  7 }
			};
