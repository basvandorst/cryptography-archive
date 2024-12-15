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
 *			     -1
 * The initial permutation IP   as described in FIPS 46 1977 January 15.
 *
 * The permuted input has bit 40 of the input block as its first bit,
 * bit 8 as it second bit, and so on with bit 25 as its last bit.
 */

#define IPinv_ROWS	8
#define IPinv_COLUMNS	8

const unsigned char	ip_inv[IPinv_ROWS][IPinv_COLUMNS] = {
				{ 40,  8, 48, 16, 56, 24, 64, 32 },
				{ 39,  7, 47, 15, 55, 23, 63, 31 },
				{ 38,  6, 46, 14, 54, 22, 62, 30 },
				{ 37,  5, 45, 13, 53, 21, 61, 29 },
				{ 36,  4, 44, 12, 52, 20, 60, 28 },
				{ 35,  3, 43, 11, 51, 19, 59, 27 },
				{ 34,  2, 42, 10, 50, 18, 58, 26 },
				{ 33,  1, 41,  9, 49, 17, 57, 25 }
			};
