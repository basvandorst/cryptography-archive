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
#include	<stdio.h>
#include	<string.h>
#include	"des.h"
#include	"../lib/read.h"
#include	"filekey.h"
#include	"config.h"

/*
 * read_key
 *
 *	This routine reads a key from `file' and returns a des key throug
 *	`key'. If the flag `hex_flag' is non-zero the key is assumed to be a
 *	64 bit hexadecimal key. Parity adjustment is made to the any 64 bit
 *	hexadecimal key read.
 *
 *	If the key can not be read or the key is faulty, `read_key' returns
 *	-1. 0 is returned if the operation compleated successfuly.
 *
 */


int	read_key(
#ifdef __STDC__
	des_cblock	*key,
	char		*file,
	int		hex_flag)
#else
	key, file, hex_flag)
des_cblock	*key;
char		*file;
int		hex_flag;
#endif
{
	register char	*file_str;
	FILE		*fd;


	file_str = NULL;
	fd = fopen(file, "r");
	if (fd == NULL)
		goto error;

	file_str = read_line(fd);

	VOID fclose(fd);
	fd = NULL;

	if (!file_str)
		goto error;

	if (hex_flag) {
		if (des_hex_to_cblock(file_str, key))
			goto error;
		VOID des_fixup_key_parity(key);
	} else {
		VOID des_string_to_key(file_str, *key);
	}

	free(file_str);

	return 0;

error:
	if (fd != NULL)
		VOID fclose(fd);
	if (file_str)
		free(file_str);
	return -1;
}
