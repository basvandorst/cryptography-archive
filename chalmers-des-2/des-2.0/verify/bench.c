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

#include	<stdio.h>
#include	<string.h>
#include	<des.h>
#include	"config.h"


char		*prog;

#define k_FLAG	0x1L
unsigned long	flags = 0L;


main(argc, argv)
int	argc;
char	*argv[];
{
	register int		o;
	register unsigned int	i, key;
	des_key_schedule	schedule;
	des_cblock		b;
	extern int		optind;


	prog = (prog = strrchr(*argv, '/')) ? (prog + 1) : *argv;

	while ((o = getopt(argc, argv, "k")) != EOF)
		switch (o) {
		case 'k':
			flags |= k_FLAG;
			break;
		case '?':
		default:
			goto usage;
		}
	if (optind + 1 != argc)
		goto usage;
	i = atoi(argv[optind]);

	if (flags & k_FLAG) 
		while (i-- > 0) 
			VOID des_sched((des_cblock *) b, schedule);
	else
		while (i-- > 0) 
			VOID des_dea((des_cblock *) b, (des_cblock *) b,
				     schedule, DES_ENCRYPT);

	exit(0);
usage:
	VOID fprintf(stderr, "Usage: %s [-k] iterations\n", prog);
	exit(1);
}
