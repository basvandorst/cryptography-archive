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

#define FAKE_SIGACTION

/*
 * This systems does NOT have the sigaction function !.
 */

#include	<errno.h>


typedef unsigned long	sigset_t;

struct	sigaction {
		SIGRETURN	(*sa_handler)();
		sigset_t	sa_mask;
		int		sa_flags;
	};


static int	sigaction(
#ifdef __STDC__
	int			sig,
	struct sigaction 	*act,
	struct sigaction 	*oact)
#else
	sig, act, oact)
int			sig;
struct sigaction 	*act;
struct sigaction 	*oact;
#endif
{
	register int	status;


	if (oact) {
		oact->sa_handler = signal(sig, act->sa_handler);
		status = (int) oact->sa_handler;
	} else
		status = (int) signal(sig, act->sa_handler);

	return (status == -1) ? -1 : 0;
}

static int	sigemptyset(
#ifdef __STDC__
	sigset_t	*set)
#else
	set)
sigset_t	*set;
#endif
{
	*set = 0;

	return 0;
}

static int	sigaddset(
#ifdef __STDC__
	sigset_t	*set,
	int		signo)
#else
	set, signo)
sigset_t	*set;
int		signo;
#endif
{
	extern int	errno;


	if (signo < 0 && signo > 31) {
		errno = EINVAL;
		return -1;
	}

	*set |= (0x1l << signo);

	return 0;
}
