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
#include	"local_def.h"
#include	"version.h"

/*
 * des_hex_to_cblock
 *
 *	Generation of a cblock from a string contaning 16 hexadecimal
 *	digits. The string may have an optional "0x" or "0X" prepended.
 *
 *	The function returns -1 if the string does not contain excatly
 *	16 digits or if any non hexadecimal characters are found.
 *
 *	`cblock' is modified if the function returns -1.
 *
 */


char	*des_scan_format = "%2x%2x%2x%2x%2x%2x%2x%2x";


int	des_hex_to_cblock(
#ifdef __STDC__
	char		*str,
	des_cblock	*cblock)
#else
	str, cblock)
char		*str;
des_cblock	*cblock;
#endif
{
#ifdef int
/*
 * This must be an int to avoid alignment problems
 */
# undef int
#endif
	int			buf[DES_BLOCK_BYTES];
	register char		*sp;


	sp = str;

	if (!sp)
		return -1;

	/*
	 * Remove "0x" or "0X".
	 */
	if (*sp == '0' && (sp[1] == 'x' || sp[1] == 'X'))
		sp += 2;

	if (strlen(sp) != DES_BLOCK_BYTES * 2)
		return -1;
	if (sscanf(sp, des_scan_format, &buf[0], &buf[1], &buf[2], &buf[3],
		   &buf[4], &buf[5], &buf[6], &buf[7]) != DES_BLOCK_BYTES)
		return -1;

	(*cblock)[0] = (unsigned char) buf[0];
	(*cblock)[1] = (unsigned char) buf[1];
	(*cblock)[2] = (unsigned char) buf[2];
	(*cblock)[3] = (unsigned char) buf[3];
	(*cblock)[4] = (unsigned char) buf[4];
	(*cblock)[5] = (unsigned char) buf[5];
	(*cblock)[6] = (unsigned char) buf[6];
	(*cblock)[7] = (unsigned char) buf[7];

	return 0;
}
