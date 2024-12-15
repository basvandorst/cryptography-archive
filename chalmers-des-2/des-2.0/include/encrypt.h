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
#ifndef _ENCRYPT_H_
#define _ENCRYPT_H_

/*
 * setkey
 *
 *	setkey - Optional X/Open set encoding key.
 *
 */

extern
#ifdef __STDC__
	void
#endif
		setkey(
#ifdef __STDC__
			char	*key
#endif
);

/*
 * encrypt
 *
 *	encrypt - Optional X/Open encoding function.
 *
 */

extern
#ifdef __STDC__
	void
#endif
		encrypt(
#ifdef __STDC__
			char	block[64],
			int	edflag
#endif
);

#endif  /* !_ENCRYPT_H_ */
