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
#include	"des.h"
#include	"config.h"
#include	"version.h"

/*
 * des_print_cblock
 *
 *	prints the contentse of a `cblock' as 16 hexadecimal digits on
 *	stdout.
 *
 */


FILE	*des_print_file = stdout;
char	*des_print_format = "%02x%02x%02x%02x%02x%02x%02x%02x\n";


int	des_print_cblock(
#ifdef __STDC__
	des_cblock	*cblock,
	int		items)
#else
	cblock, items)
des_cblock	*cblock;
int		items;
#endif
{
	register int	n;


	for (n = 0; n < items && ! ferror(des_print_file); n++, cblock++)
		VOID fprintf(des_print_file, des_print_format,
			     (unsigned int) (*cblock)[0],
			     (unsigned int) (*cblock)[1],
			     (unsigned int) (*cblock)[2],
			     (unsigned int) (*cblock)[3],
			     (unsigned int) (*cblock)[4],
			     (unsigned int) (*cblock)[5],
			     (unsigned int) (*cblock)[6],
			     (unsigned int) (*cblock)[7]);

	return n;
}
