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

#include	"des.h"
#include	"config.h"
#include	"parity.h"
#include	"version.h"

/*
 * des_fixup_key_parity
 *
 *	Adjusts the parity bits in a key.
 */

int	des_fixup_key_parity(
#ifdef __STDC__
	des_cblock *key)
#else
	key)
des_cblock	*key;
#endif
{
	(*key)[0] = parity_tab[(*key)[0]];
	(*key)[1] = parity_tab[(*key)[1]];
	(*key)[2] = parity_tab[(*key)[2]];
	(*key)[3] = parity_tab[(*key)[3]];
	(*key)[4] = parity_tab[(*key)[4]];
	(*key)[5] = parity_tab[(*key)[5]];
	(*key)[6] = parity_tab[(*key)[6]];
	(*key)[7] = parity_tab[(*key)[7]];

	return 0;
}
