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
/* #define USE_DES_DEA /* Define if des_dea should be called */
#ifndef USE_DES_DEA
#include	"dea.h"
#endif  /* USE_DES_DEA */
#include	"version.h"

/*
 * des_dea3
 *
 *	Trippel DES encryption alogrithm as described in
 *	ANSI X9.17-1985.
 *
 *	E = Encrypt DES.
 *	D = Dencrypt DES.
 *	E3 = Encrypt Trippel DES.
 *	D3 = Dencrypt Trippel DES.
 *
 *	E3(K1, K2, x) = E(K1, D(K2, E(K1, x)))
 *	D3(K1, K2, x) = D(K1, E(K2, D(K1, x)))
 *
 *	Note: K1 = K2 -> E3 = E, D3 = D.
 *
 *	`input' and `output' may be overlapping.
 */


int	des_dea3(
#ifdef __STDC__
	des_cblock		*input,
	des_cblock		*output,
	des_key_schedule	schedule[],
	int			encrypt)
#else
	input, output, schedule, encrypt)
des_cblock		*input;
des_cblock		*output;
des_key_schedule	schedule[];
int			encrypt;
#endif
{
	register des_key_schedule	*s1, *s2;
#ifndef USE_DES_DEA
	register unsigned long		l, r;
#ifdef IP_DATA
	IP_DATA;
#endif /* IP_DATA */
#ifdef CRYPT_DATA
	CRYPT_DATA;
#endif /* CRYPT_DATA */
#ifdef IPinv_DATA
	IPinv_DATA;
#endif /* IPinv_DATA */
#endif  USE_DES_DEA


	s1 = &schedule[0];
	s2 = &schedule[1];
#ifdef USE_DES_DEA
	VOID des_dea(input, output, *s1, encrypt);
	VOID des_dea(output, output, *s2, !encrypt);
	VOID des_dea(output, output, *s1, encrypt);
#else  /* USE_DES_DEA */
	/* IP - Initial Permutaion */
	IP(l, r, (*input));

	/* I 3 x (1 .. DES_ITERATIONS) - Iterations */
	/* The `IP' permutaions between each step eliminates eachother */
	if (encrypt) {
		ENCRYPT(l, r, *s1)
		/* `IP' and `IPinv' eliminates eachother */
		DECRYPT(r, l, *s2)
		/* `IP' and `IPinv' eliminates eachother */
		ENCRYPT(l, r, *s1)
	} else {
		DECRYPT(l, r, *s1)
		/* `IP' and `IPinv' eliminates eachother */
		ENCRYPT(r, l, *s2)
		/* `IP' and `IPinv' eliminates eachother */
		DECRYPT(l, r, *s1)
	}

	/* IPinv - Inverse of Initial Permutaion IP */
	IPinv((*output), r, l);
#endif /* USE_DES_DEA */

	return 0;
}
