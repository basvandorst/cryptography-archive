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

#include	<errno.h>
#include	"encrypt.h"
#include	"des.h"
#include	"local_def.h"
#include	"version.h"


#define	BLOCK_TO_DES(block,des_block) \
{ \
	register unsigned int	i, bit, byte; \
\
\
	for (i = byte = 0; byte < 8; byte++) { \
		des_block[byte] = 0; \
		for (bit = 0x80; bit; i++, bit >>= 1) \
			if (block[i]) \
				des_block[byte] |= bit; \
	} \
}

#define DES_TO_BLOCK(des_block,block) \
{ \
	register unsigned int	i, bit, byte; \
\
\
	for (i = byte = 0; byte < 8; byte++) \
		for (bit = 0x80; bit; i++, bit >>= 1) \
			block[i] = (des_block[byte] & bit) ? 1 : 0; \
}


/*
extern int		errno;

static int		key_valid = 0;
*/
static des_key_schedule	key_schedule;

/*
 * setkey
 *
 *	setkey - Optional X/Open set encoding key.
 *
 */

#ifdef __STDC__
void
#endif
	setkey(
#ifdef __STDC__
	char	*key)
#else
	key)
char	*key;
#endif
{
	des_cblock	des_block;


	BLOCK_TO_DES(key, des_block);

	VOID des_fixup_key_parity((des_cblock *) des_block);

	VOID des_sched((des_cblock *) des_block, key_schedule);

/*
	key_valid = 1;
*/
}

/*
 * encrypt
 *
 *	encrypt - Optional X/Open encoding function.
 *
 */

#ifdef __STDC__
void
#endif
	encrypt(
#ifdef __STDC__
	char	block[64],
	int	edflag)
#else
	block, edflag)
char	block[64];
int	edflag;
#endif
{
	des_cblock	crypt_block;


/*
	if (!key_valid) {
		errno = EINVAL;
		return;
	}
*/

	BLOCK_TO_DES(block, crypt_block);

	VOID des_dea((des_cblock *) crypt_block,
		     (des_cblock *) crypt_block,
		     key_schedule,
		     (edflag) ? DES_DECRYPT : DES_ENCRYPT);

	DES_TO_BLOCK(crypt_block, block);
}
