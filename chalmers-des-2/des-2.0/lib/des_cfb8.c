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
 * des_cfb8_encrypt
 *
 *	8-bit Cipher Feedback.
 *
 *	Data Encryption Standard encrypytion/decryption procedure.
 *
 */

int	des_cfb8_encrypt(
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
	register unsigned char	c, *inp, *outp;
	des_cblock		tmp_ivec;
	register des_cblock	*ivecp;


	if (des_return_ivec) {
		ivecp = ivec;
	} else {
		COPY_8(tmp_ivec, (*ivec));
		ivecp = (des_cblock *) tmp_ivec;
	}
	inp = (unsigned char *) input;
	outp = (unsigned char *) output;
	if (encrypt)
		while (length-- > 0) {
			VOID des_dea(ivecp, ivecp, schedule, DES_ENCRYPT);
			*outp = *inp++ ^ **ivecp;
			SHIFT_RIGHT_8((*ivecp), *outp);
			outp++;
		}
	else
		while (length-- > 0) {
			VOID des_dea(ivecp, ivecp, schedule, DES_ENCRYPT);
			/* shift right 8 bits */
			c = **ivec;
			SHIFT_RIGHT_8((*ivec), *inp);
			*outp++ = *inp++ ^ c;
		}

	return 0;
}
