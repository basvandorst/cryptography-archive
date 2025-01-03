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
#include	"tab_PC1.h"
#include	"tab_PC2.h"
#include	"tab_LS.h"
#include	"version.h"

/*
 * des_sched
 *
 *	The key schedule generation as described in FIPS 46 1977 January 15.
 *
 */

/*
#define USE_LOOP
*/

int	des_sched(
#ifdef __STDC__
	des_cblock		*key,
	des_key_schedule	schedule)
#else
	key, schedule)
des_cblock 		*key;
des_key_schedule	schedule;
#endif
{
	register unsigned long	c, d;
#ifdef USE_LOOP
	register int		n;
#endif /* USE_LOOP */
#ifdef PC1_DATA
	PC1_DATA;
#endif /* PC1_DATA */
#ifdef LS_DATA
	LS_DATA;
#endif /* LS_DATA */
#ifdef PC2_DATA
	PC2_DATA;
#endif /* PC2_DATA */


	/* Permuted choise 1 */
	PC1(c, d, (*key));

	/* Key generation iterations */
#ifdef USE_LOOP
	for (n = 0; n < 16; n++) {
		/* Left shift */
		LS(n, c);
		LS(n, d);
		/* Permuted choise 2 */
		PC2(schedule[n]._, c, d);
	}
#else  /* USE_LOOP */

	/* Left shift */
	LS(0, c);
	LS(0, d);
	/* Permuted choise 2 */
	PC2(schedule[0]._, c, d);

	/* Left shift */
	LS(1, c);
	LS(1, d);
	/* Permuted choise 2 */
	PC2(schedule[1]._, c, d);

	/* Left shift */
	LS(2, c);
	LS(2, d);
	/* Permuted choise 2 */
	PC2(schedule[2]._, c, d);
	
	/* Left shift */
	LS(3, c);
	LS(3, d);
	/* Permuted choise 2 */
	PC2(schedule[3]._, c, d);

	/* Left shift */
	LS(4, c);
	LS(4, d);
	/* Permuted choise 2 */
	PC2(schedule[4]._, c, d);

	/* Left shift */
	LS(5, c);
	LS(5, d);
	/* Permuted choise 2 */
	PC2(schedule[5]._, c, d);

	/* Left shift */
	LS(6, c);
	LS(6, d);
	/* Permuted choise 2 */
	PC2(schedule[6]._, c, d);

	/* Left shift */
	LS(7, c);
	LS(7, d);
	/* Permuted choise 2 */
	PC2(schedule[7]._, c, d);

	/* Left shift */
	LS(8, c);
	LS(8, d);
	/* Permuted choise 2 */
	PC2(schedule[8]._, c, d);

	/* Left shift */
	LS(9, c);
	LS(9, d);
	/* Permuted choise 2 */
	PC2(schedule[9]._, c, d);

	/* Left shift */
	LS(10, c);
	LS(10, d);
	/* Permuted choise 2 */
	PC2(schedule[10]._, c, d);

	/* Left shift */
	LS(11, c);
	LS(11, d);
	/* Permuted choise 2 */
	PC2(schedule[11]._, c, d);

	/* Left shift */
	LS(12, c);
	LS(12, d);
	/* Permuted choise 2 */
	PC2(schedule[12]._, c, d);

	/* Left shift */
	LS(13, c);
	LS(13, d);
	/* Permuted choise 2 */
	PC2(schedule[13]._, c, d);

	/* Left shift */
	LS(14, c);
	LS(14, d);
	/* Permuted choise 2 */
	PC2(schedule[14]._, c, d);

	/* Left shift */
	LS(15, c);
	LS(15, d);
	/* Permuted choise 2 */
	PC2(schedule[15]._, c, d);

#endif /* USE_LOOP */

	return 0;
}
