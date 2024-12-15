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
 * mit_convert
 *
 *	Transfomration of a an arbitrary null terminated string to a 64 bit
 *	block.
 *
 *	This algorithm SHOULD be the same as used in the MIT DES package
 *	used in the Kerberos system.
 *
 */

static unsigned int	mit_convert(
#ifdef __STDC__
	char		*str,
	des_cblock	ivec)
#else
	str, ivec)
char		*str;
des_cblock	ivec;
#endif
{
	register int		i;
	register unsigned char	*cp, c;


	DES_ZERO_CBLOCK(ivec);
	cp = (unsigned char *) str;
	while (*cp) {
		for (i = 0; *cp && i < DES_BLOCK_BYTES; i++)
			ivec[i] ^= *cp++;
		while (*cp && --i >= 0) {
			c = *cp++ << 1;
			CHAR_ROTATE(c);	
			ivec[i] ^= c;
		}
	}
	for (i = 0; i < DES_BLOCK_BYTES; i++)
		ivec[i] <<= 1;
	return (unsigned int) (cp - (unsigned char *) str);
}


#ifdef DES_STRING_MODE_CTH
/*
 * cth_convert
 *
 *	Transfomration of a an arbitrary null terminated string to a 64 bit
 *	block.
 *
 *	This is my own algorithm. This algorithm is the same as the MIT
 *	algorithm for strings <= eight characters. This algorithm is
 *	obsolete and is only present for backward compability.
 *
 */

static unsigned int	cth_convert(
#ifdef __STDC__
	char		*str,
	des_cblock	ivec)
#else
	str, ivec)
char		*str;
des_cblock	ivec;
#endif
{
	register unsigned int	i;
	register unsigned char	*cp;
#ifdef DES_LSB_FIRST
	register unsigned char  c;
#endif /* DES_LSB_FIRST */


	DES_ZERO_CBLOCK(ivec);
	cp = (unsigned char *) str;
	while (*cp) 
		for (i = 0; *cp && i < DES_BLOCK_BYTES; i++) {
		/* Shift the character "away" from the parity	*/
		/* bit. Since the key characters (mostly) are	*/
		/* in the range 1 .. 127, the shift ensures	*/
		/* usage of all character bits in the key.	*/
#ifdef DES_LSB_FIRST
			c = *cp++;
			CHAR_ROTATE(c);	
			ivec[i] ^= c;
#else  /* DES_LSB_FIRST */
			ivec[i] ^= *cp++;
#endif /* DES_LSB_FIRST */
		}
	for (i = 0; i < DES_BLOCK_BYTES; i++)
#ifdef DES_LSB_FIRST
		ivec[i] >>= 1;
#else  /* DES_LSB_FIRST */
		ivec[i] <<= 1;
#endif /* DES_LSB_FIRST */
	return (unsigned int) (cp - (unsigned char *) str);
}
#endif /* DES_STRING_MODE_CTH */


static unsigned int	(*convert_func)(
#ifdef __STDC__
	char		*str,
	des_cblock	ivec;
#endif
) = mit_convert;


/*
 * des_set_string_mode
 *
 *	Set the algorithm to use int the `des_string_to_key' function.
 *	Default is to use the MIT algorithm.
 *
 *	This function will be removed in a future release and should NOT
 *	be used for new applications.
 *
 */


int	des_set_string_mode(
#ifdef __STDC__
	int	mode)
#else
	mode)
int		mode;
#endif
{
	switch (mode) {
	case DES_STRING_MODE_MIT:
		convert_func = mit_convert;
		break;
#ifdef DES_STRING_MODE_CTH
	case DES_STRING_MODE_CTH:
		convert_func = cth_convert;
		break;
#endif /* DES_STRING_MODE_CTH */
	default:
		return -1;
	}
	return 0;
}


/*
 * des_string_to_key
 *
 *	Generation of a key from an arbitrary null terminated string.
 *
 */


int	des_string_to_key(
#ifdef __STDC__
	char		*str,
	des_cblock	key)
#else
	str, key)
char		*str;
des_cblock	key;
#endif
{
	des_cblock		ivec;
	des_key_schedule	schedule;
	register unsigned int	n;


	n = (*convert_func)(str, ivec);

	VOID des_fixup_key_parity((des_cblock *) ivec);

	VOID des_sched((des_cblock *) ivec, schedule);
	
	VOID des_cbc_cksum((des_cblock *) str, (des_cblock *) key, n,
			     schedule, (des_cblock *) ivec);

	/* Correct the key parity. */
	VOID des_fixup_key_parity((des_cblock *) key);

	return 0;
}
