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
 * des_ecb2_encrypt
 *
 *	Electronic Code Book.
 *
 *	Data Encryption Standard encrypytion/decryption procedure.
 *	The procedure does only encrypt/decrypt 64 bit/8 byte blocks.
 *
 */

int	des_ecb2_encrypt(
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
	register int	i;
	des_cblock	b;


	for (; length >= DES_BLOCK_BYTES; length -= DES_BLOCK_BYTES) {
		VOID des_dea(input, output, schedule, encrypt);
		input++;
		output++;
	}
	/* Padd with zeros if neccessary */
	if (length > 0 && encrypt) {
		DES_ZERO_CBLOCK(b);
		for (i = 0; i < length; i++)
			b[i] = (*input)[i];
		VOID des_dea((des_cblock *) b, output, schedule, DES_ENCRYPT);
	}

	return 0;
}
