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
#include	<setjmp.h>
#include	"des.h"
#include	"tty.h"
#include	"sig.h"
#include	"prompt.h"
#include	"read.h"
#include	"config.h"

/*
 * prompt_read
 *
 *	This routine shows the string `prompt' on the current tty and reads
 *	a line from the current tty. If the `verify' argument is non-zero,
 *	then a second prompt requesting verification is shown. If the first
 *	and second lines do not match, then the process is tried again until
 *	both match.
 *
 *	The input echo is switched off during the operation.
 *
 *	If the key can not be read from the tty, NULL is returned and the
 *	global variable `prompt_read_error' set to -2. `prompt_read_error'
 *	is set -1 is there was any problems with terminal echo manipulation.
 *
 */


/*
 * Signal catch routine;
 */

static int	has_set_jmp = 0;
static jmp_buf	signal_jmp;

static SIGRETURN	catch_signal(
#ifdef __STDC__
	void
#endif
)
{
	if (has_set_jmp)
		longjmp(signal_jmp, 1);
	RETURN_SIGFUNC(0);
}

/*
 * Global error information.
 */

int	prompt_read_error;


static char	*prompt_simple(
#ifdef __STDC__
	FILE	*ttyw,
	FILE	*ttyr,
	char	*prompt,
	char	*buf,
	int	buf_len)
#else
	ttyw, ttyr, prompt, buf, buf_len)
FILE	*ttyw;
FILE	*ttyr;
char	*prompt;
char	*buf;
int	buf_len;
#endif
{
	register char	*line;


	line = NULL;

	VOID fputs(prompt, ttyw);
	VOID fflush(ttyw);
	if (ferror(ttyw))
		goto error;

	line = read_line(ttyr, buf, buf_len);
#if (! ECHONL)
	VOID fputc('\n', ttywfd);
#endif /* ! ECHONL */

	return line;

error:
#ifndef DES_MAX_PASSWD
	if (!buf && line)
		free(line);
#endif  /* !DES_MAX_PASSWD */
	return NULL;
}


static char	*prompt_verify(
#ifdef __STDC__
	FILE	*ttyw,
	FILE	*ttyr,
	char	*prompt,
	char	*buf,
	int	buf_len)
#else
	ttyw, ttyr, prompt, buf, buf_len)
FILE	*ttyw;
FILE	*ttyr;
char	*prompt;
char	*buf;
int	buf_len;
#endif
{
	register int	equal, vbuf_len;
	register char	*line;
#ifdef DES_MAX_PASSWD
	char		vbuf[DES_MAX_PASSWD + 1];
#else  /* DES_MAX_PASSWD */
	register int	len;
	register char	*vbuf;
#endif /* DES_MAX_PASSWD */


	line = vbuf = NULL;
#ifdef DES_MAX_PASSWD
	vbuf_len = sizeof(vbuf);
#else  /* DES_MAX_PASSWD */
	vbuf_len = 0;
#endif  /* !DES_MAX_PASSWD */

	for (;;) {
		line = prompt_simple(ttyw, ttyr, prompt, buf, buf_len);
		if (line == NULL)
			goto error;

#ifndef DES_MAX_PASSWD
		len = (buf) ? buf_len : strlen(line) + 1;
		if (vbuf_len < len + 1) {
			if (vbuf)
				free(vbuf);
			vbuf_len = len + 1;
			vbuf = malloc(vbuf_len);
			if (!vbuf)
				goto error;
		}
#endif /* !DES_MAX_PASSWD */

		VOID fputs("Verify ", ttyw);
		if (prompt_simple(ttyw, ttyr, prompt, vbuf, vbuf_len) == NULL)
			goto error;

		if (strcmp(line, vbuf) == 0)
			break;
#ifndef DES_MAX_PASSWD
		if (!buf)
			free(line);
#endif  /* !DES_MAX_PASSWD */

		VOID fputs("The keys do not match, try again\n", ttyw);

	} while (!equal);

#ifndef DES_MAX_PASSWD
	free(vbuf);
#endif /* !DES_MAX_PASSWD */

	return line;

error:
#ifndef DES_MAX_PASSWD
	if (!buf && line)
		free (line);
	if (vbuf)
		free(vbuf);
#endif /* !DES_MAX_PASSWD */
	return NULL;
}


char	*prompt_read(
#ifdef __STDC__
	char	*prompt,
	char	*buf,
	int	buf_len,
	int	verify)
#else
	prompt, buf, buf_len, verify)
char	*prompt;
char	*buf;
int	buf_len;
int	verify;
#endif
{
	register int		tty_modified;
	register char		*line;
	FILE			*ttywfd, *ttyrfd;
	struct termios		old_tty_state;


	line = NULL;
	tty_modified = 0;
	ttywfd = ttyrfd = NULL;
	if (setjmp(signal_jmp))
		goto error;
	has_set_jmp = 1;
#ifdef TTY
	ttywfd = fopen(TTY, "w");
	if (ttywfd == NULL)
		goto error;
	ttyrfd = fopen(TTY, "r");
	if (ttyrfd == NULL)
		goto error;
#else  /* TTY */
	ttywfd = stdout;
	ttyrfd = stdin;
#endif /* TTY */

	push_signals(catch_signal);
	if (tty_disable_echo(ttyrfd, & old_tty_state) < 0)
		prompt_read_error = -1;
	else
		tty_modified = 1;

	line = (verify) ?
		prompt_verify(ttywfd, ttyrfd, prompt, buf, buf_len) :
		prompt_simple(ttywfd, ttyrfd, prompt, buf, buf_len);
	if (line == NULL)
		goto error;

	if (tty_modified) {
		if (tty_reset(ttyrfd, & old_tty_state) < 0)
			prompt_read_error = -1;
		tty_modified = 0;
	}
	pop_signals();

#ifdef TTY
	VOID fclose(ttywfd);
	VOID fclose(ttyrfd);
#endif /* TTY */

	return line;

error:
	if (tty_modified) {
		if (tty_reset(ttyrfd, & old_tty_state) < 0)
			prompt_read_error = -1;
		tty_modified = 0;
	}
	pop_signals();
#ifdef TTY
	if (ttywfd != NULL) {
		VOID fclose(ttywfd);
		ttywfd = NULL;
	}
	if (ttyrfd != NULL) {
		VOID fclose(ttyrfd);
		ttyrfd = NULL;
	}
#endif /* TTY */
#ifndef DES_MAX_PASSWD
	if (!buf && line) {
		free(line);
		line = NULL;
	}
#endif  /* !DES_MAX_PASSWD */
	prompt_read_error = -2;
	return NULL;
}
