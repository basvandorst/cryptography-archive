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
 * des_cbc_cksum
 *
 *	Cipher Block Chaining Checksum.
 *
 */

unsigned long	des_cbc_cksum(
#ifdef __STDC__
	des_cblock		*input,
	des_cblock		*output,
	int			length,
	des_key_schedule	schedule,
	des_cblock		*ivec)
#else
	input, output, length, schedule, ivec)
des_cblock		*input;
des_cblock		*output;
int			length;
des_key_schedule	schedule;
des_cblock		*ivec;
#endif
{
	register int		i;
	register des_cblock	*ivecp;
	des_cblock		tmp_ivec;
	register unsigned long	res;


	if (des_return_ivec) {
		ivecp = ivec;
	} else {
		COPY_8(tmp_ivec, (*ivec));
		ivecp = (des_cblock *) tmp_ivec;
	}

	for (; length >= DES_BLOCK_BYTES; length -= DES_BLOCK_BYTES, input++) {
		XOR_8((*ivecp), (*input));
		VOID des_dea(ivecp, ivecp, schedule, DES_ENCRYPT);
	}
	if (length > 0) {
		/* This method procuces zero padding ! */
		for (i = 0; i < length; i++)
			(*ivecp)[i] ^= (*input)[i];
		VOID des_dea(ivecp, ivecp, schedule, DES_ENCRYPT);
	}
	COPY_8((*output), (*ivecp));

	res  = (unsigned long) (*output)[7];
	res <<= UNSIGNED_CHAR_BITS;
	res |= (unsigned long) (*output)[6];
	res <<= UNSIGNED_CHAR_BITS;
	res |= (unsigned long) (*output)[5];
	res <<= UNSIGNED_CHAR_BITS;
	res |= (unsigned long) (*output)[4];

	return	res;
}
