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
 * The bit slection E as described in FIPS 46 1977 January 15.
 *
 * The selected input has bit 32 pf the input block as its first bit,
 * bit 1 as it second bit, and so on with bit 1 as its last bit.
 */

#define E_ROWS		8
#define E_COLUMNS	6

const unsigned char	e[E_ROWS][E_COLUMNS] = {
				{ 32,  1,  2,  3,  4,  5 },
				{  4,  5,  6,  7,  8,  9 },
				{  8,  9, 10, 11, 12, 13 },
				{ 12, 13, 14, 15, 16, 17 },
				{ 16, 17, 18, 19, 20, 21 },
				{ 20, 21, 22, 23, 24, 25 },
				{ 24, 25, 26, 27, 28, 29 },
				{ 28, 29, 30, 31, 32,  1 }
			};
