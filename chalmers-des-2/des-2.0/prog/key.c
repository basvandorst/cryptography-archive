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
#include	"filekey.h"
#include	"strutil.h"
#include	"config.h"


char		*prog;

#ifdef DES_STRING_MODE_CTH
# define b_FLAG	0x1L
#endif /* DES_STRING_MODE_CTH */
#define f_FLAG	0x2L
#define k_FLAG	0x4L
#define r_FLAG	0x8L

unsigned long	flags = 0L;


main(argc, argv)
int	argc;
char	*argv[];
{
	register int	o;
	register char	*key_spec;
	des_cblock	key;
	extern int	optind;
	extern char	*optarg;


	prog = (prog = strrchr(*argv, '/')) ? (prog + 1) : *argv;

	key_spec = NULL;

#ifdef b_FLAG
	while ((o = getopt(argc, argv, "bf:k:r")) != EOF)
#else  /* b_FLAG */
	while ((o = getopt(argc, argv, "f:k:r")) != EOF)
#endif /* b_FLAG */
		switch (o) {
#ifdef b_FLAG
		case 'b':
			if (flags & b_FLAG)
				goto usage;
			VOID des_set_string_mode(DES_STRING_MODE_CTH);
			flags |= b_FLAG;
			break;
#endif /* b_FLAG */
		case 'f':
			if (flags & (f_FLAG | k_FLAG | r_FLAG))
				goto usage;
			if ((key_spec = new_string(optarg)) == NULL)
				goto memalloc;
			null_string(optarg);
			flags |= f_FLAG;
			break;
		case 'k':
			if (flags & (f_FLAG | k_FLAG | r_FLAG))
				goto usage;
			if ((key_spec = new_string(optarg)) == NULL)
				goto memalloc;
			null_string(optarg);
			flags |= k_FLAG;
			break;
		case 'r':
			if (flags & (f_FLAG | k_FLAG | r_FLAG))
				goto usage;
			flags |= r_FLAG;
			break;
		case '?':
		default:
			goto usage;
		}
	if (optind != argc)
		goto usage;

	if (flags & f_FLAG) {
		if (read_key((des_cblock *) key, key_spec, 0))
			goto nofilekey;
		null_string(key_spec);
		free(key_spec);
	} else if (flags & k_FLAG) {
		VOID des_string_to_key(key_spec, key);
		null_string(key_spec);
		free(key_spec);
	} else if (flags & r_FLAG) {
		VOID des_random_key((des_cblock *) key);
	} else {
		if (des_read_password((des_cblock *) key, "Key: ", 1) == -2)
			goto nokey;
	}

	VOID des_print_cblock((des_cblock *) key, 1);

	DES_ZERO_CBLOCK(key);

	exit(0);

usage:
#ifdef b_FLAG
	VOID fprintf(stderr, "Usage %s [-b] [-r|-k key|-f file]\n", prog);
#else  /* b_FLAG */
	VOID fprintf(stderr, "Usage %s [-r|-k key|-f file]\n", prog);
#endif /* b_FLAG */
	exit(1);

memalloc:
	fputs(prog, stderr);
	perror(": Could not allocate memory, ");
	exit(1);

nokey:
	VOID fprintf(stderr, "%s: Could not obtain key\n", prog);
	exit(1);

nofilekey:
	null_string(key_spec);
	free(key_spec);
	VOID fprintf(stderr, "%s: Could not read key from \"%s\"\n", prog, optarg);
	exit(1);
}
