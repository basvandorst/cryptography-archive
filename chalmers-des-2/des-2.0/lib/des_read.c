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

#include	<stdlib.h>
#include	<string.h>
#include	"config.h"
#include	"prompt.h"
#include	"version.h"

/*
 * des_read_pw_string
 *
 *	`des_read_pw_string' reads a string from the terminal and returns
 *	the contentse in the give buffer.
 *	The function is only used for compability reasons with the MIT
 *	DES implementation.
 *
 *	If any error was detected 1 is retuned, otherwise 0.
 */

int	des_read_pw_string(
#ifdef __STDC__
	char	buf[],
	int	buf_len,
	char	*prompt,
	int	verify)
#else
	buf, buf_len, prompt, verify)
char	buf[];
int	buf_len;
char	*prompt;
int	verify;
#endif
{
	return prompt_read(prompt, buf, buf_len, verify) == NULL;
}
