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


char	*prog;

#define h_FLAG	0x1L
#define v_FLAG	0x2L
unsigned long	flags = 0L;


main(argc, argv)
int	argc;
char	*argv[];
{
	register int		o, st, errors = 0;
	register unsigned int	byte, bit, i;
	des_key_schedule	schedule;
	des_cblock		key, ib, cb, ob;
	extern int		optind;


	prog = (prog = strrchr(*argv, '/')) ? (prog + 1) : *argv;

	while ((o = getopt(argc, argv, "hv")) != EOF)
		switch (o) {
		case 'h':
			flags |= h_FLAG;
			break;
		case 'v':
			flags |= v_FLAG;
			break;
		case '?':
		default:
			goto usage;
		}
	if (optind + 1 != argc)
		goto usage;

	if (flags & h_FLAG) {
		if (des_hex_to_cblock(argv[optind], (des_cblock *) key) < 0)
			goto badhex;
		if (flags & v_FLAG)
			VOID printf("HEXKEY = \"%s\"\n\n", argv[optind]);
	} else {
		VOID des_string_to_key(argv[optind], key);
		if (flags & v_FLAG)
			VOID printf("STRKEY = \"%s\"\n\n", argv[optind]);
	}

	st = des_check_key_parity((des_cblock *) key);
	if (!st || flags & v_FLAG) {
		if (!st)
			errors++;
		VOID fputs("   KEY = ", stdout);
		des_print_cblock((des_cblock *) key, 1);
		VOID printf("PARITY = %s\n\n",
			     (st) ? "CORRECT (ODD)" : "INCORRECT");
	}

	VOID des_sched((des_cblock *) key, schedule);

	if (flags & v_FLAG) {
		VOID printf("  KEYS = %d\n",
			    des_no_key_schedule((des_cblock *) key));
		for (i = 0; i < 16; i++) {
			VOID printf("KS[%2d] = ", i);
			des_print_cblock((des_cblock *) schedule[i]._, 1);
		}

		VOID putchar('\n');
	}

	for (byte = 0; byte < DES_BLOCK_BYTES; byte++) {
		for (bit = 0x80; bit >= 0x01; bit >>= 1) {
			DES_ZERO_CBLOCK(ib);
			ib[byte] = bit;
			if (flags & v_FLAG) {
				VOID fputs("   IN = ", stdout);
				des_print_cblock((des_cblock *) ib, 1);
			}
			VOID des_dea((des_cblock *) ib, (des_cblock *) cb,
				     schedule, DES_ENCRYPT);
			if (flags & v_FLAG) {
				VOID fputs("CRYPT = ", stdout);
				des_print_cblock((des_cblock *) cb, 1);
			}
			VOID des_dea((des_cblock *) cb, (des_cblock *) ob,
				     schedule, DES_DECRYPT);
			if (flags & v_FLAG) {
				VOID fputs("  OUT = ", stdout);
				des_print_cblock((des_cblock *) ob, 1);
			}
			if (memcmp(ib, ob, sizeof(des_cblock))) {
				des_print_cblock((des_cblock *) ib, 1);
				VOID printf("NOT EQUAL TO");
				des_print_cblock((des_cblock *) ob, 1);
				errors++;
			}
		}
	}

	exit(errors);

usage:
	VOID fprintf(stderr, "Usage: %s [-h] key\n", prog);
	exit(1);

badhex:
	VOID fprintf(stderr, "%s: Key must be a 64 bit hex. number.\n", prog);
	exit(1);
}
