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

/*
 * read_line
 *
 *	`read_line' read from `fd' up to a newline or end of file.
 *	and returns a null terminated string with the result.
 *
 *	If `buf' is zero, the string returned is dynamicaly allocated. 
 *	If `buf' is non-zero, `buf_len' must point to an integer contaning
 *	the buffer length (including trailing null character). The actual
 *	number of characters (excluding traling null character) read are
 *	returned in the integer pointed to by `buf_len'.
 *
 *	If any read error was detected or no storage space could be
 *	allocated, null is retuned.
 */

extern char	*read_line(
#ifdef __STDC__
	FILE	*fd,
	char	*buf,
	int	buf_len
#endif
);
