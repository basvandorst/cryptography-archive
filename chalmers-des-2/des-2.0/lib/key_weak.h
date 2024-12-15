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
 * List of weak keys.
 */

#define NO_WEAK_KEYS	4

const des_cblock	weak_key[NO_WEAK_KEYS] = {
#ifdef DES_LSB_FIRST
			{ 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 },
			{ 0xf8, 0xf8, 0xf8, 0xf8, 0x70, 0x70, 0x70, 0x70 },
			{ 0x07, 0x07, 0x07, 0x07, 0x8f, 0x8f, 0x8f, 0x8f },
			{ 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f }
#else
			{ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
			{ 0x1f, 0x1f, 0x1f, 0x1f, 0x70, 0x70, 0x70, 0x70 },
			{ 0xe0, 0xe0, 0xe0, 0xe0, 0x8f, 0x8f, 0x8f, 0x8f },
			{ 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe }
#endif
		};
