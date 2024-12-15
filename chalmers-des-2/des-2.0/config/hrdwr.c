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

#include	<stdio.h>
#include	<signal.h>
#include	<setjmp.h>
#include	"config.h"

/*
 * hardware.c
 *
 *	Generates hardware specific information
 *
 */


jmp_buf		sig_jmp;
static int	caught_signal = 0;

static SIGRETURN	caught_sig(
#ifdef __STDC__
	void
#endif
)
{
	caught_signal = 1;
	longjmp(sig_jmp, 1);
	RETURN_SIGFUNC(0);
}

main(argc, argv)
int	argc;
char	*argv[];
{
	unsigned long		l, lm;
	unsigned int		in, im;
	unsigned char		c, cm, cl[sizeof(l)], al[sizeof(l) * 2];
	register int		i;
	struct sigaction	new_action, old_action;


        VOID printf("/*\n");
	VOID printf(" * This file is automaticly generated, do not change.\n");
	VOID printf(" */\n\n");

	/*
	 * Number of bits in an unsigned char.
	 */
	for (c = cm = 0x1, i = 0; c; c <<= 1, cm = (cm << 1) | 0x1, i++);
	VOID printf("#define UNSIGNED_CHAR_BITS\t%u\n", i);
	VOID printf("#define UNSIGNED_CHAR_MAX\t%u\n\n", (int) cm);

	/*
	 * Number of bits in an unsigned int.
	 */
	for (in = im = 0x1, i = 0; in; in <<= 1, im = (im << 1) | 0x1, i++);
	VOID printf("#define UNSIGNED_INT_BITS\t%u\n", i);
	VOID printf("#define UNSIGNED_INT_MAX\t%u\n\n", im);

	/*
	 * Number of bits in an unsigned long.
	 */
	for (l = lm = 0x1l, i = 0; l; l <<= 1, lm = (lm << 1) | 0x1l, i++);
	VOID printf("#define UNSIGNED_LONG_BITS\t%u\n", i);
	VOID printf("#define UNSIGNED_LONG_MAX\t%lu\n\n", lm);

	/*
	 * Byte order in an unsigned long.
	 */
	cl[0] = 1;
	for (i = 1; i < sizeof(cl); i++)
		cl[i] = 0;
	l = *((unsigned long *) cl);
	if (l > 1) {
		VOID printf("#define UNSIGNED_MSB_FIRST\n\n");
	} else {
		VOID printf("#define UNSIGNED_LSB_FIRST\n\n");
	}

#ifdef SIGBUS
	/*
	 * Check if address alignment is necesry on unsigned long.
	 * This method does only work if a SIGBUS signal is generated
	 * when fetching an unsigned long from an unaligned address.
	 */

	for (i = 0; i < sizeof(al); i++)
		al[i] = i;

	new_action.sa_handler = caught_sig;
	VOID sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;
	VOID sigaction(SIGBUS, &new_action, &old_action);
	if (!setjmp(sig_jmp))
		for (i = 0; i < sizeof(l); i++)
			l += *((unsigned long *) & al[i]);
	VOID sigaction(SIGBUS, &old_action, NULL);
#endif /* SIGBUS */

	if (caught_signal) {
		VOID printf("#define UNSIGNED_LONG_ALIGN\n\n");
		caught_signal = 0;
	} else {
		VOID printf("/* #define UNSIGNED_LONG_ALIGN */\n\n");
	}

	exit(0);
}
