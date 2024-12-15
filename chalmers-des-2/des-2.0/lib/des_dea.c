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
#include	"dea.h"
#include	"version.h"

/*
 * des_dea
 *
 *	The standard data encryption algorithm as described in
 *	FIPS 46 1977 january 15. 
 *
 *	`input' and `output' may be overlapping.
 */


int	des_dea(
#ifdef __STDC__
	des_cblock		*input,
	des_cblock		*output,
	des_key_schedule	schedule,
	int			encrypt)
#else
	input, output, schedule, encrypt)
des_cblock		*input;
des_cblock		*output;
des_key_schedule	schedule;
int			encrypt;
#endif
{
	register unsigned long	l, r;
#ifdef IP_DATA
	IP_DATA;
#endif /* IP_DATA */
#ifdef CRYPT_DATA
	CRYPT_DATA;
#endif /* CRYPT_DATA */
#ifdef IPinv_DATA
	IPinv_DATA;
#endif /* IPinv_DATA */


	/* IP - Initial Permutaion */
	IP(l, r, (*input));

	/* I 1 .. DES_ITERATIONS - Iterations */
	if (encrypt) {
		ENCRYPT(l, r, schedule);
	} else {
		DECRYPT(l, r, schedule);
	}

	/* IPinv - Inverse of Initial Permutaion IP */
	IPinv((*output), r, l);

	return 0;
}
