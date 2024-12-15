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

/*
 * This program generates a table to use for key parity adjustment.
 *
 */

main()
{
	register int		i, bit;
	register unsigned char	res, p;
#ifdef DES_LSB_FIRST
	unsigned char		resv[(UNSIGNED_CHAR_MAX + 1) / 2];
#endif


        VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");
	VOID printf("const unsigned char\tparity_tab[%d] = {\n",
		    UNSIGNED_CHAR_MAX + 1);
#ifdef DES_LSB_FIRST
	for (i = 0; i < (UNSIGNED_CHAR_MAX + 1) / 2; i++) {

		res = (unsigned char) i;
		
		for (p = 0x80, bit = 0x01; bit <= 0x80; bit <<= 1)
			if (res & bit)
				p ^= 0x80;

		resv[i] = (res |= p);

		VOID printf("\t\t\t\t0x%02x,\n", res);
	}
	for (i = 0; i < ((UNSIGNED_CHAR_MAX + 1) / 2) - 1; i++)
		VOID printf("\t\t\t\t0x%02x,\n", resv[i]);
	VOID printf("\t\t\t\t0x%02x\n", resv[i]);
#else
	for (i = 0; i < UNSIGNED_CHAR_MAX + 1; i += 2) {

		res = (unsigned char) i;
		
		for (p = 0x01, bit = 0x80; bit >= 0x02; bit >>= 1)
			if (res & bit)
				p ^= 0x01;

		res |= p;

		VOID printf("\t\t\t\t0x%02x,\n", res);
		if (i < UNSIGNED_CHAR_MAX)
			VOID printf("\t\t\t\t0x%02x,\n", res);
		else
			VOID printf("\t\t\t\t0x%02x\n", res);
	}
#endif
	VOID printf("\t\t\t};\n");

	exit(0);
}
