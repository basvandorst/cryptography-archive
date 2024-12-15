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

#include	<stdio.h>
#include	<string.h>
#include	"tty.h"
#include	"config.h"

/*
 * TTY modification routines.
 */


extern int	tty_disable_echo(
#ifdef __STDC__
	FILE		*tty,
	struct termios	*orig_tty_state)
#else
	tty, orig_tty_state)
FILE		*tty;
struct termios	*orig_tty_state;
#endif
{
	register int	status;
	struct termios	no_echo;


	status = gtty(fileno(tty), orig_tty_state);
	status = tcgetattr(fileno(tty), orig_tty_state);
	if (!status) {
		VOID memcpy(& no_echo ,orig_tty_state, sizeof(struct termios));
		no_echo.c_lflag = (no_echo.c_lflag & ~ ECHO) | ECHONL;
		status = tcsetattr(fileno(tty), TCSAFLUSH, &no_echo);
	}
	return status;
}

extern int	tty_reset(
#ifdef __STDC__
	FILE		*tty,
	struct termios	*orig_tty_state)
#else
	tty, orig_tty_state)
FILE		*tty;
struct termios	*orig_tty_state;
#endif
{
	return tcsetattr(fileno(tty), TCSADRAIN, orig_tty_state);
}
