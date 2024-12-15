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

#define NEED_TIMES

#include	<sys/types.h>
#include	<sys/times.h>
#include	<time.h>
#include	"local_def.h"
#include	"des.h"
#include	"version.h"

/*
 * des_random_cblock
 *
 *	The routine returns a random generated DES 64-bit block.
 *
 *	This method may not be the fastest, but produces a fairly random
 *	result that is not easy to predict (at least I belive this is the
 *	case).
 *
 *	This alogirthm is purely of my own construction.
 *
 *	One weakness is if any "cracker" has the following information:
 *
 *		1) This alogrithm (must be assumed).
 *
 *		2) The process id of the process that used the
 *		   algorithm is within a limited set.
 *
 *		3) The time when the algorithm was used is within
 *		   a limited set.
 *
 *		4) The time of the last reboot for the machine on
 *		   which the algorithm was used.
 *
 *	A "cracker" can then perform an exhaustive serach for all
 *	input values to the algorithm.
 *
 *	The instruduction of the `seed' (1992 03 27) may however strengthen
 *	the algorithm from the weaknesses above. That is ofcourse if it is
 *	used visely.
 *
 */


static des_cblock	seed = {
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};


int	des_random_cblock(
#ifdef __STDC__
	des_cblock *cblock)
#else
	cblock)
des_cblock	*cblock;
#endif
{
	des_cblock		time_pid;
	des_key_schedule	ks;
	register unsigned long	st, pid, t1, t2;
	static unsigned long	s1 = 0x672ffda4l, s2 = 0xe32c5240l;
	struct tms		buf;


	t1 = (unsigned long) time(NULL);	/* Current time in seconds. */
	t2 = (unsigned long) times(&buf);	/* Clockticks since reboot. */

	pid = (unsigned long) getpid();

	st = s1;
	s1 = ((s2 << 1) | !(0x80000000l & s2)) ^ (pid << 17) ^ (t2 << 1);
	s2 = ((st << 1) | !(0x80000000l & st)) ^ (t1 << 1) ^ (t2 << 17);

	LONG_TO_CHAR_8(time_pid, s1, s2);

	/* Make a DES key from the seed */
	VOID des_key_sched((des_cblock *) time_pid, ks);

	/* Encrypt the counter with the new key */
	VOID des_dea((des_cblock *) seed, (des_cblock *) seed, ks, DES_ENCRYPT);

	/* Put the xor of the counter and "seed key" as the new key. */
	XOR2_8((*cblock), seed, time_pid);

	return 0;
}

/*
 * des_init_random_number_generator
 *
 *	The routine initalizes the random numer generator DES key.
 *
 */

int	des_init_random_number_generator(
#ifdef __STDC__
	des_cblock *key)
#else
	key)
des_cblock	*key;
#endif
{
	/* Set the seed value. */
	COPY_8(seed, (*key));

	return 0;
}
