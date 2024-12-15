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
#include	"local_def.h"
#include	"key_sweak.h"
#include	"version.h"

/*
 * des_is_semiweak_key
 *
 *	Check for keys that produces only two different values int the
 *	internal key schedule. There are twelve such keys.
 *
 *	The key must have correct parity.
 *
 *	0 => The key is not semiweak.
 *	1 => The key is semiweak.
 */

int	des_is_semiweak_key(
#ifdef __STDC__
	des_cblock *key)
#else
	key)
des_cblock	*key;
#endif
{
	register int		i;


	for (i = 0; i < NO_SEMI_WEAK_KEYS; i++)
		if (CMP_8(semiweak_key[i],(*key)))
			return 1;
	return 0;
}
