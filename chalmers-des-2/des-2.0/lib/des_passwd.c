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
#include	"des.h"
#include	"config.h"
#include	"prompt.h"
#include	"version.h"

/*
 * des_read_password
 *
 *	This routine shows the string `prompt' on the current tty and reads
 *	a line from the current tty. If the `verify' argument is non-zero,
 *	then a second prompt requesting verification is shown. If the first
 *	and second lines do not match, then the process is tried again until
 *	both match.
 *
 *	The input echo is switched off during the operation.
 *
 *	The string (excluding any newline) is converted to a DES key by use
 *	of `des_string_to_key'.
 *
 *	`des_read_password' returns 0 if the operation was successful,
 *	-1 if the echo could not be switched off or -2 if the input string
 *	could not be obtained. In teh last case, no key is generated.
 */


int	des_read_password(
#ifdef __STDC__
	des_cblock	*key,
	char		*prompt,
	int		verify)
#else
	key, prompt, verify)
des_cblock	*key;
char		*prompt;
int		verify;
#endif
{
	register char	*keystr;
#ifdef DES_MAX_PASSWD
	char		pwd_buf[DES_MAX_PASSWD];


	keystr = prompt_read(prompt, pwd_buf, sizeof(pwd_buf), verify);
#else  /* DES_MAX_PASSWD */
	keystr = prompt_read(prompt, (char *) 0, 0, verify);
#endif /* DES_MAX_PASSWD */

	/* Make a des key from the string key. */
	if (keystr != NULL) {
		VOID des_string_to_key(keystr, *key);
#ifndef DES_MAX_PASSWD
		free(keystr);
#endif  /* !DES_MAX_PASSWD */
	}

	return prompt_read_error;
}
