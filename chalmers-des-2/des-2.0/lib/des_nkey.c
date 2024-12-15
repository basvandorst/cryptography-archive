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
#include	"version.h"

/*
 * des_no_key_schedule
 *
 *	Returns the number of different internal keys generated in a key
 *	schedule. The output is allways in the range 1 .. 16.
 */

int	des_no_key_schedule(
#ifdef __STDC__
	des_cblock *key)
#else
	key)
des_cblock	*key;
#endif
{
	register int		i, j, n;
	int			eq[16];
	des_key_schedule	ks;


	VOID des_sched(key, ks); 

	for (i = 0; i < 16; i++)
		eq[i] = 0;
	for (n = 16, i = 0; i < 16; i++)
		if (!eq[i])
			for (j = i + 1; j < 16; j++)
				if (!eq[j])
					if (CMP_8(ks[i]._, ks[j]._)) {
						eq[j] = 1;
						n--;
					}

	return n;
}
