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
#include	"config.h"
#include	"read.h"
#include	"version.h"


#ifndef DES_MAX_PASSWD

# define realloc fake_realloc

/*
 * fake_realloc
 *
 *	`fake_realloc' is similar to the `realloc' function but does only
 *	work on null terminated character strings.
 *
 *	`fake_realloc' replaces `realloc' to ensure that the contentse in
 *	`ptr' is left undisturbed if `malloc' fails.
 *
 */

static
#ifdef __STDC__
       void
#else
       void
#endif
		*fake_realloc(
#ifdef __STDC__
	void	*ptr,
	size_t	size)
#else
	ptr, size)
char	*ptr;
size_t	size;
#endif
{
	register
#ifdef __STDC__
		void
#else
		char
#endif
			*new;


	if ((new = malloc(size)) && ptr) {
		VOID strcpy((char *) new, (char *) ptr);
		free(ptr);
	}

	return new;
}


static char	*read_malloc_line(
#ifdef __STDC__
	FILE	*fd)
#else
	fd)
FILE	*fd;
#endif
{
	register int	c;
	register size_t	size, len;
	register char	*str, *tmp;


	size = len = 0;

	do {
		size += 16;
		tmp = realloc(str, size);
		if (!tmp)
			goto error;
		str = tmp;
		do {
			c = fgetc(fd);
			if (ferror(fd))
				goto error;
			str[len++] = (char) c;
		} while (len >= size - 2 && c != EOF &&  c != '\n');
	} while (c != EOF && c != '\n');

	str[len] = '\0';
	return str;

error:
	if (str)
		free(str);
	return NULL;
}
#endif /* !DES_MAX_PASSWD */



/*
 * read_line
 *
 *	`read_line' read from `fd' up to a newline or end of file.
 *	and returns a null terminated string with the result.
 *	If the `buf' parameter is null then dynamic storage space is
 *	allocated. If `buf' is non-null, the `buf_len' paramter must contain
 *	the buffer length.
 *
 *	If any read error was detected or no storage space could be
 *	allocated, null is retuned.
 */


char	*read_line(
#ifdef __STDC__
	FILE	*fd
	char	*buf,
	int	buf_len)
#else
	fd, buf, buf_len)
FILE	*fd;
char	*buf;
int	buf_len;
#endif
{
	register int	len;
	register char	*str;


	str = NULL;
#ifndef DES_MAX_PASSWD
	if (buf != NULL) {
#endif  /* DES_MAX_PASSWD */
		if (buf_len <= 0)
			goto error;

		if (fgets(buf, buf_len, fd) == NULL)
			goto error;
		if (ferror(fd))
			goto error;

		len = strlen(buf);
		if (len > 0)
			if (buf[len - 1] == '\n')
				buf[--len] = '\0';

		str = buf;
#ifndef DES_MAX_PASSWD
	} else {
		str = read_malloc_line(fd);
		if (str == NULL)
			goto error;
		return str;
	}
#endif  /* DES_MAX_PASSWD */

	return str;

error:
	return NULL;
}
