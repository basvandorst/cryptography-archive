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
 * des_cbc_encrypt
 *
 *	Cipher Block Chaining.
 *
 */

int	des_cbc_encrypt(
#ifdef __STDC__
	des_cblock		*input,
	des_cblock		*output,
	int			length,
	des_key_schedule	schedule,
	des_cblock		*ivec,
	int			encrypt)
#else
	input, output, length, schedule, ivec, encrypt)
des_cblock		*input;
des_cblock		*output;
int			length;
des_key_schedule	schedule;
des_cblock		*ivec;
int			encrypt;
#endif
{
	register int		i;
	des_cblock		c, tmp_ivec;
	register des_cblock	*ivecp;


	if (des_return_ivec) {
		ivecp = ivec;
	} else {
		COPY_8(tmp_ivec, (*ivec));
		ivecp = (des_cblock *) tmp_ivec;
	}
	if (encrypt) {
		for (; length >= DES_BLOCK_BYTES;
		       length -= DES_BLOCK_BYTES, output++, input++) {
			XOR_8((*ivecp), (*input));
			VOID des_dea(ivecp, ivecp, schedule, DES_ENCRYPT);
			COPY_8((*output), (*ivecp));
		}
		/* Padd with zeros. */
		if (length > 0) {
			for (i = 0; i < length; i++)
				(*ivecp)[i] ^= (*input)[i];
			VOID des_dea(ivecp, ivecp, schedule, DES_ENCRYPT);
			COPY_8((*output), (*ivecp));
		}
	} else
		for (; length > 0;
		       length -= DES_BLOCK_BYTES, output++, input++) {
			COPY_8(c, (*input));
			VOID des_dea(input, output, schedule, DES_DECRYPT);
			XOR_8((*output), (*ivecp));
			COPY_8((*ivecp), c);
		}
		/* There is no idea to use padding in decryption */

	return 0;
}
