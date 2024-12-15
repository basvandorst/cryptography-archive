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

#define NEED_SIGACTION

#include	<signal.h>
#include	"sig.h"
#include	"config.h"

/*
 * Functions to store and restore signal status.
 * The stack has only one level.
 */

static struct sigaction	orig_sig[4];



push_signals(
#ifdef __STDC__
	SIGRETURN (*func)())
#else
	func)
SIGRETURN (*func)();
#endif
{
	struct sigaction	new_action;


	new_action.sa_handler = func;
	VOID sigemptyset(&new_action.sa_mask);
	VOID sigaddset(&new_action.sa_mask, SIGHUP);
	VOID sigaddset(&new_action.sa_mask, SIGINT);
	VOID sigaddset(&new_action.sa_mask, SIGTERM);
	VOID sigaddset(&new_action.sa_mask, SIGQUIT);
	new_action.sa_flags = 0;

	VOID sigaction(SIGHUP, &new_action, &orig_sig[0]);
	VOID sigaction(SIGINT, &new_action, &orig_sig[1]);
	VOID sigaction(SIGTERM, &new_action, &orig_sig[2]);
	VOID sigaction(SIGQUIT, &new_action, &orig_sig[3]);
}

pop_signals()
{
	VOID sigaction(SIGHUP, &orig_sig[0], NULL);
	VOID sigaction(SIGINT, &orig_sig[1], NULL);
	VOID sigaction(SIGTERM, &orig_sig[2], NULL);
	VOID sigaction(SIGQUIT, &orig_sig[3], NULL);
}
